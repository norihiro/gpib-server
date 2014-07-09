
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
			if(handle==INVALID_HANDLE_VALUE) {
				DWORD err = GetLastError();
				LPVOID lpMsgBuf;
				FormatMessage(
						FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_FROM_SYSTEM |
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						err,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR) &lpMsgBuf,
						0,
						NULL
						);
				fprintf(stderr, "error: serial::open CreateFile err=%d msg=%s\n", err, lpMsgBuf);

				return 1;
			}
			dcb.BaudRate = 9600;
			dcb.ByteSize = 8;
			SetCommState(handle, &dcb);

			COMMTIMEOUTS ct = {
				100,
				2, 1000,
				2, 1000
			};
			SetCommTimeouts(handle, &ct);

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
		int timeout(double t) {
			COMMTIMEOUTS ct = {
				0,
				0, (DWORD)(t*1e3),
				0, (DWORD)(t*1e3)
			};
			return SetCommTimeouts(handle, &ct) ? 0 : (int)GetLastError();
		}
		int close() {
			if(handle!=INVALID_HANDLE_VALUE)
				CloseHandle(handle);
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
