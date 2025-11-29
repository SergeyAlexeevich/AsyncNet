#include <csignal>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <thread>

#include "http_server/server.h"
#include "handlers/handler.h"
#include "logging/logger.h"
#include "tests/tests.h"

using namespace std::string_literals;

// Обработчик сигнала
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        //shutdown_requested = true;
        utils::SHUTDOWN_REQUESTED = true;
        std::cout << "\nПолучен сигнал "s << signal << ", завершение работы..."s << std::endl;
    }
}

int main(int argc, const char* argv[]) {
    // 0. Получение доступного количества потоков
    const unsigned num_threads = std::thread::hardware_concurrency();
    // регистрация обработчика сигналов SIGINT и SIGTERM
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, nullptr) == -1 ||
        sigaction(SIGTERM, &sa, nullptr) == -1) {
        std::cerr << "Ошибка установки обработчика сигналов"s << std::endl;
        return 1;
    }

    // Запускаем тесты
    tests::TestHttpServer();

    // 1. Создаем структуру адреса
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;           // Указываем семейство протокола
    server_addr.sin_addr.s_addr = INADDR_ANY;   // Слушаем на всех интерфейсах
    server_addr.sin_port = htons(8080);         // порт в сетевом порядке байт

    // 2. Создаем логер
    // Получаем наименование программы
    std::string name_prog = argv[0];
    size_t pos_start_name = name_prog.find_last_of('/') + 1;
    size_t count_ch = name_prog.size() - pos_start_name;
    name_prog = name_prog.substr(pos_start_name, count_ch);
    logger::InitLog(name_prog);

    // 3. Создаем экзампляр обработчика
    handler::RequestHandler handler;
    // 4. Создаем экземпляр сервера
    http_server::ServerHttp(server_addr, [&handler](auto&& request, auto&& send) {
        handler.operator()(std::forward<decltype(request)>(request), std::forward<decltype(send)>(send));
    });

    return 0;
}
