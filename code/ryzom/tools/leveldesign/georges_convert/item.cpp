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
#include "item.h"
#include "georges_loader.h"
#include "mold_elt_define.h"
#include "mold_elt_type.h"
#include "item_elt_atom.h"
#include "item_elt_struct.h"
#include "item_elt_list.h"
#include "common.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItem::	CItem()
{
	pitemes = 0;
//	pitemelparents = 0;      
//	pitemeacomments = 0;
}

CItem::~CItem()
{
	Clear();
}

void CItem::Clear()
{
	if( pitemes )
		delete( pitemes );
	pitemes = 0;
	vsxparents.clear();
/*
	if( pitemelparents )
		delete( pitemelparents );
	pitemes = 0;
	if( pitemeacomments )
		delete( pitemeacomments );
	pitemes = 0;
*/
}

void CItem::SetLoader( CLoader* const _pl )
{
	nlassert( _pl );
	pl = _pl;
}

void CItem::Load( const CStringEx& _sxfullname )
{
	Clear();

	// Load the form
	CForm form, formcurrent, formparent;
	pl->LoadForm( formcurrent, _sxfullname );
	
	// Load Parents
	unsigned int i = 0;
	CStringEx sxparent = formcurrent.GetParent( 0 );
	CStringEx sxactivity = formcurrent.GetActivity( 0 );
	while( !sxparent.empty() )								
	{
		vsxparents.push_back( std::make_pair( sxactivity, sxparent ) );
		if( sxactivity == "true" )
		{
			pl->LoadSearchForm( form, sxparent );			
			formparent += form;
		}
		i++;
		sxparent = formcurrent.GetParent( i );
		sxactivity = formcurrent.GetActivity( i );
	}

	// Find the name of the dfn file
	int ipos = _sxfullname.reverse_find('.');
	if( ipos == -1 )
		return;
	moldfilename = _sxfullname.get_right(_sxfullname.length()-ipos-1);
	moldfilename += ".dfn";

	// Load the mold and build the item's tree
	CMoldElt* pme = pl->LoadMold( moldfilename );
	CMoldEltDefine* pmed = dynamic_cast< CMoldEltDefine* >( pme );
	nlassert( pmed );
	pitemes = new CItemEltStruct( pl );
	pitemes->BuildItem( pmed );

	// Fill the tree with parents' form fields 
	pitemes->FillParent( formparent.GetBody() );

	// Fill the tree with current's form
	pitemes->FillCurrent( formcurrent.GetBody() );
}

// Convert CItem to a CForm (in is this)
void CItem::MakeForm (CForm &out)
{
	if( !pitemes )
		return;
	pitemes->BuildForm( out.GetBody(), vsxparents );
}

// Convert CForm to CItem (out is this)
void CItem::MakeItem (CForm &in)
{
	CForm form, formparent;

	Clear();
	unsigned int i = 0;
	CStringEx sxparent = in.GetParent( 0 );
	CStringEx sxactivity = in.GetActivity( 0 );
	while( !sxparent.empty() )								
	{
		vsxparents.push_back( std::make_pair( sxactivity, sxparent ) );
		if( sxactivity == "true" )
		{
			pl->LoadSearchForm( form, sxparent );			
			formparent += form;
		}
		i++;
		sxparent = in.GetParent( i );
		sxactivity = in.GetActivity( i );
	}

	CMoldElt* pme = pl->LoadMold( moldfilename );
	CMoldEltDefine* pmed = dynamic_cast< CMoldEltDefine* >( pme );
	nlassert( pmed );
	pitemes = new CItemEltStruct( pl );
	pitemes->BuildItem( pmed );

	pitemes->FillParent( formparent.GetBody() );
	pitemes->FillCurrent( in.GetBody() );
}

void CItem::VirtualSaveLoad()
{
//-------save
	if( !pitemes )
		return;
	CForm form, formcurrent, formparent;
	pitemes->BuildForm( formcurrent.GetBody(), vsxparents );
//------load
	Clear();
	unsigned int i = 0;
	CStringEx sxparent = formcurrent.GetParent( 0 );
	CStringEx sxactivity = formcurrent.GetActivity( 0 );
	while( !sxparent.empty() )								
	{
		vsxparents.push_back( std::make_pair( sxactivity, sxparent ) );
		if( sxactivity == "true" )
		{
			pl->LoadSearchForm( form, sxparent );			
			formparent += form;
		}
		i++;
		sxparent = formcurrent.GetParent( i );
		sxactivity = formcurrent.GetActivity( i );
	}

	CMoldElt* pme = pl->LoadMold( moldfilename );
	CMoldEltDefine* pmed = dynamic_cast< CMoldEltDefine* >( pme );
	nlassert( pmed );
	pitemes = new CItemEltStruct( pl );
	pitemes->BuildItem( pmed );

	pitemes->FillParent( formparent.GetBody() );
	pitemes->FillCurrent( formcurrent.GetBody() );
}

void CItem::New( const CStringEx& _sxdfnfilename )
{
	Clear();

	moldfilename = _sxdfnfilename;

	CMoldElt* pme = pl->LoadMold( _sxdfnfilename );
	CMoldEltDefine* pmed = dynamic_cast< CMoldEltDefine* >( pme );
	nlassert( pmed );
	pitemes = new CItemEltStruct( pl );
	pitemes->BuildItem( pmed );
}

void CItem::Load( const CStringEx& _sxfilename, const CStringEx _sxdate ) 
{
}

