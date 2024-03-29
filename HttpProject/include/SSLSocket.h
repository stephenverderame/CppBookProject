#pragma once
#include "Port.h"
/// A port to a secure socket
/// Encrypted with TLS 1.2
class SSLSocket : public Port {
    struct Impl;
    std::unique_ptr<Impl> pimpl;

    /// Constructs an SSL socket by taking ownership of a socket
    /// and ssl class
    SSLSocket(unsigned long long sock, void* ssl, class Address&& addr);
public:
    /// Creates a client ssl socket connecting to the given address
    explicit SSLSocket(const class Address& addr);

    /// Creates a server ssl socket on the given address
    /// @param certificateFile .pem certificate file
    /// @param keyFile .pem key file
    SSLSocket(const class Address& addr, const char* certificateFile, const char* keyFile);


    ~SSLSocket();

    size_t available() const noexcept override;

    void write(std::string_view data) override;

    std::vector<char> read(size_t minBytes) override;

    std::vector<char> try_read() override;

    void add_to_fd(class FdSet& fd) const override;

    bool is_in_fd(const class FdSet& fd) const override;

    void remove_from_fd(class FdSet& fd) const override;

    SSLSocket(const SSLSocket&) = delete;
    SSLSocket& operator=(const SSLSocket&) = delete;

    SSLSocket(SSLSocket&&) noexcept;
    SSLSocket& operator=(SSLSocket&&) noexcept;

    /**
    * Gets a new socket connection on this server socket.
    * Requires that this socket is a server socket.
    * Blocks until a connection is available
    */
    SSLSocket accept() const;
};