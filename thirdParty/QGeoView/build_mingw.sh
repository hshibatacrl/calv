#!/bin/bash

common_path=$(pwd)/../common

install_path=$(pwd)/mingw73_64

source $common_path/mingw_build_env
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
cmake -G "MinGW Makefiles" -S .. -B . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX:PATH=$install_path
cmake --build . -j 8
cmake --install .
#
# move library and dll to debug folder
#
mkdir -p $install_path/lib/debug
mkdir -p $install_path/bin/debug
cp $install_path/lib/static/libqgeoview.dll.a $install_path/lib/debug/
cp $install_path/bin/libqgeoview.dll $install_path/bin/debug/

#
# then release build
#

# clenaup
cd ..
rm -rf build
mkdir -p build
cd build

cmake -G "MinGW Makefiles" -S .. -B . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=$install_path
cmake --build . -j 8
cmake --install .
#
# move library and dll to release folder
#
mkdir -p $install_path/lib/release
mkdir -p $install_path/bin/release
cp $install_path/lib/static/libqgeoview.dll.a $install_path/lib/release/
cp $install_path/bin/libqgeoview.dll $install_path/bin/release/

#
# cleanup residue
#
rm -rf $install_path/lib/static
rm -f $install_path/bin/libqgeoview.dll


# move up to temp folder
cd ..
cd ..


#
# clone openssl
#
rm -rf openssl
git clone git://git.openssl.org/openssl.git -b OpenSSL_1_1_1s
cd openssl
./configure --prefix=$install_path mingw64
make -j8
make install_sw

#
# done
#

