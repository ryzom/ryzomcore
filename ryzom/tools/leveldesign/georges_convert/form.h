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

#ifndef NLGEORGES_FORM_H
#define NLGEORGES_FORM_H

#include "nel/misc/stream.h"
#include "form_head.h"
#include "form_body_elt_struct.h"

namespace NLOLDGEORGES
{

class CFormBodyElt;

// CForm class represents a sheet.
// It has a Head and a Body.
// It has operators that allows to add or subtract from history.
class CForm  
{
public:
	CFormHead			head;
	CFormBodyEltStruct	body;
	bool				bmodified;

	CForm();
	CForm( const CForm& _f );
	virtual ~CForm();

	void serial( NLMISC::IStream& s );
	
	CStringEx GetDate() const;
	CStringEx GetVersion() const;
	CStringEx GetUser() const;
	CStringEx GetComment() const;
	void SetDate( const CStringEx _sxdate );
	void SetVersion( const CStringEx _sxversion );
	void SetUser( const CStringEx _sxuser );
	void SetComment( const CStringEx _sxcomment );
	
	
	CStringEx GetParent( unsigned int _index ) const;
	CStringEx GetActivity( unsigned int _index ) const;
	CFormBodyElt* GetElt( const CStringEx _sxname ) const;
	CFormBodyEltStruct* GetBody() const;
	bool GetModified() const;
	void SetModified( const bool _b );
	
	CForm&	operator  =( const CForm& _f );
	CForm&	operator +=( const CForm& _f );
	CForm&	operator -=( const CForm& _f );
	bool	operator ==( const CForm& _f ) const;
};

} // NLGEORGES

#endif // NLGEORGES_FORM_H

