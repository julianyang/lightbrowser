/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include "include/platform/BrowserRequestContext.h"

#include "build/build_config.h"

#include "base/compiler_specific.h"
#include "base/file_path.h"
#include "base/thread_task_runner_handle.h"
#include "base/threading/worker_pool.h"
#include "net/base/cert_verifier.h"
#include "net/base/default_server_bound_cert_store.h"
#include "net/base/host_resolver.h"
#include "net/base/server_bound_cert_service.h"
#include "net/base/ssl_config_service_defaults.h"
#include "net/cookies/cookie_monster.h"
#include "net/ftp/ftp_network_layer.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/proxy/proxy_config_service.h"
#include "net/proxy/proxy_config_service_fixed.h"
#include "net/proxy/proxy_service.h"
#include "net/url_request/http_user_agent_settings.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebKit.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebKitPlatformSupport.h"
#include "webkit/blob/blob_storage_controller.h"
#include "webkit/blob/blob_url_request_job_factory.h"
#include "webkit/fileapi/file_system_context.h"
#include "webkit/fileapi/file_system_url_request_job_factory.h"
#include "webkit/user_agent/user_agent.h"
#include "include/platform/BrowserResourceLoaderBridge.h"

namespace LightBrowser {

class BrowserHttpUserAgentSettings : public net::HttpUserAgentSettings {
public:
    BrowserHttpUserAgentSettings() {}
    virtual ~BrowserHttpUserAgentSettings() {}

    virtual std::string GetAcceptLanguage() const 
    {
        return "en-us,en";
    }

    virtual std::string GetAcceptCharset() const 
    {
        return "iso-8859-1,*,utf-8";
    }

    virtual std::string GetUserAgent(const GURL& url) const 
    {
        return webkit_glue::GetUserAgent(url);
    }

private:
    DISALLOW_COPY_AND_ASSIGN(BrowserHttpUserAgentSettings);
};

BrowserRequestContext::BrowserRequestContext()
    : ALLOW_THIS_IN_INITIALIZER_LIST(m_storage(this))
{
    initialize(FilePath(), net::HttpCache::NORMAL, std::string());
}

BrowserRequestContext::BrowserRequestContext(const FilePath &cachePath, net::HttpCache::Mode cacheMode, const std::string &proxyConfig)
    : ALLOW_THIS_IN_INITIALIZER_LIST(m_storage(this))
{
    initialize(cachePath, cacheMode, proxyConfig);
}

void BrowserRequestContext::initialize(const FilePath &cachePath, net::HttpCache::Mode cacheMode, const std::string &proxyConfig)
{
    m_storage.set_cookie_store(new net::CookieMonster(NULL, NULL));
    m_storage.set_server_bound_cert_service(new net::ServerBoundCertService(
        new net::DefaultServerBoundCertStore(NULL),
        base::WorkerPool::GetTaskRunner(true)));

    m_storage.set_http_user_agent_settings(new BrowserHttpUserAgentSettings);

#if defined(OS_POSIX) && !defined(OS_MACOSX)
    // Use no proxy to avoid ProxyConfigServiceLinux.
    // Enabling use of the ProxyConfigServiceLinux requires:
    // -Calling from a thread with a TYPE_UI MessageLoop,
    // -If at all possible, passing in a pointer to the IO thread's MessageLoop,
    // -Keep in mind that proxy auto configuration is also
    //  non-functional on linux in this context because of v8 threading
    //  issues.
    // TODO(port): rename "linux" to some nonspecific unix.
    scoped_ptr<net::ProxyConfigService> proxy_config_service(
        new net::ProxyConfigServiceFixed(net::ProxyConfig()));
#else
    // Use the system proxy settings.
    scoped_ptr<net::ProxyConfigService> proxy_config_service(
        net::ProxyService::CreateSystemProxyConfigService(
        base::ThreadTaskRunnerHandle::Get(), NULL));
#endif

    m_storage.set_host_resolver(net::HostResolver::CreateDefaultResolver(NULL));
    m_storage.set_cert_verifier(net::CertVerifier::CreateDefault());

    // set network proxy here.
    if (!proxyConfig.empty()) {
        net::ProxyConfig proxy_config;
        proxy_config.proxy_rules().ParseFromString(proxyConfig);
        m_storage.set_proxy_service(net::ProxyService::CreateFixed(proxy_config));
    } else 
        m_storage.set_proxy_service(net::ProxyService::CreateUsingSystemProxyResolver(proxy_config_service.release(), 0, NULL));

    m_storage.set_ssl_config_service(new net::SSLConfigServiceDefaults);

    m_storage.set_http_auth_handler_factory(net::HttpAuthHandlerFactory::CreateDefault(host_resolver()));
    m_storage.set_http_server_properties(new net::HttpServerPropertiesImpl);

    net::HttpCache::DefaultBackend* backend = new net::HttpCache::DefaultBackend(
        cachePath.empty() ? net::MEMORY_CACHE : net::DISK_CACHE,
        cachePath, 0, BrowserResourceLoaderBridge::getCacheThread());

    net::HttpNetworkSession::Params network_session_params;
    network_session_params.host_resolver = host_resolver();
    network_session_params.cert_verifier = cert_verifier();
    network_session_params.server_bound_cert_service = server_bound_cert_service();
    network_session_params.proxy_service = proxy_service();
    network_session_params.ssl_config_service = ssl_config_service();
    network_session_params.http_auth_handler_factory = http_auth_handler_factory();
    network_session_params.http_server_properties = http_server_properties();
    network_session_params.host_resolver = host_resolver();

    net::HttpCache *cache = new net::HttpCache(network_session_params, backend);
    cache->set_mode(cacheMode);
    m_storage.set_http_transaction_factory(cache);

    m_storage.set_ftp_transaction_factory(new net::FtpNetworkLayer(host_resolver()));

    m_blobStorageController.reset(new webkit_blob::BlobStorageController());

    // use default URLRequestJobFactory.
    net::URLRequestJobFactoryImpl *job_factory = new net::URLRequestJobFactoryImpl();
    m_storage.set_job_factory(job_factory);
}

}