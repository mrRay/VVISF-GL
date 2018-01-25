#-------------------------------------------------
#
# Project created by QtCreator 2018-01-30T11:11:51
#
#-------------------------------------------------

QT       += opengl

TARGET = VVISF
TEMPLATE = lib

DEFINES += VVISF_LIBRARY

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
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




# additions for GLEW
unix: LIBS += -L/usr/local/lib/ -lGLEW

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include
unix: PRE_TARGETDEPS += /usr/local/lib/libGLEW.a




# this code was added when i added the VVGL library
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../VVGL/release/ -lVVGL
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../VVGL/debug/ -lVVGL
else:unix: LIBS += -L$$OUT_PWD/../VVGL/ -lVVGL

#INCLUDEPATH += $$PWD/../VVGL
DEPENDPATH += $$PWD/../VVGL




# additions for the VVGL header files
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../VVGL/include
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../VVISF/include




SOURCES += \
    ../../../VVISF/src/ISFDoc.cpp \
    ../../../VVISF/src/ISFStringUtils.cpp \
    ../../../VVISF/src/ISFScene.cpp \
    ../../../VVISF/src/ISFPassTarget.cpp \
    ../../../VVISF/src/ISFBase.cpp \
    ../../../VVISF/src/ISFAttr.cpp \
    ../../../VVISF/src/ISFVal.cpp

HEADERS += \
	../../../VVISF/include/vvisf_qt_global.h \
    ../../../VVISF/include/ISFVal.hpp \
    ../../../VVISF/include/ISFStringUtils.hpp \
    ../../../VVISF/include/ISFScene.hpp \
    ../../../VVISF/include/ISFDoc.hpp \
    ../../../VVISF/include/ISFConstants.hpp \
    ../../../VVISF/include/ISFBase.hpp \
    ../../../VVISF/include/ISFAttr.hpp \
    ../../../VVISF/include/ISFPassTarget.hpp \
    ../../../VVISF/include/ISFErr.hpp \
    ../../../VVISF/include/ISFKit.h

unix {
	target.path = /usr/lib
	INSTALLS += target
}



