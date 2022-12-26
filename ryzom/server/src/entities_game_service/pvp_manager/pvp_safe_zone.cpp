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


#include "stdpch.h"
#include "nel/ligo/primitive_utils.h"

#include "pvp_manager/pvp_safe_zone.h"
#include "primitives_parser.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(CPVPSafeZone);

//----------------------------------------------------------------------------
CPVPSafeZone::CPVPSafeZone()
: _Alias(CAIAliasTranslator::Invalid), _Center(CVector::Null), _SqrRadius(0)
{
}

#define PRIM_ASSERT(exp) \
	nlassertex( exp, ("<CPVPSafeZone::build> fatal error in primitive: '%s'", NLLIGO::buildPrimPath(point).c_str() ) )

#define PRIM_VERIFY(exp) \
	nlverifyex( exp, ("<CPVPSafeZone::build> fatal error in primitive: '%s'", NLLIGO::buildPrimPath(point).c_str() ) )

//----------------------------------------------------------------------------
NLMISC::CSmartPtr<CPVPSafeZone> CPVPSafeZone::build(const NLLIGO::CPrimPoint * point)
{
	nlassert(point);

	string value;

	CSmartPtr<CPVPSafeZone> safeZone = new CPVPSafeZone;
	nlassert( !safeZone.isNull() );

	PRIM_VERIFY( point->getPropertyByName("name", safeZone->_Name) );

	PRIM_VERIFY( CPrimitivesParser::getAlias(point, safeZone->_Alias) );
	PRIM_ASSERT( safeZone->_Alias != CAIAliasTranslator::Invalid );

	safeZone->_Center = point->Point;

	PRIM_VERIFY( point->getPropertyByName("radius", value) );
	const float radius = (float) atof(value.c_str());
	if (radius == 0)
		return NULL;
	safeZone->_SqrRadius = radius * radius;

	return safeZone;
}

#undef PRIM_ASSERT
#undef PRIM_VERIFY

//----------------------------------------------------------------------------
bool CPVPSafeZone::contains(const NLMISC::CVector & v) const
{
	if ( (_Center - v).sqrnorm() <= _SqrRadius )
		return true;

	return false;
}
