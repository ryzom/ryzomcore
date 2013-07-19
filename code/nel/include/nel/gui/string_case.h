#ifndef STRING_CASE_H
#define STRING_CASE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/ucstring.h"

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


	void setCase( ucstring &str, TCaseMode mode );


}

#endif


