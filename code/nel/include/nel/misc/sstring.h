// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef NL_SSTRING_H
#define NL_SSTRING_H

//#include "types_nl.h"
#include <string>
#include <vector>
#include <cstdio>

#include "stream.h"
#include "path.h"
#include "string_common.h"

namespace	NLMISC
{

// advanced class declaration...
//class CVectorSString;
class CSString;
typedef std::vector<CSString> CVectorSString;

/**
 * CSString : std::string with more functionalities and case insensitive compare
 *
 * \author Daniel Miller
 * \author Nevrax
 * \date 2003
 */
class CSString: public std::string
{
public:
	///	ctor
	CSString();
	///	ctor
	CSString(const char *s);
	/// ctor
	CSString(const std::string &s);
	///	ctor
	CSString(char c);
	///	ctor
	CSString(int i,const char *fmt="%d");
	///	ctor
	CSString(uint32 u,const char *fmt="%u");
	/// ctor
	CSString(double d,const char *fmt="%f");
	///	ctor
	CSString(const char *s,const char *fmt);
	///	ctor
	CSString(const std::string &s,const char *fmt);
	///	ctor
	CSString(const std::vector<NLMISC::CSString>& v,const std::string& separator="\n");

	/// Const [] operator
	std::string::const_reference operator[](std::string::size_type idx) const;
	/// Non-Const [] operator
	std::string::reference operator[](std::string::size_type idx);

	/// Return the first character, or '\\0' is the string is empty
	char operator*();
	/// Return the n right hand most characters of a string
	char back() const;

	/// Return the n left hand most characters of a string
	CSString left(uint32 count) const;
	/// Return the n right hand most characters of a string
	CSString right(uint32 count) const;

	/// Return the string minus the n left hand most characters of a string
	CSString leftCrop(uint32 count) const;
	/// Return the string minus the n right hand most characters of a string
	CSString rightCrop(uint32 count) const;

	/// Return sub string up to but not including first instance of given character, starting at 'iterator'
	/// on exit 'iterator' indexes first character after extracted string segment
	CSString splitToWithIterator(char c,uint32& iterator) const;
	/// Return sub string up to but not including first instance of given character
	CSString splitTo(char c) const;
	/// Return sub string up to but not including first instance of given character
	CSString splitTo(char c,bool truncateThis=false,bool absorbSeparator=true);
	/// Return sub string up to but not including first instance of given character
	CSString splitTo(const char *s,bool truncateThis=false);
	/// Return sub string up to but not including first non-quote encapsulated '//'
	CSString splitToLineComment(bool truncateThis=false, bool useSlashStringEscape=true);

	/// Return sub string from character following first instance of given character on
	CSString splitFrom(char c) const;
	/// Return sub string from character following first instance of given character on
	CSString splitFrom(const char *s) const;

	/// Behave like a s strtok() routine, returning the sun string extracted from (and removed from) *this
	CSString strtok(const char *separators,
					bool useSmartExtensions=false,			// if true then match brackets etc (and refine with following args)
					bool useAngleBrace=false,				// - treat '<' and '>' as brackets
					bool useSlashStringEscape=true,			// - treat '\' as escape char so "\"" == '"'
					bool useRepeatQuoteStringEscape=true);	// - treat """" as '"')

	/// Return first word (blank separated) - can remove extracted word from source string
	CSString firstWord(bool truncateThis=false);
	///	Return first word (blank separated)
	CSString firstWordConst() const;
	/// Return sub string remaining after the first word
	CSString tailFromFirstWord() const;
	/// Count the number of words in a string
	uint32 countWords() const;
	/// Extract the given word
	CSString word(uint32 idx) const;

