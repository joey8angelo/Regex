#pragma once
#include "includes.h"

class Regex{
    public:
    Regex(std::string);
    ~Regex();
    std::string getProcessedExpression(){return this->processedReg;}
    std::pair<int, std::string> find(std::string);
    bool test(std::string);
    std::vector<std::pair<int, std::string>> group(std::string);
    private:
    void printNFAStates();
    std::string preprocess();
    std::string findPrevChar(std::string);
    std::string reg;
    std::string processedReg;
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
    int nfaStart;
    int nfaAcc;
    std::unordered_map<int, Regex::NFAState*> nfa;
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
    std::unordered_set<int> epsilonClosure(int);
    std::unordered_set<int> makeList(Regex::NFAState*, Regex::NFAState*);
    std::vector<Regex::NFAState*> copy(std::unordered_set<int>&, int, int);
    std::pair<int, std::string> Rfind(std::string);
};