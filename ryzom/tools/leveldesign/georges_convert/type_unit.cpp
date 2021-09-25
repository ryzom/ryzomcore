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
#include "type_unit.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnit::CTypeUnit( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf )
{
	sxlowlimit = _sxll;
	sxhighlimit = _sxhl;
	sxdefaultvalue = _sxdv;
	sxformula = _sxf;
}

CTypeUnit::~CTypeUnit()
{
}

CStringEx CTypeUnit::GetFormula() const
{
	return	sxformula;
}

CStringEx CTypeUnit::GetDefaultValue() const
{
	return	sxdefaultvalue;
}

CStringEx CTypeUnit::GetLowLimit() const
{
	return	sxlowlimit;
}

CStringEx CTypeUnit::GetHighLimit() const													
{
	return	sxhighlimit;
}

CStringEx CTypeUnit::Format( const CStringEx _sxvalue ) const
{
	return( _sxvalue );
}
									
CStringEx CTypeUnit::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	return( _sxbasevalue );
}

void CTypeUnit::SetDefaultValue( const CStringEx _sxdv )
{
	sxdefaultvalue = _sxdv;
}

void CTypeUnit::SetLowLimit( const CStringEx _sxll )
{
	sxlowlimit = _sxll;
}

void CTypeUnit::SetHighLimit( const CStringEx _sxhl )
{
	sxhighlimit = _sxhl;
}

}