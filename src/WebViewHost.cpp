/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include "include/WebViewHost.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebSize.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebRect.h"

namespace LightBrowser {

WebViewHost::WebViewHost(WebKit::WebViewClient *client)
    : m_x(0)
    , m_y(0)
    , m_width(0)
    , m_height(0)
    , m_webWidget(WebKit::WebView::create(client))
{

}

WebViewHost::~WebViewHost()
{
    // FixMe: how to manage webview's life.
    // detouch window handle and browserView.
}

void WebViewHost::didInvalidateRect(int x, int y, int w, int h)
{
    // NotImplementation
}

void WebViewHost::didScrollRect(int dx, int dy, int x, int y, int w, int h)
{
    // NotImplementation
}

void WebViewHost::ScheduleComposite()
{
    // NotImplementation
}

void WebViewHost::ScheduleAnimation()
{
    // NotImplementation
}

void WebViewHost::discardBackingStore()
{
    // NotImplementation
}

void WebViewHost::updatePaintRect(int x, int y, int w, int h)
{
    // NotImplementation
}

void WebViewHost::setFocus(bool enable)
{
    // NotImplementation
}

void WebViewHost::show()
{
    // NotImplementation
}

void WebViewHost::setWindowRect(int x, int y, int w, int h)
{
    m_x = x;
    m_y = y;
    m_width = w;
    m_height = h;
    webView()->resize(WebKit::WebSize(m_width, m_height));
}

void WebViewHost::windowRect(int &x, int &y, int &w, int &h)
{
    x = m_x;
    y = m_y;
    w = m_width;
    h = m_height;
}

void WebViewHost::rootWindowRect(int &x, int &y, int &w, int &h)
{
    x = m_x;
    y = m_y;
    w = m_width;
    h = m_height;
}

void WebViewHost::close()
{
    // NotImplementation
    m_webWidget->close();
}

void WebViewHost::paint(skia::PlatformCanvas *canvas, int w, int h)
{
    static_cast<WebKit::WebView *>(m_webWidget)->layout();
    static_cast<WebKit::WebView *>(m_webWidget)->paint(canvas, WebKit::WebRect(0, 0, w, h));
}

WebKit::WebView *WebViewHost::webView()
{
    return static_cast<WebKit::WebView *>(m_webWidget);
}

void WebViewHost::sizeToDefault()
{
    m_width = s_defaultWindowWidth;
    m_height = s_defaultWindowHeight;
   
    webView()->resize(WebKit::WebSize(m_width, m_height));
}

}