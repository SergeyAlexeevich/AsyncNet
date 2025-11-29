#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <syslog.h>

namespace logger {

class LoggerSingleton {
    class Logger {
    public:
        Logger(std::string name_prog);

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;

        void LogInfo(const std::string& mes);

        void LogWar(const std::string& mes);

        void LogErr(const std::string& mes);

        ~Logger();
    private:
        std::string name_prog_;
    };

public:
    // Инициализируем и получаем единственный экземпляр
    static LoggerSingleton& GetInstance(std::string name_prog = "");

    // Обёртки для вызова
    void LogInfo(const std::string& mes);

    void LogWar(const std::string& mes);

    void LogErr(const std::string& mes);

private:
    LoggerSingleton(std::string name_prog);

    static LoggerSingleton& GetInstanceImpl(std::string name_prog);

private:
    Logger logger_;
    static bool IS_INITIALIZATION_;
    static std::string NAME_PROG_;
};

    // Глобальные обёртки
    LoggerSingleton& InitLog(const std::string& name_prog);
    void LogInfo(const std::string& mes);

    void LogWar(const std::string& mes);

    void LogErr(const std::string& mes);

} // namespace loger
