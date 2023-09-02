#include "../../headers/Regex.h"

int main(){
    {
        Regex r("[.^?+{}()\\[\\]]?");
    }
    {
        Regex r("ab(abc)*d");
    }
    {
        Regex r("abc*");
    }
    {
        Regex r("[abcd]*efg");
    }
    {
        Regex r("(123(abc)|(def)*)?ghi+(((jkl)?)m)*");
    }
    {
        Regex r("(\\d{3}) (\\d{2})");
    }
    {
        Regex r("(((((a)*)*)*)*)*");
    }
    {
        Regex r("(((((a)+)+)+)+)+");
    }
    {
        Regex r("(((((a)?)?)?)?)?");
    }
    {
        Regex r("abcde([0-4]*)|(hej*(xy)\\w?)");
    }
    {
        Regex r("abcde([0-4]*)|(hej*(xy)\\W?)");
    }
    {
        Regex r("[a-z][A-Z]*\\+.{4}");
    }
    {
        Regex r("([^1234]abc)*");
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