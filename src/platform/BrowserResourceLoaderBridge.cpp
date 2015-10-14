/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include "include/platform/BrowserResourceLoaderBridge.h"

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/file_path.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop.h"
#include "base/message_loop_proxy.h"
#include "base/string_util.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "base/time.h"
#include "base/timer.h"
#include "net/base/file_stream.h"
#include "net/base/io_buffer.h"
#include "net/base/load_flags.h"
#include "net/base/mime_util.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "net/base/network_delegate.h"
#include "net/base/static_cookie_policy.h"
#include "net/base/upload_data_stream.h"
#include "net/cookies/cookie_store.h"
#include "net/http/http_cache.h"
#include "net/http/http_request_headers.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_job.h"
#include "webkit/appcache/appcache_interfaces.h"
#include "webkit/blob/blob_storage_controller.h"
#include "webkit/blob/blob_url_request_job.h"
#include "webkit/blob/shareable_file_reference.h"
#include "webkit/fileapi/file_system_context.h"
#include "webkit/fileapi/file_system_dir_url_request_job.h"
#include "webkit/fileapi/file_system_url_request_job.h"
#include "webkit/glue/resource_loader_bridge.h"
#include "webkit/glue/resource_request_body.h"
#include "webkit/glue/webkit_glue.h"

#include "include/platform/BrowserRequestContext.h"

using webkit_glue::ResourceLoaderBridge;
using webkit_glue::ResourceRequestBody;
using webkit_glue::ResourceResponseInfo;
using net::StaticCookiePolicy;
using net::HttpResponseHeaders;
using webkit_blob::ShareableFileReference;

namespace LightBrowser {

// -----------class BrowserNetworkDelegate-----------
class BrowserNetworkDelegate : public net::NetworkDelegate {
public:
    virtual ~BrowserNetworkDelegate() {}

protected:
    virtual int OnBeforeURLRequest(net::URLRequest *request,
        const net::CompletionCallback &callback,
        GURL* new_url) 
    {
        return net::OK;
    }

    virtual int OnBeforeSendHeaders(net::URLRequest *request,
        const net::CompletionCallback &callback,
        net::HttpRequestHeaders *headers) 
    {
        return net::OK;
    }

    virtual void OnSendHeaders(net::URLRequest *request, const net::HttpRequestHeaders &headers) {}
    virtual int OnHeadersReceived(net::URLRequest *request,
        const net::CompletionCallback &callback,
        const net::HttpResponseHeaders *original_response_headers,
        scoped_refptr<net::HttpResponseHeaders> *override_response_headers) 
    {
        return net::OK;
    }

    virtual void OnBeforeRedirect(net::URLRequest *request, const GURL &new_location) {}
    virtual void OnResponseStarted(net::URLRequest *request) {}
    virtual void OnRawBytesRead(const net::URLRequest &request, int bytes_read) {}
    virtual void OnCompleted(net::URLRequest *request, bool started) {}
    virtual void OnURLRequestDestroyed(net::URLRequest *request) {}

    virtual void OnPACScriptError(int line_number, const string16& error) {}

    virtual AuthRequiredResponse OnAuthRequired(net::URLRequest *request,
        const net::AuthChallengeInfo &auth_info,
        const AuthCallback &callback,
        net::AuthCredentials *credentials) 
    {
        return AUTH_REQUIRED_RESPONSE_NO_ACTION;
    }

    virtual bool OnCanGetCookies(const net::URLRequest &request,
        const net::CookieList &cookie_list) 
    {
        return true;
    }

    virtual bool OnCanSetCookie(const net::URLRequest &request,
        const std::string &cookie_line,
        net::CookieOptions *options) 
    {
        return true;
    }

    virtual bool OnCanAccessFile(const net::URLRequest &request, const FilePath &path) const { return true; }
    virtual bool OnCanThrottleRequest(const net::URLRequest &request) const { return false; }

