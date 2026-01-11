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

#ifndef NL_ANIMATED_MORPH_H
#define NL_ANIMATED_MORPH_H


#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/animatable.h"
#include "nel/3d/track.h"
#include <map>


namespace NL3D
{

// ***************************************************************************
/**
 * An animated morph (blend shape) base
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class CMorphBase : public NLMISC::CRefCount
{
public:

	CMorphBase();

	// Name of this material, for Animation access.
	std::string				Name;

	// Default tracks.
	CTrackDefaultFloat		DefaultFactor;

	/// save/load.
	void			serial(NLMISC::IStream &f);
};


// ***************************************************************************
/**
 * An animated morph (blend shape)
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class CAnimatedMorph : public IAnimatable
{
public:

	/// \name ctor / setup.
	// @{
	/** Constructor.
	 */
	CAnimatedMorph (CMorphBase*mb);
	// @}


	/// \name Herited from IAnimatable
	// @{
	/// Added values.
	enum	TAnimValues
	{
		OwnerBit= IAnimatable::AnimValueLast,
		FactorValue,
		AnimValueLast
	};

	/// From IAnimatable
	virtual IAnimatedValue* getValue (uint valueId);

	/// From IAnimatable
	virtual const char *getValueName (uint valueId) const;

	/// From IAnimatable.
	virtual ITrack* getDefaultTrack (uint valueId);

	/// From IAnimatable.
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);

	// @}

	float	getFactor() const { return _Factor.Value; }
	void	setFactor (float rFactor) { _Factor.Value = rFactor; }

	const std::string& getName() const { return _MorphBase->Name; }

// ********************
private:
	CMorphBase				*_MorphBase;

	CAnimatedValueFloat		_Factor;
};


} // NL3D


#endif // NL_ANIMATED_MORPH_H

/* End of animated_morph.h */
