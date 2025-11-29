#include "utils.h"

#include <chrono>
#include <iomanip>
#include <sstream>


namespace utils {

    std::string GetCurrentTime(){
       auto now = std::chrono::system_clock::now();
       std::stringstream ss;
       std::time_t time_now = std::chrono::system_clock::to_time_t(now);
       ss << std::put_time(std::localtime(&time_now), "%Y-%m-%d %H:%M:%S");
       return ss.str();
    }

} // namespace utils
