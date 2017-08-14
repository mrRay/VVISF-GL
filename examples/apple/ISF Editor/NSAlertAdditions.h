#import <Cocoa/Cocoa.h>




@interface NSAlert (NSAlertAdditions)

- (NSInteger) runModalForWindow:(NSWindow *)aWindow;
- (IBAction) closeAlertsAppModalSession:(id)sender;

@end



NSInteger VVRunAlertPanel(NSString *title, NSString *msg, NSString *btnA, NSString *btnB, NSString *btnC);
NSInteger VVRunAlertPanelSuppressString(NSString *title, NSString *msg, NSString *btnA, NSString *btnB, NSString *btnC, NSString *suppressString, BOOL *returnSuppressValue);

