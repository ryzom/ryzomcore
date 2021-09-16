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

#ifndef NL_LOD_CHARACTER_BUILDER_H
#define NL_LOD_CHARACTER_BUILDER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/lod_character_shape.h"


namespace NL3D
{


class	CSkeletonShape;
class	CAnimation;


// ***************************************************************************
/**
 * A tool class used to build a CLodCharacterShape
 *	Use it by first calling setShape(), then add animations to it, and finally get result with getLodShape
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLodCharacterBuilder
{
public:

	/// Constructor
	CLodCharacterBuilder();
	~CLodCharacterBuilder();

	/** init process with the Mesh and the SkeletonShape to use.
	 *	NB: nlWarnings may occurs if don't find bones used by lodBuild, in the skeletonShape.
	 *	In this case, root bone 0 of the skeleton shape is used.
	 *
	 *	\param name is the name to give to the shape
	 *	\param skeletonShape the skeleton used for animation. Ptr is handled by the builder as a smartPtr.
	 *	\param lodBuild the mesh information. Ptr is hold by the builder, but not deleted
	 */
	void			setShape(const std::string &name, CSkeletonShape *skeletonShape, CLodCharacterShapeBuild *lodBuild);

	/** add an animation to the process
	 *	\param animName is the name of the animation, used as a key to receover animations in the CLodCharacterShape built
	 *	\param animation is the animation to bake/over sample. NB: the animation is deleted at the end of addAnim
	 *	\param frameRate is the desired overSampling rate. eg: 20 frame per second.
	 */
	void			addAnim(const char *animName, CAnimation *animation, float frameRate);

	/// return the lod shape in its current state.
	const CLodCharacterShape	&getLodShape() const {return _LodCharacterShape;}


// *****************************
private:
	CLodCharacterShape			_LodCharacterShape;

	NLMISC::CSmartPtr<CSkeletonShape>	_SkeletonShape;
	CLodCharacterShapeBuild		*_LodBuild;
	std::vector<uint>			_BoneRemap;

	// A Tmp Scene, builded at setShape/dtor.
	CScene						*_TmpScene;

	// apply the skin, from _LodBuild, into dstVertices array
	void			applySkin(CSkeletonModel *skeleton, CVector	*dstVertices);
};


} // NL3D


#endif // NL_LOD_CHARACTER_BUILDER_H

/* End of lod_character_builder.h */
