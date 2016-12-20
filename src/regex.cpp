#include <iostream>
#include <fstream>
#include <iomanip>
#include "automaton.h"
#include "regex_expression.h"
#include "regex_interpreter.h"

int main(int argc, char *argv[])
{
    // const char *input = "(([0-9]+)(\\.[0-9]+)(e(\\+|-)?([0-9]+))?|([0-9]+)e(\\+|-)?([0-9]+))([lL]|[fF])?";
    // const char *input = "(L)?'([^\\\\\\n]|(\\\\.))*?'";
    // const char *input = "[acb-xd-fz]";
    // const char *input = "[aaa]";
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " pattern [match] ..." << std::endl;
        return -1;
    }
    auto regex = parseRegex(argv[1]);
    std::ofstream ofs("r.dot");
    regex->graphviz(ofs);
    ofs.close();
    Range<unsigned char>::List unifiedRanges;
    regex->setNormalize(&unifiedRanges);
    regex->setUnify(unifiedRanges);
    auto automaton = regex->generateEpsilonNfa();
    automaton->toMermaid(std::cout) << std::endl;
    std::map<State::List, State::Ptr> nfaStateMap;
    auto dfa = powerset(automaton, richEpsilonChecker, nfaStateMap);
    dfa->toMermaid(std::cout) << std::endl;
    std::map<State::Ptr, State::Ptr> dfaStateMap;
    dfa = Hopcroft(dfa, dfaStateMap);
    dfa->toMermaid(std::cout) << std::endl;
    RichInterpreter *iterpreter = new RichInterpreter(dfa);
    std::cout << "Case #\tMatch\tStart\tLength\tAccept\tTerminate" << std::endl;
    for (int i = 2; i < argc; ++i) {
        RichInterpreter::Result result;
        bool match = iterpreter->search(argv[i], &result);
        std::cout << i-1 << "\t" << std::boolalpha << match << "\t" << result.start << "\t" << result.length << "\t" << result.terminateState << "\t" << result.acceptedState << std::endl;
    }
    return 0;
}
