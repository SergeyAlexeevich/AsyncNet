#include "tests_sessions.h"
#include "tests_assert.h"
#include "tests_handlers.h"

namespace tests {

    void TestSession() {
        detail::TestBaseSession tbs;
        // Для более гибкой проверки создаем отдельное поле uri
        std::string uri= "/";
        // Делаем имметацию запроса
        std::string imit_req= {"GET "+ uri +" HTTP/1.1\r\n"
            "Host: localhost:8080\r\n"
            "Connection: keep-alive\r\n"
            "Cache-Control: max-age=0\r\n"
            "sec-ch-ua: \"Chromium\";v=\"140\", \"Not=A?Brand\";v=\"24\", \"YaBrowser\";v=\"25.10\","
                " \"Yowser\";v=\"2.5\"\r\n"
            "sec-ch-ua-mobile: ?0\r\n"
            "sec-ch-ua-platform: \"Windows\"\r\n"
            "Upgrade-Insecure-Requests: 1\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"
                " (KHTML, like Gecko) Chrome/140.0.0.0 YaBrowser/25.10.0.0 Safari/537.36\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif"
                ",image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n"
            "Sec-Fetch-Site: none\r\n"
            "Sec-Fetch-Mode: navigate\r\n"
            "Sec-Fetch-User: ?1\r\n"
            "Sec-Fetch-Dest: document\r\n"
            "Accept-Encoding: gzip, deflate, br, zstd\r\n"
            "Accept-Language: ru,en;q=0.9\r\n\r\n"};

        // Проверка работы OnReadRequest
        {
           // Создаем структуру эталонного запроса
            utils::HttpRequest orig_req {
                .method = "GET",
                .uri = "/",
                .http_version = "1.1",
                .headers = {
                    {"Host:", "localhost:8080"}
                    ,{"Connection:", "keep-alive"}
                    ,{"Cache-Control:","max-age=0"}
                    ,{"sec-ch-ua:","\"Chromium\";v=\"140\", \"Not=A?Brand\";v=\"24\", \"YaBrowser\";v=\"25.10\","
                        " \"Yowser\";v=\"2.5\""}
                    ,{"sec-ch-ua-mobile:", "?0"}
                    ,{"sec-ch-ua-platform:","\"Windows\""}
                    ,{"Upgrade-Insecure-Requests:","1"}
                    ,{"User-Agent:","Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"
                        " (KHTML, like Gecko) Chrome/140.0.0.0 YaBrowser/25.10.0.0 Safari/537.36"}
                    ,{"Accept:","text/html,application/xhtml+xml,application/xml;q=0.9,image/avif"
                        ",image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7"}
                    ,{"Sec-Fetch-Site:", "none"}
                    ,{"Sec-Fetch-Mode:", "navigate"}
                    ,{"Sec-Fetch-User:","?1"}
                    ,{"Sec-Fetch-Dest:","document"}
                    ,{"Accept-Encoding:","gzip, deflate, br, zstd"}
                    ,{"Accept-Language:","ru,en;q=0.9"}
                },
                .body = ""
            };
            // Создаем временный канал
            int pipefd[2];
            if(pipe(pipefd) == -1 ){
                std::cerr << "pipe() failed: " << errno << std::endl;
                return;
            }
            // Создаем запись строки
            ssize_t bytes_written = write(pipefd[1], imit_req.c_str(), imit_req.size());
            if (bytes_written == -1) {
                std::cerr << "write() failed: " << errno << std::endl;
                close(pipefd[0]);
                close(pipefd[1]);
                return;
            }

            close(pipefd[1]);
            auto &return_req = tbs.TestBaseOnRead(pipefd[0]);
            close(pipefd[0]);
            
            detail::ASSERT_EQUAL_HINT(orig_req, return_req, "Тестирование чтения");
        }

        // Проверка работы OnWriteResponse
        {
            handler::RequestHandler test_req;
            int sockfd[2];
            if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) == -1){
                std::cerr << "socketpair() failed: " << strerror(errno) << std::endl;
                return;
            }

            std::string current_body = utils::GetCurrentTime();

            utils::Response resp{
                .status = "OK",
                .content_type = "text/html",
                .connection = "keep-alive",
                .code = "200",
                .body = current_body
            };

            std::string orig_out_resp = "HTTP/1.1 " + resp.code + " " + resp.status + "\r\n" 
                "Content-Type: text/html; charset=UTF-8\r\n"
                "Connection: keep-alive\r\n"
                "Content-Length: " + std::to_string(current_body.size() + 1) +
                "\r\n\r\n" + // конец заголовков
                current_body + "\n";

            // Записываем  данные 
            tbs.TestOnWriteResponse(sockfd[1], utils::Response{resp});
            char buf[http_server::BUFFER_SIZE] = {0};
            // Читаем данные
            read(sockfd[0], buf, http_server::BUFFER_SIZE);
            // Проверяем данные
            detail::ASSERT_EQUAL_HINT(orig_out_resp, std::string(buf), "Тестирование записи в сокет при"
                " полноценного ответа возвращенного из обработчика");

            // Проверка при отсутствующем статусе
            try{
            resp.status = "";
            resp.code = "";
            tbs.TestOnWriteResponse(sockfd[1], utils::Response{resp});
            // Если исключение не брошено, значит метод отработал неверно
            std::cerr<<__FILE__ << "(" << __LINE__ << "): " << __FUNCTION__ << ": ошибка тестирования. Hint: "
                << "Тестирование записи записи в сокет при отсутствующем статусе ответа возвращенного из обработчика" << std::endl;
                std::abort();
            } catch (...) {
            }
        }
    }


    void TestHttpServer() {
        detail::RUN_TEST(TestRequestHandler);
        detail::RUN_TEST(TestSession);
        std::cerr <<  "HttpServer testing completed: all tests passed" << std::endl << std::endl;
    }

} // namespace tests