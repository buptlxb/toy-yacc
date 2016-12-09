#include <algorithm>
#include <unordered_map>
#include <cstring>
#include <stack>
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
    const char *reading = input;
    while (currentState != InvalidState) {
        if (acceptedStates[currentState]) {
            acceptedState = currentState;
            length = reading - input;
        }
        if (!*reading || *reading >= CharMapSize)
            break;
        int16_t charCat = charMap[*reading++];
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

RichInterpreter::RichInterpreter(Automaton::Ptr _dfa) : dfa(_dfa) {
    for (auto state : dfa->states) {
        int32_t charEdge = 0, nonCharEdge = 0;
        for (auto transition : state->outbounds) {
            switch (transition->type) {
                case Transition::Chars:
                    ++charEdge;
                    break;
                default:
                    ++nonCharEdge;
            }
        }
        stateMap[state] = nonCharEdge > 1 || nonCharEdge && charEdge;
    }
}

bool RichInterpreter::search(const char *input, Result *result, uint32_t offset) {
    if (offset >= strlen(input))
        return false;
    while (input[offset]) {
        if (match(input, result, offset++))
            return true;
    }
}

bool RichInterpreter::match(const char *input, Result *result, uint32_t offset) {
    if (offset >= strlen(input))
        return false;
    input += offset;
    const char *read = input;
    std::stack<StatusSaver> statusStack;
    StatusSaver currentStatus;
    currentStatus.state = dfa->startState;
    currentStatus.reading = input;
    currentStatus.transition = dfa->startState->outbounds.begin();
    while (!currentStatus.state->isAccepted) {
        StatusSaver saved = currentStatus;
        bool found = false;
        for (auto transition = currentStatus.transition; transition != currentStatus.state->outbounds.end(); ++transition) {
            switch ((*transition)->type) {
                case Transition::Chars:
                    if ((*transition)->range.contains(*currentStatus.reading)) {
                        found = true;
                        ++(currentStatus.reading);
                    }
                    break;
                case Transition::Nop:
                    found = true;
                    break;
                case Transition::BeginString:
                    found = !offset && currentStatus.reading == input;
                    break;
                case Transition::EndString:
                    found = *currentStatus.reading == '\0';
                    break;
                default:
                    assertm(0, "Unkown transition type");
            }
            if (found) {
                if (stateMap[currentStatus.state]) {
                    saved.transition = transition;
                    ++(saved.transition);
                    statusStack.push(saved);
                }
                currentStatus.state = (*transition)->target;
                currentStatus.transition = currentStatus.state->outbounds.begin();
                break;
            }
        }
        if (!found) {
            if (!statusStack.empty()) {
                currentStatus = statusStack.top();
                statusStack.pop();
            } else
                break;
        }
    }
    if (result) {
        result->start = offset;
        result->length = currentStatus.reading - input;
        result->terminateState = currentStatus.state;
        result->acceptedState = currentStatus.state;
    }
    return currentStatus.state->isAccepted;
}
