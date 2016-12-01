#include <queue>
#include <map>
#include <ostream>
#include "automaton.h"
#include "utility.h"

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
    std::queue<State::Ptr> q;
    std::map<State::Ptr, unsigned> dict;
    unsigned index = 0;
    q.push(this->startState);
    dict.emplace(this->startState, index++);
    while (!q.empty()) {
            auto cur = q.front();
            q.pop();
            for (auto trans : cur->outbounds) {
                if (dict.find(trans->target) == dict.end()) {
                    dict.emplace(trans->target, index++);
                    q.push(trans->target);
                }
                os << "s" << dict[cur] << "--";
                switch (trans->type) {
                    case Transition::Chars:
                        if (trans->range.begin == trans->range.end)
                            os << repr(trans->range.begin);
                        else
                            os << "\"[" << repr(trans->range.begin) << "-" << repr(trans->range.end) << "]\"";
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
                os << "-->" << "s" << dict[trans->target];
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

void epsilonClosure(typename State::Ptr dfaState, typename State::Ptr nfaState, bool (*epsilonChecker)(Transition::Ptr), State::Set &epsilonStates, Transition::List &transitions) {
    if (epsilonStates.find(nfaState) == epsilonStates.end()) {
        epsilonStates.emplace(nfaState);
        for (auto trans : nfaState->outbounds) {
            if (epsilonChecker(trans)) {
                if (trans->target->isAccepted)
                    dfaState->isAccepted = true;
                epsilonClosure(dfaState, trans->target, epsilonChecker, epsilonStates, transitions);
            } else
                transitions.emplace_back(trans);
        }
    }
}

bool epsilonClosure(typename State::Ptr nfaState, bool (*epsilonChecker)(Transition::Ptr), State::Set &epsilonStates, std::unordered_map<Transition::Ptr, State::Set> &transitions) {
    bool isAccepted = nfaState->isAccepted;
    if (epsilonStates.find(nfaState) == epsilonStates.end()) {
        epsilonStates.emplace(nfaState);
        for (auto trans : nfaState->outbounds) {
            if (epsilonChecker(trans)) {
                if (trans->target->isAccepted)
                    isAccepted = true;
                isAccepted |= epsilonClosure(trans->target, epsilonChecker, epsilonStates, transitions);
            } else {
                transitions[trans].emplace(trans->target);
            }
        }
    }
    return isAccepted;
}

Automaton::Ptr subset(Automaton::Ptr nfa, bool (*epsilonChecker)(Transition::Ptr)) {
    Automaton::Ptr dfa(new Automaton);
    std::map<State::Set, State::Ptr> dict;
    std::queue<State::Set> stateSetsQ;
    std::queue<std::unordered_map<Transition::Ptr, State::Set>> transitionsQ;
    State::Set epsilonStates;
    std::unordered_map<Transition::Ptr, State::Set> transitions;
    bool isAccepted = false;

    dfa->startState = dfa->getState();
    isAccepted = epsilonClosure(nfa->startState, epsilonChecker, epsilonStates, transitions);
    dfa->startState->isAccepted = isAccepted;
    stateSetsQ.push(epsilonStates);
    transitionsQ.push(transitions);
    dict.emplace(epsilonStates, dfa->startState);

    while (!stateSetsQ.empty()) {
        State::Set curStates = stateSetsQ.front();
        stateSetsQ.pop();
        std::unordered_map<Transition::Ptr, State::Set> curTransitions = transitionsQ.front();
        transitionsQ.pop();
        for (auto &t : curTransitions) {
            epsilonStates.clear();
            transitions.clear();
            isAccepted = false;
            for (auto s : t.second)
                isAccepted |= epsilonClosure(s, epsilonChecker, epsilonStates, transitions);
            if (dict.find(epsilonStates) == dict.end()) {
                State::Ptr dfaState = dfa->getState();
                dfaState->isAccepted = isAccepted;
                dict.emplace(epsilonStates, dfaState);
                stateSetsQ.push(epsilonStates);
                transitionsQ.push(transitions);
            }
            Transition::Ptr transition = dfa->getTransition(dict[curStates], dict[epsilonStates]);
            transition->type = t.first->type;
            transition->range = t.first->range;
        }
    }
    return dfa;
}

Automaton::Ptr epsilonNfaToDfa(Automaton::Ptr nfa, bool (*epsilonChecker)(Transition::Ptr)) {
    Automaton::Ptr dfa(new Automaton);
    std::map<State::Ptr, State::Ptr> nfaStateMap;
    std::map<State::Ptr, State::Ptr> dfaStateMap;
    State::Set epsilonStates;
    Transition::List transitions;

    dfa->startState = dfa->getState();
    nfaStateMap.emplace(nfa->startState, dfa->startState);
    dfaStateMap.emplace(dfa->startState, nfa->startState);

    for (auto iter = dfa->states.begin(), iend = dfa->states.end(); iter != iend; ++iter) {
        State::Ptr dfaState = *iter, nfaState = dfaStateMap[dfaState];
        if (nfaState->isAccepted)
            dfaState->isAccepted = true;

        epsilonStates.clear();
        transitions.clear();
        epsilonClosure(dfaState, nfaState, epsilonChecker, epsilonStates, transitions);

        for (auto trans : transitions) {
            if (nfaStateMap.find(trans->target) == nfaStateMap.end()) {
                auto t = dfa->getState();
                nfaStateMap.emplace(trans->target, t);
                dfaStateMap.emplace(t, trans->target);
            }
            auto ntrans = dfa->getTransition(dfaState, nfaStateMap[trans->target]);
            ntrans->type = trans->type;
            ntrans->range = trans->range;
        }
    }
    return dfa;
}
