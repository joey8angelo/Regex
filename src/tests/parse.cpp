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
    try{
        Regex r("abcd{}");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    try{
        Regex r("(hello world){6,5}");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    try{
        Regex r("(132){2}*");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    try{
        Regex r("[8305[90]]");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    try{
        Regex r("[-a]");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    try{
        Regex r("(12)*)(ab)");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    try{
        Regex r("[1234*ba+c");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    try{
        Regex r("((((((abcd)ef)+)g)");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    try{
        Regex r("(abc2,3})");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }try{
        Regex r("[a-]");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    try{
        Regex r("[a-");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    try{
        Regex r("a{123,a6}");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    try{
        Regex r("a{1 2 3, 6 7}");
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    return 0;
}