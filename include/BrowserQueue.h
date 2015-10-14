/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

BrowerQueue implementation as a browser request queue.
*/

#ifndef BROWSER_QUEUE
#define BROWSER_QUEUE
#include <stddef.h>

namespace LightBrowser {

class BrowserRequest;

class BrowserQueue {
public:
    ~BrowserQueue() {}

    virtual void wait() = 0;
    virtual void signal() = 0;
    virtual size_t size() = 0;
    virtual bool peekRequest(BrowserRequest &request) = 0;
    virtual void appendRequest(const BrowserRequest &request, bool &isOriginEmptyQueue) = 0; 
};

BrowserQueue *createBrowserQueue(int type);

}

#endif
