//
//  ISFQL_RendererProtocols.h
//  ISFSandbox
//
//  Created by testAdmin on 9/3/19.
//  Copyright © 2019 vidvox. All rights reserved.
//

#ifndef ISFQL_RendererProtocols_h
#define ISFQL_RendererProtocols_h




@protocol ISFQLAgentService
- (void) renderThumbnailForPath:(NSString *)n sized:(NSSize)s;
@end




@protocol ISFQLService
- (void) renderedBitmapData:(NSData *)d sized:(NSSize)s;
@end




#endif /* ISFQL_RendererProtocols_h */
