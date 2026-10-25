// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <gtest/gtest.h>

#include <string>

#include <nel/misc/path.h>

using NLMISC::CFile;
using std::string;

namespace {

// isSafeFilename(filename, allowSubdirs) == expected
void expectSafe(const char *name, bool expected, const string &filename, bool allowSubdirs = false)
{
	SCOPED_TRACE(name);
	EXPECT_EQ(expected, CFile::isSafeFilename(filename, allowSubdirs));
}

// normalizeRelativePath(input) == expected
void expectNormalized(const char *name, const string &expected, const string &input)
{
	SCOPED_TRACE(name);
	EXPECT_EQ(expected, CFile::normalizeRelativePath(input));
}

// the intended pairing: normalize what the caller gave, then validate it as a relative path
void expectCombined(const char *name, bool expected, const string &input)
{
	SCOPED_TRACE(name);
	EXPECT_EQ(expected, CFile::isSafeFilename(CFile::normalizeRelativePath(input), true));
}

} // namespace

TEST(CUTMiscPathSafety, basicValid)
{
	expectSafe("simple ascii",            true,  "hello.txt");
	expectSafe("no extension",            true,  "readme");
	expectSafe("multiple dots",           true,  "archive.tar.gz");
	expectSafe("leading dot (hidden)",    true,  ".gitignore");
	expectSafe("unicode filename",        true,  "\xc3\xa9\xc3\xa0\xc3\xbc.txt"); // éàü.txt
	expectSafe("precomposed accent",      true,  "caf\xc3\xa9");                   // café (U+00E9)
	expectSafe("combining accent",        true,  "cafe\xcc\x81");                  // café (e + U+0301)
	expectSafe("japanese hiragana",       true,  "\xe3\x81\x82\xe3\x81\x84\xe3\x81\x86.txt"); // あいう.txt
	expectSafe("korean hangul",           true,  "\xed\x95\x9c\xea\xb8\x80.txt"); // 한글.txt
	expectSafe("japanese filename",       true,  "\xe6\x97\xa5\xe6\x9c\xac\xe5\xbc\x8f\xe3\x83\x95\xe3\x82\xa1\xe3\x82\xa4\xe3\x83\xab\xe3\x81\xae\xe8\x8a\xb8\xe8\xa1\x93.bin"); // 日本式ファイルの芸術.bin
	expectSafe("emoji (4-byte)",          true,  "\xf0\x9f\x98\x80.txt");
	expectSafe("simple binary file",      true,  "file.bin");
	expectSafe("space inside",            true,  "a b.txt");
	expectSafe("max length (255 bytes)",  true,  string(255, 'a'));
}

TEST(CUTMiscPathSafety, basicInvalid)
{
	expectSafe("empty string",            false, "");
	expectSafe("over 255 bytes",          false, string(256, 'a'));
	expectSafe("just a dot",              false, ".");
	expectSafe("double dot",              false, "..");
	expectSafe("leading space",           false, " foo");
	expectSafe("trailing space",          false, "foo ");
	expectSafe("trailing dot",            false, "foo.");
	expectSafe("dot path",                false, "./././");
}

TEST(CUTMiscPathSafety, doubleDots)
{
	expectSafe("contains double dot",     true,  "foo..bar");
	expectSafe("leading double dot",      true,  "..foo");
	expectSafe("trailing double dot",     false, "foo.."); // trailing dot
}

TEST(CUTMiscPathSafety, controlCharacters)
{
	expectSafe("null byte",               false, string("foo\x00""bar", 7));
	expectSafe("newline",                 false, "foo\nbar");
	expectSafe("tab",                     false, "foo\tbar");
	expectSafe("C0 control (0x01)",       false, "foo\x01""bar");
	expectSafe("DEL (0x7F)",              false, "foo\x7f""bar");
	expectSafe("C1 control (0x80)",       false, "foo\xc2\x80""bar"); // U+0080
	expectSafe("C1 control (0x85)",       false, "foo\xc2\x85""bar"); // U+0085
	expectSafe("C1 control (0x9F)",       false, "foo\xc2\x9f""bar"); // U+009F
}

TEST(CUTMiscPathSafety, illegalCharacters)
{
	expectSafe("colon",                   false, "foo:bar");
	expectSafe("drive letter",            false, "C:a");
	expectSafe("asterisk",                false, "foo*bar");
	expectSafe("question mark",           false, "foo?bar");
	expectSafe("double quote",            false, "foo\"bar");
	expectSafe("less than",               false, "foo<bar");
	expectSafe("greater than",            false, "foo>bar");
	expectSafe("pipe",                    false, "foo|bar");
	expectSafe("forward slash",           false, "foo/bar");
	expectSafe("backslash",               false, "foo\\bar");
}

TEST(CUTMiscPathSafety, unicodeSpecialCodepoints)
{
	expectSafe("fullwidth period U+FF0E", false, "foo\xef\xbc\x8e""bar");
	expectSafe("replacement char U+FFFD", false, "foo\xef\xbf\xbd""bar");
	expectSafe("BOM U+FEFF",              false, "foo\xef\xbb\xbf""bar");
	expectSafe("BOM U+FEFF leading",      false, "\xef\xbb\xbf""a");
}

