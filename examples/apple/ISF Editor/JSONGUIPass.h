#import <Foundation/Foundation.h>
//#import <VVBasics/VVBasics.h>
@class JSONGUITop;
#import "MutLockDict.h"
#import "ObjectHolder.h"




@interface JSONGUIPass : NSObject	{
	MutLockDict			*dict;
	ObjectHolder		*top;
}

- (id) initWithDict:(NSDictionary *)n top:(JSONGUITop *)t;

- (id) objectForKey:(NSString *)k;
- (void) setObject:(id)n forKey:(NSString *)k;
- (JSONGUITop *) top;

- (NSMutableDictionary *) createExportDict;

@end
