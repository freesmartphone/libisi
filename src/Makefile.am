NULL = 

AM_CFLAGS = \
	    $(GLIB_CFLAGS) \
	    -std=c99 \
	    $(NULL)
lib_LTLIBRARIES = \
		  libisi.la \
		  $(NULL)

libisi_la_SOURCES = \
		    debug.c \
		    device_info.c \
		    gpds.c \
		    gps.c \
		    modem.c \
		    network.c \
		    simauth.c \
		    sim.c \
		    gisi/client.c \
		    gisi/iter.c \
		    gisi/gisimodem.c \
		    gisi/netlink.c \
		    gisi/pep.c \
		    gisi/pipe.c \
		    gisi/server.c \
		    gisi/socket.c \
		    gisi/verify.c \
		    $(NULL)

libisi_la_LIBADD = \
		   $(GLIB_LIBS) \
		   $(NULL)

enum_headers = modem.h network.h device_info.h sim.h simauth.h gps.h

isi-enum-types.h: $(enum_headers)
	@echo "Generating isi-enum-types.h"
	$(GLIB_MKENUMS) \
		--fprod '\n/* enumerations from "@filename@" */\n#include "simauth.h"\n#include "gps.h"\n' \
		--vhead 'GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {' \
		--vprod '      { @VALUENAME@, "@VALUENAME@", "@valuenick@" },' \
		--vtail '      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static ("@EnumName@", values);\n  }\n  return etype;\n}\n#define @ENUMNAME@_TYPE (@enum_name@_get_type())\n\n' \
		$^ > isi-enum-types.h

libisiincludedir = $(includedir)/isi
libisiinclude_DATA = \
		     debug.h \
		     device_info.h \
		     gpds.h \
		     gps.h \
		     helper.h \
		     modem.h \
		     network.h \
		     simauth.h \
		     sim.h \
		     isi-enum-types.h \
		     $(NULL)

libisiopcodesincludedir = $(includedir)/isi/opcodes
libisiopcodesinclude_DATA = \
			    opcodes/call.h \
			    opcodes/gpds.h \
			    opcodes/gps.h \
			    opcodes/gss.h \
			    opcodes/info.h \
			    opcodes/mtc.h \
			    opcodes/network.h \
			    opcodes/simauth.h \
			    opcodes/sim.h \
			    opcodes/sms.h \
			    opcodes/ss.h \
			    $(NULL)

libisigisiincludedir = $(includedir)/isi//gisi
libisigisiinclude_DATA = \
			 gisi/client.h \
			 gisi/iter.h \
			 gisi/modem.h \
			 gisi/netlink.h \
			 gisi/pep.h \
			 gisi/phonet.h \
			 gisi/pipe.h \
			 gisi/server.h \
			 gisi/socket.h \
			 $(NULL)

CLEANFILES = isi-enum-types.h
