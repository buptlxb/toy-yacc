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


#define SET_NORMALIZATION_ASSERT(str, node) { \
    const char *input = str; \
    Range<unsigned char>::List unifiedRanges; \
    auto  regex = parseRegex(input); \
    regex->setNormalize(&unifiedRanges); \
    EXPECT_TRUE(regex->equals((node).expression.get())); \
} while (0)

#define SET_UNIFICATION_ASSERT(str, node) { \
    const char *input = str; \
    Range<unsigned char>::List unifiedRanges; \
    auto  regex = parseRegex(input); \
    regex->setNormalize(&unifiedRanges); \
    regex->setUnify(unifiedRanges); \
    EXPECT_TRUE(regex->equals((node).expression.get())); \
} while (0)

TEST(RegexAlgorithm, SetNormalization) {
    SET_NORMALIZATION_ASSERT("[a-g][h-n]", rC('a', 'g') + rC('h', 'n'));
    SET_NORMALIZATION_ASSERT("[a-gg-n]", rC('a', 'n'));
    SET_NORMALIZATION_ASSERT("[0-21-32-4]", rC('0', '4'));
    SET_NORMALIZATION_ASSERT("[^C-X][A-Z]", (rC('\x01', 'C'-1) <<= rC('X'+1, '\xFF')) + rC('A', 'Z'));
    SET_NORMALIZATION_ASSERT("[0-21-32-46-76-9]", rC('0', '4') <<= rC('6', '9'));
}

TEST(RegexAlgorithm, SetUnification) {
    SET_UNIFICATION_ASSERT("[a-g][h-n]", rC('a', 'g') + rC('h', 'n'));
    SET_UNIFICATION_ASSERT("[a-gg-n]", rC('a', 'n'));
    SET_UNIFICATION_ASSERT("[0-21-32-4]", rC('0', '4'));
    SET_UNIFICATION_ASSERT("[^C-X][A-Z]", (rC('\x01', '@') <<= rC('A', 'B') <<= rC('Y', 'Z') <<= rC('[', '\xFF')) + (rC('A', 'B') <<= rC('C', 'X') <<= rC('Y', 'Z')));
    SET_UNIFICATION_ASSERT("[0-21-32-46-76-9]", rC('0', '4') <<= rC('6', '9'));
    SET_UNIFICATION_ASSERT("[a-b]|(ax)", (rC('a', 'a')<<=rC('b', 'b')) | (rR('a', 'a') + rR('x', 'x')));
    SET_UNIFICATION_ASSERT("(ax)|[a-b]", (rR('a', 'a') + rR('x', 'x')) | (rC('a', 'a')<<=rC('b', 'b')));
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
