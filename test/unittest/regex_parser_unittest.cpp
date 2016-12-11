// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include <climits>
#include <iostream>
#include "regex_expression.h"
#include "regex_writer.h"
#include "gtest/gtest.h"


// Step 2. Use the TEST macro to define your tests.
//
// TEST has two parameters: the test case name and the test name.
// After using the macro, you should define your test logic between a
// pair of braces.  You can use a bunch of macros to indicate the
// success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
// examples of such macros.  For a complete list, see gtest.h.


const char *input;

typedef Expression::Ptr (*Parser)(const char *&);

#define REGEX_ASSERT2(str, expect) do { \
    const char *input = str; EXPECT_EQ(parseChar(input), expect); \
} while (0)

#define REGEX_ASSERT3(str, node, parse) { \
    const char *input = str; EXPECT_TRUE(parse(input)->equals((node).expression.get())); \
} while (0)

TEST(RegexParser, Char) {
    REGEX_ASSERT2("\r", '\r');
    REGEX_ASSERT2("\n", '\n');
    REGEX_ASSERT2("\t", '\t');
    REGEX_ASSERT2("\\-", '-');
    REGEX_ASSERT2("\\[", '[');
    REGEX_ASSERT2("\\]", ']');
    REGEX_ASSERT2("\\^", '^');
    REGEX_ASSERT2("\\$", '$');
}

TEST(RegexParser, SetItem) {
    REGEX_ASSERT3("a-b", rR('a', 'b'), parseSetItem);
    REGEX_ASSERT3("a", rR('a'), parseSetItem);
    REGEX_ASSERT3("\\-", rR('-'), parseSetItem);
}

TEST(RegexParser, SetItems) {
    REGEX_ASSERT3("a-b", rR('a', 'b'), parseSetItems);
    REGEX_ASSERT3("a-bx", rR('a', 'b') | rR('x'), parseSetItems);
}

TEST(RegexParser, ElementaryRE) {
    REGEX_ASSERT3("\\r", rR('\r'), parseElementaryRE);
    REGEX_ASSERT3("\\n", rR('\n'), parseElementaryRE);
    REGEX_ASSERT3("\\t", rR('\t'), parseElementaryRE);
    REGEX_ASSERT3("\\.", rR('.'), parseElementaryRE);
    REGEX_ASSERT3("^", rBegin(), parseElementaryRE);
    REGEX_ASSERT3("$", rEnd(), parseElementaryRE);
    REGEX_ASSERT3(".", rAnyChar(), parseElementaryRE);
    REGEX_ASSERT3("[0-9]", rD(), parseElementaryRE);
    REGEX_ASSERT3("[A-Za-z_]", rL(), parseElementaryRE);
    REGEX_ASSERT3("[A-Za-z0-9_]", rW(), parseElementaryRE);
    REGEX_ASSERT3("[^0-9]", !rD(), parseElementaryRE);
    REGEX_ASSERT3("[^A-Za-z_]", !rL(), parseElementaryRE);
    REGEX_ASSERT3("[^A-Za-z0-9_]", !rW(), parseElementaryRE);
}

TEST(RegexParser, BasicRE) {
    REGEX_ASSERT3("[0-9]*", rD().zeroOrMore(), parseBasicRE);
    REGEX_ASSERT3("[0-9]+", rD().oneOrMore(), parseBasicRE);
    REGEX_ASSERT3("[0-9]?", rD().zeroOrOne(), parseBasicRE);
    REGEX_ASSERT3("[0-9]*?", rD().zeroOrMore(false), parseBasicRE);
    REGEX_ASSERT3("[0-9]+?", rD().oneOrMore(false), parseBasicRE);
    REGEX_ASSERT3("[0-9]??", rD().zeroOrOne(false), parseBasicRE);
    REGEX_ASSERT3("[a-bx]*", (rC('a', 'b') <<= rC('x')).zeroOrMore(), parseBasicRE);
    REGEX_ASSERT3("[1-9]+", rC('1', '9').oneOrMore(), parseBasicRE);
}

