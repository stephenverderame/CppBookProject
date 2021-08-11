/// \file Tests the parsing of URL encoded payloads in HTTP requests
#include "MockPort.h"
#include <gtest/gtest.h>
#include <map>
#include <string>
#include <random>
#include <sstream>
using namespace testing;

/**
* Test fixture for testing url encoded requests to an HTTP server
*/
class QueryTestFixture : public Test {
protected:
    std::map<std::string, std::string> lastQueryData;

    MockPort port;

    std::default_random_engine randEng{ std::random_device{}() };
    std::uniform_int_distribution<short> asciiGenerator{ 32, 126 };
    std::uniform_int_distribution<int> sizeGenerator{ 10, 2048 };
    std::uniform_int_distribution<int> paramCountGenerator{ 1, 30 };

    /**
    * Generates a random sequence of ascii characters
    */
    auto gen_word() {
        const auto sz = sizeGenerator(randEng);
        std::string res;
        std::generate_n(std::back_inserter(res), sz,
            [this]() { return static_cast<char>(asciiGenerator(randEng)); });
        return res;
    }

    /**
    * Clears `lastQueryData` and refills it with a random set of key value pairs
    */
    const auto& gen_params() {
        const auto params = paramCountGenerator(randEng);
        lastQueryData.clear();
        std::generate_n(std::inserter(lastQueryData, lastQueryData.end()), 
            params, [this]() {
            return std::make_pair(gen_word(), gen_word());
        });
        return lastQueryData;
    }

    /// URL encodes the given parameter
    auto url_encode(const std::string& s) const {
        //TODO
        return s;
    }

    /**
    * Gets a url encoded string of `lastQueryData`
    */
    auto query_params_to_str() {
        std::stringstream ss;
        for (auto& [key, val] : lastQueryData) {
            ss << url_encode(key) << '=' << url_encode(val) << '&';
        }
        const auto res = ss.str();
        return res.substr(0, res.size() - 1); // cut off last &
    }

    /**
    * Gets a HTTP client request with a url encoded payload
    */
    std::vector<char> get_message() {
        gen_params();
        std::stringstream ss;
        if (rand() % 2) {
            ss << "GET /index.html?" << query_params_to_str()
                << "HTTP/1.1\r\n"
                << "Host: 127.0.0.1\r\n"
                << "Accept: */*\r\n\r\n";
        }
        else {
            const auto data = query_params_to_str();
            ss << "POST /index.html HTTP/1.1\r\n"
                << "Host: 127.0.0.1\r\n"
                << "Content-Type: application/x-www-form-urlencode\r\n"
                << "Content-Length: " << data.size() << "\r\n\r\n"
                << data << "\r\n";
        }
        return { std::istream_iterator<char>(ss), 
            std::istream_iterator<char>() };
    }
public:
    QueryTestFixture() {
        ON_CALL(port, read(_)).WillByDefault(
            InvokeWithoutArgs([this]() { return get_message(); })
        );

        ON_CALL(port, read()).WillByDefault(
            Invoke([this]() {return get_message(); })
        );
    }
};

TEST_F(QueryTestFixture, parseUrlEncode) {
    ASSERT_THAT(port.read(), Not(IsEmpty()));
}