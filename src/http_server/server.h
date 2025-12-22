#pragma once

#include <csignal>
#include <cstring>
#include <errno.h>

#include <arpa/inet.h>
#include <stdexcept>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <unordered_map>

#include "../logging/logger.h"
#include "../utils/utils.h"

namespace http_server {

const int MAX_EVENTS = 100;  // Максимальное количество событий
const int BUFFER_SIZE = 4096; // Максимальный размер буфера

using namespace std::string_literals;

    class BaseSession {
    public:
        explicit BaseSession()
        : client_fd_{}{

        }
        BaseSession(const BaseSession&) = delete;
        BaseSession& operator=(const BaseSession&) = delete;

        virtual int Run(int client_fd) = 0;

        const utils::HttpRequest& GetRuquest() const noexcept{
            return http_request_;
        }

    protected:
        virtual ~BaseSession() = default;

        virtual int OnReadRequest() {
            char buffer[BUFFER_SIZE];
            ssize_t bytes_read = read(client_fd_, buffer, sizeof(buffer));
            if(bytes_read > 0) {
                ParseRequest(buffer);
            }
            return bytes_read;
        }

        virtual int OnWriteResponse(utils::Response&& response) {
            // Подготовка ответа
            if(response.status.empty()){
                logger::LogErr ("Отсутствует статус ответного сообщения ( " + std::string(__FILE__) + std::to_string (__LINE__) + " )");
                throw std::logic_error("Отсутствует статус ответного сообщения");
            } 

            if(response.connection.empty()){
                response.connection = http_request_.headers.at("Connection:");
            }

            if(response.content_type.empty()){
                response.content_type = "text/html";
            }

            std::string resp = CreateResponse(response);
            int bytes_write = send(client_fd_, resp.c_str(), resp.size(), 0);
            if(bytes_write < 0) {
                logger::LogErr (std::string("Ошибка отправки: ") + strerror(errno));
            }
            return bytes_write;
        }

        void SetClientFd(int client_fd) {
            client_fd_ = client_fd;
        }

        int GetClientFd() const {
            return client_fd_;
        }
        
    private:
        void ParseRequest(const std::string &buffer) {
            std::string buf_str;
            std::stringstream ss{buffer};
            bool is_body = false;
            while (std::getline(ss,buf_str)){
                size_t shift = 1; // Смещение для исключения спец. символа
                size_t pos_start = buf_str.find_first_not_of(' ', 0);
                if(auto pos_del = buf_str.find_first_of('\r',0); pos_del != std::string::npos) {
                    buf_str.erase(pos_del);
                }
                // Проверяем начало тела запроса
                if(buf_str == "\r") {
                    is_body = true;
                }

                if(buf_str.empty()){
                    continue;
                }

                // Заполняем первую строку
                if(http_request_.method.empty()) {
                    size_t pos_end = buf_str.find_first_of(' ', pos_start);
                    http_request_.method = buf_str.substr(pos_start, pos_end - pos_start);

                    pos_start = buf_str.find_first_not_of(' ', pos_end);
                    pos_end = buf_str.find_first_of(' ', pos_start);
                    http_request_.uri = buf_str.substr(pos_start, pos_end - pos_start);

                    pos_start = buf_str.find_first_of('/', pos_end) + shift;
                    pos_end = buf_str.size();
                    http_request_.http_version = buf_str.substr(pos_start, pos_end - pos_start);

                // Заполняем тело
                } else if (is_body) {
                    http_request_.body = buf_str;
                // Заполняем карту с заголовками
                } else {
                    size_t pos_delim = buf_str.find_first_of(':', pos_start);
                    size_t space = buf_str.find_first_of(' ', pos_delim);
                    http_request_.headers.emplace(buf_str.substr(pos_start, space - pos_start)
                            , buf_str.substr(space + shift, buf_str.size() - space - shift));
                }
            }
        }

