SOURCES:=$(wildcard gisi/*.c *.c)
OBJECTS:=$(patsubst %.c, %.o, $(SOURCES))
CFLAGS:=`pkg-config --cflags glib-2.0`
LIBS:=`pkg-config --libs glib-2.0`

all: test

%.o: %.c
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -I. -Igisi -I isi -g -c -std=c99 -o $@ $<

test: $(OBJECTS) main.o modem.o
	@echo "[LD] $@"
	@$(CC) $(LIBS) -g -o $@ $^

clean:
	rm -f test gisi/*.o *.o

.PHONY: all clean
