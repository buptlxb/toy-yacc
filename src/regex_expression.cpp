#include <sstream>
#include <iostream>
#include "regex_expression.h"
#include "regex_exception.h"
#include "regex_algorithm.h"

bool Expression::equals(Expression *target) {
    return EqualsVisitor().invoke(this, target);
}

void Expression::graphviz(std::ostream &os) {
    os << "digraph {\n";
    GraphvizVisitor(os).invoke(this, nullptr);
    os << "}" << std::endl;
}

void Expression::setNormalize(Range<unsigned char>::List *unifiedRanges) {
    SetNormalizationVisitor().invoke(this, unifiedRanges);
}

void Expression::setUnify(Range<unsigned char>::List unifiedRanges) {
    SetUnificationVisitor().invoke(this, &unifiedRanges);
}

Automaton::Ptr Expression::generateEpsilonNfa() {
    Automaton::Ptr automaton(new Automaton);
    EpsilonNfa nfa = EpsilonNfaVisitor().invoke(this, automaton.get());
    automaton->startState = nfa.start;
    nfa.finish->isAccepted = true;
    return automaton;
}

CharRangeExpression::CharRangeExpression() : range('\x00', '\x00') {
}

CharRangeExpression::CharRangeExpression(unsigned char b,unsigned char e) : range(b, e) {
}

void CharRangeExpression::accept(Visitor &visitor) {
    visitor.visit(this);
}

void BeginExpression::accept(Visitor &visitor) {
    visitor.visit(this);
}

void EndExpression::accept(Visitor &visitor) {
    visitor.visit(this);
}

RepeatExpression::RepeatExpression(int32_t min, int32_t max, bool gdy) : times(min, max), isGreedy(gdy) {
}

void RepeatExpression::accept(Visitor &visitor) {
    visitor.visit(this);
}

void SetExpression::accept(Visitor &visitor) {
    visitor.visit(this);
}

void ConcatenationExpression::accept(Visitor &visitor) {
    visitor.visit(this);
}

void SelectExpression::accept(Visitor &visitor) {
    visitor.visit(this);
}

bool isChar(const char *&input, char c) {
    if (*input == c) {
        ++input;
        return true;
    }
    return false;
}

bool isChars(const char *&input, std::string chars) {
    if (chars.find(*input) == std::string::npos)
        return false;
    ++input;
    return true;
}

/**
 *  <char> ::= non-metacharacter | "\" metacharacter
**/
char parseChar(const char *&input) {
    char c = *input;
    if (isChar(input, '\\')) {  //  metacharacter
        switch (*input) {
            case 'r':
                c = '\r';
                break;
            case 'n':
                c = '\n';
                break;
            case 't':
                c = '\t';
                break;
            case '-': case '[': case ']': case '\\': case '/': case '^': case '$': case '.': case '+': case '*': case '?': case '|':
                c = *input;
                break;
            default:
            {
                std::ostringstream msg("Illegal character escapoing: ", std::ostringstream::ate);
                msg << repr(*input) << "(Only \"rnt-[]\\/^$.+*?|\" are legal escaped characters)";
                throw LexerException(msg.str());
            }
        }
    }
    ++input;
    return c;
}

/**
 *  <SetItem> ::= <range> | <char>
 *  <range> ::= <char> "-" <char>
**/
Expression::Ptr parseSetItem(const char *&input) {
    unsigned char b, e;
    b = e = parseChar(input);
    if (isChar(input, '-')) {   // <range>
        e = parseChar(input);
    }
    if (b > e) {
        std::ostringstream msg("Range out of order in character class: ", std::ostringstream::ate);
        msg << repr(b) << "-" << repr(e);
        throw LexerException(msg.str());
    }
    return Expression::Ptr(new CharRangeExpression(b, e));
}

/**
 *  <SetItems> ::= <SetItem> | <SetItem> <SetItems>
**/
Expression::Ptr parseSetItems(const char *&input) {
    if (!*input || *input == ']')
        return nullptr;
    Expression::Ptr setItem = parseSetItem(input), right = parseSetItems(input);
    if (right) {
        SelectExpression *select = new SelectExpression;
        select->left = setItem;
        select->right = right;
        return Expression::Ptr(select);
    } else
        return setItem;
}

