CFLAGS+=-I/usr/include/wireshark -DHAVE_STDARG_H -DHAVE_CONFIG_H -g
OBJECTS:=src/packet-isi.o src/plugin.o src/isi-simauth.o src/isi-gps.o

PREFIX?=/usr
PLUGINDIR?=lib/wireshark/libwireshark0/plugins

all: isi.so

%.o: %.c
	@echo "[CC] $<"
	@$(CC) -o $@ $(CFLAGS) `pkg-config --cflags glib-2.0` -c -fPIC $<

isi.so: $(OBJECTS)
	@echo "[LD] $@"
	@$(CC) -o $@ -shared -Wl,-soname,$@ $^

clean:
	@rm -f isi.so src/*.o

maintainer-clean: clean


install: isi.so
	install isi.so ${PREFIX}/${PLUGINDIR}

.PHONEY: clean maintainer-clean install
