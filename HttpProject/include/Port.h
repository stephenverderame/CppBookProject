#pragma once
#include <string_view>
#include <vector>
#include <memory>
/// Interface for using an IO port
class Port {
public:
    virtual ~Port() = default;

    // you may add to, but DO NOT modify the existing interface

    /**
    * Gets the lower bound of amount of data in bytes that can be read.
    * 
    * If there is any data on the port, will be at least 1
    */
    virtual size_t available() const noexcept = 0;

    /**
    * Writes all of data to the port.
    */
    virtual void write(std::string_view data) = 0;

    /**
    * Blocking read call.
    * 
    * @param bytes the amount of bytes to read. Will wait until bytes data is read
    *   If bytes is 0, this will read however much data is first available
    * @returns non-empty buffer of read data
    */
    virtual std::vector<char> read(size_t bytes = 0) = 0;

    /**
    * Non blocking read call.
    * 
    * @returns read data or empty buffer if no data is immediately available
    */
    virtual std::vector<char> try_read() = 0;

    /// Adds a port to an fd set
    /// the fd can query activity on this port
    virtual void add_to_fd(class FdSet& fd) const = 0;

    /// Checks if there is activity on this port
    virtual bool is_in_fd(const class FdSet& fd) const = 0;

    /// Removes the port from the fd set
    virtual void remove_from_fd(class FdSet& fd) const = 0;
};

// TODO
// You may want to pass a URI string, or maybe some enums
// std::unique_ptr<Port> make_port();

/**
* Determines if the given type adheres to the remote port concept, which is a port to a remote
* peer such as a socket. If the type adheres, the constexpr member `value` is `true`,
* otherwise it is `false`.
* @{
*/
template<class T, typename = void>
struct RemotePortConcept : public std::false_type {};

template<class T>
struct RemotePortConcept<T, std::void_t<
    std::enable_if_t<std::is_base_of_v<Port, T>>,
    decltype(std::declval<const T>().accept())
    >> : public std::true_type {};
/// @}

/// True if the specified type is a remote port
/// @see RemotePortConcept
template<class T>
constexpr auto is_remote_port_v = 
    RemotePortConcept<std::remove_cv_t<std::remove_reference_t<T>>>::value;