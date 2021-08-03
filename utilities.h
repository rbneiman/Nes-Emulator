
#ifndef EMULATORTEST_UTILITIES_H
#define EMULATORTEST_UTILITIES_H
#include <utility>
#include <vector>
#include <cstdint>

typedef enum{
    NONE = 0,
    NON_NES_FILE = 1,
    NON_SUPPORTED_CONSOLE = 2
}error_code_t;

class ErrorMessage{
private:
    const error_code_t errorCode;
    const std::string message;
public:
    ErrorMessage(const error_code_t errorCode, std::string message) : errorCode(errorCode), message(std::move(message)) {}

    error_code_t getErrorCode() const {
        return errorCode;
    }

    const std::string &getMessage() const {
        return message;
    }

    explicit operator bool() const {
        return errorCode;
    }
};

int checkBit(uint8_t a, uint8_t bit);
int abs(int a);
std::vector<char> loadFile(const std::string& filepath);

#endif //EMULATORTEST_UTILITIES_H
