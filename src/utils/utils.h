#pragma once

#include <string>
#include <string_view>
#include <map>

namespace utils {

    inline unsigned FULL_COUNT_CONNECTION = 0;
    inline unsigned CURRENT_COUNT_CONNECTION = 0;
    inline bool SHUTDOWN_REQUESTED = false;

    std::string GetCurrentTime();

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

namespace endpoints {
    constexpr std::string_view TIME_REQ = "/time";
    constexpr std::string_view STAT_REQ = "/stats";
    constexpr std::string_view SHOUTDOWN_REQ = "/shutdown";
} // namespace endpoints
