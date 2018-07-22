
#ifndef _SNOX_SOCKINC_H_INCLUDE__
#define _SNOX_SOCKINC_H_INCLUDE__

#include <sys/types.h>
#include <assert.h>

#ifdef WIN32

// XXX before include winsock2.h
//#define FD_SETSIZE 1024

#include <winsock2.h>
typedef int socklen_t;

#else

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <poll.h>

typedef int SOCKET;
#define INVALID_SOCKET	-1
#define SOCKET_ERROR -1

// see: setnodelay()
#include <netinet/tcp.h>

#endif

#endif // _SNOX_SOCKINC_H_INCLUDE__
