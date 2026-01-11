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
#include "item_elt_list.h"
#include "item_elt_struct.h"
#include "mold_elt_define.h"
#include "mold_elt_define_list.h"
#include "form_body_elt.h"
#include "form_body_elt_atom.h"
#include "form_body_elt_struct.h"
#include "form_body_elt_list.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemEltStruct::CItemEltStruct( CLoader* const _pl ) : CItemElt( _pl )
{
	infos = ITEM_ISSTRUCT;
	pmed = 0;
}

CItemEltStruct::~CItemEltStruct()
{
	Clear();
}

void CItemEltStruct::Clear()
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( *it )
			delete( *it );
	vpie.clear();
}

void CItemEltStruct::BuildItem( CMoldElt* const _pme )
{
	if( _pme->IsList() )
		pmed = dynamic_cast< CMoldEltDefineList* >( _pme );      
	else
		pmed = dynamic_cast< CMoldEltDefine* >( _pme );      
	nlassert( pmed );

	int i = 0;
	CMoldElt* pme = pmed->GetEltPtr( i );
	while( pme )
	{
		switch( pme->GetType() )
		{
		case 0:
			{
				CItemEltAtom* piea = new CItemEltAtom( pl );
				piea->BuildItem( pme );
				if( pme->IsList() )
				{
					CItemEltList* piel = new CItemEltList( pl );
					piel->BuildItem( piea );
					piel->SetName( pmed->GetEltName( i ) );
					vpie.push_back( piel );
				}
				else
				{
					piea->SetName( pmed->GetEltName( i ) );
					vpie.push_back( piea );
				}
				break;
			}
		case 1:
			{
				CItemEltStruct* pies = new CItemEltStruct( pl );
				pies->BuildItem( pme );
				if( pme->IsList() )
				{
					CItemEltList* piel = new CItemEltList( pl );
					piel->BuildItem( pies );
					piel->SetName( pmed->GetEltName( i ) );
					vpie.push_back( piel );
				}
				else
				{
					pies->SetName( pmed->GetEltName( i ) );
					vpie.push_back( pies );
				}
				break;
			}
		}
		pme = pmed->GetEltPtr( ++i );
	}
}

CStringEx CItemEltStruct::GetFormula() const 
{
	nlassert( pmed );
	return( pmed->GetFormula() );
}

void CItemEltStruct::FillParent( const CFormBodyElt* const _pfbe )
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
	{
		CFormBodyElt* pfbe = _pfbe->GetElt( (*it)->GetName() );
		if( pfbe )
			(*it)->FillParent( pfbe );
	}
}

void CItemEltStruct::FillCurrent( const CFormBodyElt* const _pfbe )
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
	{
		CFormBodyElt* pfbe = _pfbe->GetElt( (*it)->GetName() );
		if( pfbe )
			(*it)->FillCurrent( pfbe );
	}
}

CItemElt* CItemEltStruct::Clone()
{
	CItemEltStruct* pies = new CItemEltStruct( pl );
	pies->BuildItem( pmed );
	return( pies );
}

CFormBodyElt* CItemEltStruct::BuildForm()
{
	CFormBodyEltStruct* pfbes = new CFormBodyEltStruct();
	pfbes->SetName( sxname );
	BuildForm( pfbes );
	if( !pfbes->Empty() )
		return( pfbes );
	delete pfbes;
	return( 0 );
}

void CItemEltStruct::BuildForm( CFormBodyEltStruct* const _pfbes )
{
	if( bmodified )
		for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
			(*it)->SetModified( true );
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		_pfbes->AddElt( (*it)->BuildForm() );
}

void CItemEltStruct::BuildForm( CFormBodyEltStruct* const _pfbes, const std::vector< std::pair< CStringEx, CStringEx > >& _vsxparents ) 
{
	BuildForm( _pfbes );

	if( !_vsxparents.empty() )
	{
		CFormBodyEltList* pfbel = new CFormBodyEltList;
		pfbel->SetName( "Parents" );
		int i = 0;
		CStringEx sx;
		for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = _vsxparents.begin(); it != _vsxparents.end(); ++it )
		{
			CFormBodyEltStruct* pfbes = new CFormBodyEltStruct;
			sx.format( "#%d", i++ );
			pfbes->SetName( sx );
			pfbel->AddElt( pfbes );

			CFormBodyEltAtom* pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Activity" );
			pfbea->SetValue( it->first );
			pfbes->AddElt( pfbea );

			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Filename" );
			pfbea->SetValue( it->second );
			pfbes->AddElt( pfbea );
		}
		_pfbes->AddElt( pfbel );
	}
}

unsigned int CItemEltStruct::GetNbElt() const
{
	unsigned int nb = 0;
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		nb += (*it)->GetNbElt();
	return( ++nb );
}

CItemElt* CItemEltStruct::GetElt( const unsigned int _index ) const
{
	if( !_index )
		return( ( CItemElt * )( this ) );
	unsigned int isum = 1;				
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
	{
		unsigned int nb = (*it)->GetNbElt();
		if( isum+nb > _index )
			return( (*it)->GetElt( _index-isum ) );
		isum += nb;
	}
	return( 0 );
}

CItemElt* CItemEltStruct::GetElt( const CStringEx _sxname ) const
{
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( (*it)->GetName() == _sxname )
			return( *it );
	return( 0 );
}

bool CItemEltStruct::SetModified( const unsigned int _index )
{
//	SetModified( true );
	if( !_index )
		return( true );
	unsigned int isum = 1;				
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
	{
		unsigned int nb = (*it)->GetNbElt();
		if( isum+nb > _index )
			return( (*it)->SetModified( _index-isum ) );
		isum += nb;
	}
	return( false );
}

void CItemEltStruct::SetModified( const bool _b )
{
	bmodified = _b;
	if( !_b )
		for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
			(*it)->SetModified( _b );
}

unsigned int CItemEltStruct::GetNbChild ()
{
	return 0;
}

CItemElt* CItemEltStruct::GetChild (unsigned int _index)
{
	return NULL;
}

unsigned int CItemEltStruct::GetNbStructElt ()
{
	return vpie.size();
}

CItemElt* CItemEltStruct::GetStructElt (unsigned int _index)
{
	return vpie[_index];
}

}