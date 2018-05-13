#!/usr/bin/env bash

set -e


MY_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo $MY_DIR

cd "${MY_DIR}"
cd ..


function download_and_unpack () {
 set -e
 MPT_GET_DESTDIR="$1"
 MPT_GET_URL="$2"
 MPT_GET_FILE="$3"
 MPT_GET_SUBDIR="$4"
 if [ ! -f "build/externals/$3" ]; then
  wget "$2" -O "build/externals/$3"
 fi
 cd include
  if [ -d "$1" ]; then
   rm -rf "$1"
  fi
  if [ "$4" = "." ]; then
   mkdir "$1"
   cd "$1"
    unzip -x "../../build/externals/$3"
   cd ..
  else
   unzip -x "../build/externals/$3"
   if [ ! "$4" = "$1" ]; then
    mv "$4" "$1"
   fi
  fi
 cd ..
 return 0
}

if [ ! -d "build/externals" ]; then
 mkdir build/externals
fi



download_and_unpack "genie" "https://github.com/bkaradzic/GENie/archive/78817a9707c1a02e845fb38b3adcc5353b02d377.zip" "GENie-78817a9707c1a02e845fb38b3adcc5353b02d377.zip" "GENie-78817a9707c1a02e845fb38b3adcc5353b02d377"

cd include/genie

make

mkdir -p build/vs2015
./bin/linux/genie --to=../build/vs2015 vs2015

mkdir -p build/vs2017
./bin/linux/genie --to=../build/vs2017 vs2015

cd ../..

cp -ar include/genie/build/vs* build/genie/genie/build/



download_and_unpack "premake" "https://github.com/premake/premake-core/archive/2e7ca5fb18acdbcd5755fb741710622b20f2e0f6.zip" "premake-core-2e7ca5fb18acdbcd5755fb741710622b20f2e0f6.zip" "premake-core-2e7ca5fb18acdbcd5755fb741710622b20f2e0f6"

cd include/premake

make -f Bootstrap.mak linux
bin/release/premake5 embed --bytecode
bin/release/premake5 --to=build/gmake.unix gmake --no-curl --no-zlib --no-luasocket
cd build/gmake.unix
make
cd ../..
bin/release/premake5 test
