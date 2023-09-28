#include "../headers/Regex.h"

/*
    Start parsing the regular expression adds a state in front
    and back to act as start and accept states
*/
void Regex::parse(){
    int p;
    if(reg.size() && reg[0] == '^')
        p = 1;
    else
        p = 0;
    auto t = parse(p);

    Regex::NFAState* s = makeEpsilonState();
    Regex::NFAState* e = makeEpsilonState();

    if(t.first){
        s->out1 = t.first;
        auto c = t.second;
        for(int v : c){
            nfa[v]->out1 = e;
        }
    }
    else
        s->out1 = e;

    nfaStart = s->ID;
    nfaAcc = e->ID;
}

/*
    Main parsing logic, given a position in the regular expression
    parse subexpressions and concatenate them to the existing expression
    or if reverse is toggled concatenate the existing expression to the next subexpression
*/
std::pair<Regex::NFAState*, std::vector<int>> Regex::parse(int& currPos){
    Regex::NFAState*i,*s;
    std::vector<int> c,e;
    i = nullptr;
    for(;currPos < reg.size(); currPos++){
        std::pair<Regex::NFAState*, std::vector<int>> temp;
        if(reg[currPos] == '('){
            currPos++;
            temp = parseGroup(currPos);
        }
        else if(reg[currPos] == '['){
            currPos++;
            temp = parseCharClass(currPos);
        }
        else if(reg[currPos] == '.'){
            temp = parseSpecial(currPos);
        }
        else if(reg[currPos] == '\\'){
            currPos++;
            temp = parseSpecial(currPos);
        }
        else if(reg[currPos] == '}')
            throw std::runtime_error("Unbalanced curly brackets at position " + std::to_string(currPos));
        else if(reg[currPos] == ')')
            throw std::runtime_error("Unbalanced parenthesis at position " + std::to_string(currPos));
        else if(reg[currPos] == ']')
            throw std::runtime_error("Unbalanced square brackets at position " + std::to_string(currPos));
        else if(reg[currPos] == '*' || reg[currPos] == '+' || reg[currPos] == '?' || reg[currPos] == '|')
            throw std::runtime_error("Invalid use of " + std::string(1, reg[currPos]) + " operator at position " + std::to_string(currPos));
        else{
            temp = parseChar(currPos);
        }

        s=temp.first;
        e=temp.second;
        if(!i){
            i=s;
            c=e;
        }
        else if(!reversed){
            for(int v : c){
                nfa[v]->out1 = s;
            }
            c=e;
        }
        else{
            for(int v : e){
                nfa[v]->out1 = i;
            }
            i=s;
        }
    }
    return {i,c};
}

/*
    Parses a group, returns when a ) is seen, allows for checking balanced parenthesis
    Uses a single lookahead to apply operators on the group
*/
std::pair<Regex::NFAState*, std::vector<int>> Regex::parseGroup(int& currPos){
    Regex::NFAState*i,*s;
    std::vector<int> c,e;
    i = nullptr;
    std::pair<Regex::NFAState*, std::vector<int>> temp;
    for(;currPos < reg.size(); currPos++){
        if(reg[currPos] == '('){
            currPos++;
            temp = parseGroup(currPos);
        }
        else if(reg[currPos] == '['){
            currPos++;
            temp = parseCharClass(currPos);
        }
        else if(reg[currPos] == '.'){
            temp = parseSpecial(currPos);
        }
        else if(reg[currPos] == '\\'){
            currPos++;
            temp = parseSpecial(currPos);
        }
        else if(reg[currPos] == '}')
            throw std::runtime_error("Unbalanced curly brackets at position " + std::to_string(currPos));
        else if(reg[currPos] == ')'){
            break;
        }
        else if(reg[currPos] == ']')
            throw std::runtime_error("Unbalanced square brackets at position " + std::to_string(currPos));
        else if(reg[currPos] == '*' || reg[currPos] == '+' || reg[currPos] == '?' || reg[currPos] == '|')
            throw std::runtime_error("Invalid use of " + std::string(1, reg[currPos]) + " operator at position " + std::to_string(currPos));
        else{
            temp = parseChar(currPos);
        }

        s=temp.first;
        e=temp.second;
        if(!i){
            i=s;
            c=e;
        }
        else if(!reversed){
            for(int v : c){
                nfa[v]->out1 = s;
            }
            c=e;
        }
        else{
            for(int v : c){
                nfa[v]->out1 = i;
            }
            i=s;
        }
    }

    if((currPos == reg.size()-1 && reg[currPos] != ')') || currPos == reg.size())
        throw std::runtime_error("Unbalanced parenthesis");

    temp = std::make_pair(i,c);
    if(reg[currPos+1] == '*'){
        currPos++;
        temp = doStar(i,c);
    }
    else if(reg[currPos+1] == '?'){
        currPos++;
        temp = doQuestion(i,c);
    }
    else if(reg[currPos+1] == '+'){
        currPos++;
        temp = doPlus(i,c);
    }
    else if(reg[currPos+1] == '|'){
        currPos+=2;
        temp = doPipe(i,c,currPos);
    }
    else if(reg[currPos+1] == '{'){
        currPos+=2;
        temp = parseInterval(i,c,currPos);
    }

    i=temp.first;
    c=temp.second;

    return {i,c};
}

