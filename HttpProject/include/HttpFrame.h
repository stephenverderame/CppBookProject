#pragma once
#include <string>
#include <string_view>
/// Encapsulates the headers and information of an HTTP request or response frame
class HttpFrame {
    enum class Protocol {
        GET, POST
    } protocol;

    virtual ~HttpFrame() = default;

    /**
    * Composes the frame into a well-formatted HTTP frame
    *
    * @throws <something> if the request is malformed
    * @return string of HTTP request frame
    */
    virtual std::string compose() = 0;

    /**
    * Gets or sets the value of the specified header.
    *
    * If the specified header is not present in the HTTP frame, adds it
    *   with a default value of the empty string
    */
    std::string& operator[](std::string header);

    /// @return true if the HTTP frame contains the specified header
    bool has_header(std::string_view header) noexcept;

    /**
    * Gets the specified header
    * @throws ... if the header isn't found
    */
    const std::string& get(std::string_view header);
};

namespace HttpResponse {
    constexpr const char* ok = "200 OK";
    constexpr const char* created = "201 Created";
    constexpr const char* bad = "400 Bad Request";
    constexpr const char* forbidden = "403 Forbidden";
    constexpr const char* unauth = "401 Unauthorized";
    constexpr const char* not_found = "404 Not Found";
    constexpr const char* not_allow = "405 Method Not Allowed";
    constexpr const char* not_implement = "501 Not Implemented";
    constexpr const char* switch_proto = "101 Switching Protocols";
}

namespace HttpRespNum {
    constexpr const int ok = 200;
    constexpr const int created = 201;
    constexpr const int bad = 400;
    constexpr const int forbidden = 403;
    constexpr const int unauth = 401;
    constexpr const int not_found = 404;
    constexpr const int not_allow = 405;
    constexpr const int not_implement = 501;
    constexpr const int switch_proto = 101;
}