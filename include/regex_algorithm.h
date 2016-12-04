#ifndef REGEX_ALGORITHM_H
#define REGEX_ALGORITHM_H

#include "regex_expression.h"
#include "automaton.h"

struct Visitor {
    virtual void visit(CharRangeExpression *expression) = 0;
    virtual void visit(BeginExpression *expression) = 0;
    virtual void visit(EndExpression *expression) = 0;
    virtual void visit(RepeatExpression *expression) = 0;
    virtual void visit(SetExpression *expression) = 0;
    virtual void visit(ConcatenationExpression *expression) = 0;
    virtual void visit(SelectExpression *expression) = 0;
};

template <typename ReturnType, typename ParameterType>
class RegexVisitor : public Visitor {
private:
    ReturnType returnValue;
    ParameterType *parameterValue;
public:
    ReturnType invoke(Expression *expression, ParameterType parameter) {
        parameterValue = &parameter;
        expression->accept(*this);
        return returnValue;
    }

    ReturnType invoke(typename Expression::Ptr expression, ParameterType parameter) {
        parameterValue = &parameter;
        expression->accept(*this);
        return returnValue;
    }

    virtual ReturnType visit(CharRangeExpression *expression, ParameterType parameter) = 0;
    virtual ReturnType visit(BeginExpression *expression, ParameterType parameter) = 0;
    virtual ReturnType visit(EndExpression *expression, ParameterType parameter) = 0;
    virtual ReturnType visit(RepeatExpression *expression, ParameterType parameter) = 0;
    virtual ReturnType visit(SetExpression *expression, ParameterType parameter) = 0;
    virtual ReturnType visit(ConcatenationExpression *expression, ParameterType parameter) = 0;
    virtual ReturnType visit(SelectExpression *expression, ParameterType parameter) = 0;

    /*virtual*/ void visit(CharRangeExpression *expression) {
        returnValue = visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(BeginExpression *expression) {
        returnValue = visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(EndExpression *expression) {
        returnValue = visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(RepeatExpression *expression) {
        returnValue = visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(SetExpression *expression) {
        returnValue = visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(ConcatenationExpression *expression) {
        returnValue = visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(SelectExpression *expression) {
        returnValue = visit(expression, *parameterValue);
    }
};

template <typename ParameterType>
class RegexVisitor<void, ParameterType> : public Visitor {
private:
    ParameterType *parameterValue;
public:
    void invoke(Expression *expression, ParameterType parameter) {
        parameterValue = &parameter;
        expression->accept(*this);
    }

    void invoke(typename Expression::Ptr expression, ParameterType parameter) {
        parameterValue = &parameter;
        expression->accept(*this);
    }

    virtual void visit(CharRangeExpression *expression, ParameterType parameter) = 0;
    virtual void visit(BeginExpression *expression, ParameterType parameter) = 0;
    virtual void visit(EndExpression *expression, ParameterType parameter) = 0;
    virtual void visit(RepeatExpression *expression, ParameterType parameter) = 0;
    virtual void visit(SetExpression *expression, ParameterType parameter) = 0;
    virtual void visit(ConcatenationExpression *expression, ParameterType parameter) = 0;
    virtual void visit(SelectExpression *expression, ParameterType parameter) = 0;

    /*virtual*/ void visit(CharRangeExpression *expression) {
        visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(BeginExpression *expression) {
        visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(EndExpression *expression) {
        visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(RepeatExpression *expression) {
        visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(SetExpression *expression) {
        visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(ConcatenationExpression *expression) {
        visit(expression, *parameterValue);
    }
    /*virtual*/ void visit(SelectExpression *expression) {
        visit(expression, *parameterValue);
    }
};

class EqualsVisitor : public RegexVisitor<bool, Expression *> {
public:
    /*virtual*/ bool visit(CharRangeExpression *expression, Expression *);
    /*virtual*/ bool visit(BeginExpression *expression, Expression *);
    /*virtual*/ bool visit(EndExpression *expression, Expression *);
    /*virtual*/ bool visit(RepeatExpression *expression, Expression *);
    /*virtual*/ bool visit(SetExpression *expression, Expression *);
    /*virtual*/ bool visit(ConcatenationExpression *expression, Expression *);
    /*virtual*/ bool visit(SelectExpression *expression, Expression *);
};

class GraphvizVisitor : public RegexVisitor<std::string, void *> {
    unsigned long id;
    std::ostream &dot;
    GraphvizVisitor(const GraphvizVisitor &);
public:
    GraphvizVisitor(std::ostream &os, unsigned long x=0);
    /*virtual*/ std::string visit(CharRangeExpression *expression, void *);
    /*virtual*/ std::string visit(BeginExpression *expression, void *);
    /*virtual*/ std::string visit(EndExpression *expression, void *);
    /*virtual*/ std::string visit(RepeatExpression *expression, void *);
    /*virtual*/ std::string visit(SetExpression *expression, void *);
    /*virtual*/ std::string visit(ConcatenationExpression *expression, void *);
    /*virtual*/ std::string visit(SelectExpression *expression, void *);

    static std::string repr(unsigned char c);
};

class SetNormalizationVisitor : public RegexVisitor<void, Range<unsigned char>::List *> {
    Expression::Ptr rebuild(Expression::Ptr, unsigned char begin, unsigned char end);
public:
    /*virtual*/ void visit(CharRangeExpression *expression, Range<unsigned char>::List *);
    /*virtual*/ void visit(BeginExpression *expression, Range<unsigned char>::List *);
    /*virtual*/ void visit(EndExpression *expression, Range<unsigned char>::List *);
    /*virtual*/ void visit(RepeatExpression *expression, Range<unsigned char>::List *);
    /*virtual*/ void visit(SetExpression *expression, Range<unsigned char>::List *);
    /*virtual*/ void visit(ConcatenationExpression *expression, Range<unsigned char>::List *);
    /*virtual*/ void visit(SelectExpression *expression, Range<unsigned char>::List *);
};

class EpsilonNfaVisitor : public RegexVisitor<EpsilonNfa, Automaton *> {
public:
    EpsilonNfa connect(EpsilonNfa, EpsilonNfa, Automaton *);
    /*virtual*/ EpsilonNfa visit(CharRangeExpression *expression, Automaton *);
    /*virtual*/ EpsilonNfa visit(BeginExpression *expression, Automaton *);
    /*virtual*/ EpsilonNfa visit(EndExpression *expression, Automaton *);
    /*virtual*/ EpsilonNfa visit(RepeatExpression *expression, Automaton *);
    /*virtual*/ EpsilonNfa visit(SetExpression *expression, Automaton *);
    /*virtual*/ EpsilonNfa visit(ConcatenationExpression *expression, Automaton *);
    /*virtual*/ EpsilonNfa visit(SelectExpression *expression, Automaton *);
};

#endif
