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

#ifndef NLGEORGES_FORM_BODY_ELT_H
#define NLGEORGES_FORM_BODY_ELT_H

#include "nel/misc/stream.h"
#include "string_ex.h"

namespace NLOLDGEORGES
{

// La classe CFormBodyElt est la classe de base pour le polymorphisme des CFormBodyEltAtom, CFormBodyEltList, CFormBodyEltStruct
// Elle comprend le nom de la branche ou de la feuille.
class CFormBodyElt : public NLMISC::IStreamable  
{
public:
	CStringEx	sxname;

	NLMISC_DECLARE_CLASS( CFormBodyElt );
	CFormBodyElt();
	CFormBodyElt( const CFormBodyElt& _fbe );
	virtual ~CFormBodyElt();
	virtual void serial( NLMISC::IStream& s );

	virtual CFormBodyElt&	operator  =( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator +=( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator -=( const CFormBodyElt& _fbe );
	virtual bool			operator ==( const CFormBodyElt& _fbe ) const;

	virtual CFormBodyElt* Clone() const;
	virtual bool Empty() const;

	CStringEx GetName() const;
	virtual CStringEx GetValue() const;
	virtual CFormBodyElt* GetElt( const unsigned int _index ) const;
	virtual CFormBodyElt* GetElt( const CStringEx _sxname ) const;

	void SetName( const CStringEx _sxname );
};

} // NLGEORGES

#endif // NLGEORGES_FORM_BODY_ELT_H
