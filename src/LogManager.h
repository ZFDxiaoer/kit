#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <string>
#include <sstream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "util_singleton.h"

using namespace std;

class LogManager
{
    public:
        static uint32_t init();
        static void addLog(const string& str);
};

#define GLOBALLOG(x) do \
    { \
        ostringstream _internal_ss; \
        _internal_ss << x << endl; \
        LogManager::addLog(_internal_ss.str()); \
    } while (0)

#define GLOBALDEBUG(x) do \
    { \
        GLOBALLOG("DEBUG" << "|" << x); \
    } while (0)

#define GLOBALINFO(x) do \
    { \
        GLOBALLOG("INFO" << "|" << x); \
    } while (0)

#define GLOBALERROR(x) do \
    { \
        GLOBALLOG("ERROR" << "|" << x); \
    } while (0)

#endif