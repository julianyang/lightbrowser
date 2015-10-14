/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include "include/platform/BrowserWebMimeRegistry.h"

#include "base/string_util.h"
#include "base/sys_string_conversions.h"
#include "base/utf_string_conversions.h"
#include "net/base/mime_util.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebString.h"
#include "webkit/base/file_path_string_conversions.h"
#include "webkit/media/crypto/key_systems.h"

namespace {

std::string ToASCIIOrEmpty(const WebKit::WebString& string) {
    return IsStringASCII(string) ? UTF16ToASCII(string) : std::string();
}

}

namespace LightBrowser {


WebKit::WebMimeRegistry::SupportsType BrowserWebMimeRegistry::supportsMIMEType(const WebKit::WebString& mimeType)
{
    return net::IsSupportedMimeType(ToASCIIOrEmpty(mimeType)) ?
        WebMimeRegistry::IsSupported : WebMimeRegistry::IsNotSupported;
}

WebKit::WebMimeRegistry::SupportsType BrowserWebMimeRegistry::supportsImageMIMEType(const WebKit::WebString& mimeType)
{
    return net::IsSupportedImageMimeType(ToASCIIOrEmpty(mimeType)) ?
        WebMimeRegistry::IsSupported : WebMimeRegistry::IsNotSupported;
}

WebKit::WebMimeRegistry::SupportsType BrowserWebMimeRegistry::supportsJavaScriptMIMEType(const WebKit::WebString& mimeType)
{
    return net::IsSupportedJavascriptMimeType(ToASCIIOrEmpty(mimeType)) ?
        WebMimeRegistry::IsSupported : WebMimeRegistry::IsNotSupported;
}

WebKit::WebMimeRegistry::SupportsType BrowserWebMimeRegistry::supportsMediaMIMEType(const WebKit::WebString& mimeType,
    const WebKit::WebString& codecs)
{
    return supportsMediaMIMEType(mimeType, codecs, WebKit::WebString());
}

WebKit::WebMimeRegistry::SupportsType BrowserWebMimeRegistry::supportsMediaMIMEType(const WebKit::WebString& mimeType,
    const WebKit::WebString& codecs,
    const WebKit::WebString& keySystem)
{
    return WebKit::WebMimeRegistry::MayBeSupported;
}

WebKit::WebMimeRegistry::SupportsType BrowserWebMimeRegistry::supportsNonImageMIMEType(const WebKit::WebString& mimeType)
{
    return net::IsSupportedNonImageMimeType(ToASCIIOrEmpty(mimeType)) ?
        WebMimeRegistry::IsSupported : WebMimeRegistry::IsNotSupported;
}

WebKit::WebString BrowserWebMimeRegistry::mimeTypeForExtension(const WebKit::WebString& fileExtension)
{
    std::string mime_type;
    net::GetMimeTypeFromExtension(
        webkit_base::WebStringToFilePathString(fileExtension), &mime_type);
    return ASCIIToUTF16(mime_type);
}

WebKit::WebString BrowserWebMimeRegistry::wellKnownMimeTypeForExtension(const WebKit::WebString& fileExtension)
{
    std::string mime_type;
    net::GetWellKnownMimeTypeFromExtension(
        webkit_base::WebStringToFilePathString(fileExtension), &mime_type);
    return ASCIIToUTF16(mime_type);
}

WebKit::WebString BrowserWebMimeRegistry::mimeTypeFromFile(const WebKit::WebString& filePath)
{
    std::string mime_type;
    net::GetMimeTypeFromFile(webkit_base::WebStringToFilePath(filePath),
        &mime_type);
    return ASCIIToUTF16(mime_type);
}

WebKit::WebString BrowserWebMimeRegistry::preferredExtensionForMIMEType(const WebKit::WebString& mimeType)
{
    FilePath::StringType file_extension;
    net::GetPreferredExtensionForMimeType(ToASCIIOrEmpty(mimeType),
        &file_extension);
    return webkit_base::FilePathStringToWebString(file_extension);
}

}