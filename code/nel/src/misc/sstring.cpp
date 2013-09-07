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

#include "stdmisc.h"
#include "nel/misc/sstring.h"

namespace NLMISC
{

	CSString CSString::strtok(	const char *separators,
										bool useSmartExtensions,			// if true then match brackets etc (and refine with following args)
										bool useAngleBrace,					// - treat '<' and '>' as brackets
										bool useSlashStringEscape,			// - treat '\' as escape char so "\"" == '"'
										bool useRepeatQuoteStringEscape)	// - treat """" as '"')
	{
		if (useSmartExtensions)
		{
			CSString token;

			// split to the first non empty token, or until the this string is empty
			while (!empty() && token.empty())
				token = splitToOneOfSeparators(separators,true,useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,true);

			return token;
		}

		uint i, j;
		CSString result;

		// skip leading junk
		for (i=0;i<size();++i)
		{
			// look for the next character in the 'separator' character list supplied
			for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
			{}
			// if not found then we're at end of leading junk
			if (!separators[j])
				break;
		}

		// copy out everything up to the next separator character
		for (;i<size();++i)
		{
			// look for the next character in the 'separator' character list supplied
			for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
			{}
			// if not found then we're at end of text chunk
			if (separators[j])
				break;
			result+=(*this)[i];
		}

		// skip trailing junk
		for (;i<size();++i)
		{
			// look for the next character in the 'separator' character list supplied
			for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
			{}
			// if not found then we're at end of leading junk
			if (!separators[j])
				break;
		}

		// delete the treated bit from this string
		(*this)=substr(i);

		return result;
	}


	CSString CSString::splitToOneOfSeparators(	const CSString& separators,
													bool truncateThis,
													bool useAngleBrace,					// treat '<' and '>' as brackets
													bool useSlashStringEscape,			// treat '\' as escape char so "\"" == '"'
													bool useRepeatQuoteStringEscape,	// treat """" as '"'
													bool truncateSeparatorCharacter,	// if true tail begins after separator char
													bool splitStringAtBrackets)
	{
		// iterate over our string
		uint32 i;
		for (i=0;i<size();++i)
		{
			char thisChar=(*this)[i];

			// if we've found the separator character then all's cool so break out of the loop
			if (separators.contains(thisChar))
				break;

			// if we have a bracket or quote of any type then match to it's matching bracket, quote or whatever
			if (isOpeningDelimiter(thisChar,useAngleBrace) || isStringDelimiter(thisChar))
			{
				if (i != 0)
				{
					// we are not at beginning of the string, delimiter is considered as separator
					if (splitStringAtBrackets)
						break;
				}
				uint32 j=i;
				i=findMatchingDelimiterPos(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,i);
				// if there was a problem then break here
				if (j==i)
					break;
				continue;
			}
		}

		// build the return string
		CSString result=left(i);

		// if need be truncate '*this' before returning
		if (truncateThis)
		{
			if (truncateSeparatorCharacter && separators.contains((*this)[i]))
				++i;
			*this=leftCrop(i);
		}

		return result;
	}

	CSString CSString::splitToLineComment(bool truncateThis, bool useSlashStringEscape)
	{
		bool quoteOpen= false;
		bool escape= false;
		for (uint32 i=0;i<size();++i)
		{
			// if we're escaped then accept the next character blindly
			if (escape)
			{
				escape= false;
				continue;
			}

			// do we have an escape?
			if (useSlashStringEscape && operator[](i)=='\\')
			{
				escape= true;
			}
			// do we have a quote (it is not escaped by definition here)
			else if (operator[](i)=='\"')
			{
				quoteOpen= !quoteOpen;
			}
			// do we have a comment (not in quotes)
			else if (!quoteOpen && i<size()-1 && operator[](i)=='/' && operator[](i+1)=='/')
			{
				// we found a comment so strip string down here
				if (truncateThis)
				{
					CSString result= left(i);
					*this= leftCrop(i);
					return result;
				}
				else
				{
					return left(i);
				}
			}
		}
		if (truncateThis)
		{
			CSString result= *this;
			clear();
			return result;
		}
		else
		{
			return *this;
		}
	}

	bool CSString::isValidText()
	{
		// setup a handy static lookup table for differentiating valid and invalid text characters
		static bool* tbl=NULL;
		if (tbl==NULL)
		{
			tbl= new bool[256];
			for (uint32 i=0;i<256;++i)
			{
				tbl[i]= ((i>32) || isWhiteSpace((char)i));
			}
		}

		// scan the string for binary characters
		uint32 i=(uint32)size();
	//	while (i && !tbl[i-1])
	//	{
	//		i--;
	//	}
		while (i--)
		{
			if (!tbl[(uint8)operator[](i)])
			{
				nldebug("string is not valid text due to character: %u at index: %u",(uint8)operator[](i),i);
				return false;
			}
		}

		// no binary characters found so return true
		return true;
	}

	bool CSString::isValidFileName() const
	{
		if (empty())
			return false;

		if ((*this)[0]=='"')
		{
			if (!isDelimitedMonoBlock(false,false,false))
				return false;

			// iterate from size-2 to 1
			for (uint32 i=(uint32)size()-1; --i;)
				if (!isValidFileNameChar((*this)[i]) && (*this)[i]!=' ')
					return false;
		}
		else
		{
			// iterate from size-1 to 0
			for (uint32 i=(uint32)size(); i--;)
				if (!isValidFileNameChar((*this)[i]))
					return false;
		}
		return true;
	}

	bool CSString::isValidUnquotedFileName() const
	{
		return (CSString('\"'+*this+'\"')).isValidFileName();
	}

	bool CSString::isValidKeyword() const
	{
		if (empty())
			return false;

		if (!isValidKeywordFirstChar((*this)[0]))
			return false;

		// iterate from size-1 to 1
		for (uint32 i=(uint32)size(); --i;)
			if (!isValidKeywordChar((*this)[i]))
				return false;

		return true;
	}

