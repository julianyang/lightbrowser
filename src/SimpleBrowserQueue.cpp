/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

SimpleBrowerQueue implementation as a browser request queue.
*/

#include "include/SimpleBrowserQueue.h"

namespace LightBrowser {

const static int defaultMinRequestTime = 3000; // default min request time is 3s.

void SimpleBrowserQueue::SimpleRequestQueue::Swap(SimpleRequestQueue *queue) 
{
    c.swap(queue->c);
}

void SimpleBrowserQueue::loadRequests()
{
    if (!m_workQueue.empty())
        return;

    {
        base::AutoLock lock(m_incomeQueueLock);
        if (m_incomeQueue.empty())
            return;

        m_incomeQueue.Swap(&m_workQueue);  // Constant time
    }
}

bool SimpleBrowserQueue::peekRequest(BrowserRequest &request)
{
    loadRequests();
    if (m_workQueue.empty())
        return false;

    WrapperBrowserRequest &wrapperRequest = m_workQueue.front();
    base::TimeDelta queueWaitTime = base::TimeTicks::Now() - wrapperRequest.m_inQueueTime;
    request = wrapperRequest.m_request;
    request.m_timeout -= queueWaitTime.InMilliseconds();
    if (request.m_timeout < defaultMinRequestTime)
        request.m_timeout = defaultMinRequestTime; // min request time is 3s.

    m_workQueue.pop();
    return true;
}

void SimpleBrowserQueue::appendRequest(const BrowserRequest &request, bool &isOriginEmptyQueue)
{
    {
        base::AutoLock locked(m_incomeQueueLock);
        m_requestNumber++;
        isOriginEmptyQueue = m_incomeQueue.empty();
        m_incomeQueue.push(request);
    }
}

}