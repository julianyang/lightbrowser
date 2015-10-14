/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang

BrowerSystem implementation some system info interfaces for specific Operation System.
*/

#ifndef BROWSER_OPERATION_SYSTEM
#define BROWSER_OPERATION_SYSTEM


namespace LightBrowser {

class BrowserOperationSystem {
public:
    virtual ~BrowserOperationSystem() {}
    // if usage is 80%, then return 80.
    virtual int cupUsage() = 0;
    virtual int memoryUsage() = 0;
};

}

#endif