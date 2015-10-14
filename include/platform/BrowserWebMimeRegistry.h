/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_WEBMIMEREGISTRY
#define BROWSER_WEBMIMEREGISTRY

#include "third_party/WebKit/Source/Platform/chromium/public/WebMimeRegistry.h"


namespace LightBrowser {

class BrowserWebMimeRegistry : public WebKit::WebMimeRegistry {
public:
    BrowserWebMimeRegistry() {}
    ~BrowserWebMimeRegistry() {}

    virtual WebKit::WebMimeRegistry::SupportsType supportsMIMEType(const WebKit::WebString& mimeType);
    virtual WebKit::WebMimeRegistry::SupportsType supportsImageMIMEType(const WebKit::WebString& mimeType);
    virtual WebKit::WebMimeRegistry::SupportsType supportsJavaScriptMIMEType(const WebKit::WebString& mimeType);

    virtual WebKit::WebMimeRegistry::SupportsType supportsMediaMIMEType(const WebKit::WebString& mimeType,
        const WebKit::WebString& codecs);

    virtual WebKit::WebMimeRegistry::SupportsType supportsMediaMIMEType(const WebKit::WebString& mimeType,
        const WebKit::WebString& codecs,
        const WebKit::WebString& keySystem);

    virtual WebKit::WebMimeRegistry::SupportsType supportsNonImageMIMEType(const WebKit::WebString& mimeType);

    virtual WebKit::WebString mimeTypeForExtension(const WebKit::WebString& fileExtension);
    virtual WebKit::WebString wellKnownMimeTypeForExtension(const WebKit::WebString& fileExtension);
    virtual WebKit::WebString mimeTypeFromFile(const WebKit::WebString& filePath);
    virtual WebKit::WebString preferredExtensionForMIMEType(const WebKit::WebString& mimeType);
};

}


#endif