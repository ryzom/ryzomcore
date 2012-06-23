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

#include "nel/3d/animated_morph.h"
#include "nel/misc/common.h"

using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

CMorphBase::CMorphBase()
{
	DefaultFactor.setDefaultValue (0.0f);
}

// ***************************************************************************
void CMorphBase::serial(NLMISC::IStream &f)
{
	f.serial (Name);
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CAnimatedMorph::CAnimatedMorph (CMorphBase*mb)
{
	nlassert(mb);

	// IAnimatable.
	IAnimatable::resize (AnimValueLast);

	_MorphBase = mb;

	_Factor.Value= mb->DefaultFactor.getDefaultValue();
}

// ***************************************************************************
IAnimatedValue* CAnimatedMorph::getValue (uint valueId)
{
	switch(valueId)
	{
		case FactorValue: return &_Factor;
	};

	return NULL;
}
// ***************************************************************************
const char *CAnimatedMorph::getValueName (uint valueId) const
{
	switch(valueId)
	{
		case FactorValue: return "MorphFactor";
	};

	return "";
}
// ***************************************************************************
ITrack*	CAnimatedMorph::getDefaultTrack (uint valueId)
{
	nlassert(_MorphBase);

	switch(valueId)
	{
		case FactorValue: return &_MorphBase->DefaultFactor;
	};

	return NULL;
}
// ***************************************************************************
void	CAnimatedMorph::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	// For CAnimatedMorph, channels are detailled (morph evaluated after clip)!
	addValue(chanMixer, FactorValue, OwnerBit, prefix, true);

}


} // NL3D
