/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_VIEW_GROUP
#define BROWSER_VIEW_GROUP

#include <list>

namespace LightBrowser {
class BrowserView;

class BrowserViewGroup {
public:
    enum BVGroupType {
        BVGROUP_SINGLE_TAB,
        BVGROUP_FIXED_TAB,
        BVGROUP_NOLIMITED_TAB
    };
    virtual ~BrowserViewGroup() {}

    virtual BrowserView *getBrowserView() = 0;
    virtual size_t getBrowserViewSize() = 0;
    virtual bool canGetMoreBrowserView() = 0;
    
};

BrowserViewGroup *createBrowserViewGroup(BrowserViewGroup::BVGroupType type, int count = 5);

}


#endif