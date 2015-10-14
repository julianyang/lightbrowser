/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_TASK_MONITOR
#define BROWSER_TASK_MONITOR

#include "base/timer.h"
#include "base/message_loop.h"
#include "base/synchronization/lock.h"
#include "base/memory/ref_counted.h"

namespace v8 {
    class Isolate;
}

namespace LightBrowser {

class BrowserView;

class BrowserTaskMonitor : public MessageLoop::TaskObserver
        , public base::RefCountedThreadSafe<BrowserTaskMonitor> {
public:
    BrowserTaskMonitor(v8::Isolate *isolate, int interval)
        : m_timer(NULL)
        , m_isolate(isolate)
        , m_terminate(false)
        , m_interval(base::TimeDelta::FromMilliseconds(interval))
        , m_taskBegin(base::TimeTicks::Now())
        , m_taskEnd(m_taskBegin)
    {
   
    }

    ~BrowserTaskMonitor();

    virtual void WillProcessTask(base::TimeTicks timePosted);

    virtual void DidProcessTask(base::TimeTicks timePosted);

    void start(); // default task long time is 5000ms.

private:
    static void destroyTimer(base::Timer *timer);
    void initilizeOnWorkerThread();
    void doTimeout();

    base::RepeatingTimer<BrowserTaskMonitor> *m_timer;
    v8::Isolate *m_isolate;
    bool m_terminate;
    mutable base::Lock m_updateTaskTime;
    base::TimeDelta m_interval;
    base::TimeTicks m_taskBegin;
    base::TimeTicks m_taskEnd;
};

}

#endif