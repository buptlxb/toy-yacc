#include <iostream>
#include <vector>
#include <stack>
#include <fstream>
#include "finite_automaton.h"
#include "regex_expression.h"

std::vector<Token> postfix2postfix(const std::vector<Token> &tokens)
{
    std::vector<Token> postfix;
    postfix.reserve(tokens.size());
    std::stack<Token> st;
    for (auto &token : tokens) {
        if (token.type == TokenType::OPERAND)
            postfix.push_back(token);
        else if (token.type == TokenType::RIGHT_PARENTHESIS) {
            while (!st.empty() && TokenType::LEFT_PARENTTHESIS != st.top().type) {
                postfix.push_back(st.top());
                st.pop();
            }
            st.pop();
        } else {
            if (token.type != TokenType::LEFT_PARENTTHESIS) {
                while (!st.empty() && token.type >= st.top().type) {
                    postfix.push_back(st.top());
                    st.pop();
                }
            }
            st.push(token);
        }
    }
    while (!st.empty()) {
        postfix.push_back(st.top());
        st.pop();
    }
    return postfix;
}

int main(void)
{
    // std::vector<Token> postfix{
    //     {TokenType::OPERAND, 'a'},
    //     {TokenType::CONCATENATE, '+'},
    //     {TokenType::LEFT_PARENTTHESIS, '('},
    //     {TokenType::OPERAND, 'b'},
    //     {TokenType::BAR, '|'},
    //     {TokenType::OPERAND, 'c'},
    //     {TokenType::RIGHT_PARENTHESIS, ')'},
    //     {TokenType::STAR, '*'}
    // };
    // std::vector<Token> tokens = postfix2postfix(postfix);
    // for (auto &token : tokens)
    //     std::cout << token.value << " ";
    // std::cout << std::endl;

    // EpsilonNFA res = Thompson(tokens);
    // std::cout << res << std::endl;

    // std::cout << res.toMermaid() << std::endl;

    // DFA dfa(res);
    // std::cout << dfa.toMermaid() << std::endl;

    // dfa.minimize();
    // std::cout << dfa.toMermaid() << std::endl;

    // const char *input = "(([0-9]+)(\\.[0-9]+)(e(\\+|-)?([0-9]+))? | ([0-9]+)e(\\+|-)?([0-9]+))([lL]|[fF])?";
    const char *input = "(L)?'([^\\\\\\n]|(\\\\.))*?'";
    // const char *input = "[^abcb-x]";
    // const char *input = "(b|c)";
    auto regex = parseRegex(input);
    regex->positize();
    std::ofstream ofs("r.dot");
    regex->graphviz(ofs);
    auto automaton = regex->generateEpsilonNfa();
    automaton->toMermaid(std::cout) << std::endl;;

    // auto dfa = epsilonNfaToDfa(automaton, poorEpsilonChecker);
    // dfa->toMermaid(std::cout) << std::endl;

    std::cout << "reverse1: " << automaton->states.size() << " " << automaton->transitions.size() << std::endl;
    automaton->reverse();
    automaton->toMermaid(std::cout) << std::endl;

    std::cout << "subset1: " << automaton->states.size() << " " << automaton->transitions.size()  << std::endl;
    auto tdfa = subset(automaton, poorEpsilonChecker);
    tdfa->toMermaid(std::cout) << std::endl;
    
    std::cout << "reachable1: " << tdfa->states.size() << " " << tdfa->transitions.size()  << std::endl;
    tdfa->reachableTrim();
    tdfa->toMermaid(std::cout) << std::endl;

    std::cout << "reverse2: " << tdfa->states.size() << " " << tdfa->transitions.size()  << std::endl;
    tdfa->reverse();
    tdfa->toMermaid(std::cout) << std::endl;

    std::cout << "subset2: " << tdfa->states.size() << " " << tdfa->transitions.size()  << std::endl;
    auto mdfa = subset(tdfa, poorEpsilonChecker);
    mdfa->toMermaid(std::cout) << std::endl;

    std::cout << "reachable2: " << mdfa->states.size() << " " << mdfa->transitions.size()  << std::endl;
    mdfa->reachableTrim();
    mdfa->toMermaid(std::cout) << std::endl;
    return 0;
}
