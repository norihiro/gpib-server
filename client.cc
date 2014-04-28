
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
	dbf("client(%d) closing\n", s);
	s = INVALID_SOCKET;
}

int client_s::receive()
{
	char buf[1024] = {0};
	int ret = recv(s, buf, sizeof(buf), 0);
	if(ret>0) {
		for(int i=0; i<ret; i++) {
			if(buf[i]=='\n') {
				if(buf_recv.size() && buf_recv[buf_recv.size()-1]=='\r')
					buf_recv.resize(buf_recv.size()-1);
				buf_recv.push_back(0);
				execute(&buf_recv[0]);
				buf_recv.clear();
			}
			else
				buf_recv.push_back(buf[i]);
		}
		return 0;
	}
	else {
		close();
		return 1;
	}
}

// static void remove_last_space(char *s)
// {
// 	char *e;
// 	for(e=s; *e; e++); e--;
// 	while(
// }

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
	else {
		err("unknown command cmd=<%s>\n", cmd);
	}
}
