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

#ifndef NL_ROOT_MODEL_H
#define NL_ROOT_MODEL_H

#include "nel/misc/types_nl.h"
#include "nel/3d/transform.h"


namespace NL3D {


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		RootModelId=NLMISC::CClassId(0x25f0505d, 0x75c69f9);


// ***************************************************************************
/**
 * The purpose of this model is to do nothing in traverse*() but traverseSons() for Hrc and Clip
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CRootModel : public CTransform
{
public:
	/// Call at the beginning of the program, to register the model
	static	void	registerBasic();


	/// \name CTransform traverse specialisation
	// @{
	virtual void	traverseHrc() NL_OVERRIDE;
	virtual void	traverseClip() NL_OVERRIDE;
	virtual void	traverseAnimDetail() NL_OVERRIDE;
	virtual void	traverseLoadBalancing() NL_OVERRIDE;
	virtual void	traverseLight() NL_OVERRIDE;
	virtual void	traverseRender() NL_OVERRIDE;
	// @}


protected:
	/// Constructor
	CRootModel() {}
	/// Destructor
	virtual ~CRootModel() NL_OVERRIDE {}

private:
	static CTransform	*creator() {return new CRootModel;}

};


} // NL3D


#endif // NL_ROOT_MODEL_H

/* End of root_model.h */
