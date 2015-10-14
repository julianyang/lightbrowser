/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_SHELL_PROXY
#define BROWSER_SHELL_PROXY

#include "base/basictypes.h"
class MessageLoop;

namespace LightBrowser {

class BrowserQueueManager;

class BrowserShellProxy {
public:    
    static MessageLoop *getMainThreadForCurrentShell();
    static MessageLoop *getReceiverThreadForCurrentShell();
    static MessageLoop *getWorkerThreadForCurrentShell();
    static BrowserQueueManager *getBrowserQueueManager();
    static void tryToIncreaseBrowserView(size_t);
    static void notifyStopLoad(void *view, void *p);

private:
    BrowserShellProxy();
    DISALLOW_COPY_AND_ASSIGN(BrowserShellProxy);
};

}

#endif