	/// Return first word or quote-encompassed sub-string - can remove extracted sub-string from source string
	CSString firstWordOrWords(bool truncateThis=false,bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true);
	///	Return first word or quote-encompassed sub-string
	CSString firstWordOrWordsConst(bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true) const;
	/// Return sub string following first word (or quote-encompassed sub-string)
	CSString tailFromFirstWordOrWords(bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true) const;
	/// Count the number of words (or quote delimited sub-strings) in a string
	uint32 countWordOrWords(bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true) const;
	/// Extract the given words (or quote delimited sub-strings)
	CSString wordOrWords(uint32 idx,bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true) const;

	/// Return first line - can remove extracted line from source string
	CSString firstLine(bool truncateThis=false);
	///	Return first line
	CSString firstLineConst() const;
	/// Return sub string remaining after the first line
	CSString tailFromFirstLine() const;
	/// Count the number of lines in a string
	uint32 countLines() const;
	/// Extract the given line
	CSString line(uint32 idx) const;

	/// A handy utility routine for knowing if a character is a white space character or not (' ','\t','\n','\r',26)
	static bool isWhiteSpace(char c);
	///	Test whether character matches '{', '(','[' or '<' (the '<' test depends on the useAngleBrace parameter
	static bool isOpeningDelimiter(char c,bool useAngleBrace=false);
	///	Test whether character matches '}', ')',']' or '>' (the '>' test depends on the useAngleBrace parameter
	static bool isClosingDelimiter(char c,bool useAngleBrace=false);
	///	Test whether character matches '\'' or '\"'
	static bool isStringDelimiter(char c);
	///	Tests whether the character 'b' is the closing delimiter or string delimiter corresponding to character 'a'
	static bool isMatchingDelimiter(char a,char b);

	/// A handy utility routine for knowing if a character is a valid component of a file name
	static bool isValidFileNameChar(char c);
	/// A handy utility routine for knowing if a character is a valid first char for a keyword (a..z, '_')
	static bool isValidKeywordFirstChar(char c);
	/// A handy utility routine for knowing if a character is a valid subsequent char for a keyword (a..z, '_', '0'..'9')
	static bool isValidKeywordChar(char c);
	/// A handy utility routine for knowing if a character is printable (isValidFileNameChar + more basic punctuation)
	static bool isPrintable(char c);

	/// A handy utility routine for knowing if a character is a hex digit 0..9, a..f
	static bool isHexDigit(char c);
	/// A handy utility routine for converting a hex digit to a numeric value 0..15
	static char convertHexDigit(char c);

	// a handy routine that tests whether a given string contains binary characters or not. Only characters>32 + isWhiteSpace() are valid
	bool isValidText();
	// a handy routine that tests whether a given string is a valid file name or not
	// "\"hello there\\bla\""	is valid
	// "hello there\\bla"		is not valid - missing quotes
	// "\"hello there\"\\bla"	is not valid - text after quotes
	bool isValidFileName() const;
	// a second handy routine that tests whether a given string is a valid file name or not
	// equivalent to ('\"'+*this+'\"').isValidFileName()
	// "\"hello there\\bla\""	is not valid  - too many quotes
	// "hello there\\bla"		is valid
	bool isValidUnquotedFileName() const;
	// a handy routine that tests whether or not a given string is a valid keyword
	bool isValidKeyword() const;
	// a handy routine that tests whether or not a given string is quote encapsulated
	bool isQuoted(	bool useAngleBrace=false,						// treat '<' and '>' as brackets
					bool useSlashStringEscape=true,					// treat '\' as escape char so "\"" == '"'
					bool useRepeatQuoteStringEscape=true) const;	// treat """" as '"'

	///	Search for the closing delimiter matching the opening delimiter at position 'startPos' in the 'this' string
	/// on error returns startPos
	uint32 findMatchingDelimiterPos(bool useAngleBrace,bool useSlashStringEscape,bool useRepeatQuoteStringEscape,uint32 startPos=0) const;

