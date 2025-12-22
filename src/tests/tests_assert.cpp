#include "tests_assert.h"

namespace tests {

void AssertImpl (bool value, const std::string& value_str, const std::string& file_name
                    , const std::string& func_name, unsigned number_line, const std::string& hint) {
            if(!value) {
                std::cerr << std::boolalpha;
                std::cerr << file_name << "(" << number_line << "): " << func_name << ": ";
                std:: cerr << "ASSERT(" << value_str << ") failed.";

                if(!hint.empty ()) {
                    std::cerr << " Hint: " << hint;
                }
                std::cerr<<std::endl;
                std::abort ();
            }
}

} // namespace tests
