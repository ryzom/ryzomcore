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
#include "mold_loader.h"
#include "mold_elt.h"
#include "mold_elt_define.h"
#include "mold_elt_type.h"
#include "mold_elt_define_list.h"
#include "mold_elt_type_list.h"
#include "georges_loader.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldLoader::CMoldLoader()
{
}

CMoldLoader::~CMoldLoader()
{
	Clear();
}

void CMoldLoader::Clear()
{
	for( std::vector< CMoldElt* >::iterator it = vpme.begin(); it != vpme.end(); ++it )
		if( *it )
			delete( *it );
	vpme.clear();
	mmold.clear();
}

void CMoldLoader::SetLoader( CLoader* const _pl )
{
	nlassert( _pl );
	pl = _pl;
}

CMoldElt* CMoldLoader::LoadMold( const CStringEx _sxfilename )
{
	CStringEx sxfn = _sxfilename;	
	sxfn.purge();
	if( sxfn.empty() )
		return( 0 );											

	// liste?
	bool blst = ( sxfn.find( "list<" ) != -1 );
	if( blst )
	{
		unsigned int ipos = sxfn.find( ">" );
		if( ipos < 0 )
			return( 0 );											
		sxfn.mid( 5, ipos-5 ); 
	}

	// find extension
	int ipos = sxfn.reverse_find('.');
	if( ipos < 0 )
		return( 0 );											
	CStringEx sxfileextension = sxfn.get_right( sxfn.length()-ipos-1 );

	// Get only the filename
	ipos = sxfn.reverse_find('\\');
	if (ipos >= 0)
		sxfn = sxfn.get_right (sxfn.length()-ipos-1);

	// find if loaded
	CMoldElt* pme;
	CStringEx sxfullname = pl->WhereIsDfnTyp( sxfn );
	if (sxfullname == "")
		throw NLMISC::Exception ("Unable to find " + sxfn);

	sxfullname.make_lower();
	std::map< CStringEx, CMoldElt* >::iterator it;

	// DEBUG
	/*for (it = mmold.begin(); it != mmold.end(); ++it)
	{
		CMoldElt *pME = it->second;
		CStringEx sTmp = it->first + " Name:" + pME->GetName() + " formula:" + pME->GetFormula();
		nlwarning (sTmp.c_str());
	}*/
	// DEBUG

	it = mmold.find( sxfullname );
	if( it != mmold.end() )
		if( blst )
		{
			if( sxfileextension == "dfn" )
				pme = new CMoldEltDefineList( pl, dynamic_cast< CMoldEltDefine* >( it->second ) );
			else if( sxfileextension == "typ" )
					pme = new CMoldEltTypeList( pl, dynamic_cast< CMoldEltType* >( it->second ) );
				else
					return 0;
			pme->SetName( it->second->GetName() );
			vpme.push_back( pme );			
			return( pme );
		}
		else
			return( it->second );

	// load
	if( sxfileextension == "dfn" )
	{
		pme = new CMoldEltDefine( pl );
		vpme.push_back( pme );			
		if( blst )
		{
			pme = new CMoldEltDefineList( pl, dynamic_cast< CMoldEltDefine* >( pme ) );
			vpme.push_back( pme );			
		}
	}
	else if( sxfileextension == "typ" )
		{
			pme = new CMoldEltType( pl );
			vpme.push_back( pme );			
			if( blst )
			{
				pme = new CMoldEltTypeList( pl, dynamic_cast< CMoldEltType* >( pme ) );
				vpme.push_back( pme );			
			}
		}
	else 
		return( 0 );

	pme->SetName( sxfn );
	pme->Load( sxfullname );
	mmold.insert( std::make_pair( sxfullname, pme->GetMold() ) );
	return( pme );
}

CMoldElt* CMoldLoader::LoadMold( const CStringEx _sxfilename, const CStringEx _sxdate )
{
	return( 0 );
}
/*
CMoldElt* CMoldLoader::Find( const CStringEx _sxfullname )
{
	std::map< CStringEx, CMoldElt* >::iterator it = mmold.find( _sxfullname );
	if( it != mmold.end() )
		return( it->second );
	return( 0 );
}
*/

}