# QNV - Simple Engineer's logbook

## Build instructions

Install prerequisites:

	sudo apt install cmake ninja-build build-essential qtbase5-dev exuberant-ctags libboost-system-dev libboost-random-dev

Configure with cmake:

	mkdir build-rel
	cd build-rel
	cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..

Build and run locally

	ninja
	src/qnv

Create a Debian Package

    cpack

Install it

    sudo apt install ./qnv-1.1-Linux.deb

qnv notes are stored in `.qnv` in your home directory.
Checking this directory into git is a good way to share it between computers.
