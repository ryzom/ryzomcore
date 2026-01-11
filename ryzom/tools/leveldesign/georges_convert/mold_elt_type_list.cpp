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
#include "mold_elt_type_list.h"
#include "georges_loader.h"
#include "form_body_elt.h"
#include "type_unit_int_unsigned.h"
#include "type_unit_int_signed.h"
#include "type_unit_double.h"
#include "type_unit_string.h"
#include "type_unit_file_name.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldEltTypeList::CMoldEltTypeList( CLoader* const _pl, CMoldEltType* const _pmet ) : CMoldEltType( _pl )
{
	pmet = _pmet;
	blist = true;
	benum = pmet->IsEnum();
	sxname = pmet->GetName();
}

CMoldEltTypeList::~CMoldEltTypeList()
{
}

void CMoldEltTypeList::Load( const CStringEx _sxfullname )						// TODO: Load with parents...
{
	pmet->Load( _sxfullname );
	benum = pmet->IsEnum();
	sxname = pmet->GetName();
}

void CMoldEltTypeList::Load( const CStringEx _sxfullname, const CStringEx _sxdate )
{
	pmet->Load( _sxfullname, _sxdate );
}

CStringEx CMoldEltTypeList::GetFormula()													 
{
	return( pmet->GetFormula() );
}

CStringEx CMoldEltTypeList::Format( const CStringEx _sxvalue ) const													
{
	return( pmet->Format( _sxvalue ) );
}

CStringEx CMoldEltTypeList::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	return( pmet->CalculateResult( _sxbasevalue, _sxvalue ) );
}

CStringEx CMoldEltTypeList::GetDefaultValue() const	
{
	return( pmet->GetDefaultValue() );
}

unsigned int CMoldEltTypeList::GetType() const
{
	return( pmet->GetType() );
}

CStringEx CMoldEltTypeList::GetPredefSubstitute( const CStringEx _sxdesignation ) const
{
	return( pmet->GetPredefSubstitute( _sxdesignation ) );
}

CStringEx CMoldEltTypeList::GetPredefDesignation( const CStringEx _sxsubstitute ) const
{
	return( pmet->GetPredefDesignation( _sxsubstitute ) );
}

CStringEx CMoldEltTypeList::GetPredefDesignation( const unsigned int _index ) const
{
	return( pmet->GetPredefDesignation( _index ) );
}

CMoldElt* CMoldEltTypeList::GetMold()
{
	return( pmet );
}

}