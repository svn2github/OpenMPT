#!/usr/bin/env bash

set -e


MY_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo $MY_DIR

cd "${MY_DIR}"
cd ..


function unpack () {
 set -e
 MPT_GET_DESTDIR="$1"
 MPT_GET_FILE="$2"
 MPT_GET_SUBDIR="$3"
 cd include
  if [ -d "$1" ]; then
   rm -rf "$1"
  fi
  if [ "$3" = "." ]; then
   mkdir "$1"
   cd "$1"
    unzip "../../$2"
   cd ..
  else
   unzip "../$2"
   if [ ! "$3" = "$1" ]; then
    mv "$3" "$1"
   fi
  fi
 cd ..
 return 0
}



unpack "genie" "build/externals/GENie-ec0a4a89d8dad4d251fc7195784a275c0c322a4d.zip" "GENie-ec0a4a89d8dad4d251fc7195784a275c0c322a4d"

cd include/genie

make

mkdir -p build/vs2015
./bin/linux/genie --to=../build/vs2015 vs2015

mkdir -p build/vs2017
./bin/linux/genie --to=../build/vs2017 vs2015

cd ../..

cp -ar include/genie/build/vs* build/genie/genie/build/

echo "ec0a4a89d8dad4d251fc7195784a275c0c322a4d" > include/genie/OpenMPT-version.txt



unpack "premake" "build/externals/premake-5.0.0-alpha13-src.zip" "premake-5.0.0-alpha13"

cd include/premake

#make -f Bootstrap.mak linux
##bin/release/premake5 test
#bin/release/premake5 embed --bytecode
#bin/release/premake5 --to=build/gmake.unix gmake --no-curl --no-zlib --no-luasocket
cd build/gmake.unix
make
cd ../..
#bin/release/premake5 test --no-curl --no-zlib --no-luasocket

cd ../..

echo "5.0.0-alpha13" > include/premake/OpenMPT-version.txt

