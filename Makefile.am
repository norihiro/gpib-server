bin_PROGRAMS = gpibserver
gpibserver_SOURCES = client.cc device.cc main.cc
if SICL32
gpibserver_SOURCES += device-agilent.cc
endif
if SERIAL
gpibserver_SOURCES += device-serial.cc
endif
