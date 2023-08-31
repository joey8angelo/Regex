#include "../headers/Regex.h"

int id = 0;

/*
    Builds the NFA from the processed string
    sets the last state as the accept state
*/
void Regex::parse(){
    int p = 0;
    auto t = parse_(processedReg, p);
    nfaStart = t[0]->ID;
    if(t[1]->out1 == nullptr){
        t[1]->out1 = new Regex::NFAState(id++, &nfa);
        nfaAcc = t[1]->out1->ID;
    }
    else{
        t[1]->out2 = new Regex::NFAState(id++, &nfa);
        nfaAcc = t[1]->out2->ID;
    }
    id=0;
    constructDFA();
    id=0;
    //deleteNFA();
}

/*
    Recurses when capturing a group - when the current position is '('
    returns from the group on ')'
    1 lookahead performs operations like *,+,|,?
    calls parseChar on characters and character classes
    returns the start and end of the current NFA subset
*/
std::vector<Regex::NFAState*> Regex::parse_(std::string& in, int& currPos){
    // s - start of the nfa subgroup
    // e - end of the nfa subgroup
    // i - start of the current nfa
    // c - current position/end of the current nfa
    Regex::NFAState*s,*e,*i,*c;
    s=nullptr;
    e=s;i=e;c=i;
    for(;currPos < in.size(); currPos++){
        // if ( recurse, afterwards if the current nfa does not exist set it equal to the 
        // subgroup, else concat it to the current nfa 
        if(in[currPos] == '('){
            auto temp = parse_(in, ++currPos);
            s = temp[0];
            e = temp[1];
            if(c != nullptr){
                if(c->out1 == nullptr)
                    c->out1 = s;
                else
                    c->out2 = s;
                c=e;
            }
            else{
                i=s;
                c=e;
            }
        }
        // if ) return from the parse, if lookahead sees an operator apply it before returning
        else if(in[currPos] == ')'){
            // operators follow Thompson's construction
            if(currPos+1 < in.size() && in[currPos+1] == '*'){
                currPos++;
                Regex::NFAState* t1 = new Regex::NFAState(id++, &nfa);
                Regex::NFAState* t2 = new Regex::NFAState(id++, &nfa);
                if(c->out1)
                    c->out2 = t2;
                else
                    c->out1 = t2;
                t2->out1 = i;
                c=t2;
                t1->out1 = i;
                t1->out2 = t2;
                i=t1;
            }
            else if(currPos+1 < in.size() && in[currPos+1] == '+'){
                currPos++;
                Regex::NFAState* t1 = new Regex::NFAState(id++, &nfa);
                Regex::NFAState* t2 = new Regex::NFAState(id++, &nfa);
                if(c->out1)
                    c->out2 = t2;
                else
                    c->out1 = t2;
                t2->out1 = t1;
                c=t2;
                t1->out1 = i;
                i=t1;
            }
            else if(currPos+1 < in.size() && in[currPos+1] == '?'){
                currPos++;
                Regex::NFAState* t1 = new Regex::NFAState(id++, &nfa);
                Regex::NFAState* t2 = new Regex::NFAState(id++, &nfa);
                t1->out1 = i;
                t1->out2 = t2;
                i=t1;
                c->out1 = t2;
                c=t2;
            }
            else if(currPos+1 < in.size() && in[currPos+1] == '|'){
                currPos++;
                Regex::NFAState* t1 = new Regex::NFAState(id++, &nfa);
                Regex::NFAState* t2 = new Regex::NFAState(id++, &nfa);
                t1->out1 = i;
                i=t1;
                if(c->out1 == nullptr)
                    c->out1 = t2;
                else
                    c->out2 = t2;
                c=t2;
                // the | operator may have to recurse again if there is a group on the rhs
                if(currPos+1 < in.size() && in[currPos+1] == '('){
                    currPos+=2;
                    auto temp = parse_(in, currPos);
                    s=temp[0];
                    e=temp[1];
                }
                // an escaped character is treated like a normal character
                else if(currPos+1 < in.size() && in[currPos+1] == '\\'){
                    currPos++;
                    if(currPos < in.size()){
                        auto temp = parseChar(in, currPos, in[currPos]);
                        s=temp[0];
                        e=temp[1];
                    }
                    else{
                        throw std::runtime_error("Escape character does not exist at position " + std::to_string(pos[currPos]));
                    }
                }
                else if(currPos+1 < in.size()){
                    auto temp = parseChar(in, currPos, in[currPos]);
                    s=temp[0];
                    e=temp[1];
                }
                else{
                    throw std::runtime_error("No right hand side of '|' operator at position " + std::to_string(pos[currPos]));
                }
                i->out2 = s;
                if(e->out1 == nullptr)
                    e->out1 = c;
                else
                    e->out2 = c;
            }
            return {i,c};
        }
        // a character class
        else if(in[currPos] == '['){
            std::string curr = "";
            // gather characters in the character class, and parse a character like normal with the character being a string
            for(currPos++; currPos < in.size(); currPos++){
                if(in[currPos] == ']'){
                    auto temp = parseChar(in, currPos, curr);
                    s=temp[0];
                    e=temp[1];
                    if(c != nullptr){
                        if(c->out1)
                            c->out2 = s;
                        else
                            c->out1 = s;
                    }
                    else
                        i=s;
                    c=e;
                    break;
                }
                else if(in[currPos] == '\\'){
                    if(currPos+1 >= in.size())
                        throw std::runtime_error("Escape character not found at position " + std::to_string(pos[currPos]));
                    curr += in[currPos+1];
                    currPos++;
                }
                else{
                    curr += in[currPos];
                }
            }
        }
        // if an operator is seen not on a lookahead then the operator was used incorrectly
        else if(in[currPos] == '+' || in[currPos] == '*' || in[currPos] == '?' || in[currPos] == '|'){
            throw std::runtime_error("Invalid use of '" + std::string(1, in[currPos]) + "' operator at position " + std::to_string(pos[currPos]));
        }
        else{
            std::vector<Regex::NFAState*> temp;
            if(in[currPos] == '\\'){
                if(currPos+1 >= in.size())
                    throw std::runtime_error("Escape character not found at position " + std::to_string(pos[currPos]));
                currPos++;
                temp = parseChar(in, currPos, in[currPos]);
            }
            else{
                temp = parseChar(in, currPos, in[currPos]);
            }
            s=temp[0];
            e=temp[1];
            if(c != nullptr){
                if(c->out1)
                    c->out2 = s;
                else
                    c->out1 = s;
            }
            else
                i=s;
            c=e;
        }
    }
    return {i,c};
}

