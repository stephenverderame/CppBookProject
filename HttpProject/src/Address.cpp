#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Address.h>
#include <Networking.h>
#include <algorithm>
#include <string_view>
#include <string>
#include <stdexcept>

Address::Address(unsigned short port)
{
    addrData.sin_family = AF_INET; //ipv4
    addrData.sin_port = htons(port);
    addrData.sin_addr.S_un.S_addr = INADDR_ANY;
}

/// @return true if the address in an ipv4 address
inline bool is_ip(std::string_view addr) {
    return std::count(addr.begin(), addr.end(), '.') == 3
        && std::all_of(addr.begin(), addr.end(), [](auto e) {
        return e == '.' || isdigit(e);
            });
}

Address::Address(std::string_view addrStr, unsigned short port) : Address(port)
{
    std::string addrTerm(addrStr); // convert to string to ensure it is null-terminating
    if (is_ip(addrStr)) {
        int ret = inet_pton(AF_INET, addrTerm.c_str(), &addrData.sin_addr);
        if (ret == 0)
            throw std::invalid_argument("Invalid address string");
        else if (ret != 1)
            throw lastError;
    }
    else {
        const auto host = gethostbyname(addrTerm.c_str()); 
        //only supports Ipv4, see getaddrinfo() for ipv6 support
        if (host == NULL)
            throw lastError;
        addrData.sin_addr = *reinterpret_cast<in_addr*>(*host->h_addr_list);
    }
}

bool Address::is_server() const noexcept {
    return addrData.sin_addr.S_un.S_addr == INADDR_ANY;
}

int Address::family() const noexcept {
    return addrData.sin_family;
}

std::tuple<const sockaddr*, socklen_t> Address::addr() const noexcept
{
    return std::make_tuple(reinterpret_cast<const sockaddr*>(&addrData), 
        static_cast<socklen_t>(sizeof(addrData)));
}

std::tuple<struct sockaddr*, socklen_t> Address::addr_mut() noexcept
{
    return std::make_tuple(reinterpret_cast<sockaddr*>(&addrData), 
        static_cast<socklen_t>(sizeof(addrData)));
}