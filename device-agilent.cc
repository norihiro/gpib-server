#include "config.h"
#include "device.h"
#include <cstdio>
#include <cstring>
#include <cctype>
#define dbf(format, ...) fprintf(stderr, format, __VA_ARGS__)

#ifdef __GNUC__
typedef unsigned long IHandle;
typedef unsigned long IBusAddress;
typedef unsigned long IBusSize;
typedef int INST;
#define __int64 long long
#define SICLAPI
#define _far
extern "C" {
	INST SICLAPI iopen(char _far *addr);
	/* Write/Read */
	int SICLAPI iwrite (INST id, char _far *buf, unsigned long datalen, int endi, unsigned long _far *actual);
	int SICLAPI iread (INST id, char _far *buf, unsigned long bufsize, int _far *reason, unsigned long _far *actual);
	int SICLAPI itimeout(INST id,long tval);
	int SICLAPI iclose(INST id);
}
#else
#include "agilent/sicl.h"
#endif

class agilent : public device_s
{
	INST id;
	public:
		int open(const char *n) {
			id = iopen((char*)n);
			dbf("agilent::open id=%d\n", (int)id);
			itimeout(id, 1000);
			return 0;
		}
		int write(const char *s) {
			dbf("agilent:write(%s)\n", s);
			int len = strlen(s);
			// s[len] = '\n';
			iwrite(id, (char*)s, len, 1, NULL);
			// s[len] = 0;
			return 0;
		}
		int read(char *s, int n) {
			dbf("agilent::read(%d)\n", n);
			int actual;
			iread(id, s, n, NULL, (unsigned long*)&actual);
			while(actual>0 && isspace(s[actual-1])) s[--actual]=0;
			dbf("agilent::read s=<%s>\n", s);
			return (int)actual;
		}
		int timeout(double t) {
			return itimeout(id, (long)(t*1e3));
		}
		int close() {
			return iclose(id);
		}
};

device_s *create_device_agilent(const char *name)
{
	dbf("create_device_agilent(%s)\n", name);
	agilent *p = new agilent();
	p->open(name);
	return p;
}
