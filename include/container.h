#ifndef CONTAINER_H
#define CONTAINER_H

#include <list>
#include <string>

template <typename T>
struct Range {
    typedef std::list<Range<T>> List;
    T begin;
    T end;
    Range(T b, T e) : begin(b), end(e) {}
    Range() : begin('\x00'), end('\x00') {}

    bool operator== (const Range<T> &that) {
        return this->begin == that.begin && this->end == that.end;
    }

    bool operator!= (const Range<T> &that) {
        return !(*this == that);
    }

    bool contains(T c) {
        return this->begin <= c && c <= this->end;
    }
};

extern std::string repr(unsigned char c);
extern std::string repr(const std::string &input);
extern void marshalRange(Range<unsigned char> range, Range<unsigned char>::List &ranges);
#endif
