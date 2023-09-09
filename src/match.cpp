#include "../headers/Regex.h"

/*
    Find the longest match from the left. NOT the longest match in the string
    If we are searching the back of the string because of a $
    Then j becomes str.size()-j-1 to match from the end
    Builds and caches dfa states as needed
*/
std::pair<int, std::string> Regex::find(const std::string& str){
    Regex::DFAState* currentState;

    // make the initial DFA state if it does not exist
    if(!dfaStart){
        std::set<int> c;
        c.insert(nfaStart);
        auto t = epsilonClosure(nfaStart);
        std::unordered_set<int> temp = epsilonClosure(nfaStart);
        for(int v : temp){c.insert(v);}
        dfaStart = new DFAState(c, c.find(nfaAcc) != c.end());
        if(dfa.size() == CACHELIMIT){
            deleteDFA();
        }
        dfa[c] = dfaStart;
    }

    // if the string is empty see if we can match on epsilon
    if(!str.size()){
        if(dfaStart->accept)
            return std::make_pair(0, "");
        else
            return std::make_pair(-1, "");
    }

    // iterate only once through the string if matchStart
    int cmp = matchStart ? 1 : str.size();
    for(int i = 0; i < cmp; i++){
        currentState = dfaStart;
        int lastAcc = -1;

        for(int j = i; j < str.size(); j++){
            if(currentState->accept)
                lastAcc = reversed ? str.size()-j : j;

            // check if currentState[str[pos]] does not exist
            if(currentState->next(str[reversed ? str.size()-j-1 : j]) == nullptr){
                // compute the next set of nfa states from the currentStates set
                std::set<int> t;
                for(auto p : currentState->ls){
                    if(nfa[p]->c == str[reversed ? str.size()-j-1 : j] && nfa[p]->out1){
                        t.insert(nfa[p]->out1->ID);
                        std::unordered_set<int> temp = epsilonClosure(nfa[p]->out1->ID);
                        for(int v : temp){t.insert(v);}
                    }
                }
                // if the next set is empty this state transitions to reject on that character
                if(!t.size()){
                    currentState->out[str[reversed ? str.size()-j-1 : j]] = reject;
                }
                // if the next set does not exist as a dfa state create one and transition to it
                else if(dfa.find(t) == dfa.end()){
                    if(dfa.size() == CACHELIMIT){
                        deleteDFA();
                        dfa[std::set<int>({-1})] = dfaStart;
                    }
                    dfa[t] = new Regex::DFAState(t, t.find(nfaAcc) != t.end());
                    currentState->out[str[reversed ? str.size()-j-1 : j]] = dfa[t];
                }
                // transition to the dfa state corresponding to the set of nfa states
                else{
                    currentState->out[str[reversed ? str.size()-j-1 : j]] = dfa[t];
                }
            }
            currentState = currentState->next(str[reversed ? str.size()-j-1 : j]);

            // if next set contains the accept state and the string is completely used
            if(currentState->accept && j == str.size()-1){
                if(reversed)
                    return std::make_pair(str.size()-j-1, str.substr(str.size()-j-1, str.size()-(str.size()-j-1)));
                return std::make_pair(i, str.substr(i, j-i+1));
            }
            // if the currentState is in reject no possible paths can be taken
            // if an accept state was previously seen return from that position
            if(currentState == reject && lastAcc != -1 && !matchEnd){
                if(reversed)
                    return std::make_pair(lastAcc, str.substr(lastAcc, str.size()-lastAcc));
                return std::make_pair(i, str.substr(i, lastAcc-i));
            }
            // continue searching the string for a match if currentState is reject
            if(currentState == reject){
                break;
            }
        }
    }
    return std::make_pair(-1, "");
}

