#include <algorithm>
#include <unordered_map>
#include <cstring>
#include "regex_interpreter.h"
#include "utility.h"

constexpr int PoorInterpreter::CharMapSize;
constexpr int PoorInterpreter::InvalidState;

PoorInterpreter::PoorInterpreter(Automaton::Ptr dfa) : charMap(CharMapSize, CharMapSize) {
    Range<unsigned char>::List ranges;
    for (auto transition : dfa->transitions) {
        auto range = transition->range;
        for (auto i = ranges.begin(), iend = ranges.end(); i != iend; ++i) {
            if (range.end < i->begin) {
                ranges.emplace(i, range);
                return;
            } else if (i->end < range.begin)
                ;
            else if (i->begin < range.begin) {
                auto end = i->end;
                i->end = range.begin - 1;
                i = ranges.emplace(i, range.begin, end);
            } else if (range.begin < i->begin) {
                auto begin = i->begin;
                i = ranges.emplace(i, range.begin, i->begin-1);
                range.begin = begin;
            } else if (i->end < range.end) {
                range.begin = i->end + 1;
            } else
                return;
        }
        ranges.push_back(range);
    }
    acceptedStates.resize(stateCount);
    std::unordered_map<State::Ptr, int32_t> stateMap;
    int32_t index = 0;
    for (auto state : dfa->states) {
        acceptedStates[index] = state->isAccepted;
        stateMap.emplace(state, index++);
    }
    charCategories = ranges.size() + 1;
    stateCount = dfa->states.size();
    startState = stateMap[dfa->startState];
    auto iter = ranges.begin();
    for (size_t i = 0, iend = ranges.size(); i != iend; ++i, ++iter) {
        for (unsigned char j = iter->begin, jend = iter->end; j <= jend; ++j) {
            charMap[j] = i;
        }
    }
    transitionTable.resize(stateCount, std::vector<int32_t>(charCategories, InvalidState));
    auto stateIter = dfa->states.begin();
    for (size_t i = 0; i < stateCount; ++i, ++stateIter) {
        for (auto transition : (*stateIter)->outbounds) {
            switch (transition->type) {
                case Transition::Chars:
                    iter = ranges.begin();
                    for (size_t j = 0, iend = ranges.size(); j != iend; ++j, ++iter) {
                        if (transition->range.begin <= iter->begin && transition->range.end >= iter->end)
                            transitionTable[i][j] = stateMap[transition->target];
                    }
                    break;
                default:
                    assertm(0, "Poor Interpreter should not have non-chars transition");
            }
        }
    }
}


bool PoorInterpreter::search(const char *input, Result *result, uint32_t offset) {
    if (offset >= strlen(input))
        return false;
    while (input[offset]) {
        if (match(input, result, offset++))
            return true;
    }
}

bool PoorInterpreter::match(const char *input, Result *result, uint32_t offset) {
    if (offset >= strlen(input))
        return false;
    input += offset;
    int32_t currentState = startState;
    int32_t acceptedState = InvalidState;
    int32_t length = -1;
    const char *read = input;
    while (currentState != InvalidState) {
        if (acceptedStates[currentState]) {
            acceptedState = currentState;
            length = read - input;
        }
        if (!*read || *read >= CharMapSize)
            break;
        int16_t charCat = charMap[*read++];
        currentState = transitionTable[currentState][charCat];
    }
    if (result) {
        result->start = offset;
        result->length = length;
        result->terminateState = currentState;
        result->acceptedState = acceptedState;
    }
    return acceptedState != InvalidState;
}
