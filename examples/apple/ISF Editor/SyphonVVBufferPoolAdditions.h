//
//  SyphonVVBufferPoolAdditions.h
//  ISF Syphon Filter Tester
//
//  Created by bagheera on 11/25/13.
//  Copyright (c) 2013 zoidberg. All rights reserved.
//

#import <Cocoa/Cocoa.h>
//#import <VVGLKit/VVGLKit.h>
//#import <VVBufferPool/VVBufferPool.h>
//#import <VVISFKit/VVISFKit.h>
#include "ISFKit.h"
#import <Syphon/Syphon.h>




//	i'm defining this arbitrary constant as the value "100".  the VVBufferBackID type declared in the vvgl lib is a convenience variable- it doesn't affect the functionality of the backend at all, and exists to make it easier to quickly create ad-hoc bridges between this framework and other graphic APIs.
#define VVGLBufferBackingID_Syphon 100



VVGLBufferRef CreateBufferForSyphonClient(SyphonClient * c, const VVGLBufferPoolRef & inPoolRef=GetGlobalBufferPool());

