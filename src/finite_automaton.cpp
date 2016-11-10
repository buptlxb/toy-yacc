#include <iostream>
#include <sstream>
#include <set>
#include "finite_automaton.h"
#include "utility.h"

NFA NFA::operator+ (const NFA &that) const {
    NFA res(this);
    return res += that;
}

NFA NFA::operator| (const NFA &that) const {
    NFA res(this);
    return res |= that;
}

NFA & NFA::operator* () {
    debug("star(NFA@0x%p)\n", this);
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

NFA & NFA::operator+= (const NFA &that) {
    debug("NFA@0x%p + NFA@0x%p\n", this, &that);
    assertm(this->sa->next.empty(), "The accepted state of NFA@0x%p should not have any succeeding", this);
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

NFA & NFA::operator|= (const NFA &that) {
    debug("NFA@0x%p | NFA@0x%p\n", this, &that);
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

std::ostream & operator<< (std::ostream &os, const NFA &nfa) {
    std::queue<FANode *> q;
    std::set<FANode *> mem;
    q.push(nfa.s0);
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

std::string NFA::to_mermaid() {
    std::ostringstream os;
    std::queue<FANode *> q;
    std::map<FANode *, unsigned> dict;
    unsigned index = 0;
    q.push(this->s0);
    dict.emplace(this->s0, index++);
    dict.emplace(this->sa, index++);
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
                    os << "eplisoin";
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

#ifdef _TEST_NFA
int main(void)
{
    NFA nfa1('a'), nfa2('b');
    std::cout << nfa1 + nfa2 << std::endl;
    std::cout << (nfa1 | nfa2) << std::endl;
    std::cout << *nfa1 << std::endl;
    std::cout << nfa1.to_mermaid() << std::endl;
    return 0;
}
#endif
