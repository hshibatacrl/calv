#!/bin/bash

common_path=$(pwd)/../common

install_path=$(pwd)/mingw73_64

source $common_path/mingw_build_env
echo $install_path

#cleanup first
rm -rf $install_path

mkdir -p ../../../temp
cd ../../../temp

rm -rf qcustomplot

#
# download files
#

wget https://www.qcustomplot.com/release/2.1.1/QCustomPlot.tar.gz
tar xvf QCustomPlot.tar.gz
cd qcustomplot
wget https://www.qcustomplot.com/release/2.1.1/QCustomPlot-sharedlib.tar.gz
tar xvf QCustomPlot-sharedlib.tar.gz
cd QCustomPlot-sharedlib
cd sharedlib-compilation

#
# build
#

qmake
make

#
# install
#

mkdir -p $install_path/include
mkdir -p $install_path/lib
mkdir -p $install_path/bin
cp debug/libqcustomplotd2.a $install_path/lib/libqcustomplotd2.a
cp debug/qcustomplotd2.dll $install_path/bin/qcustomplotd2.dll
cp release/libqcustomplot2.a $install_path/lib/libqcustomplot2.a
cp release/qcustomplot2.dll $install_path/bin/qcustomplot2.dll
cp ../../qcustomplot.h $install_path/include/qcustomplot.h

#
# done
#

