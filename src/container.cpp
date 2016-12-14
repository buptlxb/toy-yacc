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

void marshalRange(Range<unsigned char> range, Range<unsigned char>::List &ranges) {
    for (auto i = ranges.begin(), iend = ranges.end(); i != iend; ++i) {
        if (range.end < i->begin) {
            ranges.emplace(i, range);
            return;
        } else if (i->end < range.begin)
            ;   
        else if (i->begin < range.begin) {
            unsigned char begin = i->begin;                                                                                                                                                
            i->begin = range.begin;
            i = ranges.emplace(i, begin, range.begin-1);
        } else if (range.begin < i->begin) {
            auto begin = i->begin;
            i = ranges.emplace(i, range.begin, i->begin-1);
            range.begin = begin;
        } else if (i->end < range.end) {
            range.begin = i->end + 1;
        } else if (range.end < i->end) {
            i->begin = range.end+1;
            i = ranges.emplace(i, range.begin, range.end);
            return;
        } else
            return;
    }   
    ranges.push_back(range);
}
