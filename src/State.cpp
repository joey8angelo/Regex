#include "../headers/Regex.h"
/*
    Deletes nfa
*/
void Regex::deleteNFA(){
    for(auto i = nfa.begin(); i != nfa.end(); i++){
        delete i->second;
    }
    nfa.clear();
}

Regex::NFAState::~NFAState(){}

Regex::DFAState::DFAState(std::set<int>* s) : accept(false), ls(s){}
Regex::DFAState::DFAState(std::set<int>* s, bool a) : accept(a), ls(s){}

/*
    Find the next DFA state on a given character from the current state
*/
Regex::DFAState* Regex::nextDFA(char c, Regex::DFAState* d){
    // if next state does not exist build it
    bool del = dfa.size() == CACHELIMIT;
    if(d->out.find(c) == d->out.end()){
        std::set<int>* t = new std::set<int>;
        for(auto p : *(d->ls)){
            if(nfa[p]->hasChar(c) && nfa[p]->out1){
                t->insert(nfa[p]->out1->ID);
                epsilonClosure(nfa[p]->out1->ID, t);
            }
        }
        // if the next set is empty this state transitions to reject on that character
        if(!t->size()){
            d->out[c] = reject;
            delete t;
        }
        // if the next set does not exist as a dfa state create one and transition to it
        else if(dfa.find(t) == dfa.end()){
            if(dfa.size() == CACHELIMIT){
                deleteDFA(d);
            }
            dfa[t] = new Regex::DFAState(t, t->find(nfaAcc) != t->end());
            d->out[c] = dfa[t];
        }
        // transition to the dfa state corresponding to the set of nfa states
        else{
            d->out[c] = dfa[t];
            delete t;
        }
    }
    return d->out[c];
}

/*
    Creates dfaStart from nfaStart if it does not exist already
*/
void Regex::buildDFAStart(){
    if(dfaStart)
        return;

    std::set<int>* c = new std::set<int>;
    epsilonClosure(nfaStart, c);
    c->insert(nfaStart);
    dfaStart = new DFAState(c, c->find(nfaAcc) != c->end());
    if(dfa.size() == CACHELIMIT){
        deleteDFA();
    }
    dfa[c] = dfaStart;
}

/*
    Deletes dfa - does not delete dfaStart if there is one and given DFAState
*/
void Regex::deleteDFA(Regex::DFAState* st){
    if(dfaStart){
        dfa.erase(dfaStart->ls);
        dfaStart->out.clear();
    }
    if(st){
        dfa.erase(st->ls);
        st->out.clear();
    }
    for(auto i = dfa.begin(); i != dfa.end(); i++){
        delete i->second;
    }
    dfa.clear();
    if(dfaStart)
        dfa[dfaStart->ls] = dfaStart;
    if(st)
        dfa[st->ls] = st;
}

Regex::DFAState::~DFAState(){
    delete ls;
}

bool Regex::NFAStateChar::isEpsilon(){ return false; }
bool Regex::NFAStateChar::hasChar(char ch){ return ch == c; }
Regex::NFAState* Regex::NFAStateChar::makeCopy(){ return new NFAStateChar(-1, c); }

bool Regex::NFAStateCharClass::isEpsilon(){ return false; }
bool Regex::NFAStateCharClass::hasChar(char ch){ return cc == ch; }
Regex::NFAState* Regex::NFAStateCharClass::makeCopy(){return new NFAStateCharClass(-1, cc); }

bool Regex::NFAStateEpsilon::isEpsilon(){ return true; }
bool Regex::NFAStateEpsilon::hasChar(char ch){ return false; }
Regex::NFAState* Regex::NFAStateEpsilon::makeCopy(){ return new NFAStateEpsilon(-1); }