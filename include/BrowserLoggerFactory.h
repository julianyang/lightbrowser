/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_LOGGER_FACTORY
#define BROWSER_LOGGER_FACTORY

#include "base/basictypes.h"
#include "include/BrowserLogger.h"

namespace LightBrowser {

class BrowserLoggerFactory {
public:
    static void setBrowserLoggerImplType(int type);
    static BrowserLogger *createBrowserRequestLogger();
    static BrowserLogger *getBrowserUIThreadLogger();
    static BrowserLogger *getBrowserWorkerThreadLogger();
    static BrowserLogger *getBrowserReceiverThreadLogger();

private:
    BrowserLoggerFactory();
    DISALLOW_COPY_AND_ASSIGN(BrowserLoggerFactory);

    static int s_browserLoggerImplType;
    static BrowserLogger *s_ui_logger;
    static BrowserLogger *s_worker_logger;
    static BrowserLogger *s_receiver_logger;
};

#define UI_LOGGER() (BrowserLoggerFactory::getBrowserUIThreadLogger())
#define WORKER_LOGGER() (BrowserLoggerFactory::getBrowserWorkerThreadLogger())
#define RECEIVER_LOGGER() (BrowserLoggerFactory::getBrowserReceiverThreadLogger())

}

#endif