    virtual int OnBeforeSocketStreamConnect(net::SocketStream *stream,
        const net::CompletionCallback &callback)  
    {
        return net::OK;
    }

    virtual void OnRequestWaitStateChange(const net::URLRequest& request, RequestWaitState state)  {}
};

// request context parameter.
struct BrowserRequestContextParam {
    BrowserRequestContextParam(const FilePath &cachePath, net::HttpCache::Mode cacheMode, const std::string &proxyConfig)
        : m_cachePath(cachePath)
        , m_cacheMode(cacheMode)
        , m_proxyConfig(proxyConfig)
    {

    }

    FilePath m_cachePath;
    net::HttpCache::Mode m_cacheMode;
    std::string m_proxyConfig;
};

// -----------class BrowserIOThread-------------
class BrowserIOThread : public base::Thread {
public:
    BrowserIOThread()
        : base::Thread("BrowserIOThread")
    {
        // nothing to do
    }

    ~BrowserIOThread() { Stop(); }

    virtual void Init();
    virtual void CleanUp();
};

void BrowserIOThread::Init()
{
    if (BrowserResourceLoaderBridge::s_browserRequestContextParam) {
        BrowserResourceLoaderBridge::s_browserRequestContext = new BrowserRequestContext(
            BrowserResourceLoaderBridge::s_browserRequestContextParam->m_cachePath,
            BrowserResourceLoaderBridge::s_browserRequestContextParam->m_cacheMode,
            BrowserResourceLoaderBridge::s_browserRequestContextParam->m_proxyConfig);

        delete BrowserResourceLoaderBridge::s_browserRequestContextParam;
        BrowserResourceLoaderBridge::s_browserRequestContextParam = NULL;
    } else 
        BrowserResourceLoaderBridge::s_browserRequestContext = new BrowserRequestContext;

    BrowserResourceLoaderBridge::s_browserNetworkDelegate = new BrowserNetworkDelegate;

    BrowserResourceLoaderBridge::s_browserRequestContext->set_network_delegate(BrowserResourceLoaderBridge::s_browserNetworkDelegate);

    // Initialize other systems.
}

void BrowserIOThread::CleanUp()
{
    if (BrowserResourceLoaderBridge::s_browserRequestContext) {
        BrowserResourceLoaderBridge::s_browserRequestContext->set_network_delegate(NULL);
        delete BrowserResourceLoaderBridge::s_browserRequestContext;
        BrowserResourceLoaderBridge::s_browserRequestContext = NULL;
    }

    if (BrowserResourceLoaderBridge::s_browserNetworkDelegate) {
        delete BrowserResourceLoaderBridge::s_browserNetworkDelegate;
        BrowserResourceLoaderBridge::s_browserNetworkDelegate = NULL;
    }
}

// request parameter.
struct RequestParams {
    std::string m_method;
    GURL m_url;
    GURL m_firstPartyForCookie;
    GURL m_referrer;
    WebKit::WebReferrerPolicy m_referrerPolicy;
    std::string m_headers;
    int m_loadFlags;
    ResourceType::Type m_requestType;
    int m_appcacheHostId;
    bool m_downloadToFile;
    scoped_refptr<webkit_glue::ResourceRequestBody> m_requestBody;
};

// -----------class BrowserRequestProxy--------------
struct DeleteOnIOThread;

class BrowserRequestProxy : public net::URLRequest::Delegate, public base::RefCountedThreadSafe<BrowserRequestProxy, DeleteOnIOThread> {
public:
    BrowserRequestProxy()
        : m_downloadToFile(false)
        , m_buffer(new net::IOBuffer(s_dataSize))
        , m_ownerLoop(NULL)
        , m_peer(NULL)
        , m_lastUploadPosition(0)
    {
        
    }

    void DropPeer() { m_peer = NULL; }
    void Start(ResourceLoaderBridge::Peer *peer, RequestParams *params);
    void Cancel();

protected:
    friend class base::DeleteHelper<BrowserRequestProxy>;
    friend class base::RefCountedThreadSafe<BrowserRequestProxy>;
    friend struct DeleteOnIOThread;

