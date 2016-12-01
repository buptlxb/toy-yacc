#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <set>
#include <iosfwd>
#include <unordered_map>
#include <list>
#include <memory>
#include "container.h"

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

#endif
