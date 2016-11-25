#include <iostream>
#include "regex_expression.h"
#include "regex_algorithm.h"
#include <cassert>

void test_Graphviz() {
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

    //assert(parseRE(input)->equals(con6));
    //GraphvizVisitor().invoke(con6, nullptr);
    //GraphvizVisitor().invoke(parseRE(input), nullptr);
    //input = "(L)?'([^\\\\\\n]|(\\\\.))*?'";
    input = "(([0-9]+)(\\.[0-9]+)(e(\\+|-)?([0-9]+))? | ([0-9]+)e(\\+|-)?([0-9]+))([lL]|[fF])?";
    GraphvizVisitor().invoke(parseRegex(input), nullptr);
};

int main(void)
{
    test_Graphviz();
    return 0;
}
