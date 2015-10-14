/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_PLATFORM
#define BROWSER_PLATFORM

#include "third_party/WebKit/Source/WebKit/chromium/public/WebIDBFactory.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebStorageNamespace.h"
#include "webkit/glue/webkitplatformsupport_impl.h"

#include "include/platform/BrowserResourceLoaderBridge.h"
#include "include/platform/BrowserWebMimeRegistry.h"
#include "include/platform/BrowserWebStorageNamespace.h"

namespace WebKit {
    class WebClipboard;
    class WebFileUtilities;
    class WebSandboxSupport;
    class WebCookieJar;
    class WebFileSystem;
    class WebString;
    class WebGraphicsContext3D;
    class WebMessagePortChannel;
}


namespace LightBrowser {

class BrowserPlatform : public webkit_glue::WebKitPlatformSupportImpl {
public:
    explicit BrowserPlatform();
    virtual ~BrowserPlatform();

    virtual WebKit::WebMimeRegistry *mimeRegistry();
    virtual WebKit::WebClipboard *clipboard();
    virtual WebKit::WebFileUtilities *fileUtilities();
    virtual WebKit::WebSandboxSupport *sandboxSupport();
    virtual WebKit::WebCookieJar* cookieJar();
    virtual WebKit::WebBlobRegistry *blobRegistry();
    virtual WebKit::WebFileSystem *fileSystem();
    virtual bool sandboxEnabled();
    virtual WebKit::WebKitPlatformSupport::FileHandle databaseOpenFile(const WebKit::WebString &vfs_file_name, int desired_flags);
    virtual int databaseDeleteFile(const WebKit::WebString &vfs_file_name, bool sync_dir);
    virtual long databaseGetFileAttributes(const WebKit::WebString &vfs_file_name);
    virtual long long databaseGetFileSize(const WebKit::WebString &vfs_file_name);
    virtual long long databaseGetSpaceAvailableForOrigin(const WebKit::WebString &origin_identifier);
    virtual unsigned long long visitedLinkHash(const char *canonicalURL, size_t length);
    virtual bool isLinkVisited(unsigned long long linkHash);
    virtual WebKit::WebMessagePortChannel *createMessagePortChannel();
    virtual void prefetchHostName(const WebKit::WebString&);
    virtual WebKit::WebData loadResource(const char *name);
    virtual WebKit::WebString queryLocalizedString(WebKit::WebLocalizedString::Name name);
    virtual WebKit::WebString queryLocalizedString(WebKit::WebLocalizedString::Name name, const WebKit::WebString &value);
    virtual WebKit::WebString queryLocalizedString(WebKit::WebLocalizedString::Name name, const WebKit::WebString &value1, const WebKit::WebString &value2);

    virtual WebKit::WebString defaultLocale();

    virtual WebKit::WebStorageNamespace *createLocalStorageNamespace(const WebKit::WebString &path, unsigned quota);

    virtual WebKit::WebIDBFactory *idbFactory();

    virtual WebKit::WebGraphicsContext3D *createOffscreenGraphicsContext3D(const WebKit::WebGraphicsContext3D::Attributes &attributes);

    virtual string16 GetLocalizedString(int message_id);
    virtual base::StringPiece GetDataResource(int resource_id, ui::ScaleFactor scale_factor);
    virtual void GetPlugins(bool refresh, std::vector<webkit::WebPluginInfo> *plugins);
    virtual webkit_glue::ResourceLoaderBridge *CreateResourceLoader(const webkit_glue::ResourceLoaderBridge::RequestInfo &request_info);
    virtual webkit_glue::WebSocketStreamHandleBridge *CreateWebSocketBridge(
        WebKit::WebSocketStreamHandle *handle,
        webkit_glue::WebSocketStreamHandleDelegate *delegate);

private:
    BrowserWebMimeRegistry *m_mimeRegistry;
    BrowserWebStorageNamespace m_storageNamespace;    
};

}


#endif