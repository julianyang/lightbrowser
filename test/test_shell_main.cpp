#include <unistd.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>

#include "LightBrowser.h"

void RequestCallbackImp(const LightBrowser::BrowserResponse &response)
{
    std::cout << "Response Code:" << response.m_errorCode << endl;
    std::cout << "Response Headers:" << std::endl;
    map<std::string, std::string>::const_iterator iter = response.m_requestResponseHeaders.begin();
    while (iter != response.m_requestResponseHeaders.end()) {
        std::cout << iter->first << ":" << iter->second << std::endl;
        iter++;
    }

    std::cout << "Observer info:" << std::endl;
    for (size_t i=0; i<response.m_observerLogger.size(); i++) {
        std::cout << response.m_observerLogger[i] << std::endl;
    }

    std::cout << "Failed URL:" << std::endl;
    for (size_t j=0; j<response.m_failedURL.size(); j++) {
        std::cout << response.m_failedURL[j] << std::endl;
    }

    int id = time(NULL);
    {
        char fileName[128] = { 0 };
        sprintf(fileName, "%s_%d.html", "SaveHtml", id);
        std::cout << "save content:" << fileName << std::endl;

        std::ofstream of(fileName);
        of << response.m_content;
        of.close();
    }
    
    if (response.m_renderInfo.size()) {
        char fileName[128] = { 0 };
        sprintf(fileName, "%s_%d.txt", "SaveRender", id);
        std::cout << "save Render:" << fileName << std::endl;

        std::ofstream of(fileName);
        map<int, LightBrowser::BrowserResponse::RenderInfo>::const_iterator it = response.m_renderInfo.begin();
        while (it != response.m_renderInfo.end()) {
            of << it->first << ":" 
                << "\t" << "x: " << it->second.m_x << std::endl
                << "\t" << "y: " << it->second.m_y << std::endl
                << "\t" << "w: " << it->second.m_width << std::endl
                << "\t" << "h: " << it->second.m_height << std::endl
                << "\t" << "font size: " << it->second.m_fontSize << std::endl
                << "\t" << "color: " << it->second.m_color << std::endl
                << "\t" << "bgcolor: " << it->second.m_bgColor << std::endl
                << "\t" << "border: " << it->second.m_border << std::endl
                << "\t" << "hasBgImage: " << it->second.m_hasBgImage << std::endl
                << "\t" << "font weight: " << it->second.m_fontWeight << std::endl   
                << "\t" << "align: " << it->second.m_align << std::endl  
                << std::endl;
            it++;
        }
        of.close();
        std::cout << "===============>content:" << fileName << std::endl;   
    }
    

    if (response.m_image.size()) {
        char fileName[128] = { 0 };
        sprintf(fileName, "%s_%d.png", "SaveImage", id);
        std::cout << "save image:" << fileName << std::endl;

        std::ofstream of(fileName, std::ios::binary);
        of.write(&response.m_image[0], response.m_image.size());
        of.close();
    }
    std::cout << "End response!" << endl;
}

int main(int argc, char *argv[]) 
{
    if (argc < 2) {
        std::cout << "must need two parameters" << std::endl
            << "use --help for help!" << std::endl;
        return -1;
    }

    if (strcmp(argv[1], "--help") == 0) {
        std::cout << "./test_shell p1 p2 p3" << std::endl
            << "p1: load type, 0 for normal, 1 for mhtstring, 2 for htmlstring." << std::endl
            << "p2: according to p1, if p1 is 1, p2 is url, if p1 is 2 and 3, p2 is file name" << std::endl
            << "    file save the normal data." << std::endl
            << "p3: extend javascript file name, file save the javascript code." << std::endl
            << "more parameter not specific now." << std::endl;

        return 0;
    }

    if (argc < 3) {
        std::cout << "must need two parameters" << std::endl
            << "use --help for help!" << std::endl;
        return -1;
    }

    LightBrowser::BrowserCreationContext context;
    context["systemloggerlevel"] = "2";
    LightBrowser::BrowserHandle handle = LightBrowser::CreateBrowser(context);

    if (!handle) 
        return -1;

    int loadType = strtol(argv[1], NULL, 10);
    if (loadType > 3 || loadType < 0)
        loadType = 0;

    LightBrowser::BrowserRequest request;
    request.m_loadType = static_cast<LightBrowser::BrowserRequest::LoadType>(loadType);

    if (request.m_loadType == LightBrowser::BrowserRequest::LOAD_DEFAULT)
        request.m_loadValue = argv[2];
    else {
        std::ifstream dataIn(argv[2]);
        std::stringstream dataBuffer;  
        dataBuffer << dataIn.rdbuf();  
        request.m_loadValue = dataBuffer.str();
    }

    request.m_timeout = 30000;

    if (argc > 3) {
        std::ifstream jsIn(argv[3]);
        std::stringstream jsBuffer;  
        jsBuffer << jsIn.rdbuf();  
        request.m_extendJavaScript[LightBrowser::BrowserRequest::EXECUTESCRIPT_AFTER_WINDOW_LOADED] = jsBuffer.str();
        request.m_settings.m_enableExecuteExtendJavaScript = true;
    }

    request.m_settings.m_enableObserver = true;
    request.m_settings.m_enableOutputRender = true;
    request.m_callback = RequestCallbackImp;
    LightBrowser::BrowserLoad(handle, request);

    int i=0;
    while (i<40) {
        sleep(1);
        i++;
    }

    LightBrowser::ReleaseBrowser(handle);

    return 0;
}

