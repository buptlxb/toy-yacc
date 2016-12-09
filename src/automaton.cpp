#include <queue>
#include <map>
#include <stack>
#include <iostream>
#include <algorithm>
#include "automaton.h"
#include "utility.h"

struct Transition::Hash : std::hash<Ptr> {
    std::size_t operator() (Transition::Ptr ptr) const {
        switch (ptr->type) {
            case Transition::Chars:
                return std::hash<uint32_t>()(((size_t)ptr->range.begin<<8) | ptr->range.end);
            default:
                return std::hash<int>()(ptr->type);
        }
    }
};
struct Transition::EqualTo : std::equal_to<Ptr> {
    bool operator() (Transition::Ptr lhs, Transition::Ptr rhs) const {
        if (lhs->type != rhs->type)
            return false;
        return lhs->type != Transition::Chars || lhs->range == rhs->range;
    }
};

std::shared_ptr<State> Automaton::getState() {
    auto s = std::make_shared<State>();
    s->isAccepted = false;
    states.emplace_back(s);
    return s;
}

Transition::Ptr Automaton::getTransition(State::Ptr from, State::Ptr to) {
    auto t = std::make_shared<Transition>();
    t->source = from;
    t->target = to;
    from->outbounds.emplace_back(t);
    to->inbounds.emplace_back(t);
    transitions.emplace_back(t);
    return t;
}

Transition::Ptr Automaton::getChars(State::Ptr from, State::Ptr to, Range<unsigned char> range) {
    auto c = getTransition(from, to);
    c->range = range;
    c->type = Transition::Chars;
    return c;
}

Transition::Ptr Automaton::getEpsilon(State::Ptr from, State::Ptr to) {
    auto e = getTransition(from, to);
    e->type = Transition::Epsilon;
    return e;
}


Transition::Ptr Automaton::getBeginString(State::Ptr from, State::Ptr to) {
    auto b = getTransition(from, to);
    b->type = Transition::BeginString;
    return b;
}

Transition::Ptr Automaton::getEndString(State::Ptr from, State::Ptr to) {
    auto e = getTransition(from, to);
    e->type = Transition::EndString;
    return e;
}

Transition::Ptr Automaton::getNop(State::Ptr from, State::Ptr to) {
    auto n = getTransition(from, to);
    n->type = Transition::Nop;
    return n;
}

std::ostream & Automaton::toMermaid(std::ostream &os) {
    std::map<State::Ptr, unsigned> dict;
    unsigned index = 0;
    for (auto state : states)
        dict.emplace(state, index++);
    for (auto state : states) {
        unsigned count = 0;
        for (auto trans : state->outbounds) {
            os << "s" << dict[state] << "--\"" << count++ << ":";
            switch (trans->type) {
                case Transition::Chars:
                    if (trans->range.begin == trans->range.end)
                        os << repr(trans->range.begin);
                    else
                        os << "[" << repr(trans->range.begin) << "-" << repr(trans->range.end) << "]";
                    break;
                case Transition::Epsilon:
                    os << "epsilon";
                    break;
                case Transition::BeginString:
                    os << "bos";
                    break;
                case Transition::EndString:
                    os << "eos";
                    break;
                case Transition::Nop:
                    os << "nop";
                    break;
                default:
                    assertm(0, "Unkonwn Transition Type: %d", trans->type);
            }
            os << "\"-->" << "s" << dict[trans->target];
            if (trans->target->isAccepted)
                os << "((" << "s" << dict[trans->target] << "))";
            os << '\n';
        }
    }
    return os;
}

void Automaton::reverse() {
    // 1. save the start state
    auto saved = startState;
    // 2. reset the startState
    startState = getState();
    // 3. reverse all the transitions;
    for (auto &transition : transitions)
        swap(transition->source, transition->target);
    for (auto &state : states) {
        if (state == startState)
            continue;
        swap(state->inbounds, state->outbounds);
        if (state->isAccepted) {
            getEpsilon(startState, state);
            state->isAccepted = false;
        }
    }
    // 4. set the saved start state as accepted state
    saved->isAccepted = true;
}

