/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

BrowerViewRequestContext implementation for current BrowserView.
*/

#include <algorithm>
#include "include/BrowserViewRequestContext.h"
#include "include/BrowserLoggerFactory.h"
#include "v8/include/v8.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebHTTPHeaderVisitor.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebDocument.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebString.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebURLResponse.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScriptSource.h"


namespace LightBrowser {

// WebURLResponseVisitor
class WebURLResponseVisitor : public WebKit::WebHTTPHeaderVisitor {
public:
    WebURLResponseVisitor(std::string &responseHeaders)
        :m_responseHeaders(responseHeaders)
    {}

    virtual void visitHeader(const WebKit::WebString& name, const WebKit::WebString& value);

private:
    std::string &m_responseHeaders;
};

void WebURLResponseVisitor::visitHeader(const WebKit::WebString& name, const WebKit::WebString& value)
{
    m_responseHeaders += std::string(name.utf8()) + ":" + std::string(value.utf8()) + "\r\n";
}

static inline void getHeaderLine(std::string &header, const WebKit::WebURLResponse &response)
{
    static std::string sHttp_1_1 = "HTTP/1.1 ";
    static std::string sHttp_1_0 = "HTTP/1.0 ";
    static std::string sHttp_0_9 = "HTTP/0.9 ";

    header = sHttp_1_1; // default is HTTP/1.1.

    switch (response.httpVersion()) {
    case WebKit::WebURLResponse::HTTP_0_9:
        header = sHttp_0_9;
        break;
    case WebKit::WebURLResponse::HTTP_1_0:
        header = sHttp_1_0;
        break;
    case WebKit::WebURLResponse::HTTP_1_1:
        header = sHttp_1_1;
        break;
    default:
        break;
    }

    char statusCode[12] = {0};
    sprintf(statusCode, "%d ", response.httpStatusCode());
    header += statusCode;
    header += std::string(response.httpStatusText().utf8().data());
    header += "\r\n";
}


BrowserViewRequestContext::BrowserViewRequestContext(const BrowserRequest &request)
    : m_request(request)
    , m_browserLooger(BrowserLoggerFactory::createBrowserRequestLogger())
    , m_executeScriptAfterDcoumentCreated(false)
    , m_executeScriptAfterSpecificResourceLoaded(false)
    , m_executeScriptAfterFinishParse(false)
    , m_executeScriptAfterWindowLoaded(false)
    , m_prix("BrowserMainThread|")
{
    // create real context according to browser's request.
    if (m_request.m_extendJavaScript.find(BrowserRequest::EXECUTESCRIPT_AFTER_DOCUMENT_CREATED) 
        != m_request.m_extendJavaScript.end())
        m_executeScriptAfterDcoumentCreated = true;

    if (m_request.m_mapUrlAndExtendJavaScript.size() > 0)
        m_executeScriptAfterSpecificResourceLoaded = true;

    if (m_request.m_extendJavaScript.find(BrowserRequest::EXECUTESCRIPT_AFTER_FINISH_PARSE) 
        != m_request.m_extendJavaScript.end())
        m_executeScriptAfterFinishParse = true;

    if (m_request.m_extendJavaScript.find(BrowserRequest::EXECUTESCRIPT_AFTER_WINDOW_LOADED) 
        != m_request.m_extendJavaScript.end())
        m_executeScriptAfterWindowLoaded = true;

    if (m_request.m_settings.m_enableObserver)
        m_browserLooger->setLoggerLevel(BrowserLogger::E_INFO);
        
    m_response.m_url = m_request.m_url;
}

BrowserViewRequestContext::~BrowserViewRequestContext()
{
    delete m_browserLooger;
}

bool BrowserViewRequestContext::canHandleRequest(const string &url)
{
    m_browserLooger->debug() << __FUNCTION__ << ", url:" << url << std::endl; 
    if (std::find(m_request.m_excludeURL.begin(), m_request.m_excludeURL.end(), url) 
        != m_request.m_excludeURL.end()) {
        if (m_request.m_settings.m_enableObserver) {
            string slog = m_prix + "exclude:" + url;
            m_response.m_observerLogger.push_back(slog);
        }
        m_browserLooger->debug() << __FUNCTION__ << ", exclude url:" << url << std::endl;
        return false;
    }
    return true;
}

bool BrowserViewRequestContext::shouldBeginEditing()
{
    return true;
}

bool BrowserViewRequestContext::shouldEndEditing()
{
    return true;
}

bool BrowserViewRequestContext::shouldInsertNode()
{
    return true;
}

bool BrowserViewRequestContext::shouldInsertText()
{
    return true;
}

bool BrowserViewRequestContext::shouldApplyStyle(const string &style)
{
    return true;
}

bool BrowserViewRequestContext::shouldChangeSelectedRange()
{
    return true;
}

bool BrowserViewRequestContext::shouldDeleteRange()
{
    return true;
}

bool BrowserViewRequestContext::isSmartInsertDeleteEnabled()
{
    return true;
}

bool BrowserViewRequestContext::isSelectTrailingWhitespaceEnabled()
{
    return true;
}

void BrowserViewRequestContext::runModalAlertDialog(const string &message)
{
    m_browserLooger->debug() << __FUNCTION__ << ", message:" << message << std::endl;
    if (m_request.m_settings.m_enableObserver) {
        string slog = m_prix + "alert:" + message;
        m_response.m_observerLogger.push_back(slog);
    }
}

bool BrowserViewRequestContext::runModalConfirmDialog(const string &message)
{
    m_browserLooger->debug() << __FUNCTION__ << ", message:" << message << std::endl;
    if (m_request.m_settings.m_enableObserver) {
        string slog = m_prix + "confirm:" + message;
        m_response.m_observerLogger.push_back(slog);
    }
    return true;
}

bool BrowserViewRequestContext::runModalPromptDialog(const string &message, const string &defaultValue, string &actualValue)
{
    m_browserLooger->debug() << __FUNCTION__ << ", message:" << message << std::endl;
    if (m_request.m_settings.m_enableObserver) {
        string slog = m_prix + "prompt:" + message;
        m_response.m_observerLogger.push_back(slog);
    }

    actualValue = defaultValue;
    return true;
}

bool BrowserViewRequestContext::runModalBeforeUnloadDialog(const string &message)
{
    m_browserLooger->debug() << __FUNCTION__ << ", message:" << message << std::endl;
    if (m_request.m_settings.m_enableObserver) {
        string slog = m_prix + "dialog:" + message;
        m_response.m_observerLogger.push_back(slog);
    }
    return true;
}

bool BrowserViewRequestContext::willSendRequest(string &url, int identifier)
{
    if (std::find(m_request.m_excludeURL.begin(), m_request.m_excludeURL.end(), url) 
        != m_request.m_excludeURL.end()) {
            if (m_request.m_settings.m_enableObserver) {
                string slog = m_prix + "exclude:" + url;
                m_response.m_observerLogger.push_back(slog);
            }
            m_browserLooger->debug() << __FUNCTION__ << ", exclude url:" << url << std::endl;
            return false;
    }

    m_browserLooger->debug() << __FUNCTION__ << ", url:" << url
        << ", identifier:" << identifier << std::endl;
    return true;
}

string BrowserViewRequestContext::userAgent()
{
    m_browserLooger->debug() << __FUNCTION__ << ", " << m_request.m_userAgent << std::endl;
    return m_request.m_userAgent;
}


// request observer.
void BrowserViewRequestContext::didCreateWebView()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didCreateFrame(bool isMainFrame)
{
    m_browserLooger->debug() << __FUNCTION__ << ", isMainFrame:" << isMainFrame << std::endl;
}

void BrowserViewRequestContext::didAddMessageToConsole(string &message)
{
    m_browserLooger->debug() << __FUNCTION__ << ", message:" << message << std::endl;
    if (m_request.m_settings.m_enableObserver) {
        string slog = m_prix + "console:" + message;
        m_response.m_observerLogger.push_back(slog);
    }
}

void BrowserViewRequestContext::didStartLoading()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
    m_response.m_requestResponseHeaders.clear();
    m_response.m_adURLs.clear();
}

