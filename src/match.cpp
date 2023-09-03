#include "../headers/Regex.h"

/*
    Find the longest match from the left. NOT the longest match in the string
*/
std::pair<int, std::string> Regex::find(std::string str){
    // if the string must be matched from the start only iterate one time
    int cmp = matchStart ? 1 : str.size();
    for(int i = 0; i < cmp; i++){
        // step over the NFA, keep a set of all the states currently in, and compute the next step
        std::unordered_set<int> current;
        std::unordered_set<int> next;
        current.insert(nfaStart);
        std::unordered_set<int> temp = epsilonClosure(nfaStart);
        for(int v : temp){current.insert(v);}
        int lastAcc = -1;
        
        for(int j = i; j < str.size(); j++){
            if(current.find(nfaAcc) != current.end())
                lastAcc = j;
            for(int p : current){
                if(nfa[p]->c == str[j]){
                    next.insert(nfa[p]->out1->ID);
                    temp = epsilonClosure(nfa[p]->out1->ID);
                    for(int v : temp){next.insert(v);}
                }
            }
            // if next set contains the accept state and the string is completely used
            if(next.find(nfaAcc) != next.end() && j == str.size()-1){
                return std::make_pair(i, str.substr(i, j-i+1));
            }
            // if the next set is empty no possible paths can be taken
            // if an accept state was previously seen return from that position
            if(!next.size() && lastAcc != -1 && !matchEnd){
                return std::make_pair(i, str.substr(i, lastAcc-i));
            }
            // continue searching the string for a match if next is empty
            if(!next.size()){
                break;
            }
            current = next;
            next.clear();
        }
    }
    // check if the start state is also an accept state via epsilon
    std::unordered_set<int> t;
    t.insert(nfaStart);
    std::unordered_set<int> t1 = epsilonClosure(nfaStart);
    for(int v : t1){t.insert(v);}
    if(t.find(nfaAcc) != t.end())
        return std::make_pair(0, "");
    // if not, no match
    return std::make_pair(-1, "");
}

/*
    Returns true if there is a match in the string
*/
bool Regex::test(std::string str){
    return find(str).first == -1 ? false : true; 
}

/*
    Returns a list of positions and strings that the pattern matches from the left
    The same algorithm as find, but instead of returning matches puts them in a list and moves on
*/
std::vector<std::pair<int, std::string>> Regex::group(std::string str){
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
                lastAcc = j;
            for(int p : current){
                if(nfa[p]->c == str[j]){
                    next.insert(nfa[p]->out1->ID);
                    temp = epsilonClosure(nfa[p]->out1->ID);
                    for(int v : temp){next.insert(v);}
                }
            }
            if(next.find(nfaAcc) != next.end() && j == str.size()-1){
                res.push_back(std::make_pair(i, str.substr(i, j-i+1)));
                if(j == str.size()-1)
                    j++;
                i=j-1;
                break;
            }
            else if(!next.size() && lastAcc != -1 && !matchEnd){
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