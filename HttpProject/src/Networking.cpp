#include <Networking.h>
#include <stdexcept>
#include <string>

void sock_block(socket_t sock, bool blocking) {
#ifdef WIN32
    unsigned long mode = !blocking;
    const auto ret = ioctlsocket(sock, FIONBIO, &mode);
#else
    auto flags = fcntl(sock, F_GETFL, 0);
    if (!blocking)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;
    const auto ret = fcntl(sock, F_SETFL, flags);
#endif
    if (ret != 0) throw
        std::runtime_error("Could not set sock flag: " +
            std::to_string(ret));
}
