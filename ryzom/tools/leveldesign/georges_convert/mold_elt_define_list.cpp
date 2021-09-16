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
#include "georges_loader.h"
#include "mold_elt_define_list.h"
#include "form_body_elt_struct.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldEltDefineList::CMoldEltDefineList( CLoader* const _pl, CMoldEltDefine* const _pmed ) : CMoldEltDefine( _pl )
{
	pmed = _pmed;
	blist = true;
	benum = pmed->IsEnum();
	sxname = pmed->GetName();
}

CMoldEltDefineList::~CMoldEltDefineList()
{
}

void CMoldEltDefineList::Load( const CStringEx _sxfullname )
{
	pmed->Load( _sxfullname );
}

void CMoldEltDefineList::Load( const CStringEx _sxfullname, const CStringEx _sxdate )
{
	pmed->Load( _sxfullname, _sxdate );
	benum = pmed->IsEnum();
	sxname = pmed->GetName();
}

CStringEx CMoldEltDefineList::GetEltName( const unsigned int _index ) const
{
	return( pmed->GetEltName( _index ) );
}

CMoldElt* CMoldEltDefineList::GetEltPtr( const unsigned int _index ) const
{
	return( pmed->GetEltPtr( _index ) );
}

unsigned int CMoldEltDefineList::GetType() const
{
	return( pmed->GetType() );
}

CMoldElt* CMoldEltDefineList::GetMold()
{
	return( pmed );
}
/*
CStringEx CMoldEltDefineList::GetName() const													
{
	return( pmed->GetName() );
}

void CMoldEltDefineList::SetName( const CStringEx& _sxname )
{
	pmed->SetName( _sxname );
}

bool CMoldEltDefineList::IsEnum() const
{
	return( pmed->IsEnum() );
}

bool CMoldEltDefineList::IsList() const
{
	return( pmed->IsList() );
}
*/

}