

HEADERS += \
    $$PWD/customPlotView.h

SOURCES += \
    $$PWD/customPlotView.cpp

contains(DEFINES,USE_QCUSTOMPLOT)
{
message( "QCustomPlot is enabled" )

HEADERS += \
    $$PWD/qcpPlotView.h

SOURCES += \
    $$PWD/qcpPlotView.cpp

include(../thirdParty/QCustomPlot/QCustomPlot.pri)
}



INCLUDEPATH += $$PWD
