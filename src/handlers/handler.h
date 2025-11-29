#pragma once

#include <string>
#include <csignal>

#include "../logging/logger.h"
#include "../utils/utils.h"

namespace handler {

    class RequestHandler {
    public:
        RequestHandler() = default;
        explicit RequestHandler(std::string root_path) : root_path_{root_path} {
        }

        RequestHandler(const RequestHandler&) = delete;
        RequestHandler& operator=(const RequestHandler&) = delete;

        template<typename Request, typename Send>
        void operator()(const Request& req, Send&& send){
            utils::Response resp;

            if(req.uri[0] == ('/')) {
                if(req.uri == endpoints::TIME_REQ) {
                    resp.code = "200";
                    resp.status = "OK";
                    resp.body = utils::GetCurrentTime() + ".";
                    send(resp);

                } else if(req.uri == endpoints::STAT_REQ) {
                    resp.code = "200";
                    resp.status = "OK";
                    resp.body = "Total number of connected clients: " + std::to_string(utils::FULL_COUNT_CONNECTION) +
                            "\nNumber of clients currently connected: " + std::to_string(utils::CURRENT_COUNT_CONNECTION) +
                             ".";
                    send(resp);
                } else if (req.uri == endpoints::SHOUTDOWN_REQ) {
                    raise(SIGINT);
                } else {
                    resp.code = "404";
                    resp.status = "Not Found";
                    resp.body = std::string("The requested URL was not found on this server. ") +
                                " At present, the server supports the following URIs:<br>" + 
                                " /time — возврат текущего времени и даты в формате \"2025-11-10 17:28:45\"<br>" +
                                " /stats — возврат статистики (общее количество подключившихся клиентов и подключенных" +
                                " в данный момент)<br>" +
                                " /shutdown — завершение работы.";
                    send(resp);
                }

            } else {
                resp.code = "200";
                resp.status = "OK";
                resp.body = req.uri;
                send(resp);
            }
        }

    private:
        std::string root_path_;
    };

} // namespace handler
