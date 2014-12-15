#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>

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
