/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

SimpleBrowerQueue implementation as a browser request queue.
*/

#ifndef SIMPLE_BROWSER_QUEUE
#define SIMPLE_BROWSER_QUEUE

#include <queue>
#include "LightBrowser.h"
#include "include/BrowserQueue.h"
#include "base/timer.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"

namespace LightBrowser {

class SimpleBrowserQueue : public BrowserQueue {
public:
    static BrowserQueue *create() { return new SimpleBrowserQueue; }
    ~SimpleBrowserQueue() {}

    virtual void wait() { m_event.Wait(); }
    virtual void signal() { m_event.Signal(); }

    virtual size_t size() { return m_workQueue.size() + m_incomeQueue.size(); }
    virtual bool peekRequest(BrowserRequest &request);
    virtual void appendRequest(const BrowserRequest &request, bool &isOriginEmptyQueue);

private:
    SimpleBrowserQueue() 
        : m_requestNumber(0)
        , m_event(false, false)
    {}

    void loadRequests();

    struct WrapperBrowserRequest {
        WrapperBrowserRequest(const BrowserRequest &request)
            : m_request(request)
            , m_inQueueTime(base::TimeTicks::Now())
        {
        }
        
        BrowserRequest m_request;
        base::TimeTicks m_inQueueTime;
    };

    class SimpleRequestQueue : public std::queue<WrapperBrowserRequest> {
    public:
        void Swap(SimpleRequestQueue* queue);
    };

    int m_requestNumber;
    base::WaitableEvent m_event;
    mutable base::Lock m_incomeQueueLock;
    SimpleRequestQueue m_incomeQueue;
    SimpleRequestQueue m_workQueue;
};


}

#endif
