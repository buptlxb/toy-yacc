#include <iostream>
#include "regex_expression.h"
#include <cassert>

void test_parseChar() {
    const char *input = "\r";
    assert(parseChar(input) == '\r');
    input = "\n";
    assert(parseChar(input) == '\n');
    input = "\t";
    assert(parseChar(input) == '\t');
    input = "\\-";
    assert(parseChar(input) == '-');
    input = "\\[";
    assert(parseChar(input) == '[');
    input = "\\]";
    assert(parseChar(input) == ']');
    input = "\\^";
    assert(parseChar(input) == '^');
    input = "\\$";
    assert(parseChar(input) == '$');
}

void test_parseSetItem() {
    const char *input = "a-b";
    assert(parseSetItem(input)->equals(new CharRangeExpression('a', 'b')));
    input = "a";
    assert(parseSetItem(input)->equals(new CharRangeExpression('a', 'a')));
    input = "\\-";
    assert(parseSetItem(input)->equals(new CharRangeExpression('-', '-')));
}

void test_parseSetItems() {
    const char *input = "a-b";
    assert(parseSetItems(input)->equals(new CharRangeExpression('a', 'b')));
    input = "a-bx";
    ConcatenationExpression *con = new ConcatenationExpression;
    con->left = Expression::Ptr(new CharRangeExpression('a', 'b'));
    con->right = Expression::Ptr(new CharRangeExpression('x', 'x'));
    assert(parseSetItems(input)->equals(con));
}

void test_parseElementaryRE() {
    const char *input = "^";
    assert(parseElementaryRE(input)->equals(new BeginExpression));
    input = "$";
    assert(parseElementaryRE(input)->equals(new EndExpression));
    input = ".";
    assert(parseElementaryRE(input)->equals(new CharRangeExpression('\x00', '\xFF')));

    input = "[a-bx]*";
    ConcatenationExpression *con = new ConcatenationExpression;
    con->left = Expression::Ptr(new CharRangeExpression('a', 'b'));
    con->right = Expression::Ptr(new CharRangeExpression('x', 'x'));
    SetExpression *set = new SetExpression;
    set->isComplementary = false;
    set->expression = Expression::Ptr(con);
    assert(parseElementaryRE(input)->equals(set));

    input = "[^qa-bx-z]";
    con = new ConcatenationExpression;
    con->left = Expression::Ptr(new CharRangeExpression('a', 'b'));
    con->right = Expression::Ptr(new CharRangeExpression('x', 'z'));
    ConcatenationExpression *con1 = new ConcatenationExpression;
    con1->left = Expression::Ptr(new CharRangeExpression('q', 'q'));
    con1->right = Expression::Ptr(con);
    set = new SetExpression;
    set->isComplementary = true;
    set->expression = Expression::Ptr(con1);
    assert(parseElementaryRE(input)->equals(set));
}

void test_parseBasicRE() {
    const char *input = "[a-bx]*";
    ConcatenationExpression *con = new ConcatenationExpression;
    con->left = Expression::Ptr(new CharRangeExpression('a', 'b'));
    con->right = Expression::Ptr(new CharRangeExpression('x', 'x'));
    SetExpression *set = new SetExpression;
    set->isComplementary = false;
    set->expression = Expression::Ptr(con);
    RepeatExpression *repeat = new RepeatExpression;
    repeat->min = 0, repeat->max = -1;
    repeat->expression = Expression::Ptr(set);
    assert(parseBasicRE(input)->equals(repeat));

    input = "[1-9]+";
    SetExpression *set2 = new SetExpression;
    set2->isComplementary = false;
    set2->expression = Expression::Ptr(new CharRangeExpression('1', '9'));
    RepeatExpression *repeat2 = new RepeatExpression;
    repeat2->min = 1, repeat2->max = -1;
    repeat2->expression = Expression::Ptr(set2);
    assert(parseBasicRE(input)->equals(repeat2));
}

void test_parseSimpleRE() {
    const char *input = "[a-bx]*[1-9]+";

    ConcatenationExpression *con = new ConcatenationExpression;
    con->left = Expression::Ptr(new CharRangeExpression('a', 'b'));
    con->right = Expression::Ptr(new CharRangeExpression('x', 'x'));
    SetExpression *set = new SetExpression;
    set->isComplementary = false;
    set->expression = Expression::Ptr(con);
    RepeatExpression *repeat = new RepeatExpression;
    repeat->min = 0, repeat->max = -1;
    repeat->expression = Expression::Ptr(set);

    SetExpression *set2 = new SetExpression;
    set2->isComplementary = false;
    set2->expression = Expression::Ptr(new CharRangeExpression('1', '9'));
    RepeatExpression *repeat2 = new RepeatExpression;
    repeat2->min = 1, repeat2->max = -1;
    repeat2->expression = Expression::Ptr(set2);

    ConcatenationExpression *con2 = new ConcatenationExpression;
    con2->left = Expression::Ptr(repeat);
    con2->right = Expression::Ptr(repeat2);

    assert(parseSimpleRE(input)->equals(con2));
}

void test_parseRE() {
    const char *input = "[A-Za-z_][A-Za-z0-9_]*";

    ConcatenationExpression *con1 = new ConcatenationExpression;
    con1->right = Expression::Ptr(new CharRangeExpression('_', '_'));
    con1->left = Expression::Ptr(new CharRangeExpression('a', 'z'));

    ConcatenationExpression *con2 = new ConcatenationExpression;
    con2->left = Expression::Ptr(new CharRangeExpression('A', 'Z'));
    con2->right = Expression::Ptr(con1);

    SetExpression *set1 = new SetExpression;
    set1->expression = Expression::Ptr(con2);


    ConcatenationExpression *con3 = new ConcatenationExpression;
    con3->right = Expression::Ptr(new CharRangeExpression('_', '_'));
    con3->left = Expression::Ptr(new CharRangeExpression('0', '9'));

    ConcatenationExpression *con4 = new ConcatenationExpression;
    con4->right = Expression::Ptr(con3);
    con4->left = Expression::Ptr(new CharRangeExpression('a', 'z'));

    ConcatenationExpression *con5 = new ConcatenationExpression;
    con5->right = Expression::Ptr(con4);
    con5->left = Expression::Ptr(new CharRangeExpression('A', 'Z'));

    SetExpression *set2 = new SetExpression;
    set2->expression = Expression::Ptr(con5);

    RepeatExpression *repeat = new RepeatExpression;
    repeat->expression = Expression::Ptr(set2);
    repeat->min = 0, repeat->max = -1;

    ConcatenationExpression *con6 = new ConcatenationExpression;
    con6->left = Expression::Ptr(set1);
    con6->right = Expression::Ptr(repeat);

    assert(parseRE(input)->equals(con6));
};

int main(void)
{
    test_parseChar();
    test_parseSetItem();
    test_parseSetItems();
    test_parseElementaryRE();
    test_parseBasicRE();
    test_parseSimpleRE();
    test_parseRE();
    return 0;
}
