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

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdgeorgesconvert.h"
#include "mold_elt.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldElt::CMoldElt( CLoader* const _pl )
{
	pl = _pl;
	benum = false;
	blist = false;
}

CMoldElt::~CMoldElt()
{
}

CStringEx CMoldElt::GetFormula()
{
	return sxname;
}

void CMoldElt::Load( const CStringEx _sxfullname )
{
}

void CMoldElt::Load( const CStringEx _sxfullname, const CStringEx _sxdate )
{
}

CStringEx CMoldElt::GetEltName( const unsigned int _index ) const
{
  CStringEx object;
	return( object );
}

CMoldElt* CMoldElt::GetEltPtr( const unsigned int _index ) const
{
	return( 0 );
}

unsigned int CMoldElt::GetType() const
{
	return( -1 );
}

CStringEx CMoldElt::GetName() const
{
	return( sxname );
}

void CMoldElt::SetName( const CStringEx& _sxname )
{
	sxname = _sxname;
}

bool CMoldElt::IsEnum() const
{
	return( benum );
}

bool CMoldElt::IsList() const
{
	return( blist );
}

CMoldElt* CMoldElt::GetMold()
{
	return( this );
}

}
