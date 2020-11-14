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

#ifndef NL_U_FORM_LOADER_H
#define NL_U_FORM_LOADER_H

#include "nel/misc/types_nl.h"


namespace NLGEORGES
{

class UType;
class UForm;
class UFormDfn;

/**
 * Georges form loader interface
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class UFormLoader
{
public:
	virtual ~UFormLoader() {}

	/** Load a form, use NMISC::CPath to find the file.
	  *
	  * The pointer on the form must be held in a CSmartPtr<UForm>. Returns NULL if the form can't be loaded.
	  */
	virtual UForm *loadForm (const std::string &filename) = 0;

	/** Load a DFN, use NMISC::CPath to find the file.
	  *
	  * The pointer on the form must be held in a CSmartPtr<UFormDfn>. Returns NULL if the DFN can't be loaded.
	  */
	virtual UFormDfn *loadFormDfn (const std::string &filename) = 0;

	/** Load a type, use NMISC::CPath to find the file.
	  *
	  * The pointer on the form must be held in a CSmartPtr<UType>. Returns NULL if the type can't be loaded.
	  */
	virtual UType *loadFormType (const std::string &filename) = 0;

	/// Create a form loader
	static UFormLoader *createLoader ();

	/// Create a form loader
	static void releaseLoader (UFormLoader *loader);
};


} // NLGEORGES


#endif // NL_U_FORM_LOADER_H

/* End of u_form_loader.h */
