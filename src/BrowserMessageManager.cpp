/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include <iostream>
#include "include/BrowserMessageManager.h"
#include "include/BrowserView.h"
#include "include/BrowserLoggerFactory.h"
#include "ui/gfx/native_widget_types.h"
#if defined(OS_WIN)
    #include "ui/base/win/hwnd_util.h"
#elif defined(OS_POSIX) 
    #include <string.h>
    #include "base/logging.h"
    #include "base/posix/eintr_wrapper.h"
    #include <glib.h>
#endif

namespace LightBrowser {

#if defined(OS_WIN)

static const wchar_t kMessageWndClass[] = L"LightBrowser_MessagePumpWindow";
static const unsigned int StopLoadingMessage = WM_USER + BrowserMessageManager::E_STOPLOADING_MESSAGE;

#elif defined(OS_POSIX)

struct MessageWorkSource : public GSource {
    BrowserMessageManager *m_manager;
};

gboolean MessageWorkSourcePrepare(GSource *source, gint *timeout_ms) 
{
    *timeout_ms = static_cast<MessageWorkSource*>(source)->m_manager->HandlePrepare();
    // We always return FALSE, so that our timeout is honored.  If we were
    // to return TRUE, the timeout would be considered to be 0 and the poll
    // would never block.  Once the poll is finished, Check will be called.
    return FALSE;
}

gboolean MessageWorkSourceCheck(GSource *source)
{
    // Only return TRUE if Dispatch should be called.
    return static_cast<MessageWorkSource*>(source)->m_manager->HandleCheck();
}

gboolean MessageWorkSourceDispatch(GSource *source, GSourceFunc unused_func, gpointer unused_data)
{
    static_cast<MessageWorkSource*>(source)->m_manager->HandleDispatch();
    // Always return TRUE so our source stays registered.
    return TRUE;
}

GSourceFuncs MessageWorkSourceFuncs = {
    MessageWorkSourcePrepare,
    MessageWorkSourceCheck,
    MessageWorkSourceDispatch,
    NULL
};

#endif

BrowserMessageManager::BrowserMessageManager() 
#if defined(OS_WIN)
    : m_messageHandle(0)
#elif defined(OS_POSIX) 
    : m_context(NULL)
    , m_source(NULL)
    , m_readFd(-1)
    , m_writeFd(-1)
#endif
{
    initialMessageHandle();
}

BrowserMessageManager::~BrowserMessageManager() 
{
#if defined(OS_WIN)
    DestroyWindow(m_messageHandle);
    UnregisterClass(kMessageWndClass, GetModuleHandle(NULL));
#elif defined(OS_POSIX)
    g_source_destroy(m_source);
    g_source_unref(m_source);
    close(m_readFd);
    close(m_writeFd);
#endif
}

void BrowserMessageManager::initialMessageHandle()
{
#if defined(OS_WIN)
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = BrowserMessageManager::WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = kMessageWndClass;
    RegisterClassEx(&wc);

    m_messageHandle =
        CreateWindow(kMessageWndClass, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, GetModuleHandle(NULL), 0);
#elif defined(OS_POSIX) 
    m_context = g_main_context_default();
    m_gpollFd.reset(new GPollFD);
    int fds[2];
    int ret = pipe(fds);
    DCHECK_EQ(ret, 0);

    m_readFd = fds[0];
    m_writeFd = fds[1];
    m_gpollFd->fd = m_readFd;
    m_gpollFd->events = G_IO_IN;

    m_source = g_source_new(&MessageWorkSourceFuncs, sizeof(MessageWorkSource));
    static_cast<MessageWorkSource*>(m_source)->m_manager = this;
    g_source_add_poll(m_source, m_gpollFd.get());
    g_source_set_priority(m_source, G_PRIORITY_DEFAULT);
    g_source_set_can_recurse(m_source, TRUE);
    g_source_attach(m_source, m_context);
#endif
}

void BrowserMessageManager::postMessage(const BrowserMessage &message)
{
#if defined(OS_WIN) // os Windows

    PostMessage(m_messageHandle, message.m_message + WM_USER,
        reinterpret_cast<WPARAM>(message.m_callbackObject), reinterpret_cast<LPARAM>(message.m_parameter));

#elif defined(OS_POSIX) // os Linux

    char msg[64] = { 0 };
    sprintf(msg, "%d,%p,%p", message.m_message, message.m_callbackObject, message.m_parameter);
    WORKER_LOGGER()->debug() << "send UI message:" << msg << std::endl;
    if (HANDLE_EINTR(write(m_writeFd, msg, sizeof(msg))) != sizeof(msg))
        NOTREACHED() << "Could not write to the UI message loop message pipe!";

#endif
}

#if defined(OS_WIN)
LRESULT CALLBACK BrowserMessageManager::WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {
    case StopLoadingMessage: {
        int *id = reinterpret_cast<int *>(lparam);
        reinterpret_cast<BrowserView *>(wparam)->stopLoad(*id);
        delete id;
        }
        break;
    default:
        break;
    }
    return DefWindowProc(hwnd, message, wparam, lparam);
}
#elif defined(OS_POSIX) 

int BrowserMessageManager::HandlePrepare() 
{
    return -1;
}

bool BrowserMessageManager::HandleCheck()
{
    if (m_gpollFd->revents & G_IO_IN)
    	return true;
    return false;
}

void BrowserMessageManager::HandleDispatch()
{
    char msg[64] = { 0 };
    int num_bytes = HANDLE_EINTR(read(m_readFd, msg, sizeof(msg)));
    if (num_bytes != sizeof(msg)) {
        UI_LOGGER()->error() << "get UI message size error, size:" << num_bytes << std::endl;
        return;
    }
    UI_LOGGER()->debug() << "get UI message:" << msg << ",size:" << num_bytes << std::endl;

    int message = 0;
    void *c = NULL;
    void *p = NULL;    
    sscanf(msg, "%d,%p,%p", &message, &c, &p);

    switch (message) {
    case E_STOPLOADING_MESSAGE: {
        int *pInt = static_cast<int *>(p);
        int id = *pInt;
        static_cast<BrowserView *>(c)->stopLoad(id);
        delete pInt;
        }
        break;
    default:
        break;
    }
}
#endif

}
