# Copyright 2021 Wagon Wheel Robotics
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

QT += opengl

HEADERS += \
    $$PWD/customGLWidget.h \
    $$PWD/entitiesTree.h \
    $$PWD/gl_3axis_entity.h \
    $$PWD/gl_draw_params.h \
    $$PWD/gl_entity_ctx.h \
    $$PWD/gl_model_entity.h \
    $$PWD/gl_pcloud_entity.h \
    $$PWD/gl_polyline_entity.h \
    $$PWD/gl_poses_entity.h \
    $$PWD/gl_stock_entity.h \
    $$PWD/model.h \
    $$PWD/pointcloud_packet.h \
    $$PWD/qt_opengl_unproj.h \
    $$PWD/rot.h \
    $$PWD/viewOptionsDialog.h

SOURCES += \
    $$PWD/customGLWidget.cpp \
    $$PWD/entitiesTree.cpp \
    $$PWD/gl_3axis_entity.cpp \
    $$PWD/gl_entity_ctx.cpp \
    $$PWD/gl_model_entity.cpp \
    $$PWD/gl_pcloud_entity.cpp \
    $$PWD/gl_polyline_entity.cpp \
    $$PWD/gl_poses_entity.cpp \
    $$PWD/gl_stock_entity.cpp \
    $$PWD/model.cpp \
    $$PWD/mqo.cpp \
    $$PWD/obj.cpp \
    $$PWD/qt_opengl_unproj.cpp \
    $$PWD/rot.cpp \
    $$PWD/viewOptionsDialog.cpp

LIBS += -lstdc++fs

INCLUDEPATH += $$PWD

DEFINES += "MAX_VBO_SIZE=0x000000017fffffff"        #32bit maximum
#DEFINES +=  "MAX_VBO_SIZE=0x000000003ffffff"        #reduced

FORMS += \
    $$PWD/viewOptionsDialog.ui

DISTFILES += \
    $$PWD/gl_pcloud_entity_noAA.frag

RESOURCES += \
    $$PWD/shaders.qrc

contains( DEFINES, USE_EDL ): include(../thirdParty/fbo/fbo.pri)