	///	Extract a chunk from the 'this' string
	/// if first non-blank character is a string delimiter or an opening delimiter then extracts up to the matching closing delimiter
	/// in all other cases an empty string is returned
	/// the return string includes the opening blank characters (it isn't stripped)
	CSString matchDelimiters(bool truncateThis=false,
							 bool useAngleBrace=false,				// treat '<' and '>' as brackets
							 bool useSlashStringEscape=true,		// treat '\' as escape char so "\"" == '"'
							 bool useRepeatQuoteStringEscape=true);	// treat """" as '"'

	/// copy out section of string up to separator character, respecting quotes (but not brackets etc)
	/// on error tail after returned string doesn't begin with valid separator character
	CSString splitToStringSeparator(	char separator,
										bool truncateThis=false,
										bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
										bool useRepeatQuoteStringEscape=true,	// treat """" as '"'
										bool truncateSeparatorCharacter=false);	// if true tail begins after separator char

	/// copy out section of string up to separator character, respecting quotes, brackets, etc
	/// on error tail after returned string doesn't begin with valid separator character
	/// eg: splitToSeparator(','); - this might be used to split some sort of ',' separated input
	CSString splitToSeparator(	char separator,
								bool truncateThis=false,
								bool useAngleBrace=false,				// treat '<' and '>' as brackets
								bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
								bool useRepeatQuoteStringEscape=true,	// treat """" as '"'
								bool truncateSeparatorCharacter=false);	// if true tail begins after separator char

	CSString splitToSeparator(	char separator,
								bool useAngleBrace=false,						// treat '<' and '>' as brackets
								bool useSlashStringEscape=true,					// treat '\' as escape char so "\"" == '"'
								bool useRepeatQuoteStringEscape=true) const;	// treat """" as '"'

	/// copy out section of string up to any of a given set of separator characters, respecting quotes, brackets, etc
	/// on error tail after returned string doesn't begin with valid separator character
	/// eg: splitToOneOfSeparators(",;",true,false,false,true); - this might be used to split a string read from a CSV file
	CSString splitToOneOfSeparators(	const CSString& separators,
										bool truncateThis=false,
										bool useAngleBrace=false,				// treat '<' and '>' as brackets
										bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
										bool useRepeatQuoteStringEscape=true,	// treat """" as '"'
										bool truncateSeparatorCharacter=false,	// if true tail begins after separator char
										bool splitStringAtBrackets=true);		// if true consider brackets as breaks in the string

	CSString splitToOneOfSeparators(	const CSString& separators,
										bool useAngleBrace=false,						// treat '<' and '>' as brackets
										bool useSlashStringEscape=true,					// treat '\' as escape char so "\"" == '"'
										bool useRepeatQuoteStringEscape=true) const;	// treat """" as '"'

	/// Return true if the string is a single block encompassed by a pair of delimiters
	/// eg: "((a)(b)(c))" or "(abc)" return true wheras "(a)(b)(c)" or "abc" return false
	bool isDelimitedMonoBlock(	bool useAngleBrace=false,				// treat '<' and '>' as brackets
								bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
								bool useRepeatQuoteStringEscape=true	// treat """" as '"'
							 ) const;

	/// Return the sub string with leading and trailing delimiters ( such as '(' and ')' or '[' and ']' ) removed
	/// if the string isn't a delimited monoblock then the complete string is returned
	/// eg "((a)b(c))" returns "(a)b(c)" whereas "(a)b(c)" returns the identical "(a)b(c)"
	CSString stripBlockDelimiters(	bool useAngleBrace=false,				// treat '<' and '>' as brackets
									bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
									bool useRepeatQuoteStringEscape=true	// treat """" as '"'
								 ) const;

	/// Append the individual words in the string to the result vector
	/// retuns true on success
	bool splitWords(CVectorSString& result) const;

	/// Append the individual "wordOrWords" elements in the string to the result vector
	/// retuns true on success
	bool splitWordOrWords(CVectorSString& result,bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true) const;

	/// Append the individual lines in the string to the result vector
	/// retuns true on success
	bool splitLines(CVectorSString& result) const;

