#include <colorer/cregexp/cregexp.h>
#include <catch2/catch_amalgamated.hpp>

namespace {

UnicodeString ustr(const char16_t* text)
{
  return UnicodeString(text);
}

bool parse_re(CRegExp& re, const UnicodeString& text, SMatches& match, bool moves = false, int pos = 0,
              int soscheme = 0)
{
  re.setPositionMoves(moves);
  return re.parse(&text, pos, text.length(), &match, soscheme);
}

void require_match(const char16_t* pattern, const char16_t* text, int s0, int e0, bool moves = false, int pos = 0)
{
  const auto pre = ustr(pattern);
  const auto str = ustr(text);
  CRegExp re(&pre);
  INFO("pattern: " << UStr::to_stdstr(&pre) << " text: " << UStr::to_stdstr(&str));
  REQUIRE(re.isOk());
  SMatches match;
  REQUIRE(parse_re(re, str, match, moves, pos));
  REQUIRE(match.s[0] == s0);
  REQUIRE(match.e[0] == e0);
}

void require_no_match(const char16_t* pattern, const char16_t* text, bool moves = false, int pos = 0)
{
  const auto pre = ustr(pattern);
  const auto str = ustr(text);
  CRegExp re(&pre);
  INFO("pattern: " << UStr::to_stdstr(&pre) << " text: " << UStr::to_stdstr(&str));
  REQUIRE(re.isOk());
  SMatches match;
  REQUIRE_FALSE(parse_re(re, str, match, moves, pos));
}

}  // namespace

TEST_CASE("CRegExp compilation", "[cregexp]")
{
  SECTION("empty object is not ready")
  {
    CRegExp re;
    REQUIRE_FALSE(re.isOk());
    REQUIRE(re.getError() == EError::EERROR);
  }

  SECTION("valid slash-delimited pattern")
  {
    const auto pattern = ustr(u"/abc/");
    CRegExp re(&pattern);
    REQUIRE(re.isOk());
    REQUIRE(re.getError() == EError::EOK);
  }

  SECTION("empty pattern // is valid")
  {
    require_match(u"//", u"", 0, 0);
    require_match(u"//", u"abc", 0, 0);
  }

  SECTION("setRE replaces previous pattern")
  {
    const auto first = ustr(u"/abc/");
    const auto second = ustr(u"/xyz/");
    const auto text = ustr(u"xyz");
    CRegExp re(&first);
    REQUIRE(re.setRE(&second));
    SMatches match;
    REQUIRE(re.parse(&text, &match));
    REQUIRE(match.s[0] == 0);
    REQUIRE(match.e[0] == 3);
  }

  SECTION("parse of invalid regexp fails")
  {
    CRegExp re;
    const auto text = ustr(u"abc");
    SMatches match;
    REQUIRE_FALSE(re.parse(&text, &match));
  }
}

TEST_CASE("CRegExp compilation errors", "[cregexp]")
{
  SECTION("missing delimiters")
  {
    const auto [pattern, error] = GENERATE(table<const char16_t*, EError>({
        {u"", EError::EERROR},
        {u"abc", EError::ESYNTAX},
        {u"/abc", EError::ESYNTAX},
        {u"abc/", EError::ESYNTAX},
    }));
    const auto pre = ustr(pattern);
    CRegExp re(&pre);
    INFO("pattern: " << UStr::to_stdstr(&pre));
    REQUIRE_FALSE(re.isOk());
    REQUIRE(re.getError() == error);
  }

  SECTION("malformed pattern")
  {
    const auto [pattern, error] = GENERATE(table<const char16_t*, EError>({
        {u"/(abc/", EError::EBRACKETS},
        {u"/abc)/", EError::EBRACKETS},
        {u"/[abc/", EError::EENUM},
        {u"/a{/", EError::EBRACKETS},
        {u"/+/", EError::EOP},
        {u"/{2}/", EError::EOP},
        {u"/*/", EError::EOP},
        {u"/\\/", EError::ESYNTAX},
        {u"/\\y/", EError::ESYNTAX},
        {u"/\\p/", EError::ESYNTAX},
        {u"   ", EError::ESYNTAX},
    }));
    const auto pre = ustr(pattern);
    CRegExp re(&pre);
    INFO("pattern: " << UStr::to_stdstr(&pre));
    REQUIRE_FALSE(re.isOk());
    REQUIRE(re.getError() == error);
  }
}

