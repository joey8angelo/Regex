#include "../headers/Regex.h"

Regex::Regex(std::string in) : id(0), CACHELIMIT(200), reg(in), matchStart(false), matchEnd(false), reversed(false), dfaStart(nullptr){
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
    reject = new Regex::DFAState(std::set<Regex::NFAState*>());
    dfa[reject->ls] = reject;
    // printStates();
}

Regex::~Regex(){
    deleteNFA();
    deleteDFA();
    delete reject;
    if(dfaStart)
        delete dfaStart;
}

/*
    Given a state and a set insert the states that can be reached through epsilon into the set
*/
void Regex::epsilonClosure(Regex::NFAState* n, std::set<Regex::NFAState*>& states){
    if(epsilonClosureCache.find(n) != epsilonClosureCache.end()){
        for(auto i : epsilonClosureCache.at(n)){
            states.insert(i);
        }
        return;
    }
    Regex::NFAState* s = n;
    std::vector<Regex::NFAState*> stack;
    stack.push_back(s);
    epsilonClosureCache[n] = std::unordered_set<Regex::NFAState*>();
    
    while(stack.size()){
        Regex::NFAState* top = stack[stack.size()-1];
        stack.pop_back();
        if(top->isEpsilon()){
            if(top->out1 != nullptr && states.find(top->out1) == states.end()){
                states.insert(top->out1);
                stack.push_back(top->out1);
                epsilonClosureCache[n].insert(top->out1);
            }
            if(top->out2 != nullptr && states.find(top->out2) == states.end()){
                states.insert(top->out2);
                stack.push_back(top->out2);
                epsilonClosureCache[n].insert(top->out2);
            }
        }
    }
}

/*
    make a new epsilon state
*/
Regex::NFAState* Regex::makeEpsilonState(){
    return new Regex::NFAStateEpsilon();
}
/*
    make a new char state
*/
Regex::NFAState* Regex::makeCharState(char c){
    return new Regex::NFAStateChar(c);
}
/*
    make a new CharacterClass state
*/
Regex::NFAState* Regex::makeCharClassState(std::string s){
    return new Regex::NFAStateCharClass(s);
}