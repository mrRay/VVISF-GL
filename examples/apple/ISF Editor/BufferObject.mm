//
//  BufferObject.m
//  ISFSandbox
//
//  Created by testAdmin on 7/26/17.
//  Copyright © 2017 vidvox. All rights reserved.
//

#import "BufferObject.h"




@implementation BufferObject

+ (id) createWithBuffer:(VVGL::VVGLBufferRef)n	{
	return [[[BufferObject alloc] initWithBuffer:n] autorelease];
}
- (id) initWithBuffer:(VVGL::VVGLBufferRef)n	{
	self = [super init];
	if (self != nil)	{
		bufferRef = n;
	}
	if (n == nullptr)	{
		[self release];
		return nil;
	}
	return self;
}
- (void) dealloc	{
	bufferRef = nullptr;
	[super dealloc];
}
/*
- (VVGL::VVGLBufferRef) bufferRef	{
	return buffer;
}
*/
- (const VVGL::VVGLBufferRef &) bufferRef	{
	return bufferRef;
}

@end
