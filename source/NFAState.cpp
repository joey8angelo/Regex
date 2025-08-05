#include <Regex/NFA.h>

bool NFA::LiteralState::transitions_on(char32_t c) const {
  return c == literal;
}
std::shared_ptr<NFA::State> NFA::LiteralState::clone() const {
  return make_literal_state(literal);
}
void NFA::LiteralState::print() const {
  std::cout << "Literal NFAState id: " << did << std::endl;
  std::cout << "    Value : " << literal << std::endl;
  std::cout << "    Out1: " << (out1 ? std::to_string(out1->did) : "null")
            << std::endl;
}
std::shared_ptr<NFA::State> NFA::make_literal_state(char32_t c) {
  return std::make_shared<NFA::LiteralState>(c);
}

bool NFA::CharClassState::transitions_on(char32_t c) const {
  if (c == 0)
    return false;
  bool res = char_class.find(c) != char_class.end();
  return negated ? !res : res;
}
std::shared_ptr<NFA::State> NFA::CharClassState::clone() const {
  return make_char_class_state(char_class, negated);
}
void NFA::CharClassState::print() const {
  std::cout << "CharClass NFAState id: " << did << std::endl;
  std::cout << "    Characters: { ";
  for (const auto &c : char_class) {
    std::cout << c << " ";
  }
  std::cout << "}" << std::endl;
  std::cout << "    Negated: " << (negated ? "true" : "false") << std::endl;
  std::cout << "    Out1: " << (out1 ? std::to_string(out1->did) : "null")
            << std::endl;
}
std::shared_ptr<NFA::State>
NFA::make_char_class_state(std::unordered_set<char32_t> &&char_class,
                           bool negated) {
  return std::make_shared<NFA::CharClassState>(std::move(char_class), negated);
}
std::shared_ptr<NFA::State>
NFA::make_char_class_state(const std::unordered_set<char32_t> &char_class,
                           bool negated) {
  return std::make_shared<NFA::CharClassState>(char_class, negated);
}

std::shared_ptr<NFA::State> NFA::EpsilonState::clone() const {
  return make_epsilon_state();
}
void NFA::EpsilonState::print() const {
  std::cout << "Epsilon NFAState id: " << did << std::endl;
  std::cout << "    Out1: " << (out1 ? std::to_string(out1->did) : "null")
            << std::endl;
  std::cout << "    Out2: " << (out2 ? std::to_string(out2->did) : "null")
            << std::endl;
}
std::shared_ptr<NFA::State> NFA::make_epsilon_state() {
  return std::make_shared<NFA::EpsilonState>();
}

std::shared_ptr<NFA::State> NFA::make_reject_state() {
  return std::make_shared<NFA::RejectState>();
}

std::shared_ptr<NFA::State> NFA::make_wildcard_state() {
  return NFA::make_char_class_state(std::unordered_set<char32_t>{'\n', '\r'},
                                    true);
}

std::shared_ptr<NFA::State> NFA::make_space_state(bool negated) {
  return NFA::make_char_class_state({'\t', '\n', '\v', '\f', '\r', ' '},
                                    negated);
}

std::shared_ptr<NFA::State> NFA::make_word_state(bool negated) {
  return NFA::make_char_class_state(
      {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
       'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
       'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
       'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
       '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_'},
      negated);
}

std::shared_ptr<NFA::State> NFA::make_digit_state(bool negated) {
  return NFA::make_char_class_state(
      {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}, negated);
}

void NFA::RejectState::print() const {
  std::cout << "Reject NFAState id: " << did << std::endl;
}
