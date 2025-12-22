#include "utils.h"

#include <chrono>
#include <errno.h>
#include <cstring>
#include <iomanip>
#include <sstream>

#include <iostream>

namespace utils {

    std::string GetCurrentTime(){
       auto now = std::chrono::system_clock::now();
       std::stringstream ss;
       std::time_t time_now = std::chrono::system_clock::to_time_t(now);
       ss << std::put_time(std::localtime(&time_now), "%Y-%m-%d %H:%M:%S");
       return ss.str();
    }

    std::string GetDocument(std::string path) {
        const auto not_found = std::string::npos;
        std::fstream in_file(path, std::ios::in);
        if(!in_file.is_open()) {
            return "Error open file: " + std::string(strerror(errno));
        }

        size_t size_file = GetFileSize(in_file);

        std::string buffer;
        buffer.reserve(size_file * 2);
        std::string temp;
        while(std::getline(in_file,temp)){
            if(StrOnlyEscapeSequences(temp)) {
                buffer += "<br>";
                continue;
            }

            for(auto ch : temp) {
                if('<' == ch){
                    buffer += "&lt";
                } else if ('>' == ch) {
                    buffer += "&gt";
                } else {
                    buffer += ch;
                }
            }
            
            buffer += "<br>";
        }

        return buffer;
    }

    size_t GetFileSize(std::fstream &file) {
        if(!file.is_open()) {
            throw std::runtime_error("File is not open: " + std::string(strerror(errno)));
        }
        file.seekg(0, std::ios::end);
        size_t size_file = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);
        return size_file;
    }

    bool StrOnlyEscapeSequences(std::string str) {
        for(auto ch : str){
            if(!escapes.count(ch)){
                return false;
            }
        }
        return true;
    }

} // namespace utils
