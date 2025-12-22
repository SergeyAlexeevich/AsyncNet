#include "tests_handlers.h"
#include "tests_assert.h"

namespace tests {

    
    void TestRequestHandler() {
        utils::HttpRequest req {
            .method  = "Get",
            .uri = "/",
            .http_version = "HTTP/1.1",
            .headers {
                {"Connection:","keep-alive"}
            },
            .body = ""
        };
        utils::Response orig_resp{
            .status = "Not Found",
            .code = "404",
            .body = ""
        };
        handler::RequestHandler rh;

        // Тестирование обработки пустой команды "/"
        {
            orig_resp.body = handler::endpoints::COMANDS_LIST;
            utils::Response new_resp;
            rh.operator()(req, [&](utils::Response& resp){
                new_resp = resp;
            });
            detail::ASSERT_EQUAL_HINT(orig_resp, new_resp, "Тестирование обработки пустой команды \"/\"");
        }

        // Тестирование обработки команды "/time"
        {
            orig_resp.body = utils::GetCurrentTime() + ".";
            orig_resp.code = "200";
            orig_resp.status = "OK";
            utils::Response new_resp;
            req.uri = "/time";
            rh.operator()(req, [&](utils::Response& resp){
                new_resp = resp;
            });

            detail::ASSERT_EQUAL_HINT(orig_resp, new_resp, "Тестирование обработки команды \"/time\"");
        }

        // Тестирование обработки команды "/stats"
        {
            orig_resp.body = handler::endpoints::STATISTIC_CONNECTIONS;
            utils::Response new_resp;
            req.uri = "/stats";
            rh.operator()(req, [&](utils::Response& resp){
                new_resp = resp;
            });

            detail::ASSERT_EQUAL_HINT(orig_resp, new_resp, "Тестирование обработки команды \"/stats\"");
        }

        // Тестирование обработки команды "/shutdown"
        {
            // Чтобы не смущать посторонним выводом в консоль, подавляем вывод: переводим поток в состояние fail, 
            // заставляя поток молча отбрасывать все выходные данные до тех пор, пока не будет сброшен флаг сбоя
            std::cout.setstate(std::ios_base::failbit);

            orig_resp.body = "";
            utils::Response new_resp;
            req.uri = "/shutdown";
            rh.operator()(req, [&](utils::Response& resp){
                new_resp = resp;
            });

            // Не забываем сбросить все биты состояния, чтобы восстановить нормальную работу потока
            std::cout.clear();
            detail::ASSERT_HINT(utils::SHUTDOWN_REQUESTED, "Тестирование обработки команды \"/shutdown\","
                " SHUTDOWN_REQUESTED не выставлена в положение false");
            utils::SHUTDOWN_REQUESTED=false;
        }
    
        // Тестирование обработки запроса не являющегося командой (без "/")
        {
            req.uri = "This is test bad command";
            orig_resp.body = req.uri;
            orig_resp.code = "200";
            orig_resp.status = "OK";
            utils::Response new_full_resp;
            rh.operator()(req,[&](utils::Response& resp){
                new_full_resp = resp;
            });

            detail::ASSERT_EQUAL_HINT(orig_resp, new_full_resp, "Тестирование обработки" 
                "не пустого запроса не являющегося командой");

            req.uri.clear();
            orig_resp.body.clear();
            utils::Response new_empty_resp;
            rh.operator()(req,[&](utils::Response& resp){
                new_empty_resp = resp;
            });

            detail::ASSERT_HINT(new_empty_resp.body.empty(), "Тестирование обработки" 
                "пустого запроса не являющегося командой");
        }
    }



} // namespace tests