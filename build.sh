#!/bin/bash

sudo apt-get install -y libqt5websockets5-dev
mkdir build
cd build
qmake ..
make
