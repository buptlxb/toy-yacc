#ifndef REGEX_WRITER_H
#define REGEX_WRITER_H

#include "regex_expression.h"

struct RegexNode {
    Expression::Ptr expression;
    explicit RegexNode(Expression::Ptr expr);
    RegexNode oneOrMore(bool isGreedy=true) const;
    RegexNode zeroOrMore(bool isGreedy=true) const;
    RegexNode zeroOrOne(bool isGreedy=true) const;
    RegexNode repeat(int32_t min, int32_t max, bool isGreedy=true) const;
    RegexNode operator+(RegexNode node) const;
    RegexNode operator|(RegexNode node) const;
    RegexNode operator<<=(RegexNode node) const;
    RegexNode operator!() const;
};

extern RegexNode rBegin();
extern RegexNode rEnd();
extern RegexNode rR(char a);
extern RegexNode rR(unsigned char a, unsigned char b);
extern RegexNode rC(char a);
extern RegexNode rC(unsigned char a, unsigned char b);
extern RegexNode rD();
extern RegexNode rL();
extern RegexNode rW();
extern RegexNode rAnyChar();
#endif
