/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include "include/BrowserViewGroupImp.h"
#include "include/BrowserView.h"

namespace LightBrowser {

static const int g_maxFixedTabCount = 800;

BrowserViewGroup *createBrowserViewGroup(BrowserViewGroup::BVGroupType type, int count)
{
    BrowserViewGroup *bvg = NULL;

    switch (type) {
    case BrowserViewGroup::BVGROUP_SINGLE_TAB:
        bvg = SingleTabBrowserViewGroup::create();
        break;
    case BrowserViewGroup::BVGROUP_FIXED_TAB:
        count = count < 0 ? 5 : count;
        count = count > g_maxFixedTabCount ? g_maxFixedTabCount : count;
        bvg = FixedTabBrowserViewGroup::create(count);
        break;
    case BrowserViewGroup::BVGROUP_NOLIMITED_TAB:
        bvg = NolimitedTabBrowserViewGroup::create();
        break;
    default: // default is single tab mode.
        bvg = SingleTabBrowserViewGroup::create();
        break;
    }

    return bvg;
}

BrowserViewGroup *SingleTabBrowserViewGroup::create()
{
    return new SingleTabBrowserViewGroup;
}

SingleTabBrowserViewGroup::SingleTabBrowserViewGroup()
    : m_browserView(NULL)
{

}

SingleTabBrowserViewGroup::~SingleTabBrowserViewGroup()
{
    delete m_browserView;
}

BrowserView *SingleTabBrowserViewGroup::getBrowserView()
{
    std::string blank("about:blank");
    if (!m_browserView)
        BrowserView::createNewWindow(blank, &m_browserView);

    return m_browserView;
}

BrowserViewGroup *FixedTabBrowserViewGroup::create(const int count)
{
    return new FixedTabBrowserViewGroup(count);
}

FixedTabBrowserViewGroup::FixedTabBrowserViewGroup(const int count)
    : m_maxCount(count)
    , m_count(0)
{

}

FixedTabBrowserViewGroup::~FixedTabBrowserViewGroup()
{
    WindowList::iterator it = m_windowList.begin();
    while (it != m_windowList.end()) {
        delete (*it);
        it++;
    }

    m_windowList.clear();
}


BrowserView *FixedTabBrowserViewGroup::getBrowserView()
{
    BrowserView *view = NULL;
    if (m_count < m_maxCount) {
        // find a no use browserView.
        std::string blank("about:blank");
        BrowserView::createNewWindow(blank, &view);
        m_count++;
        m_windowList.push_back(view);
    }

    return view;
}

BrowserViewGroup *NolimitedTabBrowserViewGroup::create()
{
    return new NolimitedTabBrowserViewGroup;
}

NolimitedTabBrowserViewGroup::~NolimitedTabBrowserViewGroup()
{
    WindowList::iterator it = m_windowList.begin();
    while (it != m_windowList.end()) {
        delete (*it);
        it++;
    }
    
    m_windowList.clear();
}

void NolimitedTabBrowserViewGroup::finishLoad(BrowserView *view)
{
    m_windowList.remove(view);
    delete view;
}

BrowserView *NolimitedTabBrowserViewGroup::getBrowserView()
{
    BrowserView *view = NULL;
    
    std::string blank("about:blank");
    BrowserView::createNewWindow(blank, &view);
    view->addObserver(this);
    m_windowList.push_back(view);

    return view;
}

}