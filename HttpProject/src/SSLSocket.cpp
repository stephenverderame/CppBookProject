#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <SSLSocket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/applink.c>
#include "Address.h"
#include "Networking.h"
#include "FdSet.h"
#include <stdexcept>
#include <string>
#include <sstream>
#undef min
#undef max
struct SSLStart {
    SSLStart() {
        SSL_library_init();
        SSL_load_error_strings();
    }
};

template<typename ... Ts>
std::string format(Ts&& ... args) {
    std::stringstream ss;
    (ss << ... << std::forward<Ts>(args));
    return ss.str();
}

struct SSLSocket::Impl {
    SSL* ssl;
    SSL_CTX* ctx; //< can be nullptr
    socket_t sock;
    Address addr;
    static SSLStart sslCtx;

    Impl(SSL* ssl, SSL_CTX* ctx, socket_t sock, const Address& addr) :
        ssl(ssl), ctx(ctx), sock(sock), addr(addr) {}

    Impl(SSL* ssl, SSL_CTX* ctx, socket_t sock, Address&& addr) :
        ssl(ssl), ctx(ctx), sock(sock), addr(std::move(addr)) {}
};

std::tuple<SSL*, SSL_CTX*> connect_client(const Address& addr, socket_t s) {
    const auto [sockAddr, size] = addr.addr();
    if (connect(s, sockAddr, size))
        throw std::runtime_error(format("Connect client failed: ", lastError));
    const auto method = TLSv1_2_client_method();

    auto ctx = SSL_CTX_new(method);
    if (ctx == NULL)
        throw std::runtime_error(
            format("Failed to create ssl ctx: ", ERR_get_error()));
    const auto ssl = SSL_new(ctx);
    if (ssl == NULL)
        throw std::runtime_error(
            format("Failed to create ssl: ", ERR_get_error()));
    SSL_set_fd(ssl, static_cast<int>(s));
    if (SSL_connect(ssl) <= 0)
        throw std::runtime_error(
            format("Failed to connect ssl: ", ERR_get_error()));

    return std::make_tuple(ssl, ctx);

}

