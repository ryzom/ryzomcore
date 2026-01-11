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
#include "sentence_appraisal.h"
// nel
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace SENTENCE_APPRAISAL
{
	// The conversion table
	const CStringConversion<ESentenceAppraisal>::CPair stringTable [] =
	{
		{ "ChildPlay", ChildPlay },
		{ "RealEasy", RealEasy },
		{ "Easy", Easy },
		{ "FairlyEasy", FairlyEasy },
		{ "Average", Average },
		{ "QuiteDifficult", QuiteDifficult },
		{ "Difficult", Difficult },
		{ "ReallyDifficult", ReallyDifficult },
		{ "Harsh", Harsh },
		{ "ExtremelyDifficult", ExtremelyDifficult },

		{ "FeatureUnderConstruction", FeatureUnderConstruction },
		{ "InvalidSentence", InvalidSentence },
		{ "ErrorCreating", ErrorCreating },
		{ "Uncomplete", Uncomplete },
		{ "Cheater", Cheater },

		{ "Undefined", Undefined },
	};

	CStringConversion<ESentenceAppraisal> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  Undefined);

// convert type id to type name string
const std::string& toString( ESentenceAppraisal type )
{
	return conversion.toString(type);
}

// convert type name to type enum value
ESentenceAppraisal toAppraisal( const std::string& str )
{
	return conversion.fromString(str);
}


} // SENTENCE_APPRAISAL
