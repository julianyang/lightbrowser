/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/
#include <vector>
#include "include/BrowserView.h"
#include "LightBrowser.h"
#include "include/BrowserLoggerFactory.h"
#include "include/BrowserShellProxy.h"
#include "include/BrowserViewRequestProxy.h"
#include "include/BrowserViewRequestContext.h"
#include "include/BrowserViewRequestMonitor.h"
#include "include/BrowserViewObserver.h"
#include "include/BrowserQueueManager.h"
#include "include/WebViewHost.h"
#include "include/BrowserViewDelegate.h"
#include "base/utf_string_conversions.h"
#include "base/file_util.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebSize.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebSettings.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebURLRequest.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebData.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebURL.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebString.h"
#include "googleurl/src/gurl.h"
#include "webkit/glue/webpreferences.h"

namespace LightBrowser {

static const int g_estimatedReciveCost = 200; // 100 ms
static const int g_estimatedResponseCost = 100; // 100 ms

webkit_glue::WebPreferences *BrowserView::m_webPreferences = NULL;

BrowserView::BrowserView()
    : m_delegate(new BrowserViewDelegate(this))
    , m_requestProxy(new BrowserViewRequestProxy(this,
        BrowserShellProxy::getMainThreadForCurrentShell(), 
        BrowserShellProxy::getWorkerThreadForCurrentShell()))
    , m_requestMonitor(new BrowserViewRequestMonitor(this))
    , m_webviewHost(NULL)
    , m_isloading(false)
    , m_identifier(-1)
{
    m_requestProxy->AddRef();
    m_requestMonitor->AddRef();
}

BrowserView::~BrowserView()
{
    // set request context empty, make sure not do response for pre-request.
    m_requestMonitor->stop();
    m_requestProxy->setRequestContext(NULL);
    webView()->mainFrame()->loadRequest(WebKit::WebURLRequest(WebKit::WebURL(GURL("about:blank"))));
    
    callJavaScriptGC();
    callJavaScriptGC();

    if (m_webviewHost) {
        m_webviewHost->close();
        delete m_webviewHost;
    }

    m_requestMonitor->Release();
    m_requestProxy->Release();
    delete m_delegate;
}

void BrowserView::initialize()
{
    // do something according to the specific platform.
    m_webviewHost = new WebViewHost(m_delegate);
    // CompleteMe: use default websettings.
    m_webviewHost->webView()->initializeMainFrame(m_delegate);

    m_webviewHost->sizeToDefault();
}

void BrowserView::addObserver(BrowserViewObserver *observer)
{
    if (observer)
        m_observerList.push_back(observer);
}

void BrowserView::removeObserver(BrowserViewObserver *observer)
{
    if (observer)
        m_observerList.remove(observer);
}

void BrowserView::setIsLoading(bool value)
{
    m_isloading = value;
}

bool BrowserView::isLoading()
{
    return m_isloading;
}

bool BrowserView::createNewWindow(std::string &url, BrowserView **view)
{
    BrowserView *t = new BrowserView();
    if (!t) 
        return false;

    t->initialize();
    // CompleteMe:: add current browserview into window list.
    *view = t;

    return true;
}

WebKit::WebView *BrowserView::createWebView()
{
    BrowserView *view = NULL;
    std::string blank;
    if (!createNewWindow(blank, &view))
        return NULL;
    return view->webView();
}

void BrowserView::didInvalidateRect(int x, int y, int w, int h)
{
    m_webviewHost->didInvalidateRect(x, y, w, h);
}

void BrowserView::didScrollRect(int dx, int dy, int x, int y, int w, int h)
{
    m_webviewHost->didScrollRect(dx, dy, x, y, w, h);
}

void BrowserView::setFocus(bool enable)
{
    m_webviewHost->setFocus(enable);
}

void BrowserView::show()
{
    m_webviewHost->show();
}

void BrowserView::setWindowRect(int x, int y, int w, int h)
{
    m_webviewHost->setWindowRect(x, y, w, h);
}

void BrowserView::windowRect(int &x, int &y, int &w, int &h)
{
    m_webviewHost->windowRect(x, y, w, h);
}

void BrowserView::rootWindowRect(int &x, int &y, int &w, int &h)
{
    m_webviewHost->rootWindowRect(x, y, w, h);
}

void BrowserView::close()
{
    m_webviewHost->close();
}

void BrowserView::stopLoad(int id)
{
    UI_LOGGER()->debug() << "stop load, id:" << id << ", identifier:" << m_identifier << std::endl;
    if (id != m_identifier)
        return;
    
    m_delegate->setTimeout();

    if (webView())
        webView()->mainFrame()->stopLoading();

    if (!m_delegate->isDoResponse()) {
        UI_LOGGER()->error() << "stop load, id:" << id << ", identifier:" << m_identifier << "not do response!" << std::endl;
        m_delegate->doResponseError(BrowserResponse::E_CANCEL);
    }
}

void BrowserView::prepareDoNextRequest()
{
    m_requestProxy->setRequestContext(NULL);
    webView()->mainFrame()->loadRequest(WebKit::WebURLRequest(WebKit::WebURL(GURL("about:blank"))));

    if (!m_observerList.size()) {
        UI_LOGGER()->debug() << "Load next request on UI thread, view:" << this
            << ", old id:" << m_identifier << std::endl;
        BrowserShellProxy::getBrowserQueueManager()->tryToLoadNextRequest(this);
    } else {
        UI_LOGGER()->debug() << "Current View will be destroy, view:" << this
            << ", old id:" << m_identifier << std::endl;
        BVObserverList::iterator it = m_observerList.begin();
        if (it != m_observerList.end()) 
            (*it)->finishLoad(this);
    }
}

void BrowserView::load(const BrowserRequest &request)
{
    if (!webView())
        return;

    m_identifier++;
    m_delegate->reset();
    m_requestProxy->setRequestContext(new BrowserViewRequestContext(request));
    m_requestMonitor->setNextFireInterval(request.m_timeout-(g_estimatedReciveCost+g_estimatedResponseCost), m_identifier);
    // CompleteMe: reset websettings according to request.
    resetWebSettings(request);

    setWindowRect(0, 0, request.m_screenWidth, request.m_screenHeight);

    WebKit::WebFrame *frame = webView()->mainFrame();

    switch (request.m_loadType) {
    case BrowserRequest::LOAD_DEFAULT: 
        {
            UI_LOGGER()->debug() << "Load a new request, view:" << this
                << ", id:" << m_identifier 
                << ", url:" << request.m_url
                << std::endl;
            WebKit::WebURLRequest urlRequest;
            contructWebURLRequest(urlRequest, request);
            frame->loadRequest(urlRequest);
        }
        break;
    case BrowserRequest::LOAD_MHTSTRING: 
        {
            UI_LOGGER()->debug() << "Load a new request for MHTString, view:" << this
                << ", id:" << m_identifier 
                << std::endl;
            WebKit::WebData data(request.m_loadData.c_str(), request.m_loadData.length());
            WebKit::WebString mimeType = WebKit::WebString::fromUTF8("multipart/related");
            WebKit::WebURL baseURL(GURL(request.m_url));
            if (request.m_url == "")
                baseURL = GURL("file:///a.mht");
            m_delegate->subResourceForbidden();
            frame->loadData(data, mimeType, WebKit::WebString(), baseURL);
        }
        break;
    case BrowserRequest::LOAD_HTMLSTRING:
        {
            UI_LOGGER()->debug() << "Load a new request for HTMLString, view:" << this
                << ", id:" << m_identifier 
                << std::endl;
            WebKit::WebData data(request.m_loadData.c_str(), request.m_loadData.length());
            WebKit::WebURL baseURL(GURL(request.m_url));
            if (!request.m_settings.m_enableLoadSubresources)
                m_delegate->subResourceForbidden();
            frame->loadHTMLString(data, baseURL);
        } 
        break;
    default:
        break;
    }

    if (!m_isloading && !m_delegate->isDoResponse()) {
        UI_LOGGER()->error() << "load error, identifier:" << m_identifier << "not do response!" << std::endl;
        m_delegate->doResponseError(BrowserResponse::E_UNKNOWN);
    }
}

void BrowserView::goBackOrForward(int offset)
{
    if (!webView())
        return;

    // CompleteMe: handle navigator history list.
}

void BrowserView::reload()
{
    if (!webView())
        return;

    webView()->mainFrame()->reload(false);
}

BrowserViewRequestProxy *BrowserView::requestProxy()
{
    return m_requestProxy;
}

void BrowserView::stopMonitor()
{
    m_requestMonitor->stop();
}

void BrowserView::paint(int width, int height, skia::PlatformCanvas *canvas)
{
    UI_LOGGER()->debug() << "paint rect:" << width << "," << height << std::endl;
    m_webviewHost->paint(canvas, width, height);
}

void BrowserView::resetWebSettings(const BrowserRequest &request)
{
    // CompleteMe: handle websettings according to request.
    if (!m_webPreferences)
        m_webPreferences = new webkit_glue::WebPreferences;
    // reset webPreferences.
    *m_webPreferences = webkit_glue::WebPreferences();

    m_webPreferences->serif_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
        ASCIIToUTF16("times new roman");

    m_webPreferences->cursive_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
        ASCIIToUTF16("Comic Sans MS");
    m_webPreferences->fantasy_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
        ASCIIToUTF16("Impact");
    m_webPreferences->standard_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
        m_webPreferences->serif_font_family_map[webkit_glue::WebPreferences::kCommonScript];
    m_webPreferences->fixed_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
        ASCIIToUTF16("Courier");
    m_webPreferences->sans_serif_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
        ASCIIToUTF16("Helvetica");

    m_webPreferences->javascript_enabled = request.m_settings.m_enableJavaScript;
    m_webPreferences->plugins_enabled = request.m_settings.m_enablePlugin;
    m_webPreferences->images_enabled = request.m_settings.m_enableImage;
    m_webPreferences->default_encoding = "ISO-8859-1";
    m_webPreferences->default_font_size = 16;
    m_webPreferences->default_fixed_font_size = 13;
    m_webPreferences->minimum_font_size = 0;
    m_webPreferences->minimum_logical_font_size = 9;
    m_webPreferences->javascript_can_open_windows_automatically = false;
    m_webPreferences->dom_paste_enabled = true;
    m_webPreferences->developer_extras_enabled = false;
    m_webPreferences->site_specific_quirks_enabled = true;
    m_webPreferences->shrinks_standalone_images_to_fit = false;
    m_webPreferences->uses_universal_detector = false;
    m_webPreferences->text_areas_are_resizable = false;
    m_webPreferences->java_enabled = false;
    m_webPreferences->allow_scripts_to_close_windows = false;
    m_webPreferences->javascript_can_access_clipboard = true;
    m_webPreferences->xss_auditor_enabled = false;
    m_webPreferences->remote_fonts_enabled = true;
    m_webPreferences->local_storage_enabled = true;
    m_webPreferences->application_cache_enabled = true;
    m_webPreferences->databases_enabled = false;
    m_webPreferences->allow_file_access_from_file_urls = true;
    m_webPreferences->tabs_to_links = false;
    m_webPreferences->accelerated_2d_canvas_enabled = false;
    m_webPreferences->accelerated_compositing_enabled = false;
    m_webPreferences->visual_word_movement_enabled = false;
    m_webPreferences->touch_enabled = true; // added by julianyang@2015/10/14, support touch event.

    m_webPreferences->Apply(webView());
}

bool BrowserView::contructWebURLRequest(WebKit::WebURLRequest &destRequest, const BrowserRequest &request)
{
    // CompleteMe: construct WebURLRequest according to request.
    static WebKit::WebString httpGet = WebKit::WebString::fromUTF8("GET");
    static WebKit::WebString httpPost = WebKit::WebString::fromUTF8("POST");
    static WebKit::WebString httpHead = WebKit::WebString::fromUTF8("HEAD");
    static WebKit::WebString httpPut = WebKit::WebString::fromUTF8("PUT");

    static WebKit::WebString userAgent = WebKit::WebString::fromUTF8("UserAgent");
    static WebKit::WebString referer = WebKit::WebString::fromUTF8("Referer");
    static WebKit::WebString contentType = WebKit::WebString::fromUTF8("Content-Type");
    static WebKit::WebString contentLength = WebKit::WebString::fromUTF8("Content-Length");
    // default post content-type is application/x-www-form-urlencoded.
    static WebKit::WebString defaultContentType = WebKit::WebString::fromUTF8("application/x-www-form-urlencoded");

    destRequest.initialize();
    destRequest.setURL(WebKit::WebURL(GURL(request.m_url)));

    // handle http header.
    if (!request.m_httpRequestHeader.empty()) {
        std::istringstream headerIn(request.m_httpRequestHeader);
        WebKit::WebString name;
        WebKit::WebString value;
        char line[4096] = { 0 };
        while (headerIn.getline(line, sizeof(line))) {
            const char *pos = strchr(line, ':');
            if (pos) {
                name = WebKit::WebString::fromUTF8(line, (pos-line));
                value = WebKit::WebString::fromUTF8(pos+1);

                // the key cann't be null or empty and the value cann't be null
                if (!((name.data()==NULL) || (name.length()==0) || (value.data()==NULL)))
                    destRequest.addHTTPHeaderField(name, value);
            }  
            memset(line, 0x00, sizeof(line));
        }
    }

    if (!request.m_userAgent.empty()) {
        destRequest.setHTTPHeaderField(userAgent,
            WebKit::WebString::fromUTF8(request.m_userAgent.c_str()));
    }

    if (!request.m_referer.empty()) {
        destRequest.setHTTPHeaderField(referer,
            WebKit::WebString::fromUTF8(request.m_referer.c_str()));
    }

    // handle http method and http body.
    switch (request.m_httpRequestMethod) {
    case BrowserRequest::HTTP_POST:
        destRequest.setHTTPMethod(httpPost);
        if (!request.m_httpRequestBody.empty()) {
            // CompleteMe: add http body here.
            //webkit::npapi::WebPluginImpl::SetPostData(&destRequest, 
                //request.m_httpRequestBody.c_str(), request.m_httpRequestBody.length());
            if (destRequest.httpHeaderField(contentType).isEmpty())
                destRequest.setHTTPHeaderField(contentType, defaultContentType);
            WebKit::WebHTTPBody httpBody;
            httpBody.initialize();
            httpBody.appendData(WebKit::WebData(request.m_httpRequestBody.c_str(), request.m_httpRequestBody.length()));
            destRequest.setHTTPBody(httpBody);
        }
        break;
    case BrowserRequest::HTTP_HEAD:
        destRequest.setHTTPMethod(httpHead);
        break;
    case BrowserRequest::HTTP_PUT:
        destRequest.setHTTPMethod(httpPut);
        break;
    case BrowserRequest::HTTP_GET:
    default:
        destRequest.setHTTPMethod(httpGet);
        destRequest.clearHTTPHeaderField(contentLength);
        break;
    }

    return true;
}

void BrowserView::callJavaScriptGC()
{
    if (!webView())
        return;

    webView()->mainFrame()->collectGarbage();
}

WebKit::WebView *BrowserView::webView()
{
    return m_webviewHost ? m_webviewHost->webView() : NULL;
}

}