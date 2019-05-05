#!/bin/bash
cd build-lin
cmake .. -DCMAKE_C_COMPILER=$1 -DBUILD_RELEASE=$2
make -j$3