void CItem::Save( const CStringEx& _sxfilename )
{
	if( !pitemes )
		return;
	CForm form;
	pitemes->BuildForm( form.GetBody(), vsxparents );
	pl->SaveForm( form, _sxfilename );
}

CItemElt* CItem::GetElt( const unsigned int _index ) const
{
	if( pitemes )
		return( pitemes->GetElt( _index ) );
	return( 0 );
}

CItemElt* CItem::GetElt( const CStringEx _sxname ) const
{
	if( pitemes )
		return( pitemes->GetElt( _sxname ) );
	return( 0 );
}

void CItem::SetCurrentValue( const unsigned int _index, const CStringEx s )
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return;
	pie->SetCurrentValue( s );
	pitemes->SetModified( _index );
}

uint CItem::GetNbElt() const
{
	if( pitemes )
		return( pitemes->GetNbElt() );
	return( 0 );
}

uint CItem::GetNbParents() const
{
	return (uint)vsxparents.size();
}

uint CItem::GetNbElt( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( 0 );
	return( pie->GetNbElt() );
}

uint CItem::GetInfos( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( 0 );
	return( pie->GetInfos() );
}

CStringEx CItem::GetName( const unsigned int _index ) const
{

	CItemElt* pie = GetElt( _index );
	if( !pie )
	  {
	    CStringEx object;
		return( object );
	  }
	return( pie->GetName() );
}

CStringEx CItem::GetCurrentResult( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
	  {
	    CStringEx object;
		return( object );
	  }
	return( pie->GetCurrentResult() );
}

CStringEx CItem::GetCurrentValue( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
	  {
	    CStringEx object;
		return( object );
	  }
	return( pie->GetCurrentValue() );
}

bool CItem::Update()
{
	if( !pitemes )
		return false;
	bool b = false;
	unsigned int i = 0;
	CItemElt* pie = pitemes->GetElt( i++ );
	while( pie )
	{
		CStringEx sxold = pie->GetCurrentValue();
		pie->SetParentValue( pie->GetParentValue() );
		pie->SetCurrentValue( pie->GetCurrentValue() );
		b |= ( sxold != pie->GetCurrentValue() );
		pie = pitemes->GetElt( i++ );
	}
	return b;
}

CStringEx CItem::GetFormula( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
	  {
	    CStringEx object;
		return( object );
	  }
	return( pie->GetFormula() );
}

bool CItem::IsEnum( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( false );
	return( ( pie->GetInfos() & ITEM_ISENUM ) != 0 );
}

bool CItem::IsPredef( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( false );
	CItemEltAtom* piea = dynamic_cast< CItemEltAtom* >( pie );
	if( !piea )
		return( false );
	CMoldEltType* pmet = piea->GetMoldType();
	CStringEx sx = pmet->GetPredefDesignation( 0 );
	return( !sx.empty() );
}

bool CItem::CanEdit( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( false );
	CItemEltAtom* piea = dynamic_cast< CItemEltAtom* >( pie );
	if( !piea )
		return( false );
	return( !( pie->GetInfos() & ITEM_ISENUM ) );
}

void CItem::GetListPredef( const unsigned int _index, std::vector< CStringEx >& _vsx ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return;
	CItemEltAtom* piea = dynamic_cast< CItemEltAtom* >( pie );
	if( !piea )
		return;

	CMoldEltType* pmet = piea->GetMoldType();
	
	unsigned int i = 0;
	CStringEx sx = pmet->GetPredefDesignation( i++ );
	while( !sx.empty() )
	{
		_vsx.push_back( sx );
		sx = pmet->GetPredefDesignation( i++ );
	}
}

CStringEx CItem::GetParent( const unsigned int _index ) const
{
	if( _index >= vsxparents.size() )
	  {
	    CStringEx object;
		return( object );
	  }
	return( vsxparents[_index].second );
}

void CItem::SetParent( const unsigned int _index, const CStringEx _sx )
{
	if( _index >= vsxparents.size() )
		return;
	vsxparents[_index].second = _sx;
}

CStringEx CItem::GetActivity( const unsigned int _index ) const
{
	if( _index >= vsxparents.size() )
	  {
	    CStringEx object;
		return( object );
	  }
	return( vsxparents[_index].first );
}

void CItem::SetActivity( const unsigned int _index, const CStringEx _sx )
{
	if( _index >= vsxparents.size() )
		return;
	vsxparents[_index].first = _sx;
}

void CItem::AddList( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return;
	CItemEltList* piel = dynamic_cast< CItemEltList* >( pie );
	if( !piel )
		return;
	piel->NewElt();
}
/*
void CItem::AddListChild( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return;
	CItemElt* piep = pie->GetListParent();
	if( !piep )
		return;
	CItemEltList* piel = dynamic_cast< CItemEltList* >( piep );
	if( !piel )
		return;
	piel->AddElt( pie );
}
*/
void CItem::DelListChild( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return;
	CItemElt* piep = pie->GetListParent();
	if( !piep )
		return;
	CItemEltList* piel = dynamic_cast< CItemEltList* >( piep );
	if( !piel )
		return;
	piel->DelElt( pie );
}

void CItem::AddParent( const unsigned int _index )
{
	vsxparents.push_back( std::make_pair( CStringEx("false"), CStringEx("filename.extension") ) );
}

void CItem::DelParent( const unsigned int _index )
{
	unsigned int i = 0;
	for( std::vector< std::pair< CStringEx, CStringEx > >::iterator it = vsxparents.begin(); it != vsxparents.end(); ++it )
		if( i++ == _index )
		{
			vsxparents.erase( it );
			return;
		}
}

void CItem::ClearParents ()
{
	vsxparents.clear();
}

}
