/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

BrowerQueueManager implementation as a browser request queue manager.
*/

#ifndef BROWSER_QUEUE_MANAGER
#define BROWSER_QUEUE_MANAGER

#include "base/memory/ref_counted.h"

namespace LightBrowser {

class BrowserQueue;
class BrowserRequest;
class BrowserView;

class BrowserQueueManager : public base::RefCountedThreadSafe<BrowserQueueManager> {
public:
    BrowserQueueManager(int type);
    virtual ~BrowserQueueManager();

    virtual void willDelete();
    // must do not run on receiver thread.
    virtual void addRequestToQueue(const BrowserRequest &request);

    virtual void tryToLoadNextRequest(const BrowserView *view);

protected:
    // block operator.
    // must do run on receiver thread.
    void peekRequestFormQueue(const BrowserView *view);
    // must be run on browser main thread.
    void loadRequest(const BrowserView *view, BrowserRequest *request);

    void increaseConsumerIfNeeded(size_t queueSize);

    volatile bool m_reciveDelete;

    BrowserQueue *m_browserQueue;
};

}

#endif