#include <iostream>
#include <sstream>
#include <set>
#include <stack>
#include "finite_automaton.h"
#include "utility.h"

EpsilonNFA EpsilonNFA::operator+ (const EpsilonNFA &that) const {
    EpsilonNFA res(*this);
    return res += that;
}

EpsilonNFA EpsilonNFA::operator| (const EpsilonNFA &that) const {
    EpsilonNFA res(*this);
    return res |= that;
}

EpsilonNFA & EpsilonNFA::operator* () {
    debug("star(EpsilonNFA@0x%p)\n", this);
    FANode *ns0 = new FANode, *nsa = new FANode;
    nsa->isAccepted = true;

    this->sa->isAccepted = false;
    this->sa->next.emplace(EPSILON, this->s0);
    this->sa->next.emplace(EPSILON, nsa);
    nsa->prev.push_back(this->sa);

    ns0->next.emplace(EPSILON, this->s0);
    ns0->next.emplace(EPSILON, nsa);

    this->s0 = ns0;
    this->sa = nsa;
    return *this;
}

EpsilonNFA & EpsilonNFA::operator+= (const EpsilonNFA &that) {
    debug("EpsilonNFA@0x%p + EpsilonNFA@0x%p\n", this, &that);
    assertm(this->sa->next.empty(), "The accepted state of EpsilonNFA@0x%p should not have any succeeding", this);
    this->sa->isAccepted = false;

    std::map<FANode *, FANode *> dict;

    dict.emplace(that.s0, new FANode);
    this->sa->next.emplace(EPSILON, dict[that.s0]);
    dict[that.s0]->prev.push_back(this->sa);

    dict.emplace(that.sa, new FANode);
    dict[that.sa]->isAccepted = true;
    copy(that.s0, dict);
    this->sa = dict[that.sa];

    return *this;
}

EpsilonNFA & EpsilonNFA::operator|= (const EpsilonNFA &that) {
    debug("EpsilonNFA@0x%p | EpsilonNFA@0x%p\n", this, &that);
    FANode *ns0 = new FANode, *nsa = new FANode;
    nsa->isAccepted = true;

    ns0->next.emplace(EPSILON, this->s0);
    this->s0->prev.push_back(ns0);
    this->sa->isAccepted = false;
    this->sa->next.emplace(EPSILON, nsa);
    nsa->prev.push_back(this->sa);

    this->s0 = ns0;
    this->sa = nsa;

    std::map<FANode *, FANode *> dict;

    dict.emplace(that.s0, new FANode);
    ns0->next.emplace(EPSILON, dict[that.s0]);
    dict[that.s0]->prev.push_back(ns0);
    dict.emplace(that.sa, new FANode);
    dict[that.sa]->next.emplace(EPSILON, nsa);
    nsa->prev.push_back(dict[that.sa]);
    copy(that.s0, dict);

    return *this;
}

std::ostream & operator<< (std::ostream &os, const FANode *fan) {
    os << "0x" << std::hex << reinterpret_cast<unsigned long>(fan) << std::dec;
    if (fan->isAccepted)
        os << "[$]";
    return os;
}

std::ostream & operator<< (std::ostream &os, const FANode &fan) {
    return operator<< (os, &fan);
}

std::ostream & operator<< (std::ostream &os, const FiniteAutomaton &fa) {
    std::queue<FANode *> q;
    std::set<FANode *> mem;
    q.push(fa.s0);
    while (!q.empty()) {
            auto cur = q.front();
            q.pop();
            os << cur << "=>{";
            for (auto n : cur->next) {
                if (mem.find(n.second) == mem.end() && !n.second->isAccepted) {
                    mem.emplace(n.second);
                    q.push(n.second);
                }
                os << n.first << ":" << n.second << ";";
            }
            os << "};";
    }
    return os;
}

