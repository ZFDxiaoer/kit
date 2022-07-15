#include "LogManager.h"

uint32_t LogManager::init()
{
    auto file_logger = spdlog::basic_logger_mt("log", "log/michatroom.log");
    spdlog::set_default_logger(file_logger);
    spdlog::flush_every(std::chrono::seconds(1));
    return 0;
}

void LogManager::addLog(const string& str)
{
    spdlog::info(str);
}