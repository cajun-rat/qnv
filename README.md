# QNV - Simple Engineer's logbook

## Build instructions

Install prerequisites:

	sudo apt-get install cmake  build-essential qtbase5-dev exuberant-ctags libboost-filesystem-dev libboost-system-dev libboost-random-dev 

Configure with cmake:

	mkdir build-rel
	cd build-rel
	cmake -DCMAKE_BUILD_TYPE=Release ..

Build:

	make

Run:
	
	./qnv