    virtual ~BrowserRequestProxy() {}

    void NotifyReceivedRedirect(const GURL &newUrl, const ResourceResponseInfo &info);
    void NotifyReceivedResponse(const ResourceResponseInfo &info) { if (m_peer) m_peer->OnReceivedResponse(info); }
    void NotifyReceivedData(int bytesRead);
    void NotifyDownloadedData(int bytesRead);
    void NotifyCompletedRequest(int errorCode, const std::string &securityInfo, const base::TimeTicks &completeTime);
    void NotifyUploadProgress(uint64 position, uint64 size) { if (m_peer) m_peer->OnUploadProgress(position, size); }

    void AsyncStart(RequestParams* params);
    void AsyncCancel();
    void AsyncFollowDeferredRedirect(bool hasNewFirstPartyForCookies, const GURL &newFirstPartyForCookies);
    void AsyncReadData();
    virtual void OnReceivedRedirect(const GURL &newUrl, const ResourceResponseInfo &info, bool *deferRedirect);
    virtual void OnReceivedResponse(const ResourceResponseInfo &info);
    virtual void OnReceivedData(int bytesRead);
    virtual void OnCompletedRequest(int errorCode, const std::string &securityInfo, const base::TimeTicks &completeTime);

    virtual void OnReceivedRedirect(net::URLRequest *request, const GURL &newUrl, bool *deferRedirect);
    virtual void OnResponseStarted(net::URLRequest *request);
    virtual void OnSSLCertificateError(net::URLRequest *request, const net::SSLInfo &sslInfo, bool fatal);
    virtual void OnReadCompleted(net::URLRequest *request, int bytesRead);

    void Done();
    void MaybeUpdateUploadProgress();
    void PopulateResponseInfo(net::URLRequest *request, ResourceResponseInfo *info) const;
    void ConvertRequestParamsForFileOverHTTPIfNeeded(RequestParams *params);
    bool ConvertResponseInfoForFileOverHTTPIfNeeded(net::URLRequest *request, ResourceResponseInfo *info);


    scoped_ptr<net::URLRequest> m_request;
    bool m_downloadToFile;
    scoped_ptr<net::FileStream> m_fileStream;
    scoped_refptr<ShareableFileReference> m_downloadFile;
    
    scoped_refptr<net::IOBuffer> m_buffer;
    MessageLoop *m_ownerLoop; // main thread
    ResourceLoaderBridge::Peer *m_peer;
    base::RepeatingTimer<BrowserRequestProxy> m_uploadProgressTimer;
    uint64 m_lastUploadPosition;
    base::TimeTicks m_lastUploadTicks;
    std::string m_fileUrlPrefix;
    scoped_ptr<net::URLRequestStatus> m_failedFileRequestStatus;

