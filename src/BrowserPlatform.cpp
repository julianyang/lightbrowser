/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include "include/BrowserPlatform.h"

#include "base/metrics/stats_counters.h"
#include "media/base/media.h"
#include "base/path_service.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebCache.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebDatabase.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebKit.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebRuntimeFeatures.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScriptController.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebSecurityPolicy.h"
#include "ui/gl/gl_bindings_skia_in_process.h"
#include "v8/include/v8.h"
#include "webkit/plugins/npapi/plugin_list.h"
#include "webkit/plugins/webplugininfo.h"


namespace LightBrowser {

v8::ResourceConstraints g_v8ResourceConstraints;

const char kGCExtensionName[] = "v8/GCController";

v8::Extension* gcExtensionGet() 
{
    v8::Extension* extension = new v8::Extension(
      kGCExtensionName,
      "(function () {"
      "   var v8_gc;"
      "   if (gc) v8_gc = gc;"
      "   GCController = new Object();"
      "   GCController.collect ="
      "     function() {if (v8_gc) v8_gc(); };"
      " })();");
    return extension;
}

BrowserPlatform::BrowserPlatform()
    : m_mimeRegistry(NULL)
{
    v8::V8::SetCounterFunction(base::StatsTable::FindLocation);
    g_v8ResourceConstraints.set_max_young_space_size(16*1024*1024);
    g_v8ResourceConstraints.set_max_old_space_size(1024*1024*1024);
    v8::SetResourceConstraints(&g_v8ResourceConstraints);

    WebKit::initialize(this);
    WebKit::setLayoutTestMode(false);
    WebKit::WebScriptController::enableV8SingleThreadMode();
    WebKit::WebScriptController::registerExtension(gcExtensionGet());
    WebKit::WebRuntimeFeatures::enableSockets(false);
    WebKit::WebRuntimeFeatures::enableApplicationCache(false);
    WebKit::WebRuntimeFeatures::enableDatabase(false);
    WebKit::WebRuntimeFeatures::enableNotifications(false);
    WebKit::WebRuntimeFeatures::enableTouch(true);
    WebKit::WebRuntimeFeatures::enableIndexedDatabase(false);
    WebKit::WebRuntimeFeatures::enableSpeechInput(false);
    WebKit::WebRuntimeFeatures::enableFileSystem(false);

    WebKit::WebRuntimeFeatures::enableDeviceMotion(false);
    WebKit::WebRuntimeFeatures::enableDeviceOrientation(false);

    WebKit::WebRuntimeFeatures::enableJavaScriptI18NAPI(true);

    /*FilePath module_path;
    WebKit::WebRuntimeFeatures::enableMediaPlayer(
    PathService::Get(base::DIR_MODULE, &module_path) &&
    media::InitializeMediaLibrary(module_path));*/

    WebKit::WebRuntimeFeatures::enableGeolocation(false);

    WebKit::WebRuntimeFeatures::enableMediaPlayer(false);
    WebKit::WebRuntimeFeatures::enableGeolocation(false);
    WebKit::WebRuntimeFeatures::enableLocalStorage(true);
    WebKit::WebRuntimeFeatures::enableSessionStorage(false);
    WebKit::WebRuntimeFeatures::enableDataTransferItems(false);
    WebKit::WebRuntimeFeatures::enableWebAudio(false);
    WebKit::WebRuntimeFeatures::enableXHRResponseBlob(false);
    WebKit::WebRuntimeFeatures::enableQuota(false);
    WebKit::WebRuntimeFeatures::enableMediaStream(false);

    m_mimeRegistry = new BrowserWebMimeRegistry;
}

BrowserPlatform::~BrowserPlatform()
{
    WebKit::shutdown();

    delete m_mimeRegistry;
    BrowserResourceLoaderBridge::destroy();
}

WebKit::WebMimeRegistry *BrowserPlatform::mimeRegistry()
{
    return m_mimeRegistry;
}

WebKit::WebClipboard *BrowserPlatform::clipboard()
{
    return NULL;
}

WebKit::WebFileUtilities *BrowserPlatform::fileUtilities()
{
    return NULL;
}

WebKit::WebSandboxSupport *BrowserPlatform::sandboxSupport()
{
    return NULL;
}

WebKit::WebCookieJar *BrowserPlatform::cookieJar()
{
    return NULL;
}

WebKit::WebBlobRegistry *BrowserPlatform::blobRegistry()
{
    return NULL;
}

WebKit::WebFileSystem *BrowserPlatform::fileSystem()
{
    return NULL;
}

bool BrowserPlatform::sandboxEnabled()
{
    return true;
}

WebKit::WebKitPlatformSupport::FileHandle BrowserPlatform::databaseOpenFile(const WebKit::WebString &vfs_file_name, int desired_flags)
{
    return 0;
}

int BrowserPlatform::databaseDeleteFile(const WebKit::WebString &vfs_file_name, bool sync_dir)
{
    return 0;
}

long BrowserPlatform::databaseGetFileAttributes(const WebKit::WebString &vfs_file_name)
{
    return 0;
}

long long BrowserPlatform::databaseGetFileSize(const WebKit::WebString &vfs_file_name)
{
    return 0;
}

long long BrowserPlatform::databaseGetSpaceAvailableForOrigin(const WebKit::WebString &origin_identifier)
{
    return 0;
}

unsigned long long BrowserPlatform::visitedLinkHash(const char *canonicalURL, size_t length)
{
    return 0;
}

bool BrowserPlatform::isLinkVisited(unsigned long long linkHash)
{
    return false;
}

WebKit::WebMessagePortChannel *BrowserPlatform::createMessagePortChannel()
{
    return NULL;
}

void BrowserPlatform::prefetchHostName(const WebKit::WebString&)
{
    // Not Implementation
}

WebKit::WebData BrowserPlatform::loadResource(const char *name)
{
    return webkit_glue::WebKitPlatformSupportImpl::loadResource(name);
}

WebKit::WebString BrowserPlatform::queryLocalizedString(WebKit::WebLocalizedString::Name name)
{
    return WebKitPlatformSupportImpl::queryLocalizedString(name);
}

WebKit::WebString BrowserPlatform::queryLocalizedString(WebKit::WebLocalizedString::Name name, const WebKit::WebString &value)
{
    return WebKitPlatformSupportImpl::queryLocalizedString(name, value);
}

WebKit::WebString BrowserPlatform::queryLocalizedString(WebKit::WebLocalizedString::Name name, const WebKit::WebString &value1, const WebKit::WebString &value2)
{
    return WebKitPlatformSupportImpl::queryLocalizedString(name, value1, value2);
}

WebKit::WebString BrowserPlatform::defaultLocale()
{
    return WebKit::WebString("zh-CN");
}

WebKit::WebStorageNamespace *BrowserPlatform::createLocalStorageNamespace(const WebKit::WebString &path, unsigned quota)
{
    // CompleteMe: create LocalStorageNamespace class.
    return m_storageNamespace.CreateLocalStorageNamespace();
}

WebKit::WebIDBFactory *BrowserPlatform::idbFactory()
{
    return WebKit::WebIDBFactory::create();
}

WebKit::WebGraphicsContext3D *BrowserPlatform::createOffscreenGraphicsContext3D(const WebKit::WebGraphicsContext3D::Attributes &attributes)
{
    return NULL;
}

string16 BrowserPlatform::GetLocalizedString(int message_id)
{
    return string16();
}

base::StringPiece BrowserPlatform::GetDataResource(int resource_id, ui::ScaleFactor scale_factor)
{
    return base::StringPiece();
}

void BrowserPlatform::GetPlugins(bool refresh, std::vector<webkit::WebPluginInfo> *plugins)
{
    if (refresh)
        webkit::npapi::PluginList::Singleton()->RefreshPlugins();
    webkit::npapi::PluginList::Singleton()->GetPlugins(plugins);
    // Don't load the forked TestNetscapePlugIn in the chromium code, use
    // the copy in webkit.org's repository instead.
    const FilePath::StringType kPluginBlackList[] = {
        FILE_PATH_LITERAL("npapi_layout_test_plugin.dll"),
        FILE_PATH_LITERAL("WebKitTestNetscapePlugIn.plugin"),
        FILE_PATH_LITERAL("libnpapi_layout_test_plugin.so"),
    };

    for (int i = plugins->size() - 1; i >= 0; --i) {
        webkit::WebPluginInfo plugin_info = plugins->at(i);
        for (size_t j = 0; j < arraysize(kPluginBlackList); ++j) {
            if (plugin_info.path.BaseName() == FilePath(kPluginBlackList[j])) {
                plugins->erase(plugins->begin() + i);
            }
        }
    }
}

webkit_glue::ResourceLoaderBridge *BrowserPlatform::CreateResourceLoader(const webkit_glue::ResourceLoaderBridge::RequestInfo &request_info)
{
    return BrowserResourceLoaderBridge::create(request_info);
}

webkit_glue::WebSocketStreamHandleBridge *BrowserPlatform::CreateWebSocketBridge(
    WebKit::WebSocketStreamHandle *handle,
    webkit_glue::WebSocketStreamHandleDelegate *delegate)
{
    return NULL;
}

}