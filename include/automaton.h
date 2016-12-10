#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <set>
#include <iosfwd>
#include <unordered_map>
#include <list>
#include <memory>
#include <functional>
#include <vector>
#include "container.h"

struct State;
struct Transition;

struct Transition {
    struct Hash;
    struct EqualTo;
    enum Type
    {
        Chars,
        Epsilon,
        BeginString,
        EndString,
        Nop
    };
    
    using Ptr = std::shared_ptr<Transition>;
    using List = std::list<Ptr>;
    template <typename Value>
    using Map = std::unordered_map<Ptr, Value, Hash, EqualTo>;

    std::shared_ptr<State> source;
    std::shared_ptr<State> target;
    Range<unsigned char> range;
    Type type;
};

struct State {
    using Ptr = std::shared_ptr<State>;
    using List = std::list<Ptr>;
    using Set = std::set<Ptr>;

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
extern bool epsilonClosure(typename State::Ptr nfaState, bool (*epsilonChecker)(Transition::Ptr), State::Set &epsilonStates, Transition::Map<State::Set> &transitions);
extern Automaton::Ptr powerset(Automaton::Ptr nfa, bool (*epsilonChecker)(Transition::Ptr));
extern std::vector<State::Set> split(State::Set states, const std::set<State::Set> &partition);
extern Automaton::Ptr Hopcroft(Automaton::Ptr dfa);
extern Automaton::Ptr Brzozowski(Automaton::Ptr nfa, bool (*epsilonChecker)(Transition::Ptr));

struct EpsilonNfa {
    State::Ptr start;
    State::Ptr finish;
};

extern void print(Automaton::Ptr automaton);
#endif
