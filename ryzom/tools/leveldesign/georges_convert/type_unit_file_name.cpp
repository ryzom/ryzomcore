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
#include "type_unit_file_name.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnitFileName::CTypeUnitFileName( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf ) : CTypeUnit( _sxhl, _sxll, _sxdv, _sxf )
{
	if( sxformula.empty() )
		sxformula = "file name";
} 

CTypeUnitFileName::~CTypeUnitFileName()
{
}

CStringEx CTypeUnitFileName::Format( const CStringEx _sxvalue ) const
{
	if( _sxvalue.empty() )
		return( sxdefaultvalue );
	return( _sxvalue );
}
									
CStringEx CTypeUnitFileName::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	if( _sxvalue.empty() )
		return( Format( _sxbasevalue ) );
	return( Format( _sxvalue ) );
}

void CTypeUnitFileName::SetDefaultValue( const CStringEx _sxdv )
{
	sxdefaultvalue = _sxdv;
}

void CTypeUnitFileName::SetLowLimit( const CStringEx _sxll )
{
	sxlowlimit = _sxll;
}

void CTypeUnitFileName::SetHighLimit( const CStringEx _sxhl )
{
	sxhighlimit = _sxhl;
}

}