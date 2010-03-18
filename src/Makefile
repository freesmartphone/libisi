SOURCES:=$(wildcard gisi/*.c *.c)
OBJECTS:=$(patsubst %.c, %.o, $(SOURCES))
CFLAGS:=`pkg-config --cflags glib-2.0`
LIBS:=`pkg-config --libs glib-2.0`
NAME:=libisi.so.0
LONGNAME:=libisi.so.0.0.0

all: $(LONGNAME)

%.o: %.c
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -I. -Igisi -I isi -g -c -std=c99 -fPIC -o $@ $<

$(LONGNAME): $(OBJECTS)
	@echo "[LD] $@"
	@$(CC) -shared -Wl,-soname,$(NAME) -o $@ $(LIBS) -g $^

clean:
	rm -f test gisi/*.o *.o $(LONGNAME)

.PHONY: all clean
