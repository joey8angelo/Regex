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
    /*
        A more space efficient way of making character classes
        instead of translating a character class [a-z] -> (a|b|c|d|e|f..)
        the CharacterClass uses a set to easily lookup characters that could be part of the group
        also makes negating a group much simpler by !ing the result of looking up a character
    */
    struct CharacterClass{
        CharacterClass(std::string);
        ~CharacterClass();
        std::unordered_set<char> characters;
        bool negated;
        bool operator==(char);
    };
    /*
        Abstract NFAState, according to thompson's construction no state will 
        have more than 2 outgoing edges, and if the edge is not epsilon 
        it will only ever have 1 outgoing edge the edge is a pointer to 
        other NFA states where the character to transition on is defined
        in respective derived classes
        Regex can check if a given character transitions on this state
        with the hasChar method
    */
    class NFAState{
        public:
        NFAState(int i): ID(i), out1(nullptr), out2(nullptr){}
        ~NFAState();
        int ID;
        Regex::NFAState* out1;
        Regex::NFAState* out2;
        virtual bool isEpsilon() = 0;
        virtual bool hasChar(char ch) = 0;
        virtual NFAState* makeCopy() = 0;
    };

    /*
        Derived NFAState where the transition is a character
    */
    class NFAStateChar: public NFAState{
        public:
        NFAStateChar(int i, char ch): NFAState(i), c(ch){}
        char c;
        virtual bool isEpsilon();
        virtual bool hasChar(char);
        virtual NFAState* makeCopy();
    };

    /*
        Derived NFAState where the transition is a CharacterClass
    */
    class NFAStateCharClass: public NFAState{
        public:
        NFAStateCharClass(int i, std::string s): NFAState(i), cc(Regex::CharacterClass(s)){}
        NFAStateCharClass(int i, Regex::CharacterClass chc): NFAState(i), cc(chc){}
        Regex::CharacterClass cc;
        virtual bool isEpsilon();
        virtual bool hasChar(char);
        virtual NFAState* makeCopy();
    };

    /*
        Derived NFAState where the transition is epsilon
    */
    class NFAStateEpsilon: public NFAState{
        public:
        NFAStateEpsilon(int i): NFAState(i){}
        virtual bool isEpsilon();
        virtual bool hasChar(char);
        virtual NFAState* makeCopy();
    };

    /*
        A DFAState, these states are generated as needed when matching a string against the regular expression
        a state is a list of NFAState ids, with transitions on characters to other DFAStates
    */
    struct DFAState{
        DFAState(std::set<int>*);
        DFAState(std::set<int>*, bool);
        ~DFAState();
        std::unordered_map<char, Regex::DFAState*> out;
        std::set<int>* ls;
        bool accept;
    };
    /*
        Hash for std::set<int>
    */
    struct setHash{
        std::size_t operator()(const std::set<int>* s) const{
            std::size_t seed = s->size();
            for(auto& i : *s) {
                seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
    const int CACHELIMIT;
    std::string reg;
    bool matchStart;
    bool matchEnd;
    bool reversed;
    int nfaStart;
    int nfaAcc;
    int id;
    // lookup nfa state by its id
    std::unordered_map<int, Regex::NFAState*> nfa;
    // lookup dfa state by its set of nfa states
    std::unordered_map<std::set<int>*, Regex::DFAState*, Regex::setHash> dfa;
    Regex::DFAState* reject;
    Regex::DFAState* dfaStart;
    void parse();
    void deleteNFA();
    void deleteDFA(Regex::DFAState* st = nullptr);
    void epsilonClosure(int, std::set<int>*) const;
    void buildDFAStart();
    std::pair<Regex::NFAState*, std::vector<int>> parse(int&);
    std::pair<Regex::NFAState*, std::vector<int>> parseGroup(int&);
    std::pair<Regex::NFAState*, std::vector<int>> parseCharClass(int&);
    std::pair<Regex::NFAState*, std::vector<int>> parseSpecial(int&);
    std::pair<Regex::NFAState*, std::vector<int>> parseChar(int&);
    std::pair<Regex::NFAState*, std::vector<int>> parseInterval(Regex::NFAState*, std::vector<int>, int&);
    std::pair<Regex::NFAState*, std::vector<int>> doStar(Regex::NFAState*, std::vector<int>);
    std::pair<Regex::NFAState*, std::vector<int>> doQuestion(Regex::NFAState*, std::vector<int>);
    std::pair<Regex::NFAState*, std::vector<int>> doPlus(Regex::NFAState*, std::vector<int>);
    std::pair<Regex::NFAState*, std::vector<int>> doPipe(Regex::NFAState*, std::vector<int>, int&);
    std::pair<Regex::NFAState*, std::vector<int>> copy(std::unordered_set<int>&, int, std::vector<int>);
    std::unordered_set<int> makeList(Regex::NFAState*, const std::vector<int>&) const;
    Regex::DFAState* nextDFA(char, Regex::DFAState*);
    Regex::NFAState* makeEpsilonState();
    Regex::NFAState* makeCharState(char);
    Regex::NFAState* makeCharClassState(std::string);
};