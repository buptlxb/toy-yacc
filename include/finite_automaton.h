#ifndef FINITE_AUTOMATON_H
#define FINITE_AUTOMATON_H

#include <vector>
#include <set>
#include <map>
#include <queue>
#include <iosfwd>
#include "token.h"
#include "utility.h"

constexpr char EPSILON = '\xFF';

struct FANode {
    std::vector<FANode *> prev;
    std::multimap<char, FANode *> next;
    bool isAccepted;
    FANode () : isAccepted(false) {}
};

class FiniteAutomaton {
    friend std::ostream & operator<< (std::ostream &os, const FiniteAutomaton &nfa);
protected:
    FANode *s0;
public:
    FiniteAutomaton() : s0(nullptr) {}
    FiniteAutomaton(const FiniteAutomaton &that) {
        if (!that.s0)
            return;
        std::map<FANode *, FANode *> dict;
        s0 = new FANode;
        dict[that.s0] = s0;
        copy(that.s0, dict);
    }
    FiniteAutomaton(FiniteAutomaton &&that) {
        this->s0 = that.s0;
        that.s0 = nullptr;
    }
    ~FiniteAutomaton() {
        if (!s0)
            return;
        std::set<FANode *> mem;
        std::queue<FANode *> q;
        q.push(s0);
        mem.insert(s0);
        while (!q.empty()) {
            auto cur = q.front();
            q.pop();
            for (auto &n : cur->next) {
                if (mem.find(n.second) == mem.end()) {
                    q.push(n.second);
                    mem.insert(n.second);
                }
            }
            delete cur;
        }
    }
    std::string toMermaid();
protected:
    static void copy(FANode *s0, std::map<FANode *, FANode *> &dict) {
        std::queue<FANode *> q;
        q.push(s0);
        while (!q.empty()) {
            auto cur = q.front();
            q.pop();
            for (auto &n : cur->next) {
                if (dict.find(n.second) == dict.end()) {
                    dict.emplace(n.second, new FANode);
                    dict[n.second]->isAccepted = n.second->isAccepted;
                    q.push(n.second);
                }
                dict[cur]->next.emplace(n.first, dict[n.second]);
                dict[n.second]->prev.push_back(dict[cur]);
            }
        }
    }
};

class EpsilonNFA : public FiniteAutomaton {
    FANode *sa;
public:
    EpsilonNFA (char c=EPSILON) : sa(new FANode) {
        s0 = new FANode;
        s0->next.emplace(c, sa);
        sa->prev.push_back(s0);
        sa->isAccepted = true;
    }
    EpsilonNFA (const EpsilonNFA &that) {
        if (!that.s0)
            return;
        std::map<FANode *, FANode *> dict;
        s0 = new FANode;
        dict[that.s0] = s0;
        sa = new FANode;
        dict[that.sa] = sa;
        sa->isAccepted = true;
        copy(that.s0, dict);
    }
    EpsilonNFA (EpsilonNFA &&that) : FiniteAutomaton(std::move(that)) {
        this->sa = that.sa;
        that.sa = nullptr;
    }
    EpsilonNFA operator+ (const EpsilonNFA &) const;
    EpsilonNFA operator| (const EpsilonNFA &) const;
    EpsilonNFA & operator* ();
    EpsilonNFA & operator+= (const EpsilonNFA &);
    EpsilonNFA & operator|= (const EpsilonNFA &);
    std::set<FANode *> epsilonClosure(std::set<FANode *> closure) const;

    friend class DFA;
};

class DFA : public FiniteAutomaton {
    std::set<FANode *> sas;
public:
    DFA() {}
    DFA(const DFA &that) {
        if (!that.s0)
            return;
        std::map<FANode *, FANode *> dict;
        s0 = new FANode;
        dict[that.s0] = s0;
        for (auto sa : that.sas) {
            FANode *nsa = new FANode;
            dict[sa] = nsa;
            nsa->isAccepted = true;
            sas.insert(nsa);
        }
        copy(that.s0, dict);
    }
    DFA(DFA &&that) : FiniteAutomaton(std::move(that)), sas(std::move(that.sas)) {}
    DFA(const EpsilonNFA &);
    void minimize();
private:
    std::vector<std::set<FANode *>> split(std::set<FANode *>, const std::set<std::set<FANode *>> &);
};

extern EpsilonNFA Thompson(const std::vector<Token> &re);

#endif
