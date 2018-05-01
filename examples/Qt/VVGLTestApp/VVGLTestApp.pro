QT += gui
QT += opengl

TARGET = VVGLTestApp
TEMPLATE = app

CONFIG += c++14 console
#CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS




# these libs require an ISF_SDK define
DEFINES += ISF_SDK_QT




# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    VVGLTestApp.cpp \
	../GLBufferQWindow.cpp

HEADERS += \
	../GLBufferQWindow.h




# additions for VVGL lib
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../VVGL/release/ -lVVGL
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../VVGL/debug/ -lVVGL
else:unix: LIBS += -L$$OUT_PWD/../VVGL/ -lVVGL

INCLUDEPATH += $$_PRO_FILE_PWD_/../../../VVGL/include
INCLUDEPATH += $$_PRO_FILE_PWD_/../
#DEPENDPATH += $$PWD/../VVGL




# make sure the rpath includes both ways of getting libs
QMAKE_RPATHDIR = @executable_path/../Frameworks
QMAKE_RPATHDIR += @loader_path/../Frameworks




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




RESOURCES += \
    vvgltestappresources.qrc




# macs need some assembly for deployment
mac	{
	CONFIG(debug, debug|release)	{
		# intentionally blank, debug builds don't need any work (build & run works just fine)
	}
	# release builds need to have the libs bundled up and macdeployqt executed on the output app package to relink them
	else	{
		framework_dir = $$OUT_PWD/$$TARGET\.app/Contents/Frameworks
		QMAKE_POST_LINK += echo "****************************";
		QMAKE_POST_LINK += mkdir -pv $$framework_dir;
		QMAKE_POST_LINK += cp -vaRf $$_PRO_FILE_PWD_/../../../external/GLEW/mac_x86_64/libGLEW*.dylib $$framework_dir;
		QMAKE_POST_LINK += cp -vaRf $$OUT_PWD/../VVGL/libVVGL*.dylib $$framework_dir;
		QMAKE_POST_LINK += macdeployqt $$OUT_PWD/$$TARGET\.app;
	}
}


