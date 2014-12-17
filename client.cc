#include "config.h"
#include <cstdio>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <stdint.h>

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

void client_s::recv1(int n_buf_prev)
{
	if(state==text) {
		for(unsigned int i=n_buf_prev; i<buf_recv.size(); ) {
			if(buf_recv[i]=='\n') {
				if(i>0 && buf_recv[i-1]=='\r')
					buf_recv[i-1] = 0;
				buf_recv[i] = 0;
				std::vector<char> cmd(buf_recv.begin(), buf_recv.begin()+i+1);
				buf_recv.erase(buf_recv.begin(), buf_recv.begin()+i+1);
				execute(&cmd[0]);
				recv1(0); return;
			}
			else
				i++;
		}
	}
	else if(state==recv_bin) {
		if(buf_recv.size() >= size_next) {
			device_s *d = get_device(dev_next.c_str());
			if(d) {
				int ret = d->write(&buf_recv[0], &buf_recv[size_next]);
				char sz[16]; send(s, sz, sprintf(sz, "%d\n", ret), 0);
			}
			buf_recv.erase(buf_recv.begin(), buf_recv.begin()+size_next);
			state = text;
			recv1(0); return;
		}
	}
}

int client_s::receive()
{
	unsigned int n_buf_prev = buf_recv.size();
	int ret = fill_recv();
	if(ret>0) {
		recv1(n_buf_prev);
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
			d->write(line, line+strlen(line));

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
			d->write(line, line+strlen(line));
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
	else if(!strcmp(cmd, "wb")) {
		dev_next = parse_cmd(line);
		size_next = strtol(line, &line, 10);
		state = recv_bin;
	}
	else if(!strcmp(cmd, "c")) {
		char *dev = parse_cmd(line);
		dbf("cmd=<%s> dev=<%s>\n", cmd, dev);
		close_device(dev);
	}
	else if(!strcmp(cmd, "x")) {
		char *dev = parse_cmd(line);
		dbf("cmd=<%s> dev=<%s>\n", cmd, dev);
		device_s *d = get_device(dev);
		if(d) {
			std::string results;
			int ret = 0;
			bool flag_read = 0;
			bool flag_address = 0;
			int n_word = 1;
			int base = 16;
			const char *fmt = " %x";
			while(line && *line) {
				while(*line && isspace(*line)) line++;
				if(!*line) break;
				if(*line=='-') for(char c,cont=1; cont && (c=*++line); ) switch(c) {
					case 'r': flag_read=1; break;
					case 'w': flag_read=0; break;
					case 'a': flag_address=1; break;
					case '1': n_word=1; break;
					case '2': n_word=2; break;
					case '3': n_word=3; break;
					case '4': n_word=4; break;
					case 'o': base=8; fmt=" %o"; break;
					case 'd': base=10; fmt=" %d"; break;
					case 'x': base=16; fmt=" %x"; break;
					default: cont=0; break;
				}
				else if(flag_read) {
					const uint32_t a = strtol(line, &line, base);
					uint8_t c[4]={0};
					d->seek(a);
					ret |= d->read((char*)c, n_word);
					uint32_t u=0;
					for(int i=0; i<n_word; i++) u |= c[i]<<(i*8);
					char s[64]; sprintf(s, fmt, u);
					results += s;
				}
				else {
					const uint32_t u = strtol(line, &line, base);
					if(flag_address) {
						ret |= d->seek(u);
						flag_address = 0;
					}
					else {
						uint8_t c[4];
						for(int i=0; i<n_word; i++) c[i] = (u>>(i*8))&0xff; // little endian
						ret |= d->write((char*)c, (char*)(c+n_word));
					}
				}
			}
			char sz[64]; sprintf(sz, "%d", ret);
			results = sz + results + "\n";
			send(s, results.c_str(), results.size(), 0);
		}
		else
			send(s, "1\n", 2, 0);
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
