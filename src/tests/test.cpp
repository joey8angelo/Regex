#include "../../headers/Regex.h"
#include <chrono>

bool verbose = false;

/*
    reg : regular expression
    cases : vector of vector of strings,
        {{string to test, expected result of find, expected result of group}, {...}}
        expected result of find is "integer string"
        expected result of group is "integer,string,integer,string,...,"
*/
void test(std::string reg, std::vector<std::vector<std::string>> cases){
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    {
        start = std::chrono::high_resolution_clock::now();
        Regex r(reg);
        end = std::chrono::high_resolution_clock::now();
        if(verbose){
            std::cout << "time to build nfa: ";
            std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
            std::cout << reg << std::endl;
        }
    }
    for(int i = 0; i < cases.size(); i++){
        bool pass = true;
        if(verbose)
            std::cout << "  TEST " << cases[i][0] << std::endl;
        {
            Regex r(reg);
            start = std::chrono::high_resolution_clock::now();
            std::pair<int, std::string> find = r.find(cases[i][0]);
            end = std::chrono::high_resolution_clock::now();
            if(verbose)
                std::cout << "    find  - ";
            std::string fres = std::to_string(find.first) + " " + find.second;
            if(cases[i][1] != fres){
                if(verbose){
                    std::cout << "FAIL - " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
                    std::cout << "    result   : " << find.first << " " << find.second << std::endl;
                    std::cout << "    expected : " << cases[i][1] << std::endl;
                }
                pass = false;
            }
            else{
                if(verbose)
                    std::cout << "PASS - " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
            }
        }
        {
            Regex r(reg);
            if(verbose)
                std::cout << "    group - ";
            std::string gres = "";
            start = std::chrono::high_resolution_clock::now();
            std::vector<std::pair<int, std::string>> group = r.group(cases[i][0]);
            end = std::chrono::high_resolution_clock::now();
            for(auto te : group){gres+=std::to_string(te.first);gres+=",";gres+=te.second;gres+=",";}
            if(cases[i][2] != gres){
                if(verbose){
                    std::cout << "FAIL - " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
                    std::cout << "    result   : " << gres << std::endl;
                    std::cout << "    expected : " << cases[i][2] << std::endl;
                }
                pass = false;
            }
            else{
                if(verbose)
                    std::cout << "PASS - " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
            }
        }
        {
            Regex r(reg);
            if(verbose)
                std::cout << "    test  - ";
            start = std::chrono::high_resolution_clock::now();
            bool test = r.test(cases[i][0]);
            end = std::chrono::high_resolution_clock::now();
            if(cases[i][3] != (test ? "1" : "0")){
                if(verbose){
                    std::cout << "FAIL - " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
                    std::cout << "    result   : " << test << std::endl;
                    std::cout << "    expected : " << cases[i][3] << std::endl;
                }
                pass = false;
            }
            else{
                if(verbose)
                    std::cout << "PASS - " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
            }
        }
        if(!verbose && !pass)
            std::cout << "TEST " << cases[i][0] << " FAILED" << std::endl;
        else
            std::cout << std::endl;
    }
}

std::vector<int> fn;
std::vector<int> ts;

void time(std::string r, std::string p){
    Regex re(r);

    auto start = std::chrono::high_resolution_clock::now();
    re.test(p);
    auto end = std::chrono::high_resolution_clock::now();

    fn.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());

}

