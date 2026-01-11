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

#ifndef NL_U_BONE_H
#define NL_U_BONE_H

#include "nel/misc/types_nl.h"
#include "u_transformable.h"


namespace NL3D
{

// ***************************************************************************
/**
 * Base interface for manipulating Bones.
 * see UTransformable. A bone comes from a USkeleton.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UBone : public UTransformable
{
public:

	/// \name Special feature
	// @{

	/** get the last world matrix computed in last render().
	 *	NB: this WM is computed in last render() only if the object was not clipped. So use it wisely.
	 *
	 *	NB: this WM may also not be computed in last render() for "Lod skeleton" reason. ie if the skeleton
	 *	is too far, the engine may not compute a bone (for speed). To avoid problem, you should ask the artist what
	 *	bones have such a scheme, or you could stickObject() a dummy (eg a UTransform) onto this bone, because
	 *	in this case, this bone will always be computed.
	 *
	 */
	const CMatrix	&getLastWorldMatrixComputed() const;

	/** Additionally to the standard scale, you can multiply the effect on the skin with a special SkinScale
	 *	This scale is applied only on the skin (even son bones positions won't be affected)
	 *	Default to (1,1,1)
	 */
	void			setSkinScale(CVector &skinScale);
	const CVector	&getSkinScale() const;

	// @}

	/// Proxy interface

	/// Constructors
	UBone() { _Object = NULL; }
	UBone(class CBone *object) { _Object = (ITransformable*)object; };
	/// Attach an object to this proxy
	void			attach(class CBone *object) { _Object = (ITransformable*)object; }
	/// Detach the object
	void			detach() { _Object = NULL; }
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const {return _Object==NULL;}
	/// For advanced usage, get the internal object ptr
	class CBone	*getObjectPtr() const {return (CBone*)_Object;}
};


} // NL3D


#endif // NL_U_BONE_H

/* End of u_bone.h */
