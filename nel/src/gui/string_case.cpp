// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#include "stdpch.h"
#include "nel/gui/string_case.h"
#include "nel/misc/utf_string_view.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	inline bool isSeparator (u32char c)
	{
		return (c == (u32char)' ') || (c == (u32char)'\t') || (c == (u32char)'\n') || (c == (u32char)'\r');
	}

	inline bool isSeparator (ucchar c)
	{
		return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
	}

	inline bool isSeparator (char c)
	{
		return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
	}

	// ***************************************************************************

	inline bool isEndSentence (u32char c, u32char lastChar)
	{
		// Ex: One sentence. Another sentence.
		//                  ^
		// Counterexample: nevrax.com
		//                       ^
		return ((c == (u32char)' ') || (c == (u32char)'\n'))
			&& (lastChar == (u32char)'.') || (lastChar == (u32char)'!') || (lastChar == (u32char)'?');
	}

	inline bool isEndSentence (ucstring& str, uint index)
	{
		// Ex: One sentence. Another sentence.
		//                  ^
		// Counterexample: nevrax.com
		//                       ^
		ucchar c = str[index];
		if ((str[index] == ' ') || (str[index] == '\n'))
		{
			if (index < 1)
				return false;
			c = str[index-1];
			return (c == '.') || (c == '!') || (c == '?');
		}
		return false;
	}


	void setCase( ucstring &str, TCaseMode mode )
	{
		const uint length = (uint)str.length();
		uint i;
		bool newString = true;
		bool newSentence = true;
		bool newWord = true;
		switch (mode)
		{
		case CaseLower:
			str = NLMISC::toLower (str);
			break;
		case CaseUpper:
			str = NLMISC::toUpper (str);
			break;
		case CaseFirstStringLetterUp:
			for (i=0; i<length; i++)
			{
				if (!isSeparator (str[i]))
				{
					if (newString)
						str[i] = NLMISC::toUpper (str[i]);
					else
						str[i] = NLMISC::toLower (str[i]);
					newString = false;
				}
			}
			break;
		case CaseFirstSentenceLetterUp:
			for (i=0; i<length; i++)
			{
				if (isEndSentence (str, i))
					newSentence = true;
				else
				{
					if (newSentence)
						str[i] = NLMISC::toUpper (str[i]);
					else
						str[i] = NLMISC::toLower (str[i]);

					if (!isSeparator (str[i]))
						newSentence = false;
				}
			}
			break;
		case CaseFirstWordLetterUp:
			for (i=0; i<length; i++)
			{
				if (isSeparator (str[i]) || isEndSentence (str, i))
					newWord = true;
				else
				{
					if (newWord)
						str[i] = NLMISC::toUpper (str[i]);
					else
						str[i] = NLMISC::toLower (str[i]);

					newWord = false;
				}
			}
			break;
		default:
			break;
		}
	}

	void setCase(std::string &str, TCaseMode mode)
	{
		const uint length = (uint)str.length();
		uint i;
		bool newString = true;
		bool newSentence = true;
		bool newWord = true;
		switch (mode)
		{
		case CaseLower:
			str = NLMISC::toLowerAsUtf8(str);
			break;
		case CaseUpper:
			str = NLMISC::toUpperAsUtf8(str);
			break;
		case CaseFirstStringLetterUp:
		{
			NLMISC::CUtfStringView sv(str);
			std::string res;
			res.reserve(sv.largestSize());
			for (NLMISC::CUtfStringView::iterator it(sv.begin()), end(sv.end()); it != end; ++it)
			{
				u32char c = *it;
				if (c < 0x10000)
				{
					if (!isSeparator(c))
					{
						if (newString)
							c = NLMISC::toUpper((ucchar)c);
						else
							c = NLMISC::toLower((ucchar)c);
						newString = false;
					}
				}
				NLMISC::CUtfStringView::append(res, c);
			}
			str = nlmove(res);
			break;
		}
		case CaseFirstSentenceLetterUp:
		{
			NLMISC::CUtfStringView sv(str);
			std::string res;
			res.reserve(sv.largestSize());
			u32char lastChar = 0;
			for (NLMISC::CUtfStringView::iterator it(sv.begin()), end(sv.end()); it != end; ++it)
			{
				u32char c = *it;
				if (c < 0x10000)
				{
					if (isEndSentence(c, lastChar))
						newSentence = true;
					else
					{
						if (newSentence)
							c = NLMISC::toUpper((ucchar)c);
						else
							c = NLMISC::toLower((ucchar)c);

						if (!isSeparator(c))
							newSentence = false;
					}
				}
				NLMISC::CUtfStringView::append(res, c);
				lastChar = c;
			}
			str = nlmove(res);
			break;
		}
		case CaseFirstWordLetterUp:
		{
			NLMISC::CUtfStringView sv(str);
			std::string res;
			res.reserve(sv.largestSize());
			u32char lastChar = 0;
			for (NLMISC::CUtfStringView::iterator it(sv.begin()), end(sv.end()); it != end; ++it)
			{
				u32char c = *it;
				if (c < 0x10000)
				{
					if (isSeparator(c) || isEndSentence(c, lastChar))
						newWord = true;
					else
					{
						if (newWord)
							c = NLMISC::toUpper((ucchar)c);
						else
							c = NLMISC::toLower((ucchar)c);

						newWord = false;
					}
				}
				NLMISC::CUtfStringView::append(res, c);
				lastChar = c;
			}
			str = nlmove(res);
			break;
		}
		default:
			break;
		}
	}

}