void Automaton::reachableTrim() {
    typename State::Set reachableStates;
    typename Transition::List usefulTransitions;

    std::queue<State::Ptr> statesQ;
    statesQ.push(startState);
    reachableStates.insert(startState);
    while (!statesQ.empty()) {
        auto cur = statesQ.front();
        statesQ.pop();
        for (auto trans : cur->outbounds) {
            usefulTransitions.emplace_back(trans);
            if (reachableStates.insert(trans->target).second) {
                statesQ.push(trans->target);
            }
        }
    }
    if (reachableStates.size() != states.size()) {
        states.clear();
        states.insert(states.end(), reachableStates.begin(), reachableStates.end());
    }
    transitions.swap(usefulTransitions);
}

bool poorEpsilonChecker(Transition::Ptr transition) {
    switch (transition->type) {
        case Transition::Epsilon:
        case Transition::Nop:
            return true;
        default:
            return false;
    }
}

bool richEpsilonChecker(Transition::Ptr transition) {
    switch (transition->type) {
        case Transition::Epsilon:
            return true;
        default:
            return false;
    }
}

bool epsilonClosure(typename State::Ptr nfaState, bool (*epsilonChecker)(Transition::Ptr), State::List &epsilonStates, State::Set &epsilonSet, Transition::Map<State::List> &transitions, Transition::List &precedence) {
    bool isAccepted = nfaState->isAccepted;
    if (epsilonSet.find(nfaState) == epsilonSet.end()) {
        if (epsilonSet.emplace(nfaState).second)
            epsilonStates.emplace_back(nfaState);
        for (auto trans : nfaState->outbounds) {
            if (epsilonChecker(trans)) {
                if (trans->target->isAccepted)
                    isAccepted = true;
                isAccepted |= epsilonClosure(trans->target, epsilonChecker, epsilonStates, epsilonSet, transitions, precedence);
            } else {
                if (transitions[trans].empty())
                    precedence.emplace_back(trans);
                transitions[trans].emplace_back(trans->target);
            }
        }
    }
    return isAccepted;
}

Automaton::Ptr subset(Automaton::Ptr nfa, bool (*epsilonChecker)(Transition::Ptr)) {
    Automaton::Ptr dfa(new Automaton);
    std::map<State::List, State::Ptr> dict;
    std::queue<State::List> statesQ;
    std::queue<Transition::Map<State::List>> transitionsQ;
    std::queue<Transition::List> precedenceQ;
    State::List epsilonStates;
    State::Set epsilonSet;
    Transition::Map<State::List> transitions;
    Transition::List precedence;
    bool isAccepted = false;

    dfa->startState = dfa->getState();
    isAccepted = epsilonClosure(nfa->startState, epsilonChecker, epsilonStates, epsilonSet, transitions, precedence);
    dfa->startState->isAccepted = isAccepted;
    statesQ.push(epsilonStates);
    transitionsQ.push(transitions);
    precedenceQ.push(precedence);
    dict.emplace(epsilonStates, dfa->startState);

    while (!statesQ.empty()) {
        State::List curStates = statesQ.front();
        statesQ.pop();
        Transition::Map<State::List> curTransitions = transitionsQ.front();
        transitionsQ.pop();
        Transition::List curPrecedence = precedenceQ.front();
        precedenceQ.pop();
        for (auto t : curPrecedence) {
            epsilonStates.clear();
            transitions.clear();
            precedence.clear();
            epsilonSet.clear();
            isAccepted = false;
            for (auto s : curTransitions[t])
                isAccepted |= epsilonClosure(s, epsilonChecker, epsilonStates, epsilonSet, transitions, precedence);
            if (dict.find(epsilonStates) == dict.end()) {
                State::Ptr dfaState = dfa->getState();
                dfaState->isAccepted = isAccepted;
                dict.emplace(epsilonStates, dfaState);
                statesQ.push(epsilonStates);
                transitionsQ.push(transitions);
                precedenceQ.push(precedence);
            }
            Transition::Ptr transition = dfa->getTransition(dict[curStates], dict[epsilonStates]);
            transition->type = t->type;
            transition->range = t->range;
        }
    }
    return dfa;
}

