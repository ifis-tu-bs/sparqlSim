#!/bin/sh

#  build_all.sh
#  
#
#  Created by Anatoliy Kuznetsov on 9/2/17.
#

make clean

rm -rf ./stress_* ./test
make DEBUG=YES rebuild
mv ./test ./stress_debug

make rebuild
mv ./test ./stress_release

make BMOPTFLAGS=-DBMSSE2OPT rebuild
mv ./test ./stress_release_sse2

make BMOPTFLAGS=-DBMSSE42OPT rebuild
mv ./test ./stress_release_sse42

make BMOPTFLAGS=-DBMAVX2OPT rebuild
mv ./test ./stress_release_avx2

make BMOPTFLAGS=-DBM64OPT rebuild
mv ./test ./stress_release_64
