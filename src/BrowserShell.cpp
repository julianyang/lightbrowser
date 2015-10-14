/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

BrowerShell implementation as a browser.
*/

#include "include/BrowserShell.h"
#include "include/BrowserLoggerFactory.h"
#include "include/BrowserView.h"
#include "include/BrowserViewGroup.h"
#include "include/BrowserPlatform.h"
#include "include/BrowserQueueManager.h"
#include "include/BrowserMessageManager.h"
#include "include/BrowserTaskMonitor.h"
#include "include/platform/BrowserResourceLoaderBridge.h"
#include "LightBrowser.h"
#include "base/at_exit.h"
#include "base/bind.h"
#include "base/logging.h"
#include "base/i18n/icu_util.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop.h"
#include "base/string_util.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "base/time.h"
#include "base/timer.h"
#include "base/command_line.h"
#include "v8/include/v8.h"
#include "net/http/http_cache.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/user_agent/user_agent.h"
#include "webkit/user_agent/user_agent_util.h"

namespace LightBrowser {

class BrowserMainThread : public base::Thread {
public:
    BrowserMainThread()
        : base::Thread("BrowserMainThread")
        , m_shellOption(new BrowserShell::ShellOption)
    {
        // nothing to do
    }

    ~BrowserMainThread() { Stop(); }

    virtual void Init();
    virtual void CleanUp();

    BrowserShell::ShellOption *m_shellOption;
};

void BrowserMainThread::Init()
{
    BrowserShell::createOnUIThread(*m_shellOption);
    delete m_shellOption;
    m_shellOption = NULL;
}

void BrowserMainThread::CleanUp()
{
    delete BrowserShell::s_browserPlatform;
    BrowserShell::s_browserPlatform = NULL;
}

class BrowserReceiverThread : public base::Thread {
public:
    BrowserReceiverThread()
        : base::Thread("BrowserReceiverThread")
    {
        // nothing to do
    }

    ~BrowserReceiverThread() { Stop(); }

    virtual void Init() {}
    virtual void CleanUp() {}
};

class BrowserWorkerThread : public base::Thread {
public:
    BrowserWorkerThread()
        : base::Thread("BrowserWorkerThread")
    {
        // nothing to do
    }

    ~BrowserWorkerThread() { Stop(); }

