#include "../headers/Regex.h"

Regex::Regex(std::string in) : reg(in), matchStart(false), matchEnd(false), reversed(false){
    if(in.size() && in[0] == '^')
        matchStart = true;
    if(in.size() && in[in.size()-1] == '$' && in.size() > 1 && in[in.size()-2] != '\\'){
        matchEnd = true;
        reg.pop_back();
    }
    // if matching from the end and not the start, invert the regular expression
    // inverting simply means when concatenating groups/characters append them to
    // the front of the current expression
    if(!matchStart && matchEnd){
        reversed = true;
        matchEnd = false;
        matchStart = true;
    }
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
    Prints all the NFA states and their transitions (for testing)
*/
void Regex::printNFAStates(){
    std::cout << "States:" << std::endl << std::endl;

    for(auto i = this->nfa.begin(); i != this->nfa.end(); i++){
        std::cout << "State " << i->first;
        if(i->second->ID == nfaAcc)
            std::cout << " - ACCEPT" << std::endl;
        else if(i->second->ID == nfaStart)
            std::cout << " - START" << std::endl;
        else
            std::cout << std::endl;
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

/*
    Given a state compute what states are reachable through epsilon transitions
*/
std::unordered_set<int> Regex::epsilonClosure(int n) const{
    Regex::NFAState* s = nfa.at(n);
    std::unordered_set<int> states;
    std::vector<Regex::NFAState*> stack;
    stack.push_back(s);
    
    while(stack.size()){
        Regex::NFAState* top = stack[stack.size()-1];
        stack.pop_back();
        if(top->c.epsilon && top->out1 != nullptr && states.find(top->out1->ID) == states.end()){
            states.insert(top->out1->ID);
            stack.push_back(top->out1);
        }
        if(top->c.epsilon && top->out2 != nullptr && states.find(top->out2->ID) == states.end()){
            states.insert(top->out2->ID);
            stack.push_back(top->out2);
        }
    }
    return states;
}