/*
    Parse a character class, create a state with a list of characters to transition on
    uses a single lookahead to apply operators on the character class
*/
std::pair<Regex::NFAState*, std::vector<int>> Regex::parseCharClass(int& currPos){
    std::string t = "";
    int p = currPos-1;
    for(;currPos < reg.size(); currPos++){
        if(reg[currPos] == '[')
            throw std::runtime_error("Cannot nest character classes, try \\\\[ at position " + std::to_string(currPos));
        else if(reg[currPos] == ']'){
            break;
        }
        else if(reg[currPos] == '-'){
            char lhs;
            char rhs;
            if(!t.size())
                throw std::runtime_error("Invalid range at position " + std::to_string(currPos));
            
            lhs = t[t.size()-1];

            // rhs of range '-'
            if(currPos+1 < reg.size()){
                // if escaped char
                if(reg[currPos+1] == '\\'){
                    if(currPos+2 < reg.size()){
                        rhs = reg[currPos+2];
                        currPos+=2;
                    }
                    else
                        throw std::runtime_error("Invalid range at position " + std::to_string(currPos));
                }
                // normal char
                else{
                    rhs = reg[currPos+1];
                    currPos++;
                }
            }
            else
                throw std::runtime_error("Invalid range at position " + std::to_string(currPos));

            // lhs is already in t iterate from lhs+1 to rhs
            lhs++;
            for(;lhs<=rhs;lhs++){
                t+=lhs;
            }
        }
        else{
            if(reg[currPos] == '\\'){
                if(currPos == reg.size()-1)
                    throw std::runtime_error("No character after escape sequence at position" + std::to_string(currPos));
                currPos++;
            }
            t += reg[currPos];
        }
    }

    if((currPos == reg.size()-1 && reg[currPos] != ']') || currPos == reg.size())
        throw std::runtime_error("Unbalanced square brackets");

    Regex::NFAState* a = makeCharClassState(t);
    std::pair<Regex::NFAState*, std::vector<int>> temp = std::make_pair(a, std::vector<int>({a->ID}));
    if(reg[currPos+1] == '*'){
        currPos++;
        temp = doStar(a,temp.second);
    }
    else if(reg[currPos+1] == '?'){
        currPos++;
        temp = doQuestion(a,temp.second);
    }
    else if(reg[currPos+1] == '+'){
        currPos++;
        temp = doPlus(a,temp.second);
    }
    else if(reg[currPos+1] == '|'){
        currPos+=2;
        temp = doPipe(a,temp.second,currPos);
    }
    else if(reg[currPos+1] == '{'){
        currPos+=2;
        temp = parseInterval(a,temp.second,currPos);
    }

    return temp;
}

/*
    Parse an escape sequence, checks if the symbol following '\\' is special, if not it can be treated literally
    uses a single lookahead to apply operators on the character
*/
std::pair<Regex::NFAState*, std::vector<int>> Regex::parseSpecial(int& currPos){
    std::unordered_map<char, std::string> special = {{'.', "^\n\r"}, {'S', "^\t\n\v\f\r \xA0"}, {'s', "\t\n\v\f\r \xA0"},
                                                            {'W', "^abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"},
                                                            {'w', "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"},
                                                            {'d', "0123456789"}, {'D', "^0123456789"}};
    Regex::NFAState* a;
    if(reg[currPos] == '.' && currPos > 0 && reg[currPos-1] == '\\'){
        a = makeCharState(reg[currPos]);
    }
    else if(special.find(reg[currPos]) != special.end()){
        a = makeCharClassState(special[reg[currPos]]);
    }
    else{
        a = makeCharState(reg[currPos]);
    }

    std::pair<Regex::NFAState*, std::vector<int>> temp = std::make_pair(a, std::vector<int>({a->ID}));
    if(reg[currPos+1] == '*'){
        currPos++;
        temp = doStar(a,temp.second);
    }
    else if(reg[currPos+1] == '?'){
        currPos++;
        temp = doQuestion(a,temp.second);
    }
    else if(reg[currPos+1] == '+'){
        currPos++;
        temp = doPlus(a,temp.second);
    }
    else if(reg[currPos+1] == '|'){
        currPos+=2;
        temp = doPipe(a,temp.second,currPos);
    }
    else if(reg[currPos+1] == '{'){
        currPos+=2;
        temp = parseInterval(a,temp.second,currPos);
    }

    return temp;
}

