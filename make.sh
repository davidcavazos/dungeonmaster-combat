#!/bin/bash

cd ..
PARENT=$(pwd)
SRC_DIR=$PARENT/dungeonmaster
BUILD_DIR=$SRC_DIR/build

if [ ! -d $BUILD_DIR ]; then
  mkdir $BUILD_DIR
fi
cd $BUILD_DIR
cmake $SRC_DIR
make -j$(lscpu | grep "^CPU(s):" | grep -o [0-9]* || 1)
