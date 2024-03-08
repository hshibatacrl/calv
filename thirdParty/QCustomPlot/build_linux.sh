#!/bin/bash

common_path=$(pwd)/../common

install_path=$(pwd)/linux

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
cd qcustomplot-sharedlib
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
cp -d libqcustomplotd.* $install_path/lib/
cp -d libqcustomplot.* $install_path/lib/
cp ../../qcustomplot.h $install_path/include/qcustomplot.h

#
# done
#

