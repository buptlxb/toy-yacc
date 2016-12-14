// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include <climits>
#include <iostream>
#include <string>
#include "regex_expression.h"
#include "regex_interpreter.h"
#include "gtest/gtest.h"

using std::string;

#define POOR_SEARCH_ASSERT(input, begin, len) { \
    PoorInterpreter::Result match; \
    EXPECT_TRUE(interpreter->search(input, &match)); \
    EXPECT_EQ(match.start, begin); \
    EXPECT_EQ(match.length, len); \
} while (0)

#define POOR_MATCH_ASSERT(input, expect) { \
    EXPECT_EQ(interpreter->match(input), expect); \
} while (0)

// valid C identifiers (K&R2: A.2.3), plus '$' (supported by some compilers)
string identifier = "[a-zA-Z_$][0-9a-zA-Z_$]*";
string hexPrefix = "0[xX]";
string hexDigits = "[0-9a-fA-F]+";
string binPrefix = "0[bB]";
string binDigits = "[01]+";

// integer constants (K&R2: A.2.5.1)
string integerSuffixOpt = "(([uU]ll)|([uU]LL)|(ll[uU]?)|(LL[uU]?)|([uU][lL])|([lL][uU]?)|[uU])?";
string decimalConstant = "(0"+integerSuffixOpt+")|([1-9][0-9]*"+integerSuffixOpt+")";
string octalConstant = "0[0-7]*"+integerSuffixOpt;
string hexConstant = hexPrefix+hexDigits+integerSuffixOpt;
string binConstant = binPrefix+binDigits+integerSuffixOpt;
string badOctalConstant = "0[0-7]*[89]";

/**
 * character constants (K&R2: A.2.5.2)
 * Note: a-zA-Z and '.-~^_!=&;,' are allowed as escape chars to support #line
 * directives with Windows paths as filenames (..\..\dir\file)
 * For the same reason, decimalEscape allows all digit sequences. We want to
 * parse all correct code, even if it means to sometimes parse incorrect
 * code.
 **/
string simpelEscape = "([a-zA-Z._~!=&\\^\\-\\\\?'\"])";
string decimalEscape = "([0-9]+)";
string hexEscape = "(x[0-9a-fA-F]+)";
string badEscape = "([\\\\][^a-zA-Z._~^!=&\\^\\-\\\\?'\"x0-7])";
string escapeSequence = "(\\\\("+simpelEscape+'|'+decimalEscape+'|'+hexEscape+"))";
string cconstChar = "([^'\\\\\\n]|"+escapeSequence+')';
string charConst = "'"+cconstChar+"'";
string wcharConst = "L"+charConst;
string unmatchedQuote = "('"+cconstChar+"*\\n)|('"+cconstChar+"*$)";
string badCharConst = "('"+cconstChar+"[^'\\n]+')|('')|('"+badEscape+"[^'\\n]*')";

// string literals (K&R2: A.2.6)
string stringChar = "([^\"\\\\\\n]|"+escapeSequence+')';
string stringLiteral = "\""+stringChar+"*\"";
string wstringLiteral = "L"+stringLiteral;
string badStringLiteral = "\""+stringChar+"*?"+badEscape+stringChar+"*\"";

// floating constants (K&R2: A.2.5.3)
string exponentPart = "([eE][-+]?[0-9]+)";
string fractionalConstant = "([0-9]*\\.[0-9]+)|([0-9]+\\.)";
string floatingConstant = "(((("+fractionalConstant+")"+exponentPart+"?)|([0-9]+"+exponentPart+"))[FfLl]?)";
string binaryExponentPart = "([pP][+-]?[0-9]+)";
string hexFractionalConstant = "((("+hexDigits+")?\\."+hexDigits+")|("+hexDigits+"\\.))";
string hexFloatingConstant = "("+hexPrefix+"("+hexDigits+"|"+hexFractionalConstant+")"+binaryExponentPart+"[FfLl]?)";

