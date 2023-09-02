#include "../headers/Regex.h"

Regex::Regex(std::string in) : reg(in), matchStart(false), matchEnd(false){
    if(in.size() && in[0] == '^')
        this->matchStart = true;
    if(in.size() && in[in.size()-1] == '$' && in.size() > 1 && in[in.size()-2] != '\\')
        matchEnd = true;
    processedReg = preprocess();
    parse();
}

Regex::~Regex(){
    deleteNFA();
}

/*
    Deletes nfa
*/
void Regex::deleteNFA(){
    for(auto i = nfa.begin(); i != nfa.end(); i++){
        delete i->second;
    }
    nfa.clear();
}

/* 
    process the input string into a usable form to convert to an NFA, throws syntax errors
    . -> [^\n\r]
    \\w -> [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]
    \\W -> [^abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]
    \\d -> [0123456789]
    \\D -> [^0123456789]
    \\s -> [\t\n\v\f\r \xA0]
    \\S -> [^\t\n\v\f\r \xA0]
    .{a} single digit repeat . a times
    .{a,} single digit with comma repeat . a times concat .*
    .{a,b} two digits comma separated repeat . a times repeat .? a-b times 
*/
std::string Regex::preprocess(){
    std::string ret = "";
    std::unordered_map<std::string, std::string> special = {{".", "^\n\r"}, {"\\S", "^\t\n\v\f\r \xA0"}, {"\\s", "\t\n\v\f\r \xA0"},
                                                            {"\\W", "^abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"},
                                                            {"\\w", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"},
                                                            {"\\d", "0123456789"}, {"\\D", "^0123456789"}};
    int st = this->matchStart ? 1 : 0;
    int cmp = this->matchEnd ? this->reg.size()-1 : this->reg.size();
    bool charClass = false;
    int paren = 0;

    for(int i = st; i < cmp; i++){

        // begin a character class
        if(this->reg[i] == '['){
            if(charClass)
                throw std::runtime_error("Character classes cannot be nested, try '\\\\[' at position " + std::to_string(i));
            charClass = true;
        }
        // end a character class
        if(this->reg[i] == ']'){
            if(!charClass)
                throw std::runtime_error("Unbalanced brackets, try '\\\\]' at position " + std::to_string(i));
            charClass = false;
        }

        // if . is found
        if(!charClass && special.find(std::string(1, this->reg[i])) != special.end()){
            ret += "[" + special[std::string(1, this->reg[i])] + "]";
            for(int j = 0; j < special[std::string(1, this->reg[i])].size()+2; j++){pos.push_back(i);}
        }
        // escape character
        else if(this->reg[i] == '\\'){
            if(i+1 >= this->reg.size())
                throw std::runtime_error("Escape character does not exist at position " + std::to_string(i));
            else if(special.find(this->reg.substr(i, 2)) == special.end()){
                std::string t = this->reg;
                ret += "\\";
                ret += t[i+1];
                pos.push_back(i);
                pos.push_back(i+1);
            }
            else if(charClass){
                throw std::runtime_error("Cannot use special escape characters in character class at position " + std::to_string(i));
            }
            else{
                ret += "[" + special[this->reg.substr(i, 2)] + "]";
                for(int j = 0; j < special[this->reg.substr(i,2)].size()+2; j++){pos.push_back(i);}
            }
            i++;
        }
        // interval operator
        else if(!charClass && this->reg[i] == '{'){
            int init = i;
            i++;
            int a = -1;
            int b = -1;
            bool commaFlag = false;
            std::string current = "";
            for(;i < cmp; i++){
                if(this->reg[i] == '}')
                    break;
                if(!std::isdigit(this->reg[i]) && !(this->reg[i] == ',' || this->reg[i] == ' '))
                    throw std::runtime_error("Invalid expression in interval operator at position " + std::to_string(init));
                if(this->reg[i] == ','){
                    a = std::stoi(current);
                    current = "";
                    commaFlag = true;
                }
                else{
                    current += this->reg[i];
                }
            }

            if(i == cmp)
                throw std::runtime_error("No closing bracket for opening bracket at position " + std::to_string(init));

            if(current.size()){
                if(a == -1)
                    a = std::stoi(current);
                else
                    b = std::stoi(current);
            }

            if(a == -1)
                throw std::runtime_error("Invalid expression in interval operator at position " + std::to_string(init));
            if(b < a && b != -1)
                throw std::runtime_error("Invalid expression in interval operator at position " + std::to_string(init));

            std::string previousChar = findPrevChar(ret);
            char t = previousChar[0];
            if(!previousChar.size() || previousChar[0] == '\000')
                throw std::runtime_error("No group or character to capture");

            for(int j = 0; j < previousChar.size(); j++){ret.pop_back();}
            for(int j = 0; j < previousChar.size(); j++){pos.pop_back();}

            for(int j = 0; j < a; j++){
                ret += previousChar;
                for(int x = 0; x < previousChar.size(); x++){
                    pos.push_back(i);
                }
            }
            if(b != -1){
                for(int j = 0; j < b-a; j++){
                    ret += previousChar + "?";
                    for(int x = 0; x < previousChar.size()+1; x++){pos.push_back(i);}
                }
            }
            else if(commaFlag){
                ret += previousChar + "*";
                for(int j = 0; j < previousChar.size()+1; j++){pos.push_back(i);}
            }
        }
        // range in a character class
        else if(this->reg[i] == '-' && charClass){
            if((i-1 < 0 || this->reg[i-1] == '[') && (i-2 < 0 || this->reg[i-2] != '\\'))
                throw std::runtime_error("Invalid range for character class at position " + std::to_string(i));
            char l = this->reg[i-1];
            if((i+1 >= this->reg.size() || this->reg[i+1] == ']') && (i+2 >= this->reg.size() || this->reg[i+2] != '\\'))
                throw std::runtime_error("Invalid range for character class at position " + std::to_string(i));
            char r = this->reg[i+1];
            if(r < l)
                throw std::runtime_error("Invalid range for character class at position " + std::to_string(i));

            for(int j = l+1; j < r; j++){
                if(char(j) == '[' || char(j) == ']' || char(j) == '\\'){
                    ret += "\\";
                    pos.push_back(j);
                }
                ret += char(j);
                pos.push_back(i);
            }
        }
        else{
            if(this->reg[i] == ']')
                charClass = false;
            else if(!charClass &&this->reg[i] == '(')
                paren++;
            else if(!charClass && this->reg[i] == ')'){
                if(!paren)
                    throw std::runtime_error("Unbalanced parenthesis at position " + std::to_string(i));
                paren--;
            }
            else if(!charClass && this->reg[i] == '}')
                throw std::runtime_error("No opening bracket for closing bracket at position " + std::to_string(i));
            ret += this->reg[i];
            pos.push_back(i);
        }
    }
    if(paren)
        throw std::runtime_error("Unbalanced parenthesis");
    if(charClass)
        throw std::runtime_error("Unbalanced braces");
    return ret;
}

/*
    Finds and returns a group/character class/single character from the end of the given string
*/
std::string Regex::findPrevChar(std::string s){
    
    // if ) match the entire group
    // if ] match the entire character class
    // else match a single character
    int p = s.size()-1;
    if(s[p] == ')'){
        int paren = 1;
        for(int i = p-1; i >= 0; i--){
            if(s[i] == '('){
                paren--;
                if(!paren)
                    return s.substr(i, p-i+1);
            }
            else if(s[i] == ')')
                paren++;
        }
        throw std::runtime_error("Unbalanced parenthesis at position " + std::to_string(p));
    }
    else if(s[p] == ']'){
        for(int i = p-1; i >= 0; i--){
            if(s[i] == ']')
                throw std::runtime_error("Unbalanced braces at position " + std::to_string(p));
            if(s[i] == '['){
                return s.substr(i, p-i+1);
            }
        }
        throw std::runtime_error("Unbalanced parenthesis at position " + std::to_string(p));
    }
    else if((s[p] == '?' || s[p] == '+' || s[p] == '*' || s[p] == '|' || s[p] == ')' || s[p] == ']') && p-1 >= 0 && s[p-1] != '\\')
        return "";
    else
        return std::string(1, s[p]);
}
/*
    Prints all the NFA states and their transitions (for testing)
*/
void Regex::printNFAStates(){
    std::cout << "States:" << std::endl << std::endl;

    for(auto i = this->nfa.begin(); i != this->nfa.end(); i++){
        std::cout << "State " << i->first << std::endl;
        if(i->second->ID == nfaAcc)
            std::cout << "    ACCEPT" << std::endl;
        if(i->second->out1 != nullptr){
            std::cout << "    goes to state " << i->second->out1->ID << " on ";  
            std::string trans = "";
            if(i->second->c.epsilon)
                trans = "epsilon";
            else{
                auto j = i->second->c.characters.begin();
                if(i->second->c.negated)
                    trans += "! ";
                trans += "'";
                if(!isprint(*j))
                    trans += "nonprintable";
                else
                    trans += *j;
                trans += "'";
                for(j++; j != i->second->c.characters.end(); j++){
                    trans += ", '";
                    if(!isprint(*j))
                        trans += "nonprintable";
                    else
                        trans += *j;
                    trans += "'";
                }
            }
            std::cout << trans << std::endl;
        }
        if(i->second->out2 != nullptr){
            std::cout << "    goes to state " << i->second->out2->ID << " on ";  
            std::string trans = "";
            if(i->second->c.epsilon)
                trans = "epsilon";
            else{
                auto j = i->second->c.characters.begin();
                if(i->second->c.negated)
                    trans += "! ";
                trans = "'";
                if(!isprint(*j))
                    trans += "nonprintable";
                else
                    trans += *j;
                trans += "'";
                for(j++; j != i->second->c.characters.end(); j++){
                    trans += ", '";
                    if(!isprint(*j))
                        trans += "nonprintable";
                    else
                        trans += *j;
                    trans += "'";
                }
            }
            std::cout << trans << std::endl;
        }
    }
    std::cout << std::endl;
}