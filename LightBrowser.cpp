/*
Module: LightBrowser
Copyright @tencent 2014
*/

#include <stdlib.h>
#include "LightBrowser.h"
#include "include/BrowserShell.h"

namespace LightBrowser {

BrowserResponse::BrowserResponse()
    : m_errorCode(E_OK)
{

}

BrowserResponse::BrowserResponse(const BrowserResponse &o)
{
    if (this == &o)
        return;

    m_errorCode = o.m_errorCode;
    m_url = o.m_url;
    m_httpCode = o.m_httpCode;
    m_requestResponseHeaders = o.m_requestResponseHeaders;
    m_observerLogger = o.m_observerLogger;

    // personal response
    m_content = o.m_content;
    m_renderInfo = o.m_renderInfo;
    m_subResources = o.m_subResources;
    m_image = o.m_image;
    m_adURLs = o.m_adURLs;
}

BrowserRequestSettings::BrowserRequestSettings()
    : m_enableObserver(false)
    , m_enableRender(true)
    , m_enablePaint(true)
    , m_enablePlugin(true)
    , m_enableImage(true)
    , m_enableJavaScript(true)
    , m_enableExecuteExtendJavaScript(false)
    , m_enableResponseAfterWindowLoad(true)
    , m_enableExtendCSS(false)
    , m_enableOutputDOM(true)
    , m_enableOutputRender(false)
    , m_enableOutputCSSs(false)
    , m_enableOutputJavaScritp(true)
    , m_enableOutputAdURLs(false)
    , m_enableSubframeToDiv(false)
    , m_enableSimulateEvent(false)
    , m_enableAllResponse(false)
    , m_enableAbsoluteURL(true)
    , m_enableLoadSubresources(true)
{

}

BrowserRequest::BrowserRequest()
    : m_loadType(LOAD_DEFAULT)
    , m_httpRequestMethod(HTTP_GET)
    , m_screenWidth(1024)
    , m_screenHeight(768)
    , m_paintRectWidth(1024)
    , m_paintRectHeight(768)
    , m_timeout(30000) // default is 30s
    , m_callback(NULL)
    , m_callbackData(NULL)
{

}

BrowserRequest::BrowserRequest(const BrowserRequest &o)
{
    if (this == &o)
        return;

    m_url = o.m_url;
    m_loadType = o.m_loadType;
    m_loadData = o.m_loadData;

    // common parameter
    m_userAgent = o.m_userAgent;
    m_httpRequestMethod = o.m_httpRequestMethod;
    m_httpRequestHeader = o.m_httpRequestHeader;
    m_httpRequestBody = o.m_httpRequestBody;
    m_referer = o.m_referer;

    m_screenWidth = o.m_screenWidth;
    m_screenHeight = o.m_screenHeight;

    m_paintRectWidth = o.m_paintRectWidth;
    m_paintRectHeight = o.m_paintRectHeight;
    m_timeout = o.m_timeout;

    m_excludeURL = o.m_excludeURL;
    m_settings = o.m_settings;

    m_extendJavaScript = o.m_extendJavaScript;
    m_mapUrlAndExtendJavaScript = o.m_mapUrlAndExtendJavaScript;
    m_extendCSS = o.m_extendCSS;

    m_extendStringParam = m_extendStringParam;
    m_extendIntParam = m_extendIntParam;

    m_callback = o.m_callback;
    m_callbackData = o.m_callbackData;
}

BrowserHandle CreateBrowser(BrowserCreationContext &context)
{
    // parse context here.
    BrowserShell::ShellOption option;
    BrowserCreationContext::iterator it = context.find("shelltype");
    if (it != context.end()) {
        option.m_type = strtol(it->second.c_str(), NULL, 10);
        if (option.m_type > BrowserShell::NEW_TAB && option.m_type < BrowserShell::SINGLE_TAB)
            option.m_type = BrowserShell::SINGLE_TAB; // default is single tab.

        if (option.m_type == BrowserShell::FIXED_TAB) {
            it = context.find("viewcount");
            if (it == context.end())
                option.m_viewCount = 5; // default fixed tab is 5;
            else 
                option.m_viewCount = strtol(it->second.c_str(), NULL, 10);
        }
    }

    it = context.find("queuetype");
    if (it == context.end())
        option.m_queueType = 0; // default is simple queue type.
    else 
        option.m_queueType = strtol(it->second.c_str(), NULL, 10);


    it = context.find("loggerimpltype");
    if (it == context.end())
        option.m_loggerImplType = 0; // default is simple logger type.
    else 
        option.m_loggerImplType = strtol(it->second.c_str(), NULL, 10);

    it = context.find("systemloggerlevel");
    if (it == context.end())
        option.m_systemLoggerLevel = 0; // default logger level is error.
    else 
        option.m_systemLoggerLevel = strtol(it->second.c_str(), NULL, 10);

    it = context.find("longtasktime");
    if (it == context.end())
        option.m_longTaskTime = 5000; // default long time task limit is 5000ms.
    else 
        option.m_longTaskTime = strtol(it->second.c_str(), NULL, 10);

    it = context.find("cachemode");
    if (it == context.end())
        option.m_cacheMode = 0; // default is normal cache type.
    else {
        option.m_cacheMode = strtol(it->second.c_str(), NULL, 10);
        if (option.m_cacheMode > 3 && option.m_cacheMode < 0)
            option.m_cacheMode = 0;

        if (option.m_cacheMode < 3) {
            it = context.find("cachepath");
            if (it != context.end())
                option.m_cachePath = it->second;
        }
    }

    it = context.find("netproxy");
    if (it != context.end())
        option.m_networkProxy = it->second;

    BrowserShell *shell = BrowserShellFactory::CreateBrowser(option);

    return static_cast<BrowserHandle>(shell);
}

int BrowserLoad(BrowserHandle handle, BrowserRequest &request)
{
    BrowserShellFactory::BrowserLoad(static_cast<BrowserShell *>(handle), request);
    return 0;
}

int ReleaseBrowser(BrowserHandle handle)
{
    BrowserShellFactory::ReleaseBrowser(static_cast<BrowserShell *>(handle));
    return 0;
}

}
