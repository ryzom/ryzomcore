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
#include "mold_elt_define.h"
#include "form_body_elt_struct.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldEltDefine::CMoldEltDefine( CLoader* const _pl ) : CMoldElt( _pl )
{
}

CMoldEltDefine::~CMoldEltDefine()
{
}

void CMoldEltDefine::Load( const CStringEx _sxfilename )
{
	CForm f;
	pl->LoadForm( f, _sxfilename );
	CFormBodyElt* pbody = f.GetBody();

	unsigned int i = 0;
	CFormBodyElt* pfbeatom = pbody->GetElt( i++ );
	while( pfbeatom )
	{
		CStringEx sxvalue = pfbeatom->GetValue();
		CStringEx sxname = pfbeatom->GetName();
		pfbeatom = pbody->GetElt( i++ );
		CMoldElt* pme = pl->LoadMold( sxvalue );
		if( pme )
			vpair.push_back( std::make_pair( sxname, pme ) );
	}
}

void CMoldEltDefine::Load( const CStringEx _sxfilename, const CStringEx _sxdate )
{
}

CStringEx CMoldEltDefine::GetEltName( const unsigned int _index ) const
{
	if( _index < vpair.size() )
		return( vpair[_index].first );
	return( 0 );
}

CMoldElt* CMoldEltDefine::GetEltPtr( const unsigned int _index ) const
{
	if( _index < vpair.size() )
		return( vpair[_index].second );
	return( 0 );
}

unsigned int CMoldEltDefine::GetType() const
{
	return( 1 );
}

CMoldElt* CMoldEltDefine::GetMold()
{
	return( this );
}

}