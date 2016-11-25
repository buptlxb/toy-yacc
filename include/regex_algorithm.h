#ifndef REGEX_ALGORITHM_H
#define REGEX_ALGORITHM_H

#include "regex_expression.h"
#include <string>

// class Expression;
// class CharRangeExpression;
// class BeginExpression;
// class EndExpression;
// class RepeatExpression;
// class SetExpression;
// class ConcatenationExpression;
// class SelectExpression;

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
public:
    GraphvizVisitor() : id(0) {}
    /*virtual*/ std::string visit(CharRangeExpression *expression, void *);
    /*virtual*/ std::string visit(BeginExpression *expression, void *);
    /*virtual*/ std::string visit(EndExpression *expression, void *);
    /*virtual*/ std::string visit(RepeatExpression *expression, void *);
    /*virtual*/ std::string visit(SetExpression *expression, void *);
    /*virtual*/ std::string visit(ConcatenationExpression *expression, void *);
    /*virtual*/ std::string visit(SelectExpression *expression, void *);

    static std::string repr(char c);
};

#endif
