QT += quick
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Product stamp from CI / qmake PRODUCT_VERSION=YYYY-MM-DD (same as Writerdeck-server).
isEmpty(PRODUCT_VERSION): PRODUCT_VERSION = unknown
DEFINES += "PRODUCT_VERSION=\"$$PRODUCT_VERSION\""

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp sundown/src/autolink.c sundown/src/buffer.c sundown/src/markdown.c sundown/src/stack.c sundown/html/houdini_href_e.c sundown/html/houdini_html_e.c sundown/html/html.c sundown/html/html_smartypants.c

INCLUDEPATH += sundown/src
INCLUDEPATH += sundown/html

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# toltec Qt device spec (Writerdeck CI). Upstream used linux-oe-g++.
linux-arm-remarkable-g++ {
    LIBS += -lqsgepaper
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    index.txt

HEADERS += \
    edit_utils.h \
    rotation_watcher.h \
    lobby_bridge.h \
    lobby_ui_config.h \
    edit_helper.h

SOURCES += rotation_watcher.cpp lobby_bridge.cpp lobby_ui_config.cpp edit_helper.cpp

# Socket reader thread (main.cpp) needs pthread.
QMAKE_CXXFLAGS += -pthread
QMAKE_LFLAGS += -pthread
