#-------------------------------------------------
#
# Project created by QtCreator 2018-01-25T13:36:37
#
#-------------------------------------------------

QT       += opengl

TARGET = VVGL
TEMPLATE = lib

# this along with vvgl_qt_global.h are used to configured the publicly-exported symbols
DEFINES += VVGL_LIBRARY

CONFIG += c++14
CONFIG += shared

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS


# these libs require an ISF_SDK define
DEFINES += ISF_SDK_QT




# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0




# additions for GLEW
unix: LIBS += -L/usr/local/lib/ -lGLEW
INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include
unix: PRE_TARGETDEPS += /usr/local/lib/libGLEW.a




# additions for the VVGL header files
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../VVGL/include




# actual source
SOURCES += \
    ../../../VVGL/src/VVGeom.cpp \
    ../../../VVGL/src/VVGLBuffer.cpp \
    ../../../VVGL/src/VVGLBufferCopier.cpp \
    ../../../VVGL/src/VVGLBufferPool.cpp \
    ../../../VVGL/src/VVGLCachedAttrib.cpp \
    ../../../VVGL/src/VVGLCachedUni.cpp \
    ../../../VVGL/src/VVGLContext.cpp \
    ../../../VVGL/src/VVGLQtCtxWrapper.cpp \
    ../../../VVGL/src/VVGLScene.cpp \
    ../../../VVGL/src/VVStringUtils.cpp

HEADERS += \
    ../../../VVGL/include/VVBase.hpp \
    ../../../VVGL/include/VVGeom.hpp \
    ../../../VVGL/include/VVGL.hpp \
	../../../VVGL/include/VVGL_Defines.hpp \
    ../../../VVGL/include/vvgl_qt_global.h \
    ../../../VVGL/include/VVGLBuffer.hpp \
    ../../../VVGL/include/VVGLBuffer_GLFW_Enums.h \
    ../../../VVGL/include/VVGLBuffer_IOS_Enums.h \
    ../../../VVGL/include/VVGLBuffer_Mac_Enums.h \
    ../../../VVGL/include/VVGLBuffer_Qt_Enums.h \
    ../../../VVGL/include/VVGLBuffer_RPI_Enums.h \
    ../../../VVGL/include/VVGLBufferCopier.hpp \
    ../../../VVGL/include/VVGLBufferPool.hpp \
    ../../../VVGL/include/VVGLBufferPool_CocoaAdditions.h \
    ../../../VVGL/include/VVGLCachedAttrib.hpp \
    ../../../VVGL/include/VVGLCachedUni.hpp \
    ../../../VVGL/include/VVGLContext.hpp \
    ../../../VVGL/include/VVGLQtCtxWrapper.hpp \
    ../../../VVGL/include/VVGLScene.hpp \
    ../../../VVGL/include/VVRange.hpp \
    ../../../VVGL/include/VVStringUtils.hpp \
    ../../../VVGL/include/VVTime.hpp


#unix {
	#target.path = /usr/local/lib
	#INSTALLS += target
	#header_files.path = /usr/local/include/VVGL/
	#header_files.files = $$_PRO_FILE_PWD_/../../../VVGL/include/*.h
	#header_files.files += $$_PRO_FILE_PWD_/../../../VVGL/include/*.hpp
	#INSTALLS += header_files
#}


mac	{
	# This adds the @rpath prefix to the lib install name (the lib is expected to be bundled with the app package)
	QMAKE_SONAME_PREFIX = @rpath
}