TEST_CASE("CRegExp basic matching", "[cregexp]")
{
  const auto [pattern, text, should_match, s0, e0] = GENERATE(table<const char16_t*, const char16_t*, bool, int, int>({
      {u"/abc/", u"abc", true, 0, 3},
      {u"/abc/", u"ab", false, 0, 0},
      {u"/abc/", u"abcd", true, 0, 3},
      {u"/abc/", u"xabc", false, 0, 0},
      {u"/^abc/", u"abc", true, 0, 3},
      {u"/^abc/", u"xabc", false, 0, 0},
      {u"/abc$/", u"abc", true, 0, 3},
      {u"/abc$/", u"abcd", false, 0, 0},
      {u"/^abc$/", u"abc", true, 0, 3},
      {u"/a.c/", u"axc", true, 0, 3},
      {u"/\\d+/", u"42", true, 0, 2},
      {u"/\\D+/", u"ab", true, 0, 2},
      {u"/\\w+/", u"ab_1", true, 0, 4},
      {u"/\\s+/", u" \t", true, 0, 2},
      {u"/[abc]/", u"b", true, 0, 1},
      {u"/[^a]/", u"b", true, 0, 1},
      {u"/[^a]/", u"a", false, 0, 0},
      {u"/a*/", u"", true, 0, 0},
      {u"/a*/", u"b", true, 0, 0},
      {u"/a+/", u"", false, 0, 0},
      {u"/a+/", u"aaa", true, 0, 3},
      {u"/a?/", u"", true, 0, 0},
      {u"/a?/", u"a", true, 0, 1},
      {u"/a|b/", u"a", true, 0, 1},
      {u"/a|b/", u"b", true, 0, 1},
      {u"/ab|cd/", u"cd", true, 0, 2},
      {u"/a{2}/", u"aa", true, 0, 2},
      {u"/a{2}/", u"a", false, 0, 0},
      {u"/a{2,4}/", u"aaaaa", true, 0, 4},
      {u"/a{2,}/", u"aaa", true, 0, 3},
      {u"/\\t/", u"\t", true, 0, 1},
      {u"/\\n/", u"\n", true, 0, 1},
      {u"/\\x{41}/", u"A", true, 0, 1},
      {u"/a\\/b/", u"a/b", true, 0, 3},
  }));

  const auto pre = ustr(pattern);
  const auto str = ustr(text);
  CRegExp re(&pre);
  INFO("pattern: " << UStr::to_stdstr(&pre) << " text: " << UStr::to_stdstr(&str));
  REQUIRE(re.isOk());
  SMatches match;
  REQUIRE(re.parse(&str, &match) == should_match);
  if (should_match) {
    REQUIRE(match.s[0] == s0);
    REQUIRE(match.e[0] == e0);
  }
}

TEST_CASE("CRegExp positionMoves", "[cregexp]")
{
  SECTION("does not search forward by default")
  {
    require_no_match(u"/abc/", u"xabc");
  }

  SECTION("setPositionMoves searches inside the string")
  {
    require_match(u"/abc/", u"xabc", 1, 4, true);
  }

  SECTION("parse moves argument overrides the flag")
  {
    const auto pre = ustr(u"/abc/");
    const auto str = ustr(u"xxabc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, 0, str.length(), &match, 0, 1));
    REQUIRE(match.s[0] == 2);
    REQUIRE(match.e[0] == 5);
  }

  SECTION("parse range limits the search")
  {
    const auto pre = ustr(u"/abc/");
    const auto str = ustr(u"xxabcxx");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, 2, 5, &match));
    REQUIRE(match.s[0] == 2);
    REQUIRE(match.e[0] == 5);
    REQUIRE_FALSE(re.parse(&str, 2, 4, &match));
  }
}

TEST_CASE("CRegExp flags", "[cregexp]")
{
  SECTION("ignore case")
  {
    require_match(u"/abc/i", u"ABC", 0, 3);
    require_match(u"/abc/i", u"AbC", 0, 3);
    require_no_match(u"/abc/", u"ABC");
  }

  SECTION("dot does not match newline unless /s")
  {
    require_no_match(u"/a.c/", u"a\nc");
    require_match(u"/a.c/s", u"a\nc", 0, 3);
  }

  SECTION("extended /x ignores whitespace in pattern")
  {
    require_match(u"/a b c/x", u"abc", 0, 3);
  }
}

TEST_CASE("CRegExp greedy and lazy quantifiers", "[cregexp]")
{
  require_match(u"/\".*\"/", u"\"a\"b\"", 0, 5);
  require_match(u"/\".*?\"/", u"\"a\"b\"", 0, 3);
  require_match(u"/a+?/", u"aaa", 0, 1);
  require_match(u"/a*?b/", u"aaab", 0, 4);
  require_match(u"/a??b/", u"b", 0, 1);
  require_match(u"/a??b/", u"ab", 0, 2);
}