    // static members
    static const int s_dataSize = 16 * 1024;
};

void BrowserRequestProxy::Start(ResourceLoaderBridge::Peer *peer, RequestParams *params)
{
    m_peer = peer;
    m_ownerLoop = MessageLoop::current();

    ConvertRequestParamsForFileOverHTTPIfNeeded(params);

    BrowserResourceLoaderBridge::s_browserIOThread->message_loop()->PostTask(FROM_HERE, 
        base::Bind(&BrowserRequestProxy::AsyncStart, this, params));
}

void BrowserRequestProxy::Cancel()
{
    BrowserResourceLoaderBridge::s_browserIOThread->message_loop()->PostTask(FROM_HERE,
        base::Bind(&BrowserRequestProxy::AsyncCancel, this));
}

void BrowserRequestProxy::NotifyReceivedRedirect(const GURL &newUrl, const ResourceResponseInfo &info)
{
    bool hasNewFirstPartyForCookies = false;
    GURL newFirstPartyForCookies;

    if (m_peer && m_peer->OnReceivedRedirect(newUrl, info, &hasNewFirstPartyForCookies, &newFirstPartyForCookies)) {
        BrowserResourceLoaderBridge::s_browserIOThread->message_loop()->PostTask(FROM_HERE,
            base::Bind(&BrowserRequestProxy::AsyncFollowDeferredRedirect, this, hasNewFirstPartyForCookies, newFirstPartyForCookies));
    } else
        Cancel();
}

void BrowserRequestProxy::NotifyReceivedData(int bytesRead)
{
    if (!m_peer)
        return;

    scoped_ptr<char> buff(new char[bytesRead]);
    memcpy(buff.get(), m_buffer->data(), bytesRead);

    BrowserResourceLoaderBridge::s_browserIOThread->message_loop()->PostTask(FROM_HERE,
        base::Bind(&BrowserRequestProxy::AsyncReadData, this));

    m_peer->OnReceivedData(buff.get(), bytesRead, -1);
}

void BrowserRequestProxy::NotifyDownloadedData(int bytesRead)
{
    if (!m_peer)
        return;

    BrowserResourceLoaderBridge::s_browserIOThread->message_loop()->PostTask(FROM_HERE,
        base::Bind(&BrowserRequestProxy::AsyncReadData, this));

    m_peer->OnDownloadedData(bytesRead);
}

void BrowserRequestProxy::NotifyCompletedRequest(int errorCode, const std::string &securityInfo, const base::TimeTicks &completeTime)
{
    if (m_peer) {
        m_peer->OnCompletedRequest(errorCode, false, securityInfo, completeTime);

        DropPeer();
    }
}

void BrowserRequestProxy::AsyncStart(RequestParams* params)
{
    m_request.reset(BrowserResourceLoaderBridge::s_browserRequestContext->CreateRequest(params->m_url, this));
    m_request->set_method(params->m_method);
    m_request->set_first_party_for_cookies(params->m_firstPartyForCookie);
    m_request->set_referrer(params->m_referrer.spec());

    webkit_glue::ConfigureURLRequestForReferrerPolicy(m_request.get(), params->m_referrerPolicy);

    net::HttpRequestHeaders headers;
    headers.AddHeadersFromString(params->m_headers);
    m_request->SetExtraRequestHeaders(headers);
    m_request->set_load_flags(params->m_loadFlags);
    if (params->m_requestBody) {
        m_request->set_upload(make_scoped_ptr(
            params->m_requestBody->ResolveElementsAndCreateUploadDataStream(
            static_cast<BrowserRequestContext*>(BrowserResourceLoaderBridge::s_browserRequestContext)->
            blobStorageController())));
    }

    m_downloadToFile  = params->m_downloadToFile;
    // CompleteMe: add download to file here.

    m_request->Start();

    // CompleteMe: add upload logical here.

    delete params;
}

void BrowserRequestProxy::AsyncCancel()
{
    if (!m_request.get())
        return;

    m_request->Cancel();
    Done();
}

void BrowserRequestProxy::AsyncFollowDeferredRedirect(bool hasNewFirstPartyForCookies, const GURL &newFirstPartyForCookies)
{
    if (!m_request.get())
        return;

    if (hasNewFirstPartyForCookies)
        m_request->set_first_party_for_cookies(newFirstPartyForCookies);
    m_request->FollowDeferredRedirect();
}

void BrowserRequestProxy::AsyncReadData()
{
    if (!m_request.get())
        return;

    if (m_request->status().is_success()) {
        int bytesRead = 0;
        if (m_request->Read(m_buffer, s_dataSize, &bytesRead) && bytesRead)
            OnReceivedData(bytesRead);
        else if (!m_request->status().is_io_pending()) 
            Done();
    } else 
        Done();
}

void BrowserRequestProxy::OnReceivedRedirect(const GURL &newUrl, const ResourceResponseInfo &info, bool *deferRedirect)
{
    *deferRedirect = true;
    m_ownerLoop->PostTask(FROM_HERE,
        base::Bind(&BrowserRequestProxy::NotifyReceivedRedirect, this, newUrl, info));
}

void BrowserRequestProxy::OnReceivedResponse(const ResourceResponseInfo &info)
{
    m_ownerLoop->PostTask(FROM_HERE,
        base::Bind(&BrowserRequestProxy::NotifyReceivedResponse, this, info));
}

void BrowserRequestProxy::OnReceivedData(int bytesRead)
{
    if (m_downloadToFile) {
        // CompleteMe: write date to file.
        m_ownerLoop->PostTask(FROM_HERE,
            base::Bind(&BrowserRequestProxy::NotifyDownloadedData, this, bytesRead));
        return;
    }

    m_ownerLoop->PostTask(FROM_HERE,
        base::Bind(&BrowserRequestProxy::NotifyReceivedData, this, bytesRead));
}

void BrowserRequestProxy::OnCompletedRequest(int errorCode, const std::string &securityInfo, const base::TimeTicks &completeTime)
{
    if (m_downloadToFile) {
        // CompleteMe: reset file stream.
    }

    m_ownerLoop->PostTask(FROM_HERE,
        base::Bind(&BrowserRequestProxy::NotifyCompletedRequest, this, errorCode, securityInfo, completeTime));
}

void BrowserRequestProxy::OnReceivedRedirect(net::URLRequest *request, const GURL &newUrl, bool *deferRedirect)
{
    ResourceResponseInfo info;
    PopulateResponseInfo(request, &info);

    OnReceivedRedirect(newUrl, info, deferRedirect);
}

void BrowserRequestProxy::OnResponseStarted(net::URLRequest *request)
{
    if (request->status().is_success()) {
        ResourceResponseInfo info;
        PopulateResponseInfo(request, &info);
        
        if (ConvertResponseInfoForFileOverHTTPIfNeeded(request, &info) && m_failedFileRequestStatus.get())
            AsyncCancel();
        else {
            OnReceivedResponse(info);
            AsyncReadData();
        }
    } else
        Done();
}

void BrowserRequestProxy::OnSSLCertificateError(net::URLRequest *request, const net::SSLInfo &sslInfo, bool fatal)
{
    request->ContinueDespiteLastError();
}

void BrowserRequestProxy::OnReadCompleted(net::URLRequest *request, int bytesRead)
{
    if (request->status().is_success() && bytesRead > 0)
        OnReceivedData(bytesRead);
    else 
        Done();
}

void BrowserRequestProxy::Done()
{
    if (m_uploadProgressTimer.IsRunning()) {
        MaybeUpdateUploadProgress();
        m_uploadProgressTimer.Stop();
    }

    OnCompletedRequest(m_failedFileRequestStatus.get() ? 
        m_failedFileRequestStatus->error() : m_request->status().error(), 
        std::string(), base::TimeTicks());

    m_request.reset(); // destroy on io thread
}

void BrowserRequestProxy::MaybeUpdateUploadProgress()
{
    // Not Implementation
}

void BrowserRequestProxy::PopulateResponseInfo(net::URLRequest *request, ResourceResponseInfo *info) const
{
    info->request_time = request->request_time();
    info->response_time = request->response_time();
    info->headers = request->response_headers();
    request->GetMimeType(&info->mime_type);
    request->GetCharset(&info->charset);
    info->content_length = request->GetExpectedContentSize();
    if (m_downloadToFile)
        info->download_file_path = m_downloadFile->path();

    if(info->headers) {
        if(info->headers->HasHeader("Content-Disposition")) {
            string disp;
            info->headers->GetNormalizedHeader("Content-Disposition", &disp);
            info->headers->RemoveHeader("Content-Disposition");
            info->headers->AddHeader("Tencent-Disposition:" + disp);
        }
        info->headers->RemoveHeader("refresh");
    }
}

void BrowserRequestProxy::ConvertRequestParamsForFileOverHTTPIfNeeded(RequestParams *params)
{
    // Not Implementation
}

bool BrowserRequestProxy::ConvertResponseInfoForFileOverHTTPIfNeeded(net::URLRequest *request, ResourceResponseInfo *info)
{
    // Not Implementation
    return false;
}

struct DeleteOnIOThread {
    static void Destruct(const BrowserRequestProxy *obj)
    {
        if (MessageLoop::current() == BrowserResourceLoaderBridge::s_browserIOThread->message_loop())
            delete obj;
        else 
            BrowserResourceLoaderBridge::s_browserIOThread->message_loop()->DeleteSoon(FROM_HERE, obj);
    }
};

// -----------class BrowserSyncRequestProxy------------
class BrowserSyncRequestProxy : public BrowserRequestProxy {
public:
    explicit BrowserSyncRequestProxy(ResourceLoaderBridge::SyncLoadResponse *result)
        : m_result(result)
        , m_event(true, false)
    {
    }

