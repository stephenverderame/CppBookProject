#include <string_view>
#include <tuple>
#include "Networking.h"
/// Encapsulates a sockaddr structure
class Address {
    sockaddr_in addrData;
public:
    /// Constructs an address for a server socket
    /// Accepts all clients from any address
    /// @param port port to start server on in host byte order
    explicit Address(unsigned short port);

    /**
    * Constructs an address from an ip or uri and a port
    * @param addr an ip address stored in a string or a named address
    * @param port the port to use in host byte order
    */ 
    Address(std::string_view addr, unsigned short port);

    /**
    * Gets a non-owning pointer of internal address struct
    * @return tuple of pointer to address struct and size of the structure being pointed to
    *   Requires the pointer is not deleted and not used beyond the lifetime of this object
    */
    std::tuple<const struct sockaddr*, socklen_t> addr() const noexcept;

    /**
    * Gets a non-owning mutable pointer of internal address struct
    * @return tuple of pointer to address struct and size of the structure being pointed to
    *   Requires the pointer is not deleted and not used beyond the lifetime of this object
    */
    std::tuple<struct sockaddr*, socklen_t> addr_mut() noexcept;

    /// @return true if this address is an address for a client listener
    bool is_server() const noexcept;

    /// Gets the socket family type (ipv4, ipv6 etc).
    int family() const noexcept;
};