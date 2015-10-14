/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include "include/BrowserViewDelegate.h"
#include "LightBrowser.h"
#include "include/BrowserView.h"
#include "include/BrowserViewRequestProxy.h"
#include "include/BrowserViewRequestContext.h"
#include "base/file_util.h"
#include "base/bind.h"
#include "base/message_loop.h"
#include "base/process_util.h"
#include "base/string_util.h"
#include "base/stringprintf.h"
#include "base/string_number_conversions.h"
#include "base/utf_string_conversions.h"
#include "media/base/filter_collection.h"
#include "media/base/media_log.h"
#include "media/base/message_loop_factory.h"
#include "net/base/net_errors.h"
#include "include/platform/BrowserWebStorageNamespace.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebAccessibilityObject.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebConsoleMessage.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebContextMenuData.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebDeviceOrientationClientMock.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebCString.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebData.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebDataSource.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebDocument.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebDragData.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebHistoryItem.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebImage.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFileError.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFileSystemCallbacks.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebGeolocationClientMock.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebKit.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebKitPlatformSupport.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebNode.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebNotificationPresenter.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebPluginParams.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebPoint.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebPopupMenu.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebRange.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScreenInfo.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebStorageNamespace.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebString.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebURL.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebURLError.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebURLRequest.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebURLResponse.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebVector.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebWindowFeatures.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/point.h"
#include "webkit/appcache/web_application_cache_host_impl.h"
#include "webkit/glue/glue_serialize.h"
#include "webkit/glue/webdropdata.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/glue/webpreferences.h"
#include "webkit/glue/weburlrequest_extradata_impl.h"
#include "webkit/glue/window_open_disposition.h"
#include "webkit/gpu/webgraphicscontext3d_in_process_impl.h"
#include "webkit/media/webmediaplayer_impl.h"
#include "webkit/plugins/npapi/webplugin_impl.h"
#include "webkit/plugins/npapi/plugin_list.h"
#include "webkit/plugins/npapi/webplugin_delegate_impl.h"

