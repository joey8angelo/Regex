#include "../headers/Regex.h"

int num = -100;
/*
    Find the longest match from the left. NOT the longest match in the string
    If we are searching the back of the string because of a $
    Then j becomes str.size()-j-1 to match from the end
    Builds and caches dfa states as needed
*/
std::pair<int, std::string> Regex::find(const std::string& str){
    Regex::DFAState* currentState;

    buildDFAStart();

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
            
            currentState = nextDFA(str[reversed ? str.size()-j-1 : j], currentState);

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
*/
bool Regex::test(const std::string& str){
    Regex::DFAState* currentState;

    buildDFAStart();

    // if the string is empty see if we can match on epsilon
    if(!str.size()){
        if(dfaStart->accept)
            true;
        else
            return false;
    }

    if(dfaStart->accept)
        return true;

    // iterate only once through the string if matchStart
    int cmp = matchStart ? 1 : str.size();
    for(int i = 0; i < cmp; i++){
        currentState = dfaStart;
        int lastAcc = -1;

        for(int j = i; j < str.size(); j++){
            if(currentState->accept && !matchEnd)
                return true;
            
            currentState = nextDFA(str[reversed ? str.size()-j-1 : j], currentState);

            // if next set contains the accept state and the string is completely used
            if(currentState->accept && j == str.size()-1){
                return true;
            }
            // continue searching the string for a match if currentState is reject
            if(currentState == reject){
                break;
            }
        }
    }
    return false;
}

/*
    Returns a list of positions and strings that the pattern matches from the left
    The same algorithm as find, but instead of returning matches puts them in a list and moves on
*/
std::vector<std::pair<int, std::string>> Regex::group(const std::string& str){
    Regex::DFAState* currentState;
    std::vector<std::pair<int, std::string>> res;

    buildDFAStart();

    // if the string is empty see if we can match on epsilon
    if(!str.size()){
        if(dfaStart->accept)
            res.push_back(std::make_pair(0, ""));
        return res;
    }

    // iterate only once through the string if matchStart
    int cmp = matchStart ? 1 : str.size();
    for(int i = 0; i < cmp; i++){
        currentState = dfaStart;
        int lastAcc = -1;

        for(int j = i; j < str.size(); j++){
            if(currentState->accept)
                lastAcc = reversed ? str.size()-j : j;

            currentState = nextDFA(str[reversed ? str.size()-j-1 : j], currentState);

            // if next set contains the accept state and the string is completely used
            if(currentState->accept && j == str.size()-1){
                if(reversed)
                    res.push_back(std::make_pair(str.size()-j-1, str.substr(str.size()-j-1, str.size()-(str.size()-j-1))));
                else
                    res.push_back(std::make_pair(i, str.substr(i, j-i+1)));
                if(j == str.size()-1)
                    j++;
                i=j-1;
                break;
            }
            // if the currentState is in reject no possible paths can be taken
            // if an accept state was previously seen return from that position
            if(currentState == reject && lastAcc != -1 && !matchEnd){
                if(reversed)
                    res.push_back(std::make_pair(lastAcc, str.substr(lastAcc, str.size()-lastAcc)));
                else
                    res.push_back(std::make_pair(i, str.substr(i, lastAcc-i)));
                if(j == str.size()-1)
                    j++;
                i=j-1;
                break;
            }
            // continue searching the string for a match if currentState is reject
            if(currentState == reject){
                break;
            }
        }
    }
    return res;
}