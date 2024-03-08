
QT += openglextensions

HEADERS += \
        $$PWD/include/ccBilateralFilter.h \
        $$PWD/include/ccEDLFilter.h \
        $$PWD/include/ccFrameBufferObject.h \
        $$PWD/include/ccGlFilter.h \
        $$PWD/include/ccShader.h

SOURCES += \
    $$PWD/src/ccBilateralFilter.cpp \
    $$PWD/src/ccEDLFilter.cpp \
    $$PWD/src/ccFrameBufferObject.cpp \
    $$PWD/src/ccShader.cpp

INCLUDEPATH += $$PWD/include

DISTFILES += \
    $$PWD/shaders/Bilateral/bilateral.frag \
    $$PWD/shaders/Bilateral/bilateral.vert \
    $$PWD/shaders/EDL/edl_mix.frag \
    $$PWD/shaders/EDL/edl_mix.vert \
    $$PWD/shaders/EDL/edl_shade.frag \
    $$PWD/shaders/EDL/edl_shade.vert

RESOURCES += \
    $$PWD/shader.qrc

