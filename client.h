#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>

struct client_s
{
	SOCKET s;
	int receive();
	void close();
	void execute(char *);
	std::vector<char> buf_recv;
};

#endif
