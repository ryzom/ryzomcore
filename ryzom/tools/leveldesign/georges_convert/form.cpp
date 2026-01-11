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

#include "form.h"
#include "form_body_elt.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CForm::CForm()
{
	body.SetName( "BODY" );
	bmodified = false;
}

CForm::CForm( const CForm& _f )
{
	bmodified = _f.bmodified;
	head = _f.head;
	body = _f.body;
}

CForm::~CForm()
{
}

void CForm::serial( NLMISC::IStream& s )
{
	s.serial( head );
	s.serial( body );
}

CStringEx CForm::GetDate() const
{
	return( head.GetDate() );
}

CStringEx CForm::GetVersion() const
{
	return( head.GetVersion() );
}

CStringEx CForm::GetUser() const
{
	return( head.GetUser() );
}

CStringEx CForm::GetComment() const
{
	return( body.GetComment() );
}

void CForm::SetDate( const CStringEx _sxdate )
{
	head.SetDate( _sxdate );
}

void CForm::SetVersion( const CStringEx _sxversion )
{
	head.SetVersion( _sxversion );
}

void CForm::SetUser( const CStringEx _sxuser )
{
	head.SetUser( _sxuser );
}

void CForm::SetComment( const CStringEx _sxcomment )
{
	body.SetComment( _sxcomment );
}

CStringEx CForm::GetParent( unsigned int _index ) const
{
	return( body.GetParent( _index ) );
}

CStringEx CForm::GetActivity( unsigned int _index ) const
{
	return( body.GetActivity( _index ) );
}

CFormBodyElt* CForm::GetElt( const CStringEx _sxname ) const
{
	return( body.GetElt( _sxname ) );
}

CFormBodyEltStruct* CForm::GetBody() const
{
	return( (CFormBodyEltStruct*)(&body) );
}

bool CForm::GetModified() const
{
	return( bmodified );
}

void CForm::SetModified( const bool _b )
{
	bmodified = _b;
}

CForm& CForm::operator =( const CForm& _f )
{
	head = _f.head;
	body = _f.body;
	bmodified = true;
	return( *this );
}

CForm& CForm::operator +=( const CForm& _f )
{
	head = _f.head;
	body += _f.body;
	bmodified = true;
	return( *this );
}

CForm& CForm::operator -=( const CForm& _f )
{
	body -= _f.body;
	bmodified = true;
	return( *this );
}

bool CForm::operator ==( const CForm& _f ) const
{
	return( (body == _f.body) );
}

}