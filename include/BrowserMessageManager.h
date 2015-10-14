/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_MESSAGE_MANAGER
#define BROWSER_MESSAGE_MANAGER

#include "base/compiler_specific.h"

#if defined(OS_WIN)
    #include <windows.h>
#elif defined(OS_POSIX) 
    #include "base/memory/scoped_ptr.h"
#else
    #error "Bad Platfrom for Message manager!"
#endif


#if defined(OS_POSIX)
    typedef struct _GMainContext GMainContext;
    typedef struct _GPollFD GPollFD;
    typedef struct _GSource GSource;
#endif

namespace LightBrowser {

struct BrowserMessage {
    unsigned int m_message;
    void *m_callbackObject;
    void *m_parameter;
};

class BrowserMessageManager {
public:
    enum MessageType {
        E_STOPLOADING_MESSAGE = 11
    };
    BrowserMessageManager();
    ~BrowserMessageManager();

    void postMessage(const BrowserMessage &message);

private:
    void initialMessageHandle();

#if defined(OS_WIN)
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    HWND m_messageHandle;
#elif defined(OS_POSIX) 
public:
    int HandlePrepare();
    bool HandleCheck();
    void HandleDispatch();

private:
    GMainContext *m_context;
    GSource *m_source;
    int m_readFd;
    int m_writeFd;
    scoped_ptr<GPollFD> m_gpollFd;
#endif
};

}

#endif