TEST(RegexParser, SimpleRE) {
    REGEX_ASSERT3("a+(bc)*", rR('a').oneOrMore() + (rR('b') + rR('c')).zeroOrMore(), parseSimpleRE);
    REGEX_ASSERT3("(1+2)*(3+4)", (rR('1').oneOrMore() + rR('2')).zeroOrMore() + (rR('3').oneOrMore() + rR('4')), parseSimpleRE);
    REGEX_ASSERT3("[A-Za-z_][A-Za-z0-9_]*", rL() + rW().zeroOrMore(), parseSimpleRE);
    REGEX_ASSERT3(".*[\\r\\n\\t]", rAnyChar().zeroOrMore() + (rC('\r') <<= rC('\n') <<= rC('\t')), parseSimpleRE);
    REGEX_ASSERT3("[a-bx]*[1-9]+", ((rC('a', 'b') <<= rC('x')).zeroOrMore() + rC('1', '9').oneOrMore()), parseSimpleRE);
}

TEST(RegexParser, RE) {
    REGEX_ASSERT3("\\r", rR('\r'), parseRE);
    REGEX_ASSERT3("\\n", rR('\n'), parseRE);
    REGEX_ASSERT3("\\t", rR('\t'), parseRE);
    REGEX_ASSERT3("\\.", rR('.'), parseRE);
    REGEX_ASSERT3("^", rBegin(), parseRE);
    REGEX_ASSERT3("$", rEnd(), parseRE);
    REGEX_ASSERT3(".", rAnyChar(), parseRE);
    REGEX_ASSERT3("[0-9]", rD(), parseRE);
    REGEX_ASSERT3("[A-Za-z_]", rL(), parseRE);
    REGEX_ASSERT3("[A-Za-z0-9_]", rW(), parseRE);
    REGEX_ASSERT3("[^0-9]", !rD(), parseRE);
    REGEX_ASSERT3("[^A-Za-z_]", !rL(), parseRE);
    REGEX_ASSERT3("[^A-Za-z0-9_]", !rW(), parseRE);

    REGEX_ASSERT3("[0-9]*", rD().zeroOrMore(), parseRE);
    REGEX_ASSERT3("[0-9]+", rD().oneOrMore(), parseRE);
    REGEX_ASSERT3("[0-9]?", rD().zeroOrOne(), parseRE);
    REGEX_ASSERT3("[0-9]*?", rD().zeroOrMore(false), parseRE);
    REGEX_ASSERT3("[0-9]+?", rD().oneOrMore(false), parseRE);
    REGEX_ASSERT3("[0-9]??", rD().zeroOrOne(false), parseRE);
    REGEX_ASSERT3("[a-bx]*", (rC('a', 'b') <<= rC('x')).zeroOrMore(), parseRE);
    REGEX_ASSERT3("[1-9]+", rC('1', '9').oneOrMore(), parseRE);

    REGEX_ASSERT3("a+(bc)*", rR('a').oneOrMore() + (rR('b') + rR('c')).zeroOrMore(), parseRE);
    REGEX_ASSERT3("(1+2)*(3+4)", (rR('1').oneOrMore() + rR('2')).zeroOrMore() + (rR('3').oneOrMore() + rR('4')), parseRE);
    REGEX_ASSERT3("[A-Za-z_][A-Za-z0-9_]*", rL() + rW().zeroOrMore(), parseSimpleRE);
    REGEX_ASSERT3(".*[\\r\\n\\t]", rAnyChar().zeroOrMore() + (rC('\r') <<= rC('\n') <<= rC('\t')), parseRE);
    REGEX_ASSERT3("[a-bx]*[1-9]+", ((rC('a', 'b') <<= rC('x')).zeroOrMore() + rC('1', '9').oneOrMore()), parseRE);

    REGEX_ASSERT3("ab|ac", (rR('a') + rR('b')) | (rR('a') + rR('c')), parseRE);
    REGEX_ASSERT3("a(b|c)", rR('a') + (rR('b') | rR('c')), parseRE);
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