/*
    Parse a single character, makes a state with the current character as a transition
    uses a single lookahead to apply operators on the character
*/
std::pair<Regex::NFAState*, std::vector<int>> Regex::parseChar(int& currPos){
    Regex::NFAState* a = makeCharState(reg[currPos]);
    std::pair<Regex::NFAState*, std::vector<int>> temp = std::make_pair(a, std::vector<int>({a->ID}));
    if(reg[currPos+1] == '*'){
        currPos++;
        temp = doStar(a,temp.second);
    }
    else if(reg[currPos+1] == '?'){
        currPos++;
        temp = doQuestion(a,temp.second);
    }
    else if(reg[currPos+1] == '+'){
        currPos++;
        temp = doPlus(a,temp.second);
    }
    else if(reg[currPos+1] == '|'){
        currPos+=2;
        temp = doPipe(a,temp.second,currPos);
    }
    else if(reg[currPos+1] == '{'){
        currPos+=2;
        temp = parseInterval(a,temp.second,currPos);
    }

    return temp;
}

/*
    Parses the interval operator and applies the transformation to the given nfa subsequence, e
    {l,r} -> e^l e?^r-l
    {l,} -> e^l e*
    {l} -> e^l
*/
std::pair<Regex::NFAState*, std::vector<int>> Regex::parseInterval(Regex::NFAState* a, std::vector<int> b, int& currPos){
    int* tid = &id;
    int init = currPos-1;
    int l = -1;
    int r = -1;
    bool commaFlag = false;
    std::string current = "";
    for(;currPos < reg.size(); currPos++){
        if(reg[currPos] == '}')
            break;
        if(reg[currPos] == ' ')
            throw std::runtime_error("Do not use spaces in interval operator at position " + std::to_string(currPos));
        if(!std::isdigit(reg[currPos]) && !(reg[currPos] == ','))
            throw std::runtime_error("Invalid expression in interval operator at position " + std::to_string(init));
        if(reg[currPos] == ','){
            l = std::stoi(current);
            current = "";
            commaFlag = true;
        }
        else{
            current += reg[currPos];
        }
    }

    if(currPos == reg.size()-1 && reg[currPos] != '}' || currPos == reg.size())
        throw std::runtime_error("No closing bracket for opening bracket at position " + std::to_string(init));

    if(current.size()){
        if(l == -1)
            l = std::stoi(current);
        else
            r = std::stoi(current);
    }

    if(l == -1)
        throw std::runtime_error("Invalid expression in interval operator at position " + std::to_string(init));
    if(r < l && r != -1)
        throw std::runtime_error("Invalid expression in interval operator at position " + std::to_string(init));

    if(l == 0 && !commaFlag)
        return std::make_pair(nullptr, std::vector<int>());
    else if(l == 0 && commaFlag && r == -1)
        return doStar(a,b);
    
    auto ls = makeList(a,b);
    // The list is used to make deep copies of the subsequence without having to traverse the nfa
    // every time a copy needs to be made
    Regex::NFAState*i;
    std::vector<int> c;
    
    // just l
    if(!commaFlag){
        i=a;
        c=b;
        for(int j = 1; j < l; j++){
            auto temp = copy(ls, a->ID, b);
            for(int v : c){
                nfa[v]->out1 = temp.first;
            }
            c=temp.second;
        }
    }
    // l,
    else if(commaFlag && r == -1){
        auto temp = copy(ls, a->ID, b);
        i=temp.first;
        c=temp.second;
        for(int j = 1; j < l; j++){
            temp = copy(ls, a->ID, b);
            for(int v : c){
                nfa[v]->out1 = temp.first;
            }
            c = temp.second;
        }
        temp = doStar(a,b);
        for(int v : c){
            nfa[v]->out1 = temp.first;
        }
        c = temp.second;
    }
    // l,r
    else{
        i=a;
        c=b;
        for(int j = 1; j < l; j++){
            auto temp = copy(ls, a->ID, b);
            for(int v : c){
                nfa[v]->out1 = temp.first;
            }
            c = temp.second;
        }
        for(int j = 0; j < r-l; j++){
            auto temp = copy(ls, a->ID, b);
            temp = doQuestion(temp.first, temp.second);
            for(int v : c){
                nfa[v]->out1 = temp.first;
            }
            c = temp.second;
        }
    }

    if(reg[currPos] == '|'){
        currPos++;
        auto temp = doPipe(i,c,currPos);
        i = temp.first;
        c = temp.second;
    }

    return {i,c};
}

