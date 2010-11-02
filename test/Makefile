all: authenticate network

libisi.so:
	ln -s ../src/libisi.so.0.0.0 libisi.so

isi:
	ln -s ../src isi

network: isi libisi.so network.vala
	PKG_CONFIG_PATH=$(PKG_CONFIG_PATH):../data valac -X -L. -X -I. -X -Iisi --vapidir=../data --pkg libisi -g network.vala

authenticate: isi libisi.so authenticate.vala
	PKG_CONFIG_PATH=$(PKG_CONFIG_PATH):../data valac -X -L. -X -I. -X -Iisi --vapidir=../data --pkg libisi -g authenticate.vala

clean:
	rm -rf network authenticate
	rm -rf isi libisi.so

.PHONY: all clean
