/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

BrowerShell implementation as a browser.
*/

#ifndef BROWSER_SHELL
#define BROWSER_SHELL

#include <string>
#include "base/memory/ref_counted.h"

namespace base {
    class AtExitManager;
    class WaitableEvent;
}

class MessageLoop;

namespace LightBrowser {

class BrowserRequest;
class BrowserView;
class BrowserViewGroup;
class BrowserPlatform;
class BrowserMainThread;
class BrowserReceiverThread;
class BrowserWorkerThread;
class BrowserQueueManager;
class BrowserMessageManager;
class BrowserTaskMonitor;

class BrowserShell : public base::RefCountedThreadSafe<BrowserShell> {
public:
    enum {
        SINGLE_TAB = 0,
        FIXED_TAB,
        NEW_TAB
    };

    struct ShellOption {
        int m_type;
        int m_viewCount;
        int m_queueType;
        int m_loggerImplType;
        int m_systemLoggerLevel;
        int m_longTaskTime;
        int m_cacheMode;
        std::string m_cachePath;
        std::string m_networkProxy;
    };

    ~BrowserShell();

    static BrowserShell *create(const ShellOption &option);
    
    void load(const BrowserRequest &request);
    void destroy();

private:
    friend class BrowserMainThread;
    friend class BrowserShellProxy;

    static BrowserShell *createOnUIThread(const ShellOption &option);

    BrowserShell(const ShellOption &option);

    void initialize();
    void asyncDestroy();
    void initilaOnUIThreadAfterCreateMessageLoop();
    void tryToIncreaseBrowserView(size_t);
    void addUITaskObserver();
    void increaseBrowserView(BrowserViewGroup *viewGroup);
    bool canCreateNewTab(size_t);
    void notifyStopLoad(void *view, void *p);

    static MessageLoop *getMainThread();
    static MessageLoop *getReceiverThread();
    static MessageLoop *getWorkerThread();

    static BrowserMainThread *s_browserMainThread;
    static BrowserReceiverThread *s_browserReceiverThread;
    static BrowserWorkerThread *s_browserWorkerThread;
    static BrowserShell *s_browserShell;
    static base::AtExitManager *s_atExitManager;
    static base::WaitableEvent *s_waitableEvent;

    static BrowserPlatform *s_browserPlatform;
    BrowserViewGroup *m_browserViewGroup;
    BrowserViewGroup *m_newTabBrowserViewGroup; // used for deal with delayed request.

    BrowserQueueManager *m_queueManager;
    BrowserMessageManager *m_messageManager;
    BrowserTaskMonitor *m_browserTaskMonitor;
    std::string m_cachePath;
    std::string m_networkProxy;
    int m_cacheMode;
};

class BrowserShellFactory {
public:
    static BrowserShell *CreateBrowser(BrowserShell::ShellOption &option);
    static void BrowserLoad(BrowserShell *shell, BrowserRequest &request);
    static void ReleaseBrowser(BrowserShell *shell);
};

}

#endif