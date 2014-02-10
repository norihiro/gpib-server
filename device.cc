
#include <vector>
#include <string>
#include <cstring>
#include <queue>
#include <map>
#include <algorithm>
#include <cstdio>
#include "device.h"

#define dbf(format, ...) fprintf(stderr, format, __VA_ARGS__)

device_s *create_device_agilent(const char *name);
device_s *create_device_serial(const char *name);

class device_echo : public device_s
{
	std::queue<std::string> buf;
	public:
		int write(const char *s) {
			dbf("echo::write(%s)\n", s);
			buf.push(s);
			return 0;
		}
		int read(char *s, int n) {
			dbf("echo::read(%d)\n", n);
			if(buf.size()) {
				strncpy(s, buf.front().c_str(), n);
				n = std::min((int)buf.front().size(), n);
				dbf("echo::read returns n=%d <%s>\n", n, buf.front().c_str());
				buf.pop();
				return n;
			}
			return 0;
		}
		int close() {
			return 0;
		}
};

static std::map<std::string, device_s*> devices;

static device_s *create_device(const char *name)
{
	dbf("create_device(%s)\n", name);
	if(!strcmp(name, "echo")) {
		return new device_echo();
	}
	else if(name[0]=='a') {
		while(*name && *name!=':') name++;
		while(*name && *name==':') name++;
		if(*name)
			return create_device_agilent(name);
		else
			return NULL;
	}
	else if(!strncmp(name, "com", 3)) {
		return create_device_serial(name);
	}
	return NULL;
}

device_s* get_device(const char *name)
{
	dbf("get_device(%s)\n", name);
	if(devices.count(name))
		return devices[name];
	else {
		device_s *d = create_device(name);
		if(d) {
			dbf("create_device succeeded(%d) name=<%s>\n", (int)d, name);
			return devices[name] = d;
		}
		else {
			dbf("create_device failed name=<%s>\n", name);
		}
	}
	return NULL;
}

int close_device(const char *name)
{
	dbf("close_device(%s)\n", name);
	if(devices.count(name)) {
		int ret = devices[name]->close();
		delete devices[name];
		devices.erase(name);
		return ret;
	}
	else {
		dbf("close_device(%s) device not found\n", name);
		return 1;
	}
}
