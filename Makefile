all:
	cd src ; make all

install:
	cd src ; make install

clean:
	cd src ; make clean
	#cd test ; make clean

#test:
#	cd test ; make

doc:
	rm -rf html
	valadoc vapi/*.vapi -o html
