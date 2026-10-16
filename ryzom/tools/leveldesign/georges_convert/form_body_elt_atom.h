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


// CFormBodyEltAtom class represents terminal item from the tree. 
// It's composited from two strings: sxname and sxvalue
class CFormBodyEltAtom : public CFormBodyElt  
{
public:
	CStringEx	sxvalue;

	NLMISC_DECLARE_CLASS( CFormBodyEltAtom );
	CFormBodyEltAtom();
	CFormBodyEltAtom( const CFormBodyEltAtom& _fbea );
	virtual ~CFormBodyEltAtom() NL_OVERRIDE;
	virtual void serial( NLMISC::IStream& s ) NL_OVERRIDE;

	virtual CFormBodyElt&	operator  =( const CFormBodyElt& _fbe ) NL_OVERRIDE;
	virtual CFormBodyElt&	operator +=( const CFormBodyElt& _fbe ) NL_OVERRIDE;
	virtual CFormBodyElt&	operator -=( const CFormBodyElt& _fbe ) NL_OVERRIDE;
	virtual bool			operator ==( const CFormBodyElt& _fbe ) const NL_OVERRIDE;

	virtual CFormBodyElt* Clone() const NL_OVERRIDE;
	virtual bool Empty() const NL_OVERRIDE;
	virtual CStringEx GetValue() const NL_OVERRIDE;

	void SetValue( const CStringEx _sxvalue );
};

} // NLGEORGES

#endif // NLGEORGES_FORM_BODY_ELT_ATOM_H