	/// Append the separator-separated elements in the string to the result vector
	/// retuns true on success
	bool splitBySeparator(	char separator, CVectorSString& result,
							bool useAngleBrace=false,				// treat '<' and '>' as brackets
							bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
							bool useRepeatQuoteStringEscape=true,	// treat """" as '"'
							bool skipBlankEntries=false				// dont add blank entries to the result vector
						 ) const;

	/// Append the separator-separated elements in the string to the result vector
	/// retuns true on success
	bool splitByOneOfSeparators(	const CSString& separators, CVectorSString& result,
									bool useAngleBrace=false,				// treat '<' and '>' as brackets
									bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
									bool useRepeatQuoteStringEscape=true,	// treat """" as '"'
									bool retainSeparators=false,			// have the separators turn up in the result vector
									bool skipBlankEntries=false				// dont add blank entries to the result vector
								 ) const;

	/// join an array of strings to form a single string (appends to the existing content of this string)
	/// if this string is not empty then a separator is added between this string and the following
	const CSString& join(const std::vector<CSString>& strings, const CSString& separator="");
	const CSString& join(const std::vector<CSString>& strings, char separator);

	/// Return a copy of the string with leading and trainling spaces removed
	CSString strip() const;
	/// Return a copy of the string with leading spaces removed
	CSString leftStrip() const;
	/// Return a copy of the string with trainling spaces removed
	CSString rightStrip() const;

	/// Making an upper case copy of a string
	CSString toUpper() const;

	/// Making a lower case copy of a string
	CSString toLower() const;

	/// encapsulate string in quotes, adding escape characters as necessary
	CSString quote(	bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
					bool useRepeatQuoteStringEscape=true	// treat """" as '"'
					) const;

	/// if a string is not already encapsulated in quotes then return quote() else return *this
	CSString quoteIfNotQuoted(	bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
								bool useRepeatQuoteStringEscape=true	// treat """" as '"'
								) const;

	/// if a string is not a single word and is not already encapsulated in quotes then return quote() else return *this
	CSString quoteIfNotAtomic(	bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
								bool useRepeatQuoteStringEscape=true	// treat """" as '"'
								) const;

	/// strip delimiting quotes and clear through escape characters as necessary
	CSString unquote(bool useSlashStringEscape=true,		// treat '\' as escape char so "\"" == '"'
					 bool useRepeatQuoteStringEscape=true	// treat """" as '"'
					) const;

	/// equivalent to if (isQuoted()) unquote()
	CSString unquoteIfQuoted(bool useSlashStringEscape=true,
					 bool useRepeatQuoteStringEscape=true
					) const;

	///	encode special characters such as quotes, gt, lt, etc to xml encoding
	/// the isParameter paramter is true if the string is to be used in an XML parameter block
	CSString encodeXML(bool isParameter=false) const;

	///	decode special characters such as quotes, gt, lt, etc from xml encoding
	CSString decodeXML() const;

	/// verifies whether a string contains sub-strings that correspond to xml special character codes
	bool isEncodedXML() const;

	/// verifies whether a string contains any XML incompatible characters
	/// in this case the string can be converted to xml compatible format via encodeXML()
	/// the isParameter paramter is true if the string is to be used in an XML parameter block
	bool isXMLCompatible(bool isParameter=false) const;

	/// Replacing all occurences of one string with another
	CSString replace(const char *toFind,const char *replacement) const;

	/// Find index at which a sub-string starts (case not sensitive) - if sub-string not found then returns string::npos
	std::string::size_type find(const char *toFind, std::string::size_type startLocation=0) const;

	/// Find index at which a sub-string starts (case NOT sensitive) - if sub-string not found then returns string::npos
	std::string::size_type findNS(const char *toFind, std::string::size_type startLocation=0) const;

	/// Return true if this contains given sub string
	bool contains(const char *toFind) const;

	/// Return true if this contains given sub string
	bool contains(int character) const;

