QT += gui

CONFIG += c++14 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS




# these defines are specific to these libs
DEFINES += ISF_TARGET_QT
#DEFINES += ISF_TARGET_GL3PLUS




# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    VVGLTestApp.cpp \
	../VVBufferGLWindow.cpp

HEADERS += \
	../VVBufferGLWindow.h




# additions for VVGL lib
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../VVGL/release/ -lVVGL
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../VVGL/debug/ -lVVGL
else:unix: LIBS += -L$$OUT_PWD/../VVGL/ -lVVGL

INCLUDEPATH += $$_PRO_FILE_PWD_/../../../VVGL/include
INCLUDEPATH += $$_PRO_FILE_PWD_/../
DEPENDPATH += $$PWD/../VVGL




# additions for GLEW
unix: LIBS += -L/usr/local/lib/ -lGLEW

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include
unix: PRE_TARGETDEPS += /usr/local/lib/libGLEW.a




RESOURCES += \
    vvgltestappresources.qrc



