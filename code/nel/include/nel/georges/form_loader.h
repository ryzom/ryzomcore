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

#ifndef NL_FORM_LOADER_H
#define NL_FORM_LOADER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/georges/u_form_loader.h"


namespace NLGEORGES
{

class UForm;
class CType;
class CFormDfn;
class CForm;

/**
 * Georges form loader implementation
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class CFormLoader : public UFormLoader
{
public:
	virtual ~CFormLoader();
	// From UFormLoader
	UForm		*loadForm (const char *filename);
	UFormDfn	*loadFormDfn (const char *filename);
	UType		*loadFormType (const char *filename);

	// Load type and formDfn
	CType		*loadType (const char *filename);
	CFormDfn	*loadFormDfn (const char *filename, bool forceLoad);

private:

	// Error handling
	virtual void		warning (bool exception, const char *function, const char *format, ... ) const;

	typedef std::map<std::string, NLMISC::CRefPtr<CType> >		TTypeMap;
	typedef std::map<std::string, NLMISC::CRefPtr<CFormDfn> >	TFormDfnMap;
	typedef std::map<std::string, NLMISC::CRefPtr<CForm> >		TFormMap;

	// Map of filename / CRefPtr<CType>
	TTypeMap		_MapType;

	// Map of filename / CRefPtr<CFormDfnCFormDfn>
	TFormDfnMap	_MapFormDfn;

	// Map of form / CRefPtr<CForm>
	TFormMap		_MapForm;
};


} // NLGEORGES


#endif // NL_FORM_LOADER_H

/* End of form_loader.h */
