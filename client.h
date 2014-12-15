#ifndef CLIENT_H
#define CLIENT_H

#ifdef HAVE_WINSOCKET
#include <winsock2.h>
typedef int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define closesocket ::close
#endif

struct client_s
{
	SOCKET s;
	int receive();
	int fill_recv();
	void close();
	void execute(char *);
	std::vector<char> buf_recv;
};

#endif
