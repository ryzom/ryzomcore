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

#ifndef NL_ANIM_DETAIL_TRAV_H
#define NL_ANIM_DETAIL_TRAV_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/plane.h"
#include "nel/3d/trav_scene.h"


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class	CTransform;


// ***************************************************************************
/**
 * The AnimDetail traversal.
 * There is no AnimDetail graph. traverse() use the clipTrav VisibilityList to traverse all models.
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CAnimDetailTrav : public CTraversal
{
public:

	/// Constructor
	CAnimDetailTrav();


	/// \name ITrav/ITravScene Implementation.
	//@{
	/** render after Clip and before light.
	 * This order is important for possible lights sticked to bones of skeletons.
	 */
	void				traverse();
	//@}


public:
	// ONLY FOR MODEL TRAVERSING.

	sint64		CurrentDate;	// The current date of the traversal, useful for evaldetail just one time..


	// For clipTrav. cleared at beginning of CClipTrav::traverse
	void				clearVisibleList();

	// For ClipTrav only. NB: list is cleared at beginning of traverse(). NB: only CTransform are supported
	void				addVisibleModel(CTransform *model)
	{
		_VisibleList[_CurrentNumVisibleModels]= model;
		_CurrentNumVisibleModels++;
	}

	// for createModel().
	void				reserveVisibleList(uint numModels);


// ********************
private:
	/// traverse the model recursively, following Hrc hierarchy
	void				traverseHrcRecurs(CTransform *model);

	// traverse list of model visible and useful to animDetail.
	std::vector<CTransform*>	_VisibleList;
	uint32						_CurrentNumVisibleModels;

};


} // NL3D


#endif // NL_ANIM_DETAIL_TRAV_H

/* End of anim_detail_trav.h */
