// MacMenuWorkaroundBridge.cpp
// Set file type to Objective-C++ Source

// Code from StackOverflow user fardjad
// See: http://stackoverflow.com/questions/16482471/empty-wxwidgets-menus-on-osx

#import "MacMenuWorkaround.h"
#include "MacMenuWorkaroundBridge.h"

namespace CocoaBridge {
    void hideEmptyMenus() {
        [MacMenuWorkaround hideEmptyMenus];
    }
}