// Step 2. Use the TEST macro to define your tests.
//
// TEST has two parameters: the test case name and the test name.
// After using the macro, you should define your test logic between a
// pair of braces.  You can use a bunch of macros to indicate the
// success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
// examples of such macros.  For a complete list, see gtest.h.

PoorInterpreter::Ptr initPoorInterpreter(string re) {
    auto  regex = parseRegex(re);
    Range<unsigned char>::List unifiedRanges;
    regex->setNormalize(&unifiedRanges);
    regex->setUnify(unifiedRanges);
    auto nfa = regex->generateEpsilonNfa();
    auto dfa = powerset(nfa, poorEpsilonChecker);
    auto mdfa = Hopcroft(dfa);
    //mdfa->toMermaid(std::cout);
    return PoorInterpreter::Ptr(new PoorInterpreter(mdfa));
}

// identifier = "[a-zA-Z_$][0-9a-zA-Z_$]*";
TEST(PoorInterpreter, Identifier) {
    auto interpreter = initPoorInterpreter(identifier);
    POOR_SEARCH_ASSERT("abc", 0, 3);
    POOR_SEARCH_ASSERT("a101", 0, 4);
    EXPECT_FALSE(interpreter->search("10", nullptr));
}

// hexPrefix = "0[xX]";
TEST(PoorInterpreter, HexPrefix) {
    auto interpreter = initPoorInterpreter(hexPrefix);
    POOR_MATCH_ASSERT("0x", true);
    POOR_MATCH_ASSERT("0X", true);
    EXPECT_FALSE(interpreter->search("0", nullptr));
}

//string hexDigits = "[0-9a-fA-F]+";
TEST(PoorInterpreter, HexDigits) {
    auto interpreter = initPoorInterpreter(hexDigits);
    POOR_MATCH_ASSERT("deadcode", false);
    POOR_MATCH_ASSERT("badbeef", true);
    POOR_MATCH_ASSERT("0123456789", true);
    POOR_MATCH_ASSERT("DEADCODE", false);
    POOR_MATCH_ASSERT("BADBEEF", true);
    POOR_MATCH_ASSERT("0123456789ABCDEF", true);
}

// binPrefix = "0[bB]";
TEST(PoorInterpreter, BinPrefix) {
    auto interpreter = initPoorInterpreter(binPrefix);
    POOR_MATCH_ASSERT("0b", true);
    POOR_MATCH_ASSERT("0B", true);
    POOR_MATCH_ASSERT("0x", false);
    POOR_MATCH_ASSERT("b", false);
}

// binDigits = "[01]+";
TEST(PoorInterpreter, BinDigits) {
    auto interpreter = initPoorInterpreter(binDigits);
    POOR_MATCH_ASSERT("0123456789", false);
    POOR_SEARCH_ASSERT("0123456789", 0, 2);
    POOR_MATCH_ASSERT("001010101", true);
}

// integerSuffixOpt = "(([uU]ll)|([uU]LL)|(ll[uU]?)|(LL[uU]?)|([uU][lL])|([lL][uU]?)|[uU])?";
TEST(PoorInterpreter, IntegerSuffixOpt) {
    auto interpreter = initPoorInterpreter(integerSuffixOpt);
    POOR_MATCH_ASSERT("ull", true);
    POOR_MATCH_ASSERT("Ull", true);
    POOR_MATCH_ASSERT("uLL", true);
    POOR_MATCH_ASSERT("ULL", true);
    POOR_MATCH_ASSERT("llu", true);
    POOR_MATCH_ASSERT("llU", true);
    POOR_MATCH_ASSERT("LLu", true);
    POOR_MATCH_ASSERT("LLU", true);
    POOR_MATCH_ASSERT("ul", true);
    POOR_MATCH_ASSERT("uL", true);
    POOR_MATCH_ASSERT("Ul", true);
    POOR_MATCH_ASSERT("UL", true);
    POOR_MATCH_ASSERT("l", true);
    POOR_MATCH_ASSERT("lu", true);
    POOR_MATCH_ASSERT("lU", true);
    POOR_MATCH_ASSERT("L", true);
    POOR_MATCH_ASSERT("Lu", true);
    POOR_MATCH_ASSERT("LU", true);
    POOR_MATCH_ASSERT("u", true);
    POOR_MATCH_ASSERT("U", true);
    POOR_MATCH_ASSERT("", true);
}

