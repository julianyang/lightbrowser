/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#ifndef BROWSER_LOGGER
#define BROWSER_LOGGER

#include <string>
#include <iostream>
#if defined(USE_TAF_LOGGER)
#undef LOG
#undef DLOG
#include "log/taf_logger.h"
#undef LOG
#undef DLOG
#endif

namespace LightBrowser {

#if !defined(USE_TAF_LOGGER)
class BrowserLoggerStream {
public:
    BrowserLoggerStream(bool isOut, std::string &header)
        : m_isOut(isOut)
        , m_header(header)
    {
        if (m_isOut) {
            std::cout << header;
        }
    }

    ~BrowserLoggerStream()
    {
        if (m_isOut) {
            std::cout.flush();
        }
    }

    template<typename P>
    BrowserLoggerStream &operator<<(const P &t) 
    {
        if (m_isOut) {
            std::cout << t;
        }

        return *this;
    }

    typedef std::ostream &(*F)(std::ostream& os);
    BrowserLoggerStream &operator<<(F f)
    { 
        if (m_isOut) {
            (f)(std::cout);
        }

        return *this;
    }

    typedef std::ios_base &(*I)(std::ios_base &os);
    BrowserLoggerStream &operator<<(I f) 
    { 
        if (m_isOut) {
            (f)(std::cout);
        }

        return *this;
    }

    BrowserLoggerStream(const BrowserLoggerStream &o)
    {
        if (this == &o)
            return;

        if (m_isOut) {
            std::cout.flush();
        }

        m_isOut = o.m_isOut;
        m_header = o.m_header;
    }

    BrowserLoggerStream &operator=(const BrowserLoggerStream &o)
    {
        if (this == &o)
            return *this;

        if (m_isOut) {
            std::cout.flush();
        }

        m_isOut = o.m_isOut;
        m_header = o.m_header;
    }

private:
    bool m_isOut;
    std::string m_header;  
};
#else
    typedef taf::LoggerStream BrowserLoggerStream;
#endif

class BrowserLogger {
public:
    enum LoggerLevel {
        E_ERROR,
        E_WARN,
        E_DEBUG,
        E_INFO
    };
    virtual ~BrowserLogger() {}

    virtual void setLoggerLevel(int level) { if (isInvalidLevel(level)) m_level = level; }
    
    virtual BrowserLoggerStream error() { return getOutStream(E_ERROR); }
    virtual BrowserLoggerStream warn() { return getOutStream(E_WARN); }
    virtual BrowserLoggerStream debug() { return getOutStream(E_DEBUG); }
    virtual BrowserLoggerStream info() { return getOutStream(E_INFO); }

protected:
    BrowserLogger(int level)
        : m_level(level)
        , m_identifier(s_totalIdentifiers++)
    {

    }

private:
    friend class BrowserLoggerFactory;
    bool isInvalidLevel(int level) { return (level <= E_INFO && level >= E_ERROR); }
    void constructHead(char *head, int level);
    BrowserLoggerStream getOutStream(int level);

    int m_level;
    unsigned int m_identifier;

    static unsigned int s_totalIdentifiers;
};

}

#endif