std::string FiniteAutomaton::toMermaid() {
    std::ostringstream os;
    std::queue<FANode *> q;
    std::map<FANode *, unsigned> dict;
    unsigned index = 0;
    q.push(this->s0);
    dict.emplace(this->s0, index++);
    while (!q.empty()) {
            auto cur = q.front();
            q.pop();
            for (auto n : cur->next) {
                if (dict.find(n.second) == dict.end()) {
                    dict.emplace(n.second, index++);
                    q.push(n.second);
                }
                os << "s" << dict[cur] << "--";
                if (n.first == EPSILON)
                    os << "epsilon";
                else
                    os << n.first;
                os << "-->" << "s" << dict[n.second];
                if (n.second->isAccepted)
                    os << "((" << "s" << dict[n.second] << "))";
                os << '\n';
            }
    }
    return os.str();
}

std::set<FANode *> EpsilonNFA::epsilonClosure(std::set<FANode *> nodes) const {
    std::set<FANode *> closure(nodes);
    std::queue<FANode *> q;
    for (auto node : nodes)
        q.push(node);
    while (!q.empty()) {
        FANode *cur = q.front();
        q.pop();
        auto range = cur->next.equal_range(EPSILON);
        for (auto i = range.first, iend = range.second; i != iend; ++i) {
            if (closure.insert(i->second).second)
                q.push(i->second);
        }
    }
    return closure;
}

#define EpsilonNFA_UNARY(st, op) do { \
    assert(st.size() >= 1); \
    auto rhs = st.top(); \
    st.pop(); \
    st.emplace(op rhs); \
} while(0)

#define EpsilonNFA_BINARY(st, op) do { \
    assert(st.size() >= 2); \
    auto rhs = st.top(); \
    st.pop(); \
    auto lhs = st.top(); \
    st.pop(); \
    st.emplace(lhs op rhs); \
} while(0)

EpsilonNFA Thompson(const std::vector<Token> &re)
{
    std::stack<EpsilonNFA> st;
    for (auto &token : re) {
        switch (token.type) {
            case TokenType::OPERAND:
                st.emplace(token.value);
                break;
            case TokenType::STAR:
                EpsilonNFA_UNARY(st, *);
                break;
            case TokenType::CONCATENATE:
                EpsilonNFA_BINARY(st, +);
                break;
            case TokenType::BAR:
                EpsilonNFA_BINARY(st, |);
                break;
            default:
                abort();
        }
    }
    return st.top();
}

DFA::DFA(const EpsilonNFA &that) {
    std::map<std::set<FANode *>, FANode *> dict;
    std::queue<std::set<FANode *>> q;
    q.push(that.epsilonClosure({that.s0}));
    this->s0 = new FANode;
    this->s0->isAccepted = q.front().find(that.sa) != q.front().end();
    if (this->s0->isAccepted)
        sas.push_back(this->s0);
    dict.emplace(q.front(), this->s0);
    while (!q.empty()) {
        std::set<FANode *> cur = q.front();
        q.pop();
        std::map<char, std::set<FANode *>> transitions;
        for (auto n : cur) {
            for (auto p : n->next) {
                if (p.first == EPSILON)
                    continue;
                transitions[p.first].insert(p.second);
            }
        }
        for (auto &t : transitions) {
            t.second = that.epsilonClosure(t.second);
            if (dict.find(t.second) == dict.end()) {
                FANode *tmp = new FANode;
                tmp->isAccepted = t.second.find(that.sa) != t.second.end();
                if (tmp->isAccepted)
                    sas.push_back(tmp);
                dict.emplace(t.second, tmp);
                q.push(t.second);
            }
            dict[cur]->next.emplace(t.first, dict[t.second]);
        }
    }
}

#ifdef _TEST_EpsilonNFA
int main(void)
{
    EpsilonNFA nfa1('a'), nfa2('b');
    std::cout << nfa1 + nfa2 << std::endl;
    std::cout << (nfa1 | nfa2) << std::endl;
    std::cout << *nfa1 << std::endl;
    std::cout << nfa1.toMermaid() << std::endl;
    return 0;
}
#endif
