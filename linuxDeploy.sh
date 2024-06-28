#!/bin/bash

OUTPUT=../calv_executable

CALV=../build-calv-Desktop-Release/calv

rm -rf $OUTPUT
mkdir -p $OUTPUT
mkdir -p $OUTPUT/temp

cp -a default.png $OUTPUT/temp
cp -a $CALV $OUTPUT/temp

pushd $OUTPUT
wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
chmod a+x appimagetool-x86_64.AppImage

wget "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
LINUXDEPLOYQT=`pwd`/linuxdeployqt-continuous-x86_64.AppImage
echo $LINUXDEPLOYQT
popd

pushd $OUTPUT/temp
echo "[Desktop Entry]" >> default.desktop
echo "Type=Application" >> default.desktop
echo "Name=calv" >> default.desktop
echo "Exec=AppRun %F" >> default.desktop
echo "Icon=default" >> default.desktop
echo "Comment=Feature Based Calibration Viewer" >> default.desktop
echo "Terminal=true" >> default.desktop
echo "Categories=Utility;" >> default.desktop
$LINUXDEPLOYQT calv -bundle-non-qt-libs -verbose=2
popd

pushd $OUTPUT
./appimagetool-x86_64.AppImage  ./temp calv.AppImage
popd




