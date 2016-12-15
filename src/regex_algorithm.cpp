#include <sstream>
#include "regex_algorithm.h"
#include "utility.h"


bool EqualsVisitor::visit(CharRangeExpression *expression, Expression *target) {
    CharRangeExpression *that = dynamic_cast<CharRangeExpression *>(target);
    if (!that)
        return false;
    return expression->range == that->range;
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
    if (expression->times != expression->times || expression->isGreedy != that->isGreedy)
        return false;
    return invoke(expression->expression, that->expression.get());
}

bool EqualsVisitor::visit(SetExpression *expression, Expression *target) {
    SetExpression *that = dynamic_cast<SetExpression *>(target);
    if (!that)
        return false;
    if (expression->isComplementary != that->isComplementary)
        return false;
    // if (!expression->expression ^ !that->expression)
    //     return false;
    if (!expression->expression && !that->expression)
        return true;
    if (expression->expression && that->expression)
        return invoke(expression->expression, that->expression.get());
    return false;
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

GraphvizVisitor::GraphvizVisitor(std::ostream &os, unsigned long x) : dot(os), id(x) { }

std::string GraphvizVisitor::visit(CharRangeExpression *expression, void *) {
    std::ostringstream os;
    os << "CharRange_" << id++;
    std::string name = os.str();
    dot << name << " [ label=\"" << repr(expression->range.begin) << "-" << repr(expression->range.end) << "\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(BeginExpression *expression, void *) {
    std::ostringstream os;
    os << "Begin_" << id++;
    std::string name = os.str();
    dot << name << " [ label=\"BEGIN\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(EndExpression *expression, void *) {
    std::ostringstream os;
    os << "End_" << id++;
    std::string name = os.str();
    dot << name << " [ label=\"END\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(RepeatExpression *expression, void *) {
    std::ostringstream os;
    os << "Repeat_" << id++;
    std::string name = os.str(), target = invoke(expression->expression, nullptr);
    dot << name << "->" << target << '\n';

    dot << name << " [ label=\"";
    if (expression->isGreedy)
        dot << "Greedy";
    dot << "{" << expression->times.begin << "-";
    if (expression->times.end < 0)
        dot << "INF";
    else
        dot << expression->times.end;
    dot << "}\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(SetExpression *expression, void *) {
    std::ostringstream os;
    os << "Set_" << id++;
    std::string name = os.str();
    if (expression->expression) {
        std::string target = invoke(expression->expression, nullptr);
        dot << name << "->" << target << '\n';
    }

    dot << name << " [ label=\"[";
    if (expression->isComplementary)
        dot << '^';
    dot << "]\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(ConcatenationExpression *expression, void *) {
    std::ostringstream os;
    os << "Concatenation_" << id++;
    std::string name = os.str();
    std::string ltarget = invoke(expression->left, nullptr), rtarget = invoke(expression->right, nullptr);
    dot << name << "->{" << ltarget << " " << rtarget << "}" << '\n';
    dot << name << " [ label=\"Con\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::visit(SelectExpression *expression, void *) {
    std::ostringstream os;
    os << "Select_" << id++;
    std::string name = os.str();
    std::string ltarget = invoke(expression->left, dot), rtarget = invoke(expression->right, dot);
    dot << name << "->{" << ltarget << " " << rtarget << "}" << '\n';
    dot << name << " [ label=\"|\" ]" << "\n";
    return name;
}

std::string GraphvizVisitor::repr(unsigned char c) {
    std::string tmp = ::repr(c);
    if (tmp.size() == 2 && tmp != "\\\\")
        tmp = ::repr(tmp);
    if (tmp.back() == '\\')
        tmp.push_back(' ');
    return tmp;
}

Expression::Ptr SetNormalizationVisitor::rebuild(Expression::Ptr posit, unsigned char begin, unsigned char end) {
    CharRangeExpression *range = new CharRangeExpression(begin, end);
    if (posit) {
        SelectExpression *select = new SelectExpression;
        select->left = Expression::Ptr(range);
        select->right = posit;
        posit = Expression::Ptr(select);
    } else
        posit.reset(range);
    return posit;
}

void SetNormalizationVisitor::visit(CharRangeExpression *expression, Range<unsigned char>::List *unifiedRanges) {
    marshalRange(expression->range, *unifiedRanges);
}
void SetNormalizationVisitor::visit(BeginExpression *expression, Range<unsigned char>::List *) { }
void SetNormalizationVisitor::visit(EndExpression *expression, Range<unsigned char>::List *) { }
void SetNormalizationVisitor::visit(RepeatExpression *expression, Range<unsigned char>::List *unifiedRanges) {
    invoke(expression->expression, unifiedRanges);
}
void SetNormalizationVisitor::visit(SetExpression *expression, Range<unsigned char>::List *unifiedRanges) {
    if (!expression->expression)
        return;
    Range<unsigned char>::List ranges;
    invoke(expression->expression, &ranges);
    if (expression->isComplementary) {
        unsigned char end = '\xff';
        Expression::Ptr posit;
        for (auto i = ranges.rbegin(), iend = ranges.rend(); i != iend; ++i) {
            if (end > i->end) {
                posit = rebuild(posit, i->end+1, end);
            }
            end = i->begin - 1;
            if (i->begin == '\x01')
                break;
        }
        if (end != '\x00')
            posit = rebuild(posit, '\x01', end);

        expression->isComplementary = false;
        expression->expression = posit;
    } else {
        Expression::Ptr posit;
        unsigned char begin = ranges.rbegin()->end+1, end = ranges.rbegin()->end;
        for (auto i = ranges.rbegin(), iend = ranges.rend(); i != iend; ++i) {
            if (begin-1 != i->end) {
                posit = rebuild(posit, begin, end);
                end = i->end;
            }
            begin = i->begin;
        }
        posit = rebuild(posit, begin, end);
        expression->expression = posit;
    }
    invoke(expression->expression, unifiedRanges);
}
void SetNormalizationVisitor::visit(ConcatenationExpression *expression, Range<unsigned char>::List *unifiedRanges) {
    invoke(expression->left, unifiedRanges);
    invoke(expression->right, unifiedRanges);
}
void SetNormalizationVisitor::visit(SelectExpression *expression, Range<unsigned char>::List *unifiedRanges) {
    invoke(expression->left, unifiedRanges);
    invoke(expression->right, unifiedRanges);
}

void SetUnificationVisitor::visit(SetExpression *expression, Range<unsigned char>::List *unifiedRanges) {
    if (!expression->expression)
        return;
    assertm(!expression->isComplementary, "Unable to apply SetUnificationVisitor to negative SetExpression.\nPlease class setNormalize() first.");
    Range<unsigned char>::List ranges;
    invoke(expression->expression, &ranges);
    Expression::Ptr posit;
    for (auto i = ranges.rbegin(), iend = ranges.rend(); i != iend; ++i) {
        for (auto j = unifiedRanges->rbegin(), jend = unifiedRanges->rend(); j != jend; ++j) {
            if (i->begin <= j->begin && j->end <= i->end)
                posit = rebuild(posit, j->begin, j->end);
        }
    }
    expression->expression = posit;
}

EpsilonNfa EpsilonNfaVisitor::connect(EpsilonNfa a, EpsilonNfa b, Automaton *automaton) {
    if (a.start) {
        automaton->getEpsilon(a.finish, b.start);
        a.finish = b.finish;
        return a;
    } else
        return b;
}

EpsilonNfa EpsilonNfaVisitor::visit(CharRangeExpression *expression, Automaton *automaton) {
    EpsilonNfa nfa;
    nfa.start = automaton->getState();
    nfa.finish = automaton->getState();
    automaton->getChars(nfa.start, nfa.finish, expression->range);
    return nfa;
}

EpsilonNfa EpsilonNfaVisitor::visit(BeginExpression *expression, Automaton *automaton) {
    EpsilonNfa nfa;
    nfa.start = automaton->getState();
    nfa.finish = automaton->getState();
    automaton->getBeginString(nfa.start, nfa.finish);
    return nfa;
}

EpsilonNfa EpsilonNfaVisitor::visit(EndExpression *expression, Automaton *automaton) {
    EpsilonNfa nfa;
    nfa.start = automaton->getState();
    nfa.finish = automaton->getState();
    automaton->getEndString(nfa.start, nfa.finish);
    return nfa;
}

EpsilonNfa EpsilonNfaVisitor::visit(RepeatExpression *expression, Automaton *automaton) {
    EpsilonNfa nfa;
    for (int i = 0; i < expression->times.begin; ++i) {
        EpsilonNfa replica = invoke(expression->expression, automaton);
        nfa = connect(nfa, replica, automaton);
    }
    if (expression->times.end == -1) {
        EpsilonNfa replica = invoke(expression->expression, automaton);
        if (!nfa.start) {
            nfa.start = nfa.finish = automaton->getState();
        }
        State::Ptr begin = nfa.finish;
        State::Ptr end = automaton->getState();
        if (expression->isGreedy) {
            automaton->getEpsilon(begin, replica.start);
            automaton->getEpsilon(replica.finish, begin);
            automaton->getNop(begin, end);
        } else {
            automaton->getNop(begin, end);
            automaton->getEpsilon(begin, replica.start);
            automaton->getEpsilon(replica.finish, begin);
        }
        nfa.finish = end;
    } else if (expression->times.end > expression->times.begin) {
        for (int i = expression->times.begin, iend = expression->times.end; i != iend; ++i) {
            EpsilonNfa replica = invoke(expression->expression, automaton);
            State::Ptr begin = automaton->getState();
            State::Ptr end = automaton->getState();
            if (expression->isGreedy) {
                automaton->getEpsilon(begin, replica.start);
                automaton->getEpsilon(replica.finish, end);
                automaton->getNop(begin, end);
            } else {
                automaton->getNop(begin, end);
                automaton->getEpsilon(begin, replica.start);
                automaton->getEpsilon(replica.finish, end);
            }
            replica.start = begin;
            replica.finish = end;
            nfa = connect(nfa, replica, automaton);
        }
    }
    return nfa;
}

EpsilonNfa EpsilonNfaVisitor::visit(SetExpression *expression, Automaton *automaton) {
    assertm(!expression->isComplementary, "Unable to apply EpsilonNfaVisitor to negative SetExpression.\nPlease call setNormalize() first.");
    if (expression->expression)
        return invoke(expression->expression, automaton);
    EpsilonNfa nfa;
    nfa.start = nfa.finish = automaton->getState();
    return nfa;
}

EpsilonNfa EpsilonNfaVisitor::visit(ConcatenationExpression *expression, Automaton *automaton) {
    return connect(invoke(expression->left, automaton), invoke(expression->right, automaton), automaton);
}

EpsilonNfa EpsilonNfaVisitor::visit(SelectExpression *expression, Automaton *automaton) {
    EpsilonNfa nfa;
    nfa.start = automaton->getState();
    nfa.finish = automaton->getState();
    auto a = invoke(expression->left, automaton);
    auto b = invoke(expression->right, automaton);
    automaton->getEpsilon(nfa.start, a.start);
    automaton->getEpsilon(nfa.start, b.start);
    automaton->getEpsilon(a.finish, nfa.finish);
    automaton->getEpsilon(b.finish, nfa.finish);
    return nfa;
}
