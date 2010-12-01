bin_PROGRAMS = temperature
temperature_SOURCES = sensors-amd.c  sensors-common.c  sensors.h  sensors-intel.c  sensors-main.c  sensors-pci.c  sensors-xml.c sensors.h
temperature_CFLAGS = -Wall
if USE_PKGCONFIG
  temperature_CPPFLAGS = `@PKGCONFIG@ libxml-2.0 --cflags`
endif
