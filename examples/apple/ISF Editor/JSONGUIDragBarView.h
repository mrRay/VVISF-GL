#import <Foundation/Foundation.h>
//#import <VVUIToolbox/VVUIToolbox.h>
#import "ObjectHolder.h"



/*
//	must always be in an instance of either ISFPropInputTableCellView or ISFPropPassTableCellView
@interface JSONGUIDragBarView : VVSpriteView <NSDraggingSource>	{
	ObjectHolder		*bgSpriteHolder;
}
*/
@interface JSONGUIDragBarView : NSView	{
	ObjectHolder		*bgSpriteHolder;
}

@end