TEST(CUTMiscPathSafety, windowsBestFitCharacters)
{
	// map to path traversal characters under WideCharToMultiByte
	expectSafe("fullwidth solidus U+FF0F",     false, "foo\xef\xbc\x8f""bar"); // / on CP 874, 1250-1258
	expectSafe("fullwidth rev solidus U+FF3C", false, "foo\xef\xbc\xbc""bar"); // \ on CP 874, 1250-1258
	expectSafe("fullwidth colon U+FF1A",       false, "foo\xef\xbc\x9a""bar"); // : on CP 874, 1250-1258
	expectSafe("division slash U+2215",        false, "foo\xe2\x88\x95""bar"); // / on CP 1250, 1252, 1254
	expectSafe("set minus U+2216",             false, "foo\xe2\x88\x96""bar"); // \ on CP 1250, 1252, 1254
	expectSafe("fraction slash U+2044",        false, "foo\xe2\x81\x84""bar"); // / on CP 1250, 1252, 1254
	expectSafe("ratio U+2236",                 false, "foo\xe2\x88\xb6""bar"); // : on CP 1250, 1252, 1254
	expectSafe("armenian full stop U+0589",    false, "foo\xd6\x89""bar");     // : on CP 1250, 1252, 1254
	expectSafe("yen sign U+00A5",              false, "foo\xc2\xa5""bar");     // \ on CP 932 (Japanese)
	expectSafe("won sign U+20A9",              false, "foo\xe2\x82\xa9""bar"); // \ on CP 949 (Korean)
	expectSafe("acute accent U+00B4",          false, "foo\xc2\xb4""bar");     // / on CP 1253 (Greek)
	// the same with subdirectories allowed, since those are the separator look-alikes
	expectSafe("division slash U+2215 subdirs", false, "unicode\xe2\x88\x95""bad.bin", true);
	expectSafe("set minus U+2216 subdirs",      false, "unicode\xe2\x88\x96""bad.bin", true);
	expectSafe("fullwidth period U+FF0E subdirs", false, "unicode\xef\xbc\x8e""bad.bin", true);
}

TEST(CUTMiscPathSafety, invalidUtf8)
{
	expectSafe("invalid continuation",    false, "foo\x80""bar");
	expectSafe("truncated sequence",      false, "foo\xc3");
	expectSafe("5-byte lead",             false, "foo\xf8\x88\x80\x80\x80");
	expectSafe("overlong slash (2-byte)", false, "foo\xc0\xaf""bar"); // U+002F as 2-byte
	expectSafe("overlong dot (2-byte)",   false, "foo\xc0\xae""bar"); // U+002E as 2-byte
	expectSafe("overlong 'a' (2-byte)",   false, "foo\xc1\xa1""bar"); // U+0061 as 2-byte
	expectSafe("overlong 'A' (2-byte)",   false, "foo\xc1\x81""bar"); // U+0041 as 2-byte
	expectSafe("overlong null (2-byte)",  false, "foo\xc0\x80""bar"); // U+0000 as 2-byte
	expectSafe("overlong slash (3-byte)", false, "overlong\xe0\x80\xaf""bad.bin", true);
	expectSafe("overlong slash (4-byte)", false, "overlong\xf0\x80\x80\xaf""bad.bin", true);
	expectSafe("overlong 0xC0 0x2E",      false, "overlong\xc0\x2e""bad.bin");
	expectSafe("overlong 0xE0 0x40 0xAE", false, "overlong\xe0\x40\xae""bad.bin");
	expectSafe("overlong 0xC0 0x2F",      false, "overlong\xc0\x2f""bad.bin");
	expectSafe("overlong 0xC0 0x5C",      false, "overlong\xc0\x5c""bad.bin");
	expectSafe("overlong 0xC0 0x80 0x5C", false, "overlong\xc0\x80\x5c""bad.bin");
	expectSafe("surrogate U+D800",        false, "foo\xed\xa0\x80""bar");
	expectSafe("above U+10FFFF",          false, "foo\xf4\x90\x80\x80""bar");
}

TEST(CUTMiscPathSafety, pathsWithoutSubdirs)
{
	expectSafe("forward slash blocked",   false, "foo/bar");
	expectSafe("backslash blocked",       false, "foo\\bar");
	expectSafe("dotdot slash",            false, "../bad.bin");
	expectSafe("dotdot backslash",        false, "..\\bad.bin");
	expectSafe("subdir dotdot backslash", false, "also/..\\bad.bin");
	expectSafe("subdir slash",            false, "also/bad.bin");
}

TEST(CUTMiscPathSafety, pathsWithSubdirs)
{
	expectSafe("simple subdir",           true,  "foo/bar",           true);
	expectSafe("backslash subdir",        true,  "foo\\bar",          true);
	expectSafe("deep path",               true,  "a/b/c/d.txt",       true);
	expectSafe("trailing slash",          true,  "foo/bar/",          true);
	expectSafe("duplicate separator",     true,  "a//b",              true); // harmless; normalizeRelativePath collapses it
	expectSafe("colon in path",           false, "foo/b:r/baz",       true);
	expectSafe("drive letter path",       false, "C:\\a",             true);
	expectSafe("control char in path",    false, "foo/b\nar/baz",     true);
	expectSafe("dot path",                false, "./././",            true);
}

