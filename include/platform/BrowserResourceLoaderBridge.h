/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_RESOURCELOADERBRIDGE
#define BROWSER_RESOURCELOADERBRIDGE

#include <string>
#include "base/message_loop_proxy.h"
#include "net/http/http_cache.h"
#include "webkit/glue/resource_loader_bridge.h"

namespace base {
    class Thread;
}

namespace LightBrowser {

class BrowserIOThread;
class BrowserRequestContext;
class BrowserNetworkDelegate;
struct BrowserRequestContextParam;

class BrowserResourceLoaderBridge {
public:
    static void initialize(const FilePath &cachePath, net::HttpCache::Mode cacheMode, const std::string &proxyConfig);
    static void destroy();

    static bool ensureBackendIOThread();

    static scoped_refptr<base::MessageLoopProxy> getCacheThread();
    static scoped_refptr<base::MessageLoopProxy> getIoThread();

    static webkit_glue::ResourceLoaderBridge *create(const webkit_glue::ResourceLoaderBridge::RequestInfo &requestInfo);

    static BrowserRequestContextParam *s_browserRequestContextParam;
    static BrowserRequestContext *s_browserRequestContext;
    static BrowserIOThread *s_browserIOThread;
    static base::Thread *s_browserCacheThread;
    static BrowserNetworkDelegate *s_browserNetworkDelegate;
};

}

#endif