#include "../headers/Regex.h"

Regex::Regex(std::string in) : reg(in), matchStart(false), matchEnd(false), reversed(false), dfaStart(nullptr), CACHELIMIT(200){
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
    reject = new Regex::DFAState(new std::set<int>({-1}));
}

Regex::~Regex(){
    deleteNFA();
    deleteDFA();
    if(reject)
        delete reject;
    if(dfaStart)
        delete dfaStart;
}

/*
    Given a state and a set insert the states that can be reached through epsilon into the set
*/
void Regex::epsilonClosure(int n, std::set<int>* states) const{
    Regex::NFAState* s = nfa.at(n);
    std::vector<Regex::NFAState*> stack;
    stack.push_back(s);
    
    while(stack.size()){
        Regex::NFAState* top = stack[stack.size()-1];
        stack.pop_back();
        if(top->c.epsilon && top->out1 != nullptr && states->find(top->out1->ID) == states->end()){
            states->insert(top->out1->ID);
            stack.push_back(top->out1);
        }
        if(top->c.epsilon && top->out2 != nullptr && states->find(top->out2->ID) == states->end()){
            states->insert(top->out2->ID);
            stack.push_back(top->out2);
        }
    }
}