#include <Regex/Regex.h>

bool all_failed = false;

void test(const std::u32string &pattern,
          const std::vector<std::tuple<std::u32string, bool, size_t, size_t,
                                       std::vector<std::pair<size_t, size_t>>>>
              &tests) {
  Regex re(pattern);
  bool failed = false;
  for (size_t i = 0; i < tests.size(); ++i) {
    auto [input, expected_match, expected_start, expected_end,
          expected_find_all] = tests[i];
    auto find_res = re.find(input);
    if (std::get<0>(find_res) == expected_match &&
        std::get<1>(find_res) == expected_start &&
        std::get<2>(find_res) == expected_end) {
    } else {
      if (!failed) {
        std::cout << "FAIL on pattern: "
                  << std::string(pattern.begin(), pattern.end()) << std::endl;
      }
      std::cout << "  Test " << i << " '"
                << std::string(input.begin(), input.end()) << "': Expected ("
                << expected_match << ", " << expected_start << ", "
                << expected_end << "), got (" << std::get<0>(find_res) << ", "
                << std::get<1>(find_res) << ", " << std::get<2>(find_res)
                << ")." << std::endl;
      all_failed = failed = true;
    }

    bool test_res = re.test(input);
    if (test_res != expected_match) {
      if (!failed) {
        std::cout << "FAIL on pattern: "
                  << std::string(pattern.begin(), pattern.end()) << std::endl;
      }
      std::cout << "  Test " << i << " '"
                << std::string(input.begin(), input.end()) << "': Expected "
                << expected_match << ", got " << test_res << "." << std::endl;
      all_failed = failed = true;
    }

    auto find_all_res = re.find_all(input);
    if (find_all_res == expected_find_all) {
    } else {
      if (!failed) {
        std::cout << "FAIL on pattern: "
                  << std::string(pattern.begin(), pattern.end()) << std::endl;
      }
      std::cout << "  Test " << i << " '"
                << std::string(input.begin(), input.end()) << "': Expected: {";
      for (const auto &p : expected_find_all) {
        std::cout << " (" << p.first << ", " << p.second << ")";
      }
      std::cout << " }, got: {";
      for (const auto &p : find_all_res) {
        std::cout << " (" << p.first << ", " << p.second << ")";
      }
      std::cout << " }." << std::endl;
      all_failed = failed = true;
    }
  }
  if (!failed) {
    std::cout << "PASS tests on pattern: "
              << std::string(pattern.begin(), pattern.end()) << std::endl;
  }
}

