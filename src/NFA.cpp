#include "../headers/Regex.h"
// build state with epsilon transition
Regex::NFAState::NFAState() : accept(false), c(Regex::CharacterClass("", true)), out1(nullptr), out2(nullptr) {}
// build state with char transition
Regex::NFAState::NFAState(char c) : accept(false), c(Regex::CharacterClass(std::string(1,c), false)), out1(nullptr), out2(nullptr) {}
// build state with string transition (character class)
Regex::NFAState::NFAState(std::string s) : accept(false), c(Regex::CharacterClass(s, false)), out1(nullptr), out2(nullptr) {}

Regex::NFAState::~NFAState(){}