/*
    Make a deep copy of the nfa states in the given list
*/
std::pair<Regex::NFAState*, std::vector<int>> Regex::copy(std::unordered_set<int>& ls, int s, std::vector<int> e){
    std::unordered_map<int,int> mp;
    for(int i : ls){
        auto t = nfa[i]->makeCopy();
        t->ID = id++;
    }

    for(int i : ls){
        if(nfa[i]->out1 && mp.find(nfa[i]->out1->ID) != mp.end()){
            nfa[mp[i]]->out1 = nfa[mp[nfa[i]->out1->ID]];
        }
        if(nfa[i]->out2){
            nfa[mp[i]]->out2 = nfa[mp[nfa[i]->out2->ID]];
        }
    }
    std::vector<int> re;
    for(int v : e){
        re.push_back(mp[v]);
    }
    return std::make_pair(nfa[mp[s]], re);
}

/*
    Make a list of states in a given start and end of an nfa expression
*/
std::unordered_set<int> Regex::makeList(Regex::NFAState* s, const std::vector<int>& e) const{
    std::unordered_set<int> states;
    std::vector<Regex::NFAState*> stack;
    std::unordered_set<int> eL;
    for(int v : e){
        eL.insert(v);
    }

    if(!s)
        return states;

    stack.push_back(s);
    states.insert(s->ID);

    while(stack.size()){
        Regex::NFAState* c = stack[stack.size()-1];
        stack.pop_back();
        // The end of a subexpression will always reserve out1 for later concatenation
        // if the current state is in the end list do not leave the given subexpression by
        // looking at out1
        if(c->out1 && states.find(c->out1->ID) == states.end() && eL.find(c->ID) == eL.end()){
            states.insert(c->out1->ID);
            stack.push_back(c->out1);
        }
        if(c->out2 && states.find(c->out2->ID) == states.end()){
            states.insert(c->out2->ID);
            stack.push_back(c->out2);
        }
    }
    return states;
}

/*
    Applies the * operator on the given nfa subexpression
     |------>
    >c <--|
     |--> e
*/
std::pair<Regex::NFAState*, std::vector<int>> Regex::doStar(Regex::NFAState* a, std::vector<int> b){
    Regex::NFAState* c = makeEpsilonState();
    
    c->out2 = a;
    for(int v : b){
        nfa[v]->out1 = c;
    }
    b.clear();
    b.push_back(c->ID);

    return std::make_pair(c, b);
}

/*
    Applies the ? operator on the given nfa subexpression

     |-------->
    >c
     |--> e -->
*/
std::pair<Regex::NFAState*, std::vector<int>> Regex::doQuestion(Regex::NFAState* a, std::vector<int> b){
    Regex::NFAState* c = makeEpsilonState();

    c->out2 = a;
    b.push_back(c->ID);

    return std::make_pair(c, b);
}

/*
    Applies the + operator on the given subexpression

    |------>
    c <--|
    |--> e
         ^
*/
std::pair<Regex::NFAState*, std::vector<int>> Regex::doPlus(Regex::NFAState* a, std::vector<int> b){
    Regex::NFAState* c = makeEpsilonState();

    c->out2 = a;
    for(int v : b){
        nfa[v]->out1 = c;
    }
    b.clear();
    b.push_back(c->ID);

    return std::make_pair(a, b);
}

/*
    Applies the | operator on the given subexpression, parses the right hand side of the operator

     |--> e1 -->
    >c
     |--> e2 -->
*/
std::pair<Regex::NFAState*, std::vector<int>> Regex::doPipe(Regex::NFAState* a, std::vector<int> b, int& currPos){
    std::pair<Regex::NFAState*, std::vector<int>> temp;
    if(reg[currPos] == '('){
        currPos++;
        temp = parseGroup(currPos);
    }
    else if(reg[currPos] == '['){
        currPos++;
        temp = parseCharClass(currPos);
    }
    else if(reg[currPos] == '.'){
        temp = parseSpecial(currPos);
    }
    else if(reg[currPos] == '\\'){
        currPos++;
        temp = parseSpecial(currPos);
    }
    else if(reg[currPos] == '}')
        throw std::runtime_error("Unbalanced curly brackets at position " + std::to_string(currPos));
    else if(reg[currPos] == ')')
        throw std::runtime_error("Unbalanced parenthesis at position " + std::to_string(currPos));
    else if(reg[currPos] == ']')
        throw std::runtime_error("Unbalanced square brackets at position " + std::to_string(currPos));
    else if(reg[currPos] == '*' || reg[currPos] == '+' || reg[currPos] == '?' || reg[currPos] == '|')
        throw std::runtime_error("Invalid use of " + std::string(1, reg[currPos]) + " operator at position " + std::to_string(currPos));
    else{
        temp = parseChar(currPos);
    }

    Regex::NFAState* c = makeEpsilonState();

    c->out1 = a;
    c->out2 = temp.first;

    for(int v : temp.second){
        b.push_back(v);
    }

    return {c,b};
}