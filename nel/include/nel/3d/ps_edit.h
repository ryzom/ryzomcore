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

#ifndef NL_PS_EDIT_H
#define NL_PS_EDIT_H

#include "nel/misc/types_nl.h"



namespace NLMISC
{
	class CMatrix ;
	class CVector ;
} ;


namespace NL3D {


/**
 * In this file, we define interfaces to interact more precisely with located in a particle system
 * It allows to manipulate individual elements of a located
 * For example, it allows you to move a collision plane after its creation,
 * or to move a particular particle that you've instancied yourself.
 * This is needed because of the packed format used for data representation
 * (each attribute is packed into its own table, see ps_attrib.h )
 */



/**
 * This interface allow to move a particle system element given its index in a located
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */


struct IPSMover
{
	virtual ~IPSMover() {}

	/** Send back true if uniform scaling can be applied
	 *  If it sends false, uniform scaling leads to undefine results
	 */
	virtual bool					supportUniformScaling(void) const { NL_PS_FUNC(supportUniformScaling); return false ; }

	/** Send back true if non-uniform scaling can be applied
	 *  If it sends false, non-uniform scaling leads to undefine results (default has no effect)
	 */
	virtual bool					supportNonUniformScaling(void) const {	NL_PS_FUNC(supportNonUniformScaling); return false ; }

	// set the scale of the object (uniform scale). The default does nothing
	virtual void					setScale(uint32 /* index */, float /* scale */) {}

	// set a non uniform scale (if supported)
	virtual void					setScale(uint32 /* index */, const NLMISC::CVector &/* s */) { NL_PS_FUNC(setScale); }

	// get the scale of the object
	virtual	NLMISC::CVector			getScale(uint32 /* index */) const { NL_PS_FUNC(getScale); return NLMISC::CVector(1.f, 1.f, 1.f) ; }

	/** some object may not store a whole matrix (e.g planes)
	 *  this return true if only a normal is needed to set the orientation of the object
	 */
	virtual bool					onlyStoreNormal(void) const { NL_PS_FUNC(onlyStoreNormal); return false ; }

	///  if the object only needs a normal, this return the normal. If not, is return (0, 0, 0)
	virtual NLMISC::CVector			getNormal(uint32 /* index */) { NL_PS_FUNC(getNormal); return NLMISC::CVector::Null ; }

	/// if the object only stores a normal, this set the normal of the object. Otherwise it has no effect
	virtual void					setNormal(uint32 /* index */, NLMISC::CVector /* n */) { NL_PS_FUNC(setNormal); }

	// set a new orthogonal matrix for the object
	virtual void					setMatrix(uint32 index, const NLMISC::CMatrix &m) = 0 ;
	// return an orthogonal matrix of the system. No valid index -> assert
	virtual NLMISC::CMatrix			getMatrix(uint32 index) const = 0 ;

};



} // NL3D


#endif // NL_PS_EDIT_H

/* End of ps_edit.h */
