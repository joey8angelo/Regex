#include "../headers/Regex.h"

Regex::Regex(std::string in) : reg(in), matchStart(false), matchEnd(false), reversed(false), dfaStart(nullptr), CACHELIMIT(200), id(0){
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
void Regex::epsilonClosure(int n, std::set<int>* states){
    if(epsilonClosureCache.find(n) != epsilonClosureCache.end()){
        for(auto i : epsilonClosureCache.at(n)){
            states->insert(i);
        }
        return;
    }
    Regex::NFAState* s = nfa.at(n);
    std::vector<Regex::NFAState*> stack;
    stack.push_back(s);
    epsilonClosureCache[n] = std::unordered_set<int>();
    
    while(stack.size()){
        Regex::NFAState* top = stack[stack.size()-1];
        stack.pop_back();
        if(top->isEpsilon()){
            if(top->out1 != nullptr && states->find(top->out1->ID) == states->end()){
                states->insert(top->out1->ID);
                stack.push_back(top->out1);
                epsilonClosureCache[n].insert(top->out1->ID);
            }
            if(top->out2 != nullptr && states->find(top->out2->ID) == states->end()){
                states->insert(top->out2->ID);
                stack.push_back(top->out2);
                epsilonClosureCache[n].insert(top->out2->ID);
            }
        }
    }
}

/*
    make a new epsilon state and add it to nfa
*/
Regex::NFAState* Regex::makeEpsilonState(){
    nfa[id] = new Regex::NFAStateEpsilon(id);
    return nfa[id++];
}
/*
    make a new char state and add it to nfa
*/
Regex::NFAState* Regex::makeCharState(char c){
    nfa[id] = new Regex::NFAStateChar(id, c);
    return nfa[id++];
}
/*
    make a new CharacterClass state and add it to nfa
*/
Regex::NFAState* Regex::makeCharClassState(std::string s){
    nfa[id] = new Regex::NFAStateCharClass(id, s);
    return nfa[id++];
}