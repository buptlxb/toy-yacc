#include <iostream>
#include <fstream>
#include "automaton.h"
#include "regex_expression.h"

int main(int argc, char *argv[])
{
    // const char *input = "(([0-9]+)(\\.[0-9]+)(e(\\+|-)?([0-9]+))?|([0-9]+)e(\\+|-)?([0-9]+))([lL]|[fF])?";
    // const char *input = "(L)?'([^\\\\\\n]|(\\\\.))*?'";
    // const char *input = "[acb-xd-fz]";
    // const char *input = "[aaa]";
    for (int i = 1; i < argc; ++i) {
        auto regex = parseRegex(argv[i]);
        std::ofstream ofs("r.dot");
        regex->graphviz(ofs);
        ofs.close();
        regex->setNormalize();
        auto automaton = regex->generateEpsilonNfa();
        automaton->toMermaid(std::cout) << std::endl;;

        // auto dfa = epsilonNfaToDfa(automaton, poorEpsilonChecker);
        // dfa->toMermaid(std::cout) << std::endl;

        // std::cout << "reverse1: " << automaton->states.size() << " " << automaton->transitions.size() << std::endl;
        // automaton->reverse();
        // automaton->toMermaid(std::cout) << std::endl;

        // std::cout << "subset1: " << automaton->states.size() << " " << automaton->transitions.size()  << std::endl;
        // auto tdfa = subset(automaton, poorEpsilonChecker);
        // tdfa->toMermaid(std::cout) << std::endl;
        // 
        // std::cout << "reachable1: " << tdfa->states.size() << " " << tdfa->transitions.size()  << std::endl;
        // tdfa->reachableTrim();
        // tdfa->toMermaid(std::cout) << std::endl;

        // std::cout << "reverse2: " << tdfa->states.size() << " " << tdfa->transitions.size()  << std::endl;
        // tdfa->reverse();
        // tdfa->toMermaid(std::cout) << std::endl;

        // std::cout << "subset2: " << tdfa->states.size() << " " << tdfa->transitions.size()  << std::endl;
        // auto mdfa = subset(tdfa, poorEpsilonChecker);
        // mdfa->toMermaid(std::cout) << std::endl;

        // std::cout << "reachable2: " << mdfa->states.size() << " " << mdfa->transitions.size()  << std::endl;
        // mdfa->reachableTrim();
        // mdfa->toMermaid(std::cout) << std::endl;
        Brzozowski(automaton, poorEpsilonChecker)->toMermaid(std::cout) << std::endl;
    }
    return 0;
}
