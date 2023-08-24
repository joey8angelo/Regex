#pragma once
#include "includes.h"

class Regex{
    public:
    Regex::Regex(std::string);
    Regex::~Regex();
    private:
    std::string Regex::preprocess() const;
    std::string Regex::infixToPostfix(std::string);
    std::string reg;
    bool matchStart;
    bool matchEnd;
};