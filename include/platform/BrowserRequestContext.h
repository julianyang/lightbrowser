/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_REQUESTCONTEXT
#define BROWSER_REQUESTCONTEXT

#include "base/threading/thread.h"
#include "net/http/http_cache.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_storage.h"

class FilePath;

namespace fileapi {
    class FileSystemContext;
}

namespace webkit_blob {
    class BlobStorageController;
}

namespace LightBrowser {

class BrowserRequestContext : public net::URLRequestContext {
public:
    BrowserRequestContext();
    BrowserRequestContext(const FilePath &cachePath, net::HttpCache::Mode cacheMode, const std::string &proxyConfig);

    virtual ~BrowserRequestContext() {}

    webkit_blob::BlobStorageController *blobStorageController() const 
    {
        return m_blobStorageController.get();
    }

    fileapi::FileSystemContext *fileSystemContext() const 
    {
        return m_fileSystemContext.get();
    }

private:
    void initialize(const FilePath &cachePath, net::HttpCache::Mode cacheMode, const std::string &proxyConfig);

    net::URLRequestContextStorage m_storage;
    scoped_ptr<webkit_blob::BlobStorageController> m_blobStorageController;
    scoped_refptr<fileapi::FileSystemContext> m_fileSystemContext;
};

}


#endif