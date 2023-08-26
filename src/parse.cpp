#include "../headers/Regex.h"

int id = 0;

void Regex::parse(){
    int p = 0;
    auto t = parse_(processedReg, p);
    nfa = t[0];
    t[1]->accept = true;
}

std::vector<Regex::NFAState*> Regex::parse_(std::string& in, int& pos){
    Regex::NFAState*I,*e,*s,*t;
    I=new Regex::NFAState();
    I->ID = id++;
    e=I;
    s=e;
    t=s;

    for(;pos < in.size(); pos++){
        if(in[pos] == '('){
            auto temp = parse_(in, ++pos);
            s = temp[0];
            e = temp[1];
            t->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), s));
        }
        else if(in[pos] == '|'){
            t = new Regex::NFAState();
            t->ID = id++;
            e=t;
            if(pos+1 >= in.size())
                throw std::runtime_error("No right hand side of '|' operator at position " + std::to_string(pos));
            if(!pos)
                throw std::runtime_error("No left hand side of '|' operator at position " + std::to_string(pos));
            if(in[pos+1] == '('){
                pos += 2;
                auto t = parse_(in, pos);
                s=t[0];
                e=t[1];
            }
            else{
                auto temp = parseChar(in[pos]);
                pos++;
                s=temp[0];
                e=temp[1];
                I->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), s));
                e->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), t));
                e=t;
            }
        }
        else if(in[pos] == ')'){
            if(pos+1 < in.size()){
                if(in[pos+1] == '*'){
                    I->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), e));
                    e->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), I));
                    pos++;
                }
                else if(in[pos+1] == '+'){
                    e->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), I));
                    pos++;
                }
                else if(in[pos+1] == '?'){
                    I->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), e));
                    pos++;
                }
            }
            return {I,e};
        }
        else if(in[pos] == '['){
            std::string curr = "";
            for(pos++; pos < in.size(); pos++){
                if(in[pos] == ']'){
                    auto temp = parseChar(Regex::CharacterClass(curr, false));
                    s = temp[0];
                    e = temp[1];
                    t->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), s));
                    t=e;
                    break;
                }
                else if(in[pos] == '\\'){
                    if(pos+1 >= in.size())
                        throw std::runtime_error("Escape character not found at position " + std::to_string(pos));
                    curr += in[pos+1];
                    pos++;
                }
                else{
                    curr += in[pos];
                }
            }
        }
        else if(in[pos] == '*'){
            s->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), e));
            e->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), s));
        }
        else if(in[pos] == '+'){
            e->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), s));
        }
        else if(in[pos] == '?'){
            s->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), e));
        }
        else{
            std::vector<Regex::NFAState*> temp;
            if(in[pos] == '\\'){
                if(pos+1 >= in.size())
                    throw std::runtime_error("Escape character not found at position " + std::to_string(pos));
                temp = parseChar(in[pos+1]);
                pos++;
            }
            else{
                temp = parseChar(in[pos]);
            }
            s=temp[0];
            e=temp[1];
            t->transitions.push_back(std::make_pair(Regex::CharacterClass("", true), s));
            t=e;
        }
    }
    return {I,e};
}

std::vector<Regex::NFAState*> Regex::parseChar(char c){
    Regex::NFAState*s,*e;
    s = new Regex::NFAState();
    s->ID = id++;
    e = new Regex::NFAState();
    e->ID = id++;
    s->transitions.push_back(std::make_pair(Regex::CharacterClass(std::string(1, c), false), e));
    return {s,e};
}
std::vector<Regex::NFAState*> Regex::parseChar(Regex::CharacterClass c){
    Regex::NFAState*s,*e;
    s = new Regex::NFAState();
    s->ID = id++;
    e = new Regex::NFAState();
    e->ID = id++;
    s->transitions.push_back(std::make_pair(c, e));
    return {s,e};
}