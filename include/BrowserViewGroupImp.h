/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_VIEW_GROUP_IMP
#define BROWSER_VIEW_GROUP_IMP

#include <list>
#include "include/BrowserViewGroup.h"
#include "include/BrowserViewObserver.h"

namespace LightBrowser {

typedef std::list<BrowserView *> WindowList;


class SingleTabBrowserViewGroup : public BrowserViewGroup {
public:
    static BrowserViewGroup *create();
    virtual ~SingleTabBrowserViewGroup();

    virtual BrowserView *getBrowserView();
    virtual size_t getBrowserViewSize() { return m_browserView ? 1 : 0; }
    virtual bool canGetMoreBrowserView() { return !m_browserView; }

protected:
    SingleTabBrowserViewGroup();

private:
    BrowserView *m_browserView;
};

class FixedTabBrowserViewGroup : public BrowserViewGroup {
public:
    static BrowserViewGroup *create(const int count);
    virtual ~FixedTabBrowserViewGroup();

    virtual BrowserView *getBrowserView();
    virtual size_t getBrowserViewSize() { return m_count; }
    virtual bool canGetMoreBrowserView() { return m_count < m_maxCount; }

protected:
    FixedTabBrowserViewGroup(const int count);   

private:
    int m_maxCount;
    int m_count;
    WindowList m_windowList;
};

class NolimitedTabBrowserViewGroup : public BrowserViewGroup, public BrowserViewObserver {
public:
    static BrowserViewGroup *create();
    virtual ~NolimitedTabBrowserViewGroup();

    virtual BrowserView *getBrowserView();
    virtual size_t getBrowserViewSize() { return m_windowList.size(); }
    virtual bool canGetMoreBrowserView() { return true; }
    virtual void finishLoad(BrowserView *view);

protected:
    NolimitedTabBrowserViewGroup() {}

private:
    WindowList m_windowList;
};

}


#endif