TEST(CUTMiscPathSafety, leadingSeparators)
{
	expectSafe("leading slash",           false, "/foo/bar",          true);
	expectSafe("leading backslash",       false, "\\foo\\bar",        true);
}

TEST(CUTMiscPathSafety, dotdotInPaths)
{
	expectSafe("double dot with subdirs",     false, "..",              true);
	expectSafe("leading dotdot in path",      false, "../bar",          true);
	expectSafe("leading dotdot backslash",    false, "..\\a",           true);
	expectSafe("dotdot in path",              false, "foo/../bar",      true);
	expectSafe("dotdot at end",               false, "foo/..",          true);
	expectSafe("dotdot component leading",    true,  "foo/..bar/baz",   true);
	expectSafe("dotdot component middle",     true,  "foo/ba..r/baz",   true);
	expectSafe("dotdot component trailing",   false, "foo/bar../baz",   true); // trailing dot
}

TEST(CUTMiscPathSafety, perComponentChecks)
{
	expectSafe("leading space in component",  false, "foo/ bar/baz",    true);
	expectSafe("trailing space in component", false, "foo/bar /baz",    true);
	expectSafe("trailing dot in component",   false, "foo/bar./baz",    true);
	expectSafe("dot component in path",       false, "foo/./bar",       true);
	expectSafe("leading space after slash",   false, "foo/ bar",        true);
	expectSafe("trailing space before slash", false, "bar /baz",        true);
	expectSafe("trailing dot before slash",   false, "bar./baz",        true);
}

TEST(CUTMiscPathSafety, normalizeRelativePath)
{
	// NeL keeps '/' as the separator on every platform
	expectNormalized("empty",                          "",            "");
	expectNormalized("passthrough simple",             "foo.txt",     "foo.txt");
	expectNormalized("passthrough subdir",             "foo/bar.txt", "foo/bar.txt");
	expectNormalized("backslash to sep",               "foo/bar.txt", "foo\\bar.txt");
	expectNormalized("mixed separators",               "a/b/c",       "a/b\\c");
	expectNormalized("duplicate slashes",              "foo/bar",     "foo//bar");
	expectNormalized("duplicate backslashes",          "foo/bar",     "foo\\\\bar");
	expectNormalized("triple slashes",                 "foo/bar",     "foo///bar");
	expectNormalized("leading slash stripped",         "foo",         "/foo");
	expectNormalized("leading backslash stripped",     "foo",         "\\foo");
	expectNormalized("multiple leading slashes",       "foo",         "///foo");
	expectNormalized("unc root is relative",           "server/x",    "\\\\server\\x");
	expectNormalized("leading dot-slash stripped",     "foo",         "./foo");
	expectNormalized("leading dot-backslash stripped", "foo",         ".\\foo");
	expectNormalized("leading dot segments stripped",  "x",           ".\\./x");
	expectNormalized("leading slash dot stripped",     "x",           "/./x");
	expectNormalized("deep path normalized",           "a/b/c/d.txt", "/a//b\\c/d.txt");
	// not validated, and dot segments inside the path are kept for isSafeFilename to refuse
	expectNormalized("dotdot retained",                "foo/../bar",  "foo/../bar");
	expectNormalized("dotdot at start retained",       "../bar",      "../bar");
	expectNormalized("dotdot at end retained",         "foo/..",      "foo/..");
	expectNormalized("dot component retained mid",     "foo/./bar",   "foo/./bar");
	// a lone dot is a name, not a segment marker
	expectNormalized("lone dot kept",                  ".",           ".");
	expectNormalized("hidden file kept",               ".hidden",     "./.hidden");
}

TEST(CUTMiscPathSafety, combined)
{
	expectCombined("absolute windows path",  false, "C:\\Tools\\secrets.txt"); // absolute path
	expectCombined("root is relative",       true,  "/meow/image.jpg");        // root separators are normalized to relative
	expectCombined("relative dot path",      true,  "././meow/image.jpg");     // ok because no effect
	expectCombined("inner dot path",         false, "././meow/./image.jpg");   // blocked because plausibly a downstream traversal attempt
	expectCombined("double dot path",        false, "../meow/image.jpg");      // direct traversal attempt
	expectCombined("mid double dot path",    false, "meow/../image.jpg");      // technically a subpath but plausibly a downstream traversal attempt
	expectCombined("end double dot path",    false, "meow/..");                // blank path
	expectCombined("triple single dot root", false, "./././");                 // blank path
	expectCombined("triple single dot file", true,  "./././image.jpg");        // weird but okay
	expectCombined("scenario path",          true,  ".\\scenarios//my scenario.r2");
	expectCombined("rooted examples path",   true,  "/examples/list.lua");
	expectCombined("traversal to cfg",       false, "examples/../../client.cfg");
	expectCombined("absolute unix path",     false, "/../etc/passwd");
}
