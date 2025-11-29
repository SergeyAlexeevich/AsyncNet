#pragma once

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../handlers/handler.h"
#include "../utils/utils.h"

namespace tests {

    namespace detail {

        template <typename K, typename V>
        std::ostream& operator<<(std::ostream& output, const std::unordered_map<K, V>& items) {
            output << "{";
            bool first_item = true;
            for (const auto& [key, value] : items) {
                if (!first_item) {
                    output << ", ";
                }
                output << key << ": " << value;
                first_item = false;
            }
            output << "}";
            return output;
        } 

        inline std::ostream& operator<<(std::ostream& output, const utils::Response& resp) {
            output << "Response {\n"
            << "  status: '" << resp.status << "'\n"
            << "  content_type: '" << resp.content_type << "'\n"
            << "  content_length: '" << resp.content_length << "'\n"
            << "  connection: '" << resp.connection << "'\n"
            << "  code: '" << resp.code << "'\n"
            << "  message: '" << resp.message << "'\n"
            << "  body: '" << resp.body << "'\n"
            << "}";
            return output;
        }

        inline std::ostream& operator<<(std::ostream& output, const utils::HttpRequest& req) {
            output << "Request {\n"
            << "  Method: '" << req.method << "'\n"
            << "  uri: '" << req.uri << "'\n"
            << "  http_version: '" << req.http_version << "'\n";
            for(const auto [key,val] : req.headers){
                output <<key << " '" << val << "'\n";
            }
            output<< "  body: '" << req.body << "'\n"
            << "}";
            return output;
        }

        template <typename First, typename Second>
        void AssertEqualImpl (const First& f_value, const Second& s_value
                            , const std::string& f_value_str, const std::string& s_value_str
                            , const std::string& file_name, const std::string& func_name
                            , unsigned number_line, const std::string& hint) {
            if(f_value != s_value) {
                std::cerr << std::boolalpha;
                std::cerr << file_name << "(" << number_line << "): " << func_name << ": ";
                std::cerr << "ASSERT_EQUAL(" << f_value_str << ", " << s_value_str << ") failed: ";
                std::cerr << f_value << " != " << s_value << ".";

                if(!hint.empty ()) {
                    std::cerr << " Hint: " << hint;
                }
                std::cerr<<std::endl;
                std::abort ();
            }
        }
    #define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, "")
    #define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

    void AssertImpl (bool value, const std::string& value_str, const std::string& file_name
                        , const std::string& func_name, unsigned number_line, const std::string& hint);
    #define ASSERT(exp) AssertImpl(!!(exp), #exp, __FILE__, __FUNCTION__, __LINE__, "")
    #define ASSERT_HINT(exp, hint) AssertImpl(!!(exp), #exp, __FILE__, __FUNCTION__, __LINE__, (hint))

        template <typename TestFunc>
        void RunTestImpl(const TestFunc& func, const std::string& test_name) {
            func();
            std::cerr<< "Testing " + test_name + " is successfully" << std::endl;
        }
    #define RUN_TEST(func) RunTestImpl(func, #func)

    } // namespace detail

    void TestHttpServer();

} // namespace tests