std::tuple<SSL*, SSL_CTX*> setup_server(const char * certFile, const char* keyFile) 
{
    const SSL_METHOD* meth = TLSv1_2_server_method();
    auto ctx = SSL_CTX_new(meth);
    SSL_CTX_set_ecdh_auto(ctx, 1);

    if (SSL_CTX_use_certificate_file(ctx, certFile, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        throw ERR_get_error();
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, keyFile, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        throw ERR_get_error();
    }

    const auto ssl = SSL_new(ctx);
    return std::make_tuple(ssl, ctx);
}

SSLSocket::SSLSocket(const Address& addr) {
    auto s = socket(addr.family(), SOCK_STREAM, NULL);
    if (s == INVALID_SOCKET)
        throw std::runtime_error(format("Failed to create client sock: ", lastError));
    auto [ssl, ctx] = connect_client(addr, s);
    pimpl = std::make_unique<Impl>(ssl, ctx, s, addr);
};

SSLSocket::SSLSocket(const Address& addr, const char * certFile, const char * keyFile) {
    auto s = socket(addr.family(), SOCK_STREAM, NULL);
    if (s == INVALID_SOCKET)
        throw std::runtime_error(format("Failed to create server sock: ", lastError));
    auto [ssl, ctx] = setup_server(certFile, keyFile);
    const auto [sockAddr, sz] = addr.addr();
    if (bind(s, sockAddr, sz) == SOCKET_ERROR)
        throw std::runtime_error(format("Failed to bind sock: ", lastError));
    if (listen(s, SOMAXCONN) == SOCKET_ERROR)
        throw std::runtime_error(format("Failed to listen sock: ", lastError));
    pimpl = std::make_unique<Impl>(ssl, ctx, s, addr);
};

SSLSocket::~SSLSocket() {
    if (pimpl) {
        if (pimpl->ssl != nullptr)
            SSL_free(pimpl->ssl);
        if (pimpl->ctx != nullptr)
            SSL_CTX_free(pimpl->ctx);
    }
};

void SSLSocket::write(std::string_view data) {
    auto sent = decltype(data.size()){0};
    while (sent < data.size()) {
        auto ret = SSL_write(pimpl->ssl, data.data() + sent, 
            static_cast<int>(data.size() - sent));
        if (ret <= 0) {
            throw std::runtime_error(
                format("Failed to write ssl: ", SSL_get_error(pimpl->ssl, ret)));
            // TODO: actual execption
        }
        sent += ret;
    }
}

std::vector<char> SSLSocket::read(size_t minBytes) {
    sock_block(pimpl->sock, true);
    size_t read = 0;
    std::vector<char> buf(4096);
    do {
        if (read >= buf.size() - 1024)
            buf.resize(buf.size() * 2);
        auto ret = SSL_read(pimpl->ssl, &buf[read], 
            static_cast<int>(std::min(buf.size() - read, minBytes - read)));
        if (ret <= 0)
            throw std::runtime_error(
                format("Failed to read ssl: ", SSL_get_error(pimpl->ssl, ret)));
        read += ret;
    } while (minBytes != 0 && minBytes - read > 0);

    buf.resize(read);
    return buf;
}

std::vector<char> SSLSocket::try_read() {
    sock_block(pimpl->sock, false);
    size_t read = 0;
    std::vector<char> buf(4096);
    do {
        if (read >= buf.size() - 1024)
            buf.resize(buf.size() * 2);
        auto ret = SSL_read(pimpl->ssl, &buf[read], 
            static_cast<int>(buf.size() - read));
        if (ret <= 0) {
            const auto errCode = SSL_get_error(pimpl->ssl, ret);
            if (errCode == SSL_ERROR_WANT_READ || errCode == SSL_ERROR_WANT_WRITE)
                break;
            else
                throw std::runtime_error(
                    format("Failed to read ssl nb: ", errCode));
        }
        read += ret;
    } while (true);

    buf.resize(read);
    return buf;
}

size_t SSLSocket::available() const noexcept {
    return SSL_pending(pimpl->ssl);
}

SSLSocket SSLSocket::accept() const {
    if (!pimpl->addr.is_server())
        throw std::string("Can only accept on a server socket");
    auto connectionAddr = pimpl->addr;
    auto [addr, sz] = connectionAddr.addr_mut();
    const auto connection = ::accept(pimpl->sock, addr, &sz);
    if (connection == INVALID_SOCKET) {
        throw std::runtime_error(
            format("Failed to accept connection: ", lastError));
    }
    auto connectionSsl = SSL_new(pimpl->ctx);
    if (connectionSsl == NULL) {
        ERR_print_errors_fp(stderr);
        throw ERR_get_error();
    }
    if (SSL_set_fd(connectionSsl, static_cast<int>(connection)) == 0)
        throw SSL_get_error(connectionSsl, 0);
    const auto ret = SSL_accept(connectionSsl);
    if (ret <= 0) {
        throw std::runtime_error(
            format("Failed to accept ssl connection: ", 
                SSL_get_error(connectionSsl, ret)));
    }
    return SSLSocket(connection, connectionSsl, std::move(connectionAddr));
    
}

SSLSocket::SSLSocket(size_t sock, void* ssl, Address&& addr) :
    pimpl(std::make_unique<Impl>(reinterpret_cast<SSL*>(ssl), nullptr, sock, std::move(addr))) {}

SSLSocket::SSLSocket(SSLSocket&&) noexcept = default;
SSLSocket& SSLSocket::operator=(SSLSocket&&) noexcept = default;

void SSLSocket::add_to_fd(FdSet& fd) const {
    return fd.add(static_cast<int>(pimpl->sock));
}

bool SSLSocket::is_in_fd(const FdSet& fd) const {
    return fd.is_set(static_cast<int>(pimpl->sock));
}

void SSLSocket::remove_from_fd(FdSet& fd) const {
    fd.remove(static_cast<int>(pimpl->sock));
}