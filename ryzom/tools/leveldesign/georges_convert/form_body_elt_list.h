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

#ifndef NLGEORGES_FORM_BODY_ELT_LIST_H
#define NLGEORGES_FORM_BODY_ELT_LIST_H


#include "nel/misc/stream.h"
#include "form_body_elt.h"

namespace NLOLDGEORGES
{


// CFormBodyEltList provides a set of CFormBodyElt in a list form. 
// In operator +=, set of old items are removed. New ones replace them.
class CFormBodyEltList : public CFormBodyElt  
{
public:
	std::vector< CFormBodyElt* > vpbodyelt;
	std::vector< CFormBodyElt* >::iterator Find( const CStringEx _sxname );
	std::vector< CFormBodyElt* >::const_iterator Find( const CStringEx _sxname ) const;

	NLMISC_DECLARE_CLASS( CFormBodyEltList );
	CFormBodyEltList();
	CFormBodyEltList( const CFormBodyEltList& _fbel );
	virtual ~CFormBodyEltList();
	virtual void serial( NLMISC::IStream& s );

	virtual CFormBodyElt&	operator  =( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator +=( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator -=( const CFormBodyElt& _fbe );
	virtual bool			operator ==( const CFormBodyElt& _fbe ) const;

	virtual CFormBodyElt* Clone() const;
	virtual bool Empty() const;
	virtual CFormBodyElt* GetElt( const unsigned int _index ) const;
	virtual CFormBodyElt* GetElt( const CStringEx _sxname ) const;

	void AddElt( CFormBodyElt* const pfbe );
	void Clear();
};

} // NLGEORGES

#endif // NLGEORGES_FORM_BODY_ELT_LIST_H

