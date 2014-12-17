#include "config.h"
#include <cstdio>
#include <vector>
#include <algorithm>
#undef max

#include "client.h"

#define PORT 1700
#define BACKLOG 5
#define db(x) x
#define dbf(format, ...) fprintf(stderr, format, __VA_ARGS__)
#define err(format, ...) fprintf(stderr, "error: " format, __VA_ARGS__)

static int init()
{
#ifdef HAVE_WINSOCKET
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
#endif // HAVE_WINSOCKET
	return 0;
}

static int bind_and_listen(SOCKET s)
{
	dbf("bind_and_listen(%d)\n", (int)s);
	struct sockaddr_in a;
	int ret;

	int yes=1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

	a.sin_family = AF_INET;
	a.sin_port = htons(PORT);
#ifdef HAVE_WINSOCKET
	a.sin_addr.S_un.S_addr = INADDR_ANY;
#else
	a.sin_addr.s_addr = INADDR_ANY;
#endif

	ret = bind(s, (struct sockaddr *)&a, sizeof(a));
	if(ret) {
		err("bind returns %d\n", ret);
		return ret;
	}

	ret = listen(s, BACKLOG);
	if(ret) {
		err("listen returns %d\n", ret);
		return ret;
	}

	return 0;
}

client_s accept_socet(SOCKET soc)
{
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	client_s c;
	c.s = accept(soc, (struct sockaddr*)&client, &len);
	dbf("accept %d\n", c.s);
	return c;
}

static int loop()
{
	int ret;
	fd_set fd_mask;
	SOCKET soc = socket(AF_INET, SOCK_STREAM, 0);
	ret = bind_and_listen(soc);
	if(ret)
		return ret;

	FD_ZERO(&fd_mask);
	FD_SET(soc, &fd_mask);
	int nfds = soc+1;
	std::vector<client_s> clients;

	for(;;) {
		fd_set fd_read = fd_mask;
		int ready = select(nfds, &fd_read, NULL, NULL, NULL);
		if(ready>0) {
			if(FD_ISSET(soc, &fd_read)) {
				ready--;
				client_s c = accept_socet(soc);
				FD_SET(c.s, &fd_mask);
				nfds = std::max(nfds, (int)c.s+1);
				clients.push_back(c);
			}
			for(u_int i=0; i<clients.size() && ready>0; i++) {
				SOCKET s = clients[i].s;
				if(FD_ISSET(s, &fd_read)) {
					ready--;
					if(clients[i].receive()) {
						FD_CLR(s, &fd_mask);
						clients.erase(clients.begin()+i);
						i--;
						nfds = soc+1; for(u_int j=0; j<clients.size(); j++) nfds = std::max(nfds, (int)clients[j].s+1);
					}
				}
			}
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	if(int ret=init()) {
		fprintf(stderr, "error: while initializing: %d\n", ret);
		return ret;
	}
	return loop();
}
