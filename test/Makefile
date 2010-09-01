build: authenticate network

network: network.vala
	valac --vapidir=../data --pkg libisi -g network.vala

authenticate: authenticate.vala
	valac --vapidir=../data --pkg libisi -g authenticate.vala

clean:
	rm -rf network authenticate

.PHONY: build clean
