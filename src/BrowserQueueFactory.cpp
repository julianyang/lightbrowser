/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

SimpleBrowerQueue implementation as a browser request queue.
*/

#include "include/BrowserQueue.h"
#include "include/SimpleBrowserQueue.h"

namespace LightBrowser {

BrowserQueue *createBrowserQueue(int type) 
{
    BrowserQueue *bq = NULL;

    switch (type) {
    // CompleteMe: create browser queue according to type.
    case 0:
    default:
        bq = SimpleBrowserQueue::create();
        break;
    }

    return bq;
}

}