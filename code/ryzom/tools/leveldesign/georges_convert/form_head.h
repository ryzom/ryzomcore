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

#ifndef NLGEORGES_FORM_HEAD_H
#define NLGEORGES_FORM_HEAD_H

#include "nel/misc/stream.h"
#include "string_ex.h"

namespace NLOLDGEORGES
{

// CFormHead class provides three strings: user name, last modification date and version.
class CFormHead  
{
protected:
	CStringEx	sxuser;	
	CStringEx	sxversion;	
	CStringEx	sxdate;	

public:
	CFormHead();
	CFormHead( const CFormHead& _fh );
	virtual ~CFormHead();
	void serial( NLMISC::IStream& s );
	CStringEx GetUser() const;
	CStringEx GetVersion() const;
	CStringEx GetDate() const;
	void SetUser( const CStringEx _sxuser );
	void SetVersion( const CStringEx _sxversion );
	void SetDate( const CStringEx _sxdate );

	CFormHead&	operator =( const CFormHead& _f );
};

} // NLGEORGES

#endif // NLGEORGES_FORM_HEAD_H
