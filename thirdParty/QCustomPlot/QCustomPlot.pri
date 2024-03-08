QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++17
lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++17

DEFINES += QCUSTOMPLOT_USE_LIBRARY

linux{
QCUSTOMPLOT=$$PWD/linux

INCLUDEPATH += \
    $${QCUSTOMPLOT}/include/


CONFIG(debug, debug|release){
LIBS += \
    $${QCUSTOMPLOT}/lib/libqcustomplotd.so
#QMAKE_POST_LINK += ln -sf $${QCUSTOMPLOT}/lib/libqcustomplotd.so.2 $${OUT_PWD}
}else{
LIBS += \
    $${QCUSTOMPLOT}/lib/libqcustomplot.so
#QMAKE_POST_LINK += ln -sf $${QCUSTOMPLOT}/lib/libqcustomplot.so.2 $${OUT_PWD}
}
LIBS += -L$${QCUSTOMPLOT}/lib
}

win32-g++{

QCUSTOMPLOT=$$PWD/mingw73_64

INCLUDEPATH += \
    $${QCUSTOMPLOT}/include/


CONFIG(debug, debug|release){
LIBS += \
    $${QCUSTOMPLOT}/lib/libqcustomplotd2.a
SRCDLL = $${QCUSTOMPLOT}/bin/qcustomplotd2.dll
}else{
LIBS += \
    $${QCUSTOMPLOT}/lib/libqcustomplot2.a
SRCDLL = $${QCUSTOMPLOT}/bin/qcustomplot2.dll
}


CONFIG(debug, debug|release){
DSTDLL=$${OUT_PWD}/debug/
}else{
DSTDLL=$${OUT_PWD}/release/
}

DSTDLL ~= s,/,\\,g
SRCDLL ~= s,/,\\,g

QMAKE_POST_LINK +=$$quote(cmd /c xcopy /Y /D $${SRCDLL} $${DSTDLL}$${BUILD_TYPE}) &

message($$QMAKE_POST_LINK)
}
