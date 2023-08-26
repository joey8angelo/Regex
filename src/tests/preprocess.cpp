#include "../../headers/includes.h"
#include "../../headers/Regex.h"

int main(){
    {
        Regex r("[abcd]{0,5}ef{0}g{3,}(a(bc)*){0,2}(abc)(def)?");
        std::cout << r.getProcessedExpression() << std::endl;
    }
    {
        Regex r("abcd[efg\\[hij\\]]*");
        std::cout << r.getProcessedExpression() << std::endl;
    }
    {
        Regex r("abcd[a\\[?+*]"); // do not need to escape special characters - except for [ and ] inside character classes
        std::cout << r.getProcessedExpression() << std::endl;
    }
    {
        Regex r("^a...s[\n\t]$"); // non-printable symbols used
    }
    {
        Regex r("abc\\\\slash"); // literal \ is '\\\\'
        std::cout << r.getProcessedExpression() << std::endl;
    }
    {
        Regex r("abcd[0-9]");
        std::cout << r.getProcessedExpression() << std::endl;
    }
    {
        Regex r("[a-gA-Z_0123]*[^abc\\-45]+$");
        std::cout << r.getProcessedExpression() << std::endl;
    }
    {
        Regex r("[\x21-\x7e]"); // special ranges are allowed
        std::cout << r.getProcessedExpression() << std::endl;
    }
    {
        Regex r("\\d{3,4}");
        std::cout << r.getProcessedExpression() << std::endl;
    }
    {
        try{
            Regex r("{3,5}");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("((ab|c)+[efg]+){}");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("a{b}");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("b|c*d{1502,-12}");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("(12345){-2,}");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("(((()())(()()))((()))");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r(")");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("[abdc]123*x[group");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("[\\s]");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("[xyz.]+");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("[z-a]");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("[z-]");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
    }
    return 0;
}