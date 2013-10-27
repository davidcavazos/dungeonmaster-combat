#!/bin/bash

cd ..
PARENT=$(pwd)
SRC_DIR=$PARENT/dungeonmaster-combat
BUILD_DIR=$SRC_DIR/build

if [ ! -d $BUILD_DIR ]; then
  mkdir $BUILD_DIR
fi
cd $BUILD_DIR
cmake $SRC_DIR
make -j$(grep -c ^processor /proc/cpuinfo)
