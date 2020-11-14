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

#ifndef NL_SKELETON_SHAPE_H
#define NL_SKELETON_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/shape.h"
#include "nel/3d/bone.h"
#include "nel/misc/aabbox.h"
#include <vector>
#include <map>


namespace NL3D
{


// ***************************************************************************
/**
 * a  definition of a skeleton. can be instanciated into a CSkeletonModel.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSkeletonShape : public IShape
{
public:

	/// This is a lod for skeleton.
	struct	CLod
	{
		/// The distance of activation of this bone.
		float				Distance;
		/// Size of Bones. If a bone is active in this lod, 0xFF, else 0.
		std::vector<uint8>	ActiveBones;

	public:
		void	serial(NLMISC::IStream &f);
	};

public:

	/// Constructor
	CSkeletonShape();

	/** Build a skeletonShape, replacing old.
	 * WARNING: bones must be organized in Depth-first order (this is not checked).
	 * Bone.LodDisableDistance are minimized such sons have always a distance <= father distance.
	 */
	void			build(const std::vector<CBoneBase> &bones);

	/** Retrieve Bones Information.
	 */
	void			retrieve(std::vector<CBoneBase> &bones) const;


	/// Return the id of a bone, from it's name. -1 if not present.
	sint32			getBoneIdByName(const std::string &name) const;


	/// \name From IShape
	// @{

	/// Create a CSkeletonModel, which contains bones.
	virtual	CTransformShape		*createInstance(CScene &scene);

	/// clip this skeleton.
	virtual bool	clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix);

	/// render() this skeletonshape in a driver  (no-op)
	virtual void	render(IDriver * /* drv */, CTransformShape * /* trans */, bool /* opaquePass */)
	{
	}

	/** return the bounding box of the shape. Default is to return Null bbox.
	 */
	virtual	void	getAABBox(NLMISC::CAABBox &bbox) const;

	/// get an approximation of the number of triangles this instance will render for a fixed distance.
	virtual float	getNumTriangles (float distance);

	/// serial this skeletonshape.
	virtual void	serial(NLMISC::IStream &f);
	NLMISC_DECLARE_CLASS(CSkeletonShape);

	/// flush textures used by this shape.
	virtual void	flushTextures (IDriver &/* driver */, uint /* selectedTexture */) {}

	// @}


	/// retrieve the lod to use for a given distance (log(n)).
	uint			getLodForDistance(float dist) const;

	/// get lod information.
	const CLod		&getLod(uint lod) const {return _Lods[lod];}
	uint			getNumLods() const {return (uint)_Lods.size();}

// ***************************
private:
	std::vector<CBoneBase>			_Bones;
	std::map<std::string, uint32>	_BoneMap;
	NLMISC::CAABBox					_BBox;

	std::vector<CLod>				_Lods;
};


} // NL3D


#endif // NL_SKELETON_SHAPE_H

/* End of skeleton_shape.h */
