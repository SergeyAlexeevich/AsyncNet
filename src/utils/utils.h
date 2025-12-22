#pragma once

#include <fstream>
#include <string>
#include <string_view>
#include <map>
#include <unordered_set>

namespace utils {
    static const std::unordered_set<char> escapes = {
        '\n', '\t', '\r', '\b', '\f', '\v', '\\', '\"', '\'',
        '\0', '\a', '\e'
    };

    inline unsigned FULL_COUNT_CONNECTION = 0;
    inline unsigned CURRENT_COUNT_CONNECTION = 0;
    inline bool SHUTDOWN_REQUESTED = false;

    std::string DetectLanguage(const std::string& filepath);
    std::string GenerateHtmlResponse(const std::string& code, const std::string& language);
    // std::string GenerateHtmlResponse(const std::string& code, const std::string& language, const std::string& filepath = "");
    std::string GetCurrentTime();
    std::string GetDocument(std::string path);

    size_t GetFileSize(std::fstream& file);

    bool StrOnlyEscapeSequences(std::string str);

    struct HttpRequest{
        std::string method;
        std::string uri;
        std::string http_version;
        std::map<std::string, std::string> headers;
        std::string body;

        bool operator==(const HttpRequest& rhs) const {
            return method == rhs.method && uri == rhs.uri
            && http_version == rhs.http_version
            && headers == rhs.headers && body == rhs.body;
        }

        bool operator!=(const HttpRequest& rhs) const {
            return !(*this == rhs);
        }
    };

    struct Response {
        std::string status;
        std::string content_type;
        std::string content_length;
        std::string connection;
        std::string code;
        std::string message;
        std::string body;

        bool operator==(const Response& rhs) const {
            return status == rhs.status && content_type == rhs.content_type
                && content_length == rhs.content_length && connection == rhs.connection
                && code == rhs.code && message == rhs.message && body == rhs.body;
        }

        bool operator!=(const Response rhs) const {
            return !(*this == rhs);
        }
    };

} // namespace utils