    void waitForCompletion() { m_event.TimedWait(base::TimeDelta::FromMilliseconds(2000)); }
    virtual void OnReceivedRedirect(const GURL &newUrl, const ResourceResponseInfo &info, bool *deferRedirect);
    virtual void OnReceivedResponse(const ResourceResponseInfo &info);
    virtual void OnReceivedData(int bytesRead);
    virtual void OnCompletedRequest(int errorCode, const std::string &securityInfo, const base::TimeTicks &completeTime);

protected:
    virtual ~BrowserSyncRequestProxy() {}

private:
    ResourceLoaderBridge::SyncLoadResponse *m_result;
    base::WaitableEvent m_event;
};

void BrowserSyncRequestProxy::OnReceivedRedirect(const GURL &newUrl, const ResourceResponseInfo &info, bool *deferRedirect)
{
    if (newUrl.GetOrigin() != m_result->url.GetOrigin()) {
        Cancel();
        return;
    }

    m_result->url = newUrl;
}

void BrowserSyncRequestProxy::OnReceivedResponse(const ResourceResponseInfo &info)
{
    *static_cast<ResourceResponseInfo *>(m_result) = info;
}

void BrowserSyncRequestProxy::OnReceivedData(int bytesRead)
{
    if (m_downloadToFile)
        ;// CompleteMe: write data to file sync.
    else 
        m_result->data.append(m_buffer->data(), bytesRead);

    AsyncReadData();
}

void BrowserSyncRequestProxy::OnCompletedRequest(int errorCode, const std::string &securityInfo, const base::TimeTicks &completeTime)
{
    if (m_downloadToFile) {
        ; // CompleteMe: reset file stream.
    }

    m_result->error_code = errorCode;
    m_event.Signal();
}

// -----------class ResourceLoaderBridgeImpl------------
class ResourceLoaderBridgeImpl : public ResourceLoaderBridge {
public:
    ResourceLoaderBridgeImpl(const webkit_glue::ResourceLoaderBridge::RequestInfo &requestInfo);
    virtual ~ResourceLoaderBridgeImpl();

