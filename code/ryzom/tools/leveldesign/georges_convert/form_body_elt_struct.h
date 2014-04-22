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

#ifndef NLGEORGES_FORM_BODY_ELT_STRUCT_H
#define NLGEORGES_FORM_BODY_ELT_STRUCT_H

#include "nel/misc/stream.h"
#include "form_body_elt.h"

namespace NLOLDGEORGES
{

#define SXCOMMENT "Comment"
#define SXPARENTS "Parents"

// CFormBodyEltStruct provides a set of CFormBodyElt. 
// In operator +=, new items replace old ones, not overrided old ones are kept.
class CFormBodyEltStruct : public CFormBodyElt  
{
public:
	std::vector< CFormBodyElt* > vpbodyelt;
	std::vector< CFormBodyElt* >::iterator Find( const CStringEx _sxname );
	std::vector< CFormBodyElt* >::const_iterator Find( const CStringEx _sxname ) const;
	void Clear();

	NLMISC_DECLARE_CLASS( CFormBodyEltStruct );
	CFormBodyEltStruct();
	CFormBodyEltStruct( const CFormBodyEltStruct& _fbes );
	virtual ~CFormBodyEltStruct();
	virtual void serial( NLMISC::IStream& s );

			CFormBodyElt&	operator  =( const CFormBodyEltStruct& _fbes );
	virtual CFormBodyElt&	operator  =( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator +=( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator -=( const CFormBodyElt& _fbe );
	virtual bool			operator ==( const CFormBodyElt& _fbe ) const;

	virtual CFormBodyElt* Clone() const;
	virtual bool Empty() const;
	CStringEx GetComment() const;
	void SetComment( const CStringEx _sxcomment );
	CStringEx GetParent( unsigned int _index ) const;
	CStringEx GetActivity( unsigned int _index ) const;

	uint32 GetNbElt () const;
	virtual CFormBodyElt* GetElt( const unsigned int _index ) const;
	virtual CFormBodyElt* GetElt( const CStringEx _sxname ) const;

	void AddElt( CFormBodyElt* const pfbe );
};

} // NLGEORGES

#endif // NLGEORGES_FORM_BODY_ELT_STRUCT_H
