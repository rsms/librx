// Copyright (c) 2012-2014 Rasmus Andersson <http://rsms.me/> See README.md for full MIT license.
#pragma once
#include <string>
#include <ostream>
#include <set>

namespace rx {
  using Text = std::basic_string<u32>;
    // Unicode text
}

namespace std { std::string to_string(const ::rx::Text&); }
  // Convert Unicode text to a UTF8 string

namespace rx {
namespace text {

struct CharacterSet {
  template <typename... Characters> CharacterSet(Characters...);
  bool contains(u32) const;
  std::set<u32> characters;
};

extern const CharacterSet WhitespaceCharacterSet;
  // All whitespace characters as per Unicode 7 (category Zs)
extern const CharacterSet LinebreakCharacterSet;
  // All linebreak characters as per Unicode 7 (type LF, CR and BK). Note that this only includes
  // pure line-breaking characters. It does not express Unicode Line Breaking Algorithm (UAX#14.)
extern const CharacterSet ControlCharacterSet;
  // All control characters as per Unicode 7 (category Cc)
extern const CharacterSet WhitespaceLinebreakAndControlCharacterSet;
  // A combination of WhitespaceCharacterSet, LinebreakCharacterSet and ControlCharacterSet
  // commonly used when trimming strings.

Text decodeUTF8(const std::string&);
  // Convert a UTF8 string to Unicode text. See std::to_string(const Text&) for the inverse.

u32 normalize(u32);
  // Convert a Unicode point to its normalized folded value, used for case-less comparison.

std::string normalize(const std::string&);
  // Convert a UTF8 string to its normalized folded version, used for case-less comparison.

std::string ltrim(const std::string&, const CharacterSet& cs=WhitespaceLinebreakAndControlCharacterSet);
std::string rtrim(const std::string&, const CharacterSet& cs=WhitespaceLinebreakAndControlCharacterSet);
std::string trim(const std::string&, const CharacterSet& cs=WhitespaceLinebreakAndControlCharacterSet);
  // Remove leading(1), trailing(2) or both leading and trailing(3) characters from a UTF8 string.

std::string map(const std::string&, rx::func<u32(u32)>);
std::string mapF(const std::string&, u32(*fun)(u32));
  // Apply fun to each unicode point in a UTF8 string and return a UTF8 string with the result.
  // Return UINT32_MAX to stop iteration.

enum SpecialMapFunResult : u32 {
  // The following special `characters` can be returned from `fun` passed to `map` for alternate
  // behaviour. Any value outside of `_MapSpecialMin.._MapSpecialMax` is considered to represent a
  // character.
  _MapSpecialMin = 0xfffffff0,
  MapIgnore,      // Ignore the current character.
  MapIncludeAll,  // Include the current and all remaining characters and stop iteration.
  MapIncludeRest, // Ignore the current character, but include all remaining characters.
  MapIgnoreAll,   // Ignore the current and any remaining characters and stop iteration.
  MapIgnoreRest,  // Include the current character, but ignore any remaining characters.
  _MapSpecialMax
};

std::string filter(const std::string&, rx::func<bool(u32)>);
  // Apply a function to each Unicode character in a UTF8 string, and include the character in
  // the resulting UTF8 string if the function returns `true`.


// ------------------------------------------------------------------------------------------------

inline std::string normalize(const std::string& s) {
  return mapF(s, normalize);
}

template <typename... Characters>
inline CharacterSet::CharacterSet(Characters... chars)
  : characters{std::forward<u32>(std::forward<Characters>(chars))...} {}

inline bool CharacterSet::contains(u32 uc) const {
  return characters.find(uc) != characters.cend();
}

inline std::string filter(const std::string& s, rx::func<bool(u32)> f) {
  return map(s, [=](u32 c) { return f(c) ? c : MapIgnore; });
}

}} // namespace

inline std::ostream& operator<< (std::ostream& os, const ::rx::Text& v) {
  return os << std::to_string(v);
}
