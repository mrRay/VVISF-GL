#import <Foundation/Foundation.h>
//#import <VVBasics/VVBasics.h>
#import "JSONGUITop.h"
#import "MutLockDict.h"
#import "ObjectHolder.h"




@interface JSONGUIInput : NSObject	{
	MutLockDict			*dict;
	ObjectHolder		*top;
}

- (id) initWithDict:(NSDictionary *)n top:(JSONGUITop *)t;

- (id) objectForKey:(NSString *)k;
- (void) setObject:(id)n forKey:(NSString *)k;
- (JSONGUITop *) top;

- (NSMutableDictionary *) createExportDict;


@end
