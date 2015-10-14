/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef WEBVIEW_HOST
#define WEBVIEW_HOST

#include "skia/ext/platform_canvas.h"

namespace WebKit {
    class WebViewClient;
    class WebView;
    class WebWidget;
}

namespace LightBrowser {

class WebViewHost {
public:
    explicit WebViewHost(WebKit::WebViewClient *client);
    virtual ~WebViewHost();

    virtual void didInvalidateRect(int x, int y, int w, int h);
    virtual void didScrollRect(int dx, int dy, int x, int y, int w, int h);
    virtual void ScheduleComposite();
    virtual void ScheduleAnimation();
    virtual void discardBackingStore();
    // Allow clients to update the paint rect. For example, if we get a gdk
    // expose or WM_PAINT event, we need to update the paint rect.
    virtual void updatePaintRect(int x, int y, int w, int h);
    virtual void setFocus(bool enable);
    virtual void show();
    virtual void setWindowRect(int x, int y, int w, int h);
    virtual void windowRect(int &x, int &y, int &w, int &h);
    virtual void rootWindowRect(int &x, int &y, int &w, int &h);
    virtual void close();
    virtual void paint(skia::PlatformCanvas *canvas, int w, int h);
    WebKit::WebView *webView();
    void sizeToDefault();

protected:
    //CompleteMe: add platform window handle here.
    void *m_handle;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    WebKit::WebWidget *m_webWidget;
    const static int s_defaultWindowWidth = 1024;
    const static int s_defaultWindowHeight = 768;
};

}

#endif