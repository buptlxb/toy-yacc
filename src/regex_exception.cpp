#include "regex_exception.h"

LexerException::LexerException(const std::string m) : message(m) {}

const char *LexerException::what() const noexcept {
    return message.c_str();
}
