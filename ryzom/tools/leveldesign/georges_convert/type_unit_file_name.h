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

#ifndef NLGEORGES_TYPE_UNIT_FILE_NAME_H
#define NLGEORGES_TYPE_UNIT_FILE_NAME_H

#include "type_unit.h"

namespace NLOLDGEORGES
{

class CTypeUnitFileName : public CTypeUnit  
{
protected:
	unsigned short int	usihighlimit;

public:
	CTypeUnitFileName( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf );
	virtual ~CTypeUnitFileName();

	virtual	CStringEx Format( const CStringEx _sxvalue ) const;									
	virtual	CStringEx CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const;	
	virtual	void	SetDefaultValue( const CStringEx _sxdv );
	virtual	void	SetLowLimit( const CStringEx _sxll );
	virtual	void	SetHighLimit( const CStringEx _sxhl );
};

} // NLGEORGES

#endif // NLGEORGES_TYPE_UNIT_FILE_NAME_H
