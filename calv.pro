QT += core gui gui-private
QT += network
#QT += serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

#DEFINES += USE_MAP_VIEW
DEFINES += USE_3D_VIEW      # if you remove 3D view, you must edit mainwindow.ui to remove entity tree
#DEFINES += USE_PLOT_VIEW
#DEFINES += USE_IMAGE_VIEW


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# EDL is GPL (Cloud compare)
DEFINES += USE_EDL

include(utils/utils.pri)
include(glView/glView.pri)
include(crl/crl.pri)

SOURCES += \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    MainWindow.h

FORMS += \
    MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

VERSION = 1.0a

DEFINES += APP_NAME=\\\"$$TARGET\\\"
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += APP_VERSION_QT=\\\"$$QT_VERSION\\\"
DEFINES += "COMPANY_NAME=\"\\\"Carnegie Robotics\\\"\""
DEFINES += "COPYRIGHT_YEAR=\"\\\"2023\\\"\""

win32:RC_ICONS += crl32.ico
