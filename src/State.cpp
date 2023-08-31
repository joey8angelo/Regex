#include "../headers/Regex.h"
// build state with epsilon transition
Regex::NFAState::NFAState(int i, std::unordered_map<int, Regex::NFAState*>* nfa) : ID(i), c(Regex::CharacterClass("", true)), out1(nullptr), out2(nullptr) { (*nfa)[ID] = this; }
// build state with char transition
Regex::NFAState::NFAState(char c, int i, std::unordered_map<int, Regex::NFAState*>* nfa) : ID(i), c(Regex::CharacterClass(std::string(1,c), false)), out1(nullptr), out2(nullptr) { (*nfa)[ID] = this; }
// build state with string transition (character class)
Regex::NFAState::NFAState(std::string s, int i, std::unordered_map<int, Regex::NFAState*>* nfa) : ID(i), c(Regex::CharacterClass(s, false)), out1(nullptr), out2(nullptr) { (*nfa)[ID] = this; }
Regex::NFAState::~NFAState(){}

// build state
Regex::DFAState::DFAState(int i) : accept(false), ID(i) {}
Regex::DFAState::~DFAState(){}