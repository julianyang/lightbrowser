/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_WEBSTORAGENAMESPACE
#define BROWSER_WEBSTORAGENAMESPACE

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "webkit/dom_storage/dom_storage_context.h"

namespace dom_storage {
    class DomStorageHost;
}
namespace WebKit {
    class WebStorageNamespace;
}

namespace LightBrowser {

class BrowserWebStorageNamespace : public dom_storage::DomStorageContext::EventObserver {
public:
    static BrowserWebStorageNamespace &instance() { return *g_instance_; }

    BrowserWebStorageNamespace();
    virtual ~BrowserWebStorageNamespace();

    WebKit::WebStorageNamespace *CreateLocalStorageNamespace();
    WebKit::WebStorageNamespace *CreateSessionStorageNamespace();

private:
    // Inner classes that implement the WebKit WebStorageNamespace and
    // WebStorageArea interfaces in terms of dom_storage classes.
    class NamespaceImpl;
    class AreaImpl;

    // DomStorageContext::EventObserver implementation which
    // calls into webkit/webcore to dispatch events.
    virtual void OnDomStorageItemSet(
        const dom_storage::DomStorageArea *area,
        const string16 &key,
        const string16 &new_value,
        const NullableString16 &old_value,
        const GURL &page_url);

    virtual void OnDomStorageItemRemoved(
        const dom_storage::DomStorageArea *area,
        const string16 &key,
        const string16 &old_value,
        const GURL &page_url);

    virtual void OnDomStorageAreaCleared(
        const dom_storage::DomStorageArea *area,
        const GURL &page_url);

    void DispatchDomStorageEvent(
        const dom_storage::DomStorageArea *area,
        const GURL &page_url,
        const NullableString16 &key,
        const NullableString16 &new_value,
        const NullableString16 &old_value);

    base::WeakPtrFactory<BrowserWebStorageNamespace> weak_factory_;
    scoped_refptr<dom_storage::DomStorageContext> context_;
    scoped_ptr<dom_storage::DomStorageHost> host_;
    AreaImpl *area_being_processed_;
    int next_connection_id_;

    static BrowserWebStorageNamespace *g_instance_;
};

}


#endif