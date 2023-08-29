#include "../headers/Regex.h"

int id = 0;

/*
    Builds the NFA from the processed string
    sets the last state as the accept state
*/
void Regex::parse(){
    int p = 0;
    auto t = parse_(processedReg, p);
    nfa = t[0];
    if(t[1]->out1 == nullptr){
        t[1]->out1 = new Regex::NFAState();
        t[1]->out1->ID = id++;
        t[1]->out1->accept = true;
    }
    else{
        t[1]->out2 = new Regex::NFAState();
        t[1]->out2->ID = id++;
        t[1]->out2->accept = true;
    }
    id=0;
}

/*
    Recurses when capturing a group - when the current position is '('
    returns from the group on ')'
    1 lookahead performs operations like *,+,|,?
    calls parseChar on characters and character classes
    returns the start and end of the current NFA subset
*/
std::vector<Regex::NFAState*> Regex::parse_(std::string& in, int& currPos){
    Regex::NFAState*s,*e,*i,*c;
    s=nullptr;
    e=s;i=e;c=i;
    for(;currPos < in.size(); currPos++){
        if(in[currPos] == '('){
            auto temp = parse_(in, ++currPos);
            s = temp[0];
            e = temp[1];
            if(c != nullptr){
                if(c->out1 == nullptr)
                    c->out1 = s;
                else
                    c->out2 = s;
                c=e;
            }
            else{
                i=s;
                c=e;
            }
        }
        else if(in[currPos] == ')'){
            if(currPos+1 < in.size() && in[currPos+1] == '*'){
                currPos++;
                Regex::NFAState* t1 = new Regex::NFAState();
                t1->ID = id++;
                Regex::NFAState* t2 = new Regex::NFAState();
                t2->ID = id++;
                if(c->out1)
                    c->out2 = t2;
                else
                    c->out1 = t2;
                t2->out1 = i;
                c=t2;
                t1->out1 = i;
                t1->out2 = t2;
                i=t1;
            }
            else if(currPos+1 < in.size() && in[currPos+1] == '+'){
                currPos++;
                Regex::NFAState* t1 = new Regex::NFAState();
                t1->ID = id++;
                Regex::NFAState* t2 = new Regex::NFAState();
                t2->ID = id++;
                if(c->out1)
                    c->out2 = t2;
                else
                    c->out1 = t2;
                t2->out1 = t1;
                c=t2;
                t1->out1 = i;
                i=t1;
            }
            else if(currPos+1 < in.size() && in[currPos+1] == '?'){
                currPos++;
                Regex::NFAState* t1 = new Regex::NFAState();
                t1->ID = id++;
                Regex::NFAState* t2 = new Regex::NFAState();
                t2->ID = id++;
                t1->out1 = i;
                t1->out2 = t2;
                i=t1;
                c->out1 = t2;
                c=t2;
            }
            else if(currPos+1 < in.size() && in[currPos+1] == '|'){
                currPos++;
                Regex::NFAState* t1 = new Regex::NFAState();
                t1->ID = id++;
                Regex::NFAState* t2 = new Regex::NFAState();
                t2->ID = id++;
                t1->out1 = i;
                i=t1;
                if(c->out1 == nullptr)
                    c->out1 = t2;
                else
                    c->out2 = t2;
                c=t2;
                if(currPos+1 < in.size() && in[currPos+1] == '('){
                    currPos+=2;
                    auto temp = parse_(in, currPos);
                    s=temp[0];
                    e=temp[1];
                }
                else if(currPos+1 < in.size() && in[currPos+1] == '\\'){
                    currPos++;
                    if(currPos < in.size()){
                        auto temp = parseChar(in, currPos, in[currPos]);
                        s=temp[0];
                        e=temp[1];
                    }
                    else{
                        throw std::runtime_error("Escape character does not exist at position " + std::to_string(pos[currPos]));
                    }
                }
                else if(currPos+1 < in.size()){
                    auto temp = parseChar(in, currPos, in[currPos]);
                    s=temp[0];
                    e=temp[1];
                }
                else{
                    throw std::runtime_error("No right hand side of '|' operator at position " + std::to_string(pos[currPos]));
                }
                i->out2 = s;
                if(e->out1 == nullptr)
                    e->out1 = c;
                else
                    e->out2 = c;
            }
            return {i,c};
        }
        else if(in[currPos] == '['){
            std::string curr = "";
            for(currPos++; currPos < in.size(); currPos++){
                if(in[currPos] == ']'){
                    auto temp = parseChar(in, currPos, curr);
                    s=temp[0];
                    e=temp[1];
                    if(c != nullptr){
                        if(c->out1)
                            c->out2 = s;
                        else
                            c->out1 = s;
                    }
                    else
                        i=s;
                    c=e;
                    break;
                }
                else if(in[currPos] == '\\'){
                    if(currPos+1 >= in.size())
                        throw std::runtime_error("Escape character not found at position " + std::to_string(pos[currPos]));
                    curr += in[currPos+1];
                    currPos++;
                }
                else{
                    curr += in[currPos];
                }
            }
        }
        else if(in[currPos] == '+' || in[currPos] == '*' || in[currPos] == '?' || in[currPos] == '|'){
            throw std::runtime_error("Invalid use of '" + std::string(1, in[currPos]) + "' operator at position " + std::to_string(pos[currPos]));
        }
        else{
            std::vector<Regex::NFAState*> temp;
            if(in[currPos] == '\\'){
                if(currPos+1 >= in.size())
                    throw std::runtime_error("Escape character not found at position " + std::to_string(pos[currPos]));
                currPos++;
                temp = parseChar(in, currPos, in[currPos]);
            }
            else{
                temp = parseChar(in, currPos, in[currPos]);
            }
            s=temp[0];
            e=temp[1];
            if(c != nullptr){
                if(c->out1)
                    c->out2 = s;
                else
                    c->out1 = s;
            }
            else
                i=s;
            c=e;
        }
    }
    return {i,c};
}

