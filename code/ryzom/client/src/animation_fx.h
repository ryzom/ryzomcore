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



#ifndef CL_ANIMATION_FX_H
#define CL_ANIMATION_FX_H

#include "client_sheets/animation_fx_sheet.h"
#include "client_sheets/animation_fx_set_sheet.h"




namespace NL3D
{
	class UTrack;
	class UAnimationSet;
	class UParticleSystemInstance;
}

namespace NLGEORGES
{
	class UFormElm;
}

/** Sheet of a fx that must be played with an animation.
  * Such an fx has the following properties :
  * - it can have a color
  * - it can be sticked to a bone when used with a skeleton
  * - it can follow a position spline for more sophisticated effects.
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CAnimationFX
{
public:

	NL3D::UTrack			*PosTrack; // Filled after init : additionnal pos track to animate the fx
	const CAnimationFXSheet	*Sheet;

public:

	// ctor
	CAnimationFX(const std::string &psName = "", const float *userParams = NULL);

	void init(const CAnimationFXSheet *sheet, NL3D::UAnimationSet *as);

	/** helper : create instance matching that sheet. NB : the instance is not sticked neither positionned.
	  * 4 user params may be supplied to replace the default ones
	  */
	NL3D::UParticleSystemInstance createMatchingInstance(const float *customUserParams = NULL) const;

private:
	// build track for that fx, using the given animation set
	void buildTrack(NL3D::UAnimationSet *as);
};


/** A set of anim fx serialized from a .animation_fx_set sheet.
  * Those fx are intended to be all instanciated at once, hence the small number of instances
  */
class CAnimationFXSet
{
public:
	enum { MaxNumFX = CAnimationFXSetSheet::MaxNumFX };
	std::vector<CAnimationFX> FX;	// matching track for each fx
	const CAnimationFXSetSheet      *Sheet;
public:
	CAnimationFXSet() : Sheet(NULL) {}
	void init(const CAnimationFXSetSheet *sheet, NL3D::UAnimationSet *as);
};

#endif

