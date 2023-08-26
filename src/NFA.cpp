#include "../headers/Regex.h"

Regex::NFAState::NFAState() : accept(false) {}
Regex::NFAState::~NFAState(){
    for(auto t : this->transitions){
        delete t.second;
    }
}