/*
    parseChar for a string (character class)
*/
std::vector<Regex::NFAState*> Regex::parseChar(std::string& in, int& currPos, std::string c){
    Regex::NFAState* n = new Regex::NFAState(c, id++, &nfa);
    return parseChar(in,currPos,n);
}

/*
    parseChar for a char
*/
std::vector<Regex::NFAState*> Regex::parseChar(std::string& in, int& currPos, char c){
    Regex::NFAState* n = new Regex::NFAState(c, id++, &nfa);
    return parseChar(in,currPos,n);
}

/*
    Make a state with transition symbol given from previous parseChars
    applies operations with 1 lookahead to the single character state
    returns the start and end of the current NFA subset
*/
std::vector<Regex::NFAState*> Regex::parseChar(std::string& in, int& currPos, Regex::NFAState* n){
    if(currPos+1 < in.size() && in[currPos+1] == '*'){
        currPos++;
        Regex::NFAState* t1 = new Regex::NFAState(id++, &nfa);
        n->out1 = t1;
        t1->out1 = n;
        return {t1,t1};
    }
    else if(currPos+1 < in.size() && in[currPos+1] == '+'){
        currPos++;
        Regex::NFAState* t1 = new Regex::NFAState(id++, &nfa);
        n->out1 = t1;
        t1->out1 = n;
        return {n,t1};
    }
    else if(currPos+1 < in.size() && in[currPos+1] == '?'){
        currPos++;
        Regex::NFAState* t1 = new Regex::NFAState(id++, &nfa);
        Regex::NFAState* t2 = new Regex::NFAState(id++, &nfa);
        t1->out1 = n;
        t1->out2 = t2;
        n->out1 = t2;
        return {t1, t2};
    }
    else if(currPos+1 < in.size() && in[currPos+1] == '|'){
        currPos++;
        Regex::NFAState* t1 = new Regex::NFAState(id++, &nfa);
        Regex::NFAState* t2 = new Regex::NFAState(id++, &nfa);
        std::vector<Regex::NFAState*> temp;
        if(currPos+1 < in.size() && in[currPos+1] == '('){
            currPos+=2;
            temp = parse_(in, currPos);
        }
        else if(currPos+1 < in.size() && in[currPos+1] == '\\'){
            currPos++;
            if(currPos+1 < in.size()){
                currPos++;
                temp = parseChar(in, currPos, in[currPos]);
            }
            else{
                throw std::runtime_error("Escape character does not exist at position " + std::to_string(pos[currPos]));
            }
        }
        else if(currPos+1 < in.size()){
            currPos++;
            temp = parseChar(in, currPos, in[currPos]);
        }
        else{
            throw std::runtime_error("No right hand side of '|' operator at position " + std::to_string(pos[currPos]));
        }
        t1->out1 = n;
        t1->out2 = temp[0];
        n->out1 = t2;
        temp[1]->out1 = t2;
        return {t1, t2};
    }
    return {n, n};
}