    virtual void SetRequestBody(ResourceRequestBody* requestBody);
    virtual bool Start(Peer *peer);
    virtual void Cancel() { m_requestProxy->Cancel(); }
    void SetDefersLoading(bool value) {}
    void SyncLoad(SyncLoadResponse* response);

private:
    scoped_ptr<RequestParams> m_params;
    BrowserRequestProxy *m_requestProxy;
};

ResourceLoaderBridgeImpl::ResourceLoaderBridgeImpl(const webkit_glue::ResourceLoaderBridge::RequestInfo &requestInfo)
    : m_params(new RequestParams)
    , m_requestProxy(NULL)
{
    m_params->m_method = requestInfo.method;
    m_params->m_url = requestInfo.url;
    m_params->m_firstPartyForCookie = requestInfo.first_party_for_cookies;
    m_params->m_referrer = requestInfo.referrer;
    m_params->m_referrerPolicy = requestInfo.referrer_policy;
    m_params->m_headers = requestInfo.headers;
    m_params->m_loadFlags = requestInfo.load_flags;
    m_params->m_requestType = requestInfo.request_type;
    m_params->m_appcacheHostId = requestInfo.appcache_host_id;
    m_params->m_downloadToFile = requestInfo.download_to_file;
}

ResourceLoaderBridgeImpl::~ResourceLoaderBridgeImpl()
{
    if (m_requestProxy) {
        m_requestProxy->DropPeer();
        BrowserResourceLoaderBridge::s_browserIOThread->message_loop()->ReleaseSoon(FROM_HERE, m_requestProxy);
    }
}

void ResourceLoaderBridgeImpl::SetRequestBody(ResourceRequestBody* requestBody)
{
    m_params->m_requestBody = requestBody;
}

bool ResourceLoaderBridgeImpl::Start(Peer *peer)
{
    if (!BrowserResourceLoaderBridge::ensureBackendIOThread())
        return false;

    m_requestProxy = new BrowserRequestProxy;
    m_requestProxy->AddRef();
    m_requestProxy->Start(peer, m_params.release());

    return true;
}

void ResourceLoaderBridgeImpl::SyncLoad(SyncLoadResponse* response)
{
    if (!BrowserResourceLoaderBridge::ensureBackendIOThread())
        return;

    response->url = m_params->m_url;
    m_requestProxy = new BrowserSyncRequestProxy(response);
    m_requestProxy->AddRef();
    m_requestProxy->Start(NULL, m_params.release());
    static_cast<BrowserSyncRequestProxy *>(m_requestProxy)->waitForCompletion();
}

// -----------class BrowserResourceLoaderBridge------------
// interfaces for this module.
// support resource load.

BrowserIOThread *BrowserResourceLoaderBridge::s_browserIOThread = NULL;
base::Thread *BrowserResourceLoaderBridge::s_browserCacheThread = NULL;
BrowserRequestContext *BrowserResourceLoaderBridge::s_browserRequestContext = NULL;
BrowserRequestContextParam *BrowserResourceLoaderBridge::s_browserRequestContextParam = NULL;
BrowserNetworkDelegate *BrowserResourceLoaderBridge::s_browserNetworkDelegate = NULL;

void BrowserResourceLoaderBridge::initialize(const FilePath &cachePath, net::HttpCache::Mode cacheMode, const std::string &proxyConfig)
{
    destroy();

    s_browserRequestContextParam = new BrowserRequestContextParam(cachePath, cacheMode, proxyConfig);
}

void BrowserResourceLoaderBridge::destroy()
{
    if (s_browserIOThread) {
        delete s_browserIOThread;
        s_browserIOThread = NULL;

        delete s_browserCacheThread;
        s_browserCacheThread = NULL;
    } else {
        delete s_browserRequestContextParam;
        s_browserRequestContextParam = NULL;
    }
}

bool BrowserResourceLoaderBridge::ensureBackendIOThread()
{
    if (s_browserIOThread)
        return true;

    base::Thread::Options options;
    options.message_loop_type = MessageLoop::TYPE_IO;

    s_browserCacheThread = new base::Thread("BrowserCacheThread");
    s_browserCacheThread->StartWithOptions(options);

    s_browserIOThread = new BrowserIOThread;    
    return s_browserIOThread->StartWithOptions(options);
}

scoped_refptr<base::MessageLoopProxy> BrowserResourceLoaderBridge::getCacheThread()
{
    return s_browserCacheThread->message_loop_proxy();
}

scoped_refptr<base::MessageLoopProxy> BrowserResourceLoaderBridge::getIoThread()
{
    return s_browserIOThread->message_loop_proxy();
}

webkit_glue::ResourceLoaderBridge *BrowserResourceLoaderBridge::create(const webkit_glue::ResourceLoaderBridge::RequestInfo &requestInfo)
{
    return new ResourceLoaderBridgeImpl(requestInfo);
}

}