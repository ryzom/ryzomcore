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
#include "item_elt_atom.h"
#include "mold_elt_type.h"
#include "mold_elt_type_list.h"
#include "form_body_elt.h"
#include "form_body_elt_atom.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemEltAtom::CItemEltAtom( CLoader* const _pl ) : CItemElt( _pl )
{
	infos = ITEM_ISATOM;
	pmet = 0; 
}

CItemEltAtom::~CItemEltAtom()
{
}

void CItemEltAtom::BuildItem( CMoldElt* const _pme )
{
	if( _pme->IsList() )
		pmet = dynamic_cast< CMoldEltTypeList* >( _pme );      
	else
		pmet = dynamic_cast< CMoldEltType* >( _pme );      
	nlassert( pmet );
	sxparentvalue.clear();
	sxcurrentvalue.clear();
	sxoldparentvalue.clear();
	sxoldcurrentvalue.clear();

	sxparentresult = pmet->GetDefaultValue();
	sxcurrentresult = pmet->GetDefaultValue();

	//SetParentValue(pmet->GetDefaultValue());

	if( pmet->IsEnum() )
		infos |= ITEM_ISENUM;
}

CStringEx CItemEltAtom::GetFormula() const 
{
	nlassert( pmet );
	return( pmet->GetFormula() );
}

void CItemEltAtom::SetParentValue( const CStringEx _sxparentvalue )
{
	nlassert( pmet );
	sxparentvalue = pmet->Format( _sxparentvalue );
	sxparentresult = pmet->CalculateResult( pmet->GetDefaultValue(), sxparentvalue );
	if( sxparentvalue.empty() )
		sxparentresult = pmet->GetDefaultValue();
}

void CItemEltAtom::SetCurrentValue( const CStringEx _sxcurrentvalue )
{
	nlassert( pmet );
	bmodified = true;
	sxcurrentvalue = pmet->Format( _sxcurrentvalue );
	sxcurrentresult = pmet->CalculateResult( sxparentresult, sxcurrentvalue );
	if( !sxcurrentvalue.empty() )
		return;
	sxcurrentvalue.clear();
	sxcurrentresult = sxparentresult;
}

void CItemEltAtom::FillParent( const CFormBodyElt* const _pfbe )
{
	SetParentValue( _pfbe->GetValue() );
	sxoldparentvalue = sxparentvalue;
	SetCurrentValue( sxparentvalue );
}

void CItemEltAtom::FillCurrent( const CFormBodyElt* const _pfbe )
{
	SetCurrentValue( _pfbe->GetValue() );
	sxoldcurrentvalue = sxcurrentvalue;
}

CItemElt* CItemEltAtom::Clone()
{
	CItemEltAtom* piea = new CItemEltAtom( pl );
	piea->BuildItem( pmet );
	return( piea );
}

CFormBodyElt* CItemEltAtom::BuildForm()
{
	if( !bmodified && sxoldcurrentvalue.empty() ) /* && sxcurrentvalue.empty() ??? */
		return( 0 );
	CFormBodyEltAtom* pfbea = new CFormBodyEltAtom();
	pfbea->SetName( sxname );
	pfbea->SetValue( sxcurrentvalue );
	return( pfbea );
}

unsigned int CItemEltAtom::GetNbElt() const
{
	return( 1 );
}

CMoldEltType* CItemEltAtom::GetMoldType() const
{
	return( pmet );
}

unsigned int CItemEltAtom::GetNbChild ()
{
	return 0;
}

CItemElt* CItemEltAtom::GetChild (unsigned int _index)
{
	return NULL;
}

}