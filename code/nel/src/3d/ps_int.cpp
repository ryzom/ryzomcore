// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "std3d.h"

#include "nel/3d/ps_int.h"
#include "nel/3d/ps_register_int_attribs.h"


namespace NL3D {


sint32 CPSIntGradient::_DefaultGradient[] = { 0, 10 };
uint32 CPSUIntGradient::_DefaultGradient[] = { 0, 10 };


CPSIntGradient::CPSIntGradient(const sint32 *intTab, uint32 nbValues, uint32 nbStages, float nbCycles)
				: CPSValueGradient<sint32>(nbCycles)
{
	_F.setValues(intTab, nbValues, nbStages);
}


CPSUIntGradient::CPSUIntGradient(const uint32 *intTab, uint32 nbValues, uint32 nbStages, float nbCycles)
				: CPSValueGradient<uint32>(nbCycles)
{
	_F.setValues(intTab, nbValues, nbStages);
}


/// Register attribute makers based on int (used in particle systems)
void PSRegisterIntAttribs()
{
	NLMISC_REGISTER_CLASS(CPSIntBlender);
	NLMISC_REGISTER_CLASS(CPSIntMemory);
	NLMISC_REGISTER_CLASS(CPSIntBinOp);
	NLMISC_REGISTER_CLASS(CPSIntGradient);
	NLMISC_REGISTER_CLASS(CPSUIntBlender);
	NLMISC_REGISTER_CLASS(CPSUIntMemory);
	NLMISC_REGISTER_CLASS(CPSUIntBinOp);
	NLMISC_REGISTER_CLASS(CPSUIntGradient);
}


} // NL3D
