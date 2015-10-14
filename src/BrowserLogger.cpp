/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include <stdio.h>
#include "include/BrowserLogger.h"
#include "base/time.h"

namespace LightBrowser {

unsigned int BrowserLogger::s_totalIdentifiers = 0;

BrowserLoggerStream BrowserLogger::getOutStream(int level)
{
#if !defined(USE_TAF_LOGGER)
    bool m_isOut = false;
    std::string header;
    if (level <= m_level) {
        char head[120] = { 0 };
        constructHead(head, level);
        header = head;
        m_isOut = true;
    }

    return BrowserLoggerStream(m_isOut, header);
#else
    if (level > m_level) // do not out
        return taf::TafRollLogger::getInstance()->logger()->info();

    switch (level) {
    case E_ERROR:
        return taf::TafRollLogger::getInstance()->logger()->error(); 
    case E_WARN:
        return taf::TafRollLogger::getInstance()->logger()->warn();
    case E_DEBUG:
        return taf::TafRollLogger::getInstance()->logger()->debug();
    case E_INFO:
    default:
        return taf::TafRollLogger::getInstance()->logger()->info();
    }
#endif
}

void BrowserLogger::constructHead(char *head, int level)
{
    base::Time::Exploded exploded;
    base::Time::Now().LocalExplode(&exploded);

    sprintf(head, "[%04d-%02d-%02d %02d:%02d:%02d]%d|%u|", exploded.year, exploded.month, exploded.day_of_month,
        exploded.hour, exploded.minute, exploded.second, exploded.millisecond, m_identifier);

    int length = strlen(head);

    switch (level) {
    case E_ERROR:
        sprintf(head + length, "%s|", "error");
        break;
    case E_WARN:
        sprintf(head + length, "%s|", "warn");
        break;
    case E_DEBUG:
        sprintf(head + length, "%s|", "debug");
        break;
    case E_INFO:
        sprintf(head + length, "%s|", "info");
        break;
    default:
        break;
    }
}

}
