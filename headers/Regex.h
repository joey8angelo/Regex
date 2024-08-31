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
        it will only ever have 1 outgoing edge, the edge is a pointer to 
        other NFA states where the character to transition on is defined
        in respective derived classes
        Regex can check if a given character transitions on this state
        with the hasChar method
    */
    class NFAState{
        public:
        NFAState(): out1(nullptr), out2(nullptr){}
        virtual ~NFAState() = default;
        NFAState* out1;
        NFAState* out2;
        virtual bool isEpsilon() = 0;
        virtual bool hasChar(char ch) = 0;
        virtual NFAState* makeCopy() = 0;
    };

    /*
        Derived NFAState where the transition is a character
    */
    class NFAStateChar: public Regex::NFAState{
        public:
        NFAStateChar(char);
        char c;
        virtual bool isEpsilon();
        virtual bool hasChar(char);
        virtual NFAState* makeCopy();
    };

    /*
        Derived NFAState where the transition is a CharacterClass
    */
    class NFAStateCharClass: public Regex::NFAState{
        public:
        NFAStateCharClass(std::string);
        NFAStateCharClass(CharacterClass);
        CharacterClass cc;
        virtual bool isEpsilon();
        virtual bool hasChar(char);
        virtual NFAState* makeCopy();
    };

    /*
        Derived NFAState where the transition is epsilon
    */
    class NFAStateEpsilon: public Regex::NFAState{
        public:
        NFAStateEpsilon();
        virtual bool isEpsilon();
        virtual bool hasChar(char);
        virtual NFAState* makeCopy();
        virtual std::string toString(){ return "epsilon"; }
    };

    /*
        A DFAState, these states are generated as needed when matching a string against the regular expression
        a state is a list of NFAStates, with transitions on characters to other DFAStates
    */
    struct DFAState{
        DFAState(std::set<Regex::NFAState*>);
        DFAState(std::set<Regex::NFAState*>, bool);
        std::unordered_map<char, DFAState*> out;
        bool accept;
        std::set<Regex::NFAState*> ls;
    };
    /*
        Hash for std::set<Regex::NFAState*>
    */
    struct setHash{
        std::size_t operator()(const std::set<Regex::NFAState*> s) const{
            std::size_t seed = s.size();
            for(auto i : s) {
                seed ^= reinterpret_cast<uintptr_t>(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
    int id;
    const std::size_t CACHELIMIT;
    std::string reg;
    bool matchStart;
    bool matchEnd;
    bool reversed;
    Regex::NFAState* nfaStart;
    Regex::NFAState* nfaAcc;
    // lookup dfa state by its set of nfa states
    std::unordered_map<std::set<Regex::NFAState*>, Regex::DFAState*, setHash> dfa;
    std::unordered_map<Regex::NFAState*, std::unordered_set<Regex::NFAState*>> epsilonClosureCache;
    Regex::DFAState* reject;
    Regex::DFAState* dfaStart;
    void parse();
    void deleteNFA();
    void deleteDFA(Regex::DFAState* st = nullptr);
    void epsilonClosure(Regex::NFAState*, std::set<Regex::NFAState*>&);
    void buildDFAStart();
    std::pair<Regex::NFAState*, std::vector<Regex::NFAState*>> parse(std::size_t&);
    std::pair<Regex::NFAState*, std::vector<Regex::NFAState*>> parseGroup(std::size_t&);
    std::pair<Regex::NFAState*, std::vector<Regex::NFAState*>> parseCharClass(std::size_t&);
    std::pair<Regex::NFAState*, std::vector<Regex::NFAState*>> parseSpecial(std::size_t&);
    std::pair<Regex::NFAState*, std::vector<Regex::NFAState*>> parseChar(std::size_t&);
    std::pair<Regex::NFAState*, std::vector<Regex::NFAState*>> parseInterval(Regex::NFAState*, std::vector<Regex::NFAState*>, std::size_t&);
    std::pair<Regex::NFAState*, std::vector<Regex::NFAState*>> doStar(Regex::NFAState*, std::vector<Regex::NFAState*>);
    std::pair<Regex::NFAState*, std::vector<Regex::NFAState*>> doQuestion(Regex::NFAState*, std::vector<Regex::NFAState*>);
    std::pair<Regex::NFAState*, std::vector<Regex::NFAState*>> doPlus(Regex::NFAState*, std::vector<Regex::NFAState*>);
    std::pair<Regex::NFAState*, std::vector<Regex::NFAState*>> doPipe(Regex::NFAState*, std::vector<Regex::NFAState*>, std::size_t&);
    std::pair<Regex::NFAState*, std::vector<Regex::NFAState*>> copy(std::unordered_set<Regex::NFAState*>&, Regex::NFAState*, std::vector<Regex::NFAState*>);
    std::unordered_set<Regex::NFAState*> makeList(Regex::NFAState*, const std::vector<Regex::NFAState*>&) const;
    Regex::DFAState* nextDFA(char, Regex::DFAState*);
    Regex::NFAState* makeEpsilonState();
    Regex::NFAState* makeCharState(char);
    Regex::NFAState* makeCharClassState(std::string);
    void printStates();
};