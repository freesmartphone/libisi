all:
	cd src ; make all

install:
	cd src ; make install
	cd data ; make install

uninstall:
	cd src ; make uninstall
	cd data ; make uninstall

clean:
	cd src ; make clean
	cd test ; make clean

test: all
	cd test ; make all

doc:
	rm -rf html
	valadoc data/*.vapi -o html

wireshark-plugin:
	cd wireshark-plugin ; make all