	/// Handy atoi routines...
	int atoi() const;
	sint32 atosi() const;
	uint32 atoui() const;
	sint64 atoi64() const;
	sint64 atosi64() const;
	uint64 atoui64() const;

	/// A handy atof routine...
	double atof() const;

	/// assignment operator
	CSString& operator=(const char *s);
	/// assignment operator
	CSString& operator=(const std::string &s);
	/// assignment operator
	CSString& operator=(char c);
	/// assignment operator
	CSString& operator=(int i);
	/// assignment operator
	CSString& operator=(uint32 u);
	/// assignment operator
	CSString& operator=(double d);

	/// Case insensitive string compare
	bool operator==(const CSString &other) const;
	/// Case insensitive string compare
	bool operator==(const std::string &other) const;
	/// Case insensitive string compare
	bool operator==(const char* other) const;

	/// Case insensitive string compare
	bool operator!=(const CSString &other) const;
	/// Case insensitive string compare
	bool operator!=(const std::string &other) const;
	/// Case insensitive string compare
	bool operator!=(const char* other) const;

	/// Case insensitive string compare
	bool operator<=(const CSString &other) const;
	/// Case insensitive string compare
	bool operator<=(const std::string &other) const;
	/// Case insensitive string compare
	bool operator<=(const char* other) const;

	/// Case insensitive string compare
	bool operator>=(const CSString &other) const;
	/// Case insensitive string compare
	bool operator>=(const std::string &other) const;
	/// Case insensitive string compare
	bool operator>=(const char* other) const;

	/// Case insensitive string compare
	bool operator>(const CSString &other) const;
	/// Case insensitive string compare
	bool operator>(const std::string &other) const;
	/// Case insensitive string compare
	bool operator>(const char* other) const;

	/// Case insensitive string compare
	bool operator<(const CSString &other) const;
	/// Case insensitive string compare
	bool operator<(const std::string &other) const;
	/// Case insensitive string compare
	bool operator<(const char* other) const;

	//@{
	//@name Easy concatenation operator to build strings
	template <class T>
	CSString &operator <<(const T &value)
	{
		operator +=(NLMISC::toString(value));

		return *this;
	}

	// specialisation for C string
	CSString &operator <<(const char *value)
	{
		static_cast<std::string*>(this)->operator +=(value);

		return *this;
	}

	// specialisation for character
	CSString &operator <<(char value)
	{
		static_cast<std::string*>(this)->operator +=(value);

		return *this;
	}

	// specialisation for std::string
	CSString &operator <<(const std::string &value)
	{
		static_cast<std::string*>(this)->operator +=(value);

		return *this;
	}
	// specialisation for CSString
	CSString &operator <<(const CSString &value)
	{
		static_cast<std::string*>(this)->operator +=(value);

		return *this;
	}
	//@}

	/// Case insensitive string compare (useful for use as map keys, see less<CSString> below)
	bool icompare(const std::string &other) const;

	/// Serial
	void serial( NLMISC::IStream& s );

	/// Read a text file into a string
	bool readFromFile(const CSString& fileName);

	/// Write a string to a text file
	// returns true on success, false on failure
	bool writeToFile(const CSString& fileName) const;

