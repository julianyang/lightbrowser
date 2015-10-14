/*
Module: LightBrowser
Copyright @tencent 2014
@author: julianyang
*/

#include <stdio.h>
#include "include/BrowserLogger.h"
#include "base/time.h"

namespace LightBrowser {

unsigned int SimpleBrowserLogger::s_totalIdentifiers = 0;

BrowserLoggerStream SimpleBrowserLogger::getOutStream(int level)
{
    std::ostream *outStream = NULL;

    if (level <= m_level) {
        char head[120] = { 0 };
        constructHead(head, level);
        outStream = &std::cout;
        outStream->clear();
        *outStream << head;
    }

    return BrowserLoggerStream(outStream);
}

void SimpleBrowserLogger::constructHead(char *head, int level)
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
