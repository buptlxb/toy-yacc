// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include <climits>
#include <iostream>
#include "regex_expression.h"
#include "gtest/gtest.h"


// Step 2. Use the TEST macro to define your tests.
//
// TEST has two parameters: the test case name and the test name.
// After using the macro, you should define your test logic between a
// pair of braces.  You can use a bunch of macros to indicate the
// success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
// examples of such macros.  For a complete list, see gtest.h.


const char *input;

TEST(RegexParser, Char) {
    input = "\r";
    EXPECT_EQ(parseChar(input), '\r');
    input = "\n";
    EXPECT_EQ(parseChar(input), '\n');
    input = "\t";
    EXPECT_EQ(parseChar(input), '\t');
    input = "\\-";
    EXPECT_EQ(parseChar(input), '-');
    input = "\\[";
    EXPECT_EQ(parseChar(input), '[');
    input = "\\]";
    EXPECT_EQ(parseChar(input), ']');
    input = "\\^";
    EXPECT_EQ(parseChar(input), '^');
    input = "\\$";
    EXPECT_EQ(parseChar(input), '$');
}

TEST(RegexParser, SetItem) {
    input = "a-b";
    EXPECT_TRUE(parseSetItem(input)->equals(new CharRangeExpression('a', 'b')));
    input = "a";
    EXPECT_TRUE(parseSetItem(input)->equals(new CharRangeExpression('a', 'a')));
    input = "\\-";
    EXPECT_TRUE(parseSetItem(input)->equals(new CharRangeExpression('-', '-')));
}

TEST(RegexParser, SetItems) {
    input = "a-b";
    EXPECT_TRUE(parseSetItems(input)->equals(new CharRangeExpression('a', 'b')));
    input = "a-bx";
    SelectExpression *con = new SelectExpression;
    con->left = Expression::Ptr(new CharRangeExpression('a', 'b'));
    con->right = Expression::Ptr(new CharRangeExpression('x', 'x'));
    EXPECT_TRUE(parseSetItems(input)->equals(con));
}

TEST(RegexParser, ElementaryRE) {
    input = "^";
    EXPECT_TRUE(parseElementaryRE(input)->equals(new BeginExpression));
    input = "$";
    EXPECT_TRUE(parseElementaryRE(input)->equals(new EndExpression));
    input = ".";
    EXPECT_TRUE(parseElementaryRE(input)->equals(new CharRangeExpression('\x00', '\xFF')));

    input = "[a-bx]*";
    SelectExpression *con = new SelectExpression;
    con->left = Expression::Ptr(new CharRangeExpression('a', 'b'));
    con->right = Expression::Ptr(new CharRangeExpression('x', 'x'));
    SetExpression *set = new SetExpression;
    set->isComplementary = false;
    set->expression = Expression::Ptr(con);
    EXPECT_TRUE(parseElementaryRE(input)->equals(set));

    input = "[^qa-bx-z]";
    con = new SelectExpression;
    con->left = Expression::Ptr(new CharRangeExpression('a', 'b'));
    con->right = Expression::Ptr(new CharRangeExpression('x', 'z'));
    SelectExpression *con1 = new SelectExpression;
    con1->left = Expression::Ptr(new CharRangeExpression('q', 'q'));
    con1->right = Expression::Ptr(con);
    set = new SetExpression;
    set->isComplementary = true;
    set->expression = Expression::Ptr(con1);
    EXPECT_TRUE(parseElementaryRE(input)->equals(set));
}