	/// Write a string to a text file
	// if the file already exists and its content is identicall to our own then it is not overwritten
	// returns true on success (including the case where the file exists and is not overwritten), false on failure
	bool writeToFileIfDifferent(const CSString& fileName) const;
};


/*
 * Vector of CSString compatible with vector<string>
 */
//typedef std::vector<CSString> CVectorSString;


/*CVectorSString &operator = (CVectorSString &left, const std::vector<std::string> &right)
{
	left = reinterpret_cast<CVectorSString&>(right);
}
CVectorSString &operator = (CVectorSString &left, const std::vector<CSString> &right)
{
	left = reinterpret_cast<CVectorSString&>(right);
}
*/
//class CVectorSString : public std::vector<CSString>
//{
//public:
//	// cast to and convert from std::vector<std::string>
//	operator std::vector<std::string>& ()
//	{
//		return reinterpret_cast<std::vector<std::string>&>(*this);
//	}
//	operator const std::vector<std::string>& () const
//	{
//		return reinterpret_cast<const std::vector<std::string>&>(*this);
//	}
//	CVectorSString&	operator= ( const std::vector<std::string>& v )
//	{
//		*this = reinterpret_cast<const CVectorSString&>(v);
//		return *this;
//	}
//
//	// simple ctors
//	CVectorSString()							{}
//	CVectorSString( const CVectorSString& v )	{ operator=(v); }
//
//	// ctors for building from different vetor types
//	CVectorSString( const std::vector<CSString>& v ):							std::vector<CSString>(v)			{}
//	CVectorSString( const std::vector<std::string>& v ):						std::vector<CSString>(*(std::vector<CSString>*)&v) {}
//
//	// ctor for extracting sub_section of another vector
//	CVectorSString( const const_iterator& first, const const_iterator& last ):	std::vector<CSString>(first,last)	{}
//};


/*
 * Inlines
 */

inline CSString::CSString()
{
}

inline CSString::CSString(const char *s)
{
	*(std::string *)this=s;
}

inline CSString::CSString(const std::string &s)
{
	*(std::string *)this=s;
}

inline CSString::CSString(char c)
{
	*(std::string *)this=c;
}

inline CSString::CSString(int i,const char *fmt)
{
	char buf[1024];
	sprintf(buf,fmt,i);
	*this=buf;
}

inline CSString::CSString(uint32 u,const char *fmt)
{
	char buf[1024];
	sprintf(buf,fmt,u);
	*this=buf;
}

inline CSString::CSString(double d,const char *fmt)
{
	char buf[1024];
	sprintf(buf,fmt,d);
	*this=buf;
}

inline CSString::CSString(const char *s,const char *fmt)
{
	char buf[1024];
	sprintf(buf,fmt,s);
	*this=buf;
}

inline CSString::CSString(const std::string &s,const char *fmt)
{
	char buf[1024];
	sprintf(buf,fmt,s.c_str());
	*this=buf;
}

inline CSString::CSString(const std::vector<NLMISC::CSString>& v,const std::string& separator)
{
	for (uint32 i=0;i<v.size();++i)
	{
		if (i>0)
			*this+=separator;
		*this+=v[i];
	}
}

inline char CSString::operator*()
{
	if (empty())
		return 0;
	return (*this)[0];
}

inline char CSString::back() const
{
	return (*this)[size()-1];
}

inline CSString CSString::right(uint32 count) const
{
	if (count>=size())
		return *this;
	return substr(size()-count);
}

inline CSString CSString::rightCrop(uint32 count) const
{
	if (count>=size())
		return CSString();
	return substr(0,size()-count);
}

inline CSString CSString::left(uint32 count) const
{
	return substr(0,count);
}

inline CSString CSString::leftCrop(uint32 count) const
{
	if (count>=size())
		return CSString();
	return substr(count);
}

inline CSString CSString::splitToWithIterator(char c,uint32& iterator) const
{
	uint32 i;
	CSString result;
	for (i=iterator;i<size() && (*this)[i]!=c;++i)
		result+=(*this)[i];
	iterator= i;
	return result;
}

inline bool CSString::isWhiteSpace(char c)
{
	return c==' ' || c=='\t' || c=='\n' || c=='\r' || c==26;
}

inline bool CSString::isOpeningDelimiter(char c,bool useAngleBrace)
{
	return c=='(' || c=='[' || c=='{' || (useAngleBrace && c=='<');
}

inline bool CSString::isClosingDelimiter(char c,bool useAngleBrace)
{
	return c==')' || c==']' || c=='}' || (useAngleBrace && c=='>');
}

inline bool CSString::isStringDelimiter(char c)
{
	return c=='\"' || c=='\'';
}

inline bool CSString::isMatchingDelimiter(char a,char b)
{
	return	(a=='(' && b==')') || (a=='[' && b==']') ||
			(a=='{' && b=='}') || (a=='<' && b=='>') ||
			(a=='\"' && b=='\"') || (a=='\'' && b=='\'');
}

inline bool CSString::isValidFileNameChar(char c)
{
	if (c>='a' && c<='z') return true;
	if (c>='A' && c<='Z') return true;
	if (c>='0' && c<='9') return true;
	if (c=='_') return true;
	if (c==':') return true;
	if (c=='/') return true;
	if (c=='\\') return true;
	if (c=='.') return true;
	if (c=='#') return true;
	if (c=='-') return true;
	return false;
}

inline bool CSString::isPrintable(char c)
{
	if (isValidFileNameChar(c)) return true;
	if (c==' ') return true;
	if (c=='*') return true;
	if (c=='?') return true;
	if (c=='!') return true;
	if (c=='@') return true;
	if (c=='&') return true;
	if (c=='|') return true;
	if (c=='+') return true;
	if (c=='=') return true;
	if (c=='%') return true;
	if (c=='<') return true;
	if (c=='>') return true;
	if (c=='(') return true;
	if (c==')') return true;
	if (c=='[') return true;
	if (c==']') return true;
	if (c=='{') return true;
	if (c=='}') return true;
	if (c==',') return true;
	if (c==';') return true;
	if (c=='$') return true;
	if ((uint8)c==156) return true; // Sterling Pound char causing error in gcc 4.1.2
	if (c=='^') return true;
	if (c=='~') return true;
	if (c=='\'') return true;
	if (c=='\"') return true;
	return false;
}

inline bool CSString::isQuoted(bool useAngleBrace,bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
{
	return (left(1)=="\"") && (right(1)=="\"") && isDelimitedMonoBlock(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape);
}

inline bool CSString::isValidKeywordFirstChar(char c)
{
	if (c>='a' && c<='z') return true;
	if (c>='A' && c<='Z') return true;
	if (c=='_') return true;
	return false;
}

inline bool CSString::isValidKeywordChar(char c)
{
	if (c>='a' && c<='z') return true;
	if (c>='A' && c<='Z') return true;
	if (c>='0' && c<='9') return true;
	if (c=='_') return true;
	return false;
}

inline bool CSString::isHexDigit(char c)
{
	if (c>='0' && c<='9') return true;
	if (c>='A' && c<='F') return true;
	if (c>='a' && c<='f') return true;
	return false;
}

inline char CSString::convertHexDigit(char c)
{
	if (c>='0' && c<='9') return c-'0';
	if (c>='A' && c<='F') return c-'A'+10;
	if (c>='a' && c<='f') return c-'a'+10;
	return 0;
}

inline CSString& CSString::operator=(const char *s)
{
	*(std::string *)this=s;
	return *this;
}

inline CSString& CSString::operator=(const std::string &s)
{
	*(std::string *)this=s;
	return *this;
}

inline CSString& CSString::operator=(char c)
{
	*(std::string *)this=c;
	return *this;
}

inline CSString& CSString::operator=(int i)
{
	CSString other(i);
	*this = other;
	return *this;
}

inline CSString& CSString::operator=(uint32 u)
{
	CSString other(u);
	*this = other;
	return *this;
}

inline CSString& CSString::operator=(double d)
{
	CSString other(d);
	*this = other;
	return *this;
}

inline bool CSString::operator==(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())==0;
}

inline bool CSString::operator==(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())==0;
}

