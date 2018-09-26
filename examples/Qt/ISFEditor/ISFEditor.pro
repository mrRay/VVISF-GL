#-------------------------------------------------
#
# Project created by QtCreator 2018-09-11T16:33:13
#
#-------------------------------------------------

QT       += core gui widgets opengl

TARGET = ISFEditor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14 console




# these libs require an ISF_SDK define
DEFINES += VVGL_SDK_QT




SOURCES += \
        main.cpp \
        MainWindow.cpp \
    LoadingWindow.cpp \
    OutputWindow.cpp \
	../common/GLBufferQWidget.cpp \
    DocWindow.cpp \
    SimpleSourceCodeEdit.cpp \
    ISFController.cpp \
    ISFUIItem.cpp \
    ISFRemoteImageClient.cpp \
    JSONScrollWidget.cpp \
    JSONGUI/JSONGUIBasicInfoWidget.cpp \
    JSONGUI/JSONGUIGroupInputWidget.cpp \
    JSONGUI/JSONGUIGroupPassWidget.cpp \
    JSONGUI/JSONGUIInputAudioWidget.cpp \
    JSONGUI/JSONGUIInputAudioFFTWidget.cpp \
    JSONGUI/JSONGUIInputBoolWidget.cpp \
    JSONGUI/JSONGUIInputColorWidget.cpp \
    JSONGUI/JSONGUIInputEventWidget.cpp \
    JSONGUI/JSONGUIInputFloatWidget.cpp \
    JSONGUI/JSONGUIInputImageWidget.cpp \
    JSONGUI/JSONGUIInputLongWidget.cpp \
    JSONGUI/JSONGUIInputPoint2DWidget.cpp \
    JSONGUI/JSONGUIPassWidget.cpp \
    JSONGUI/JSONGUIArrayGroup.cpp \
    JSONGUI/JSONGUIDictGroup.cpp \
    JSONGUI/JSONGUIInput.cpp \
    JSONGUI/JSONGUIPass.cpp \
    JSONGUI/JSONGUIPersistentBuffer.cpp \
    JSONGUI/JSONGUITop.cpp

HEADERS += \
        MainWindow.h \
    LoadingWindow.h \
    OutputWindow.h \
	../common/GLBufferQWidget.h \
    DocWindow.h \
    SimpleSourceCodeEdit.h \
    ISFController.h \
    ISFUIItem.h \
    ISFRemoteImageClient.h \
	QDoubleSlider.h \
    JSONScrollWidget.h \
    JSONGUI/JSONGUIBasicInfoWidget.h \
    JSONGUI/JSONGUIGroupInputWidget.h \
    JSONGUI/JSONGUIGroupPassWidget.h \
    JSONGUI/JSONGUIInputAudioWidget.h \
    JSONGUI/JSONGUIInputAudioFFTWidget.h \
    JSONGUI/JSONGUIInputBoolWidget.h \
    JSONGUI/JSONGUIInputColorWidget.h \
    JSONGUI/JSONGUIInputEventWidget.h \
    JSONGUI/JSONGUIInputFloatWidget.h \
    JSONGUI/JSONGUIInputImageWidget.h \
    JSONGUI/JSONGUIInputLongWidget.h \
    JSONGUI/JSONGUIInputPoint2DWidget.h \
    JSONGUI/JSONGUIPassWidget.h \
    JSONGUI/JSONGUIArrayGroup.h \
    JSONGUI/JSONGUIDictGroup.h \
    JSONGUI/JSONGUIInput.h \
    JSONGUI/JSONGUIPass.h \
    JSONGUI/JSONGUIPersistentBuffer.h \
    JSONGUI/JSONGUITop.h

FORMS += \
        MainWindow.ui \
    LoadingWindow.ui \
    OutputWindow.ui \
    DocWindow.ui \
    JSONGUI/JSONGUIGroupInputWidget.ui \
    JSONGUI/JSONGUIBasicInfoWidget.ui \
    JSONGUI/JSONGUIGroupPassWidget.ui \
    JSONGUI/JSONGUIInputAudioWidget.ui \
    JSONGUI/JSONGUIInputAudioFFTWidget.ui \
    JSONGUI/JSONGUIInputBoolWidget.ui \
    JSONGUI/JSONGUIInputColorWidget.ui \
    JSONGUI/JSONGUIInputEventWidget.ui \
    JSONGUI/JSONGUIInputFloatWidget.ui \
    JSONGUI/JSONGUIInputImageWidget.ui \
    JSONGUI/JSONGUIInputLongWidget.ui \
    JSONGUI/JSONGUIInputPoint2DWidget.ui \
    JSONGUI/JSONGUIPassWidget.ui

RESOURCES += \
    shaderLanguagefiles.qrc \
    resources.qrc




# additions for VVGL lib
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../VVGL/release/ -lVVGL
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../VVGL/debug/ -lVVGL
else:unix: LIBS += -L$$OUT_PWD/../VVGL/ -lVVGL

# additions for VVISF lib
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../VVISF/release/ -lVVISF
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../VVISF/debug/ -lVVISF
else:unix: LIBS += -L$$OUT_PWD/../VVISF/ -lVVISF

INCLUDEPATH += $$_PRO_FILE_PWD_/../../../VVGL/include
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../VVISF/include
INCLUDEPATH += $$_PRO_FILE_PWD_/../common
INCLUDEPATH += $$_PRO_FILE_PWD_/../
#DEPENDPATH += $$PWD/../VVGL
#DEPENDPATH += $$PWD/../VVISF
INCLUDEPATH += $$_PRO_FILE_PWD_/JSONGUI




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




# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target




# macs need some assembly for deployment
mac	{
	QMAKE_INFO_PLIST = Info.plist
	
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
		QMAKE_POST_LINK += cp -vaRf $$OUT_PWD/../VVISF/libVVISF*.dylib $$framework_dir;
		QMAKE_POST_LINK += macdeployqt $$OUT_PWD/$$TARGET\.app;
	}
}
