#-------------------------------------------------
#
# Project created by QtCreator 2018-01-25T13:36:37
#
#-------------------------------------------------

QT       += opengl multimedia

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
DEFINES += VVGL_SDK_QT




# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0




# additions for GLEW
#unix: LIBS += -L/usr/local/lib/ -lGLEW
#INCLUDEPATH += /usr/local/include
#DEPENDPATH += /usr/local/include
#unix: PRE_TARGETDEPS += /usr/local/lib/libGLEW.a
unix: LIBS += -L$$_PRO_FILE_PWD_/../../../external/GLEW/mac_x86_64/ -lGLEW
win32: LIBS += -L$$_PRO_FILE_PWD_/../../../external/GLEW/win_x64/ -lglew32 -lopengl32
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../external/GLEW/include
DEPENDPATH += $$_PRO_FILE_PWD_/../../../external/GLEW/include
unix: PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../../../external/GLEW/mac_x86_64/libGLEW.dylib
win32: PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../../../external/GLEW/win_x64/glew32.dll


# additions for the VVGL header files
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../VVGL/include




# actual source
SOURCES += \
	../../../VVGL/src/GLBuffer.cpp \
	../../../VVGL/src/GLBufferPool.cpp \
	../../../VVGL/src/GLCachedProperty.cpp \
	../../../VVGL/src/GLContext.cpp \
	../../../VVGL/src/GLCPUToTexCopier.cpp \
	../../../VVGL/src/GLQtCtxWrapper.cpp \
	../../../VVGL/src/GLScene.cpp \
	../../../VVGL/src/GLTexToCPUCopier.cpp \
	../../../VVGL/src/GLTexToTexCopier.cpp \
	../../../VVGL/src/VVGL_Geom.cpp \
	../../../VVGL/src/VVGL_StringUtils.cpp

HEADERS += \
	../../../VVGL/include/GLBuffer_Enums_GLFW.h \
	../../../VVGL/include/GLBuffer_Enums_IOS.h \
	../../../VVGL/include/GLBuffer_Enums_Mac.h \
	../../../VVGL/include/GLBuffer_Enums_Qt.h \
	../../../VVGL/include/GLBuffer_Enums_RPI.h \
	../../../VVGL/include/GLBuffer.hpp \
	../../../VVGL/include/GLBufferPool_CocoaAdditions.h \
	../../../VVGL/include/GLBufferPool.hpp \
	../../../VVGL/include/GLCachedProperty.hpp \
	../../../VVGL/include/GLContext.hpp \
	../../../VVGL/include/GLCPUToTexCopier.hpp \
	../../../VVGL/include/GLQtCtxWrapper.hpp \
	../../../VVGL/include/GLScene.hpp \
	../../../VVGL/include/GLTexToCPUCopier.hpp \
	../../../VVGL/include/GLTexToTexCopier.hpp \
	../../../VVGL/include/VVGL_Base.hpp \
	../../../VVGL/include/VVGL_Defines.hpp \
	../../../VVGL/include/VVGL_Doxygen.hpp \
	../../../VVGL/include/VVGL_Geom.hpp \
	../../../VVGL/include/VVGL_HardCodedDefines.hpp \
	../../../VVGL/include/VVGL_Qt_global.h \
	../../../VVGL/include/VVGL_Range.hpp \
	../../../VVGL/include/VVGL_StringUtils.hpp \
	../../../VVGL/include/VVGL_Time.hpp \
	../../../VVGL/include/VVGL.hpp


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


CONFIG(debug, debug|release)	{
	#	intentionally blank
}
else	{
	mac	{
	}
	win32	{
		QMAKE_LFLAGS_RELEASE += /DEBUG
		QMAKE_CXXFLAGS_RELEASE += /Zi
		QMAKE_LFLAGS_RELEASE += /OPT:REF
	}
}