// decimalConstant = "(0"+integerSuffixOpt+")|([1-9][0-9]*"+integerSuffixOpt+")";
TEST(PoorInterpreter, DecimalConstant) {
    auto interpreter = initPoorInterpreter(decimalConstant);
    POOR_MATCH_ASSERT("0ull", true);
    POOR_MATCH_ASSERT("0Ull", true);
    POOR_MATCH_ASSERT("0uLL", true);
    POOR_MATCH_ASSERT("0ULL", true);
    POOR_MATCH_ASSERT("0llu", true);
    POOR_MATCH_ASSERT("0llU", true);
    POOR_MATCH_ASSERT("0LLu", true);
    POOR_MATCH_ASSERT("0LLU", true);
    POOR_MATCH_ASSERT("0ul", true);
    POOR_MATCH_ASSERT("0uL", true);
    POOR_MATCH_ASSERT("0Ul", true);
    POOR_MATCH_ASSERT("0UL", true);
    POOR_MATCH_ASSERT("0l", true);
    POOR_MATCH_ASSERT("0lu", true);
    POOR_MATCH_ASSERT("0lU", true);
    POOR_MATCH_ASSERT("0L", true);
    POOR_MATCH_ASSERT("0Lu", true);
    POOR_MATCH_ASSERT("0LU", true);
    POOR_MATCH_ASSERT("0u", true);
    POOR_MATCH_ASSERT("0U", true);
    POOR_MATCH_ASSERT("10ull", true);
    POOR_MATCH_ASSERT("01ull", false);
    POOR_MATCH_ASSERT("123456789ull", true);
    POOR_MATCH_ASSERT("123456789", true);
}
// octalConstant = "0[0-7]*"+integerSuffixOpt;
TEST(PoorInterpreter, OctalConstant) {
    auto interpreter = initPoorInterpreter(octalConstant);
    POOR_MATCH_ASSERT("01ull", true);
    POOR_MATCH_ASSERT("01Ull", true);
    POOR_MATCH_ASSERT("01uLL", true);
    POOR_MATCH_ASSERT("01ULL", true);
    POOR_MATCH_ASSERT("01llu", true);
    POOR_MATCH_ASSERT("01llU", true);
    POOR_MATCH_ASSERT("01LLu", true);
    POOR_MATCH_ASSERT("01LLU", true);
    POOR_MATCH_ASSERT("01ul", true);
    POOR_MATCH_ASSERT("01uL", true);
    POOR_MATCH_ASSERT("01Ul", true);
    POOR_MATCH_ASSERT("01UL", true);
    POOR_MATCH_ASSERT("01l", true);
    POOR_MATCH_ASSERT("01lu", true);
    POOR_MATCH_ASSERT("01lU", true);
    POOR_MATCH_ASSERT("01L", true);
    POOR_MATCH_ASSERT("01Lu", true);
    POOR_MATCH_ASSERT("01LU", true);
    POOR_MATCH_ASSERT("01u", true);
    POOR_MATCH_ASSERT("01U", true);
    POOR_MATCH_ASSERT("01", true);
}

