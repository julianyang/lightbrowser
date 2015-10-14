/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_VIEW_DELEGATE
#define BROWSER_VIEW_DELEGATE

#include <map>
#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/string16.h"
#include "build/build_config.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebContextMenuData.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebFileSystem.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrameClient.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebRect.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebPopupType.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebViewClient.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebGraphicsContext3D.h"
#include "v8/include/v8.h"
#include "webkit/glue/webcursor.h"
#include "webkit/plugins/npapi/webplugin_page_delegate.h"

namespace WebKit {
    struct WebWindowFeatures;
    class WebStorageNamespace;
}

namespace base {
    template<typename T> class SupportsWeakPtr;
}

namespace LightBrowser {

class BrowserView;
class BrowserViewRequestContext;

class BrowserViewDelegate : public WebKit::WebViewClient, 
    public WebKit::WebFrameClient, 
    public webkit::npapi::WebPluginPageDelegate,
    public base::SupportsWeakPtr<BrowserViewDelegate> {
public:
    explicit BrowserViewDelegate(BrowserView *view);
    virtual ~BrowserViewDelegate() {}

    virtual WebKit::WebView *createView(WebKit::WebFrame *creator,
        const WebKit::WebURLRequest &request,
        const WebKit::WebWindowFeatures &features,
        const WebKit::WebString &frame_name,
        WebKit::WebNavigationPolicy policy);

    // WebKit::WebViewClient
    virtual WebKit::WebStorageNamespace* createSessionStorageNamespace(unsigned quota);
    virtual void didAddMessageToConsole(const WebKit::WebConsoleMessage &message,const WebKit::WebString &source_name, unsigned source_line);

    virtual void didStartLoading();
    virtual void didStopLoading();
    virtual bool shouldBeginEditing(const WebKit::WebRange &range);
    virtual bool shouldEndEditing(const WebKit::WebRange &range);
    virtual bool shouldInsertNode(const WebKit::WebNode &node, const WebKit::WebRange &range, WebKit::WebEditingAction action);
    virtual bool shouldInsertText(const WebKit::WebString &text, const WebKit::WebRange &range, WebKit::WebEditingAction action);
    virtual bool shouldChangeSelectedRange(const WebKit::WebRange &from, const WebKit::WebRange &to, WebKit::WebTextAffinity affinity, bool still_selecting);
    virtual bool shouldDeleteRange(const WebKit::WebRange& range);
    virtual bool shouldApplyStyle(const WebKit::WebString &style, const WebKit::WebRange &range);
    virtual bool isSmartInsertDeleteEnabled();
    virtual bool isSelectTrailingWhitespaceEnabled();
    virtual void didBeginEditing();
    virtual void didChangeSelection(bool is_selection_empty);
    virtual void didChangeContents();
    virtual void didEndEditing();
    virtual bool handleCurrentKeyboardEvent();
    virtual void spellCheck(const WebKit::WebString &text, int &misspelledOffset, int &misspelledLength);
    virtual WebKit::WebString autoCorrectWord(const WebKit::WebString &misspelled_word);
    virtual void runModalAlertDialog(WebKit::WebFrame *frame, const WebKit::WebString &message);
    virtual bool runModalConfirmDialog(WebKit::WebFrame *frame, const WebKit::WebString &message);
    virtual bool runModalPromptDialog(WebKit::WebFrame *frame, const WebKit::WebString &message, const WebKit::WebString &default_value, WebKit::WebString *actual_value);
    virtual bool runModalBeforeUnloadDialog(WebKit::WebFrame *frame, const WebKit::WebString &message);
    virtual void setStatusText(const WebKit::WebString &text);
    virtual void navigateBackForwardSoon(int offset);
    virtual int historyBackListCount();
    virtual int historyForwardListCount();

    // WebKit::WebWidgetClient
    virtual void didInvalidateRect(const WebKit::WebRect &rect);
    virtual void didScrollRect(int dx, int dy, const WebKit::WebRect &clip_rect);
    virtual void scheduleComposite();
    virtual void scheduleAnimation();
    virtual void didFocus();
    virtual void didBlur();
    virtual void didChangeCursor(const WebKit::WebCursorInfo &cursor);
    virtual void closeWidgetSoon();
    virtual void show(WebKit::WebNavigationPolicy policy);
    virtual void runModal();
    virtual WebKit::WebRect windowRect();
    virtual void setWindowRect(const WebKit::WebRect  rect);
    virtual WebKit::WebRect rootWindowRect();
    virtual WebKit::WebRect windowResizerRect();
    virtual WebKit::WebScreenInfo screenInfo();

    // WebKit::WebFrameClient
    virtual WebKit::WebPlugin* createPlugin(WebKit::WebFrame*, const WebKit::WebPluginParams&);
    virtual WebKit::WebMediaPlayer* createMediaPlayer(WebKit::WebFrame*, const WebKit::WebURL&, WebKit::WebMediaPlayerClient*);
    virtual WebKit::WebApplicationCacheHost* createApplicationCacheHost(WebKit::WebFrame*, WebKit::WebApplicationCacheHostClient*);
    virtual void loadURLExternally(WebKit::WebFrame*, const WebKit::WebURLRequest&, WebKit::WebNavigationPolicy);
    virtual WebKit::WebNavigationPolicy decidePolicyForNavigation(WebKit::WebFrame*, const WebKit::WebURLRequest&,
        WebKit::WebNavigationType, const WebKit::WebNode&,
        WebKit::WebNavigationPolicy default_policy, bool isRedirect);

    virtual bool canHandleRequest(WebKit::WebFrame*, const WebKit::WebURLRequest&);
    virtual WebKit::WebURLError cannotHandleRequestError(WebKit::WebFrame*, const WebKit::WebURLRequest &request);
    virtual WebKit::WebURLError cancelledError(WebKit::WebFrame*, const WebKit::WebURLRequest &request);
    virtual void unableToImplementPolicyWithError(WebKit::WebFrame*, const WebKit::WebURLError&);
    virtual void willPerformClientRedirect(WebKit::WebFrame*, 
        const WebKit::WebURL &from, const WebKit::WebURL &to,
        double interval, double fire_time);

    virtual void didCancelClientRedirect(WebKit::WebFrame*);
    virtual void didCreateDataSource(WebKit::WebFrame*, WebKit::WebDataSource*);
    virtual void didStartProvisionalLoad(WebKit::WebFrame*);
    virtual void didReceiveServerRedirectForProvisionalLoad(WebKit::WebFrame*);
    virtual void didFailProvisionalLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    virtual void didCommitProvisionalLoad(WebKit::WebFrame*, bool is_new_navigation);
    virtual void didReceiveTitle(WebKit::WebFrame*, const WebKit::WebString &title, WebKit::WebTextDirection direction);
    virtual void didFinishDocumentLoad(WebKit::WebFrame*);
    virtual void didHandleOnloadEvents(WebKit::WebFrame*);
    virtual void didFailLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    virtual void didFinishLoad(WebKit::WebFrame*);
    virtual void didNavigateWithinPage(WebKit::WebFrame*, bool is_new_navigation);
    virtual void didChangeLocationWithinPage(WebKit::WebFrame*);
    virtual void assignIdentifierToRequest(WebKit::WebFrame*, unsigned identifier, const WebKit::WebURLRequest&);
    virtual void willSendRequest(WebKit::WebFrame*, unsigned identifier, WebKit::WebURLRequest&,
        const WebKit::WebURLResponse& redirectResponse);
    virtual void didReceiveResponse(WebKit::WebFrame*, unsigned identifier, const WebKit::WebURLResponse&);
    virtual void didFinishResourceLoad(WebKit::WebFrame*, unsigned identifier);
    virtual void didFailResourceLoad(WebKit::WebFrame*, unsigned identifier, const WebKit::WebURLError&);
    virtual void didDisplayInsecureContent(WebKit::WebFrame *frame);
    virtual void didRunInsecureContent(WebKit::WebFrame *frame, const WebKit::WebSecurityOrigin& origin, const WebKit::WebURL& target_url);
    virtual void didCreateScriptContext(WebKit::WebFrame *frame, v8::Handle<v8::Context>, int extension_group, int world_id);
    virtual WebKit::WebString userAgentOverride(WebKit::WebFrame *frame, const WebKit::WebURL &url);
    virtual void didDestroyScriptContext(WebKit::WebFrame *frame);
    virtual void didChangeScrollOffset(WebKit::WebFrame *frame);
    virtual void didChangeContentsSize(WebKit::WebFrame *frame, const WebKit::WebSize &size);
    virtual void openFileSystem(WebKit::WebFrame *frame,
        WebKit::WebFileSystem::Type type,
        long long size,
        bool create,
        WebKit::WebFileSystemCallbacks* callbacks);

    // webkit::npapi::WebPluginPageDelegate
    virtual webkit::npapi::WebPluginDelegate* CreatePluginDelegate(const FilePath &url, const std::string &mime_type);
    virtual WebKit::WebPlugin *CreatePluginReplacement(const FilePath &file_path);
    virtual void CreatedPluginWindow(gfx::PluginWindowHandle handle);
    virtual void WillDestroyPluginWindow(gfx::PluginWindowHandle handle);
    virtual void DidMovePlugin(const webkit::npapi::WebPluginGeometry &move);
    virtual void DidStartLoadingForPlugin() {}
    virtual void DidStopLoadingForPlugin() {}
    virtual WebKit::WebCookieJar *GetCookieJar();

    bool isLoading() const { return (!m_forbiddenSubResource && m_resourceIdentifierMap.size() > 0); }
    void subResourceForbidden() { m_forbiddenSubResource = true; }
    bool isDoResponse() const { return m_didResponse; }
    void setTimeout() { m_timeout = true; }
    void doResponseError(int error) { doResponseWithoutCheck(error);}
    void reset() 
    {
        m_resourceIdentifierMap.clear();
        m_didFinishLoad = false;
        m_didHandleWindowLoadEvent = false;
        m_timeout = false;
        m_didResponse = false;
        m_forbiddenSubResource = false;
        m_isRedirect = false;
    }
 
private:
    BrowserViewRequestContext *context();
    void checkDoResponse();
    void doResponseWithoutCheck(int error);

    BrowserView *m_browserView;
    typedef std::map<uint32, std::string> ResourceMap;
    ResourceMap m_resourceIdentifierMap;

    bool m_didFinishLoad;
    bool m_didHandleWindowLoadEvent;
    bool m_timeout;
    bool m_didResponse;
    bool m_forbiddenSubResource;
    bool m_isRedirect;

    DISALLOW_COPY_AND_ASSIGN(BrowserViewDelegate);
};

}



#endif