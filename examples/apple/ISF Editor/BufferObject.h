//
//  BufferObject.h
//  ISFSandbox
//
//  Created by testAdmin on 7/26/17.
//  Copyright © 2017 vidvox. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "VVGL.hpp"


@interface BufferObject : NSObject	{
	VVGL::VVGLBufferRef		bufferRef;
}
+ (id) createWithBuffer:(VVGL::VVGLBufferRef)n;
- (id) initWithBuffer:(VVGL::VVGLBufferRef)n;
//- (VVGL::VVGLBufferRef) bufferRef;
- (const VVGL::VVGLBufferRef &) bufferRef;

@end