// hexConstant = hexPrefix+hexDigits+integerSuffixOpt;
TEST(PoorInterpreter, HexConstant) {
    auto interpreter = initPoorInterpreter(hexConstant);
    POOR_MATCH_ASSERT("0xbadbeef", true);
    POOR_MATCH_ASSERT("0x1234", true);
    POOR_MATCH_ASSERT("0x1234u", true);
    POOR_MATCH_ASSERT("0x1234ull", true);
}
// binConstant = binPrefix+binDigits+integerSuffixOpt;
TEST(PoorInterpreter, BinConstant) {
    auto interpreter = initPoorInterpreter(binConstant);
    POOR_MATCH_ASSERT("0b11", true);
    POOR_MATCH_ASSERT("0b11", true);
    POOR_MATCH_ASSERT("0B1001", true);
}
// badOctalConstant = "0[0-7]*[89]";
TEST(PoorInterpreter, BadOctalConstant) {
    auto interpreter = initPoorInterpreter(badOctalConstant);
    POOR_MATCH_ASSERT("08", true);
    POOR_MATCH_ASSERT("008", true);
    POOR_MATCH_ASSERT("01239", true);
}
// simpelEscape = "([a-zA-Z._~!=&\\^\\-\\\\?'\"])";
TEST(PoorInterpreter, SimpelEscape) {
    auto interpreter = initPoorInterpreter(simpelEscape);
    POOR_MATCH_ASSERT(".", true);
    POOR_MATCH_ASSERT("_", true);
    POOR_MATCH_ASSERT("~", true);
    POOR_MATCH_ASSERT("!", true);
    POOR_MATCH_ASSERT("=", true);
    POOR_MATCH_ASSERT("&", true);
    POOR_MATCH_ASSERT("^", true);
    POOR_MATCH_ASSERT("-", true);
    POOR_MATCH_ASSERT("\\", true);
    POOR_MATCH_ASSERT("?", true);
    POOR_MATCH_ASSERT("'", true);
    POOR_MATCH_ASSERT("\"", true);
}
// decimalEscape = "([0-9]+)";
TEST(PoorInterpreter, DecimalEscape) {
    auto interpreter = initPoorInterpreter(decimalEscape);
    POOR_MATCH_ASSERT("0", true);
    POOR_MATCH_ASSERT("1", true);
    POOR_MATCH_ASSERT("2", true);
    POOR_MATCH_ASSERT("3", true);
    POOR_MATCH_ASSERT("4", true);
    POOR_MATCH_ASSERT("5", true);
    POOR_MATCH_ASSERT("6", true);
    POOR_MATCH_ASSERT("7", true);
    POOR_MATCH_ASSERT("8", true);
    POOR_MATCH_ASSERT("9", true);
}
// hexEscape = "(x[0-9a-fA-F]+)";
TEST(PoorInterpreter, HexEscape) {
    auto interpreter = initPoorInterpreter(hexEscape);
    POOR_MATCH_ASSERT("x0", true);
    POOR_MATCH_ASSERT("xa", true);
    POOR_MATCH_ASSERT("xA", true);
    POOR_MATCH_ASSERT("x0A", true);
    POOR_MATCH_ASSERT("xaA", true);
    POOR_MATCH_ASSERT("xg", false);
}
// badEscape = "([\\\\][^a-zA-Z._~^!=&\\^\\-\\\\?'\"x0-7])";
TEST(PoorInterpreter, BadEscape) {
    auto interpreter = initPoorInterpreter(badEscape);
    POOR_MATCH_ASSERT("\\8", true);
    POOR_MATCH_ASSERT("\\a", false);
    POOR_MATCH_ASSERT("\\\"", false);
}
// escapeSequence = "(\\\\("+simpelEscape+'|'+decimalEscape+'|'+hexEscape+"))";
TEST(PoorInterpreter, EscapeSequence) {
    auto interpreter = initPoorInterpreter(escapeSequence);
    POOR_MATCH_ASSERT("\\\\", true);
    POOR_MATCH_ASSERT("\\^", true);
    POOR_MATCH_ASSERT("\\x0", true);
    POOR_MATCH_ASSERT("\\xbf", true);
    POOR_MATCH_ASSERT("\\123", true);
}
// cconstChar = "([^'\\\\\\n]|"+escapeSequence+')';
TEST(PoorInterpreter, CconstChar) {
    auto interpreter = initPoorInterpreter(cconstChar);
    POOR_MATCH_ASSERT("a", true);
    POOR_MATCH_ASSERT("\n", false);
}
// charConst = "'"+cconstChar+"'";
TEST(PoorInterpreter, CharConst) {
    auto interpreter = initPoorInterpreter(charConst);
    POOR_MATCH_ASSERT("'\\x00'", true); // beyond poor interpreter
    POOR_MATCH_ASSERT("'a'", true);
    POOR_MATCH_ASSERT("'\\n'", true);
}
// wcharConst = "L"+charConst;
TEST(PoorInterpreter, WcharConst) {
    auto interpreter = initPoorInterpreter(wcharConst);
    POOR_MATCH_ASSERT("L'a'", true);
    POOR_MATCH_ASSERT("L'\\n'", true);
}
// unmatchedQuote = "('"+cconstChar+"*\\n)|('"+cconstChar+"*$)";
// TEST(RichInterpreter, XXXX)

