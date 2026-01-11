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

#ifndef NL_ANIM_CTRL_H
#define NL_ANIM_CTRL_H

#include "nel/misc/types_nl.h"


namespace NL3D
{


// ***************************************************************************
class	CBone;
class	CSkeletonModel;


// ***************************************************************************
/**
 * This is a handler for extra Animation behavior on Skeleton Models.
 *	When a bone has ended his compute, if it has an attached AnimCtrl, it calls its ::execute() code
 *	The AnimCtrl may modify all or part of the skeleton, knowing that the sons of the bone animCtrled are still not
 *	computed.
 *	Typically, animCtrls should modify the matrix parts of the bone, and then call bone->compute(..,..,NULL)
 *
 *	Note: for convenience, when execute() is called on a bone, the LocalSkeletonMatrix and WorldMatrix
 *		of this bone are already computed.
 *
 *	Important Note about Bone Loding:
 *		- AnimCtrl are not called if the bone is loded (ie not animated/computed)
 *		- AnimCtrl is called BEFORE Bone Lod interpolation, resulting in the excepted behavior.
 *
 *	Note about UserInterface logic: CSkeletonModel and CBone appears here but are used only for the derivers of this
 *		class (that are still in 3D), so the UserInterface don't need and don't use it.
 *		Therefore, there is no way (for now) with the strict UserInterface scheme to implements new AnimCtrls.
 *		But the user is allowed to use the Layer2 (ie CScene, CSkeletonModel etc classes) to create its own AnimCtrls.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class IAnimCtrl
{
public:
	virtual	~IAnimCtrl() {}

	/// Called at compute() time.
	virtual	void	execute(CSkeletonModel *model, CBone *bone) =0;

};


} // NL3D


#endif // NL_ANIM_CTRL_H

/* End of anim_ctrl.h */
