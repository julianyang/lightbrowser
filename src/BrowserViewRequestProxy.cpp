/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

BrowerViewRequestProxy implementation for current BrowserView.
*/

#include <stdlib.h>
#include "include/BrowserViewRequestProxy.h"
#include "include/BrowserViewRequestContext.h"
#include "include/BrowserViewDelegate.h"
#include "include/BrowserView.h"
#include "base/bind.h"
#include "base/utf_string_conversions.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "skia/ext/bitmap_platform_device.h"
#include "skia/ext/platform_device.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkDevice.h"
#include "third_party/skia/include/core/SkStream.h"
#include "third_party/skia/include/images/SkImageEncoder.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "webkit/glue/webkit_glue.h"

namespace LightBrowser {

static void parseToken(BrowserResponse::RenderInfo &renderInfo, int offset, std::string &token)
{
    *(reinterpret_cast<int *>(&renderInfo) + offset) = strtoul(token.c_str(), NULL, 10);
}

static void parseRenderInfo(BrowserResponse *response, std::string &renderInfo)
{
    int offset = 0;
    int line = 0;
    int blockId = 0;
    bool inParse = false;
    std::istringstream inputStream(renderInfo);
    std::string token;
    BrowserResponse::RenderInfo stRenderInfo = { 0 };

    while (inputStream.get() != ' ' && !inputStream.eof()) {
        char c = inputStream.peek();
        // for each Line.
        switch (c) {
        case '|':
            // CompleteMe: handle block id.
            inParse = true;
            break;
        case '{':
            // begin new line
            blockId = strtoul(token.c_str(), NULL, 10);
            token.clear();
            break;
        case ',':
            // end token,
            if (token != "")
                parseToken(stRenderInfo, offset, token);
            offset++;
            token.clear();
            break;
        case '}':
            // handle another line
            inParse = false; 
            if (token != "")
                parseToken(stRenderInfo, offset, token);
            offset = 0;
            response->m_renderInfo[blockId] = stRenderInfo;
            line++;
            memset(&stRenderInfo, 0x00, sizeof(BrowserResponse::RenderInfo));
            token.clear();
            break;
        default:
            if (inParse) 
                token.push_back(c);
            break;
        }
    }
}


BrowserViewRequestProxy::BrowserViewRequestProxy(BrowserView *view, MessageLoop *main, MessageLoop *worker)
    : m_view(view)
    , m_mainThread(main)
    , m_workerThread(worker)
    , m_context(NULL)
{

}

BrowserViewRequestProxy::~BrowserViewRequestProxy()
{
    if (m_context)
        delete m_context;
}

void BrowserViewRequestProxy::setRequestContext(BrowserViewRequestContext *context)
{
    if (m_context)
        delete m_context; 
    m_context = context;
}

void BrowserViewRequestProxy::prepareDoResponse()
{
    m_mainThread->PostTask(FROM_HERE, 
        base::Bind(&BrowserViewRequestProxy::doResponseOnMainThread, this));
}

bool BrowserViewRequestProxy::needDoResponse()
{
    return !(m_view->m_delegate->isLoading());
}

void BrowserViewRequestProxy::dumpDocumentTree(std::string &domTree)
{
    int type = 0;
    if (m_context->enableSubFrameToDiv())
        type |= 0x1;
    if (!m_context->enableOutputJavaScript())
        type |= 0x2;
    if (m_context->enableAbsoluteURL())
        type |= 0x8;

    domTree =
        UTF16ToUTF8(webkit_glue::DumpDocumentTree(m_view->webView()->mainFrame(), type));
}

void BrowserViewRequestProxy::dumpRenderTree(std::string &renderInfo)
{
    renderInfo = 
        UTF16ToUTF8(webkit_glue::DumpRenderInfo(m_view->webView()->mainFrame(), 0));
}

void BrowserViewRequestProxy::doPaint(skia::PlatformCanvas *canvas)
{
    m_view->paint(m_context->request()->m_paintRectWidth, 
        m_context->request()->m_paintRectHeight, canvas);
}

void BrowserViewRequestProxy::doResponseOnMainThread()
{
    if (!m_context || !needDoResponse())
        return;

    m_view->stopMonitor();
    IndependentResponse *response = new IndependentResponse;
    response->m_callback = reinterpret_cast<void *>(m_context->request()->m_callback);
    response->m_callbackData = m_context->request()->m_callbackData;
    response->m_response = new BrowserResponse(*m_context->response());

    if (m_context->enableOutputDOM())
        dumpDocumentTree(response->m_response->m_content);

    if (m_context->enableRender() && m_context->enableOutputRender())
        dumpRenderTree(response->m_renderInfo);

    if (m_context->enableRender() && m_context->enablePaint()) {
        response->m_canvas = skia::CreatePlatformCanvas(m_context->request()->m_paintRectWidth, 
            m_context->request()->m_paintRectHeight, true);
        doPaint(response->m_canvas);
    }

    m_context->didDoResponse(response->m_response);

    m_workerThread->PostTask(FROM_HERE,
        base::Bind(&BrowserViewRequestProxy::doResponseOnWorkerThread, this, response));

    // can do next request.
    // maybe clear current browser view.
    m_view->prepareDoNextRequest();
}

void BrowserViewRequestProxy::doResponseOnWorkerThread(IndependentResponse *response)
{
    // do something which has no need to do on BrowserMainThread.
    if (response->m_callback) {
        // 1. save paint result.
        if (response->m_canvas) {
            SkDynamicMemoryWStream picDataStream;
            SkImageEncoder::EncodeStream(&picDataStream, 
                response->m_canvas->getTopDevice()->accessBitmap(true), 
                SkImageEncoder::kPNG_Type, 
                100);

            unsigned int nPicSize = picDataStream.getOffset();    
            std::vector<char> &image = response->m_response->m_image;
            image.resize(nPicSize);
            picDataStream.read(&image[0], 0, nPicSize);
        }

        // 2. parse renderInfo.
        if (response->m_renderInfo != "")
            parseRenderInfo(response->m_response, response->m_renderInfo);

        (reinterpret_cast<BrowserRequestCallback>(response->m_callback))(response->m_callbackData, *(response->m_response));
    }

    delete response;
}

BrowserViewRequestProxy::IndependentResponse::~IndependentResponse() 
{ 
    if (m_canvas) 
        delete m_canvas; 
    if (m_response)
        delete m_response;
}

}
