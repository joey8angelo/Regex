#pragma once
#include <string>

class StringIter {
public:
  explicit StringIter(const std::u32string &pattern)
      : pattern(pattern), reversed(false), pos(0), sz(pattern.size()) {}

  bool has_next() const;
  char32_t next();
  char32_t prev();
  bool end() const { return pos >= sz || pos < 0; }
  char32_t peek() const;
  void reset();
  bool consume(char32_t c);
  char32_t operator++(int);
  void offset(size_t off) {
    reversed ? pos -= off : pos += off;
    if (pos >= sz) {
      pos = sz - 1;
    } else if (pos < 0) {
      pos = 0;
    }
  }
  std::string str_pos();
  void reverse();
  size_t get_pos();
  size_t size() const { return sz; }

private:
  std::u32string pattern;
  bool reversed;
  long pos;
  long sz;
};
