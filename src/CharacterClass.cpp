#include "../headers/Regex.h"

/*
    If the first character is a ^ then the class is negated
    Inserts all the characters in the given string to the set
*/
Regex::CharacterClass::CharacterClass(std::string ch, bool e) : negated(false), epsilon(e){
    if(epsilon)
        return;
    if(ch.size() && ch[0] == '^')
        negated = true;

    for(int i = (negated ? 1 : 0); i < ch.size(); i++){
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

std::string Regex::CharacterClass::stringify(){
    std::string r = "";
    for(char ch : characters){
        r += ch;
    }
    return r;
}