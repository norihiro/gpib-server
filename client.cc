
#include <cstdio>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>

#include "client.h"
#include "device.h"

#define dbf(format, ...) fprintf(stderr, format, __VA_ARGS__)
#define err(format, ...) fprintf(stderr, "error: " format, __VA_ARGS__)

void client_s::close()
{
	if(s!=INVALID_SOCKET) {
		dbf("client(%d) closing\n", s);
		closesocket(s);
		s = INVALID_SOCKET;
	}
}

int client_s::fill_recv()
{
	char buf[1024] = {0};
	int ret = recv(s, buf, sizeof(buf), 0);
	if(ret<=0) {
		close();
	}
	else {
		buf_recv.insert(buf_recv.end(), buf, buf+ret);
	}
	return ret;
}

int client_s::receive()
{
	unsigned int n_buf_prev = buf_recv.size();
	int ret = fill_recv();
	if(ret>0) {
		for(unsigned int i=n_buf_prev; i<buf_recv.size(); ) {
			if(buf_recv[i]=='\n') {
				if(i>0 && buf_recv[i-1]=='\r')
					buf_recv[i-1] = 0;
				buf_recv[i] = 0;
				std::vector<char> cmd(buf_recv.begin(), buf_recv.begin()+i);
				buf_recv.erase(buf_recv.begin(), buf_recv.begin()+i);
				execute(&cmd[0]);
				i = 0;
			}
			else
				i++;
		}
		return 0;
	}
	else {
		close();
		return 1;
	}
}

static void replace_crlf(char *s, char *end)
{
	for(; *s && s!=end; s++) {
		if(*s=='\r' || *s=='\n')
			*s = ' ';
	}
}

static char *parse_cmd(char *&s)
{
	while(*s && isspace(*s)) s++;
	char *r = s;
	while(*s && !isspace(*s)) s++;
	if(*s) *s++ = 0;
	while(*s && isspace(*s)) s++;
	return r;
}

void client_s::execute(char *line)
{
	char *cmd = parse_cmd(line);
	if(!strcmp(cmd, "wr")) {
		char *dev = parse_cmd(line);
		dbf("cmd=<%s> dev=<%s>\n", cmd, dev);
		device_s *d = get_device(dev);
		if(d) {
			d->write(line);

			char buf[1024+1] = "0 ";
			int len = d->read(buf+2, sizeof(buf)-3);
			replace_crlf(buf+2, buf+len+2);
			buf[len+2] = '\n';
			send(s, buf, len+3, 0);
		}
		else
			send(s, "1\n", 2, 0);
	}
	else if(!strcmp(cmd, "w")) {
		char *dev = parse_cmd(line);
		dbf("cmd=<%s> dev=<%s>\n", cmd, dev);
		device_s *d = get_device(dev);
		if(d) {
			d->write(line);
			send(s, "0\n", 2, 0);
		}
		else
			send(s, "1\n", 2, 0);
	}
	else if(!strcmp(cmd, "r")) {
		char *dev = parse_cmd(line);
		dbf("cmd=<%s> dev=<%s>\n", cmd, dev);
		device_s *d = get_device(dev);
		if(d) {
			char buf[1024+1] = "0 ";
			int len = d->read(buf+2, sizeof(buf)-3);
			replace_crlf(buf+2, buf+len+2);
			buf[len+2] = '\n';
			send(s, buf, len+3, 0);
		}
		else
			send(s, "1\n", 2, 0);
	}
	else if(!strcmp(cmd, "c")) {
		char *dev = parse_cmd(line);
		dbf("cmd=<%s> dev=<%s>\n", cmd, dev);
		close_device(dev);
	}
	else if(!strcmp(cmd, "t")) {
		char *dev = parse_cmd(line);
		double t = strtof(line, &line);
		dbf("cmd=<%s> dev=<%s> t=%f\n", cmd, dev, t);
		device_s *d = get_device(dev);
		if(d) {
			int ret = d->timeout(t);
			char buf[32];
			send(s, buf, sprintf(buf, "%d\n", ret), 0);
		}
		else
			send(s, "1\n", 2, 0);
	}
	else {
		err("unknown command cmd=<%s>\n", cmd);
	}
}
