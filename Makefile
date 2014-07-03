
# to build code, use mingw32-make
LANG=C
#CL=wine /home/kamae/.wine/drive_c/Program\ Files/Microsoft\ Visual\ Studio\ 10.0/VC/bin/cl.exe

#main.obj: main.cc
#	${CL} $<

OO=main.o client.o device.o device-agilent.o device-serial.o
SRC=main.cc client.cc device.cc device-agilent.cc device-serial.cc
REL=gpib-server.exe libgcc_s_sjlj-1.dll
LDFLAGS=-g -lws2_32 agilent/sicl32.lib
CXXFLAGS=-g

gpib-server.exe: ${OO}
	${CXX} -o $@ ${OO} ${LDFLAGS}

gpib-server-source.zip: $(SRC) Makefile
	zip $@ $(SRC) Makefile

gpib-server-release.zip: $(REL)
	zip $@ $(REL)

clean:
	rm -f *.o *~ gpib-server.exe
