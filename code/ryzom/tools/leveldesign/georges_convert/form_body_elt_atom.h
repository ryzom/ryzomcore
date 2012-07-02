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

#ifndef NLGEORGES_FORM_BODY_ELT_ATOM_H
#define NLGEORGES_FORM_BODY_ELT_ATOM_H

#include "nel/misc/stream.h"
#include "string_ex.h"
#include "form_body_elt.h"

namespace NLOLDGEORGES
{


// La classe CFormBodyEltAtom représente l'élément terminal de l'arbre. 
// Il est composé de deux string: sxname et sxvalue
class CFormBodyEltAtom : public CFormBodyElt  
{
public:
	CStringEx	sxvalue;

	NLMISC_DECLARE_CLASS( CFormBodyEltAtom );
	CFormBodyEltAtom();
	CFormBodyEltAtom( const CFormBodyEltAtom& _fbea );
	virtual ~CFormBodyEltAtom();
	virtual void serial( NLMISC::IStream& s );

	virtual CFormBodyElt&	operator  =( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator +=( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator -=( const CFormBodyElt& _fbe );
	virtual bool			operator ==( const CFormBodyElt& _fbe ) const;

	virtual CFormBodyElt* Clone() const;
	virtual bool Empty() const;
	virtual CStringEx GetValue() const;

	void SetValue( const CStringEx _sxvalue );
};

} // NLGEORGES

#endif // NLGEORGES_FORM_BODY_ELT_ATOM_H
