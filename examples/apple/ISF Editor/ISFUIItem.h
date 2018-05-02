//
//  ISFUIItem.h
//  ISF Syphon Filter Tester
//
//  Created by bagheera on 11/2/13.
//  Copyright (c) 2013 zoidberg. All rights reserved.
//

#import <Cocoa/Cocoa.h>
//#import <VVBufferPool/VVBufferPool.h>
//#import <VVISF/VVISF.hpp>
#include "VVISF.hpp"
#import <Syphon/Syphon.h>
#import "SyphonVVBufferPoolAdditions.h"
#import "ISFAudioFFT.h"




using namespace VVISF;




@interface ISFUIItem : NSBox	{
	NSString				*name;
	//ISFAttribValType	type;
	ISFValType			type;
	
	NSButton		*eventButton;
	BOOL			eventNeedsSending;
	NSButton		*boolButton;
	NSPopUpButton	*longPUB;
	NSSlider		*slider;
	/*
	NSSlider		*xSlider;
	NSSlider		*ySlider;
	*/
	NSTextField		*xField;
	NSTextField		*yField;
	NSPoint			pointVal;
	NSColorWell		*colorField;
	NSPopUpButton	*audioSourcePUB;
	
	NSDictionary	*userInfoDict;	//	used to store float flag and max val for audio-type inputs
	
	SyphonClient	*syphonClient;
	NSString		*syphonLastSelectedName;
}

//- (id) initWithFrame:(NSRect)f attrib:(ISFAttrib *)a;
- (id) initWithFrame:(NSRect)f attrib:(VVISF::ISFAttrRef)a;

- (void) uiItemUsed:(id)sender;

- (NSString *) name;
- (id) getNSObjectValue;
- (void) setNSObjectValue:(id)n;
- (ISFVal) getISFVal;

@property (retain,readwrite) NSDictionary *userInfoDict;

@end
