#include "../headers/Regex.h"

/*
    Remove outer brackets and parse the inside
    If the first character is a ^ then the class is negated
    All characters are allowed without '\\' except for [ and ]
*/
Regex::CharacterClass::CharacterClass(std::string ch, bool e) : negated(false), epsilon(e){
    if(epsilon)
        return;
    bool hasBrackets = false;
    int p = 0;
    if(ch.size() && ch[0] == '['){
        hasBrackets = true;
        p++;
    }
    if(ch.size() > 1+p && ch[0+p] == '^'){
        negated = true;
        p++;
    }
    for(int i = p; i < ch.size()-(hasBrackets ? 1 : 0); i++){
        if(ch[i] == '\\'){
            if(i+1 != ch.size())
                characters.insert(ch[i+1]);
            else
                throw std::runtime_error("Escape character not found");
            i++;
        }
        else
            characters.insert(ch[i]);
    }
}
Regex::CharacterClass::~CharacterClass() {}

/*
    Returns true if the lhs is an element of the character set
    Returns the opposite if negated is true
*/
bool Regex::CharacterClass::operator==(char c){
    bool t = this->characters.find(c) != this->characters.end();
    return negated ? !t : t; 
}