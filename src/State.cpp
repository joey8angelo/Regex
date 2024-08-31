#include "../headers/Regex.h"
/*
    Deletes nfa FIXME
*/
void Regex::deleteNFA(){
    std::unordered_set<Regex::NFAState*> deleted;
    std::vector<Regex::NFAState*> stack;
    stack.push_back(nfaStart);
    while(stack.size()){
        Regex::NFAState* top = stack[stack.size()-1];
        stack.pop_back();
        if(deleted.find(top) != deleted.end())
            continue;
        if(top->out1 && deleted.find(top->out1) == deleted.end()){
            stack.push_back(top->out1);
        }
        if(top->out2 && deleted.find(top->out2) == deleted.end()){
            stack.push_back(top->out2);
        }
        deleted.insert(top);
        delete top;
    }
}

Regex::DFAState::DFAState(std::set<Regex::NFAState*> s) : accept(false), ls(s){}
Regex::DFAState::DFAState(std::set<Regex::NFAState*> s, bool a) : accept(a), ls(s){}
/*
    Deletes dfa - does not delete dfaStart or the given DFAState
*/
void Regex::deleteDFA(Regex::DFAState* st){
    for (auto i = dfa.begin(); i != dfa.end(); i++){
        if (i->second != st && i->second != dfaStart && i->second != reject)
            delete i->second;
    }
}

/*
    Find the next DFA state on a given character from the current state
*/
Regex::DFAState* Regex::nextDFA(char c, Regex::DFAState* d){
    // if next state does not exist build it
    if(d->out.find(c) == d->out.end()){
        std::set<Regex::NFAState*> t = std::set<Regex::NFAState*>();
        for(auto p : d->ls){
            if(p->hasChar(c) && p->out1){
                t.insert(p->out1);
                epsilonClosure(p->out1, t);
            }
        }
        if(dfa.find(t) == dfa.end()){
            if(dfa.size() == CACHELIMIT)
                deleteDFA(d);
            dfa[t] = new Regex::DFAState(t, t.find(nfaAcc) != t.end());
        }
        d->out[c] = dfa[t]; // an empty set is in dfa and will point to the reject state if t is empty
    }
    return d->out[c];
}

/*
    Creates dfaStart from nfaStart if it does not exist already
*/
void Regex::buildDFAStart(){
    if(dfaStart)
        return;

    auto c = std::set<Regex::NFAState*>();
    epsilonClosure(nfaStart, c);
    c.insert(nfaStart);
    dfaStart = new Regex::DFAState(c, c.find(nfaAcc) != c.end());
    if(dfa.size() == CACHELIMIT){
        deleteDFA();
    }
    dfa[c] = dfaStart;
}

Regex::NFAStateChar::NFAStateChar(char ch): Regex::NFAState(), c(ch){}
bool Regex::NFAStateChar::isEpsilon(){ return false; }
bool Regex::NFAStateChar::hasChar(char ch){ return ch == c; }
Regex::NFAState* Regex::NFAStateChar::makeCopy(){ return new Regex::NFAStateChar(c); }

Regex::NFAStateCharClass::NFAStateCharClass(std::string s): Regex::NFAState(), cc(Regex::CharacterClass(s)){}
Regex::NFAStateCharClass::NFAStateCharClass(Regex::CharacterClass chc): Regex::NFAState(), cc(chc){}
bool Regex::NFAStateCharClass::isEpsilon(){ return false; }
bool Regex::NFAStateCharClass::hasChar(char ch){ return cc == ch; }
Regex::NFAState* Regex::NFAStateCharClass::makeCopy(){return new Regex::NFAStateCharClass(cc); }

Regex::NFAStateEpsilon::NFAStateEpsilon(): Regex::NFAState(){}
bool Regex::NFAStateEpsilon::isEpsilon(){ return true; }
bool Regex::NFAStateEpsilon::hasChar(char ch){ return false; }
Regex::NFAState* Regex::NFAStateEpsilon::makeCopy(){ return new Regex::NFAStateEpsilon(); }