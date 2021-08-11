/// \file Tests the logic of remote port implementations
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Port.h>
#include <future>
#include <SSLSocket.h>
#include <Networking.h>
#include <Address.h>
#include <random>

constexpr auto testCount = 500;

/// Gets a random byte array
std::vector<char> randomBuffer(int minSize, int maxSize) {
    static auto seeder = std::random_device{};
    static auto randomEng = std::default_random_engine(seeder());
    static auto randomGen = std::uniform_int_distribution(minSize, maxSize);
    static auto randomElemGen = std::uniform_int_distribution(0, 255);

    const auto sz = randomGen(randomEng);
    std::vector<char> buf;
    std::generate_n(std::back_inserter(buf), sz, []() {
        return static_cast<char>(randomElemGen(randomEng));
    });
    return buf;
}
#ifdef WIN32
struct wsa {
    wsa() {
        WSAData data;
        auto ret = WSAStartup(MAKEWORD(2, 1), &data);
        if (ret != 0)
            throw std::runtime_error("Failed to init Winsock: "
                + std::to_string(ret));
    }

    ~wsa() {
        WSACleanup();
    }
};

static wsa ctx;
#endif

struct SSLSockFactory {
    static SSLSocket makeServer(port_t port) {
        return SSLSocket(Address(port), "data/cert.pem", "data/key.pem");
    }

    static SSLSocket makeClient(std::string_view ip, port_t port) {
        return SSLSocket(Address(ip, port));
    }
};

/**
* Test fixture for testing socket types.
* 
* @param <Sock> a pair of a socket type and its corresponding test factory
*   The test factory must define `static makeServer(port)` and
*   `static makeClient(addr, port)` functions which return instances of
*   the socket type. The socket type must be a subtype of Port
*/
template<class Sock>
class SocketTest : public testing::Test {
protected:
    using fixture_sock_t = decltype(std::declval<Sock>().first);
    static_assert(is_remote_port_v<fixture_sock_t>);
    using factory_t = decltype(std::declval<Sock>().second);

    std::unique_ptr<Port> client, server, serverConnection;
    // client is the socket of the client
    // server is the socket of the server connection listener
    // serverConnection is the server's connected socket to the client
public:
    SocketTest() {
        static port_t port = 5430;
        auto serverSocket = std::make_unique<fixture_sock_t>(factory_t::makeServer(port));
        auto fut = std::async(std::launch::async, [&serverSocket]() {
            return std::make_unique<fixture_sock_t>(serverSocket->accept());
            // new thread to accept the client since accept is blocking and so is connect
        });
        client = std::make_unique<fixture_sock_t>(factory_t::makeClient("127.0.0.1", port++));
        serverConnection = fut.get(); // waits for accept to be done, then gets result
        server = std::unique_ptr<Port>(std::move(serverSocket));
    }
protected:
    /**
    * Runs the specified test functions in both directions repeadetly with
    * different inputs
    * 
    * @param testDirection function which takes a message payload, sender, and receiver
    *   The sender sends the payload and must assert the receiver receives it correctly
    */
    void testRepititions(std::function<void(std::vector<char>&, 
        std::unique_ptr<Port>&, std::unique_ptr<Port>&)> testDirection) 
    {
        for (auto cnt = 0; cnt < testCount * 0.8; ++cnt) {
            auto data = randomBuffer(1, 5000);

            testDirection(data, client, serverConnection);

            std::shuffle(data.begin(), data.end(),
                std::default_random_engine(std::random_device{}()));

            testDirection(data, serverConnection, client);
        }
    }
};

using SocketTestTypes = testing::Types<std::pair<SSLSocket, SSLSockFactory> /*Add your socket*/>;
TYPED_TEST_SUITE(SocketTest, SocketTestTypes);

TYPED_TEST(SocketTest, readAndWrite) {
    for (auto cnt = 0; cnt < testCount; ++cnt) {
        auto data = randomBuffer(1, 5000);
        client->write({ data.data(), data.size() });
        ASSERT_THAT(serverConnection->read(data.size()), testing::ContainerEq(data));

        std::shuffle(data.begin(), data.end(), 
            std::default_random_engine(std::random_device{}()));

        serverConnection->write({ data.data(), data.size() });
        ASSERT_THAT(client->read(data.size()), testing::ContainerEq(data));
    }
}

TYPED_TEST(SocketTest, nonBlockReadWrite) {
    const auto testDirection = [](auto& data, auto& sender, auto& receiver) {
        static_cast<void>(std::async(std::launch::async, [&]() {
            sender->write({ data.data(), data.size() });
        }));

        std::vector<char> recvBuffer;
        do {
            const auto read = receiver->try_read();
            if (!read.empty())
                recvBuffer.insert(recvBuffer.end(), read.begin(), read.end());
        } while (recvBuffer.size() < data.size());
        ASSERT_THAT(recvBuffer, testing::ContainerEq(data));
    };

    testRepititions(testDirection);


}

TYPED_TEST(SocketTest, interleavedRW) {
    const auto testDirection = [](auto& data, auto& sender, auto& receiver) {
        auto midWay = data.size() / 2;
        sender->write({ data.data(), midWay });
        std::vector<char> recvBuffer = receiver->read(midWay);
        sender->write({ data.data() + midWay, data.size() - midWay });
        const auto rest = receiver->read(data.size() - midWay);
        recvBuffer.insert(recvBuffer.end(), rest.begin(), rest.end());
        
        ASSERT_THAT(recvBuffer, testing::ContainerEq(data));
    };

    testRepititions(testDirection);
}

TYPED_TEST(SocketTest, chunkedRW) {
    const auto testDirection = [](auto& data, auto& sender, auto& receiver) {
        auto midWay = data.size() / 2;
        static_cast<void>(std::async(std::launch::async, [&]() {
            sender->write({ data.data(), midWay });
            sender->write({ data.data() + midWay, data.size() - midWay });
        }));
        const auto readData = receiver->read(data.size());

        ASSERT_THAT(readData, testing::ContainerEq(data));
    };

    testRepititions(testDirection);
}