int main(){
    int mode;
    std::cout << "modes" << std::endl << "    1:test speed matching pathological input" << std::endl << "    2:run test cases" << std::endl << ": ";
    std::cin >> mode;
    if(mode == 1){
        for(int i = 1; i < 100; i++){
            std::string r = "";
            std::string p = "";
            for(int j = 0; j < i; j++){
                r += "a?";
            }
            for(int j = 0; j < i; j++){
                r += "a";
                p += "a";
            }
            time(r, p);
        }
        for(int i = 0; i < fn.size(); i++){
            std::cout << i+1 << "," << fn[i] << std::endl;
        }
        std::cout << std::endl << std::endl << std::endl;
        for(int i = 0; i < ts.size(); i++){
            std::cout << i+1 << "," << ts[i] << std::endl;
        }
    }
    else if(mode == 2){
        std::cout << "verbose? (y/n): ";
        char v;
        std::cin >> v;
        verbose = v == 'y';
        test(".", {{"hello ?", "0 h", "0,h,1,e,2,l,3,l,4,o,5, ,6,?,", "1"}, {"h", "0 h", "0,h,", "1"}});
        test("([abcd]|(1234)|xyz|(89))", {{"ayz", "0 ayz", "0,ayz,", "1"},
                                        {"by89", "0 by89", "0,by89,", "1"},
                                        {"1234yzxyz", "0 1234yz", "0,1234yz,6,xyz,", "1"},
                                        {"xy89", "0 xy89", "0,xy89,", "1"}});

        test("^(((25[0-5])|(2[0-4][0-9])|([01]?[0-9][0-9]?))\\.){3}((25[0-5])|(2[0-4][0-9])|([01]?[0-9][0-9]?))$", {{"192.168.1.1", "0 192.168.1.1", "0,192.168.1.1,", "1"},
                                                                                                        {"255.255.255.255", "0 255.255.255.255", "0,255.255.255.255,", "1"},
                                                                                                        {"0.0.0.0", "0 0.0.0.0", "0,0.0.0.0,", "1"},
                                                                                                        {"1.2.3.4", "0 1.2.3.4", "0,1.2.3.4,", "1"},
                                                                                                        {"207.132.68.68", "0 207.132.68.68", "0,207.132.68.68,", "1"},
                                                                                                        {"1.1.1.1.", "-1 ", "", "0"},
                                                                                                        {".0.0.0.0", "-1 ", "", "0"}});
        test("^(\\+\\d{1,2}\\s?)?1?\\-?\\.?\\s?\\(?\\d{3}\\)?(\\s|\\.|-)?\\d{3}(\\s|\\.|-)?\\d{4}$", {{"(909) 683-1028", "0 (909) 683-1028", "0,(909) 683-1028,", "1"},
                                                                    {"(909)683-1028", "0 (909)683-1028", "0,(909)683-1028,", "1"},
                                                                    {"909-683-1028", "0 909-683-1028", "0,909-683-1028,", "1"},
                                                                    {"909.683.1028", "0 909.683.1028", "0,909.683.1028,", "1"},
                                                                    {"9096831028", "0 9096831028", "0,9096831028,", "1"},
                                                                    {"+31909683102", "0 +31909683102", "0,+31909683102,", "1"},
                                                                    {"+91 (909) 683-1028", "0 +91 (909) 683-1028", "0,+91 (909) 683-1028,", "1"},
                                                                    {"+91 (909)683-1028", "0 +91 (909)683-1028", "0,+91 (909)683-1028,", "1"},
                                                                    {"+91 909-683-1028", "0 +91 909-683-1028", "0,+91 909-683-1028,", "1"},
                                                                    {"+91 909.683.1028", "0 +91 909.683.1028", "0,+91 909.683.1028,", "1"},
                                                                    {"+91 909 683 1028", "0 +91 909 683 1028", "0,+91 909 683 1028,", "1"},
                                                                    {"9009-683-1028", "-1 ", "", "0"},
                                                                    {"90968310295", "-1 ", "", "0"},
                                                                    {"909-682.123", "-1 ", "", "0"}});

        test("x([^1234]abc)*", {{"x_abccjkl", "0 x_abc", "0,x_abc,", "1"}, 
                                {"", "-1 ", "", "0"}, 
                                {"x10abc", "0 x", "0,x,", "1"}, 
                                {"2abcxaabcdx0abc", "4 xaabc", "4,xaabc,10,x0abc,", "1"},
                                {"x34uop7abc", "0 x", "0,x,", "1"}});
        
        test("[.^?+{}()\\[\\]]+", {{"", "-1 ", "", "0"}, 
                                {"abcd.", "4 .", "4,.,", "1"}, 
                                {"1234*.()", "5 .()", "5,.(),", "1"},
                                {".^?+{}()[]", "0 .^?+{}()[]", "0,.^?+{}()[],", "1"}, 
                                {"90]", "2 ]", "2,],", "1"},
                                {"[ ] . ^ ?", "0 [", "0,[,2,],4,.,6,^,8,?,", "1"}, 
                                {"abcd", "-1 ", "", "0"}});
        
        test("ab(abc)*d", {{"", "-1 ", "", "0"}, 
                        {"abcd", "-1 ", "", "0"}, 
                        {"ababcabcabcabcd", "0 ababcabcabcabcd", "0,ababcabcabcabcd,", "1"},
                        {"abcabcabdababcd", "6 abd", "6,abd,9,ababcd,", "1"}, 
                        {"ab", "-1 ", "", "0"}, 
                        {"abc", "-1 ", "", "0"},
                        {"abd", "0 abd", "0,abd,", "1"}, 
                        {"uqpasvbd", "-1 ", "", "0"}});
        
        test("abc*$", {{"abc", "0 abc", "0,abc,", "1"}, 
                    {"abccc0123abccc", "9 abccc", "9,abccc,", "1"}, 
                    {"abc0", "-1 ", "", "0"},
                    {"ab", "0 ab", "0,ab,", "1"}, 
                    {"0123ab", "4 ab", "4,ab,", "1"}});

        test("^[^abcd]*efg", {{"efg", "0 efg", "0,efg,", "1"}, 
                            {"a1efg", "-1 ", "", "0"}, 
                            {"12345678efg", "0 12345678efg", "0,12345678efg,", "1"},
                            {"1234aefg", "-1 ", "", "0"}, 
                            {"eeeefg", "0 eeeefg", "0,eeeefg,", "1"}});

        test("(123(abc)|(def)*)?ghi+(((jkl)?)m)*", {{"123ghi", "0 123ghi", "0,123ghi,", "1"},
                                                    {"abcd 345 123ghi ghijklm", "9 123ghi", "9,123ghi,16,ghijklm,", "1"},
                                                    {"123ghihij123ghighi", "0 123ghi", "0,123ghi,9,123ghi,15,ghi,", "1"},
                                                    {"ghiiimjkljklm", "0 ghiiim", "0,ghiiim,", "1"},
                                                    {"ghiiimjklmjklm", "0 ghiiimjklmjklm", "0,ghiiimjklmjklm,", "1"},
                                                    {"", "-1 ", "", "0"},
                                                    {"123", "-1 ", "", "0"},
                                                    {"jkl", "-1 ", "", "0"},
                                                    {"def", "-1 ", "", "0"},
                                                    {"123defdefdefghiim", "0 123defdefdefghiim", "0,123defdefdefghiim,", "1"},
                                                    {"ghighighighi", "0 ghi", "0,ghi,3,ghi,6,ghi,9,ghi,", "1"},
                                                    {"m12ghi", "3 ghi", "3,ghi,", "1"}});

        test("^(123(abc)|(def)*)?ghi+(((jkl)?)m)*$", {{"123ghi", "0 123ghi", "0,123ghi,", "1"},
                                                    {"abcd 345 123ghi ghijklm", "-1 ", "", "0"},
                                                    {"123ghihij123ghighi", "-1 ", "", "0"},
                                                    {"ghiiimjkljklm", "-1 ", "", "0"},
                                                    {"ghiiimjklmjklm", "0 ghiiimjklmjklm", "0,ghiiimjklmjklm,", "1"},
                                                    {"", "-1 ", "", "0"},
                                                    {"123", "-1 ", "", "0"},
                                                    {"jkl", "-1 ", "", "0"},
                                                    {"def", "-1 ", "", "0"},
                                                    {"123defdefdefghiim", "0 123defdefdefghiim", "0,123defdefdefghiim,", "1"},
                                                    {"ghighighighi", "-1 ", "", "0"},
                                                    {"m12ghi", "-1 ", "", "0"}});
    }
    return 0;
}