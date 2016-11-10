#ifndef FINITE_AUTOMATON_H
#define FINITE_AUTOMATON_H

#include <vector>
#include <map>
#include <queue>
#include <iosfwd>

constexpr char EPSILON = '\xFF';

struct FANode {
    std::vector<FANode *> prev;
    std::multimap<char, FANode *> next;
    bool isAccepted;
    FANode () : isAccepted(false) {}
};

class NFA {
    FANode *s0, *sa;
public:
    NFA(char c=EPSILON) : s0(new FANode), sa(new FANode) {
        s0->next.emplace(c, sa);
        sa->prev.push_back(s0);
        sa->isAccepted = true;
    }
    NFA(const NFA *that) {
        std::map<FANode *, FANode *> dict;
        s0 = new FANode;
        dict[that->s0] = s0;
        sa = new FANode;
        dict[that->sa] = sa;
        sa->isAccepted = true;
        copy(that->s0, dict);
    }
    NFA operator+ (const NFA &) const;
    NFA operator| (const NFA &) const;
    NFA & operator* ();
    NFA & operator+= (const NFA &);
    NFA & operator|= (const NFA &);
private:
    static void copy(FANode *s0, std::map<FANode *, FANode *> &dict) {
        std::queue<FANode *> q;
        q.push(s0);
        while (!q.empty()) {
            auto cur = q.front();
            q.pop();
            for (auto &n : cur->next) {
                if (dict.find(n.second) == dict.end()) {
                    dict.emplace(n.second, new FANode);
                    q.push(n.second);
                }
                dict[cur]->next.emplace(n.first, dict[n.second]);
                dict[n.second]->prev.push_back(dict[cur]);
            }
        }
    }

    friend std::ostream & operator<< (std::ostream &os, const NFA &nfa);
};

#endif
