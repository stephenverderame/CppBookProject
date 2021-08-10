/// \file OS specific includes for networking APIs
/// Includes some basic defines for cross platform usage
#pragma once
#ifdef WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
using socket_t = SOCKET;
#define lastError WSAGetLastError()
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <errno.h>
using socket_t = int;
#define lastError errno
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif
using port_t = unsigned short;

/// Enables or disables blocking mode on a socket
void sock_block(socket_t sock, bool blocking);