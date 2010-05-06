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

#ifndef NLGEORGES_TYPE_UNIT_SIGNED_H
#define NLGEORGES_TYPE_UNIT_SIGNED_H

#include "type_unit.h"

namespace NLOLDGEORGES
{

class CTypeUnitIntSigned : public CTypeUnit  
{
protected:
	sint64	ilowlimit;
	sint64	ihighlimit;
	sint64	idefaultvalue;

public:
	CTypeUnitIntSigned( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf );
	virtual ~CTypeUnitIntSigned();

	virtual	CStringEx Format( const CStringEx _sxvalue ) const;									
	virtual	CStringEx CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const;	
	virtual	void	SetDefaultValue( const CStringEx _sxdv );
	virtual	void	SetLowLimit( const CStringEx _sxll );
	virtual	void	SetHighLimit( const CStringEx _sxhl );
};

} // NLGEORGES

#endif // NLGEORGES_TYPE_UNIT_SIGNED_H
