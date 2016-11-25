#ifndef REGEX_EXPRESSION_H
#define REGEX_EXPRESSION_H

#include <cstdint>
#include <memory>
//#include "regex_algorithm.h"
class Visitor;

struct Expression {
    typedef std::shared_ptr<Expression> Ptr;
    bool equals(Expression *);
    virtual void accept(Visitor &) = 0;
};

struct CharRangeExpression : public Expression {
    char begin;
    char end;
    CharRangeExpression();
    CharRangeExpression(char b, char e);
    /*virtual*/ void accept(Visitor &);
};

struct BeginExpression : public Expression {
    /*virtual*/ void accept(Visitor &);
};

struct EndExpression : public Expression {
    /*virtual*/ void accept(Visitor &);
};

struct RepeatExpression : public Expression {
    Expression::Ptr expression;
    int32_t min;
    int32_t max;
    bool isGreedy;
    /*virtual*/ void accept(Visitor &);
};

struct SetExpression : public Expression {
    Expression::Ptr expression;
    bool isComplementary;
    /*virtual*/ void accept(Visitor &);
};

struct ConcatenationExpression : public Expression {
    Expression::Ptr left;
    Expression::Ptr right;
    /*virtual*/ void accept(Visitor &);
};

struct SelectExpression : public Expression {
    Expression::Ptr left;
    Expression::Ptr right;
    /*virtual*/ void accept(Visitor &);
};

std::string repr(unsigned char c);
std::string repr(const std::string &input);
extern bool isChar(const char *&input, char);
bool isChars(const char *&input, std::string chars);
extern char parseChar(const char *&input);
extern Expression::Ptr parseSetItem(const char *&input);
extern Expression::Ptr parseSetItems(const char *&input);
extern Expression::Ptr parseElementaryRE(const char *&input);
extern Expression::Ptr parseBasicRE(const char *&input);
extern Expression::Ptr parseSimpleRE(const char *&input);
extern Expression::Ptr parseRE(const char *&input);
extern Expression::Ptr parseRegex(const std::string &str);
#endif
