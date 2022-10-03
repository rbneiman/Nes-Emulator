
#ifndef EMULATORTEST_UTILITIES_H
#define EMULATORTEST_UTILITIES_H
#include <utility>
#include <vector>
#include <cstdint>
#include <string>
#include <fstream>

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

class DebugLogFile{
private:
    std::vector<uint16_t> pcs;
    std::vector<uint8_t> accs;
    std::vector<uint8_t> xIndexes;
    std::vector<uint8_t> yIndexes;
    std::vector<uint8_t> statuses;
    std::vector<uint8_t> sps;
    std::vector<uint64_t> times;
    std::ifstream inf;
    int lineNum = 0;
    bool endOfFile = false;

//    std::ifstream inf;

    void parseLine(const std::string& line);
public:
    explicit DebugLogFile(const std::string& filePath);

    bool checkLine(int num, uint16_t pc, uint8_t acc,  uint8_t xIndex,  uint8_t yIndex,  uint8_t status, uint8_t sp, uint64_t cycle);

    bool checkTiming(int num, uint32_t cycles);

    void printLine(int num);
};

int checkBit(uint8_t a, uint8_t bit);
int abs(int a);
std::vector<char> loadFile(const std::string& filepath);
bool isOverflowAdd(int8_t a, int8_t b, int8_t result);
bool isOverflowSubtract(int8_t a, int8_t b, int8_t result);
unsigned bitSlice(unsigned num, unsigned high, unsigned low);
#endif //EMULATORTEST_UTILITIES_H
