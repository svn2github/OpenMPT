libdir = $(plugindir)

lib_LTLIBRARIES        = libmodplug.la

libmodplug_la_LDFLAGS  = -module -avoid-version -lstdc++
libmodplug_la_SOURCES  = plugin.cpp modplugxmms.cpp
libmodplug_la_LIBADD   = ./archive/libarchive.la ./gui/libgui.la ./soundfile/libsoundfile.la

SUBDIRS = archive gui soundfile

noinst_HEADERS = modplugxmms.h plugin.h stddefs.h