        std::string CreateResponse(const utils::Response& response) const {
            // Формирование HTTP-ответа
            std::string body = response.body + "\n";

            std::string head = std::string("HTTP/") + http_request_.http_version + " " + response.code + " " + response.status + std::string("\r\n") +
                            "Content-Type: " + response.content_type +"; charset=UTF-8\r\n" +
                            "Connection: " + response.connection +"\r\n" +
                            "Content-Length: " + std::to_string(body.size()) + "\r\n" +
                            "\r\n";

            std::string resp = head + body;

            return resp;
        }
        
    private:
        int client_fd_;
        utils::HttpRequest http_request_;
    };


    template <typename RequestHandler>
    class Session : public BaseSession {
    public:
        template <typename Handler>
        Session(Handler&& request_handler)
        : request_handler_(std::forward<Handler>(request_handler)){
        }

        int Run(int client_fd) override {
            SetClientFd(client_fd);
            OnReadRequest();
            int bytes_write = 0;
            request_handler_(GetRuquest(), [&](auto&& response){
                bytes_write = OnWriteResponse(std::forward<utils::Response>(response));
            });
            return bytes_write;
        }

    private:
        RequestHandler request_handler_;
    };

    template<typename RequestHandler>
    class Listener{
    public:
        template<typename Handler>
        Listener(sockaddr_in server_addr, Handler&& request_handler)
            : listen_fd_{socket(AF_INET, SOCK_STREAM, 0)}
            , server_addr_{server_addr}
            , epoll_fd_ {epoll_create1(0)}
            , listen_event_ {}
            , request_handler_ {std::forward<Handler>(request_handler)} {

            // Проверяем создание сокета
            if (listen_fd_ == -1) {
                logger::LogErr ("Ошибка создания сокета ( " + std::string(__FILE__) + std::to_string (__LINE__) + " )");
                throw std::logic_error("Error create socket"s);
            }

            // Проверяем создание Epoll
            if (epoll_fd_ == -1) {
                logger::LogErr ("Ошибка создания epoll ( " + std::string(__FILE__) + std::to_string (__LINE__) + " )");
                close(listen_fd_);
                throw std::logic_error("Error create epoll's"s);
            }

            // Задаем низкоуровневые настройки сокета, чтобы было возможно (за счет SO_REUSEADDR) привязать сокет к порту,
            // если он находится в TIME_WAIT. Т.к. пока порт находится в TIME_WAIT привязать к нему сокет нельзя.
            // SO_REUSEADDR - специальная константа, обозначающая «разрешить повторное использование адреса/порта».
            int optval = 1;
            if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
                logger::LogErr ("Ошибка установки SO_REUSEADDR ( " + std::string(__FILE__) + std::to_string (__LINE__) + " )");
            }
            // Привязываем сокет к адресу и порту
            if (bind(listen_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
                logger::LogErr ("Ошибка привязки сокета ( " + std::string(__FILE__) + std::to_string (__LINE__) + " )");
                close(listen_fd_);
                throw std::logic_error("Error connections socket"s);
            }

            // Начинаем слушать подключения
            if (listen(listen_fd_, MAX_EVENTS) == -1) {
                logger::LogErr ("Ошибка начала прослушивания ( " + std::string(__FILE__) + std::to_string (__LINE__) + " )");
                close(listen_fd_);
                throw std::logic_error("Error start listening"s);
            }

            // Добавляем слушающий сокет в epoll
            listen_event_.events = EPOLLIN;
            listen_event_.data.fd = listen_fd_;

            if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &listen_event_) == -1) {
                logger::LogErr ("Ошибка добавления слушающего сокета в epoll ( " + std::string(__FILE__) + std::to_string (__LINE__) + " )");
                close(listen_fd_);
                close(epoll_fd_);
                throw std::logic_error("Error added listening socket in epoll's");
            }

