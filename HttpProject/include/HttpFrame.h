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