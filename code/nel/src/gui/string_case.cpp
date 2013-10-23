// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "nel/gui/string_case.h"

namespace NLGUI
{
	inline bool isSeparator (ucchar c)
	{
		return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
	}

	// ***************************************************************************

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

}