int main() {
  test(U".", {
                 {U"hello ?",
                  true,
                  0,
                  1,
                  {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}}},
                 {U"h", true, 0, 1, {{0, 1}}},
                 {U"ab", true, 0, 1, {{0, 1}, {1, 2}}},
                 {U"1234", true, 0, 1, {{0, 1}, {1, 2}, {2, 3}, {3, 4}}},
                 {U"\nabcd", true, 1, 2, {{1, 2}, {2, 3}, {3, 4}, {4, 5}}},
                 {U"\n\n", false, 0, 0, {}},
                 {U"", false, 0, 0, {}},
             });
  test(U"a*", {
                  {U"a", true, 0, 1, {{0, 1}, {1, 1}}},
                  {U"aa", true, 0, 2, {{0, 2}, {2, 2}}},
                  {U"aaa", true, 0, 3, {{0, 3}, {3, 3}}},
                  {U"xyz", true, 0, 0, {{0, 0}, {1, 1}, {2, 2}, {3, 3}}},
                  {U"", true, 0, 0, {{0, 0}}},
              });
  test(U"abc*$", {
                     {U"abc", true, 0, 3, {{0, 3}}},
                     {U"abccc0123abccc", true, 9, 14, {{9, 14}}},
                     {U"abc0", false, 0, 0, {}},
                     {U"ab", true, 0, 2, {{0, 2}}},
                     {U"0123ab", true, 4, 6, {{4, 6}}},
                 });
  test(U"abc$",
       {{U"123abc", true, 3, 6, {{3, 6}}}, {U"abc123", false, 0, 0, {}}});
  test(U"^abc$", {{U"123abc", false, 0, 0, {}},
                  {U"abc123", false, 0, 0, {}},
                  {U"abc", true, 0, 3, {{0, 3}}}});
  test(U"🨄+", {
                  {U"🨄", true, 0, 1, {{0, 1}}},
                  {U"🨄🨄", true, 0, 2, {{0, 2}}},
                  {U"🨄🨄🨄", true, 0, 3, {{0, 3}}},
                  {U"abc", false, 0, 0, {}},
                  {U"", false, 0, 0, {}},
              });
  test(U"^[^abcd]*efg", {
                            {U"efg", true, 0, 3, {{0, 3}}},
                            {U"a1efg", false, 0, 0, {}},
                            {U"12345678efg", true, 0, 11, {{0, 11}}},
                            {U"1234aefg", false, 0, 0, {}},
                            {U"eeeefg", true, 0, 6, {{0, 6}}},
                        });
  test(U"(123(abc)+|(def)*)?ghi+(((jkl)?)m)*",
       {
           {U"123abcabcabcghiiijklm", true, 0, 21, {{0, 21}}},
           {U"defdefdefghim", true, 0, 13, {{0, 13}}},
           {U"ghi", true, 0, 3, {{0, 3}}},
           {U"123abghijklm", true, 5, 12, {{5, 12}}},
           {U"ghiiim", true, 0, 6, {{0, 6}}},
           {U"", false, 0, 0, {}},
           {U"123abc", false, 0, 0, {}},
           {U"jklm", false, 0, 0, {}},
           {U"gh", false, 0, 0, {}},
       });
  test(U"(xy+z){2,4}", {{U"xyzxyyz", true, 0, 7, {{0, 7}}},
                        {U"xyzxxyzxyzxyyyz", true, 4, 15, {{4, 15}}},
                        {U"xyzxyzxyyyyzxyyzxyz", true, 0, 16, {{0, 16}}},
                        {U"xyz", false, 0, 0, {}}});
  test(U"\\d{4}", {
                      {U"1234", true, 0, 4, {{0, 4}}},
                      {U"abcd", false, 0, 0, {}},
                      {U"12345", true, 0, 4, {{0, 4}}},
                      {U"12ab34", false, 0, 0, {}},
                      {U"12ab34cd", false, 0, 0, {}},
                      {U"", false, 0, 0, {}},
                  });

  test(U"^abc",
       {{U"123abc", false, 0, 0, {}}, {U"abc123", true, 0, 3, {{0, 3}}}});

  test(U"[abcd]|1234|xy+z|89",
       {{U"axyz", true, 0, 1, {{0, 1}, {1, 4}}},
        {U"xya", true, 2, 3, {{2, 3}}},
        {U"zzxxbxxzzd", true, 4, 5, {{4, 5}, {9, 10}}},
        {U"1234yzxyz", true, 0, 4, {{0, 4}, {6, 9}}},
        {U"zz123123xy88", false, 0, 0, {}},
        {U"89", true, 0, 2, {{0, 2}}},
        {U"123123xyyyz89", true, 6, 11, {{6, 11}, {11, 13}}}});
  test(U"^(\\+\\d{1,2}\\s?)?1?\\-?\\.?\\s?\\(?\\d{3}\\)?(\\s|\\.|-)?\\d{3}(\\s|"
       "\\.|-)?\\d{4}$",
       {{U"(909) 683-1028", true, 0, 14, {{0, 14}}},
        {U"(909)683-1028", true, 0, 13, {{0, 13}}},
        {U"909-683-1028", true, 0, 12, {{0, 12}}},
        {U"909.683.1028", true, 0, 12, {{0, 12}}},
        {U"9096831028", true, 0, 10, {{0, 10}}},
        {U"+31909683102", true, 0, 12, {{0, 12}}},
        {U"+91 (909) 683-1028", true, 0, 18, {{0, 18}}},
        {U"+91 (909)683-1028", true, 0, 17, {{0, 17}}},
        {U"+91 909-683-1028", true, 0, 16, {{0, 16}}},
        {U"+91 909.683.1028", true, 0, 16, {{0, 16}}},
        {U"+91 909 683 1028", true, 0, 16, {{0, 16}}},
        {U"9009-683-1028", false, 0, 0, {}},
        {U"90968310295", false, 0, 0, {}},
        {U"909-682.123", false, 0, 0, {}}});

  test(U"x([^1234]abc)*",
       {
           {U"x_abccjkl", true, 0, 5, {{0, 5}}},
           {U"", false, 0, 0, {}},
           {U"x10abc", true, 0, 1, {{0, 1}}},
           {U"2abcxaabcdx0abc", true, 4, 9, {{4, 9}, {10, 15}}},
           {U"x34uop7abc", true, 0, 1, {{0, 1}}},
           {U"x5abc6abc7abc", true, 0, 13, {{0, 13}}},
       });
  test(U"[.^?+{}()\\[\\]]+",
       {
           {U"", false, 0, 0, {}},
           {U"abcd.", true, 4, 5, {{4, 5}}},
           {U"1234*.()", true, 5, 8, {{5, 8}}},
           {U".^?+{}()[]", true, 0, 10, {{0, 10}}},
           {U"90]", true, 2, 3, {{2, 3}}},
           {U"[ ] . ^ ?", true, 0, 1, {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}}},
           {U"abcd", false, 0, 0, {}},
       });
  test(U"ab(abc)*d", {
                         {U"", false, 0, 0, {}},
                         {U"abcd", false, 0, 0, {}},
                         {U"ababcabcabcabcd", true, 0, 15, {{0, 15}}},
                         {U"abcabcabdababcd", true, 6, 9, {{6, 9}, {9, 15}}},
                         {U"ab", false, 0, 0, {}},
                         {U"abc", false, 0, 0, {}},
                         {U"abd", true, 0, 3, {{0, 3}}},
                         {U"uqpasvbd", false, 0, 0, {}},
                     });
  return all_failed;
}
