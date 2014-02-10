#ifndef DEVICE_H
#define DEVICE_H

class device_s
{
	public:
		virtual int write(const char *) = 0;
		virtual int read(char *, int) = 0;
		virtual int close() = 0;
		virtual ~device_s() {}
};

class device_s *get_device(const char *name);
int close_device(const char *name);

#endif
