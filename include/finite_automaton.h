#ifndef FINITE_AUTOMATON_H
#define FINITE_AUTOMATON_H

#include <vector>
#include <set>
#include <map>
#include <queue>
#include <iosfwd>
#include <unordered_map>
#include "token.h"
#include "utility.h"
#include "container.h"
#include <list>
#include <memory>

struct State;
struct Transition;

struct Transition {
    typedef std::shared_ptr<Transition> Ptr;
    typedef std::list<Ptr> List;
    enum Type
    {
        Chars,
        Epsilon,
        BeginString,
        EndString,
        Nop
    };
    std::shared_ptr<State> source;
    std::shared_ptr<State> target;
    Range<unsigned char> range;
    Type type;
};

struct State {
    typedef std::shared_ptr<State> Ptr;
    typedef std::list<Ptr> List;
    typedef std::set<Ptr> Set;
    Transition::List inbounds;
    Transition::List outbounds;
    bool isAccepted;
};

class Automaton {
public:
    typename Transition::List transitions;
    typename State::List states;
    typename State::Ptr startState;
    typedef std::shared_ptr<Automaton> Ptr;
    Automaton() : startState(nullptr) {}
    State::Ptr getState();
    Transition::Ptr getTransition(State::Ptr from, State::Ptr to);
    Transition::Ptr getChars(State::Ptr from, State::Ptr to, Range<unsigned char> range);
    Transition::Ptr getEpsilon(State::Ptr from, State::Ptr to);
    Transition::Ptr getBeginString(State::Ptr from, State::Ptr to);
    Transition::Ptr getEndString(State::Ptr from, State::Ptr to);
    Transition::Ptr getNop(State::Ptr from, State::Ptr to);
    std::ostream & toMermaid(std::ostream &);
    void reverse();
    void reachableTrim();
};

extern bool poorEpsilonChecker(Transition::Ptr);
extern bool richEpsilonChecker(Transition::Ptr);
extern void epsilonClosure(State::Ptr dfaState, State::Ptr nfaState, bool (*epsilonChecker)(Transition::Ptr), State::Set &epsilonStates, Transition::List &transitions);
extern bool epsilonClosure(typename State::Ptr nfaState, bool (*epsilonChecker)(Transition::Ptr), State::Set &epsilonStates, std::unordered_map<Transition::Ptr, State::Set> &transitions);
extern Automaton::Ptr subset(Automaton::Ptr nfa, bool (*epsilonChecker)(Transition::Ptr));
extern Automaton::Ptr epsilonNfaToDfa(Automaton::Ptr nfa, bool (*epsilonChecker)(Transition::Ptr));

struct EpsilonNfa {
    State::Ptr start;
    State::Ptr finish;
};


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
