
#include "device.h"
#include <cstdio>
#include <cctype>
#include <windows.h>

#define dbf(format, ...) fprintf(stderr, format, __VA_ARGS__)

class serial : public device_s
{
	HANDLE handle;
	public:
		int open(const char *n) {
			DCB dcb = {sizeof(DCB)};
			char s[64]={0}; snprintf(s, sizeof(s)-1, "\\\\.\\%s", n);
			handle = CreateFile(s, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			dbf("serial::open handle=%d <%s>\n", (int)handle, s);
			if(handle==INVALID_HANDLE_VALUE)
				return 1;
			dcb.BaudRate = 9600;
			dcb.ByteSize = 8;
			SetCommState(handle, &dcb);
			return 0;
		}
		int write(const char *s) {
			dbf("serial:write(%s)\n", s);
			int len = strlen(s);
			DWORD dw;
			char *x = (char*)s;
			x[len] = '\n';
			WriteFile(handle, x, len+1, &dw, NULL);
			x[len] = 0;
			return 0;
		}
		int read(char *s, int n) {
			dbf("serial::read(%d)\n", n);
			DWORD dw;
			ReadFile(handle, s, n, &dw, NULL);
			int act=(int)dw;
			while(act>0 && isspace(s[act-1])) s[--act]=0;
			dbf("serial::read s=<%s>\n", s);
			return act;
		}
};

device_s *create_device_serial(const char *name)
{
	dbf("create_device_serial(%s)\n", name);
	serial *p = new serial();
	if(p->open(name)) {
		delete p;
		return NULL;
	}
	return p;
}