void BrowserViewRequestContext::didStopLoading()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didBeginEditing()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didEndEditing()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didChangeSelection()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didChangeContents()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

bool BrowserViewRequestContext::didPaint(int x, int y, int widht, int height)
{
    return true;
}

void BrowserViewRequestContext::didFocus()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didBlur()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didCreatePlugin()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didCreateMediaPlayer()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didCancelClientRedirect()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didCreateDataSource()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didClearWindowObject()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didStartProvisionalLoad()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didReceiveServerRedirectForProvinsionalLoad()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didFailProvisionalLoad(WebKit::WebFrame *frame, int reason, string &message)
{
    m_browserLooger->error() << __FUNCTION__ << ", reason:" << reason
        << ", message" << message << std::endl;
    if (m_request.m_settings.m_enableObserver) {
        string slog = m_prix + "FailProvisionalLoad:" + message;
        m_response.m_observerLogger.push_back(slog);
    }
}

void BrowserViewRequestContext::didCommitProvisionalLoad()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didReceiveDocumentData(const char *data, size_t length)
{
    m_browserLooger->debug() << __FUNCTION__ << ", length:" << length << std::endl;
}

void BrowserViewRequestContext::didReceiveTitle(string &title)
{
    m_browserLooger->debug() << __FUNCTION__ << ", title" << title << std::endl;
}