TEST_CASE("CRegExp groups and backreferences", "[cregexp]")
{
  SECTION("numeric groups")
  {
    const auto pre = ustr(u"/(ab)c/");
    const auto str = ustr(u"abc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.cMatch == 2);
    REQUIRE(match.s[0] == 0);
    REQUIRE(match.e[0] == 3);
    REQUIRE(match.s[1] == 0);
    REQUIRE(match.e[1] == 2);
  }

  SECTION("two capturing groups")
  {
    const auto pre = ustr(u"/(a)(b)/");
    const auto str = ustr(u"ab");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.cMatch == 3);
    REQUIRE(match.s[1] == 0);
    REQUIRE(match.e[1] == 1);
    REQUIRE(match.s[2] == 1);
    REQUIRE(match.e[2] == 2);
  }

  SECTION("non-capturing (?:) does not create a group")
  {
    const auto pre = ustr(u"/(?:ab)c/");
    const auto str = ustr(u"abc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.cMatch == 1);
    REQUIRE(match.s[0] == 0);
    REQUIRE(match.e[0] == 3);
  }

  SECTION("numeric backreference")
  {
    require_match(u"/(a)\\1/", u"aa", 0, 2);
    require_no_match(u"/(a)\\1/", u"ab");
  }
}

TEST_CASE("CRegExp named groups", "[cregexp]")
{
  SECTION("named capture and lookup")
  {
    const auto pre = ustr(u"/(?{n}ab)c/");
    const auto str = ustr(u"abc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    const auto name = ustr(u"n");
    REQUIRE(re.getBracketNo(&name) == 0);
    REQUIRE(*re.getBracketName(0) == name);
    REQUIRE(re.getBracketName(1) == nullptr);

    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.cnMatch == 1);
    REQUIRE(match.ns[0] == 0);
    REQUIRE(match.ne[0] == 2);
  }

  SECTION("named backreference \\p{name}")
  {
    require_match(u"/(?{n}a)\\p{n}/", u"aa", 0, 2);
    require_no_match(u"/(?{n}a)\\p{n}/", u"ab");
  }

  SECTION("empty name (?{}) is non-capturing")
  {
    const auto pre = ustr(u"/(?{}ab)c/");
    const auto str = ustr(u"abc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.cnMatch == 0);
  }

  SECTION("bracket names are matched case-insensitively")
  {
    const auto pre = ustr(u"/(?{Foo}x)/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    const auto name = ustr(u"foo");
    REQUIRE(re.getBracketNo(&name) == 0);
  }

  SECTION("unknown name")
  {
    const auto pre = ustr(u"/(?{n}x)/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    const auto name = ustr(u"missing");
    REQUIRE(re.getBracketNo(&name) == -1);
  }
}

TEST_CASE("CRegExp lookahead and lookbehind", "[cregexp]")
{
  SECTION("positive lookahead pattern?=")
  {
    require_match(u"/abc(def)?=/", u"abcdef", 0, 3);
    require_no_match(u"/abc(def)?=/", u"abcxyz");
  }

  SECTION("negative lookahead pattern?!")
  {
    require_match(u"/abc(x)?!/", u"abc", 0, 3);
    require_no_match(u"/abc(x)?!/", u"abcx");
  }

  SECTION("positive lookbehind pattern?#N")
  {
    require_match(u"/(abc)?#3xyz/", u"abcxyz", 3, 6, false, 3);
    require_no_match(u"/(abc)?#3xyz/", u"abcxyz");
  }

  SECTION("negative lookbehind pattern?~N")
  {
    require_match(u"/(z)?~1abc/", u"xabc", 1, 4, false, 1);
    require_no_match(u"/(x)?~1abc/", u"xabc", false, 1);
  }
}

TEST_CASE("CRegExp word boundaries", "[cregexp]")
{
  require_match(u"/\\babc\\b/", u"abc", 0, 3);
  require_no_match(u"/\\babc\\b/", u"abcd");
  require_no_match(u"/\\babc\\b/", u"xabc");
  require_match(u"/\\babc\\b/", u" abc ", 1, 4, true);
  require_match(u"/\\Babc/", u"xabc", 1, 4, true);
}

TEST_CASE("CRegExp unicode character classes", "[cregexp]")
{
  require_match(u"/[{Nd}]+/", u"123", 0, 3);
  require_match(u"/[{L}]+/", u"Hello", 0, 5);
  require_no_match(u"/[{Nd}]+/", u"abc");
}

TEST_CASE("CRegExp Colorer \\m \\M and scheme start", "[cregexp]")
{
  SECTION("\\m and \\M change group 0 bounds")
  {
    const auto pre = ustr(u"/x\\mabc\\M/");
    const auto str = ustr(u"xabc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.s[0] == 1);
    REQUIRE(match.e[0] == 4);
  }

  SECTION("~ matches scheme start")
  {
    const auto pre = ustr(u"/~abc/");
    const auto str = ustr(u"xxabc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, 2, str.length(), &match, 2));
    REQUIRE(match.s[0] == 2);
    REQUIRE(match.e[0] == 5);
    REQUIRE_FALSE(re.parse(&str, 0, str.length(), &match, 2));
  }
}

TEST_CASE("CRegExp Colorer backtrace \\y", "[cregexp]")
{
  SECTION("numeric \\yN copies another regexp group")
  {
    const auto start_re = ustr(u"/(foo)/");
    const auto text = ustr(u"foobar");
    CRegExp start(&start_re);
    REQUIRE(start.isOk());
    SMatches start_match;
    REQUIRE(start.parse(&text, &start_match));

    const auto end_re = ustr(u"/\\y1bar/");
    CRegExp end;
    end.setBackTrace(&text, &start_match);
    REQUIRE(end.setRE(&end_re));
    SMatches end_match;
    REQUIRE(end.parse(&text, &end_match));
    REQUIRE(end_match.s[0] == 0);
    REQUIRE(end_match.e[0] == 6);
  }

  SECTION("named \\y{name} copies a named group")
  {
    const auto start_re = ustr(u"/(?{n}foo)/");
    const auto start_text = ustr(u"foo");
    CRegExp start(&start_re);
    REQUIRE(start.isOk());
    SMatches start_match;
    REQUIRE(start.parse(&start_text, &start_match));

    const auto end_re = ustr(u"/\\y{n}bar/");
    const auto end_text = ustr(u"foobar");
    CRegExp end;
    REQUIRE(end.setBackRE(&start));
    end.setBackTrace(&start_text, &start_match);
    REQUIRE(end.setRE(&end_re));
    SMatches end_match;
    REQUIRE(end.parse(&end_text, &end_match));
    REQUIRE(end_match.s[0] == 0);
    REQUIRE(end_match.e[0] == 6);
  }

  SECTION("named backtrace without setBackRE fails to compile")
  {
    const auto end_re = ustr(u"/\\y{n}/");
    CRegExp end;
    REQUIRE_FALSE(end.setRE(&end_re));
    REQUIRE(end.getError() == EError::EERROR);
  }
}

TEST_CASE("CRegExp canStartWith", "[cregexp]")
{
  SECTION("literal")
  {
    const auto pre = ustr(u"/abc/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    REQUIRE(re.canStartWith('a'));
    REQUIRE_FALSE(re.canStartWith('b'));
    REQUIRE_FALSE(re.canStartWith('x'));
  }

  SECTION("ignore case")
  {
    const auto pre = ustr(u"/abc/i");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    REQUIRE(re.canStartWith('a'));
    REQUIRE(re.canStartWith('A'));
  }

  SECTION("alternation and class")
  {
    const auto alt = ustr(u"/a|x/");
    CRegExp re_alt(&alt);
    REQUIRE(re_alt.isOk());
    REQUIRE(re_alt.canStartWith('a'));
    REQUIRE(re_alt.canStartWith('x'));
    REQUIRE_FALSE(re_alt.canStartWith('b'));

    const auto cls = ustr(u"/[xyz]/");
    CRegExp re_cls(&cls);
    REQUIRE(re_cls.isOk());
    REQUIRE(re_cls.canStartWith('y'));
    REQUIRE_FALSE(re_cls.canStartWith('a'));
  }
}

TEST_CASE("CRegExp stack reuse after clearRegExpStack", "[cregexp]")
{
  const auto pre = ustr(u"/(a|b)+c/");
  const auto str = ustr(u"aaabbc");
  CRegExp re(&pre);
  REQUIRE(re.isOk());
  SMatches match;
  REQUIRE(re.parse(&str, &match));
  CRegExp::clearRegExpStack();
  REQUIRE(re.parse(&str, &match));
  REQUIRE(match.s[0] == 0);
  REQUIRE(match.e[0] == 6);
}

TEST_CASE("CRegExp \\Y{name} copies named group case-insensitively", "[cregexp]")
{
  const auto start_re = ustr(u"/(x)(?{n}Foo)/");
  const auto start_text = ustr(u"xFoo");
  CRegExp start(&start_re);
  REQUIRE(start.isOk());
  SMatches start_match;
  REQUIRE(start.parse(&start_text, &start_match));

  const auto end_re = ustr(u"/\\Y{n}/");
  const auto end_text = ustr(u"foo");
  CRegExp end;
  REQUIRE(end.setBackRE(&start));
  end.setBackTrace(&start_text, &start_match);
  REQUIRE(end.setRE(&end_re));
  SMatches end_match;
  REQUIRE(end.parse(&end_text, &end_match));
  REQUIRE(end_match.s[0] == 0);
  REQUIRE(end_match.e[0] == 3);
}
