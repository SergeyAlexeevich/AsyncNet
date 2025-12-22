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
                    buffer += "&lt;";
                } else if ('>' == ch) {
                    buffer += "&gt;";
                } else if ('&' == ch) {
                    buffer += "&amp;";
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

    std::string DetectLanguage(const std::string& filepath) {
        // Извлекаем расширение
        size_t pos = filepath.find_last_of(".");
        if (pos == std::string::npos) return "text";  // Нет расширения

        std::string ext = filepath.substr(pos + 1);

        // Сопоставляем расширения с классами Highlight.js
        if (ext == "cpp" || ext == "cc" || ext == "cxx") {
            return "cpp";
        }

        if (ext == "h" || ext == "hpp") {
             return "cpp";
        }

        if (ext == "c") {
            return "c";
        }

        if (ext == "py") {
            return "python";
        }

        if (ext == "js") {

        return "javascript";
        }

        if (ext == "html" || ext == "htm") {
            return "html";
        }
        if (ext == "css") {
            return "css";
        }

        if (ext == "sh" || ext == "bash") {
            return "bash";
        }

        if (ext == "json") {

        return "json";
        }
        
        if (ext == "xml") { return "xml";
        }

        if (ext == "sql") { 
            return "sql";
        }

        return "text";  // Неизвестный язык
    }

    std::string GenerateHtmlResponse(const std::string& code, const std::string& language) {
    return R"(<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <title>)" + language + R"(</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/vs2015.min.css">
</head>
<body>
    <div id="code-container">
        <pre><code class=\")" + 
        language +
        R"(\">
)" +
        code +
        R"(</code></pre>
    </div>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            hljs.highlightAll();
        });
    </script>
</body>
</html>)";
}


} // namespace utils
