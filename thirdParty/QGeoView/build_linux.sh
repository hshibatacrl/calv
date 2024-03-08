#!/bin/bash

common_path=$(pwd)/../common

install_path=$(pwd)/linux

source $common_path/linux_build_env
echo $install_path

#cleanup first
rm -rf $install_path


mkdir -p ../../../temp
cd ../../../temp

#
# clone QGeoView
#
rm -rf QGeoView
git clone https://github.com/AmonRaNet/QGeoView.git -b 1.0.4
cd QGeoView

#
# debug build first
#

# clenaup
rm -rf build
mkdir -p build
cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX:PATH=$install_path
cmake --build . -j 8
cmake --install .
#
# move library and dll to debug folder
#
mkdir -p $install_path/lib/debug
cp $install_path/lib/libqgeoview.so $install_path/lib/debug/
rm -f $install_path/lib/libqgeoview.so

#
# then release build
#

# clenaup
cd ..
rm -rf build
mkdir -p build
cd build

cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=$install_path
cmake --build . -j 8
cmake --install .
#
# move library and dll to release folder
#
mkdir -p $install_path/lib/release
cp $install_path/lib/libqgeoview.so $install_path/lib/release/

#
# cleanup residue
#
rm -f $install_path/lib/libqgeoview.so


# move up to temp folder
cd ..
cd ..


#
# clone openssl
#
rm -rf openssl
git clone git://git.openssl.org/openssl.git -b OpenSSL_1_1_1s
cd openssl
./Configure --prefix=$install_path linux-x86_64
make -j8
make install_sw

#
# done
#

