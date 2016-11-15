#include <iostream>
#include <vector>
#include <stack>
#include "finite_automaton.h"

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
    std::vector<Token> postfix{
        {TokenType::OPERAND, 'a'},
        {TokenType::CONCATENATE, '+'},
        {TokenType::LEFT_PARENTTHESIS, '('},
        {TokenType::OPERAND, 'b'},
        {TokenType::BAR, '|'},
        {TokenType::OPERAND, 'c'},
        {TokenType::RIGHT_PARENTHESIS, ')'},
        {TokenType::STAR, '*'}
    };
    std::vector<Token> tokens = postfix2postfix(postfix);
    for (auto &token : tokens)
        std::cout << token.value << " ";
    std::cout << std::endl;

    EpsilonNFA res = Thompson(tokens);
    std::cout << res << std::endl;

    std::cout << res.toMermaid() << std::endl;

    DFA dfa(res);
    std::cout << dfa.toMermaid() << std::endl;
    return 0;
}