    virtual void Init() {}
    virtual void CleanUp() {}
};


BrowserMainThread *BrowserShell::s_browserMainThread = NULL;
BrowserReceiverThread *BrowserShell::s_browserReceiverThread = NULL;
BrowserWorkerThread *BrowserShell::s_browserWorkerThread = NULL;
BrowserShell *BrowserShell::s_browserShell = NULL;
base::AtExitManager *BrowserShell::s_atExitManager = NULL;
base::WaitableEvent *BrowserShell::s_waitableEvent = NULL;
BrowserPlatform *BrowserShell::s_browserPlatform = NULL;

BrowserShell::BrowserShell(const ShellOption &option)
    : m_browserViewGroup(NULL)
    , m_newTabBrowserViewGroup(NULL) 
    , m_queueManager(NULL)
    , m_messageManager(NULL)
    , m_browserTaskMonitor(NULL)
    , m_cachePath(option.m_cachePath)
    , m_networkProxy(option.m_networkProxy)
    , m_cacheMode(option.m_cacheMode)
{
    // CompleteMe: apply options.
    initialize();

    m_browserViewGroup = createBrowserViewGroup(static_cast<BrowserViewGroup::BVGroupType>(option.m_type), option.m_viewCount);
    if (option.m_type != NEW_TAB)
        m_newTabBrowserViewGroup = createBrowserViewGroup(BrowserViewGroup::BVGROUP_NOLIMITED_TAB);
    
    m_messageManager = new BrowserMessageManager;
    m_queueManager = new BrowserQueueManager(option.m_queueType);
    m_queueManager->AddRef();

    m_browserTaskMonitor = new BrowserTaskMonitor(v8::Isolate::GetCurrent(), option.m_longTaskTime);
    m_browserTaskMonitor->AddRef();
}

BrowserShell::~BrowserShell()
{
    delete m_browserViewGroup;
    delete m_newTabBrowserViewGroup;
    delete m_messageManager;

    m_queueManager->willDelete();
    m_queueManager->Release();
    s_browserMainThread->message_loop()->RemoveTaskObserver(
        static_cast<MessageLoop::TaskObserver*>(m_browserTaskMonitor));
    m_browserTaskMonitor->Release();

    if (s_browserShell == this)
        s_browserShell = NULL;
    else
        exit(1);// crash.

    if (s_waitableEvent)
        s_waitableEvent->Signal();

    UI_LOGGER()->debug() << "Browser shell has been destroyed." << std::endl;
}

BrowserShell *BrowserShell::create(const ShellOption &option)
{    
    if (s_browserShell)
        return s_browserShell;

    s_atExitManager = new base::AtExitManager;
    CommandLine::Init(0, NULL);

    // initial logger system.
    BrowserLoggerFactory::setBrowserLoggerImplType(option.m_loggerImplType);
    UI_LOGGER()->setLoggerLevel(option.m_systemLoggerLevel);
    WORKER_LOGGER()->setLoggerLevel(option.m_systemLoggerLevel);
    RECEIVER_LOGGER()->setLoggerLevel(option.m_systemLoggerLevel);

    base::Thread::Options options;
    options.message_loop_type = MessageLoop::TYPE_UI;
    
    s_browserMainThread = new BrowserMainThread;
    *s_browserMainThread->m_shellOption= option;
    s_browserMainThread->StartWithOptions(options);
    s_browserShell->initilaOnUIThreadAfterCreateMessageLoop();

    options.message_loop_type = MessageLoop::TYPE_DEFAULT;
    s_browserWorkerThread = new BrowserWorkerThread;
    s_browserWorkerThread->StartWithOptions(options);

    s_browserReceiverThread = new BrowserReceiverThread;
    s_browserReceiverThread->StartWithOptions(options);

    // firstTime.
    s_browserShell->m_browserTaskMonitor->start();
    s_browserShell->tryToIncreaseBrowserView(1);
    if (option.m_type == FIXED_TAB) {
        for (int i=0; i<4; i++)
            s_browserShell->tryToIncreaseBrowserView(1);
    }

    return s_browserShell;
}

BrowserShell *BrowserShell::createOnUIThread(const ShellOption &option)
{
    if (s_browserShell)         
        exit(1); // crash.

    UI_LOGGER()->debug() << "Create browser shell on UI thread." 
        << "ShellType:" << option.m_type
        << ", ViewCount:" << option.m_viewCount
        << ", QueueType:" << option.m_queueType
        << ", LoggerImplType:" << option.m_loggerImplType
        << ", SystemLoggerLevel:" << option.m_systemLoggerLevel
        << ", LongTaskTime:" << option.m_longTaskTime
        << ", CacheMode:" << option.m_cacheMode
        << ", CachePath:" << option.m_cachePath
        << ", NetProxy:" << option.m_networkProxy
        << std::endl;

    s_browserShell = new BrowserShell(option);
    s_browserShell->AddRef();
    return s_browserShell;
}

void BrowserShell::initialize()
{
    icu_util::Initialize();

    webkit_glue::SetJavaScriptFlags("--expose-gc");

    s_browserPlatform = new BrowserPlatform;

    BrowserResourceLoaderBridge::initialize(
        FilePath(FilePath::StringType(m_cachePath.begin(), m_cachePath.end())), 
        static_cast<net::HttpCache::Mode>(m_cacheMode),
        m_networkProxy);

    webkit_glue::SetUserAgent(webkit_glue::BuildUserAgentFromProduct(
        "Mqq"), false);
    //webkit_glue::SetUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 7_0 like Mac OS X; en-us) AppleWebKit/537.51.1 (KHTML, like Gecko) Version/7.0 Mobile/11A465 Safari/9537.53", false);
}


void BrowserShell::load(const BrowserRequest &request)
{
    // put current request into queue and return.
    m_queueManager->addRequestToQueue(request);
    return;
}

void BrowserShell::destroy()
{
    if (!s_waitableEvent)
        s_waitableEvent = new base::WaitableEvent(false, false);
    s_browserMainThread->message_loop()->PostTask(FROM_HERE,
        base::Bind(&BrowserShell::asyncDestroy, this));

    //m_event.TimedWait(base::TimeDelta::FromMilliseconds(1000));
    s_waitableEvent->Wait();
    delete s_waitableEvent;
    s_waitableEvent = NULL;

    delete s_browserWorkerThread;
    s_browserWorkerThread = NULL;

    delete s_browserMainThread;
    s_browserMainThread = NULL;

    delete s_browserReceiverThread;
    s_browserReceiverThread = NULL;

    delete s_atExitManager;
    s_atExitManager = NULL;
}

void BrowserShell::asyncDestroy()
{
    UI_LOGGER()->debug() << "Browser shell will be destroy." << std::endl;
    this->Release();
}

void BrowserShell::initilaOnUIThreadAfterCreateMessageLoop()
{
    s_browserMainThread->message_loop()->PostTask(FROM_HERE,
        base::Bind(&BrowserShell::addUITaskObserver, this));
}

void BrowserShell::tryToIncreaseBrowserView(size_t queueSize)
{
    if (queueSize <= 0)
        return;

    if (m_browserViewGroup->canGetMoreBrowserView()) {
        // do increse;
        s_browserMainThread->message_loop()->PostTask(FROM_HERE,
            base::Bind(&BrowserShell::increaseBrowserView, this, m_browserViewGroup));
        return;
    }

    if (canCreateNewTab(queueSize)) {
        s_browserMainThread->message_loop()->PostTask(FROM_HERE,
            base::Bind(&BrowserShell::increaseBrowserView, this, m_newTabBrowserViewGroup));
        return;
    }
}

void BrowserShell::addUITaskObserver()
{
    s_browserMainThread->message_loop()->AddTaskObserver(
        static_cast<MessageLoop::TaskObserver*>(m_browserTaskMonitor));
}

void BrowserShell::increaseBrowserView(BrowserViewGroup *viewGroup)
{
    UI_LOGGER()->debug() << "Try to increase Browser View." << std::endl;
    BrowserView *view = viewGroup->getBrowserView();
    if (view)
        m_queueManager->tryToLoadNextRequest(view);
}

bool BrowserShell::canCreateNewTab(size_t queueSize)
{
    // FixMe: check config or system running state.
    size_t current = m_browserViewGroup->getBrowserViewSize()+m_newTabBrowserViewGroup->getBrowserViewSize();
    RECEIVER_LOGGER()->debug() << "Judge can create New tab, queue size:" << queueSize
        << ", view size:" << current << std::endl;
    if (queueSize > (3*current))
        return true;
    return false;
}

void BrowserShell::notifyStopLoad(void *view, void *p)
{
    BrowserMessage msg = { 0 };
    msg.m_message = BrowserMessageManager::E_STOPLOADING_MESSAGE;
    msg.m_callbackObject = view;
    msg.m_parameter = p;

    WORKER_LOGGER()->debug() << "Notify stop load on work thread, obj:" << view
        << ", p:" << p << std::endl;
    m_messageManager->postMessage(msg);
}

MessageLoop *BrowserShell::getMainThread()
{
    return s_browserMainThread->message_loop();
}

MessageLoop *BrowserShell::getReceiverThread()
{
    return s_browserReceiverThread->message_loop();
}

MessageLoop *BrowserShell::getWorkerThread()
{
    return s_browserWorkerThread->message_loop();
}

BrowserShell *BrowserShellFactory::CreateBrowser(BrowserShell::ShellOption &option)
{
    return BrowserShell::create(option);
}

void BrowserShellFactory::BrowserLoad(BrowserShell *shell, BrowserRequest &request)
{
    if (shell)
        shell->load(request);
}

void BrowserShellFactory::ReleaseBrowser(BrowserShell *shell)
{
    if (shell)
        shell->destroy(); // Release BrowserShell
}

}