#include <Regex/StringIter.h>

bool StringIter::has_next() const { return reversed ? pos > 0 : pos < sz - 1; }

char32_t StringIter::operator++(int) { return next(); }
char32_t StringIter::next() {
  if (pos >= sz || pos < 0)
    return 0;
  return reversed ? pattern[pos--] : pattern[pos++];
}
char32_t StringIter::peek() const {
  if (pos > 0 && pos < sz) {
    return pattern[pos];
  } else {
    return 0;
  }
}
char32_t StringIter::prev() {
  if (pos >= sz || pos < 0)
    return 0;
  return reversed ? pattern[pos++] : pattern[pos--];
}
void StringIter::reset() { reversed ? pos = sz - 1 : pos = 0; }

bool StringIter::consume(char32_t c) { return (peek() == c) ? next() : false; }

std::string StringIter::str_pos() { return std::to_string(get_pos()); }

size_t StringIter::get_pos() { return reversed ? sz - pos - 1 : pos; }

void StringIter::reverse() {
  reversed = !reversed;
  reset();
}