std::vector<State::Set> split(State::Set states, const std::set<State::Set> &partition) {
    std::vector<State::Set> ret(1);
    Transition::Map<std::set<State::Set>::iterator> dict;
    auto i = states.begin(), iend = states.end();
    for (auto transition : (*i)->outbounds) {
        for (auto j = partition.cbegin(), jend = partition.cend(); j != jend; ++j) {
            if (j->find(transition->target) != j->end()) {
                dict.emplace(transition, j);
                break;
            }
        }
    }
    ret[0].emplace(*i);
    for (++i; i != iend; ++i) {
        int index = 0;
        for (auto transition : (*i)->outbounds) {
            if (dict.find(transition) == dict.end() || dict[transition]->find(transition->target) == dict[transition]->end()) {
                index = 1;
                break;
            }
        }
        if (index == ret.size())
            ret.resize(2);
        ret[index].emplace(*i);
    }
    return ret;
}

Automaton::Ptr Hopcroft(Automaton::Ptr dfa, bool (*epsilonChecker)(Transition::Ptr)) {
    State::Set acceptedStates, nonacceptedStates;
    for (auto state : dfa->states) {
        if (state->isAccepted)
            acceptedStates.emplace(state);
        else
            nonacceptedStates.emplace(state);
    }
    std::set<State::Set> partitions, saved;
    partitions.emplace(acceptedStates);
    partitions.emplace(nonacceptedStates);
    while (partitions != saved) {
        std::swap(partitions, saved);
        partitions.clear();
        for (auto states : saved) {
            if (states.size() < 2)
                partitions.emplace(states);
            else {
                auto sets = split(states, saved);
                partitions.insert(sets.begin(), sets.end());
            }
        }
    }

    Automaton::Ptr mdfa = Automaton::Ptr(new Automaton);
    std::map<State::Ptr, State::Ptr> stateMap;
    for (auto &states : partitions) {
        auto mdfaState = mdfa->getState();
        for (auto &dfaState : states) {
            if (dfaState->isAccepted)
                mdfaState->isAccepted = true;
            if (dfaState == dfa->startState)
                mdfa->startState = mdfaState;
            stateMap.emplace(dfaState, mdfaState);
        }
    }
    for (auto &states : partitions) {
        auto dfaState = *states.begin();
        for (auto transition : dfaState->outbounds) {
            auto nTransit = mdfa->getTransition(stateMap[transition->source], stateMap[transition->target]);
            nTransit->type = transition->type;
            nTransit->range = transition->range;
        }
    }
    return mdfa;
}

Automaton::Ptr Brzozowski(Automaton::Ptr nfa, bool (*epsilonChecker)(Transition::Ptr)) {
    //nfa->toMermaid(std::cout) << std::endl;
    nfa->reverse();
    //nfa->toMermaid(std::cout) << std::endl;
    auto tdfa = subset(nfa, epsilonChecker);
    //tdfa->toMermaid(std::cout) << std::endl;
    tdfa->reachableTrim();
    //tdfa->toMermaid(std::cout) << std::endl;
    tdfa->reverse();
    //tdfa->toMermaid(std::cout) << std::endl;
    auto mdfa = subset(tdfa, epsilonChecker);
    mdfa->toMermaid(std::cout) << std::endl;
    //mdfa->reachableTrim();
    mdfa->toMermaid(std::cout) << std::endl;
    return mdfa;
}

void print(Automaton::Ptr automaton) {
    for (auto state : automaton->states) {
        if (state == automaton->startState)
            std::cout << "*";
        std::cout << state << ": ";
        for (auto transition : state->outbounds) {
            std::cout << transition->target << "(";
            if (transition->type == Transition::Chars) {
                std::cout << transition->range.begin << "-" << transition->range.end;
            } else if (transition->type == Transition::Nop) {
                std::cout << "Nop";
            } else
                std::cout << "Epsilon";
            std::cout << ") ";
        }
        if (state->isAccepted)
            std::cout << "$";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
