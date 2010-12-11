SOURCES:=$(wildcard gisi/*.c *.c)
OBJECTS:=$(patsubst %.c, %.o, $(SOURCES))
CFLAGS:=`pkg-config --cflags glib-2.0` -D_POSIX_C_SOURCE=200112L
LIBS:=`pkg-config --libs glib-2.0`
UNVERSIONED:=libisi.so
NAME:=libisi.so.0
LONGNAME:=libisi.so.0.0.0
HEADERS:=modem.h network.h device_info.h sim.h simauth.h gps.h

include ../config.mk

all: $(LONGNAME) glib-enum-binding.h

%.o: %.c
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -I. -Igisi -I isi -g -c -std=c99 -fPIC -o $@ $<

glib-enum-binding.h: $(HEADERS)
	@echo "Generating glib-enum-binding.h"
	@glib-mkenums \
		--fprod "\n/* enumerations from \"@filename@\" */\n#include \"isi/simauth.h\"\n#include \"isi/gps.h\"\n" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {" \
		--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n#define @ENUMNAME@_TYPE (@enum_name@_get_type())\n\n" \
		$^ > glib-enum-binding.h

$(LONGNAME): $(OBJECTS)
	@echo "[LD] $@"
	@$(CC) $(LDFLAGS) -shared -Wl,-soname,$(NAME) -o $@ $(LIBS) -g $^

clean:
	rm -f test gisi/*.o *.o glib-enum-binding.h $(LONGNAME)

install-headers: glib-enum-binding.h
	@echo Installing header files...
	@$(CREATE_DIR) $(DESTDIR)$(PREFIX)/include/isi/gisi
	@$(CREATE_DIR) $(DESTDIR)$(PREFIX)/include/isi/opcodes
	@$(INSTALL_DATA) $(HEADERS) $(DESTDIR)$(PREFIX)/include/isi
	@$(INSTALL_DATA) glib-enum-binding.h $(DESTDIR)$(PREFIX)/include/isi
	@$(INSTALL_DATA) gisi/*.h $(DESTDIR)$(PREFIX)/include/isi/gisi
	@$(INSTALL_DATA) opcodes/*.h $(DESTDIR)$(PREFIX)/include/isi/opcodes

install: install-headers all
	@echo Installing shared object...
	@$(CREATE_DIR) $(DESTDIR)$(PREFIX)/lib
	@$(INSTALL_LIB) $(LONGNAME) $(DESTDIR)$(PREFIX)/lib
	@ln -sf $(LONGNAME) $(DESTDIR)$(PREFIX)/lib/$(NAME)
	@ln -sf $(NAME) $(DESTDIR)$(PREFIX)/lib/$(UNVERSIONED)

uninstall:
	@echo Removing shared object...
	@rm -rf $(DESTDIR)$(PREFIX)/lib/$(LONGNAME)
	@rm -rf $(DESTDIR)$(PREFIX)/lib/$(NAME)
	@rm -rf $(DESTDIR)$(PREFIX)/lib/$(UNVERSIONED)
	@echo Removing headers...
	@rm -rf $(DESTDIR)$(PREFIX)/include/isi

.PHONY: all clean install
