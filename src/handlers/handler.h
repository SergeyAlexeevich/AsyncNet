#pragma once

#include <string>
#include <string_view>
#include <csignal>

#include "../logging/logger.h"
#include "../utils/utils.h"

namespace handler {

    using namespace std::string_literals;

    namespace endpoints {
        inline constexpr std::string_view TIME_REQ = "/time";
        inline constexpr std::string_view STAT_REQ = "/stats";
        inline constexpr std::string_view DOCUMENTS = "/documents";
        inline constexpr std::string_view SHOUTDOWN_REQ = "/shutdown";

        inline constexpr std::string_view COMANDS_LIST = "<b>В настоящее время сервер находится"
                                                                " в режиме разработки и поддерживате следующие URIs:</b><br>" 
                                "<b>/time</b> — возврат текущего времени и даты в формате \"2025-11-10 17:28:45,\"<br>"
                                "<b>/stats</b> — возврат статистики (общее количество подключившихся клиентов и подключенных"
                                " в данный момент),<br>"
                                "<b>/documents</b> — получение списка документов<br>"
                                "<b>/shutdown</b> — завершение работы.";
        inline std::string STATISTIC_CONNECTIONS = "Total number of connected clients: "s + std::to_string(utils::FULL_COUNT_CONNECTION) +
                            "\nNumber of clients currently connected: " + std::to_string(utils::CURRENT_COUNT_CONNECTION) +
                             ".";
        inline std::string VECTOR_IMPL = "/vector_impl";
    } // namespace endpoints

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
                    resp.body = endpoints::STATISTIC_CONNECTIONS;
                    send(resp);
                } else if (req.uri == endpoints::SHOUTDOWN_REQ) {
                    raise(SIGINT);
                } else if (req.uri == endpoints::DOCUMENTS) {
                    resp.code = "200";
                    resp.status = "OK";
                    resp.body = "<a href=vector_impl target=\"_blank\">Собственная реализация верктора</a>";
                    send(resp);
                } else if(req.uri == endpoints::VECTOR_IMPL) {
                    resp.code = "200";
                    resp.status = "OK";
                    std::string buf = utils::GetDocument("/home/sergey/Work/AsyncNet/static/vector.txt");
                    resp.body = buf;
                    send(resp);
                } else {
                    resp.code = "404";
                    resp.status = "Not Found";
                    resp.body = endpoints::COMANDS_LIST;
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
