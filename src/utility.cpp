#include <iostream>
#include <vector>
#include <stack>
#include <cassert>
#include "token.h"
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

#define NFA_UNARY(st, op) do { \
    assert(st.size() >= 1); \
    auto rhs = st.top(); \
    st.pop(); \
    st.emplace(op rhs); \
} while(0)

#define NFA_BINARY(st, op) do { \
    assert(st.size() >= 2); \
    auto rhs = st.top(); \
    st.pop(); \
    auto lhs = st.top(); \
    st.pop(); \
    st.emplace(lhs op rhs); \
} while(0)

NFA Thompson(const std::vector<Token> &re)
{
    std::stack<NFA> st;
    for (auto &token : re) {
        switch (token.type) {
            case TokenType::OPERAND:
                st.emplace(token.value);
                break;
            case TokenType::STAR:
                NFA_UNARY(st, *);
                break;
            case TokenType::CONCATENATE:
                NFA_BINARY(st, +);
                break;
            case TokenType::BAR:
                NFA_BINARY(st, |);
                break;
            default:
                abort();
        }
    }
    return st.top();
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

    NFA res = Thompson(tokens);
    std::cout << res << std::endl;

    std::cout << res.to_mermaid() << std::endl;
    return 0;
}