// badCharConst = "('"+cconstChar+"[^'\\n]+')|('')|('"+badEscape+"[^'\\n]*')";
TEST(PoorInterpreter, BadCharConst) {
    auto interpreter = initPoorInterpreter(badCharConst);
    POOR_MATCH_ASSERT("'ab'", true);
    POOR_MATCH_ASSERT("'\\8a'", true);
    POOR_MATCH_ASSERT("'a'", false);
    POOR_MATCH_ASSERT("'\\n'", false);
}

// stringChar = "([^\"\\\\\\n]|"+escapeSequence+')';
TEST(PoorInterpreter, StringChar) {
    auto interpreter = initPoorInterpreter(stringChar);
    POOR_MATCH_ASSERT("\\\\", true);
    POOR_MATCH_ASSERT("\\^", true);
    POOR_MATCH_ASSERT("\\x0", true);
    POOR_MATCH_ASSERT("\\xbf", true);
    POOR_MATCH_ASSERT("\\123", true);
}
// stringLiteral = "\""+stringChar+"*\"";
TEST(PoorInterpreter, StringLiteral) {
    auto interpreter = initPoorInterpreter(stringLiteral);
    POOR_MATCH_ASSERT("\"\\\\\"", true);
    POOR_MATCH_ASSERT("\"buptlxb\"", true);
}
// wstringLiteral = "L"+stringLiteral;
TEST(PoorInterpreter, WstringLiteral) {
    auto interpreter = initPoorInterpreter(wstringLiteral);
    POOR_MATCH_ASSERT("L\"\\\\\"", true);
    POOR_MATCH_ASSERT("L\"buptlxb\"", true);
}
// badStringLiteral = "\""+stringChar+"*?"+badEscape+stringChar+"*\"";
// TEST(RichInterpreter, XXXX)

