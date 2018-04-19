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
	VVGL::GLBufferRef		bufferRef;
}
+ (id) createWithBuffer:(VVGL::GLBufferRef)n;
- (id) initWithBuffer:(VVGL::GLBufferRef)n;
//- (VVGL::GLBufferRef) bufferRef;
- (const VVGL::GLBufferRef &) bufferRef;

@end
