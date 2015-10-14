/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_VIEW_REQUEST_MONITOR
#define BROWSER_VIEW_REQUEST_MONITOR

#include "base/timer.h"
#include "base/memory/ref_counted.h"

namespace LightBrowser {

class BrowserView;

class BrowserViewRequestMonitor : public base::RefCountedThreadSafe<BrowserViewRequestMonitor> {
public:
    explicit BrowserViewRequestMonitor(BrowserView *view);

    ~BrowserViewRequestMonitor();

    void setNextFireInterval(int, int);
    void stop();

private:
    static void destroyTimer(base::Timer *timer);
    void setNextFireIntervalOnWorkerThread(int, int);
    void stopOnWorkerThread();
    void doTimeout();

    base::OneShotTimer<BrowserViewRequestMonitor> *m_timer;
    int m_identifier;
    BrowserView *m_view;
};

}

#endif