#include <Networking.h>

void sock_block(socket_t sock, bool blocking) {
#ifdef WIN32
    unsigned long mode = !blocking;
    const auto ret = ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags != 0)
        throw flags;
    if (!blocking)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;
    const auto ret = fcntl(sock, F_SETFL, flags);
#endif
    if (ret != 0) throw ret;
}
