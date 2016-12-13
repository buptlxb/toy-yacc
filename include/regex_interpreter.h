#ifndef REGEX_INTERPRETER_H
#define REGEX_INTERPRETER_H

#include <vector>
#include "automaton.h"

class PoorInterpreter {
public:
    using Ptr = std::shared_ptr<PoorInterpreter>;
    struct Result {
        int32_t start;
        int32_t length;
        int32_t terminateState;
        int32_t acceptedState;
    };
protected:
    std::vector<int16_t> charMap;
    std::vector<std::vector<int32_t>> transitionTable;
    std::vector<bool> acceptedStates;
    int32_t stateCount;
    int32_t charCategories;
    int32_t startState;
public:
    static constexpr int CharMapSize = 256;
    static constexpr int InvalidState = -1;
    PoorInterpreter(Automaton::Ptr dfa);
    bool match(const char *input);
    bool search(const char *input, Result *result=nullptr, uint32_t offset=0);
    bool searchHead(const char *input, Result *result=nullptr, uint32_t offset=0); 
};

class RichInterpreter {
public:
    using Ptr = std::shared_ptr<RichInterpreter>;
    struct Result {
        int32_t start;
        int32_t length;
        State::Ptr terminateState;
        State::Ptr acceptedState;
    };
    struct StatusSaver {
        State::Ptr state;
        const char *reading;
        Transition::List::iterator transition;
    };
protected:
    Automaton::Ptr dfa;
    std::unordered_map<State::Ptr, bool> stateMap;
public:
    RichInterpreter(Automaton::Ptr dfa);
    bool match(const char *input);
    bool search(const char *input, Result *result=nullptr, uint32_t offset=0);
    bool searchHead(const char *input, Result *result=nullptr, uint32_t offset=0); 
};

#endif
