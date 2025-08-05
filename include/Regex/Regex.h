#pragma once
#include "NFA.h"
#include "RegexParser.h"
#include "StringIter.h"
#include <string>
#include <vector>

class Regex {
public:
  explicit Regex(const std::u32string &pattern);
  explicit Regex(const char *pattern);
  explicit Regex(const std::string &pattern);
  std::tuple<bool, size_t, size_t> find_at(const std::u32string &text,
                                           size_t p = 0);
  std::tuple<bool, size_t, size_t> find_at(const char *text, size_t p = 0);
  std::tuple<bool, size_t, size_t> find_at(const std::string &text,
                                           size_t p = 0);

  std::tuple<bool, size_t, size_t> find(const std::u32string &text);
  std::tuple<bool, size_t, size_t> find(const char *text);
  std::tuple<bool, size_t, size_t> find(const std::string &text);

  std::vector<std::pair<size_t, size_t>> find_all(const std::u32string &text);
  std::vector<std::pair<size_t, size_t>> find_all(const char *text);
  std::vector<std::pair<size_t, size_t>> find_all(const std::string &text);
  bool test(const std::u32string &text);
  bool test(const char *text);
  bool test(const std::string &text);

  void print_nfa() const { nfa.print(); }

private:
  NFA nfa;
  NFA aug_nfa;
  std::unique_ptr<NFA::Iterator> nfa_iter;
  std::unique_ptr<NFA::Iterator> aug_nfa_iter;
  bool match_start;
  bool match_end;
  bool reversed;
  std::tuple<bool, size_t, size_t> make_tuple(bool match_found,
                                              size_t start_index,
                                              size_t end_index,
                                              size_t iter_size) const;
};
