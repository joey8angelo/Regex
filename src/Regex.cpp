#include "../headers/Regex.h"

Regex::Regex(std::string in) : reg(in), matchStart(false), matchEnd(false){
    if(this->reg.size() && this->reg[0] == '^')
        this->matchStart = true;
    if(this->reg.size() && this->reg[this->reg.size()-1] == '$')
        matchEnd = true;
    std::string infix = infixToPostfix(preprocess());

}

Regex::~Regex(){}

std::string Regex::preprocess() const{
    std::string ret = "";
    std::unordered_map<std::string, std::string> special = {{".", "[^\n\r]"}, {"\\.", "[.]"}, {"\\*", "[*]"}, 
                                                            {"\\+", "[+]"}, {"\\?", "[?]"}, {"\\w", "[a-zA-Z0-9_]"}, 
                                                            {"\\W", "[^a-zA-Z0-9_]"}, {"\\d", "[0-9]"}, 
                                                            {"\\D", "[^0-9]"}, {"\\s", "[\t\n\v\f\r \xA0]"}, 
                                                            {"\\S", "[^\t\n\v\f\r \xA0]"}};
    for(int i = this->matchStart ? 1 : 0; i < this->matchEnd ? this->reg.size()-1 : this->reg.size(); i++){
        
    }
}