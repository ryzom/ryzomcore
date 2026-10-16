// Tests for CViewText::getFormatTagLength / getFormatTagPrefixAt.
//
// These two functions are what keeps chat text colour correct once a message is
// split into several CViewTexts (around an emoji or a URL). The previous attempt
// at emoji in chat guessed the colour state with a regex and broke /em, /shout
// and every inline @{RGBA}, so this logic gets tested rather than eyeballed.
//
// It links the real libnelgui, so it exercises the shipped code, not a copy.
// Built and run by run_format_tag_tests.sh.

#include <cstdio>
#include <string>
#include <vector>

#include "nel/gui/view_text.h"

using NLGUI::CViewText;

static int g_failed = 0;
static int g_passed = 0;

static void expectLen(const std::string &text, uint index, uint want,
                      const char *what)
{
	uint got = CViewText::getFormatTagLength(text, index);
	if (got == want) { ++g_passed; return; }
	++g_failed;
	printf("  FAIL %-46s getFormatTagLength(%s, %u) = %u, want %u\n",
	       what, text.c_str(), index, got, want);
}

static void expectPrefix(const std::string &text, uint pos, const std::string &want,
                         const char *what)
{
	std::string got = CViewText::getFormatTagPrefixAt(text, pos);
	if (got == want) { ++g_passed; return; }
	++g_failed;
	printf("  FAIL %-46s getFormatTagPrefixAt(\"%s\", %u) = \"%s\", want \"%s\"\n",
	       what, text.c_str(), pos, got.c_str(), want.c_str());
}

int main()
{
	printf("CViewText format tag helpers\n");

	// ---- getFormatTagLength ----
	printf("getFormatTagLength:\n");
	expectLen("@{F00F}hi", 0, 7, "colour tag");
	expectLen("@{FFFF}", 0, 7, "colour tag, whole string");
	expectLen("@{T4}x", 0, 5, "tab tag, one digit");
	expectLen("@{T123}x", 0, 7, "tab tag, three digits");
	expectLen("@{Hhello}x", 0, 9, "tooltip tag");          // @ { H h e l l o }
	expectLen("@{H}x", 0, 4, "empty tooltip tag (clears)"); // @ { H }
	expectLen("plain", 0, 0, "no tag");
	expectLen("hi @{F00F}", 3, 7, "tag not at index 0");
	expectLen("hi @{F00F}", 0, 0, "index before the tag");
	expectLen("@{F00F}", 99, 0, "index past the end");
	// "@{ " is NOT a tag: it is the separator in translated chat lines
	// ("{:enHello}@{ Bonjour"), and mis-parsing it would eat the translation.
	expectLen("@{ Bonjour", 0, 0, "@{ + space is not a tag");
	expectLen("@{", 0, 0, "truncated");
	expectLen("@{Hunterminated", 0, 0, "tooltip with no closing brace");

	// ---- getFormatTagPrefixAt ----
	printf("getFormatTagPrefixAt:\n");
	// untagged text must return "" so the caller keeps the plain setText path
	// and the message still gets its channel colour
	expectPrefix("hello world", 5, "", "untagged text");
	expectPrefix("", 0, "", "empty string");

	// a tagged string always yields a prefix, even before the first tag,
	// because the caller renders every piece through setTextFormatTaged which
	// starts from white
	expectPrefix("ab@{F00F}cd", 0, "@{FFFF}", "tagged, but pos before any tag");
	expectPrefix("ab@{F00F}cd", 2, "@{FFFF}", "pos exactly at the tag");
	expectPrefix("ab@{F00F}cd", 9, "@{F00F}", "pos after the tag");
	expectPrefix("ab@{F00F}cd", 11, "@{F00F}", "pos at end of string");

	// last colour wins
	expectPrefix("@{F00F}a@{0F0F}b", 15, "@{0F0F}", "second colour wins");
	expectPrefix("@{F00F}a@{0F0F}b", 8, "@{F00F}", "between two colours");

	// tooltips are carried, tabs are not
	expectPrefix("@{Htip}abc", 7, "@{FFFF}@{Htip}", "tooltip carried, default white");
	expectPrefix("@{F00F}@{Htip}abc", 14, "@{F00F}@{Htip}", "colour and tooltip");
	expectPrefix("@{T4}abc", 5, "@{FFFF}", "tab tag not carried");
	expectPrefix("@{F00F}@{T4}abc", 12, "@{F00F}", "tab dropped, colour kept");
	expectPrefix("@{Htip}a@{H}b", 12, "@{FFFF}@{H}", "empty tooltip clears");

	// the real shapes this exists for
	expectPrefix("@{F00F}red :smile: still red", 18, "@{F00F}",
	             "emoji split keeps colour");
	expectPrefix("@{F00F}red http://x.com more", 15, "@{F00F}",
	             "url split keeps colour");
	// the translated-chat separator must not be mistaken for a tag
	expectPrefix("{:enHello}@{ Bonjour", 12, "", "translation separator is not a tag");

	// pos clamped rather than reading out of bounds
	expectPrefix("@{F00F}x", 1000, "@{F00F}", "pos past end is clamped");

	printf("\n%d passed, %d failed\n", g_passed, g_failed);
	return g_failed == 0 ? 0 : 1;
}
