#ifndef CONTAINER_H
#define CONTAINER_H

#include <list>

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

    bool contain(T c) {
        return this->begin <= c && c <= this->end;
    }
};

#endif
