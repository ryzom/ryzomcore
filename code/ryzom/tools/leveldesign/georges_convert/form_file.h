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

#ifndef NLGEORGES_FORM_FILE_H
#define NLGEORGES_FORM_FILE_H

#include "nel/misc/stream.h"
#include "string_ex.h"
#include "form.h"

namespace NLOLDGEORGES
{


// CFormFile class represents a file with a set of CForm sheets.
class CFormFile  
{
public:
	std::list< CForm >	lform;

	CFormFile();
	virtual ~CFormFile();
	void serial( NLMISC::IStream& s );

	void Load( const CStringEx _sxfullname );
	void Save( const CStringEx _sxfullname );
	void GetForm( CForm& _f ) const;
	void GetForm( CForm& _f, const CStringEx& _sxdate ) const; 
	void SetForm( CForm& _f );
};

} // NLGEORGES

#endif // NLGEORGES_FORM_FILE_H