inline bool CSString::operator==(const char* other) const
{
	return stricmp(c_str(),other)==0;
}

inline bool CSString::operator!=(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())!=0;
}

inline bool CSString::operator!=(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())!=0;
}

inline bool CSString::operator!=(const char* other) const
{
	return stricmp(c_str(),other)!=0;
}

inline bool CSString::operator<=(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())<=0;
}

inline bool CSString::operator<=(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())<=0;
}

inline bool CSString::operator<=(const char* other) const
{
	return stricmp(c_str(),other)<=0;
}

inline bool CSString::operator>=(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())>=0;
}

inline bool CSString::operator>=(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())>=0;
}

inline bool CSString::operator>=(const char* other) const
{
	return stricmp(c_str(),other)>=0;
}

inline bool CSString::operator>(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())>0;
}

inline bool CSString::operator>(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())>0;
}

inline bool CSString::operator>(const char* other) const
{
	return stricmp(c_str(),other)>0;
}

inline bool CSString::operator<(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())<0;
}

inline bool CSString::operator<(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())<0;
}

inline bool CSString::operator<(const char* other) const
{
	return stricmp(c_str(),other)<0;
}

inline std::string::const_reference CSString::operator[](std::string::size_type idx) const
{
	static char zero=0;
	if (idx >= size())
		return zero;
	return data()[idx];
}

