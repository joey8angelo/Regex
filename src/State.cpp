#include "../headers/Regex.h"
// build state with epsilon transition
Regex::NFAState::NFAState(int i, std::unordered_map<int, Regex::NFAState*>* nfa) : ID(i), c(Regex::CharacterClass("", true)), out1(nullptr), out2(nullptr) { (*nfa)[ID] = this; }
// build state with char transition
Regex::NFAState::NFAState(char c, int i, std::unordered_map<int, Regex::NFAState*>* nfa) : ID(i), c(Regex::CharacterClass(std::string(1,c), false)), out1(nullptr), out2(nullptr) { (*nfa)[ID] = this; }
// build state with string transition (character class)
Regex::NFAState::NFAState(std::string s, int i, std::unordered_map<int, Regex::NFAState*>* nfa) : ID(i), c(Regex::CharacterClass(s, false)), out1(nullptr), out2(nullptr) { (*nfa)[ID] = this; }

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
            if(nfa[p]->c == c && nfa[p]->out1){
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