TEST(RegexParser, BasicRE) {
    input = "[a-bx]*";
    SelectExpression *con = new SelectExpression;
    con->left = Expression::Ptr(new CharRangeExpression('a', 'b'));
    con->right = Expression::Ptr(new CharRangeExpression('x', 'x'));
    SetExpression *set = new SetExpression;
    set->isComplementary = false;
    set->expression = Expression::Ptr(con);
    RepeatExpression *repeat = new RepeatExpression(0, -1, true);
    repeat->expression = Expression::Ptr(set);
    EXPECT_TRUE(parseBasicRE(input)->equals(repeat));

    input = "[1-9]+";
    SetExpression *set2 = new SetExpression;
    set2->isComplementary = false;
    set2->expression = Expression::Ptr(new CharRangeExpression('1', '9'));
    RepeatExpression *repeat2 = new RepeatExpression(1, -1, true);
    repeat2->expression = Expression::Ptr(set2);
    EXPECT_TRUE(parseBasicRE(input)->equals(repeat2));
}

TEST(RegexParser, SimpleRE) {
    const char *input = "[a-bx]*[1-9]+";

    SelectExpression *con = new SelectExpression;
    con->left = Expression::Ptr(new CharRangeExpression('a', 'b'));
    con->right = Expression::Ptr(new CharRangeExpression('x', 'x'));
    SetExpression *set = new SetExpression;
    set->isComplementary = false;
    set->expression = Expression::Ptr(con);
    RepeatExpression *repeat = new RepeatExpression(0, -1, true);
    repeat->expression = Expression::Ptr(set);

    SetExpression *set2 = new SetExpression;
    set2->isComplementary = false;
    set2->expression = Expression::Ptr(new CharRangeExpression('1', '9'));
    RepeatExpression *repeat2 = new RepeatExpression(1, -1, true);
    repeat2->expression = Expression::Ptr(set2);

    ConcatenationExpression *con2 = new ConcatenationExpression;
    con2->left = Expression::Ptr(repeat);
    con2->right = Expression::Ptr(repeat2);

    EXPECT_TRUE(parseSimpleRE(input)->equals(con2));
}

TEST(RegexParser, RE) {
    input = "[A-Za-z_][A-Za-z0-9_]*";

    SelectExpression *con1 = new SelectExpression;
    con1->right = Expression::Ptr(new CharRangeExpression('_', '_'));
    con1->left = Expression::Ptr(new CharRangeExpression('a', 'z'));

    SelectExpression *con2 = new SelectExpression;
    con2->left = Expression::Ptr(new CharRangeExpression('A', 'Z'));
    con2->right = Expression::Ptr(con1);

    SetExpression *set1 = new SetExpression;
    set1->expression = Expression::Ptr(con2);
    set1->isComplementary = false;

    SelectExpression *con3 = new SelectExpression;
    con3->right = Expression::Ptr(new CharRangeExpression('_', '_'));
    con3->left = Expression::Ptr(new CharRangeExpression('0', '9'));

    SelectExpression *con4 = new SelectExpression;
    con4->right = Expression::Ptr(con3);
    con4->left = Expression::Ptr(new CharRangeExpression('a', 'z'));

    SelectExpression *con5 = new SelectExpression;
    con5->right = Expression::Ptr(con4);
    con5->left = Expression::Ptr(new CharRangeExpression('A', 'Z'));

    SetExpression *set2 = new SetExpression;
    set2->expression = Expression::Ptr(con5);
    set2->isComplementary = false;

    RepeatExpression *repeat = new RepeatExpression(0, -1, true);
    repeat->expression = Expression::Ptr(set2);

    ConcatenationExpression *con6 = new ConcatenationExpression;
    con6->left = Expression::Ptr(set1);
    con6->right = Expression::Ptr(repeat);

    EXPECT_TRUE(parseRE(input)->equals(con6));
}

// Step 3. Call RUN_ALL_TESTS() in main().
//
// We do this by linking in src/gtest_main.cc file, which consists of
// a main() function which calls RUN_ALL_TESTS() for us.
//
// This runs all the tests you've defined, prints the result, and
// returns 0 if successful, or 1 otherwise.
//
// Did you notice that we didn't register the tests?  The
// RUN_ALL_TESTS() macro magically knows about all the tests we
// defined.  Isn't this convenient?