void BrowserViewRequestContext::didFinishDocumentLoad(WebKit::WebFrame *frame)
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
    if (m_request.m_settings.m_enableExecuteExtendJavaScript
        && m_executeScriptAfterFinishParse) {
        std::string sciptText = m_request.m_extendJavaScript[BrowserRequest::EXECUTESCRIPT_AFTER_FINISH_PARSE];
        if (sciptText != "") {
            if (m_request.m_settings.m_enableObserver)
                m_response.m_observerLogger.push_back(m_prix + "executeScriptAtferFinishParse");
            frame->executeScript(WebKit::WebScriptSource(WebKit::WebString::fromUTF8(sciptText.c_str())));
        }
    }
}

void BrowserViewRequestContext::didHandleOnLoadEvents(WebKit::WebFrame *frame)
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
    if (m_request.m_settings.m_enableExecuteExtendJavaScript
        && m_executeScriptAfterWindowLoaded) {
        std::string sciptText = m_request.m_extendJavaScript[BrowserRequest::EXECUTESCRIPT_AFTER_WINDOW_LOADED];
        if (sciptText != "") {
            if (m_request.m_settings.m_enableObserver)
                m_response.m_observerLogger.push_back(m_prix + "executeScriptAtferWindowLoaded");
            frame->executeScript(WebKit::WebScriptSource(WebKit::WebString::fromUTF8(sciptText.c_str())));
        }
    }
}

void BrowserViewRequestContext::didFailLoad(int reason, string &message)
{
    m_browserLooger->error() << __FUNCTION__ << std::endl;
    if (m_request.m_settings.m_enableObserver) {
        string slog = m_prix + "FailLoad:" + message;
        m_response.m_observerLogger.push_back(slog);
    }
}

void BrowserViewRequestContext::didFinishLoad(WebKit::WebFrame *frame)
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didNavigateWithinPage()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didChangeLocationWithinPage()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didAssignIdentifierToURL(string &url, int identifier)
{
    m_browserLooger->debug() << __FUNCTION__ << ", url:" << url 
        << ", identifier:" << identifier << std::endl;
}

void BrowserViewRequestContext::didReceiveResponse(string &url, int identifier, const WebKit::WebURLResponse &response)
{
    m_browserLooger->debug() << __FUNCTION__ << ", url:" << url 
        << ", identifier:" << identifier
        << ", responseCode:" << response.httpStatusCode()
        << ", statusText:" << response.httpStatusText().utf8().data()
        << std::endl;

    if (m_request.m_settings.m_enableAllResponse
        || m_response.m_requestResponseHeaders.size() <= 0) {
        std::string responseHeader;
        getHeaderLine(responseHeader, response);
        WebURLResponseVisitor visitor(responseHeader);
        response.visitHTTPHeaderFields(&visitor);
        m_response.m_requestResponseHeaders[url] = responseHeader;
        if (m_response.m_requestResponseHeaders.size() == 1) {
            m_response.m_httpCode = response.httpStatusCode();
            m_response.m_url = url;
        }
    }
    
    if (m_request.m_settings.m_enableOutputAdURLs) {
        static WebKit::WebString AdStatusText = WebKit::WebString::fromUTF8("TecentAdURL");

        if (response.httpStatusCode() == 404 && response.httpStatusText() == AdStatusText) {
            m_browserLooger->debug() << __FUNCTION__ << ", Ad url:" << url << std::endl;
            m_response.m_adURLs.push_back(url);
        }
    }
}

void BrowserViewRequestContext::didFinishResourceLoad(WebKit::WebFrame *frame, string &url, int identifier)
{
    m_browserLooger->debug() << __FUNCTION__ << ", url:" << url 
        << ", identifier:" << identifier
        << std::endl;
    if (m_request.m_settings.m_enableExecuteExtendJavaScript
        && m_executeScriptAfterSpecificResourceLoaded) {
        std::string sciptText = "";
        map<string, string>::iterator iter = m_request.m_mapUrlAndExtendJavaScript.find(url);
        if (iter != m_request.m_mapUrlAndExtendJavaScript.end())
            sciptText = iter->second;

        if (sciptText != "") {
            if (m_request.m_settings.m_enableObserver)
                m_response.m_observerLogger.push_back(m_prix + "executeScriptAtferResourceLoaded|" + url);
            frame->executeScript(WebKit::WebScriptSource(WebKit::WebString::fromUTF8(sciptText.c_str())));
        }
    }

    if (m_request.m_settings.m_enableOutputCSSs) {
        WebKit::WebString encapsulationUrl = WebKit::WebString::fromUTF8(url.c_str());
        WebKit::WebString cssData = frame->subresourceData(encapsulationUrl, 1);

        if (!cssData.isEmpty())
            m_response.m_subResources[url] = cssData.utf8().data();
    }
}

