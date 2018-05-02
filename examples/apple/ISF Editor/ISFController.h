//
//  ISFController.h
//  ISF Syphon Filter Tester
//
//  Created by bagheera on 11/2/13.
//  Copyright (c) 2013 zoidberg. All rights reserved.
//

#import <Cocoa/Cocoa.h>
//#import <VVBufferPool/VVBufferPool.h>
//#import <VVISF/VVISF.hpp>
#include "VVISF.hpp"
#import "ISFUIItem.h"
#import "MutLockArray.h"




using namespace VVGL;




@interface ISFController : NSObject	{
	IBOutlet id				appDelegate;
	IBOutlet NSScrollView	*uiScrollView;
	
	IBOutlet NSTextField	*widthField;
	IBOutlet NSTextField	*heightField;
	NSSize					renderSize;
	
	//ISFGLScene		*scene;
	VVISF::ISFSceneRef	scene;
	BOOL			sceneIsFilter;
	
	NSString				*targetFile;
	
	MutLockArray			*itemArray;
}

- (void) setSharedGLContext:(GLContextRef)n;

- (IBAction) widthFieldUsed:(id)sender;
- (IBAction) heightFieldUsed:(id)sender;
- (IBAction) doubleResClicked:(id)sender;
- (IBAction) halveResClicked:(id)sender;
- (void) _pushUIToRenderingResolution;
- (void) _pushRenderingResolutionToUI;
@property (assign,readwrite) NSSize renderSize;

- (void) loadFile:(NSString *)f;
- (VVGL::GLBufferRef) renderFXOnThisBuffer:(VVGL::GLBufferRef)n passDict:(NSMutableDictionary *)d;
//	only used to render for recording!
- (void) renderIntoBuffer:(VVGL::GLBufferRef)b atTime:(double)t;

- (void) populateUI;

- (void) passNormalizedMouseClickToPoints:(NSPoint)p;

//- (ISFGLScene *) scene;
- (VVISF::ISFSceneRef) scene;
- (NSString *) targetFile;
- (void) reloadTargetFile;

@end
