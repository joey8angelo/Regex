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
    
    /*
        when processing escape characters or interval operators 
        the resulting string changes length so "\d\d" -> "[0-9][0-9]""
                                                 0 1  ->  0000011111
        pos keeps track of this change so error messages can have more specific locations
    */
    std::vector<int> pos; 
    bool matchStart;
    bool matchEnd;
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
    std::vector<Regex::NFAState*> parse_(std::string&, int&);
    std::vector<Regex::NFAState*> parseChar(std::string&, int&, std::string);
    std::vector<Regex::NFAState*> parseChar(std::string&, int&, char);
    std::vector<Regex::NFAState*> parseChar(std::string&, int&, Regex::NFAState*);
    void deleteNFA();
    std::unordered_set<int> epsilonClosure(int);
};