/**
 *  <ElementaryRE> ::= <group> | <any> | <eos> | <bos> | <char> | <set> |
 *  <group> ::= "(" <RE> ")"
 *  <any> ::= "."
 *  <eos> ::= "$"
 *  <bos> ::= "^"
 *  <char> ::= non-metacharacter | "\" metacharacter
 *  <set> ::= <positive-set> | <negative-set>
 *  <positive-set> ::= "[" <set-items> "]"
 *  <negative-set> ::= "[^" <set-items> "]"
 *  <set-items> ::= <set-item> | <set-item> <set-items>
 *  <set-item> ::= <range> | <char>
 *  <range> ::= <char> "-" <char>
**/
Expression::Ptr parseElementaryRE(const char *&input) {
    using std::shared_ptr;
    if (!*input)
        return nullptr;
    else if (isChar(input, '^'))    // <bos>
        return Expression::Ptr(new BeginExpression);
    else if (isChar(input, '$'))    // <eos>
        return EndExpression::Ptr(new EndExpression);
    else if (isChar(input, '.'))    // <any>
        return Expression::Ptr(new CharRangeExpression('\x01', '\xFF'));
    else if (isChar(input, '[')) {  // <set>
        shared_ptr<SetExpression> expr(new SetExpression);
        expr->isComplementary = isChar(input, '^');
        expr->expression = parseSetItems(input);
        if (!isChar(input, ']'))
            throw LexerException("Expect a ']' to close a <set>");
        return expr;
    } else if (isChar(input, '(')) { // <group>
        Expression::Ptr expr = parseRE(input);
        if (!isChar(input, ')'))
            throw LexerException("Expect a ')' to close a <group>");
        return expr;
    } else if (isChars(input, "()+*?|")) { // metacharacter
        --input;
        return nullptr;
    } else { // <char>
        char c = parseChar(input);
        return Expression::Ptr(new CharRangeExpression(c, c));
    }
}

/**
 * <basicRE> :== <star> | <plus> | <question> | <ElementaryRE>
 * <star> ::= <ElementaryRE> "*" | <ElementaryRE> "*?"
 * <plus> ::= <ElementaryRE> "+" | <ElementaryRE> "+?"
 * <question> ::= <ElementaryRE> "?" | <ElementaryRE> "??"
**/
Expression::Ptr parseBasicRE(const char *&input) {
    Expression::Ptr elementary = parseElementaryRE(input);
    if (isChar(input, '*')) {   // <star>
        RepeatExpression *repeat = new RepeatExpression(0, -1, !isChar(input, '?'));
        repeat->expression = elementary;
        return Expression::Ptr(repeat);
    } else if (isChar(input, '+')) {    // <plus>
        RepeatExpression *repeat = new RepeatExpression(1, -1, !isChar(input, '?'));
        repeat->expression = elementary;
        return Expression::Ptr(repeat);
    } else if (isChar(input, '?')) {    // <question>
        RepeatExpression *repeat = new RepeatExpression(0, 1, !isChar(input, '?'));
        repeat->expression = elementary;
        return Expression::Ptr(repeat);
    } else {
        return elementary;
    }
}

/**
 * <SimpleRE> ::= <SimpleRE-1> <BasicRE>
 * <SimpleRE-1> ::= <BasicRE> <SimpleRE-1> | epsilon
**/
Expression::Ptr parseSimpleRE(const char *&input) {
    if (!*input)
        return nullptr;
    Expression::Ptr basic = parseBasicRE(input);
    if (!basic)
        return nullptr;
    Expression::Ptr right = parseSimpleRE(input);
    if (right) {
        ConcatenationExpression *concatenation = new ConcatenationExpression;
        concatenation->left = basic;
        concatenation->right = right;
        return Expression::Ptr(concatenation);
    } else
        return basic;
}

/**
 * <RE> ::= <RE-1> <SimpleRE>
 * <RE-1> ::= "|" <SimpleRE> <RE-1> | epsilon
**/
Expression::Ptr parseRE(const char *&input) {
    if (!*input)
        return nullptr;
    Expression::Ptr simple = parseSimpleRE(input);
    if (isChar(input, '|')) {
        Expression::Ptr right = parseRE(input);
        if (right) {
            SelectExpression *select = new SelectExpression;
            select->left = simple;
            select->right = right;
            return Expression::Ptr(select);
        }
    }
    return simple;
}

Expression::Ptr parseRegex(const std::string &str) {
    const char *input = str.c_str(), *start = input;
    try {
        return parseRE(input);
    } catch (LexerException e) {
        std::ostringstream os(e.what(), std::ostringstream::ate);
        os << " in position " << input-start << " of \"" << repr(str) << "\"";
        throw LexerException(os.str());
    }
}
