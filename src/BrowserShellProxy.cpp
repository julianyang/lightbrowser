/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

BrowerViewRequestProxy implementation for current BrowserView.
*/

#include "include/BrowserShellProxy.h"
#include "include/BrowserShell.h"
#include "include/BrowserQueueManager.h"
#include "include/BrowserViewGroup.h"
#include "base/logging.h"
#include "base/message_loop.h"


namespace LightBrowser {

MessageLoop *BrowserShellProxy::getMainThreadForCurrentShell()
{
    // FixMe: not safe.
    return BrowserShell::getMainThread();
}

MessageLoop *BrowserShellProxy::getReceiverThreadForCurrentShell()
{
    // FixMe: not safe.
    return BrowserShell::getReceiverThread();
}

MessageLoop *BrowserShellProxy::getWorkerThreadForCurrentShell()
{
    // FixMe: not safe.
    return BrowserShell::getWorkerThread();
}

BrowserQueueManager *BrowserShellProxy::getBrowserQueueManager()
{
    return BrowserShell::s_browserShell->m_queueManager;
}

void BrowserShellProxy::tryToIncreaseBrowserView(size_t queueSize)
{
    BrowserShell::s_browserShell->tryToIncreaseBrowserView(queueSize);
}

void BrowserShellProxy::notifyStopLoad(void *view, void *p)
{
    BrowserShell::s_browserShell->notifyStopLoad(view, p);
}

}