inline std::string::reference CSString::operator[](std::string::size_type idx)
{
	static char zero=0;
	if (idx >= size())
		return zero;
	return const_cast<std::string::value_type&>(data()[idx]);
}


inline bool CSString::icompare(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())<0;
}

inline void CSString::serial( NLMISC::IStream& s )
{
	s.serial( reinterpret_cast<std::string&>( *this ) );
}

/*
inline CSString operator+(const CSString& s0,char s1)
{
	return CSString(s0)+s1;
}

inline CSString operator+(const CSString& s0,const char* s1)
{
	return CSString(s0)+s1;
}

inline CSString operator+(const CSString& s0,const std::string& s1)
{
	return CSString(s0)+s1;
}
inline CSString operator+(const CSString& s0,const CSString& s1)
{
	return CSString(s0)+s1;
}
*/
inline CSString operator+(char s0,const CSString& s1)
{
	return CSString(s0) + s1.c_str();
}

inline CSString operator+(const char* s0,const CSString& s1)
{
	return CSString(s0) + s1.c_str();
}

} // NLMISC

// *** The following was commented out by Sadge because there were strange compilation/ link issues ***
// *** The '<' operator was implemented instead ***
//_STLP_BEGIN_NAMESPACE
//namespace std
//{
//	/*
//	 * less<CSString> is case insensitive
//	 */
//	template <>
//	struct less<NLMISC::CSString> : public std::binary_function<NLMISC::CSString, NLMISC::CSString, bool>
//	{
//		bool operator()(const NLMISC::CSString& x, const NLMISC::CSString& y) const { return x.icompare(y); }
//	};
//} // std
//_STLP_END_NAMESPACE
//namespace std
//{

//	/*
//	 * less<CSString> is case insensitive
//	 */
//	template <>
//	struct less<NLMISC::CSString> : public std::binary_function<NLMISC::CSString, NLMISC::CSString, bool>
//	{
//		bool operator()(const NLMISC::CSString& x, const NLMISC::CSString& y) const { return x.icompare(y); }
//	};
//} // std
//_STLP_END_NAMESPACE

/**
  * Instead of overriding std::less, please use the following predicate.
  * For example, declare your map as:
  *   std::map<NLMISC::CSString, CMyDataClass, NLMISC::CUnsensitiveSStringLessPred> MyMap;
  * Caution: a map declared without CUnsensitiveSStringLessPred will behave as a
  * standard string map.
  *
  * \see also CUnsensitiveStrLessPred in misc/string_conversion.h
  * for a similar predicate with std::string.
  */
struct CUnsensitiveSStringLessPred : public std::less<NLMISC::CSString>
{
	bool operator()(const NLMISC::CSString& x, const NLMISC::CSString& y) const { return x < y; /*.icompare(y);*/ }
};


#endif // NL_SSTRING_H

/* End of sstring.h */
