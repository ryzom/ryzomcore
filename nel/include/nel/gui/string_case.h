// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
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

#ifndef STRING_CASE_H
#define STRING_CASE_H

#include "nel/misc/types_nl.h"

namespace NLGUI
{
	enum TCaseMode
	{
		CaseNormal = 0,					// Nothing done
		CaseLower,						// All letters in lowercase
		CaseUpper,						// All letters in uppercase
		CaseFirstStringLetterUp,		// The first letter of the string is uppercase, the others are lowercase
		CaseFirstSentenceLetterUp,		// The first letter of the string and each sentences are uppercase, the others are lowercase. Sentences are seprated with '.'.
		CaseFirstWordLetterUp,			// The first letter of each word is uppercase, the others are lowercase
		CaseCount
	};

	void setCase( std::string &str, TCaseMode mode );
}

#endif


