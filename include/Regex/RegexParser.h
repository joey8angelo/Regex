#pragma once
#include "NFA.h"
#include "StringIter.h"
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class RegexParser {
public:
  explicit RegexParser(const std::u32string &pattern);

  NFA compile_to_nfa();
  bool match_start;
  bool match_end;
  bool reversed;

private:
  StringIter iter;
  NFA parse();
  NFA parse_alternation();
  NFA parse_concat();
  NFA parse_quantified();
  NFA parse_atom();
  NFA parse_group();
  NFA parse_char_class();
  NFA parse_escape();
  NFA parse_literal(char32_t c);
  std::tuple<size_t, size_t, bool> parse_interval();
  void do_ops(NFA &nfa);
  size_t parse_int();
  size_t group_depth;
};
