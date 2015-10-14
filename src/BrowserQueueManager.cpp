/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

BrowerQueueManager implementation as a browser request queue manager.
*/

#include "include/BrowserQueueManager.h"
#include "LightBrowser.h"
#include "include/BrowserLoggerFactory.h"
#include "include/BrowserView.h"
#include "include/BrowserQueue.h"
#include "include/BrowserShellProxy.h"
#include "base/bind.h"
#include "base/message_loop.h"

namespace LightBrowser {

BrowserQueueManager::BrowserQueueManager(int type)
    : m_reciveDelete(false)
    , m_browserQueue(createBrowserQueue(type))
{

}

void BrowserQueueManager::willDelete()
{
    m_reciveDelete = true;
    m_browserQueue->signal();
}

BrowserQueueManager::~BrowserQueueManager()
{
    delete m_browserQueue;
}

void BrowserQueueManager::addRequestToQueue(const BrowserRequest &request)
{
    bool isOriginEmpty = true;
    m_browserQueue->appendRequest(request, isOriginEmpty);

    if (isOriginEmpty)
        m_browserQueue->signal(); // notify to work.
}

void BrowserQueueManager::tryToLoadNextRequest(const BrowserView *view)
{
    BrowserShellProxy::getReceiverThreadForCurrentShell()->PostTask(FROM_HERE,
        base::Bind(&BrowserQueueManager::peekRequestFormQueue, this, view));
}

void BrowserQueueManager::peekRequestFormQueue(const BrowserView *view)
{
    if (m_reciveDelete)
        return;

    BrowserRequest *request = new BrowserRequest;
    int doWait = 0;
    while (!m_browserQueue->peekRequest(*request)) {
        m_browserQueue->wait();
        if (m_reciveDelete)
            return;
        doWait++;
    }
    
    RECEIVER_LOGGER()->debug() << "Peek a request on receiver thread|dowait:" << doWait << std::endl;
    BrowserShellProxy::getMainThreadForCurrentShell()->PostTask(FROM_HERE, 
        base::Bind(&BrowserQueueManager::loadRequest, this, view, request));

    // doWait > 0 means the queue is empty for current webview.
    // so, there is no need to increase consumer.
    if (!doWait)
        increaseConsumerIfNeeded(m_browserQueue->size());
}

void BrowserQueueManager::loadRequest(const BrowserView *view, BrowserRequest *request)
{
    const_cast<BrowserView *>(view)->load(*request);
    delete request;
}

void BrowserQueueManager::increaseConsumerIfNeeded(size_t queueSize)
{
    BrowserShellProxy::tryToIncreaseBrowserView(queueSize);
}

}