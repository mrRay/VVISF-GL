#include "InterAppVideoSource_Mac.h"

//#include <QtPlatformHeaders/QWindowsWindowFunctions>
//#include <QtPlatformHeaders/QXcbWindowFunctions>
//#import <QCocoaNativeContext>
//#include <QCocoaWindowFunctions>
//#include <QCocoaNativeContext>
//#include <QtPlatformHeaders/qcocoanativecontext.h>
#include <QtPlatformHeaders/QCocoaNativeContext>
#include <QDebug>

//#import "Syphon.h"
#import <Syphon/Syphon.h>

#import <Cocoa/Cocoa.h>

#include "VVGL.hpp"
#include "SyphonVVBufferPoolAdditions.h"




using namespace VVGL;








//	this c++ class stores objective-c objects- it exists so we can isolate the obj-c from the c++ header
class InterAppVideoSource_MacOpaque	{
public:
	/*	we need to use NSNotificationCenter to register for notifications that the list of syphon 
	servers has been updated.  we also need to UN-register for these notifications, or we'll crash.  in 
	order to UN-register, you need to retain an NSObject that was automatically created for you when you 
	register for the notification.  which means we need to retain an array of NSObjects.				*/
	InterAppVideoSource_MacOpaque()	{
		@autoreleasepool{
			observers=[[NSMutableArray alloc] init];
		}
	}
	~InterAppVideoSource_MacOpaque()	{
		@autoreleasepool{
			if (observers!=nil)
				[observers release];
			observers=nil;
			if (ctx!=nil)
				ctx=nullptr;
			if (client!=nil)
				[client release];
			client=nil;
		}
	}
	
	NSMutableArray			*observers = nil;
	GLContextRef			ctx = nullptr;
	__block SyphonClient			*client = nil;
	__block NSThread				*thread = nil;
};








//	this function executes a block on a given background thread
typedef void (^BasicBlock)(void);
void RunOnThread(NSThread *thread, BOOL wait, BasicBlock block)
{
    [[[block copy] autorelease]
    	performSelector: @selector(my_callBlock)
    	onThread: thread
    	withObject: nil
    	waitUntilDone: wait];
}
//	this static thread is created and will exist until the app launches.  only do syphon processing on it.
static NSThread			*syphonThread = nil;
static NSRunLoop		*syphonThreadRunLoop = nil;








InterAppVideoSource_Mac::InterAppVideoSource_Mac(QObject *parent) : VideoSource(parent)	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//	if we don't have a syphon thread yet, make one
	if (syphonThread == nil)	{
		@autoreleasepool	{
			syphonThread = [[NSThread alloc]
				initWithBlock:^()	{
					@autoreleasepool	{
						//	store a ref to the runloop
						syphonThreadRunLoop = [NSRunLoop currentRunLoop];
						//	add a one-year timer to the run loop, so it will run & pause when i tell the run loop to run
						[NSTimer scheduledTimerWithTimeInterval:60.0*60.0*24.0*7.0*52.0 repeats:NO block:^(NSTimer *t) { Q_UNUSED(t); }];
						
						//	loop indefinitely, running the loop.  not very elegant, but this is a simple app.
						while (1) { CFRunLoopRunInMode(kCFRunLoopDefaultMode, 2.0, false); }
						
						//	kill the runloop ref- if we're here, the thread's exiting...
						syphonThreadRunLoop = nil;
					}
				}];
			[syphonThread start];
		}
	}
	
	//	make a new opaque object
	opaque = new InterAppVideoSource_MacOpaque();
	
	@autoreleasepool	{
		//NSLog(@"%s - %@",__func__,[[SyphonServerDirectory sharedDirectory] servers]);
		//	make sure these classes are loaded/initialized (backend has singletons that are created on class init)
		[SyphonServer class];
		[SyphonClient class];
		[SyphonImage class];
		[SyphonServerDirectory class];
		
		//	register to receive notifications that the list of available servers has changed in some way
		NSArray			*notificationNames = @[ SyphonServerAnnounceNotification, SyphonServerUpdateNotification, SyphonServerRetireNotification ];
		for (NSString *notificationName in notificationNames) {
			id				tmpObserver = [[NSNotificationCenter defaultCenter]
				addObserverForName:notificationName
				object:nil
				queue:nil
				usingBlock:^(NSNotification *note)	{
					Q_UNUSED(note);
					//	emit a signal indicating that the list of static sources has changed
					emit staticSourceUpdated(this);
				}];
			//	store the tmp observer in an array so we can use it to unregister for the notification later
			if (tmpObserver != nil)
				[opaque->observers addObject:tmpObserver];
		}
	}
}
InterAppVideoSource_Mac::~InterAppVideoSource_Mac()	{
	stop();
	@autoreleasepool	{
		for (id observer in opaque->observers) {
			[[NSNotificationCenter defaultCenter] removeObserver:observer];
		}
	}
	delete opaque;
	opaque = nullptr;
}



