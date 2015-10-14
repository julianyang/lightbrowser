/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include "include/BrowserTaskMonitor.h"
#include "include/BrowserShellProxy.h"
#include "include/BrowserLoggerFactory.h"
#include "v8/include/v8.h"

namespace LightBrowser {

BrowserTaskMonitor::~BrowserTaskMonitor()
{
    m_terminate = true;
    BrowserShellProxy::getWorkerThreadForCurrentShell()->PostTask(FROM_HERE,
        base::Bind(&BrowserTaskMonitor::destroyTimer, m_timer));
}

void BrowserTaskMonitor::doTimeout()
{
    if (m_terminate)
        return;

    bool tastTimeOut = false;
    {
        base::AutoLock locked(m_updateTaskTime);
        tastTimeOut = (m_taskEnd <= m_taskBegin)
            && (base::TimeTicks::Now() > (m_interval + m_taskBegin));
    }

    if (!m_terminate
        && tastTimeOut) {
        if (!v8::V8::IsExecutionTerminating(m_isolate)) {
            v8::V8::TerminateExecution(m_isolate);
            WORKER_LOGGER()->warn() << "(=_=)Execute v8 terminate operator." << std::endl;
        }
    }
}

void BrowserTaskMonitor::WillProcessTask(base::TimeTicks timePosted)
{
    {
        base::AutoLock locked(m_updateTaskTime);
        m_taskBegin = m_taskEnd = base::TimeTicks::Now();
    }
}

void BrowserTaskMonitor::DidProcessTask(base::TimeTicks timePosted)
{
    {
        base::AutoLock locked(m_updateTaskTime);
        m_taskEnd = base::TimeTicks::Now();
    }
}


void BrowserTaskMonitor::start() 
{
    BrowserShellProxy::getWorkerThreadForCurrentShell()->PostTask(FROM_HERE,
        base::Bind(&BrowserTaskMonitor::initilizeOnWorkerThread, this));
}

void BrowserTaskMonitor::initilizeOnWorkerThread()
{
    if (!m_timer)
        m_timer = new base::RepeatingTimer<BrowserTaskMonitor>;

    WORKER_LOGGER()->debug() << "Start task observer timer, interval:" << m_interval.InMilliseconds()
        << std::endl;

    m_timer->Start(FROM_HERE, m_interval,
        this, &BrowserTaskMonitor::doTimeout);
}

void BrowserTaskMonitor::destroyTimer(base::Timer *timer)
{
    delete timer;
}

}