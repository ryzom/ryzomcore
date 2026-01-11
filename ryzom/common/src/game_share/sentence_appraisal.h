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



#ifndef RY_DIFFICULTY_H
#define RY_DIFFICULTY_H

#include "nel/misc/types_nl.h"

namespace SENTENCE_APPRAISAL
{
	// Mode
	enum ESentenceAppraisal
	{
		Undefined = 0,
		// related to difficulty
		ChildPlay,
		RealEasy,
		Easy,
		FairlyEasy,
		Average,			// 5
		QuiteDifficult,
		Difficult,
		ReallyDifficult,
		Harsh,
		ExtremelyDifficult,	 // 10

		// text not related to difficulty but displayed in the difficulty field on clients
		FeatureUnderConstruction,
		InvalidSentence,
		ErrorCreating,
		Uncomplete,
		Cheater,			// 15

		// max value is 15 so only use 4 bits in the database, if more values are added don't forget to allow more bits in DB
	};


	/**
	 * get the right enum value from the input string
	 * \param str the input string
	 * \return the ESentenceAppraisal associated to this string (Undefined if the string cannot be interpreted)
	 */
	ESentenceAppraisal toAppraisal(const std::string &str);

	/**
	 * get the string associated to a brick type
	 * \param type the ESentenceAppraisal to convert into a string
	 * \return the enum as a string
	 */
	const std::string &toString(ESentenceAppraisal type);

}; // SENTENCE_APPRAISAL

#endif // SENTENCE_APPRAISAL
/* End of sentence_appraisal.h */
