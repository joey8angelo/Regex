#include "../headers/Regex.h"

int id = 0;

void Regex::parse(){
    int p;
    if(reg.size() && reg[0] == '^')
        p = 1;
    else
        p = 0;
    auto t = parse(p);

    Regex::NFAState* s = new Regex::NFAState(id++, &nfa);
    Regex::NFAState* e = new Regex::NFAState(id++, &nfa);

    if(t[0])
        s->out1 = t[0];
    else
        s->out1 = e;

    if(t[1])
        t[1]->out1 = e;

    nfaStart = s->ID;
    nfaAcc = e->ID;

    printNFAStates();
    id=0;
}

std::vector<Regex::NFAState*> Regex::parse(int& currPos){
    Regex::NFAState*i,*c,*s,*e;
    i = nullptr;
    c = nullptr;
    for(;currPos < reg.size(); currPos++){
        std::vector<Regex::NFAState*> temp;
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

        s=temp[0];
        e=temp[1];
        if(!i){
            i=s;
            c=e;
        }
        else if(!reversed){
            c->out1 = s;
            c=e;
        }
        else{
            e->out1 = i;
            i=s;
        }
    }
    return {i,c};
}
std::vector<Regex::NFAState*> Regex::parseGroup(int& currPos){
    Regex::NFAState*i,*c,*s,*e;
    i = nullptr;
    c = nullptr;
    std::vector<Regex::NFAState*> temp;
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

        s=temp[0];
        e=temp[1];
        if(!i){
            i=s;
            c=e;
        }
        else if(!reversed){
            c->out1 = s;
            c=e;
        }
        else{
            e->out1 = i;
            i=s;
        }
    }

    if((currPos == reg.size()-1 && reg[currPos] != ')') || currPos == reg.size())
        throw std::runtime_error("Unbalanced parenthesis");

    temp.clear();
    temp.push_back(i);
    temp.push_back(c);
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

    i=temp[0];
    c=temp[1];

    return {i,c};
}
std::vector<Regex::NFAState*> Regex::parseCharClass(int& currPos){
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

    Regex::NFAState* a = new Regex::NFAState(t, id++, &nfa);
    std::vector<NFAState*> temp = {a,a};
    if(reg[currPos+1] == '*'){
        currPos++;
        temp = doStar(a,a);
    }
    else if(reg[currPos+1] == '?'){
        currPos++;
        temp = doQuestion(a,a);
    }
    else if(reg[currPos+1] == '+'){
        currPos++;
        temp = doPlus(a,a);
    }
    else if(reg[currPos+1] == '|'){
        currPos+=2;
        temp = doPipe(a,a,currPos);
    }
    else if(reg[currPos+1] == '{'){
        currPos+=2;
        temp = parseInterval(a,a,currPos);
    }

    return {temp[0],temp[1]};
    
}
std::vector<Regex::NFAState*> Regex::parseSpecial(int& currPos){
    std::unordered_map<char, std::string> special = {{'.', "^\n\r"}, {'S', "^\t\n\v\f\r \xA0"}, {'s', "\t\n\v\f\r \xA0"},
                                                            {'W', "^abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"},
                                                            {'w', "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"},
                                                            {'d', "0123456789"}, {'D', "^0123456789"}};
    Regex::NFAState* a;
    if(reg[currPos] == '.' && currPos > 0 && reg[currPos-1] == '\\'){
        a = new Regex::NFAState(reg[currPos], id++, &nfa);
    }
    else if(special.find(reg[currPos]) != special.end()){
        a = new Regex::NFAState(special[reg[currPos]], id++, &nfa);
    }
    else{
        a = new Regex::NFAState(reg[currPos], id++, &nfa);
    }

    std::vector<NFAState*> temp = {a,a};
    if(reg[currPos+1] == '*'){
        currPos++;
        temp = doStar(a,a);
    }
    else if(reg[currPos+1] == '?'){
        currPos++;
        temp = doQuestion(a,a);
    }
    else if(reg[currPos+1] == '+'){
        currPos++;
        temp = doPlus(a,a);
    }
    else if(reg[currPos+1] == '|'){
        currPos+=2;
        temp = doPipe(a,a,currPos);
    }
    else if(reg[currPos+1] == '{'){
        currPos+=2;
        temp = parseInterval(a,a,currPos);
    }

    return {temp[0],temp[1]};
}
std::vector<Regex::NFAState*> Regex::parseChar(int& currPos){
    Regex::NFAState* a = new Regex::NFAState(reg[currPos], id++, &nfa);
    std::vector<NFAState*> temp = {a,a};
    if(reg[currPos+1] == '*'){
        currPos++;
        temp = doStar(a,a);
    }
    else if(reg[currPos+1] == '?'){
        currPos++;
        temp = doQuestion(a,a);
    }
    else if(reg[currPos+1] == '+'){
        currPos++;
        temp = doPlus(a,a);
    }
    else if(reg[currPos+1] == '|'){
        currPos+=2;
        temp = doPipe(a,a,currPos);
    }
    else if(reg[currPos+1] == '{'){
        currPos+=2;
        temp = parseInterval(a,a,currPos);
    }

    return {temp[0],temp[1]};
}

