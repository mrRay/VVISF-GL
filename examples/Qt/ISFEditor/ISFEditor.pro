#-------------------------------------------------
#
# Project created by QtCreator 2018-09-11T16:33:13
#
#-------------------------------------------------

QT       += core gui widgets opengl multimedia

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




# let's try a precompiled header to reduce compilation time
PRECOMPILED_HEADER = $$_PRO_FILE_PWD_/precompiled.pch
CONFIG += precompile_header

# theoretically, this should enable some degree of multiprocessing in MSVC, but i need to test it...
*msvc* {
	QMAKE_CXXFLAGS += -MP
}




# these libs require a VVGL_SDK define
DEFINES += VVGL_SDK_QT




SOURCES += \
	../common/GLBufferQVideoSurface.cpp \
	../common/GLBufferQWidget.cpp \
	../common/ISFGLBufferQWidget.cpp \
	DocWindow.cpp \
	ISFController.cpp \
	JSONGUI/JGMCArray.cpp \
	JSONGUI/JGMObject.cpp \
	JSONGUI/JGMTop.cpp \
	JSONGUI/JSONGUIBasicInfoWidget.cpp \
	JSONGUI/JSONGUIGroupInputWidget.cpp \
	JSONGUI/JSONGUIGroupPassWidget.cpp \
	JSONGUI/JSONGUIInputAudioFFTWidget.cpp \
	JSONGUI/JSONGUIInputAudioWidget.cpp \
	JSONGUI/JSONGUIInputBoolWidget.cpp \
	JSONGUI/JSONGUIInputColorWidget.cpp \
	JSONGUI/JSONGUIInputEventWidget.cpp \
	JSONGUI/JSONGUIInputFloatWidget.cpp \
	JSONGUI/JSONGUIInputImageWidget.cpp \
	JSONGUI/JSONGUIInputLongWidget.cpp \
	JSONGUI/JSONGUIInputPoint2DWidget.cpp \
	JSONGUI/JSONGUIPassWidget.cpp \
	JSONScrollWidget.cpp \
	LoadingWindow.cpp \
	main.cpp \
	MainWindow.cpp \
	misc_classes/MediaFile.cpp \
	misc_classes/VideoSourceMenuItem.cpp \
	misc_ui/ISFUIItem.cpp \
	misc_ui/SimpleSourceCodeEdit.cpp \
	OutputWindow.cpp \
	VideoSource/DynamicVideoSource.cpp \
	VideoSource/ImgVideoSource.cpp \
	VideoSource/InterAppVideoSource.cpp \
	VideoSource/MovieVideoSource.cpp \
	VideoSource/VideoSource.cpp \
	VideoSource/WebCamVideoSource.cpp \
    misc_classes/LevenshteinCalc.cpp \
    misc_classes/RemoteImageClient.cpp \
    misc_classes/RemoteImageClient_Mac.cpp \
    misc_ui/ISFFileListView.cpp \
    misc_classes/LoadingWindowFileListModel.cpp \
    JSONGUI/JSONGUIInputWidget.cpp


HEADERS += \
	../common/GLBufferQVideoSurface.h \
	../common/GLBufferQWidget.h \
	../common/ISFGLBufferQWidget.h \
	DocWindow.h \
	ISFController.h \
	JSONGUI/JGMCArray.h \
	JSONGUI/JGMDefs.h \
	JSONGUI/JGMObject.h \
	JSONGUI/JGMTop.h \
	JSONGUI/JSONGUIBasicInfoWidget.h \
	JSONGUI/JSONGUIGroupInputWidget.h \
	JSONGUI/JSONGUIGroupPassWidget.h \
	JSONGUI/JSONGUIInputAudioFFTWidget.h \
	JSONGUI/JSONGUIInputAudioWidget.h \
	JSONGUI/JSONGUIInputBoolWidget.h \
	JSONGUI/JSONGUIInputColorWidget.h \
	JSONGUI/JSONGUIInputEventWidget.h \
	JSONGUI/JSONGUIInputFloatWidget.h \
	JSONGUI/JSONGUIInputImageWidget.h \
	JSONGUI/JSONGUIInputLongWidget.h \
	JSONGUI/JSONGUIInputPoint2DWidget.h \
	JSONGUI/JSONGUIPassWidget.h \
	JSONGUI/QLabelClickable.h \
	JSONScrollWidget.h \
	LoadingWindow.h \
	MainWindow.h \
	misc_classes/MediaFile.h \
	misc_classes/VideoSourceMenuItem.h \
	misc_ui/ISFUIItem.h \
	misc_ui/SimpleSourceCodeEdit.h \
	OutputWindow.h \
	misc_ui/QDoubleSlider.h \
	VideoSource/DynamicVideoSource.h \
	VideoSource/ImgVideoSource.h \
	VideoSource/InterAppVideoSource.h \
	VideoSource/MovieVideoSource.h \
	VideoSource/VideoSource.h \
	VideoSource/WebCamVideoSource.h \
    misc_classes/LevenshteinCalc.h \
    misc_classes/RemoteImageClient.h \
    misc_classes/RemoteImageClient_Mac.h \
    misc_ui/ISFFileListView.h \
    misc_classes/LoadingWindowFileListModel.h \
    JSONGUI/JSONGUIInputWidget.h \
    JSONGUI/QLabelDrag.h

# platform-specific classes
mac	{
	SOURCES += ../common/SyphonVVBufferPoolAdditions.mm \
		VideoSource/InterAppVideoSource_Mac.mm
	HEADERS += ../common/SyphonVVBufferPoolAdditions.h \
		VideoSource/InterAppVideoSource_Mac.h
}


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
    misc_ui/shaderLanguagefiles.qrc \
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
INCLUDEPATH += $$_PRO_FILE_PWD_/VideoSource
#DEPENDPATH += $$PWD/../VVGL
#DEPENDPATH += $$PWD/../VVISF
INCLUDEPATH += $$_PRO_FILE_PWD_/JSONGUI
INCLUDEPATH += $$_PRO_FILE_PWD_/misc_classes
INCLUDEPATH += $$_PRO_FILE_PWD_/misc_ui
INCLUDEPATH += $$_PRO_FILE_PWD_/Syphon




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




# mac-only additions for syphon
mac	{
	LIBS += -framework Foundation -framework Cocoa -framework AppKit

	SYPHON_FRAMEWORK_PATH=$$_PRO_FILE_PWD_/Syphon
	QMAKE_CXXFLAGS += -F $$SYPHON_FRAMEWORK_PATH
	QMAKE_LFLAGS += -F $$SYPHON_FRAMEWORK_PATH
	LIBS += -framework Syphon
	#LIBS += -F$$SYPHON_FRAMEWORK_PATH
	LIBS += -L$$SYPHON_FRAMEWORK_PATH

	# syphon needs a CGLContextObj, which means we need to get an NSOpenGLContext from a QOpenGLContext, the headers for which are only accessible by manually including this path so the QtPlatformHeaders directory is picked up.
	INCLUDEPATH += $$QMAKESPEC/../../include/
}




# this logs every qmake var when qmake is run
#for(var, $$list($$enumerate_vars())) {
#	message($$var)
#	message($$eval($$var))
#}




# this enables ARC.  it's commented out because ARC is a PITA to use with c++
#QMAKE_CXXFLAGS += -fobjc-arc




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
		QMAKE_POST_LINK += cp -vaRf $$_PRO_FILE_PWD_/Syphon/Syphon.framework $$framework_dir;
		QMAKE_POST_LINK += macdeployqt $$OUT_PWD/$$TARGET\.app;
	}
}




