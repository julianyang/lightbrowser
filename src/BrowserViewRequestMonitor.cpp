/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include "include/BrowserViewRequestMonitor.h"
#include "include/BrowserShellProxy.h"
#include "include/BrowserLoggerFactory.h"
#include "base/bind.h"
#include "base/message_loop.h"


namespace LightBrowser {

BrowserViewRequestMonitor::BrowserViewRequestMonitor(BrowserView *view)
    : m_timer(NULL)
    , m_identifier(-1)
    , m_view(view)
{

}

BrowserViewRequestMonitor::~BrowserViewRequestMonitor()
{
    // not implementation
    BrowserShellProxy::getWorkerThreadForCurrentShell()->PostTask(FROM_HERE,
        base::Bind(&BrowserViewRequestMonitor::destroyTimer, m_timer));
}

void BrowserViewRequestMonitor::setNextFireInterval(int nextInterval, int id)
{
    BrowserShellProxy::getWorkerThreadForCurrentShell()->PostTask(FROM_HERE,
        base::Bind(&BrowserViewRequestMonitor::setNextFireIntervalOnWorkerThread, this, nextInterval, id));
}

void BrowserViewRequestMonitor::stop()
{
    BrowserShellProxy::getWorkerThreadForCurrentShell()->PostTask(FROM_HERE,
        base::Bind(&BrowserViewRequestMonitor::stopOnWorkerThread, this));
}

void BrowserViewRequestMonitor::setNextFireIntervalOnWorkerThread(int nextInterval, int id)
{
    if (!m_timer)
        m_timer = new base::OneShotTimer<BrowserViewRequestMonitor>;

    if (nextInterval < 0)
        nextInterval = 0;

    m_timer->Stop();
    m_identifier = id;

    WORKER_LOGGER()->debug() << "Start timer, view:" << m_view 
        << ", id:" << m_identifier 
        << ", timeout:" << nextInterval
        << std::endl;

    m_timer->Start(FROM_HERE, base::TimeDelta::FromMilliseconds(nextInterval),
        this, &BrowserViewRequestMonitor::doTimeout);
}

void BrowserViewRequestMonitor::stopOnWorkerThread()
{
    WORKER_LOGGER()->debug() << "Stop timer, view:" << m_view 
        << ", id:" << m_identifier 
        << std::endl;

    m_identifier = -1;
    if (m_timer)
        m_timer->Stop();
}

void BrowserViewRequestMonitor::doTimeout()
{
    // stop current view load.
    int *p = new int(m_identifier);

    WORKER_LOGGER()->debug() << "Fire timer, view:" << m_view 
        << ", id:" << m_identifier 
        << std::endl;

    BrowserShellProxy::notifyStopLoad(static_cast<void *>(m_view), static_cast<void *>(p));
}

void BrowserViewRequestMonitor::destroyTimer(base::Timer *timer)
{
    delete timer;
}

}