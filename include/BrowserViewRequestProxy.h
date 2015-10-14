/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_VIEW_REQUEST_PROXY
#define BROWSER_VIEW_REQUEST_PROXY

#include <string>
#include "base/memory/ref_counted.h"
#include "skia/ext/platform_canvas.h"

class MessageLoop;

namespace LightBrowser {
class BrowserView;
class BrowserViewRequestContext;
class BrowserResponse;

class BrowserViewRequestProxy : public base::RefCountedThreadSafe<BrowserViewRequestProxy> {
public:
    explicit BrowserViewRequestProxy(BrowserView *view, MessageLoop *main, MessageLoop *worker);

    ~BrowserViewRequestProxy();

    void setRequestContext(BrowserViewRequestContext *context);
    BrowserViewRequestContext *getContext() const { return m_context; }

    // add public async work here.
    void prepareDoResponse();

private:
    bool needDoResponse();
    void doResponseOnMainThread();
    void dumpDocumentTree(std::string &domTree);
    void dumpRenderTree(std::string &renderInfo);
    void doPaint(skia::PlatformCanvas *canvas);

    struct IndependentResponse {
        IndependentResponse ()
            : m_callback(NULL)
            , m_callbackData(NULL)
            , m_canvas(NULL)
            , m_response(NULL)
        {
       
        }

        ~IndependentResponse();

        void *m_callback;
        void *m_callbackData;
        skia::PlatformCanvas *m_canvas;
        std::string m_renderInfo;
        BrowserResponse *m_response;

    private:
        // can not copy and assign.
        IndependentResponse(const IndependentResponse&);
        IndependentResponse & operator=(const IndependentResponse &);
    };

    void doResponseOnWorkerThread(IndependentResponse *response);

    BrowserView *m_view;
    MessageLoop *m_mainThread;
    MessageLoop *m_workerThread;
    BrowserViewRequestContext *m_context;
};

}

#endif