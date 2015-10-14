/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_VIEW_OBSERVER
#define BROWSER_VIEW_OBSERVER

namespace LightBrowser {
class BrowserView;

class BrowserViewObserver {
public:
    virtual void finishLoad(BrowserView *view) = 0;

protected:
    virtual ~BrowserViewObserver() {}
};

}

#endif