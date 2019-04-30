#!/bin/bash
cd build-lin
cmake .. -DCMAKE_CXX_COMPILER=$1 -DBUILD_RELEASE=$2
make -j$3
