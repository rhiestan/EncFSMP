// MacMenuWorkaround.h

// Code from StackOverflow user fardjad
// See: http://stackoverflow.com/questions/16482471/empty-wxwidgets-menus-on-osx

#import <Cocoa/Cocoa.h>

@interface MacMenuWorkaround : NSObject

+(void) hideEmptyMenus;

@end
