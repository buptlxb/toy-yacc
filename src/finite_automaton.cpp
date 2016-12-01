#include <stack>
#include <queue>
#include <map>
#include <ostream>
#include <sstream>
#include "finite_automaton.h"
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
        sas.insert(this->s0);
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
                    sas.insert(tmp);
                dict.emplace(t.second, tmp);
                q.push(t.second);
            }
            dict[cur]->next.emplace(t.first, dict[t.second]);
        }
    }
}

std::vector<std::set<FANode *>> DFA::split(std::set<FANode *> subset, const std::set<std::set<FANode *>> &partition) {
    std::vector<std::set<FANode *>> ret(1);
    std::map<char, std::set<std::set<FANode *>>::iterator> dict;
    auto i = subset.begin(), iend = subset.end();
    for (auto p : (*i)->next) {
        for (auto j = partition.cbegin(), jend = partition.cend(); j != jend; ++j) {
            if (j->find(p.second) != j->end()) {
                dict[p.first] = j;
                break;
            }
        }
    }
    ret[0].insert(*i);
    for (advance(i, 1); i != iend; ++i) {
        int index = 0;
        for (auto p : (*i)->next) {
            if (dict.find(p.first) == dict.end() || dict[p.first]->find(p.second) == dict[p.first]->end()) {
                index = 1;
                break;
            }
        }
        if (index == ret.size())
            ret.resize(2);
        ret[index].insert(*i);
    }
    return ret;
}


void DFA::minimize() {
    std::set<FANode *> ss, visited;
    std::queue<FANode *> q;
    q.push(this->s0);
    visited.insert(this->s0);
    while (!q.empty()) {
        FANode *cur = q.front();
        q.pop();
        if (!cur->isAccepted)
            ss.insert(cur);
        for (auto p : cur->next) {
            if (visited.insert(p.second).second)
                q.push(p.second);
        }
    }

    std::set<std::set<FANode *>> target, partition;
    target.insert(ss);
    target.insert(this->sas);
    while (target != partition) {
        partition = target;
        target.clear();
        for (auto subset : partition) {
            if (subset.size() < 2)
                target.insert(subset);
            else {
                auto sets = split(subset, partition);
                target.insert(sets.begin(), sets.end());
            }
        }
    }

    DFA old(std::move(*this));

    typedef std::set<std::set<FANode *>>::iterator Iter;
    typedef bool (*IterCmp) (Iter, Iter);
    std::map<std::set<std::set<FANode *>>::iterator, FANode *, IterCmp> mapping([](Iter lhs, Iter rhs) -> bool { return *lhs < *rhs; });
    std::map<FANode *, std::set<std::set<FANode *>>::iterator> represent;
    for (auto i = target.begin(), iend = target.end(); i != iend; ++i)
        for (auto j = i->begin(), jend = i->end(); j != jend; ++j)
            represent.emplace(*j, i);
    for (auto i = target.begin(), iend = target.end(); i != iend; ++i) {
        if (mapping.find(i) == mapping.end())
            mapping.emplace(i, new FANode);
        if (i->find(old.s0) != i->end())
            this->s0 = mapping[i];
        for (auto j = i->begin(), jend = i->end(); j != jend; ++j) {
            for (auto p : (*j)->next) {
                auto repr = represent[p.second];
                if (mapping.find(repr) == mapping.end())
                    mapping.emplace(repr, new FANode);
                if (mapping[i]->next.find(p.first) == mapping[i]->next.end())
                    mapping[i]->next.emplace(p.first, mapping[repr]);
            }
        }
    }
    for (auto n : old.sas) {
        auto t = mapping[represent[n]];
        t->isAccepted = true;
        this->sas.insert(t);
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