void Regex::constructDFA(){
    std::set<int> s0 = epsilonClosure(nfa[nfaStart]);
    s0.insert(nfaStart);
    dfa[s0] = new DFAState(id++);
    if(s0.find(nfaAcc) != s0.end())
        dfa[s0]->accept = true;
    dfaStart = dfa[s0];
    std::vector<std::set<int>> workList;
    workList.push_back(s0);

    while(workList.size()){
        std::set<int> q = workList[workList.size()-1];
        workList.pop_back();
        std::pair<std::unordered_set<char>,std::unordered_set<char>> ch = collectChars(q);

        makeTransitions(workList, q, ch.first, false);

        // negated character classes, currentTransitions is cleared so that in case a negated transition shares
        // a transition state, say [ab] -> {1,5,2} and [^ab] -> {1,5,2}, a new transition is made instead of
        // merging the character class into the other character class
        makeTransitions(workList, q, ch.second, true);
    }
}

/*
    Goes through characters the current set of nfa states (dfa state) can transition out on,
    collecting the states into one set. If the current character goes to a state that the current set 
    transitions to already, the character will be merged into the existing transitions CharacterClass
*/
void Regex::makeTransitions(std::vector<std::set<int>>& workList, std::set<int>& q, std::unordered_set<char>& ch, bool negated){
    std::unordered_map<int,int> currentTransitions;
    // for each character that the set can transition on
    for(char c : ch){
        std::set<int> t;
        // for each state in the set
        for(int s : q){
            // if the state does not transition on epsilon and the current character is an element of its CharacterClass
            if(!nfa[s]->c.epsilon && negated ? !(nfa[s]->c == c) : nfa[s]->c == c){
                // nfa states only transition on characters on out1
                t.insert(nfa[s]->out1->ID);
                auto temp = epsilonClosure(nfa[s]->out1);
                for(int v : temp){
                    t.insert(v);
                }
            }
        }
        Regex::DFAState* n;
        if(dfa.find(t) != dfa.end()){
            n = dfa[t];
            // if the set that was just made exists already, and the current state transitions to it
            // then we can add the current character to its CharacterClass and continue without pushing the transition
            if(currentTransitions.find(n->ID) != currentTransitions.end()){
                dfa[q]->transitions[currentTransitions[n->ID]].first.characters.insert(c);
                continue;
            }
            // if the set exists and the current state does not transition to it already then add the currentTransition
            // and move on to pushing the transition to the state
            else
                currentTransitions[n->ID] = dfa[q]->transitions.size();
        }
        else{
            n = new DFAState(id++);
            dfa[t] = n;
            if(t.find(nfaAcc) != t.end())
                dfa[t]->accept = true;
            workList.push_back(t);
            currentTransitions[n->ID] = dfa[q]->transitions.size();
        }
        dfa[q]->transitions.push_back(std::make_pair(Regex::CharacterClass(std::string(1, c), false), n));
        dfa[q]->transitions[dfa[q]->transitions.size()-1].first.negated = negated;
    }
}

/*
    Given a state compute what states are reachable through epsilon transitions
*/
std::set<int> Regex::epsilonClosure(Regex::NFAState* s){
    std::set<int> states;
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

/*
    Given a set of nfa states (a dfa state) collect the characters that the states transition out of
    collects negated and regular characters separately for when transitions are computed they are not treated the same
*/
std::pair<std::unordered_set<char>,std::unordered_set<char>> Regex::collectChars(std::set<int>& s){
    std::unordered_set<char> set;
    std::unordered_set<char> nset;
    for(int i : s){
        for(char j : nfa[i]->c.characters){
            if(nfa[i]->c.negated)
                nset.insert(j);
            else
                set.insert(j);
        }
    }
    return std::make_pair(set,nset);
}