//VVGL::GLBufferRef InterAppVideoSource_Mac::getBuffer()	{
//	return nullptr;
//}
QList<MediaFile> InterAppVideoSource_Mac::createListOfStaticMediaFiles()	{
	//qDebug() << __PRETTY_FUNCTION__;
	QList<MediaFile>		returnMe;
	
	@autoreleasepool	{
		for (NSDictionary *serverDict in [[SyphonServerDirectory sharedDirectory] servers])	{
			//NSLog(@"\t\tserverDict is %@",serverDict);
			NSString		*tmpAppNameNS = [serverDict objectForKey:SyphonServerDescriptionAppNameKey];
			QString			tmpAppName([tmpAppNameNS UTF8String]);

			NSString		*tmpServerNameNS = [serverDict objectForKey:SyphonServerDescriptionNameKey];
			QString			tmpServerName([tmpServerNameNS UTF8String]);
			
			NSString		*tmpUUIDNS = [serverDict objectForKey:SyphonServerDescriptionUUIDKey];
			QString			tmpUUID([tmpUUIDNS UTF8String]);
			
			returnMe.append(MediaFile(MediaFile::Type_App, tmpAppName+"-"+tmpServerName, tmpUUID));
		}
	}
	
	return returnMe;
}
void InterAppVideoSource_Mac::start()	{
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	if (_running)
		return;
	
	if (_file.type() != MediaFile::Type_App)
		return;
	
	_running=true;
	
	@autoreleasepool	{
		
		//	get a new GL context from the buffer pool
		if (opaque->ctx != nullptr)	{
			opaque->ctx = nullptr;
		}
		opaque->ctx = GetGlobalBufferPool()->context()->newContextSharingMe();
		
		//	we have a UUID, but we need a dictionary describing the server.  use the UUID to get this.
		NSDictionary		*targetServerDict = nil;
		NSString			*targetUUID = [NSString stringWithUTF8String:_file.syphonUUID().toUtf8()];
		for (NSDictionary *serverDict in [[SyphonServerDirectory sharedDirectory] servers])	{
			NSString			*tmpUUID = [serverDict objectForKey:SyphonServerDescriptionUUIDKey];
			if (tmpUUID!=nil && targetUUID!=nil && [tmpUUID isEqualToString:targetUUID])	{
				targetServerDict = serverDict;
				break;
			}
		}
		
		//	if we have a context and a target server dict, we can make a syphon client...
		if (opaque->ctx!=nullptr && targetServerDict != nil)	{
			QVariant			nativeHandle = opaque->ctx->nativeHandle();
			//qDebug() << "native handle is " << nativeHandle;
			if (nativeHandle.type() != QVariant::nameToType("QCocoaNativeContext"))	{
				qDebug() << "ERR: variant (" << nativeHandle << ") is wrong type, " << __PRETTY_FUNCTION__;
			}
			else	{
				QCocoaNativeContext		nativeCtx = nativeHandle.value<QCocoaNativeContext>();
				
				//				making a SyphonClient on the main thread breaks Qt- the pop-up button 
				//		that i clicked to load a file/call this function stops working (can't select 
				//		items in it any longer).  so instead, we execute a block on a thread we created 
				//		just for syphon processing, and create the syphon client in there.
				CFRunLoopRef		rlr = (syphonThreadRunLoop==nil) ? nil : [syphonThreadRunLoop getCFRunLoop];
				if (rlr != nil)	{
					CFRunLoopPerformBlock(rlr, kCFRunLoopDefaultMode, ^{
						//	make the client on the other thread
						opaque->client = [[SyphonClient alloc]
							initWithServerDescription:targetServerDict
							context:[nativeCtx.context() CGLContextObj]
							options:nil
							preferCurrentThread:YES
							newFrameHandler:^(SyphonClient *theClient)	{
								//	make a new GLBuffer for the latest frame, emit it
								GLBufferRef		newBuffer = CreateBufferForSyphonClient(theClient);
								if (newBuffer != nullptr)	{
									emit frameProduced(newBuffer);
								}
							}];
					});
				}
				
			}
		}
	}
}
void InterAppVideoSource_Mac::stop()	{
	//qDebug() << __PRETTY_FUNCTION__;
	VideoSource::stop();
	
	@autoreleasepool	{
		if (opaque->client != nil)	{
			[opaque->client release];
			opaque->client = nil;
		}
	}
}
bool InterAppVideoSource_Mac::playingBackItem(const MediaFile & n)	{
	return (_file == n);
}
void InterAppVideoSource_Mac::loadFile(const MediaFile & n)	{
	//if (n.type() != MediaFile::Type_App)	//	if it's not the right kind of file we'll still accept it- but we won't actually start
	//	return;
	
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	if (_file == n)
		return;
	
	stop();
	_file = n;
	start();
}
