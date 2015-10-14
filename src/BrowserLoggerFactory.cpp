/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include "include/BrowserLoggerFactory.h"

namespace LightBrowser {

int BrowserLoggerFactory::s_browserLoggerImplType = 0;
BrowserLogger *BrowserLoggerFactory::s_ui_logger = NULL;
BrowserLogger *BrowserLoggerFactory::s_worker_logger = NULL;
BrowserLogger *BrowserLoggerFactory::s_receiver_logger = NULL;

void BrowserLoggerFactory::setBrowserLoggerImplType(int type)
{
    s_browserLoggerImplType = type;
}

BrowserLogger *BrowserLoggerFactory::createBrowserRequestLogger()
{
    BrowserLogger *bl = NULL;
    switch (s_browserLoggerImplType) {
    case 0:
    default:
        bl = new BrowserLogger(0);
        break;
    }

    return bl;
}

BrowserLogger *BrowserLoggerFactory::getBrowserUIThreadLogger()
{
    if (s_ui_logger)
        return s_ui_logger;

    switch (s_browserLoggerImplType) {
    case 0:
    default:
        s_ui_logger = new BrowserLogger(0);
        break;
    }

    return s_ui_logger;
}

BrowserLogger *BrowserLoggerFactory::getBrowserWorkerThreadLogger()
{
    if (s_worker_logger)
        return s_worker_logger;

    switch (s_browserLoggerImplType) {
    case 0:
    default:
        s_worker_logger = new BrowserLogger(0);
        break;
    }

    return s_worker_logger;
}

BrowserLogger *BrowserLoggerFactory::getBrowserReceiverThreadLogger()
{
    if (s_receiver_logger)
        return s_receiver_logger;

    switch (s_browserLoggerImplType) {
    case 0:
    default:
        s_receiver_logger = new BrowserLogger(0);
        break;
    }

    return s_receiver_logger;
}


}