	uint32 CSString::findMatchingDelimiterPos(	bool useAngleBrace,
														bool useSlashStringEscape,
														bool useRepeatQuoteStringEscape,
														uint32 startPos ) const
	{
		uint32 i=startPos;
		char openingDelimiter= (*this)[i];
		if (isOpeningDelimiter(openingDelimiter,useAngleBrace))
		{
			// deal with (), [], {} or <> type delimiters
			while (i<size())
			{
				++i;
				if(isMatchingDelimiter(openingDelimiter,(*this)[i]))
				{
					// this is it! we've found the matching quote so we're done
					break;
				}
				if (isOpeningDelimiter((*this)[i],useAngleBrace) || isStringDelimiter((*this)[i]))
				{
					uint32 j=i;
					i=findMatchingDelimiterPos(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,i);
					if (j==i)
						return startPos;
					continue;
				}
				if (isClosingDelimiter((*this)[i],useAngleBrace))
				{
					// we've found a closing delimiter that doesn't match our opening delimiter so give up
					return startPos;
				}
			}
		}
		else if (isStringDelimiter(openingDelimiter))
		{
			// deal with "..." or '...' type delimiters
			while (i<size())
			{
				++i;
				if ((*this)[i]==openingDelimiter)
				{
					if (useRepeatQuoteStringEscape && (*this)[i+1]==openingDelimiter)
					{
						// we've found a "" pair and we're treating it as \" equivalent so skip an extra character
						++i;
						continue;
					}
					else
					{
						// this is it! we've found the matching quote so we're done
						break;
					}
				}
				if (useSlashStringEscape && (*this)[i]=='\\')
				{
					// we've found a '\' character so skip the next character, whatever it is
					++i;
					continue;
				}
			}
		}

		return i;
	}

	CSString CSString::matchDelimiters(	bool truncateThis,
												bool useAngleBrace,					// treat '<' and '>' as brackets
												bool useSlashStringEscape,			// treat '\' as escape char so "\"" == '"'
												bool useRepeatQuoteStringEscape)	// treat """" as '"'
	{
		// skip white space
		uint32 startPos;
		for (startPos=0;startPos<size() && isWhiteSpace((*this)[startPos]);++startPos) {}

		// locate the matching brace
		uint32 matchPos=findMatchingDelimiterPos(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,startPos);

		// if not found give up
		if (matchPos>=size())
		{
			return CSString();
		}

		// build the return string
		CSString result=left(matchPos+1);

		// if need be truncate '*this' before returning
		if (truncateThis)
		{
			*this=leftCrop(matchPos+1);
		}

		return result;
	}

	CSString CSString::splitToStringSeparator(
												char separator,
												bool truncateThis,
												bool useSlashStringEscape,			// treat '\' as escape char so "\"" == '"'
												bool useRepeatQuoteStringEscape,	// treat """" as '"'
												bool truncateSeparatorCharacter)	// if true tail begins after separator char
	{
		// iterate over our string
		uint32 i;
		for (i=0;i<size();++i)
		{
			char thisChar=(*this)[i];

			// if we've found the separator character then all's cool so break out of the loop
			if (thisChar==separator)
				break;

			// if we have a bracket or quote of any type then match to it's matching bracket, quote or whatever
			if (isStringDelimiter(thisChar))
			{
				uint32 j=i;
				i=findMatchingDelimiterPos(false,useSlashStringEscape,useRepeatQuoteStringEscape,i);
				// if there was a problem then break here
				if (j==i)
					break;
				continue;
			}
		}

		// build the return string
		CSString result=left(i);

		// if need be truncate '*this' before returning
		if (truncateThis)
		{
			if (truncateSeparatorCharacter && separator==(*this)[i])
				++i;
			*this=leftCrop(i);
		}

		return result;
	}

	CSString CSString::splitToSeparator(	char separator,
												bool useAngleBrace,						// treat '<' and '>' as brackets
												bool useSlashStringEscape,				// treat '\' as escape char so "\"" == '"'
												bool useRepeatQuoteStringEscape) const	// treat """" as '"'
	{
		return const_cast<CSString*>(this)->splitToSeparator(separator,false,useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,false);
	}

	CSString CSString::splitToSeparator(	char separator,
												bool truncateThis,
												bool useAngleBrace,					// treat '<' and '>' as brackets
												bool useSlashStringEscape,			// treat '\' as escape char so "\"" == '"'
												bool useRepeatQuoteStringEscape,	// treat """" as '"'
												bool truncateSeparatorCharacter)	// if true tail begins after separator char
	{
		// iterate over our string
		uint32 i;
		for (i=0;i<size();++i)
		{
			char thisChar=(*this)[i];

			// if we've found the separator character then all's cool so break out of the loop
			if (thisChar==separator)
				break;

			// if we have a bracket or quote of any type then match to it's matching bracket, quote or whatever
			if (isOpeningDelimiter(thisChar,useAngleBrace) || isStringDelimiter(thisChar))
			{
				uint32 j=i;
				i=findMatchingDelimiterPos(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,i);
				// if there was a problem then break here
				if (j==i)
					break;
				continue;
			}
		}

		// build the return string
		CSString result=left(i);

		// if need be truncate '*this' before returning
		if (truncateThis)
		{
			if (truncateSeparatorCharacter && separator==(*this)[i])
				++i;
			*this=leftCrop(i);
		}

		return result;
	}

	CSString CSString::splitToOneOfSeparators(	const CSString& separators,
														bool useAngleBrace,						// treat '<' and '>' as brackets
														bool useSlashStringEscape,				// treat '\' as escape char so "\"" == '"'
														bool useRepeatQuoteStringEscape) const	// treat """" as '"'
	{
		return const_cast<CSString*>(this)->splitToOneOfSeparators(separators,false,useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,false);
	}


	bool CSString::isDelimitedMonoBlock(	bool useAngleBrace,				// treat '<' and '>' as brackets
												bool useSlashStringEscape,		// treat '\' as escape char so "\"" == '"'
												bool useRepeatQuoteStringEscape	// treat """" as '"'
											 ) const
	{
		if (empty())
			return false;
		uint32 matchPos=findMatchingDelimiterPos(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape);
		return (matchPos==size()-1 && isMatchingDelimiter((*this)[0],(*this)[matchPos]));
	}

	CSString CSString::stripBlockDelimiters(	bool useAngleBrace,				// treat '<' and '>' as brackets
													bool useSlashStringEscape,		// treat '\' as escape char so "\"" == '"'
													bool useRepeatQuoteStringEscape	// treat """" as '"'
												 ) const
	{
		if (isDelimitedMonoBlock(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape))
		{
			return substr(1,size()-2);
		}
		else
		{
			return *this;
		}
	}