/*
    parseChar for a string (character class)
*/
std::vector<Regex::NFAState*> Regex::parseChar(std::string& in, int& currPos, std::string c){
    Regex::NFAState* n = new Regex::NFAState(c);
    n->ID = id++;
    return parseChar(in,currPos,n);
}

/*
    parseChar for a char
*/
std::vector<Regex::NFAState*> Regex::parseChar(std::string& in, int& currPos, char c){
    Regex::NFAState* n = new Regex::NFAState(c);
    n->ID = id++;
    return parseChar(in,currPos,n);
}

/*
    Make a state with transition symbol given from previous parseChars
    applies operations with 1 lookahead to the single character state
    returns the start and end of the current NFA subset
*/
std::vector<Regex::NFAState*> Regex::parseChar(std::string& in, int& currPos, Regex::NFAState* n){
    if(currPos+1 < in.size() && in[currPos+1] == '*'){
        currPos++;
        Regex::NFAState* t1 = new Regex::NFAState();
        t1->ID = id++;
        n->out1 = t1;
        t1->out1 = n;
        return {t1,t1};
    }
    else if(currPos+1 < in.size() && in[currPos+1] == '+'){
        currPos++;
        Regex::NFAState* t1 = new Regex::NFAState();
        t1->ID = id++;
        n->out1 = t1;
        t1->out1 = n;
        return {n,t1};
    }
    else if(currPos+1 < in.size() && in[currPos+1] == '?'){
        currPos++;
        Regex::NFAState* t1 = new Regex::NFAState();
        t1->ID = id++;
        Regex::NFAState* t2 = new Regex::NFAState();
        t2->ID = id++;
        t1->out1 = n;
        t1->out2 = t2;
        n->out1 = t2;
        return {t1, t2};
    }
    else if(currPos+1 < in.size() && in[currPos+1] == '|'){
        currPos++;
        Regex::NFAState* t1 = new Regex::NFAState();
        t1->ID = id++;
        Regex::NFAState* t2 = new Regex::NFAState();
        t2->ID = id++;
        std::vector<Regex::NFAState*> temp;
        if(currPos+1 < in.size() && in[currPos+1] == '('){
            currPos+=2;
            temp = parse_(in, currPos);
        }
        else if(currPos+1 < in.size() && in[currPos+1] == '\\'){
            currPos++;
            if(currPos+1 < in.size()){
                currPos++;
                temp = parseChar(in, currPos, in[currPos]);
            }
            else{
                throw std::runtime_error("Escape character does not exist at position " + std::to_string(pos[currPos]));
            }
        }
        else if(currPos+1 < in.size()){
            currPos++;
            temp = parseChar(in, currPos, in[currPos]);
        }
        else{
            throw std::runtime_error("No right hand side of '|' operator at position " + std::to_string(pos[currPos]));
        }
        t1->out1 = n;
        t1->out2 = temp[0];
        n->out1 = t2;
        temp[1]->out1 = t2;
        return {t1, t2};
    }
    return {n, n};
}