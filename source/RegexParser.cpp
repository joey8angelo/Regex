#include <Regex/NFA.h>
#include <Regex/RegexParser.h>
#include <cassert>

RegexParser::RegexParser(const std::u32string &pattern)
    : match_start(false), match_end(false), reversed(false), iter(pattern),
      group_depth(0) {
  if (!pattern.empty()) {
    match_start = (pattern.front() == '^');
    match_end = (pattern.back() == '$');
  }
  if (!match_start && match_end) {
    reversed = true;
    match_start = true;
    match_end = false;
  }
}

NFA RegexParser::compile_to_nfa() {
  iter.reset();
  if (iter.size() == 0) {
    return NFA(NFA::make_reject_state()).concat(NFA::make_epsilon_state());
  }
  if (match_start && !reversed) {
    iter.next();
  }
  return parse().concat(NFA::make_epsilon_state());
}

void RegexParser::do_ops(NFA &nfa) {
  switch (iter.peek()) {
  case '*':
    iter++;
    nfa.apply_star();
    break;
  case '+':
    iter++;
    nfa.apply_plus();
    break;
  case '?':
    iter++;
    nfa.apply_optional();
    break;
  case '{':
    iter++;
    nfa.apply_interval(parse_interval());
    break;
  case ')':
    break;
  }
}

NFA RegexParser::parse() { return parse_alternation(); }

NFA RegexParser::parse_alternation() {
  NFA nfa = parse_concat();
  if (nfa.empty()) {
    return nfa;
  }
  while (iter.peek() == '|') {
    iter++;
    nfa.alternate(parse_concat());
  }
  return nfa;
}

NFA RegexParser::parse_concat() {
  NFA nfa = parse_quantified();
  if (nfa.empty()) {
    return nfa;
  }
  while (iter.peek() != '|' && iter.peek() != 0) {
    NFA t = parse_quantified();
    if (t.empty()) {
      return nfa;
    }
    reversed ? nfa.prepend(std::move(t)) : nfa.concat(std::move(t));
  }
  return nfa;
}

NFA RegexParser::parse_quantified() {
  NFA nfa = parse_atom();
  if (nfa.empty()) {
    return nfa;
  }
  do_ops(nfa);
  return nfa;
}

NFA RegexParser::parse_atom() {
  char32_t c;
  if ((c = iter.next())) {
    switch (c) {
    case '(':
      return parse_group();
    case '[':
      return parse_char_class();
    case '.':
      return NFA(NFA::make_wildcard_state());
    case '\\':
      return parse_escape();
    case '}':
      throw std::runtime_error(
          "Unmatched closing brace in pattern at position " + iter.str_pos());
    case ')':
      if (group_depth) {
        return NFA();
      } else {
        throw std::runtime_error(
            "Unmatched closing parenthesis in pattern at position " +
            iter.str_pos());
      }
    case ']':
      throw std::runtime_error(
          "Unmatched closing square bracket in pattern at position " +
          iter.str_pos());
    case '*':
    case '+':
    case '?':
    case '|':
      throw std::runtime_error(
          "Quantifier or alternation operator found without preceding atom at "
          "position " +
          iter.str_pos());
    default:
      return parse_literal(c);
    }
  }
  return NFA();
}
NFA RegexParser::parse_group() {
  group_depth++;
  NFA group_nfa = parse();
  group_depth--;
  return group_nfa;
}

NFA RegexParser::parse_char_class() {
  std::unordered_set<char32_t> char_class;
  bool negated = false;
  if (iter.consume('^')) {
    negated = true;
  }
  char32_t c;
  while ((c = iter.next()) && c != ']') {
    if (c == '\\') {
      if (!(c = iter.next())) {
        throw std::runtime_error("Invalid escape sequence at end of pattern");
      }
    }
    auto next = iter.peek();
    if (next == '-') {
      iter++;
      if (!iter.has_next() || iter.peek() == ']') {
        throw std::runtime_error(
            "Invalid character range in character class at position " +
            iter.str_pos());
      }
      char32_t start = c;
      char32_t end = iter++;
      if (start > end) {
        throw std::runtime_error("Invalid character range in character class " +
                                 std::to_string(start) + "-" +
                                 std::to_string(end) + " at position " +
                                 iter.str_pos());
      }
      for (char32_t c = start; c <= end; ++c) {
        char_class.insert(c);
      }
    } else {
      char_class.insert(c);
    }
  }
  if (!(c == ']')) {
    throw std::runtime_error(
        "Unmatched opening square bracket in pattern at position " +
        iter.str_pos());
  }
  NFA nfa(NFA::make_char_class_state(std::move(char_class), negated));
  return nfa;
}

NFA RegexParser::parse_escape() {
  char32_t c;
  if (!(c = iter.next())) {
    throw std::runtime_error("Invalid escape sequence at end of pattern");
  }
  switch (c) {
  case 'S':
    return NFA(NFA::make_space_state(true));
  case 's':
    return NFA(NFA::make_space_state(false));
  case 'W':
    return NFA(NFA::make_word_state(true));
  case 'w':
    return NFA(NFA::make_word_state(false));
  case 'D':
    return NFA(NFA::make_digit_state(true));
  case 'd':
    return NFA(NFA::make_digit_state(false));
  default:
    return NFA(NFA::make_literal_state(c));
  }
}

NFA RegexParser::parse_literal(char32_t c) {
  if (!iter.has_next() && c == '$') {
    return {};
  }
  NFA nfa(NFA::make_literal_state(c));
  return nfa;
}

std::tuple<size_t, size_t, bool> RegexParser::parse_interval() {
  size_t min = parse_int();
  size_t max = min;
  bool unbounded = false;
  if (iter.consume(',')) {
    if (!iter.has_next()) {
      throw std::runtime_error(
          "Unmatched opening brace in pattern at position " + iter.str_pos());
    }
    if (iter.peek() == '}') {
      unbounded = true;
    } else {
      max = parse_int();
    }
  }

  if (!iter.consume('}')) {
    throw std::runtime_error("Unmatched opening brace in pattern at position " +
                             iter.str_pos());
  }
  if (min > max) {
    throw std::runtime_error("Invalid quantifier range {" +
                             std::to_string(min) + "," + std::to_string(max) +
                             "} at position " + iter.str_pos());
  }
  return std::make_tuple(min, max, unbounded);
}

size_t RegexParser::parse_int() {
  size_t value = 0;
  // strip leading whitespace
  while (iter.has_next() && std::isspace(iter.peek())) {
    iter++;
  }
  while (iter.has_next() && std::isdigit(iter.peek())) {
    value = value * 10 + (iter++ - '0');
  }
  if (iter.has_next() && iter.peek() != ',' && iter.peek() != '}') {
    throw std::runtime_error("Invalid quantifier at position " +
                             iter.str_pos());
  }
  // strip trailing whitespace
  while (iter.has_next() && std::isspace(iter.peek())) {
    iter++;
  }
  return value;
}
