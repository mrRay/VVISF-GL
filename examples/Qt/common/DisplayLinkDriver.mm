#include "DisplayLinkDriver.h"

#include <QDebug>

#include <CoreVideo/CoreVideo.h>
//#include "VVGL.hpp"
#import <OpenGL/OpenGL.h>
#import <AppKit/AppKit.h>




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext);




DisplayLinkDriver::DisplayLinkDriver(QObject * inParent) : QObject(inParent)	{
	//qDebug() << __PRETTY_FUNCTION__;
}
DisplayLinkDriver::~DisplayLinkDriver()	{
	stop();
}


void DisplayLinkDriver::performCallback()	{
	//qDebug() << __PRETTY_FUNCTION__;
	if (_displayLinkCallback != nullptr)
		_displayLinkCallback();
}


void DisplayLinkDriver::setDisplayLinkCallback(const DisplayLinkCallback & n)	{
	_displayLinkCallback = n;
}


void DisplayLinkDriver::start()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	if (_displayLink != NULL)
		return;
	
	@autoreleasepool	{
		//	make the displaylink, which will drive rendering
		CVReturn				err = kCVReturnSuccess;
		CGOpenGLDisplayMask		totalDisplayMask = 0;
		GLint					virtualScreen = 0;
		GLint					displayMask = 0;
	
	
	
	
		CGError					cgErr = kCGErrorSuccess;
		CGDirectDisplayID		dspys[10];
		CGDisplayCount			count = 0;
		uint32_t				glDisplayMask = 0;
		cgErr = CGGetActiveDisplayList(10,dspys,&count);
		if (cgErr == kCGErrorSuccess)	{
			int					i;
			for (i=0;i<count;++i)
				glDisplayMask = glDisplayMask | CGDisplayIDToOpenGLDisplayMask(dspys[i]);
		}
	
	
	
	
		//CGLPixelFormatObj		fmt = VVGL::CreateDefaultPixelFormat();
		CGLPixelFormatAttribute		attribs[] = {
			kCGLPFAAccelerated,
			kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_Legacy,
			kCGLPFADisplayMask, (CGLPixelFormatAttribute)glDisplayMask,
			kCGLPFANoRecovery,
			kCGLPFAAllowOfflineRenderers,
			(CGLPixelFormatAttribute)0
		};
		CGLError			cglErr = kCGLNoError;
		int32_t				screenCount = 0;
		CGLPixelFormatObj	fmt = nullptr;
		cglErr = CGLChoosePixelFormat(attribs, &fmt, &screenCount);
		//if (cglErr != kCGLNoError)
		//	cout << "\terr: " << cglErr << ", " << __PRETTY_FUNCTION__ << endl;
		NSOpenGLPixelFormat		*format = [[[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:fmt] autorelease];
		CGLReleasePixelFormat(fmt);
		
		
		
		
		for (virtualScreen=0; virtualScreen<[format numberOfVirtualScreens]; ++virtualScreen)	{
			[format getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
			totalDisplayMask |= displayMask;
		}
		CVDisplayLinkRef		tmpDisplayLink = NULL;
		err = CVDisplayLinkCreateWithOpenGLDisplayMask(totalDisplayMask, &tmpDisplayLink);
		if (err)	{
			NSLog(@"\t\terr %d creating display link in %s",err,__func__);
			tmpDisplayLink = NULL;
		}
		else	{
			CVDisplayLinkSetOutputCallback(tmpDisplayLink, displayLinkCallback, this);
			CVDisplayLinkStart(tmpDisplayLink);
		}
		
		_displayLink = static_cast<void*>(tmpDisplayLink);
	}
}
void DisplayLinkDriver::stop()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	if (_displayLink == NULL)
		return;
	
	@autoreleasepool	{
		CVDisplayLinkRef		tmpDisplayLink = static_cast<CVDisplayLinkRef>(_displayLink);
		CVDisplayLinkStop(tmpDisplayLink);
		CVDisplayLinkRelease(tmpDisplayLink);
		_displayLink = NULL;
	}
}








CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, 
	const CVTimeStamp *inNow, 
	const CVTimeStamp *inOutputTime, 
	CVOptionFlags flagsIn, 
	CVOptionFlags *flagsOut, 
	void *displayLinkContext)
{
	NSAutoreleasePool		*pool =[[NSAutoreleasePool alloc] init];
	static_cast<DisplayLinkDriver *>(displayLinkContext)->performCallback();
	[pool release];
	return kCVReturnSuccess;
}