            logger::LogInfo ("Сервер запущен на порту: " + std::to_string (ntohs(server_addr_.sin_port)) + "...");
            std::cout << "Сервер запущен на порту: " << ntohs(server_addr_.sin_port) << "..." << std::endl;
          }

        void Run(){
            // Бесконечный цикл обработки событий
            while (!utils::SHUTDOWN_REQUESTED) {
                struct epoll_event events[MAX_EVENTS];
                int event_count = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);

                if (event_count == -1) {
                    if (utils::SHUTDOWN_REQUESTED) {
                        // Прервано сигналом — продолжаем цикл, проверка shutdown_requested произойдёт на следующей итерации
                        continue;
                    }

                    logger::LogErr ("Ошибка epoll_wait ( " + std::string(__FILE__) + std::to_string (__LINE__) + " )");
                    break;
                }

                for (int i = 0; i < event_count; i++) {
                    int fd = events[i].data.fd;

                    if (fd == listen_fd_) {
                        OnAccept();
                    } else {
                        HandleClientEvent(fd, events[i].events);
                    }
                }
            }

        // Освобождаем ресурсы
        close(listen_fd_);
        close(epoll_fd_);
        logger::LogInfo ("Сервер остановлен.");
        std::cout << "Сервер остановлен."<<std::endl;
        }

        void Stop() {
            raise(SIGINT);
        }

    private:
        void OnAccept() {
            // Обработка новых подключений
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(listen_fd_, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);

            if (client_fd == -1) {
                logger::LogErr ("Ошибка accept ( " + std::string(__FILE__) + std::to_string (__LINE__) + " )");
                return;
            }

            // Добавляем клиентский сокет в epoll
            struct epoll_event client_event;
            client_event.events = EPOLLIN;
            client_event.data.fd = client_fd;
            if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
                logger::LogErr ("Ошибка добавления клиентского сокета в epoll ( " + std::string(__FILE__) + std::to_string (__LINE__) + " )");
                close(client_fd);
            }

            logger::LogInfo ("Новое подключение от " + std::string (inet_ntoa(client_addr.sin_addr)) + ":" + std::to_string (ntohs(client_addr.sin_port)));
            std::cout << "Новое подключение от " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;

            ++utils::FULL_COUNT_CONNECTION;
            ++utils::CURRENT_COUNT_CONNECTION;
        }

        void HandleClientEvent(int fd, uint32_t events) {
            // Проверяем состояние сокета
            if (events & (EPOLLERR | EPOLLHUP)) {
                DelElemFromEpoll(fd);
                return;
            }

            Session<RequestHandler> session{request_handler_};
            int bytes_session = session.Run(fd);
            if (bytes_session > 0) {
                // Успешно обработано (отправлено/получено bytes_session байт)
                // Ничего не делаем — ждём следующих событий
                return;
            } else if (bytes_session == 0) {
                // Клиент закрыл соединение
                DelElemFromEpoll(fd);

            } else {
                // Ошибка чтения
                logger::LogErr ("Ошибка чтения: " + std::string(strerror(errno)) + " ( " + std::string(__FILE__) + std::to_string (__LINE__) + " )");
                DelElemFromEpoll(fd);
            }
        }

        void DelElemFromEpoll(int fd){
            // Сначала удаляем из epoll (чтобы не получать события)
            if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
                logger::LogErr ("Предупреждение: не удалось удалить fd="s + std::to_string (fd )
                                + " из epoll: " + strerror(errno) +" ( "
                        + std::string(__FILE__) + std::to_string (__LINE__) + " )");
            }

            // Закрываем сокет
            if (close(fd) == -1) {
                logger::LogErr ("Ошибка при закрытии fd="s + std::to_string (fd)
                        + ": " + strerror(errno) +" ( "
                        + std::string(__FILE__) + std::to_string (__LINE__) + " )");
            }
            --utils::CURRENT_COUNT_CONNECTION;
        }

    private:
    int listen_fd_;
    struct sockaddr_in server_addr_;
    int epoll_fd_;
    struct epoll_event listen_event_;
    RequestHandler request_handler_;
    };


    template <typename RequestHandler>
    void ServerHttp(sockaddr_in server_addr, RequestHandler &&handler){
        // Исключим ссылки из типа RequestHandler,
        // чтобы Listener хранил RequestHandler по значению
        using MyListener = Listener<std::decay_t<RequestHandler>>;
        MyListener listener (server_addr, std::forward<RequestHandler>(handler));
        listener.Run();
    }



} // http_server
