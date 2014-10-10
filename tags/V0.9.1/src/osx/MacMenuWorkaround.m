// MacMenuWorkaround.m

// Code from StackOverflow user fardjad
// See: http://stackoverflow.com/questions/16482471/empty-wxwidgets-menus-on-osx

#import "MacMenuWorkaround.h"

@implementation MacMenuWorkaround

+(void) hideEmptyMenus {
    NSMenu* mainMenu = [[NSApplication sharedApplication] mainMenu];

    for (NSMenuItem* item in [mainMenu itemArray]) {
        NSMenu* menu = [item submenu];

        if ([[menu itemArray] count] < 2) {
            [item setHidden:YES];
        }
    }
}

@end
