#include "../../headers/Regex.h"

int main(){
    {
        Regex r("[.^?+{}()\\[\\]]?");
        r.printNFAStates();
    }
    {
        Regex r("ab(abc)*d");
        r.printNFAStates();
    }
    {
        Regex r("abc*");
        r.printNFAStates();
    }
    {
        Regex r("[abcd]*efg");
        r.printNFAStates();
    }
    {
        Regex r("(123(abc)|(def)*)?ghi+(((jkl)?)m)*");
        r.printNFAStates();
    }
    {
        Regex r("(\\d{3}) (\\d{2})");
        r.printNFAStates();
    }
    {
        Regex r("(((((a)*)*)*)*)*");
        r.printNFAStates();
    }
    {
        Regex r("(((((a)+)+)+)+)+");
        r.printNFAStates();
    }
    {
        Regex r("(((((a)?)?)?)?)?");
        r.printNFAStates();
    }
    {
        Regex r("abcde([0-4]*)|(hej*(xy)\\w?)");
        r.printNFAStates();
    }
    {
        Regex r("abcde([0-4]*)|(hej*(xy)\\W?)");
        r.printNFAStates();
    }
    {
        Regex r("[a-z][A-Z]*\\+.{4}");
        r.printNFAStates();
    }
    {
        Regex r("([^1234]abc)*");
        r.printNFAStates();
    }
    {
        try{
            Regex r("a++");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("?");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        try{
            Regex r("ab+??");
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
    }
    return 0;
}