#pragma once
#include "includes.h"

class Regex{
    public:
    Regex(std::string);
    ~Regex();
    std::pair<int, std::string> find(const std::string&);
    bool test(const std::string&);
    std::vector<std::pair<int, std::string>> group(const std::string&);
    private:
    const int CACHELIMIT;
    void printNFAStates();
    std::string reg;
    bool matchStart;
    bool matchEnd;
    bool reversed;

    /*
        A more space efficient way of making character classes
        instead of translating a character class [a-z] -> (a|b|c|d|e|f..)
        the CharacterClass uses a set to easily lookup characters that could be part of the group
        also makes negating a group much simpler by !ing the result of looking up a character
    */
    struct CharacterClass{
        CharacterClass(std::string, bool);
        ~CharacterClass();
        std::unordered_set<char> characters;
        bool negated;
        bool epsilon;
        bool operator==(char);
        std::string stringify();
    };
    /*
        A NFA state, according to thompson's construction no state will have more than 2 outgoing edges
        and if the edge is not epsilon it will only ever have 1 outgoing edge
        the edge is a pointer to other NFA states where the character to transition on is a CharacterClass
        epsilon is denoted with a boolean value in CharacterClass
    */
    struct NFAState{
        NFAState(int, std::unordered_map<int, Regex::NFAState*>*);
        NFAState(char, int, std::unordered_map<int, Regex::NFAState*>* nfa);
        NFAState(std::string, int, std::unordered_map<int, Regex::NFAState*>* nfa);
        ~NFAState();
        int ID;
        Regex::CharacterClass c;
        Regex::NFAState* out1;
        Regex::NFAState* out2;
    };
    struct DFAState{
        DFAState(std::set<int>);
        DFAState(std::set<int>, bool);
        ~DFAState();
        Regex::DFAState* next(char);
        std::unordered_map<char, Regex::DFAState*> out;
        std::set<int> ls;
        bool accept;
    };
    struct setHash{
        std::size_t operator()(const std::set<int>& s) const{
            std::size_t seed = s.size();
            for(auto& i : s) {
                seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
    int nfaStart;
    int nfaAcc;
    std::unordered_map<int, Regex::NFAState*> nfa;
    std::unordered_map<std::set<int>, Regex::DFAState*, Regex::setHash> dfa;
    Regex::DFAState* reject;
    Regex::DFAState* dfaStart;
    void parse();
    std::vector<Regex::NFAState*> parse(int&);
    std::vector<Regex::NFAState*> parseGroup(int&);
    std::vector<Regex::NFAState*> parseCharClass(int&);
    std::vector<Regex::NFAState*> parseSpecial(int&);
    std::vector<Regex::NFAState*> parseChar(int&);
    std::vector<Regex::NFAState*> parseInterval(Regex::NFAState*, Regex::NFAState*, int&);
    std::vector<Regex::NFAState*> doStar(Regex::NFAState*, Regex::NFAState*);
    std::vector<Regex::NFAState*> doQuestion(Regex::NFAState*, Regex::NFAState*);
    std::vector<Regex::NFAState*> doPlus(Regex::NFAState*, Regex::NFAState*);
    std::vector<Regex::NFAState*> doPipe(Regex::NFAState*, Regex::NFAState*, int&);
    void deleteNFA();
    void deleteDFA();
    std::unordered_set<int> epsilonClosure(int) const;
    std::unordered_set<int> makeList(Regex::NFAState*, Regex::NFAState*);
    std::vector<Regex::NFAState*> copy(std::unordered_set<int>&, int, int);
};