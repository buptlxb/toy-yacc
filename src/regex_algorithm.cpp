#include "regex_algorithm.h"
#include <sstream>
#include <iostream>


bool EqualsVisitor::visit(CharRangeExpression *expression, Expression *target) {
    CharRangeExpression *that = dynamic_cast<CharRangeExpression *>(target);
    if (!that)
        return false;
    return expression->begin == that->begin && expression->end == that->end;
}

bool EqualsVisitor::visit(BeginExpression *expression, Expression *target) {
    return dynamic_cast<BeginExpression *>(target);
}

bool EqualsVisitor::visit(EndExpression *expression, Expression *target) {
    return dynamic_cast<EndExpression *>(target);
}

bool EqualsVisitor::visit(RepeatExpression *expression, Expression *target) {
    RepeatExpression *that = dynamic_cast<RepeatExpression *>(target);
    if (!that)
        return false;
    if (expression->min != that->min || expression->max != that->max || expression->isGreedy != that->isGreedy)
        return false;
    return invoke(expression->expression, that->expression.get());
}

bool EqualsVisitor::visit(SetExpression *expression, Expression *target) {
    SetExpression *that = dynamic_cast<SetExpression *>(target);
    if (!that)
        return false;
    if (expression->isComplementary != that->isComplementary)
        return false;
    return invoke(expression->expression, that->expression.get());
}

bool EqualsVisitor::visit(ConcatenationExpression *expression, Expression *target) {
    ConcatenationExpression *that = dynamic_cast<ConcatenationExpression *>(target);
    if (!that)
        return false;
    return invoke(expression->left, that->left.get()) && invoke(expression->right, that->right.get());
}

bool EqualsVisitor::visit(SelectExpression *expression, Expression *target) {
    SelectExpression *that = dynamic_cast<SelectExpression *>(target);
    if (!that)
        return false;
    return invoke(expression->left, that->left.get()) && invoke(expression->right, that->right.get());
}

std::string GraphvizVisitor::visit(CharRangeExpression *expression, void *) {
    std::ostringstream os;
    os << "CharRange_" << id++;
    std::string name = os.str();
    std::cout << name << " [ label=\"" << repr(expression->begin) << "-" << repr(expression->end) << "\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(BeginExpression *expression, void *) {
    std::ostringstream os;
    os << "Begin_" << id++;
    std::string name = os.str();
    std::cout << name << " [ label=\"BEGIN\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(EndExpression *expression, void *) {
    std::ostringstream os;
    os << "End_" << id++;
    std::string name = os.str();
    std::cout << name << " [ label=\"END\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(RepeatExpression *expression, void *) {
    std::ostringstream os;
    os << "Repeat_" << id++;
    std::string name = os.str(), target = invoke(expression->expression, std::cout);
    std::cout << name << "->" << target << '\n';

    std::cout << name << " [ label=\"";
    if (expression->isGreedy)
        std::cout << "Greedy";
    std::cout << "{" << expression->min << "-";
    if (expression->max < 0)
        std::cout << "INF";
    else
        std::cout << expression->max;
    std::cout << "}\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(SetExpression *expression, void *) {
    std::ostringstream os;
    os << "Set_" << id++;
    std::string name = os.str(), target = invoke(expression->expression, std::cout);
    std::cout << name << "->" << target << '\n';

    std::cout << name << " [ label=\"[";
    if (expression->isComplementary)
        std::cout << '^';
    std::cout << "]\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(ConcatenationExpression *expression, void *) {
    std::ostringstream os;
    os << "Concatenation_" << id++;
    std::string name = os.str();
    std::string ltarget = invoke(expression->left, std::cout), rtarget = invoke(expression->right, std::cout);
    std::cout << name << "->{" << ltarget << " " << rtarget << "}" << '\n';
    std::cout << name << " [ label=\"Con\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(SelectExpression *expression, void *) {
    std::ostringstream os;
    os << "Select_" << id++;
    std::string name = os.str();
    std::string ltarget = invoke(expression->left, std::cout), rtarget = invoke(expression->right, std::cout);
    std::cout << name << "->{" << ltarget << " " << rtarget << "}" << '\n';
    std::cout << name << " [ label=\"|\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::repr(char c) {
    std::string tmp = ::repr(c);
    if (tmp.size() == 2 && tmp != "\\\\")
        tmp = ::repr(tmp);
    if (tmp.back() == '\\')
        tmp.push_back(' ');
    return tmp;
}
