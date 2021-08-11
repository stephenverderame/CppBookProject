/// \file Tests the parsing and handling of HTTP chunked transfer encoding
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "MockPort.h"
#include <sstream>
#include <random>
using namespace testing;

class ChunkedMockTestFixture : public testing::Test {
protected:
    MockPort port;

    std::default_random_engine randEng{ std::random_device{}() };
    std::uniform_int_distribution<short> asciiGenerator{ 32, 126 };
    std::uniform_int_distribution<int> sizeGenerator{ 10, 2048 };

    std::vector<char> lastReadPayload; 
    ///< stores the last payload generated from the last port read call
public:
    ChunkedMockTestFixture() {
        ON_CALL(port, available()).WillByDefault(Return(10));
        ON_CALL(port, read(testing::_)).WillByDefault(
            InvokeWithoutArgs([this]() { return getMessage(); })
        );
        ON_CALL(port, try_read()).WillByDefault(
            Invoke([this]() { return getMessage(); })
        );
        ON_CALL(port, read()).WillByDefault(
            Invoke([this]() { return getMessage(); })
        );
    }

    /**
    * Gets a chunked HTTP plaintext message (with headers) and stores the payload
    * (unchunked) in `lastReadPayload`
    */
    std::vector<char> getMessage() {
        lastReadPayload.clear();
        std::stringstream ss;
        ss << "HTTP/1.1 200 OK \r\n"
            << "Transfer-Encoding: chunked\r\n"
            << "Content-Type: text/plain\r\n"
            << "\r\n";
        do {
            const auto sz = sizeGenerator(randEng);
            ss << std::hex << sz << "\r\n";
            for (auto i = 0; i < sz; ++i) {
                const auto e = static_cast<char>(asciiGenerator(randEng));
                ss << e;
                lastReadPayload.push_back(e);
            }
            ss << "\r\n";
        } while (rand() % 2);
        ss << "0\r\n\r\n";
        return { std::istream_iterator<char>(ss), 
            std::istream_iterator<char>() };
    }
};

/**
* Determined if a message being sent is following chunked transfer encoding.
* @param str a non-owning view of the message being sent
* @return true if `str` is chunked
*/
bool isMsgChunked(std::string_view) {
    //TODO:
    return true;
}

TEST_F(ChunkedMockTestFixture, parseChunked) {
    ASSERT_THAT(port.read(), Not(IsEmpty()));
    //TODO
}

TEST_F(ChunkedMockTestFixture, writeChunked) {
    EXPECT_CALL(port, write(Truly(isMsgChunked)));
    port.write("TODO");
}