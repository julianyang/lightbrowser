/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

BrowerView implementation as a browser tab.
*/

#ifndef BROWSER_VIEW
#define BROWSER_VIEW

#include <list>
#include <string>
#include "base/memory/scoped_ptr.h"
#include "skia/ext/platform_canvas.h"

namespace WebKit {
    class WebView;
    class WebURLRequest;
}

namespace webkit_glue {
    struct WebPreferences;
}

namespace LightBrowser {

class WebViewHost;
class BrowserViewDelegate;
class BrowserViewObserver;
class BrowserRequest;
class BrowserViewRequestProxy;
class BrowserViewRequestMonitor;

typedef std::list<BrowserViewObserver *> BVObserverList;


class BrowserView {
public:
    ~BrowserView();

    void initialize();

    void addObserver(BrowserViewObserver *observer);
    void removeObserver(BrowserViewObserver *observer);

    void setIsLoading(bool value);
    bool isLoading();

    static bool createNewWindow(std::string &url, BrowserView **view);
    WebKit::WebView *createWebView();

    void didInvalidateRect(int x, int y, int w, int h);
    void didScrollRect(int dx, int dy, int x, int y, int w, int h);
    void setFocus(bool enable);
    void show();
    void setWindowRect(int x, int y, int w, int h);
    void windowRect(int &x, int &y, int &w, int &h);
    void rootWindowRect(int &x, int &y, int &w, int &h);
    void close();

    void stopLoad(int id=0);
    void prepareDoNextRequest();
    void load(const BrowserRequest &request);
    void goBackOrForward(int offset);
    void reload();

    BrowserViewRequestProxy *requestProxy();

private:
    friend class BrowserViewRequestProxy;

    BrowserView();

    void stopMonitor();
    void paint(int width, int height, skia::PlatformCanvas *canvas);
    void resetWebSettings(const BrowserRequest &request);
    bool contructWebURLRequest(WebKit::WebURLRequest &destRequest, const BrowserRequest &request);
    void callJavaScriptGC();
    WebKit::WebView *webView();

    BrowserViewDelegate *m_delegate;
    BrowserViewRequestProxy *m_requestProxy;
    BrowserViewRequestMonitor *m_requestMonitor;
    WebViewHost *m_webviewHost;
    bool m_isloading;
    int m_identifier;
    static webkit_glue::WebPreferences* m_webPreferences;
    BVObserverList m_observerList;
};

}

#endif