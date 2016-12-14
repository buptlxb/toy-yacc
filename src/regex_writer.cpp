#include "regex_writer.h"
#include "utility.h"

RegexNode::RegexNode(Expression::Ptr expr) : expression(expr) {
}

RegexNode RegexNode::oneOrMore(bool isGreedy) const {
    return repeat(1, -1, isGreedy);
}

RegexNode RegexNode::zeroOrMore(bool isGreedy) const {
    return repeat(0, -1, isGreedy);
}

RegexNode RegexNode::zeroOrOne(bool isGreedy) const {
    return repeat(0, 1, isGreedy);
}

RegexNode RegexNode::repeat(int32_t min, int32_t max, bool isGreedy) const {
    RepeatExpression *expr = new RepeatExpression(min, max, isGreedy);
    expr->expression = this->expression;
    return RegexNode(Expression::Ptr(expr));
}

RegexNode RegexNode::operator+(RegexNode node) const {
    ConcatenationExpression *expr = new ConcatenationExpression;
    expr->left = this->expression;
    expr->right = node.expression;
    return RegexNode(Expression::Ptr(expr));
}

RegexNode RegexNode::operator|(RegexNode node) const {
    SelectExpression *expr = new SelectExpression;
    expr->left = this->expression;
    expr->right = node.expression;
    return RegexNode(Expression::Ptr(expr));
}

RegexNode RegexNode::operator<<=(RegexNode node) const {
    SetExpression *lhs = dynamic_cast<SetExpression *>(this->expression.get());
    SetExpression *rhs = dynamic_cast<SetExpression *>(node.expression.get());
    assertm(lhs && rhs && !lhs->isComplementary && !rhs->isComplementary, "RegexNode::operator%%(const RegexNode &node) only union non-complementary SetExpression");
    SelectExpression *select = new SelectExpression;
    select->left = lhs->expression;
    select->right = rhs->expression;
    SetExpression *expr = new SetExpression;
    expr->expression = Expression::Ptr(select);
    expr->isComplementary = false;
    return RegexNode(Expression::Ptr(expr));
}

RegexNode RegexNode::operator!() const {
    SetExpression *thiz = dynamic_cast<SetExpression *>(this->expression.get());
    assertm(thiz, "RegexNode::operator!() only flip Set Expression");
    thiz->isComplementary = !thiz->isComplementary;
    return *this;
}

RegexNode rBegin() {
    return RegexNode(Expression::Ptr(new BeginExpression));
}

RegexNode rEnd() {
    return RegexNode(Expression::Ptr(new EndExpression));
}

RegexNode rR(char a) {
    return rR(a, a);
}

RegexNode rR(unsigned char a, unsigned char b) {
    return RegexNode(Expression::Ptr(new CharRangeExpression(a, b)));
}

RegexNode rC(char a) {
    return rC(a, a);
}

RegexNode rC(unsigned char a, unsigned char b) {
    SetExpression *expr = new SetExpression;
    expr->expression = rR(a, b).expression;
    expr->isComplementary = false;
    return RegexNode(Expression::Ptr(expr));
}

RegexNode rD() {
    return rC('0', '9');
}

RegexNode rL() {
    return rC('A', 'Z') <<= rC('a', 'z') <<= rC('_');
}

RegexNode rW() {
    return rC('A', 'Z') <<= rC('a', 'z') <<= rC('0', '9') <<= rC('_');
}

RegexNode rAnyChar() {
    return rR('\x01', '\xFF');
}