void BrowserViewRequestContext::didFailResourceLoad(string &url, int identifier, int reason, string &message)
{
    m_browserLooger->debug() << __FUNCTION__ << ", url:" << url 
        << ", identifier:" << identifier
        << ", reason:" << reason
        << ", message:" << message
        << std::endl;
    if (m_request.m_settings.m_enableObserver) {
        string slog = m_prix + "FailResourceLoad:" + url + "|" + message;
        m_response.m_observerLogger.push_back(slog);
    }
}

void BrowserViewRequestContext::didCreateScriptContext(WebKit::WebFrame *frame)
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
    if (!m_request.m_settings.m_enableRender) 
        frame->document().setShouldCreateRenderers(false);

    if (m_request.m_settings.m_enableExecuteExtendJavaScript
        && m_executeScriptAfterDcoumentCreated) {
        std::string sciptText = m_request.m_extendJavaScript[BrowserRequest::EXECUTESCRIPT_AFTER_DOCUMENT_CREATED];
        if (sciptText != "") {
            if (m_request.m_settings.m_enableObserver)
                m_response.m_observerLogger.push_back(m_prix + "executeScriptAtferDocumentCreated");
            frame->executeScript(WebKit::WebScriptSource(WebKit::WebString::fromUTF8(sciptText.c_str())));
        }
    }
}

void BrowserViewRequestContext::didDestroyScriptContext()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didChangeScrollOffset()
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

void BrowserViewRequestContext::didChangeContentsSize(int width, int height)
{
    m_browserLooger->debug() << __FUNCTION__ << ", width:" << width
        << ", heigth: " << height << std::endl;
}

void BrowserViewRequestContext::didDoResponse(BrowserResponse *response)
{
    m_browserLooger->debug() << __FUNCTION__ << std::endl;
}

// request settings.
bool BrowserViewRequestContext::enableRender()
{
    return m_request.m_settings.m_enableRender;
}

bool BrowserViewRequestContext::enablePaint()
{
    return m_request.m_settings.m_enablePaint;
}

bool BrowserViewRequestContext::enablePlugin()
{
    return m_request.m_settings.m_enablePlugin;
}

bool BrowserViewRequestContext::enableLoadImage()
{
    return m_request.m_settings.m_enableImage;
}

bool BrowserViewRequestContext::enableLoadJavaScript()
{
    return m_request.m_settings.m_enableJavaScript;
}

bool BrowserViewRequestContext::enableExecuteExtendJavaScript()
{
    return m_request.m_settings.m_enableExecuteExtendJavaScript;
}

bool BrowserViewRequestContext::enableResponseAfterWindowLoad()
{
    return m_request.m_settings.m_enableResponseAfterWindowLoad;
}

bool BrowserViewRequestContext::enableOutputDOM() 
{
    return m_request.m_settings.m_enableOutputDOM;
}

bool BrowserViewRequestContext::enableOutputRender() 
{
    return m_request.m_settings.m_enableOutputRender;
}

bool BrowserViewRequestContext::enableOutputCSSs()
{
    return m_request.m_settings.m_enableOutputCSSs;
}

bool BrowserViewRequestContext::enableOutputJavaScript()
{
    return m_request.m_settings.m_enableOutputJavaScritp;
}

bool BrowserViewRequestContext::enableOutputAdURLs()
{
    return m_request.m_settings.m_enableOutputAdURLs;
}

bool BrowserViewRequestContext::enableLoadExtendCSS()
{
    return m_request.m_settings.m_enableExtendCSS;
}

bool BrowserViewRequestContext::enableSimulateEvent()
{
    return m_request.m_settings.m_enableSimulateEvent;
}

bool BrowserViewRequestContext::enableSubFrameToDiv()
{
    return m_request.m_settings.m_enableSubframeToDiv;
}

bool BrowserViewRequestContext::enableAllResponse() 
{
    return m_request.m_settings.m_enableAllResponse;
}

bool BrowserViewRequestContext::enableAbsoluteURL()
{
    return m_request.m_settings.m_enableAbsoluteURL;
}

bool BrowserViewRequestContext::closeWidgetSoon() 
{
    m_browserLooger->error() << __FUNCTION__ << __LINE__ << std::endl;
    return true;
}
}
