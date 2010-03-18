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

test: install
	cd test ; make

doc:
	rm -rf html
	valadoc data/*.vapi -o html
