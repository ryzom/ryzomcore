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
	inline bool isSeparator (char c)
	{
		return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
	}

	inline bool isEndSentence (char c, char lastChar)
	{
		// Ex: One sentence. Another sentence.
		//                  ^
		// Counterexample: nevrax.com
		//                       ^
		return ((c == ' ') || (c == '\n'))
			&& ((lastChar == '.') || (lastChar == '!') || (lastChar == '?') || (lastChar == '\n'));
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
			std::string res;
			res.reserve(str.size() + (str.size() >> 2));
			for (ptrdiff_t i = 0; i < (ptrdiff_t)str.size();)
			{
				char c = str[i];
				if (!isSeparator(c))
				{
					if (newString)
						NLMISC::appendToTitle(res, str, i);
					else
						NLMISC::appendToLower(res, str, i);
					newString = false;
				}
				else
				{
					res += c;
					++i;
				}
			}
			str.swap(res);
			break;
		}
		case CaseFirstSentenceLetterUp:
		{
			std::string res;
			res.reserve(str.size() + (str.size() >> 2));
			char lastChar = 0;
			for (ptrdiff_t i = 0; i < (ptrdiff_t)str.size();)
			{
				char c = str[i];
				if (isEndSentence(c, lastChar))
				{
					newSentence = true;
					res += c;
					++i;
				}
				else
				{
					if (newSentence)
						NLMISC::appendToTitle(res, str, i);
					else
						NLMISC::appendToLower(res, str, i);

					if (!isSeparator(c))
						newSentence = false;
				}
				lastChar = c;
			}
			str.swap(res);
			break;
		}
		case CaseFirstWordLetterUp:
		{
			std::string res;
			res.reserve(str.size() + (str.size() >> 2));
			char lastChar = 0;
			for (ptrdiff_t i = 0; i < (ptrdiff_t)str.size();)
			{
				char c = str[i];
				if (isSeparator(c) || isEndSentence(c, lastChar))
				{
					newWord = true;
					res += c;
					++i;
				}
				else
				{
					if (newWord)
						NLMISC::appendToTitle(res, str, i);
					else
						NLMISC::appendToLower(res, str, i);

					newWord = false;
				}
				lastChar = c;
			}
			str.swap(res);
			break;
		}
		default:
			break;
		}
	}
}

/* end of file */
