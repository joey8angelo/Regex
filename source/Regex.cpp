#include <Regex/Regex.h>

Regex::Regex(const std::u32string &pattern) {
  auto rp = RegexParser(pattern);
  nfa = rp.compile_to_nfa();
  nfa_iter = std::make_unique<NFA::Iterator>(nfa);
  match_start = rp.match_start;
  match_end = rp.match_end;
  reversed = rp.reversed;
  aug_nfa = nfa.clone();
  if (!match_start) {
    aug_nfa.prepend(NFA(NFA::make_char_class_state({}, true)).apply_star());
  }
  if (!match_end) {
    aug_nfa.concat(NFA(NFA::make_char_class_state({}, true)).apply_star());
  }
  aug_nfa_iter = std::make_unique<NFA::Iterator>(aug_nfa);
}

Regex::Regex(const char *pattern)
    : Regex(std::u32string(reinterpret_cast<const char32_t *>(pattern))) {}

Regex::Regex(const std::string &pattern)
    : Regex(
          std::u32string(reinterpret_cast<const char32_t *>(pattern.c_str()))) {
}

/* Helper function to create a tuple with adjusted
 * indices based on reversed flag
 */
std::tuple<bool, size_t, size_t> Regex::make_tuple(bool match_found,
                                                   size_t start_index,
                                                   size_t end_index,
                                                   size_t size) const {
  return std::make_tuple(match_found,
                         reversed ? (size + 1) - end_index : start_index - 1,
                         reversed ? (size + 1) - start_index : end_index - 1);
}

/* Find the first match of the regex in the given text at a specific position.
 * Returns a tuple of (match_found, start_index, end_index).
 */
std::tuple<bool, size_t, size_t> Regex::find_at(const std::u32string &text,
                                                size_t p) {
  StringIter iter(text);
  if (reversed)
    iter.reverse();

  iter.offset(p);
  nfa_iter->reset();
  // 1 indexed position of last match start and end, non inclusive
  size_t last_accept_start = 0;
  size_t last_accept_end = 0;

  // check if start position is accepting
  if (nfa_iter->is_accept() && (!match_end || iter.end())) {
    last_accept_start = p + 1;
    last_accept_end = p + 1;
  }

  // continue iterating through text and transitioning on the characters
  char32_t c;
  while ((c = iter.next())) {
    nfa_iter->next_on(c);

    if (nfa_iter->is_accept() && (!match_end || iter.end())) {
      last_accept_start = p + 1;
      last_accept_end = iter.get_pos() + 1;
    }

    if (nfa_iter->is_reject()) {
      // if we had a previous match, return it if we dont
      // have to match from the end
      if (last_accept_start > 0 && !match_end) {
        return make_tuple(true, last_accept_start, last_accept_end,
                          text.size());
      }
      break;
    }
  }
  if (last_accept_start > 0) {
    return make_tuple(true, last_accept_start, last_accept_end, text.size());
  }
  return std::make_tuple(false, 0, 0);
}

std::tuple<bool, size_t, size_t> Regex::find_at(const char *text, size_t p) {
  return find_at(std::u32string(reinterpret_cast<const char32_t *>(text)), p);
}

std::tuple<bool, size_t, size_t> Regex::find_at(const std::string &text,
                                                size_t p) {
  return find_at(
      std::u32string(reinterpret_cast<const char32_t *>(text.c_str())), p);
}

/* Find the first match of the regex in the given text.
 * Returns a tuple of (match_found, start_index, end_index).
 */
std::tuple<bool, size_t, size_t> Regex::find(const std::u32string &text) {
  nfa_iter->reset();

  if (text.size() == 0) {
    return nfa_iter->is_accept() ? std::make_tuple(true, 0, 0)
                                 : std::make_tuple(false, 0, 0);
  }

  for (size_t i = 0; i < (match_start ? 1 : text.size()); i++) {
    auto res = find_at(text, i);
    if (std::get<0>(res)) {
      return res;
    }
  }
  return std::make_tuple(false, 0, 0);
}

std::tuple<bool, size_t, size_t> Regex::find(const char *text) {
  return find(std::u32string(reinterpret_cast<const char32_t *>(text)));
}
std::tuple<bool, size_t, size_t> Regex::find(const std::string &text) {
  return find(std::u32string(reinterpret_cast<const char32_t *>(text.c_str())));
}

/* Find all matches of the regex in the given text.
 * Returns a vector of pairs of (start_index, end_index).
 */
std::vector<std::pair<size_t, size_t>>
Regex::find_all(const std::u32string &text) {
  std::vector<std::pair<size_t, size_t>> matches;
  nfa_iter->reset();
  for (size_t i = 0; i < (match_start ? 1 : text.size()); i++) {
    auto res = find_at(text, i);
    if (std::get<0>(res)) {
      matches.emplace_back(std::get<1>(res), std::get<2>(res));
      if (std::get<1>(res) != std::get<2>(res)) {
        i = std::get<2>(res) - 1;
      }
    }
  }
  nfa_iter->reset();
  if (nfa_iter->is_accept() && !match_start) {
    matches.emplace_back(text.size(), text.size());
  }
  return matches;
}
std::vector<std::pair<size_t, size_t>> Regex::find_all(const char *text) {
  return find_all(std::u32string(reinterpret_cast<const char32_t *>(text)));
}
std::vector<std::pair<size_t, size_t>>
Regex::find_all(const std::string &text) {
  return find_all(
      std::u32string(reinterpret_cast<const char32_t *>(text.c_str())));
}

/* Return true if a match exists anywhere in the text, false otherwise.
 */
bool Regex::test(const std::u32string &text) {
  aug_nfa_iter->reset();
  if (nfa_iter->is_accept()) {
    return true;
  }
  StringIter iter(text);
  if (reversed)
    iter.reverse();

  char32_t c;
  while ((c = iter.next())) {
    aug_nfa_iter->next_on(c);
    if (aug_nfa_iter->is_accept() && (!match_end || iter.end())) {
      return true;
    }
    if (aug_nfa_iter->is_reject()) {
      return false;
    }
  }
  return false;
}
bool Regex::test(const char *text) {
  return test(std::u32string(reinterpret_cast<const char32_t *>(text)));
}
bool Regex::test(const std::string &text) {
  return test(std::u32string(reinterpret_cast<const char32_t *>(text.c_str())));
}
