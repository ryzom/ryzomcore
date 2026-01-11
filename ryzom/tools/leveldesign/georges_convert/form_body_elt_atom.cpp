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
#include "form_body_elt_atom.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormBodyEltAtom::CFormBodyEltAtom()
{
}

CFormBodyEltAtom::CFormBodyEltAtom( const CFormBodyEltAtom& _fbea ) : CFormBodyElt( _fbea )
{
	sxname = _fbea.sxname;
	sxvalue = _fbea.sxvalue;
}

CFormBodyEltAtom::~CFormBodyEltAtom()
{
}

void CFormBodyEltAtom::serial( NLMISC::IStream& s)
{

	s.xmlPushBegin( "Atom" );
		s.xmlSetAttrib( "Name" );
			s.serial( sxname );
		s.xmlSetAttrib( "Value" );
			s.serial( sxvalue );
	s.xmlPushEnd();
	s.xmlPop();
}

CFormBodyElt& CFormBodyEltAtom::operator =( const CFormBodyElt& _fbe )
{
	const CFormBodyEltAtom* pfbea = dynamic_cast< const CFormBodyEltAtom* >( &_fbe );      
	nlassert( pfbea );
	sxname = pfbea->sxname;
	sxvalue = pfbea->sxvalue;
	return( *this );
}

CFormBodyElt& CFormBodyEltAtom::operator +=( const CFormBodyElt& _fbe )
{
	const CFormBodyEltAtom* pfbea = dynamic_cast< const CFormBodyEltAtom* >( &_fbe );      
	nlassert( pfbea );
	sxvalue = pfbea->sxvalue;
	return( *this );
}

CFormBodyElt& CFormBodyEltAtom::operator -=( const CFormBodyElt& _fbe )
{
	const CFormBodyEltAtom* pfbea = dynamic_cast< const CFormBodyEltAtom* >( &_fbe );      
	nlassert( pfbea );
	if( sxvalue == pfbea->sxvalue )
		sxvalue.erase();
	return( *this );
}

bool CFormBodyEltAtom::operator ==( const CFormBodyElt& _fbe ) const
{
	const CFormBodyEltAtom* pfbea = dynamic_cast< const CFormBodyEltAtom* >( &_fbe );      
	nlassert( pfbea );
	return( sxvalue == pfbea->sxvalue );
}

CFormBodyElt* CFormBodyEltAtom::Clone() const 
{
	return( new CFormBodyEltAtom( *this ) );
}

bool CFormBodyEltAtom::Empty() const
{
	return( sxvalue.empty() );
}

CStringEx CFormBodyEltAtom::GetValue() const
{
	return( sxvalue );
}

void CFormBodyEltAtom::SetValue( const CStringEx _sxvalue )
{
	sxvalue = _sxvalue;
}

}