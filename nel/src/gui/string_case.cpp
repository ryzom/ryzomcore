// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

	inline bool isEndSentence (u32char c, u32char lastChar)
	{
		// Ex: One sentence. Another sentence.
		//                  ^
		// Counterexample: nevrax.com
		//                       ^
		return ((c == (u32char)' ') || (c == (u32char)'\n'))
			&& (lastChar == (u32char)'.') || (lastChar == (u32char)'!') || (lastChar == (u32char)'?');
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
			str = NLMISC::toLower(str);
			break;
		case CaseUpper:
			str = NLMISC::toUpper(str);
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

/* end of file */
