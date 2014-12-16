#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <string>
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
	void recv1(int n_buf_prev);
	void close();
	void execute(char *);
	std::vector<char> buf_recv;

	enum state_e {
		text,
		recv_bin
	} state;

	// state==recv_bin
	std::string dev_next;
	unsigned int size_next;

	client_s() { state=text; }
};

#endif
