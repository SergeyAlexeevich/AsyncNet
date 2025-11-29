#include "logger.h"

namespace logger {

        bool LoggerSingleton::IS_INITIALIZATION_ = false;
    std::string LoggerSingleton::NAME_PROG_;

    LoggerSingleton::Logger::Logger(std::string name_prog) : name_prog_{name_prog} {
        openlog(name_prog_.c_str(), LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "%s", "Сервер запущен");
    }

    void LoggerSingleton::Logger::LogInfo(const std::string &mes){
        syslog(LOG_INFO, "%s", mes.c_str());
    }

    void LoggerSingleton::Logger::LogWar(const std::string &mes){
        std::string message = "Предупреждение:" + mes;
        syslog(LOG_WARNING, "%s", message.c_str());
    }

    void LoggerSingleton::Logger::LogErr(const std::string &mes){
        std::string message = "Ошибка:" + mes;
        syslog(LOG_ERR, "%s", message.c_str());
    }

    LoggerSingleton::Logger::~Logger(){
        syslog(LOG_INFO, "%s", "Сервер остановлен");
        closelog();
    }

    LoggerSingleton &LoggerSingleton::GetInstance(std::string name_prog) {
        if(!IS_INITIALIZATION_ && name_prog.empty()) {
            throw std::logic_error("Logger not initialized!\nCall InitLog(std::string name_prog)"
                "by passing the parameter name_prog.");

        } else if(IS_INITIALIZATION_ && (!name_prog.empty() && NAME_PROG_ != name_prog)) {
            throw std::logic_error("Logger has already been initialized!");

        } else if(IS_INITIALIZATION_ && name_prog.empty()) {
            name_prog = NAME_PROG_;
        }

        return GetInstanceImpl(name_prog);
    }

    void LoggerSingleton::LogInfo(const std::string &mes) {
        logger_.LogInfo(mes);
    }

    void LoggerSingleton::LogWar(const std::string &mes) {
        logger_.LogWar(mes);
    }

    void LoggerSingleton::LogErr(const std::string &mes) {
        logger_.LogErr(mes);
    }

    LoggerSingleton::LoggerSingleton(std::string name_prog)
    : logger_ {
                name_prog.empty() ? "DEFAULT_APP" : name_prog
    } {
    }

    LoggerSingleton &LoggerSingleton::GetInstanceImpl(std::string name_prog) {
        static LoggerSingleton instance(name_prog);
        IS_INITIALIZATION_ = true;
        if(!name_prog.empty()) {
            NAME_PROG_ = name_prog;
        }
        return instance;
    }

    LoggerSingleton& InitLog(const std::string &name_prog) {
       return LoggerSingleton::GetInstance(name_prog);
   }

    void LogInfo(const std::string& mes) {
        LoggerSingleton::GetInstance().LogInfo(mes);
    }

    void LogWar(const std::string& mes) {
         LoggerSingleton::GetInstance().LogWar(mes);
    }

    void LogErr(const std::string& mes) {
        LoggerSingleton::GetInstance().LogErr(mes);
    }
} // namespace logger
