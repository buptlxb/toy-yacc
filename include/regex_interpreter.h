#ifndef REGEX_INTERPRETER_H
#define REGEX_INTERPRETER_H

#include <vector>
#include "automaton.h"

class PoorInterpreter {
public:
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
    bool match(const char *input, Result *result=nullptr, uint32_t offset=0); 
    bool search(const char *input, Result *result=nullptr, uint32_t offset=0);
};

class RichInterpreter {
protected:
    Automaton::Ptr dfa;
public:
};

#endif
