//
//  MouseView.h
//  ISF Syphon Filter Tester
//
//  Created by bagheera on 11/21/13.
//  Copyright (c) 2013 zoidberg. All rights reserved.
//

#import <Cocoa/Cocoa.h>
//#import <VVBufferPool/VVBufferPool.h>
//#import <VVISFKit/VVISFKit.h>
#include "ISFKit.h"
//#import <VVUIToolbox/VVUIToolbox.h>
#import "ISFVVGLBufferView.h"




@interface MouseView : ISFVVGLBufferView	{
	IBOutlet id			controller;
}

@end