std::vector<Regex::NFAState*> Regex::parseInterval(Regex::NFAState* a, Regex::NFAState* b, int& currPos){
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
        return {nullptr, nullptr};
    else if(l == 0 && commaFlag && r == -1)
        return doStar(a,b);
    
    auto ls = makeList(a,b);
    // The list is used to make deep copies of the subsequence without having to traverse the nfa
    // every time a copy needs to be made
    Regex::NFAState*i,*c;
    
    // just l
    if(!commaFlag){
        i=a;
        c=b;
        for(int j = 1; j < l; j++){
            auto temp = copy(ls, a->ID, b->ID);
            c->out1 = temp[0];
            c=temp[1];
        }
    }
    // l,
    else if(commaFlag && r == -1){
        auto temp = copy(ls, a->ID, b->ID);
        i=temp[0];
        c=temp[1];
        for(int j = 1; j < l; j++){
            temp = copy(ls, a->ID, b->ID);
            c->out1 = temp[0];
            c=temp[1];
        }
        temp = doStar(a,b);
        c->out1 = temp[0];
        c = temp[1];
    }
    // l,r
    else{
        i=a;
        c=b;
        for(int j = 1; j < l; j++){
            auto temp = copy(ls, a->ID, b->ID);
            c->out1 = temp[0];
            c = temp[1];
        }
        for(int j = 0; j < r-l; j++){
            auto temp = copy(ls, a->ID, b->ID);
            temp = doQuestion(temp[0], temp[1]);
            c->out1 = temp[0];
            c = temp[1];
        }
    }

    if(reg[currPos] == '|'){
        currPos++;
        auto temp = doPipe(i,c,currPos);
        i = temp[0];
        c = temp[1];
    }

    return {i,c};
}

std::vector<Regex::NFAState*> Regex::copy(std::unordered_set<int>& ls, int s, int e){
    std::unordered_map<int,int> mp;
    for(int i : ls){
        std::string cc = nfa[i]->c.stringify();
        mp[i] = id;
        if(!cc.size())
            auto t = new Regex::NFAState(id++, &nfa);
        else
            auto t = new Regex::NFAState(cc, id++, &nfa);
    }

    for(int i : ls){
        if(nfa[i]->out1 && mp.find(nfa[i]->out1->ID) != mp.end()){
            nfa[mp[i]]->out1 = nfa[mp[nfa[i]->out1->ID]];
        }
        if(nfa[i]->out2){
            nfa[mp[i]]->out2 = nfa[mp[nfa[i]->out2->ID]];
        }
    }
    return {nfa[mp[s]], nfa[mp[e]]};
}

std::unordered_set<int> Regex::makeList(Regex::NFAState* s, Regex::NFAState* e){
    std::unordered_set<int> states;
    std::vector<Regex::NFAState*> stack;

    if(!s)
        return states;

    stack.push_back(s);
    states.insert(s->ID);

    while(stack.size()){
        Regex::NFAState* c = stack[stack.size()-1];
        stack.pop_back();
        // do not look at out one if e is the current state
        // no operator *,|,+,? will use out1 for the final node
        // this means out2 may point back into the current subsequence,
        // and out1 will always point either to nothing, out outside
        // the current subsequence
        if(c->out1 && states.find(c->out1->ID) == states.end() && c->ID != e->ID){
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

std::vector<Regex::NFAState*> Regex::doStar(Regex::NFAState* a, Regex::NFAState* b){
    Regex::NFAState*c,*d;
    c = new Regex::NFAState(id++, &nfa);
    d = new Regex::NFAState(id++, &nfa);
    c->out1 = a;
    c->out2 = d;
    b->out1 = d;
    d->out2 = a;
    return {c,d};
}
std::vector<Regex::NFAState*> Regex::doQuestion(Regex::NFAState* a, Regex::NFAState* b){
    Regex::NFAState*c,*d;
    c = new Regex::NFAState(id++, &nfa);
    d = new Regex::NFAState(id++, &nfa);
    c->out1 = a;
    c->out2 = d;
    b->out1 = d;
    return {c,d};
}
std::vector<Regex::NFAState*> Regex::doPlus(Regex::NFAState* a, Regex::NFAState* b){
    Regex::NFAState*c,*d;
    c = new Regex::NFAState(id++, &nfa);
    d = new Regex::NFAState(id++, &nfa);
    c->out1 = a;
    b->out1 = d;
    d->out2 = a;
    return {c,d};
}
std::vector<Regex::NFAState*> Regex::doPipe(Regex::NFAState* a, Regex::NFAState* b, int& currPos){
    std::vector<Regex::NFAState*> temp;
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

    Regex::NFAState*c,*d;
    c = new Regex::NFAState(id++, &nfa);
    d = new Regex::NFAState(id++, &nfa);

    c->out1 = a;
    c->out2 = temp[0];
    b->out1 = d;
    temp[1]->out1 = d;

    return {c,d};
}