namespace LightBrowser {

BrowserViewDelegate::BrowserViewDelegate(BrowserView *view)
    : m_browserView(view)
    , m_didFinishLoad(false)
    , m_timeout(false)
    , m_didResponse(false)
    , m_forbiddenSubResource(false)
    , m_isRedirect(false)
{
}

WebKit::WebView *BrowserViewDelegate::createView(WebKit::WebFrame *creator, 
    const WebKit::WebURLRequest &request, 
    const WebKit::WebWindowFeatures &features, 
    const WebKit::WebString &frame_name, 
    WebKit::WebNavigationPolicy policy)
{
    return m_browserView->createWebView();
}

WebKit::WebStorageNamespace* BrowserViewDelegate::createSessionStorageNamespace(unsigned quota)
{
    return BrowserWebStorageNamespace::instance().CreateSessionStorageNamespace();
}

void BrowserViewDelegate::didAddMessageToConsole(const WebKit::WebConsoleMessage &message,const WebKit::WebString &source_name, unsigned source_line)
{
    if (context()) {
        std::string m(message.text.utf8().data());
        context()->didAddMessageToConsole(m);
    }
}

void BrowserViewDelegate::didStartLoading()
{
    m_browserView->setIsLoading(true);

    if (m_isRedirect)
        reset();
    else if (m_didFinishLoad)
        return;

    if (context()) 
        context()->didStartLoading();
}

void BrowserViewDelegate::didStopLoading()
{
    m_browserView->setIsLoading(false);

    if (m_didFinishLoad)
        return;

    m_didFinishLoad = true;

    if (context()) 
        context()->didStopLoading();

    checkDoResponse();
}

void BrowserViewDelegate::didBeginEditing()
{
    if (context()) 
        context()->didBeginEditing(); 
}

bool BrowserViewDelegate::shouldBeginEditing(const WebKit::WebRange &range)
{
    if (context())
        return context()->shouldBeginEditing();
    return true;
}

bool BrowserViewDelegate::shouldEndEditing(const WebKit::WebRange &range)
{
    if (context())
        return context()->shouldEndEditing();
    return true;
}

bool BrowserViewDelegate::shouldInsertNode(const WebKit::WebNode &node, const WebKit::WebRange &range, WebKit::WebEditingAction action)
{
    if (context())
        return context()->shouldInsertNode();
    return true;
}

bool BrowserViewDelegate::shouldInsertText(const WebKit::WebString &text, const WebKit::WebRange &range, WebKit::WebEditingAction action)
{
    if (context())
        return context()->shouldInsertNode();
    return true;
}

bool BrowserViewDelegate::shouldChangeSelectedRange(const WebKit::WebRange &from, const WebKit::WebRange &to, WebKit::WebTextAffinity affinity, bool still_selecting)
{
    if (context())
        return context()->shouldChangeSelectedRange();
    return true;
}

bool BrowserViewDelegate::shouldDeleteRange(const WebKit::WebRange& range)
{
    if (context())
        return context()->shouldDeleteRange();
    return true;
}

bool BrowserViewDelegate::shouldApplyStyle(const WebKit::WebString &style, const WebKit::WebRange &range)
{
    if (context())
        return context()->shouldInsertNode();
    return true;
}

bool BrowserViewDelegate::isSmartInsertDeleteEnabled()
{
    if (context())
        return context()->isSmartInsertDeleteEnabled();
    return true;
}

bool BrowserViewDelegate::isSelectTrailingWhitespaceEnabled()
{
    if (context())
        return context()->isSelectTrailingWhitespaceEnabled();
    return true;
}

void BrowserViewDelegate::didEndEditing()
{
    if (context())
        context()->didEndEditing();
}

void BrowserViewDelegate::didChangeSelection(bool is_selection_empty)
{
    if (context())
        context()->didChangeSelection();
}

void BrowserViewDelegate::didChangeContents()
{
    if (context())
        context()->didChangeContents();
}

bool BrowserViewDelegate::handleCurrentKeyboardEvent()
{
    // CompleteMe: how to handle current keyboard event.
    return true;
}

void BrowserViewDelegate::spellCheck(const WebKit::WebString &text, int &misspelledOffset, int &misspelledLength)
{
    // NotImpementation
}

WebString BrowserViewDelegate::autoCorrectWord(const WebKit::WebString &misspelled_word)
{
    // Returns an empty string as Mac WebKit ('WebKitSupport/WebEditorClient.mm')
    // does. (If this function returns a non-empty string, WebKit replaces the
    // given misspelled string with the result one. This process executes some
    // editor commands and causes layout-test failures.)
    return WebString();
}

void BrowserViewDelegate::runModalAlertDialog(WebKit::WebFrame *frame, const WebKit::WebString &message)
{
    if (context()) 
        context()->runModalAlertDialog(std::string(message.utf8().data()));
}

bool BrowserViewDelegate::runModalConfirmDialog(WebKit::WebFrame *frame, const WebKit::WebString &message)
{
    if (context()) 
        return context()->runModalConfirmDialog(std::string(message.utf8().data()));
    return false;
}

bool BrowserViewDelegate::runModalPromptDialog(WebKit::WebFrame *frame, 
    const WebKit::WebString &message, 
    const WebKit::WebString &default_value, 
    WebKit::WebString *actual_value)
{
    std::string actualValue;
    if (context()) {
        bool result = context()->runModalPromptDialog(std::string(message.utf8().data()), 
            std::string(default_value.utf8().data()), 
            actualValue);
        // FixMe: how to return here.
        return result;
    }
    return false;
}

bool BrowserViewDelegate::runModalBeforeUnloadDialog(WebKit::WebFrame *frame, const WebKit::WebString &message)
{
    if (context())
        return context()->runModalBeforeUnloadDialog(std::string(message.utf8().data()));
    return true;
}

void BrowserViewDelegate::setStatusText(const WebKit::WebString &text)
{
    // NotImpementation
}

void BrowserViewDelegate::navigateBackForwardSoon(int offset)
{
    // NotImpementation
}

int BrowserViewDelegate::historyBackListCount()
{
    // NotImpementation
    return 0;
}

int BrowserViewDelegate::historyForwardListCount()
{
    // NotImpementation
    return 0;
}

void BrowserViewDelegate::didInvalidateRect(const WebKit::WebRect &rect)
{
    m_browserView->didInvalidateRect(rect.x, rect.y, rect.width, rect.height);
}

void BrowserViewDelegate::didScrollRect(int dx, int dy, const WebKit::WebRect &clip_rect)
{
    m_browserView->didScrollRect(dx, dy, clip_rect.x, clip_rect.y, clip_rect.width, clip_rect.height);
}

void BrowserViewDelegate::scheduleComposite()
{
    // NotImpementation
}

void BrowserViewDelegate::scheduleAnimation()
{
    // NotImpementation
}

void BrowserViewDelegate::didFocus()
{
    m_browserView->setFocus(true);
}

void BrowserViewDelegate::didBlur()
{
    m_browserView->setFocus(false);
}

void BrowserViewDelegate::didChangeCursor(const WebKit::WebCursorInfo &cursor)
{
    // NotImpementation
}

void BrowserViewDelegate::closeWidgetSoon()
{
    // this operation will cause close the webview,
    // but we can't close the webview, which will'be used later
    //m_browserView->close();    
    if (context())
        context()->closeWidgetSoon();
}

void BrowserViewDelegate::show(WebKit::WebNavigationPolicy policy)
{
    m_browserView->show();
}

void BrowserViewDelegate::runModal()
{
    // NotImpementation
}

WebKit::WebRect BrowserViewDelegate::windowRect()
{
    int x, y, w, h;
    m_browserView->windowRect(x, y, w, h);

    return WebKit::WebRect(x, y, w,h);
}

void BrowserViewDelegate::setWindowRect(const WebKit::WebRect  rect)
{
    m_browserView->setWindowRect(rect.x, rect.y, rect.width, rect.height);
}

WebKit::WebRect BrowserViewDelegate::rootWindowRect()
{
    int x, y, w, h;
    m_browserView->rootWindowRect(x, y, w, h);

    return WebKit::WebRect(x, y, w,h);
}

WebKit::WebRect BrowserViewDelegate::windowResizerRect()
{
    // NotImpementation
    return WebKit::WebRect();
}

WebKit::WebScreenInfo BrowserViewDelegate::screenInfo()
{
    // NotImpementation
    return WebKit::WebScreenInfo();
}

WebKit::WebPlugin* BrowserViewDelegate::createPlugin(WebKit::WebFrame *frame, const WebKit::WebPluginParams &params) 
{
    bool allow_wildcard = true;
    std::vector<webkit::WebPluginInfo> plugins;
    std::vector<std::string> mime_types;
    webkit::npapi::PluginList::Singleton()->GetPluginInfoArray(
        params.url, params.mimeType.utf8(), allow_wildcard,
        NULL, &plugins, &mime_types);
    if (plugins.empty())
        return NULL;

    WebKit::WebPluginParams params_copy = params;
    params_copy.mimeType = WebKit::WebString::fromUTF8(mime_types.front());
    return new webkit::npapi::WebPluginImpl(
        frame, params, plugins.front().path, AsWeakPtr());
}

WebKit::WebMediaPlayer* BrowserViewDelegate::createMediaPlayer(WebKit::WebFrame *frame, const WebKit::WebURL &url, WebKit::WebMediaPlayerClient *client)
{
#if 0
    scoped_ptr<media::MessageLoopFactory> message_loop_factory(new media::MessageLoopFactory());

    scoped_ptr<media::FilterCollection> collection(new media::FilterCollection());

    return new webkit_media::WebMediaPlayerImpl(
        frame,
        client,
        base::WeakPtr<webkit_media::WebMediaPlayerDelegate>(),
        collection.release(),
        NULL,
        NULL,
        message_loop_factory.release(),
        NULL,
        new media::MediaLog());
#else
    return NULL;
#endif
}

WebKit::WebApplicationCacheHost* BrowserViewDelegate::createApplicationCacheHost(WebKit::WebFrame*, WebKit::WebApplicationCacheHostClient*)
{
    // NotImpementation
    return NULL;
}


void BrowserViewDelegate::loadURLExternally(WebKit::WebFrame *frame, const WebKit::WebURLRequest &request, WebKit::WebNavigationPolicy policy)
{
    BrowserView *view = NULL;
    std::string url(request.url().spec().data());
    if (BrowserView::createNewWindow(url, &view))
        view->show();
}

WebKit::WebNavigationPolicy BrowserViewDelegate::decidePolicyForNavigation(WebKit::WebFrame*, const WebKit::WebURLRequest&,
    WebKit::WebNavigationType, const WebKit::WebNode&,
    WebKit::WebNavigationPolicy default_policy, bool isRedirect)
{
    return default_policy;
}

bool BrowserViewDelegate::canHandleRequest(WebKit::WebFrame *frame, const WebKit::WebURLRequest &request)
{
    if (context())
        return context()->canHandleRequest(std::string(request.url().spec().data()));

    GURL url = request.url();
    return !url.SchemeIs("spaceballs");
}

WebKit::WebURLError BrowserViewDelegate::cannotHandleRequestError(WebKit::WebFrame *frame, const WebKit::WebURLRequest &request)
{
    WebKit::WebURLError error;
    // A WebKit layout test expects the following values.
    // unableToImplementPolicyWithError() below prints them.
    error.domain = WebString::fromUTF8("WebKitErrorDomain");
    error.reason = 101;
    error.unreachableURL = request.url();
    return error;
}

WebKit::WebURLError BrowserViewDelegate::cancelledError(WebKit::WebFrame *frame, const WebKit::WebURLRequest &request)
{
    WebKit::WebURLError error;
    error.domain = WebString::fromUTF8(net::kErrorDomain);
    error.reason = net::ERR_ABORTED;
    error.unreachableURL = request.url();
    return error;
}

void BrowserViewDelegate::unableToImplementPolicyWithError(WebKit::WebFrame *frame, const WebKit::WebURLError&)
{
    // NotImpementation
    if (!frame->parent())
        doResponseWithoutCheck(BrowserResponse::E_BAD_URL);
}

void BrowserViewDelegate::willPerformClientRedirect(WebKit::WebFrame *frame, 
    const WebKit::WebURL &from, const WebKit::WebURL &to,
    double interval, double fire_time) 
{
    // NotImpementation
    if (!frame->parent()) 
        m_isRedirect = true;
}

void BrowserViewDelegate::didCancelClientRedirect(WebKit::WebFrame *frame)
{
    if (context())
        context()->didCancelClientRedirect();
}

void BrowserViewDelegate::didCreateDataSource(WebKit::WebFrame *frame, WebKit::WebDataSource *ds)
{
    if (context())
        context()->didCreateDataSource();
    // NotImpementation
}

void BrowserViewDelegate::didStartProvisionalLoad(WebKit::WebFrame *frame)
{
    if (context())
        context()->didStartProvisionalLoad();
    // NotImpementation
}

void BrowserViewDelegate::didReceiveServerRedirectForProvisionalLoad(WebKit::WebFrame *frame)
{
    if (context())
        context()->didReceiveServerRedirectForProvinsionalLoad();
    // NotImpementation
}

void BrowserViewDelegate::didFailProvisionalLoad(WebKit::WebFrame *frame, const WebKit::WebURLError &error)
{
    std::string errorMessage(error.localizedDescription.utf8().data());
    if (context())
        context()->didFailProvisionalLoad(frame, error.reason, errorMessage);

    // FixMe: how to deal with provisional load error.
    // NotImpementation
    if (!frame->parent()) // main frame failed.
        doResponseWithoutCheck(BrowserResponse::E_NETWORK);
}

void BrowserViewDelegate::didCommitProvisionalLoad(WebKit::WebFrame *frame, bool is_new_navigation)
{
    if (context())
        context()->didCommitProvisionalLoad();
    // NotImpementation
}

void BrowserViewDelegate::didReceiveTitle(WebKit::WebFrame *frame, const WebKit::WebString &title, WebKit::WebTextDirection direction)
{
    std::string t(title.utf8().data());
    if (context())
        context()->didReceiveTitle(t);
}

void BrowserViewDelegate::didFinishDocumentLoad(WebKit::WebFrame *frame)
{
    if (frame->parent())
        return;

    if (context())
        context()->didFinishDocumentLoad(frame);
}

void BrowserViewDelegate::didHandleOnloadEvents(WebKit::WebFrame *frame)
{
    if (frame->parent())
        return;

    if (m_didHandleWindowLoadEvent)
        return;

    m_didHandleWindowLoadEvent = true;

    if (context())
        context()->didHandleOnLoadEvents(frame);

    checkDoResponse();
}

void BrowserViewDelegate::didFailLoad(WebKit::WebFrame *frame, const WebKit::WebURLError &error)
{
    std::string errorMessage(error.localizedDescription.utf8().data());
    if (context())
        context()->didFailLoad(error.reason, errorMessage);

    if (!frame->parent()) // main frame failed.
        doResponseWithoutCheck(BrowserResponse::E_NETWORK);
}

void BrowserViewDelegate::didFinishLoad(WebKit::WebFrame *frame)
{
    if (frame->parent())
        return;

    if (context())
        context()->didFinishLoad(frame);
}

void BrowserViewDelegate::didNavigateWithinPage(WebKit::WebFrame *frame, bool is_new_navigation)
{
    if (context())
        context()->didNavigateWithinPage();
    // NotImpementation
}

void BrowserViewDelegate::didChangeLocationWithinPage(WebKit::WebFrame *frame)
{
    if (context())
        context()->didChangeLocationWithinPage();
}

void BrowserViewDelegate::assignIdentifierToRequest(WebKit::WebFrame *frame, unsigned identifier, const WebKit::WebURLRequest &request)
{
    // NotImpementation
    std::string url(request.url().spec().data());
    if (context())
        context()->didAssignIdentifierToURL(url, identifier);
}

void BrowserViewDelegate::willSendRequest(WebKit::WebFrame *frame, unsigned identifier, 
    WebKit::WebURLRequest &request,
    const WebKit::WebURLResponse &redirectResponse)
{
    std::string url(request.url().spec().data());
    if (context()) {
        if ((m_forbiddenSubResource 
            && request.url() != WebKit::WebURL(GURL(context()->request()->m_url))) 
            || !context()->willSendRequest(url, identifier)) {
            // not send this request.
            request.setURL(WebKit::WebURL());
            return;
        }
    }

    m_resourceIdentifierMap[identifier] = url;
    // NotImpementation
}

void BrowserViewDelegate::didReceiveResponse(WebKit::WebFrame *frame, unsigned identifier, const WebKit::WebURLResponse &response)
{
    std::string url(response.url().spec().data());
    if (context())
        context()->didReceiveResponse(url, identifier, response);
}

void BrowserViewDelegate::didFinishResourceLoad(WebKit::WebFrame *frame, unsigned identifier)
{
    if (context()) {
        std::string url;
        ResourceMap::iterator it = m_resourceIdentifierMap.find(identifier);
        if (it != m_resourceIdentifierMap.end())
            url = it->second;
        context()->didFinishResourceLoad(frame, url, identifier);
    }

    m_resourceIdentifierMap.erase(identifier);
    if (m_resourceIdentifierMap.size() <= 0)
        checkDoResponse();
}

void BrowserViewDelegate::didFailResourceLoad(WebKit::WebFrame *frame, unsigned identifier, const WebKit::WebURLError &error)
{
    if (context()) {
        std::string url;
        ResourceMap::iterator it = m_resourceIdentifierMap.find(identifier);
        if (it != m_resourceIdentifierMap.end())
            url = it->second;

        std::string errorMessage(error.localizedDescription.utf8().data());
        context()->didFailResourceLoad(url, identifier, error.reason, errorMessage);
    }

    m_resourceIdentifierMap.erase(identifier);
    if (m_resourceIdentifierMap.size() <= 0)
        checkDoResponse();
}

void BrowserViewDelegate::didDisplayInsecureContent(WebKit::WebFrame *frame)
{
    // NotImpementation
}

void BrowserViewDelegate::didRunInsecureContent(WebKit::WebFrame* frame, const WebKit::WebSecurityOrigin& origin, const WebKit::WebURL& target_url)
{
    // NotImpementation
}

void BrowserViewDelegate::didCreateScriptContext(WebKit::WebFrame *frame, v8::Handle<v8::Context>, int extension_group, int world_id)
{
    if (frame->parent())
        return;

    if (context())
        context()->didCreateScriptContext(frame);
}

WebKit::WebString BrowserViewDelegate::userAgentOverride(WebKit::WebFrame *frame, const WebKit::WebURL &url)
{
    if (context()) {
        std::string userAgent = context()->userAgent();
        if (!userAgent.empty())
            return WebString::fromUTF8(userAgent.c_str());
    }

    return WebString();
}

void BrowserViewDelegate::didDestroyScriptContext(WebKit::WebFrame *frame)
{
    if (frame->parent())
        return;

    if (context())
        context()->didDestroyScriptContext();
}

void BrowserViewDelegate::didChangeScrollOffset(WebKit::WebFrame *frame)
{
    if (context())
        context()->didChangeScrollOffset();
}

void BrowserViewDelegate::didChangeContentsSize(WebKit::WebFrame *frame, const WebKit::WebSize &size)
{
    if (context())
        context()->didChangeContentsSize(size.width, size.height);
}

void BrowserViewDelegate::openFileSystem(WebKit::WebFrame* frame,
    WebKit::WebFileSystem::Type type,
    long long size,
    bool create,
    WebKit::WebFileSystemCallbacks* callbacks)
{
    // NotImpementation
}

webkit::npapi::WebPluginDelegate *BrowserViewDelegate::CreatePluginDelegate(const FilePath &url, const std::string &mime_type)
{
    return webkit::npapi::WebPluginDelegateImpl::Create(url, mime_type);
}

WebKit::WebPlugin *BrowserViewDelegate::CreatePluginReplacement(const FilePath &file_path)
{
    return NULL;
}

void BrowserViewDelegate::CreatedPluginWindow(gfx::PluginWindowHandle handle)
{
    // NotImpementation
}

void BrowserViewDelegate::WillDestroyPluginWindow(gfx::PluginWindowHandle handle)
{
    // NotImpementation
}

void BrowserViewDelegate::DidMovePlugin(const webkit::npapi::WebPluginGeometry &move)
{
    // NotImpementation
}

WebKit::WebCookieJar *BrowserViewDelegate::GetCookieJar()
{
    return WebKit::webKitPlatformSupport()->cookieJar();
}

BrowserViewRequestContext *BrowserViewDelegate::context()
{
    return m_browserView->requestProxy()->getContext();
}

void BrowserViewDelegate::checkDoResponse()
{
    if (!m_forbiddenSubResource && (m_resourceIdentifierMap.size() > 0 || !m_didFinishLoad))
        return;

    if (context()) {
        if (context()->enableResponseAfterWindowLoad() 
            && !m_didHandleWindowLoadEvent)
            return;

        context()->response()->m_errorCode = m_timeout ? BrowserResponse::E_TIMEOUT : BrowserResponse::E_OK;
        m_browserView->requestProxy()->prepareDoResponse();
        m_didResponse = true;
    }
}

void BrowserViewDelegate::doResponseWithoutCheck(int error)
{
    if (context()) {
        context()->response()->m_errorCode = m_timeout ? BrowserResponse::E_TIMEOUT : error;
        m_browserView->requestProxy()->prepareDoResponse();
        m_didResponse = true;
    }
}

}