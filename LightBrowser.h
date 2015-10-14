/*
Module: LightBrowser
Copyright @tencent 2014
*/

#ifndef LIGHT_BROWSER
#define LIGHT_BROWSER

#include <string>
#include <map>
#include <vector>

using namespace std;

namespace LightBrowser {

typedef void *BrowserHandle;
typedef map<string, string> BrowserCreationContext;

class BrowserResponse {
public:
    BrowserResponse();
    BrowserResponse(const BrowserResponse &o);
    ~BrowserResponse() {}

    enum ErrorInfo {
        E_OK = 0,
        E_BAD_URL,
        E_CANCEL,
        E_NETWORK,
        E_TIMEOUT,
        E_UNKNOWN
    };

    struct RenderInfo {
        int m_x;
        int m_y;
        int m_width;
        int m_height;
        int m_fontSize;
        int m_color;
        int m_bgColor;
        int m_border;
        int m_hasBgImage;
        int m_fontWeight;
        int m_align;
    };

    int m_errorCode; // error code

    string m_url; // response url.
    int m_httpCode; // http response code.
    // response header for resources, default only return main resource.
    // enable return response for all to get all resource for subresources.
    map<string, string> m_requestResponseHeaders; 

    vector<string> m_observerLogger;

    // personal response
    string m_content;
    map<int, RenderInfo> m_renderInfo;
    map<string, string> m_subResources;
    vector<char> m_image;
    vector<string> m_adURLs;
};

typedef void (*BrowserRequestCallback)(void *, const BrowserResponse &response);

class BrowserRequestSettings {
public:
    BrowserRequestSettings();

    bool m_enableObserver;
    bool m_enableRender;
    bool m_enablePaint;
    bool m_enablePlugin;
    bool m_enableImage;
    bool m_enableJavaScript;
    bool m_enableExecuteExtendJavaScript;
    bool m_enableResponseAfterWindowLoad;
    bool m_enableExtendCSS;
    bool m_enableOutputDOM;
    bool m_enableOutputRender;
    bool m_enableOutputCSSs;
    bool m_enableOutputJavaScritp;
    bool m_enableOutputAdURLs;
    bool m_enableSubframeToDiv;
    bool m_enableSimulateEvent;
    bool m_enableAllResponse;
    bool m_enableAbsoluteURL;
    bool m_enableLoadSubresources;
};

class BrowserRequest {
public:
    BrowserRequest();
    BrowserRequest(const BrowserRequest &o);

    enum LoadType{
        LOAD_DEFAULT,
        LOAD_MHTSTRING,
        LOAD_HTMLSTRING
    };

    enum HttpMethod {
        HTTP_GET,
        HTTP_POST,
        HTTP_HEAD,
        HTTP_PUT
    };

    enum ScriptExecuteTime {
        EXECUTESCRIPT_AFTER_DOCUMENT_CREATED,
        EXECUTESCRIPT_AFTER_SPECIFIC_RESOURCE_LOADED,
        EXECUTESCRIPT_AFTER_FINISH_PARSE,
        EXECUTESCRIPT_AFTER_WINDOW_LOADED
    };

    LoadType m_loadType;

    string m_url;

    // for LOAD_MHTSTRING, m_loadValue is MHT text,
    // for LOAD_HTMLSTRING, m_loadValue is HTML text.
    string m_loadData; 

    // common parameter
    string m_userAgent;

    int m_httpRequestMethod;
    string m_httpRequestHeader;
    string m_httpRequestBody;
    string m_referer;
    
    int m_screenWidth;
    int m_screenHeight;

    int m_paintRectWidth;
    int m_paintRectHeight;
    
    int m_timeout; // from ms, default is 30000 as 30s.

    // for EXECUTESCRIPT_AFTER_DOCUMENT_CREATED
    // and EXECUTESCRIPT_AFTER_FINISH_PARSE
    // and EXECUTESCRIPT_AFTER_WINDOW_LOADED
    map<int, string> m_extendJavaScript;

    // for EXECUTESCRIPT_AFTER_SPECIFIC_RESOURCE_LOADED
    map<string, string> m_mapUrlAndExtendJavaScript;

    string m_extendCSS;

    // CompleteMe: add Events here.
    vector<string> m_excludeURL;

    // common settings.
    BrowserRequestSettings m_settings;

    // extend parameters for future.
    map<int, string> m_extendStringParam;
    map<int, int> m_extendIntParam;

    BrowserRequestCallback m_callback;
    void *m_callbackData;
};


BrowserHandle CreateBrowser(BrowserCreationContext &context);
int BrowserLoad(BrowserHandle handle, BrowserRequest &request);
int ReleaseBrowser(BrowserHandle handle);

}

#endif