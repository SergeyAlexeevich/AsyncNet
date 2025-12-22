#pragma once

namespace tests {
    /**
     * @brief Тестирует методы
     * OnReadRequest и OnWriteResponse класса Session
     * 
     */
    void TestSession();
    
    /**
     * @brief Тестирует сессию и обработчик
     * запуская TestSession и TestRequestHandler
     */
    void TestHttpServer();

} // namespace tests