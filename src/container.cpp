#include <sstream>
#include <iomanip>
#include "container.h"

std::string repr(unsigned char c) {
    switch (c) {
        case '\0':
            return "\\0";
        case '\a':
            return "\\a";
        case '\b':
            return "\\b";
        case '\t':
            return "\\t";
        case '\n':
            return "\\n";
        case '\v':
            return "\\v";
        case '\f':
            return "\\f";
        case '\r':
            return "\\r";
        case '\\':
            return "\\\\";
        default:
            {
                if (32 < c && c < 127)
                    return std::string(1, c);
                std::ostringstream os;
                os << "\\x" << std::setw(2) << std::setfill('0') << std::hex << (int)c;
                return os.str();
            }
    }
}

std::string repr(const std::string &input) {
    std::string ret;
    for (auto c : input)
        ret.append(repr(c));
    return ret;
}
