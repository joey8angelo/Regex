#include "../headers/Regex.h"
// build state with epsilon transition
Regex::NFAState::NFAState(int i, std::unordered_map<int, Regex::NFAState*>* nfa) : ID(i), c(Regex::CharacterClass("", true)), out1(nullptr), out2(nullptr) { (*nfa)[ID] = this; }
// build state with char transition
Regex::NFAState::NFAState(char c, int i, std::unordered_map<int, Regex::NFAState*>* nfa) : ID(i), c(Regex::CharacterClass(std::string(1,c), false)), out1(nullptr), out2(nullptr) { (*nfa)[ID] = this; }
// build state with string transition (character class)
Regex::NFAState::NFAState(std::string s, int i, std::unordered_map<int, Regex::NFAState*>* nfa) : ID(i), c(Regex::CharacterClass(s, false)), out1(nullptr), out2(nullptr) { (*nfa)[ID] = this; }

/*
    Deletes nfa
*/
void Regex::deleteNFA(){
    for(auto i = nfa.begin(); i != nfa.end(); i++){
        delete i->second;
    }
    nfa.clear();
}

Regex::NFAState::~NFAState(){}

Regex::DFAState::DFAState(std::set<int> s) : accept(false), ls(s){}
Regex::DFAState::DFAState(std::set<int> s, bool a) : accept(a), ls(s){}

Regex::DFAState* Regex::DFAState::next(char c){
    if(out.find(c) == out.end())
        return nullptr;
    return out[c];
}
/*
    Deletes dfa - does not delete dfaStart if there is one
*/
void Regex::deleteDFA(){
    if(dfaStart){
        dfa.erase(dfaStart->ls);
        dfaStart->out.clear();
    }
    for(auto i = dfa.begin(); i != dfa.end(); i++){
        delete i->second;
    }
    dfa.clear();
    if(dfaStart)
        dfa[dfaStart->ls] = dfaStart;
}

Regex::DFAState::~DFAState(){}