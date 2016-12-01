#ifndef REGEX_EXCEPTION_H
#define REGEX_EXCEPTION_H

#include <exception>
#include <string>

class LexerException : public std::exception {
    std::string message;
public:
    explicit LexerException (const std::string m);
    const char *what() const noexcept;
    virtual ~LexerException() {}
};
#endif
