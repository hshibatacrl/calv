SOURCES += \
    $$PWD/QGVLayerGSI.cpp \
    $$PWD/customMapView.cpp \
    $$PWD/groupLayer.cpp \
    $$PWD/trackItem.cpp \
    $$PWD/trackOptionsDialog.cpp

HEADERS += \
    $$PWD/QGVLayerGSI.h \
    $$PWD/customMapView.h \
    $$PWD/groupLayer.h \
    $$PWD/trackItem.h \
    $$PWD/trackOptionsDialog.h

FORMS += \
    $$PWD/trackOptionsDialog.ui

INCLUDEPATH += $$PWD

contains( DEFINES, USE_QGEOVIEW ): include(../thirdParty/QGeoView/QGeoView.pri)
