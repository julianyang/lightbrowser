/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_VIEW_REQUEST_CONTEXT
#define BROWSER_VIEW_REQUEST_CONTEXT

#include <string>
#include "LightBrowser.h"

using namespace std;

namespace WebKit {
    class WebFrame;
    class WebURLResponse;
}

namespace LightBrowser {

class BrowserLogger;

class BrowserViewRequestContext {
public:
    explicit BrowserViewRequestContext(const BrowserRequest &request);
    ~BrowserViewRequestContext();

    // request client.
    virtual bool canHandleRequest(const string &url);
    virtual bool shouldBeginEditing();
    virtual bool shouldEndEditing();
    virtual bool shouldInsertNode();
    virtual bool shouldInsertText();
    virtual bool shouldApplyStyle(const string &style);
    virtual bool shouldChangeSelectedRange();
    virtual bool shouldDeleteRange();
    virtual bool isSmartInsertDeleteEnabled();
    virtual bool isSelectTrailingWhitespaceEnabled();
    virtual void runModalAlertDialog(const string &message);
    virtual bool runModalConfirmDialog(const string &message);
    virtual bool runModalPromptDialog(const string &message, const string &defaultValue, string &actualValue);
    virtual bool runModalBeforeUnloadDialog(const string &message);
    virtual bool willSendRequest(string &url, int identifier);
    virtual string userAgent();

    // request observer.
    virtual void didCreateWebView();
    virtual void didCreateFrame(bool isMainFrame);
    virtual void didAddMessageToConsole(string &message);
    virtual void didStartLoading();
    virtual void didStopLoading();
    virtual void didBeginEditing();
    virtual void didEndEditing();
    virtual void didChangeSelection();
    virtual void didChangeContents();
    virtual bool didPaint(int x, int y, int widht, int height);
    virtual void didFocus();
    virtual void didBlur();
    virtual void didCreatePlugin();
    virtual void didCreateMediaPlayer();
    virtual void didCancelClientRedirect();
    virtual void didCreateDataSource();
    virtual void didClearWindowObject();
    virtual void didStartProvisionalLoad();
    virtual void didReceiveServerRedirectForProvinsionalLoad();
    virtual void didFailProvisionalLoad(WebKit::WebFrame *frame, int reason, string &message);
    virtual void didCommitProvisionalLoad();
    virtual void didReceiveDocumentData(const char *data, size_t length);
    virtual void didReceiveTitle(string &title);
    virtual void didFinishDocumentLoad(WebKit::WebFrame *frame);
    virtual void didHandleOnLoadEvents(WebKit::WebFrame *frame);
    virtual void didFailLoad(int reason, string &message);
    virtual void didFinishLoad(WebKit::WebFrame *frame);
    virtual void didNavigateWithinPage();
    virtual void didChangeLocationWithinPage();
    virtual void didAssignIdentifierToURL(string &url, int identifier);
    virtual void didReceiveResponse(string &url, int identifier, const WebKit::WebURLResponse &response);
    virtual void didFinishResourceLoad(WebKit::WebFrame *frame, string &url, int identifier);
    virtual void didFailResourceLoad(string &url, int identifier, int reason, string &message);
    virtual void didCreateScriptContext(WebKit::WebFrame *frame);
    virtual void didDestroyScriptContext();
    virtual void didChangeScrollOffset();
    virtual void didChangeContentsSize(int width, int height);
    virtual void didDoResponse(BrowserResponse *response);

    // requst settings.
    virtual bool enableRender();
    virtual bool enablePaint();
    virtual bool enablePlugin();
    virtual bool enableLoadImage();
    virtual bool enableLoadJavaScript();
    virtual bool enableExecuteExtendJavaScript();
    virtual bool enableResponseAfterWindowLoad();
    virtual bool enableOutputDOM();
    virtual bool enableOutputRender();
    virtual bool enableOutputCSSs();
    virtual bool enableOutputJavaScript();
    virtual bool enableOutputAdURLs();
    virtual bool enableSubFrameToDiv();
    virtual bool enableLoadExtendCSS();
    virtual bool enableSimulateEvent();
    virtual bool enableAllResponse();
    virtual bool enableAbsoluteURL();
	virtual bool closeWidgetSoon();

    BrowserRequest *request() { return &m_request; }
    BrowserResponse *response() { return &m_response; }

private:
    BrowserRequest m_request;    
    BrowserResponse m_response;
    BrowserLogger *m_browserLooger;

    bool m_executeScriptAfterDcoumentCreated;
    bool m_executeScriptAfterSpecificResourceLoaded;
    bool m_executeScriptAfterFinishParse;
    bool m_executeScriptAfterWindowLoaded;

    string m_prix;
};

}

#endif