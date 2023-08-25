#pragma once
#include "includes.h"

class Regex{
    public:
    Regex(std::string);
    ~Regex();
    std::string getProcessedExpression(){return this->processedReg;}
    private:
    
    /* 
    process the input string into a usable form to convert to an NFA, throws syntax errors
    . -> [^\n\r]
    \\w -> [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]
    \\W -> [^abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]
    \\d -> [0123456789]
    \\D -> [^0123456789]
    \\s -> [\t\n\v\f\r \xA0]
    \\S -> [^\t\n\v\f\r \xA0]
    .{a} single digit repeat . a times
    .{a,} single digit with comma repeat . a times concat .*
    .{a,b} two digits comma separated repeat . a times repeat .? a-b times 
    */
    std::string preprocess();

    /*
    Matches and returns a group/character class/single character from the end of the given string
    */
    std::string findPrevChar(std::string);
    
    std::string reg;
    std::string processedReg;
    
    // when processing escape characters or interval operators 
    // the resulting string changes length so "\d\d" -> "[0-9][0-9]""
    //                                          0 1 ->   0000011111
    // pos keeps track of this change so error messages can have more specific locations
    std::vector<int> pos; 
    
    bool matchStart;
    
    bool matchEnd;
};