	bool CSString::splitWords(CVectorSString& result) const
	{
		CSString s=strip();
		while(!s.empty())
		{
			uint32 pre=(uint32)s.size();
			result.push_back(s.firstWord(true));
			uint32 post=(uint32)s.size();
			if (post>=pre)
				return false;
		}
		return true;
	}

	bool CSString::splitWordOrWords(CVectorSString& result,bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
	{
		CSString s=*this;
		while(!s.empty())
		{
			uint32 pre=(uint32)s.size();
			result.push_back(s.firstWordOrWords(true,useSlashStringEscape,useRepeatQuoteStringEscape));
			uint32 post=(uint32)s.size();
			if (post>=pre)
				return false;
		}
		return true;
	}

	bool CSString::splitLines(CVectorSString& result) const
	{
		CSString s=*this;

		// make sure we deal with '\n\r' style carriage returns cleanly
		if (s.contains('\r'))
			s=s.replace("\r","");

		uint32 it=0;
		uint32 len= (uint32)s.size();
		while(it<len)
		{
			// extract the text up to the next '\n'character
			result.push_back(s.splitToWithIterator('\n',it));
			// skip the '\n' character
			++it;
		}
		return true;
	}

	bool CSString::splitBySeparator(	char separator, CVectorSString& result,
											bool useAngleBrace,					// treat '<' and '>' as brackets
											bool useSlashStringEscape,			// treat '\' as escape char so "\"" == '"'
											bool useRepeatQuoteStringEscape,	// treat """" as '"'
											bool skipBlankEntries				// dont add blank entries to the result vector
										 ) const
	{
		CSString s=*this;
		while(!s.empty())
		{
			uint32 pre=(uint32)s.size();
			result.push_back(s.splitToSeparator(separator,true,useAngleBrace,useSlashStringEscape,
												useRepeatQuoteStringEscape,true));
			if (skipBlankEntries && result.back().empty())
				result.pop_back();
			uint32 post=(uint32)s.size();
			if (post>=pre)
				return false;
		}
		return true;
	}

	bool CSString::splitByOneOfSeparators(	const CSString& separators, CVectorSString& result,
													bool useAngleBrace,				// treat '<' and '>' as brackets
													bool useSlashStringEscape,		// treat '\' as escape char so "\"" == '"'
													bool useRepeatQuoteStringEscape,// treat """" as '"'
													bool retainSeparators,			// have the separators turn up in the result vector
													bool skipBlankEntries			// dont add blank entries to the result vector
												 ) const
	{
		CSString s=*this;
		while (!s.empty() && skipBlankEntries && !retainSeparators && separators.contains(s[0]))
			s= s.leftCrop(1);

		while(!s.empty())
		{
			uint32 pre=(uint32)s.size();
			result.push_back(s.splitToOneOfSeparators(	separators,true,useAngleBrace,useSlashStringEscape,
														useRepeatQuoteStringEscape,!retainSeparators ));

			// if we failed to extract a string segment then we must be looking at a separator
			if (result.back().empty())
			{
				if (skipBlankEntries && result.back().empty())
					result.pop_back();

				if (!s.empty())
				{
					if (retainSeparators)
						result.back()=s[0];
					s=s.leftCrop(1);
				}
			}

			uint32 post=(uint32)s.size();
			if (post>=pre)
				return false;
		}
		return true;
	}

	const CSString& CSString::join(const std::vector<CSString>& strings, const CSString& separator)
	{
		for (uint32 i=0;i<strings.size();++i)
		{
			// add in separators before all but the first string
			if (!empty())
				operator+=(separator);
			// append next string
			operator+=(strings[i]);
		}

		// return a ref to ourselves
		return *this;
	}

	const CSString& CSString::join(const std::vector<CSString>& strings, char separator)
	{
		for (uint32 i=0;i<strings.size();++i)
		{
			// add in separators before all but the first string
			if (!empty())
				operator+=(separator);
			// append next string
			operator+=(strings[i]);
		}

		// return a ref to ourselves
		return *this;
	}

	CSString CSString::strip() const
	{
		CSString result;
		int i,j;
		for (j=(int)size()-1; j>=0 && isWhiteSpace((*this)[j]); --j) {}
		for (i=0;		 i<j  && isWhiteSpace((*this)[i]); ++i) {}
		result=substr(i,j-i+1);
		return result;
	}

	CSString CSString::leftStrip() const
	{
		CSString result;
		int i,j=(int)size()-1;
		for (i=0; i<j  && isWhiteSpace((*this)[i]); ++i) {}
		result=substr(i,j-i+1);
		return result;
	}

	CSString CSString::rightStrip() const
	{
		CSString result;
		int i=0,j;
		for (j=(int)size()-1; j>=0 && isWhiteSpace((*this)[j]); --j) {}
		result=substr(i,j-i+1);
		return result;
	}

	CSString CSString::toUpper() const
	{
		CSString result;
		std::string::const_iterator it;
		for (it=begin();it!=end();++it)
		{
			char c=(*it);
			if (c>='a' && c<='z')
				c^=('a'^'A');
			result+=c;
		}
		return result;
	}

	CSString CSString::toLower() const
	{
		CSString result;
		std::string::const_iterator it;
		for (it=begin();it!=end();++it)
		{
			char c=(*it);
			if (c>='A' && c<='Z')
				c^=('a'^'A');
			result+=c;
		}
		return result;
	}

	CSString CSString::splitTo(char c) const
	{
		uint i;
		CSString result;
		for (i=0;i<size() && (*this)[i]!=c;++i)
			result+=(*this)[i];
		return result;
	}

	CSString CSString::splitTo(char c,bool truncateThis,bool absorbSeparator)
	{
		uint i;
		CSString result;
		for (i=0;i<size() && (*this)[i]!=c;++i)
			result+=(*this)[i];

		// remove the result string from the input string if so desired
		if (truncateThis)
		{
			if (absorbSeparator)
				++i;
			if (i<size())
				(*this)=substr(i);
			else
				clear();
		}

		return result;
	}

	CSString CSString::splitTo(const char *s,bool truncateThis)
	{
		uint i;
		CSString result;
		for (i=0;i<size();++i)
		{
			// perform a quick string compare
			int j;
			for (j=0;s[j]!=0 && s[j]==(&((*this)[i]))[j];++j)
			{
			}
			// if string compare matched then return result so far
			if (s[j]==0)
			{
				// remove the result string from the input string if so desired
				if (truncateThis)
				{
					if (i+1<size())
						(*this)=substr(i+1);	// +1 to skip the separator character
					else
						clear();
				}

				return result;
			}
			result+=(*this)[i];
		}
		// we didn't find the separator string so we're returning a copy of the whole string
		if (truncateThis)
			clear();
		return result;
	}

	CSString CSString::splitFrom(char c) const
	{
		CSString result;
		std::string::const_iterator it;
		for (it=begin();it!=end() && *it!=c;++it)
		{}
		if (it!=end())
		{
			++it;
			for (;it!=end();++it)
				result+=*it;
		}
		return result;
	}

	CSString CSString::splitFrom(const char *s) const
	{
		uint i;
		CSString result;
		for (i=0;i<size();++i)
		{
			// perform a quick string compare
			uint j;
			for (j=0;i+j<size() && s[j]!=0 && s[j]==(*this)[i+j];++j)
			{
			}
			// if string compare matched then build and return a result
			if (s[j]==0)
			{
				result=substr(i+j);
				return result;
			}
		}
		return result;
	}


	CSString CSString::firstWord(bool truncateThis)
	{
		// idiot test to avoid accessing index 0 in empty strings
		if (empty())
			return CSString();

		CSString result;
		uint i=0;
		// skip white space
		for (i=0;i<size() && isWhiteSpace((*this)[i]);++i)
		{}

		if ( ((*this)[i]>='A' && (*this)[i]<='Z') || ((*this)[i]>='a' && (*this)[i]<='z') ||
			 ((*this)[i]>='0' && (*this)[i]<='9') || (*this)[i]=='_')
		{
			// copy out an alpha-numeric string
			for (;i<(*this).size() &&
				( ((*this)[i]>='A' && (*this)[i]<='Z') || ((*this)[i]>='a' && (*this)[i]<='z') ||
				  ((*this)[i]>='0' && (*this)[i]<='9') || (*this)[i]=='_')
				;++i)
				result+=(*this)[i];
		}
		else
		{
			// just take the first character of the input
			result=(*this)[i];
			++i;
		}

		// remove the result string from the input string if so desired
		if (truncateThis)
		{
			if (i<size())
				(*this)=substr(i);
			else
				clear();
		}

		return result;
	}

	CSString CSString::firstWordConst() const
	{
		return const_cast<CSString *>(this)->firstWord();
	}

	CSString CSString::tailFromFirstWord() const
	{
		CSString hold=*this;
		hold.firstWord(true);
		return hold;
	}

	unsigned CSString::countWords() const
	{
		unsigned count=0;
		CSString hold=strip();
		while (!hold.empty())
		{
			hold=hold.tailFromFirstWord().strip();
			++count;
		}
		return count;
	}

	CSString CSString::word(uint32 idx) const
	{
		CSString hold=strip();

		for (unsigned count=0;count<idx;++count)
			hold=hold.tailFromFirstWord().strip();

		return hold.firstWord();
	}

	CSString CSString::firstWordOrWords(bool truncateThis,bool useSlashStringEscape,bool useRepeatQuoteStringEscape)
	{
		uint32 startPos;
		for (startPos=0;startPos<size();++startPos)
			if (!isWhiteSpace((*this)[startPos]))
				break;

		if (isStringDelimiter((*this)[startPos]))
		{
			uint32 endPos= findMatchingDelimiterPos(false,useSlashStringEscape,useRepeatQuoteStringEscape,startPos);
			CSString result=substr(startPos,endPos-startPos+1);
			result=result.unquote(useSlashStringEscape,useRepeatQuoteStringEscape);
			if (truncateThis)
				*this=leftCrop(endPos+1);
			return result;
		}
		else
			return firstWord(truncateThis);
	}

	CSString CSString::firstWordOrWordsConst(bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
	{
		return const_cast<CSString *>(this)->firstWordOrWords(useSlashStringEscape,useRepeatQuoteStringEscape);
	}

	CSString CSString::tailFromFirstWordOrWords(bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
	{
		CSString hold=*this;
		hold.firstWordOrWords(true,useSlashStringEscape,useRepeatQuoteStringEscape);
		return hold;
	}

	unsigned CSString::countWordOrWords(bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
	{
		unsigned count=0;
		CSString hold=strip();
		while (!hold.empty())
		{
			hold=hold.tailFromFirstWordOrWords(useSlashStringEscape,useRepeatQuoteStringEscape).strip();
			++count;
		}
		return count;
	}

	CSString CSString::wordOrWords(uint32 idx,bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
	{
		CSString hold=strip();

		for (unsigned count=0;count<idx;++count)
			hold=hold.tailFromFirstWordOrWords(useSlashStringEscape,useRepeatQuoteStringEscape).strip();

		return hold.firstWordOrWords(useSlashStringEscape,useRepeatQuoteStringEscape);
	}


	CSString CSString::firstLine(bool truncateThis)
	{
		return splitTo('\n',truncateThis);
	}

	CSString CSString::firstLineConst() const
	{
		return const_cast<CSString *>(this)->firstLine();
	}

	CSString CSString::tailFromFirstLine() const
	{
		CSString hold=*this;
		hold.firstLine(true);
		return hold;
	}

	unsigned CSString::countLines() const
	{
		unsigned count=0;
		CSString hold=strip();
		while (!hold.empty())
		{
			hold=hold.tailFromFirstLine().strip();
			++count;
		}
		return count;
	}

	CSString CSString::line(uint32 idx) const
	{
		CSString hold=strip();

		for (unsigned count=0;count<idx;++count)
			hold= hold.tailFromFirstLine().strip();

		return hold.firstLine();
	}


	CSString CSString::quote(bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
	{
		CSString result;

		result+='\"';
		for (uint32 i=0;i<size();++i)
		{
			switch ((*this)[i])
			{
			case '\"':
				if (useSlashStringEscape)
				{
					result+="\\\"";
					continue;
				}
				else if (useRepeatQuoteStringEscape)
				{
					result+="\"\"";
					continue;
				}
				break;
			case '\\':	if (useSlashStringEscape)	{	result+="\\\\";	continue;	}	break;
			case '\a':	if (useSlashStringEscape)	{	result+="\\a";	continue;	}	break;
			case '\b':	if (useSlashStringEscape)	{	result+="\\b";	continue;	}	break;
			case '\f':	if (useSlashStringEscape)	{	result+="\\f";	continue;	}	break;
			case '\n':	if (useSlashStringEscape)	{	result+="\\n";	continue;	}	break;
			case '\r':	if (useSlashStringEscape)	{	result+="\\r";	continue;	}	break;
			case '\t':	if (useSlashStringEscape)	{	result+="\\t";	continue;	}	break;
			case '\v':	if (useSlashStringEscape)	{	result+="\\v";	continue;	}	break;
				break;
			default:
				if ((signed char)(*this)[i]<32 && useSlashStringEscape)
				{
					result+=NLMISC::toString("\\x%02x",(unsigned char)(*this)[i]);
					continue;
				}
				break;
			}
			result+=(*this)[i];
		}
		result+='\"';

		return result;
	}

	CSString CSString::quoteIfNotQuoted(	bool useSlashStringEscape, bool useRepeatQuoteStringEscape ) const
	{
		if (empty()||(*this)[0]!='\"'||!isDelimitedMonoBlock(false,useSlashStringEscape,useRepeatQuoteStringEscape))
			return quote(useSlashStringEscape,useRepeatQuoteStringEscape);

		return *this;
	}

	CSString CSString::quoteIfNotAtomic(	bool useSlashStringEscape, bool useRepeatQuoteStringEscape ) const
	{
		if (empty())
			return "\"\"";

		uint32 i=1;
		if (((*this)[0]>='0' && (*this)[0]<='9')||(*this)[0]=='-')
		{
			for (i=1;i<size();++i)
				if ((*this)[i]<'0' || (*this)[i]>'9')
					break;
		}
		else if ( CSString::isValidKeywordFirstChar((*this)[0]) )
		{
			for (i=1;i<size();++i)
				if (!CSString::isValidFileNameChar((*this)[i]))
					break;
		}
		else if ((*this)[0]=='\"' && isDelimitedMonoBlock(false,useSlashStringEscape,useRepeatQuoteStringEscape))
		{
			i=(uint32)size();
		}
		if (i!=size())
			return quote(useSlashStringEscape,useRepeatQuoteStringEscape);

		return *this;
	}

	CSString CSString::unquote(bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
	{
		CSString result=stripBlockDelimiters(false,useSlashStringEscape,useRepeatQuoteStringEscape);
		uint32 i,j;
		for (i=0,j=0;i<result.size();++i,++j)
		{
			if (useSlashStringEscape && result[i]=='\\')
			{
				++i;
				if (i<result.size())
				{
					switch(result[i])
					{
					case 'a': result[i]='\a'; break;
					case 'b': result[i]='\b'; break;
					case 'f': result[i]='\f'; break;
					case 'n': result[i]='\n'; break;
					case 'r': result[i]='\r'; break;
					case 't': result[i]='\t'; break;
					case 'v': result[i]='\v'; break;

					case '0':
					case '1':
					case '2':
					case '3':
						{
							char hold=result[i]-'0';
							++i;
							if (i<result.size() && result[i]>='0' && result[i]<='7')
							{
								hold=8*hold+(result[i]-'0');
								++i;
								if (i<result.size() && result[i]>='0' && result[i]<='7')
								{
									hold=8*hold+(result[i]-'0');
									++i;
								}
							}
							result[j]=hold;
							continue;
						}
						break;

					case '4':
					case '5':
					case '6':
					case '7':
						{
							char hold=result[i]-'0';
							++i;
							if (i<result.size() && result[i]>='0' && result[i]<='7')
							{
								hold=8*hold+(result[i]-'0');
								++i;
							}
							result[j]=hold;
							continue;
						}
						break;

					case 'x':
						if (i+1<result.size() && isHexDigit(result[i+1]))
						{
							char hold=convertHexDigit(result[i+1]);
							i+=2;
							if (i<result.size() && isHexDigit(result[i]))
							{
								hold=16*hold+convertHexDigit(result[i]);
								++i;
							}
							result[j]=hold;
							continue;
						}
						break;
					}
				}
			}
			else if (useRepeatQuoteStringEscape && (i+1<result.size()) && result[i]=='\"' && result[i+1]=='\"')
				++i;
			if (i<result.size())
				result[j]=result[i];
		}
		if (i!=j)
			result.resize(j);

		return result;
	}

	CSString CSString::unquoteIfQuoted(bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
	{
		if (isQuoted())
			return unquote(useSlashStringEscape,useRepeatQuoteStringEscape);
		else
			return *this;
	}

	CSString CSString::encodeXML(bool isParameter) const
	{
		CSString result;

		for (uint32 i=0;i<size();++i)
		{
			unsigned char c= (*this)[i];
			switch(c)
			{
			// special xml characters
			case '\"':	result+="&quot;";	continue;
			case '&':	result+="&amp;";	continue;
			case '<':	result+="&lt;";		continue;
			case '>':	result+="&gt;";		continue;

			// characters that are not allowed inside a parameter block
			case '\n':
			case '\r':
			case '\t':
				if (!isParameter) { result+=c; continue; }
			}

			// hex coding for extended characters
			if (c<32 || c>127)
			{
				result+="&#x";
				result+= ((c>>4)>=10? 'A'+(c>>4)-10: '0'+(c>>4));
				result+= ((c&15)>=10? 'A'+(c&15)-10: '0'+(c&15));
				result+=";";
				continue;
			}

			// all the special cases are catered for... treat this as a normal character
			result+=c;
		}

		return result;
	}

	CSString CSString::decodeXML() const
	{
		CSString result;

		for (uint32 i=0;i<size();++i)
		{
			breakable
			{
				if((*this)[i]=='&')
				{
					// special xml characters
					if (substr(i+1,5)=="quot;")	{ i+=5; result+='\"'; continue; }
					if (substr(i+1,4)=="amp;")	{ i+=4; result+='&'; continue; }
					if (substr(i+1,3)=="lt;")	{ i+=3; result+='<'; continue; }
					if (substr(i+1,3)=="gt;")	{ i+=3; result+='>'; continue; }

					// hex coding for extended characters
					if ((size()-i)>=5)
					{
						if ((*this)[i+1]!='#') break;
						if (((*this)[i+2]|('a'-'A'))!='x') break;
						// setup j to point at the first character following "&#x"[0-9|a-f]*
						uint32 j; for (j=i+3;j<size();++j) if (!isHexDigit((*this)[j])) break;
						// make sure that at least 1 hex character was found
						if (j==i+3) break;
						// make sure have the terminating ';' to complete the token: "&#x"[0-9|a-f]*";"
						if (j>=size() || (*this)[j]!=';') break;
						// make sure that the value we have is only one or 2 hex digits
						if (j>=i+6) nlwarning("Truncating extended char code '%s",(substr(i,j-i+1)+"' => using "+substr(j-2,2)).c_str());

						// convert the 1 or 2 last hex digits to a char value
						char c=0;
						if (j>=i+5)
						{
							// if we have 2 or more digits then grab the high digit
							c+= convertHexDigit( (*this)[j-2] )*16;
						}
						c+= convertHexDigit( (*this)[j-1] );

						// append our new character to the result string
						result+=c;
						// move 'i' forward to point at the ';' .. the for(...) will increment i to point to next char
						i=j;
						continue;
					}
				}
			}

			// all the special cases are catered for... treat this as a normal character
			result+=(*this)[i];
		}

		return result;
	}

	bool CSString::isEncodedXML() const
	{
		bool foundToken= false;

		for (uint32 i=(uint32)size();i--;)
		{
			switch((*this)[i])
			{
			// decoded special xml characters
			case '\"':
			case '<':
			case '>':
			case '&':
				return false;

			case ';':
				// encoded special xml characters
				if (i>=5 && substr(i-5,5)=="&quot")	{ foundToken= true; i-=5; break; }
				if (i>=4 && substr(i-4,4)=="&amp")	{ foundToken= true; i-=4; break; }
				if (i>=3 && substr(i-3,3)=="&lt")	{ foundToken= true; i-=3; break; }
				if (i>=3 && substr(i-3,3)=="&gt")	{ foundToken= true; i-=3; break; }

				// hex coding for extended characters
				if (i>=3 && isHexDigit((*this)[i-1]))
				{
					uint32 j,k;
					// locate the start of a potential hex string
					for (j=i;j--;)
						if ((*this)[j]=='&')
							break;
					// make sure that at least 5 chars were found for: &#x0;
					if (i-j<4) continue;
					// make sure we have '&#x' at the start
					if ((*this)[j]!='&') continue;
					if ((*this)[j+1]!='#') continue;
					if ((*this)[j+2]!='x') continue;
					// ensure that the remainder between the leading '&#x' and trailing ';' are hex digits
					for (k=j+3;k<i;++k)
						if (!isHexDigit((*this)[k]))
							break;
					if (k!=i) continue;
					// skip the characters that were matched - i now refs the opening '&'
					i=j;
					foundToken= true;
					break;
				}
			}
		}

		return foundToken;
	}

	bool CSString::isXMLCompatible(bool isParameter) const
	{
		for (uint32 i=(uint32)size();i--;)
		{
			switch((*this)[i])
			{
			// decoded special xml characters
			case '\"':
			case '<':
			case '>':
			case '&':
				return false;

			case ';':
				// encoded special xml characters
				if (i>=5 && substr(i-5,5)=="&quot")	{ i-=5; continue; }
				if (i>=4 && substr(i-4,4)=="&amp")	{ i-=4; continue; }
				if (i>=3 && substr(i-3,3)=="&lt")	{ i-=3; continue; }
				if (i>=3 && substr(i-3,3)=="&gt")	{ i-=3; continue; }

				// hex coding for extended characters
				if (i>=3 && isHexDigit((*this)[i-1]))
				{
					uint32 j,k;
					// locate the start of a potential hex string
					for (j=i;j--;)
						if ((*this)[j]=='&')
							break;
					// make sure that at least 5 chars were found for: &#x0;
					if (i-j<4) continue;
					// make sure we have '&#x' at the start
					if ((*this)[j]!='&') continue;
					if ((*this)[j+1]!='#') continue;
					if ((*this)[j+2]!='x') continue;
					// ensure that the remainder between the leading '&#x' and trailing ';' are hex digits
					for (k=j+3;k<i;++k)
						if (!isHexDigit((*this)[k]))
							break;
					if (k!=i) continue;
					// skip the characters that were matched - i now refs the opening '&'
					i=j;
					continue;
				}

			// characters that are not allowed inside a parameter block
			case '\n':
			case '\r':
			case '\t':
				if (!isParameter) continue;
			}

			if ((uint8)((*this)[i])>127 || (uint8)((*this))[i]<32)
				return false;
		}

		return true;
	}

	CSString CSString::replace(const char *toFind,const char *replacement) const
	{
		// just bypass the problems that can cause a crash...
		if (toFind==NULL || *toFind==0)
			return *this;

		std::string::size_type i,j;
		CSString result;
		for (i=0;i<size();)
		{
			// string compare toFind against (*this)+i ...
			for (j=0;toFind[j];++j)
				if ((*this)[i+j]!=toFind[j])
					break;
			// if strings were identical then j reffers to ASCIIZ terminator at end of 'toFind'
			if (toFind[j]==0)
			{
				if (replacement!=NULL)
					result+=replacement;
				i+=j;
			}
			else
			{
				result+=(*this)[i];
				++i;
			}
		}
		return result;
	}

	std::string::size_type CSString::find(const char *toFind, std::string::size_type startLocation) const
	{
	//	const char *constStr = c_str();

		// just bypass the problems that can cause a crash...
		if (toFind==NULL || *toFind==0 || startLocation>=size())
			return std::string::npos;

		std::string::size_type i,j;
		for (i=startLocation;i<size();++i)
		{
			// string compare toFind against (*this)+i ...
			for (j=0;toFind[j];++j)
				if ((i+j>=size()) || (*this)[i+j]!=toFind[j])
					break;
			// if strings were identical then we're done
			if (toFind[j]==0)
				return i;
		}
		return std::string::npos;
	}

	/// Find index at which a sub-string starts (case NOT sensitive) - if sub-string not found then returns string::npos
	std::string::size_type CSString::findNS(const char *toFind, std::string::size_type startLocation) const
	{
		const char *constStr = c_str();

		// just bypass the problems that can cause a crash...
		if (toFind==NULL || *toFind==0 || startLocation>=size())
			return std::string::npos;

		std::string::size_type i,j;
		for (i=startLocation;i<size();++i)
		{
			// string compare toFind against (*this)+i ...
			for (j=0;toFind[j];++j)
				if ((i+j>=size()) || tolower(constStr[i+j])!=tolower(toFind[j]))
					break;
			// if strings were identical then we're done
			if (toFind[j]==0)
				return i;
		}
		return std::string::npos;
	}

	bool CSString::contains(const char *toFind) const
	{
		return find(toFind)!=std::string::npos;
	}

	bool CSString::contains(int character) const
	{
		for (const_iterator it=begin();it!=end();++it)
			if ((*it)==character)
				return true;

		return false;
	}

	static const uint32 MaxUint32= ~0u;
	static const uint32 MaxUint32LastDigit= MaxUint32-(MaxUint32/10)*10;
	static const uint32 MaxUint32PreLastDigit= (MaxUint32/10);

	static const uint32 MaxNegSint32= ~0u/2+1;
	static const uint32 MaxNegSint32LastDigit= MaxNegSint32-(MaxNegSint32/10)*10;
	static const uint32 MaxNegSint32PreLastDigit= (MaxNegSint32/10);

	static const uint32 MaxPosSint32= ~0u/2;
	static const uint32 MaxPosSint32LastDigit= MaxPosSint32-(MaxPosSint32/10)*10;
	static const uint32 MaxPosSint32PreLastDigit= (MaxPosSint32/10);

	int CSString::atoi() const
	{
		if (empty())
			return 0;

		bool neg= false;
		uint32 result;
		switch (*begin())
		{
			case '+':	result=0; break;
			case '-':	result=0; neg=true; break;
			case '0':	result=0; break;
			case '1':	result=1; break;
			case '2':	result=2; break;
			case '3':	result=3; break;
			case '4':	result=4; break;
			case '5':	result=5; break;
			case '6':	result=6; break;
			case '7':	result=7; break;
			case '8':	result=8; break;
			case '9':	result=9; break;
			default:	return 0;
		}

		for (const_iterator it=begin()+1;it!=end();++it)
		{
			uint32 offset;
			switch (*it)
			{
				case '0':	offset=0; break;
				case '1':	offset=1; break;
				case '2':	offset=2; break;
				case '3':	offset=3; break;
				case '4':	offset=4; break;
				case '5':	offset=5; break;
				case '6':	offset=6; break;
				case '7':	offset=7; break;
				case '8':	offset=8; break;
				case '9':	offset=9; break;
				default:	return 0;
			}
			if (!neg)
			{
				if (result>=MaxUint32PreLastDigit/*~0u/10*/)
				{
					if (result>MaxUint32PreLastDigit || offset>MaxUint32LastDigit)
						return 0;
				}
			}
			else
			{
				if (result>=MaxNegSint32PreLastDigit /*~0u/20+1*/)
				{
					if (result>MaxNegSint32PreLastDigit || offset>MaxNegSint32LastDigit)
						return 0;
				}
			}
			result=10*result+offset;
		}
		return neg? -(sint32)result: (sint32)result;
	}

	sint32 CSString::atosi() const
	{
		if (empty())
			return 0;

		bool neg= false;
		uint32 result;
		switch (*begin())
		{
			case '+':	result=0; break;
			case '-':	result=0; neg=true; break;
			case '0':	result=0; break;
			case '1':	result=1; break;
			case '2':	result=2; break;
			case '3':	result=3; break;
			case '4':	result=4; break;
			case '5':	result=5; break;
			case '6':	result=6; break;
			case '7':	result=7; break;
			case '8':	result=8; break;
			case '9':	result=9; break;
			default:	return 0;
		}

		for (const_iterator it=begin()+1;it!=end();++it)
		{
			uint32 offset;
			switch (*it)
			{
				case '0':	offset=0; break;
				case '1':	offset=1; break;
				case '2':	offset=2; break;
				case '3':	offset=3; break;
				case '4':	offset=4; break;
				case '5':	offset=5; break;
				case '6':	offset=6; break;
				case '7':	offset=7; break;
				case '8':	offset=8; break;
				case '9':	offset=9; break;
				default:	return 0;
			}
			if (result>=MaxPosSint32PreLastDigit /*~0u/20*/)
			{
				if (result>MaxPosSint32PreLastDigit || offset>(neg?MaxNegSint32LastDigit:MaxPosSint32LastDigit))
					return 0;
			}
			result=10*result+offset;
		}
		return neg? -(sint32)result: (sint32)result;
	}

	uint32 CSString::atoui() const
	{
		uint32 result=0;
		for (const_iterator it=begin();it!=end();++it)
		{
			uint32 offset;
			switch (*it)
			{
			case '0':	offset=0; break;
			case '1':	offset=1; break;
			case '2':	offset=2; break;
			case '3':	offset=3; break;
			case '4':	offset=4; break;
			case '5':	offset=5; break;
			case '6':	offset=6; break;
			case '7':	offset=7; break;
			case '8':	offset=8; break;
			case '9':	offset=9; break;
			default:	return 0;
			}
			if (result>=MaxUint32PreLastDigit/*~0u/10*/)
			{
				if (result>MaxUint32PreLastDigit || offset>MaxUint32LastDigit)
					return 0;
			}
			result=10*result+offset;
		}
		return result;
	}

	static const uint64 MaxUint64= (uint64)0-(uint64)1;
	static const uint64 MaxUint64LastDigit= MaxUint64-(MaxUint64/10)*10;
	static const uint64 MaxUint64PreLastDigit= (MaxUint64/10);

	static const uint64 MaxNegSint64= ((uint64)0-(uint64)1)/2+1;
	static const uint64 MaxNegSint64LastDigit= MaxNegSint64-(MaxNegSint64/10)*10;
	static const uint64 MaxNegSint64PreLastDigit= (MaxNegSint64/10);

	static const uint64 MaxPosSint64= ((uint64)0-(uint64)1)/2;
	static const uint64 MaxPosSint64LastDigit= MaxPosSint64-(MaxPosSint64/10)*10;
	static const uint64 MaxPosSint64PreLastDigit= (MaxPosSint64/10);

	sint64 CSString::atoi64() const
	{
		if (empty())
			return 0;

		bool neg= false;
		uint64 result;
		switch (*begin())
		{
			case '+':	result=0; break;
			case '-':	result=0; neg=true; break;
			case '0':	result=0; break;
			case '1':	result=1; break;
			case '2':	result=2; break;
			case '3':	result=3; break;
			case '4':	result=4; break;
			case '5':	result=5; break;
			case '6':	result=6; break;
			case '7':	result=7; break;
			case '8':	result=8; break;
			case '9':	result=9; break;
			default:	return 0;
		}

		for (const_iterator it=begin()+1;it!=end();++it)
		{
			uint64 offset;
			switch (*it)
			{
				case '0':	offset=0; break;
				case '1':	offset=1; break;
				case '2':	offset=2; break;
				case '3':	offset=3; break;
				case '4':	offset=4; break;
				case '5':	offset=5; break;
				case '6':	offset=6; break;
				case '7':	offset=7; break;
				case '8':	offset=8; break;
				case '9':	offset=9; break;
				default:	return 0;
			}
			if (!neg)
			{
				if (result>=MaxUint64PreLastDigit/*~0u/10*/)
				{
					if (result>MaxUint64PreLastDigit || offset>MaxUint64LastDigit)
						return 0;
				}
			}
			else
			{
				if (result>=MaxNegSint64PreLastDigit /*~0u/20+1*/)
				{
					if (result>MaxNegSint64PreLastDigit || offset>MaxNegSint64LastDigit)
						return 0;
				}
			}
			result=10*result+offset;
		}
		return neg? -(sint64)result: (sint64)result;
	}

	sint64 CSString::atosi64() const
	{
		if (empty())
			return 0;

		bool neg= false;
		uint64 result;
		switch (*begin())
		{
			case '+':	result=0; break;
			case '-':	result=0; neg=true; break;
			case '0':	result=0; break;
			case '1':	result=1; break;
			case '2':	result=2; break;
			case '3':	result=3; break;
			case '4':	result=4; break;
			case '5':	result=5; break;
			case '6':	result=6; break;
			case '7':	result=7; break;
			case '8':	result=8; break;
			case '9':	result=9; break;
			default:	return 0;
		}

		for (const_iterator it=begin()+1;it!=end();++it)
		{
			uint64 offset;
			switch (*it)
			{
				case '0':	offset=0; break;
				case '1':	offset=1; break;
				case '2':	offset=2; break;
				case '3':	offset=3; break;
				case '4':	offset=4; break;
				case '5':	offset=5; break;
				case '6':	offset=6; break;
				case '7':	offset=7; break;
				case '8':	offset=8; break;
				case '9':	offset=9; break;
				default:	return 0;
			}
			if (result>=MaxPosSint64PreLastDigit /*~0u/20*/)
			{
				if (result>MaxPosSint64PreLastDigit || offset>(neg?MaxNegSint64LastDigit:MaxPosSint64LastDigit))
					return 0;
			}
			result=10*result+offset;
		}
		return neg? -(sint64)result: (sint64)result;
	}

	uint64 CSString::atoui64() const
	{
		uint64 result=0;
		for (const_iterator it=begin();it!=end();++it)
		{
			uint64 offset;
			switch (*it)
			{
			case '0':	offset=0; break;
			case '1':	offset=1; break;
			case '2':	offset=2; break;
			case '3':	offset=3; break;
			case '4':	offset=4; break;
			case '5':	offset=5; break;
			case '6':	offset=6; break;
			case '7':	offset=7; break;
			case '8':	offset=8; break;
			case '9':	offset=9; break;
			default:	return 0;
			}
			if (result>=MaxUint64PreLastDigit/*~0u/10*/)
			{
				if (result>MaxUint64PreLastDigit || offset>MaxUint64LastDigit)
					return 0;
			}
			result=10*result+offset;
		}
		return result;
	}

	double CSString::atof() const
	{
		return ::atof(c_str());
	}

	bool CSString::readFromFile(const CSString& fileName)
	{
		FILE* file;
		file=fopen(fileName.c_str(),"rb");
		if (file==NULL)
		{
			clear();
			// There was previously a warning displayed here but that was incorrect as it is defined that refaFromFile returns an empty result if the file is not found
			// nlwarning("Failed to open file for reading: %s",fileName.c_str());
			return false;
		}
		resize(NLMISC::CFile::getFileSize(file));
		uint32 bytesRead=(uint32)fread(const_cast<char*>(data()),1,size(),file);
		fclose(file);
		if (bytesRead!=size())
		{
			resize(bytesRead);
			nlwarning("Failed to read file contents (requested %u bytes but fread returned %u) for file:%s",size(),bytesRead,fileName.c_str());
			return false;
		}
		return true;
	}

	bool CSString::writeToFile(const CSString& fileName) const
	{
		FILE* file;
		file=fopen(fileName.c_str(),"wb");
		if (file==NULL)
		{
			nlwarning("Failed to open file for writing: %s",fileName.c_str());
			return false;
		}
		uint32 recordsWritten=(uint32)fwrite(const_cast<char*>(data()),size(),1,file);
		fclose(file);
		if (recordsWritten!=1)
		{
			nlwarning("Failed to write file contents (requested %u bytes but fwrite returned %u) for file:%s",size(),recordsWritten,fileName.c_str());
			return false;
		}
		nldebug("CSSWTF Wrote %u bytes to file %s",size(),fileName.c_str());
		return true;
	}

	bool CSString::writeToFileIfDifferent(const CSString& fileName) const
	{
		// if the file exists...
		if (NLMISC::CFile::fileExists(fileName))
		{
			// the file exists so check it's the right size
			if (NLMISC::CFile::getFileSize(fileName)==size())
			{
				// the file is the right size so read its data from disk...
				CSString hold;
				hold.readFromFile(fileName);
				// check whether data read from file and our own data are identical
				if (hold.size()==size() && memcmp(&hold[0],&(*this)[0],size())==0)
				{
					// data is identical so drop out
					nldebug("CSSWTF Request to write data to file %s IGNORED because file already contains correct data",fileName.c_str());
					return true;
				}
			}
		}

		// the file didn't already exist or content
		return writeToFile(fileName);
	}

} // namespace NLMISC



