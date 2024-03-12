HEADERS += \
    $$PWD/../../featureDetector/client/fdc.h \
#    $$PWD/../../featureDetector/fdv/cameraConnectDialog.h \
    $$PWD/../../featureDetector/fdv/fdd.h \
    $$PWD/../../featureDetector/src/orb_packet_type.h \
    $$PWD/../../oncal/src/pointcloud_packet.h \
    $$PWD/../../oncal/src/pose_packet.h

SOURCES += \
    $$PWD/../../featureDetector/client/fdc.cc \
#    $$PWD/../../featureDetector/fdv/cameraConnectDialog.cpp \
    $$PWD/../../featureDetector/fdv/fdd.cpp

#FORMS += \
#    $$PWD/../../featureDetector/fdv/cameraConnectDialog.ui

INCLUDEPATH += $$PWD/../../oncal/src \
               $$PWD/../../featureDetector/src \
               $$PWD/../../featureDetector/client \
               $$PWD/../../featureDetector/fdv
DEFINES += NO_PLAYBACK_DIALOG
