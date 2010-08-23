SOURCES:=$(wildcard gisi/*.c *.c)
OBJECTS:=$(patsubst %.c, %.o, $(SOURCES))
CFLAGS:=`pkg-config --cflags glib-2.0` -D_POSIX_C_SOURCE=200112L
LIBS:=`pkg-config --libs glib-2.0`
UNVERSIONED:=libisi.so
NAME:=libisi.so.0
LONGNAME:=libisi.so.0.0.0
HEADERS:=modem.h network.h device_info.h sim.h

include ../config.mk

all: $(LONGNAME)

%.o: %.c
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -I. -Igisi -I isi -g -c -std=c99 -fPIC -o $@ $<

$(LONGNAME): $(OBJECTS)
	@echo "[LD] $@"
	@$(CC) -shared -Wl,-soname,$(NAME) -o $@ $(LIBS) -g $^

clean:
	rm -f test gisi/*.o *.o $(LONGNAME)

install: all
	@echo Installing shared object...
	@$(INSTALL_LIB) $(LONGNAME) $(DESTDIR)$(PREFIX)/lib
	@ln -sf $(LONGNAME) $(DESTDIR)$(PREFIX)/lib/$(NAME)
	@ln -sf $(NAME) $(DESTDIR)$(PREFIX)/lib/$(UNVERSIONED)
	@echo Installing header files...
	@$(CREATE_DIR) $(DESTDIR)$(PREFIX)/include/isi
	@$(INSTALL_DATA) $(HEADERS) opcodes gisi $(DESTDIR)$(PREFIX)/include/isi

uninstall:
	@echo Removing shared object...
	@rm -rf $(DESTDIR)$(PREFIX)/lib/$(LONGNAME)
	@rm -rf $(DESTDIR)$(PREFIX)/lib/$(NAME)
	@rm -rf $(DESTDIR)$(PREFIX)/lib/$(UNVERSIONED)
	@echo Removing headers...
	@rm -rf $(DESTDIR)$(PREFIX)/include/isi

.PHONY: all clean install
