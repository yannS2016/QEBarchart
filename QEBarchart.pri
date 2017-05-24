# QEBarchart.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEBarchart/QEBarchart.png \
    widgets/QEBarchart/QEScalarBarchart.png \

 
RESOURCES += \
    widgets/QEBarchart/QEBarchart.qrc

HEADERS += \
    widgets/QEBarchart/QEBarchart.h \
    widgets/QEBarchart/QEBarchartManager.h \
    widgets/QEBarchart/QEScalarBarchart.h \
    widgets/QEBarchart/QEScalarBarchartManager.h \



SOURCES += \
    widgets/QEBarchart/QEBarchart.cpp \
    widgets/QEBarchart/QEScalarBarchart.cpp \
    widgets/QEBarchart/QEBarchartManager.cpp \
    widgets/QEBarchart/QEScalarBarchartManager.cpp \

INCLUDEPATH += \
    widgets/QEBarchart

# end
