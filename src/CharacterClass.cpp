#include "../headers/Regex.h"

Regex::CharacterClass::CharacterClass(std::string ch, bool e) : negated(false), epsilon(e){
    if(epsilon)
        return;
    bool hasBrackets = false;
    int p = 0;
    if(ch.size() && ch[0] == '['){
        hasBrackets = true;
        p++;
    }
    if(hasBrackets && ch.size() > 2 && ch[1] == '^'){
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

bool Regex::CharacterClass::operator==(char c){
    bool t = this->characters.find(c) != this->characters.end();
    return negated ? !t : t; 
}