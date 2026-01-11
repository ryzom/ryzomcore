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

#include "stdgeorgesconvert.h"
#include "form_body_elt.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormBodyElt::CFormBodyElt()
{
}

CFormBodyElt::CFormBodyElt( const CFormBodyElt& _fbe )
{
}

CFormBodyElt::~CFormBodyElt()
{
}

void CFormBodyElt::serial( NLMISC::IStream& s)
{
}

CFormBodyElt& CFormBodyElt::operator =( const CFormBodyElt& _fbe )
{
	return( *this );
}

CFormBodyElt& CFormBodyElt::operator +=( const CFormBodyElt& _fbe )
{
	return( *this );
}

CFormBodyElt& CFormBodyElt::operator -=( const CFormBodyElt& _fbe )
{
	return( *this );
}

bool CFormBodyElt::operator ==( const CFormBodyElt& _fbe ) const
{
	return( false );
}

CFormBodyElt* CFormBodyElt::Clone() const 
{
	return( new CFormBodyElt( *this ) );
}

CStringEx CFormBodyElt::GetName() const
{
	return( sxname );
}

void CFormBodyElt::SetName( const CStringEx _sxname )
{
	sxname = _sxname;
}

bool CFormBodyElt::Empty() const
{
	return( false );
}

CStringEx CFormBodyElt::GetValue() const
{
  CStringEx object;
	return( object );
}

CFormBodyElt* CFormBodyElt::GetElt( const unsigned int _index ) const
{
	return( 0 );
}

CFormBodyElt* CFormBodyElt::GetElt( const CStringEx _sxname ) const
{
	return( 0 );
}

}