// exponentPart = "([eE][-+]?[0-9]+)";
TEST(PoorInterpreter, ExponentPart) {
    auto interpreter = initPoorInterpreter(exponentPart);
    POOR_MATCH_ASSERT("e1", true);
    POOR_MATCH_ASSERT("E2", true);
    POOR_MATCH_ASSERT("e+123", true);
    POOR_MATCH_ASSERT("e-123", true);
    POOR_MATCH_ASSERT("E+012", true);
    POOR_MATCH_ASSERT("E-012", true);
}
// fractionalConstant = "([0-9]*\\.[0-9]+)|([0-9]+\\.)";
TEST(PoorInterpreter, FractionalConstant) {
    auto interpreter = initPoorInterpreter(fractionalConstant);
    POOR_MATCH_ASSERT(".0", true);
    POOR_MATCH_ASSERT("0.0", true);
    POOR_MATCH_ASSERT(".01", true);
    POOR_MATCH_ASSERT("1.10", true);
    POOR_MATCH_ASSERT("0.", true);
    POOR_MATCH_ASSERT("123.", true);
}
// floatingConstant = "(((("+fractionalConstant+")"+exponentPart+"?)|([0-9]+"+exponentPart+"))[FfLl]?)";
TEST(PoorInterpreter, FloatingConstant) {
    auto interpreter = initPoorInterpreter(floatingConstant);
    POOR_MATCH_ASSERT(".0", true);
    POOR_MATCH_ASSERT("0.0", true);
    POOR_MATCH_ASSERT(".01", true);
    POOR_MATCH_ASSERT("1.10", true);
    POOR_MATCH_ASSERT("0.", true);
    POOR_MATCH_ASSERT("123.", true);
    POOR_MATCH_ASSERT(".0e1", true);
    POOR_MATCH_ASSERT("0.0E2", true);
    POOR_MATCH_ASSERT(".01e+123", true);
    POOR_MATCH_ASSERT("1.10e-123", true);
    POOR_MATCH_ASSERT("0.E+012", true);
    POOR_MATCH_ASSERT("123.E-012", true);
    POOR_MATCH_ASSERT(".0F", true);
    POOR_MATCH_ASSERT("0.0f", true);
    POOR_MATCH_ASSERT(".01L", true);
    POOR_MATCH_ASSERT("1.10l", true);
}
// binaryExponentPart = "([pP][+-]?[0-9]+)";
TEST(PoorInterpreter, BinaryExponentPart) {
    auto interpreter = initPoorInterpreter(binaryExponentPart);
    POOR_MATCH_ASSERT("p0", true);
    POOR_MATCH_ASSERT("p123", true);
    POOR_MATCH_ASSERT("p+0", true);
    POOR_MATCH_ASSERT("p-0", true);
    POOR_MATCH_ASSERT("p+123", true);
    POOR_MATCH_ASSERT("P0", true);
    POOR_MATCH_ASSERT("P123", true);
    POOR_MATCH_ASSERT("P+0", true);
    POOR_MATCH_ASSERT("P-0", true);
    POOR_MATCH_ASSERT("P+123", true);
}
// hexFractionalConstant = "((("+hexDigits+")?\\."+hexDigits+")|("+hexDigits+"\\.))";
TEST(PoorInterpreter, HexFractionalConstant) {
    auto interpreter = initPoorInterpreter(hexFractionalConstant);
    POOR_MATCH_ASSERT("badbeef.0123", true);
    POOR_MATCH_ASSERT("0123.badbeef", true);
    POOR_MATCH_ASSERT("badbeef.", true);
    POOR_MATCH_ASSERT("0123.", true);
    POOR_MATCH_ASSERT("123badbeef.badbeef123", true);
}
// hexFloatingConstant = "("+hexPrefix+"("+hexDigits+"|"+hexFractionalConstant+")"+binaryExponentPart+"[FfLl]?)";
TEST(PoorInterpreter, HexFloatingConstant) {
    auto interpreter = initPoorInterpreter(hexFloatingConstant);
    POOR_MATCH_ASSERT("0xbadbeep0f", true);
    POOR_MATCH_ASSERT("0xbadbeef.213P0", true);
    POOR_MATCH_ASSERT("0xbadbeef.213p+0F", true);
    POOR_MATCH_ASSERT("0xbadbeef.213p-0f", true);
    POOR_MATCH_ASSERT("0xbadbeef.213p+123l", true);
    POOR_MATCH_ASSERT("0xbadbeef.213P-123L", true);
    POOR_MATCH_ASSERT("0Xbadbeep0f", true);
    POOR_MATCH_ASSERT("0Xbadbeef.213P0", true);
    POOR_MATCH_ASSERT("0Xbadbeef.213p+0F", true);
    POOR_MATCH_ASSERT("0Xbadbeef.213p-0f", true);
    POOR_MATCH_ASSERT("0Xbadbeef.213p+123l", true);
    POOR_MATCH_ASSERT("0Xbadbeef.213P-123L", true);
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