/*
    Returns true if any sequence of characters leads to an accept state
    Augments the nfa with .* at the start/end to nondeterministically find the accept state
    if its not necessary to match from the start/end of the string
*/
bool Regex::test(const std::string& str){
    int augStart;
    int augAcc;
    if(matchStart){
        augStart = nfaStart;
    }
    else{
        Regex::NFAState* augS = new Regex::NFAState(-1, &nfa);
        Regex::NFAState* a = new Regex::NFAState("^\n\r", -2, &nfa);
        Regex::NFAState* b = new Regex::NFAState(-3, &nfa);

        augS->out1 = a;
        augS->out2 = b;
        a->out1 = b;
        b->out1 = nfa[nfaStart];
        b->out2 = augS;
        augStart = -1;
    }

    if(matchEnd){
        augAcc = nfaAcc;
    }
    else{
        Regex::NFAState* a = new Regex::NFAState(-4, &nfa);
        Regex::NFAState* b = new Regex::NFAState("^\n\r", -5, &nfa);
        Regex::NFAState* augA = new Regex::NFAState(-6, &nfa);

        nfa[nfaAcc]->out1 = a;
        a->out1 = b;
        a->out2 = augA;
        b->out1 = augA;
        augA->out2 = b;
        augAcc = -6;
    }
    // step over the NFA, keep a set of all the states currently in, and compute the next step
    std::unordered_set<int> current;
    std::unordered_set<int> next;
    current.insert(augStart);
    std::unordered_set<int> temp = epsilonClosure(augStart);
    for(int v : temp){current.insert(v);}
        
    for(int j = 0; j < str.size(); j++){
        if(current.find(augAcc) != current.end() && !matchEnd){
            if(augStart != nfaStart){
                delete nfa[-1];
                nfa.erase(-1);
            }
            if(augAcc != nfaAcc){
                nfa[nfaAcc]->out1 = nullptr;
                delete nfa[-2];
                nfa.erase(-2);
            }
            return true;
        }
        for(int p : current){
            if(nfa.at(p)->c == str[reversed ? str.size()-j-1 : j] && nfa.at(p)->out1){
                next.insert(nfa.at(p)->out1->ID);
                temp = epsilonClosure(nfa.at(p)->out1->ID);
                for(int v : temp){next.insert(v);}
            }
        }
        // if next set contains the accept state and the string is completely used
        if(next.find(augAcc) != next.end() && (j == str.size()-1 || !matchEnd)){
            if(augStart != nfaStart){
                delete nfa[-1];
                nfa.erase(-1);
            }
            if(augAcc != nfaAcc){
                nfa[nfaAcc]->out1 = nullptr;
                delete nfa[-2];
                nfa.erase(-2);
            }
            return true;
        }
        // continue searching the string for a match if next is empty
        if(!next.size()){
            break;
        }
        current = next;
        next.clear();
    }
    // check if the start state is also an accept state via epsilon
    std::unordered_set<int> t;
    t.insert(augStart);
    std::unordered_set<int> t1 = epsilonClosure(augStart);
    for(int v : t1){t.insert(v);}
    if(t.find(nfaAcc) != t.end()){
        if(augStart != nfaStart){
            delete nfa[-1];
            nfa.erase(-1);
        }
        if(augAcc != nfaAcc){
            nfa[nfaAcc]->out1 = nullptr;
            delete nfa[-2];
            nfa.erase(-2);
        }
        return true;
    }
    
    if(augStart != nfaStart){
        delete nfa[-1];
        nfa.erase(-1);
    }
    if(augAcc != nfaAcc){
        nfa[nfaAcc]->out1 = nullptr;
        delete nfa[-2];
        nfa.erase(-2);
    }
    return false;
}

/*
    Returns a list of positions and strings that the pattern matches from the left
    The same algorithm as find, but instead of returning matches puts them in a list and moves on
*/
std::vector<std::pair<int, std::string>> Regex::group(const std::string& str) const{
    std::vector<std::pair<int, std::string>> res;
    int cmp = matchStart ? 1 : str.size();
    for(int i = 0; i < cmp; i++){
        std::unordered_set<int> current;
        std::unordered_set<int> next;
        current.insert(nfaStart);
        std::unordered_set<int> temp = epsilonClosure(nfaStart);
        for(int v : temp){current.insert(v);}
        int lastAcc = -1;
        
        for(int j = i; j < str.size(); j++){
            if(current.find(nfaAcc) != current.end())
                lastAcc = reversed ? str.size()-j : j;
            for(int p : current){
                if(nfa.at(p)->c == str[reversed ? str.size()-j-1 : j] && nfa.at(p)->out1){
                    next.insert(nfa.at(p)->out1->ID);
                    temp = epsilonClosure(nfa.at(p)->out1->ID);
                    for(int v : temp){next.insert(v);}
                }
            }
            if(next.find(nfaAcc) != next.end() && j == str.size()-1){
                if(reversed)
                    res.push_back(std::make_pair(str.size()-j-1, str.substr(str.size()-j-1, str.size()-(str.size()-j-1))));
                else
                    res.push_back(std::make_pair(i, str.substr(i, j-i+1)));
                if(j == str.size()-1)
                    j++;
                i=j-1;
                break;
            }
            else if(!next.size() && lastAcc != -1 && !matchEnd){
                if(reversed)
                    res.push_back(std::make_pair(lastAcc, str.substr(lastAcc, str.size()-lastAcc)));
                else
                    res.push_back(std::make_pair(i, str.substr(i, lastAcc-i)));
                if(j == str.size()-1)
                    j++;
                i=j-1;
                break;
            }
            else if(!next.size()){
                break;
            }
            current = next;
            next.clear();
        }
    }
    std::unordered_set<int> t;
    t.insert(nfaStart);
    std::unordered_set<int> t1 = epsilonClosure(nfaStart);
    for(int v : t1){t.insert(v);}
    if(t.find(nfaAcc) != t.end())
        res.push_back(std::make_pair(0, ""));
    return res;
}