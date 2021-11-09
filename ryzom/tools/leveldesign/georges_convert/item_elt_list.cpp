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
#include "item_elt_list.h"
#include "item_elt_atom.h"
#include "item_elt_struct.h"
#include "form_body_elt.h"
#include "form_body_elt_list.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemEltList::CItemEltList( CLoader* const _pl ) : CItemElt( _pl )
{
	infos = ITEM_ISLIST;
	piemodel = 0;
}

CItemEltList::~CItemEltList()
{
	if( piemodel )
		delete piemodel;
	piemodel = 0;
	Clear();
}

void CItemEltList::Clear()
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( *it )
			delete( *it );
	vpie.clear();
}

void CItemEltList::BuildItem( CItemElt* const _pie )
{
	piemodel = _pie;
	piemodel->AddInfos( ITEM_ISLISTCHILD );
	piemodel->SetListParent( this );
	nlassert( piemodel );
}

CStringEx CItemEltList::GetFormula() const 
{
	nlassert( piemodel );
	return( CStringEx("list<") + piemodel->GetFormula() + ">" );
}

void CItemEltList::FillParent( const CFormBodyElt* const _pfbe )
{
	unsigned int i = 0;
	CStringEx sx;
	CFormBodyElt* pfbe = _pfbe->GetElt(0);
	if( pfbe )
		Clear();	
	while( pfbe )
	{
		CItemElt* pie = piemodel->Clone();
		pie->AddInfos( ITEM_ISLISTCHILD );
		pie->SetListParent( this );
		pie->FillParent( pfbe );
		sx.format( "#%d", i );
		pie->SetName( sx );
		vpie.push_back( pie );
		pfbe = _pfbe->GetElt(++i);
	}
}

void CItemEltList::FillCurrent( const CFormBodyElt* const _pfbe )
{
	unsigned int i = 0;
	CStringEx sx;
	CFormBodyElt* pfbe = _pfbe->GetElt(0);
	if( pfbe )
		Clear();	
	if( !pfbe ) 
		return;
	while( pfbe )
	{
		CItemElt* pie = piemodel->Clone();
		pie->AddInfos( ITEM_ISLISTCHILD );
		pie->SetListParent( this );
		pie->FillCurrent( pfbe );
		sx.format( "#%d", i );
		pie->SetName( sx );
		vpie.push_back( pie );
		pfbe = _pfbe->GetElt(++i);
	}
	SetModified( true );
}

CItemElt* CItemEltList::Clone()
{
	CItemEltList* piel = new CItemEltList( pl );
	piel->BuildItem( piemodel );
	return( piel );
}

CFormBodyElt* CItemEltList::BuildForm()
{
	if( bmodified )
		for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
			(*it)->SetModified( true );
	CFormBodyEltList* pfbel = new CFormBodyEltList();
	pfbel->SetName( sxname );
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		pfbel->AddElt( (*it)->BuildForm() );
	return( pfbel );
}

unsigned int CItemEltList::GetNbElt() const
{
	unsigned int nb = 0;
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		nb += (*it)->GetNbElt();
	return( ++nb );
}

CItemElt* CItemEltList::GetElt( const unsigned int _index ) const
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

CItemElt* CItemEltList::GetElt( const CStringEx _sxname ) const
{
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( (*it)->GetName() == _sxname )
			return( *it );
	return( 0 );
}

bool CItemEltList::SetModified( const unsigned int _index )
{
	SetModified( true );
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

void CItemEltList::SetModified( const bool _b )
{
	bmodified = _b;
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		(*it)->SetModified( _b );
}

void CItemEltList::NewElt()
{
	CItemElt* pie = piemodel->Clone();
	pie->AddInfos( ITEM_ISLISTCHILD );
	pie->SetListParent( this );
	CStringEx sx;
	sx.format( "#%d", vpie.size() );
	pie->SetName( sx );
	vpie.push_back( pie );
	SetModified( true );
}

void CItemEltList::AddElt( const CItemElt* const _pie )
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( (*it) == _pie )
		{
			CItemElt* pie = piemodel->Clone();
			pie->AddInfos( ITEM_ISLISTCHILD );
			pie->SetListParent( this );
			vpie.insert( it, pie );
			VerifyName();
			return;
		}
}

void CItemEltList::DelElt( CItemElt* const _pie )
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( (*it) == _pie )
		{
			delete( _pie );
			vpie.erase( it );
			VerifyName();
			return;
		}
}

void CItemEltList::VerifyName()
{
	unsigned int i = 0;
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
	{
			CStringEx sx;
			sx.format( "#%d", i++ );
			(*it)->SetName( sx );
	}
}

unsigned int CItemEltList::GetNbChild ()
{
	return vpie.size();
}

CItemElt* CItemEltList::GetChild (unsigned int _index)
{
	return vpie[_index];
}

}
