QT += gui widgets network

linux{
OPENSSL=$$PWD/linux
QGEOVIEW=$$PWD/linux

INCLUDEPATH += \
    $${QGEOVIEW}/include/ \
    $${QGEOVIEW}/include/QGeoView/

CONFIG(debug, debug|release){
LIBS += \
    $${QGEOVIEW}/lib/debug/libqgeoview.so
LIBS += -L$${QGEOVIEW}/lib/debug
#QMAKE_POST_LINK += ln -sf $${QGEOVIEW}/lib/debug/libqgeoview.so $${OUT_PWD}
}else{
LIBS += \
    $${QGEOVIEW}/lib/release/libqgeoview.so
LIBS += -L$${QGEOVIEW}/lib/release
}

#for openssl
LIBS += -L$${QGEOVIEW}/lib \
    $${QGEOVIEW}/lib/libcrypto.so \
    $${QGEOVIEW}/lib/libssl.so

}

win32-g++{

OPENSSL=$$PWD/mingw73_64
QGEOVIEW=$$PWD/mingw73_64

INCLUDEPATH += \
    $${QGEOVIEW}/include/ \
    $${QGEOVIEW}/include/QGeoView/


CONFIG(debug, debug|release){
LIBS += \
    $${QGEOVIEW}/lib/debug/libqgeoview.dll.a
SRCDLL1 = $${QGEOVIEW}/bin/debug/libqgeoview.dll

}else{
LIBS += \
    $${QGEOVIEW}/lib/release/libqgeoview.dll.a
SRCDLL1 = $${QGEOVIEW}/bin/release/libqgeoview.dll
}

SRCDLL2 += $${OPENSSL}/bin/libcrypto-1_1-x64.dll
SRCDLL3 += $${OPENSSL}/bin/libssl-1_1-x64.dll

CONFIG(debug, debug|release){
DSTDLL=$${OUT_PWD}/debug/
}else{
DSTDLL=$${OUT_PWD}/release/
}

DSTDLL ~= s,/,\\,g
SRCDLL1 ~= s,/,\\,g
SRCDLL2 ~= s,/,\\,g
SRCDLL3 ~= s,/,\\,g

QMAKE_POST_LINK +=$$quote(cmd /c xcopy /Y /D $${SRCDLL1} $${DSTDLL}$${BUILD_TYPE}) &
QMAKE_POST_LINK +=$$quote(cmd /c xcopy /Y /D $${SRCDLL2} $${DSTDLL}$${BUILD_TYPE}) &
QMAKE_POST_LINK +=$$quote(cmd /c xcopy /Y /D $${SRCDLL3} $${DSTDLL}$${BUILD_TYPE}) &

message($$QMAKE_POST_LINK)
}
