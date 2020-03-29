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



#ifndef _WORLD_MAP_LINK_
#define	_WORLD_MAP_LINK_

#include "ai_share/ai_entity_physical_list_link.h"

//------------------------------------------------------------------
// linkage to dynamic world map
template	<class	T>
class	CWorldMapLink
{
public:	
	CWorldMapLink	()	{}
	virtual ~CWorldMapLink() {}

	inline	bool	isLinkedToWorldMap() const	
	{
		return	!_matrixListLink.unlinked();
	}
	
	//	link the entity to its type Matrix.
	inline	void	linkEntityToMatrix	(const CAIVectorMirror&	pos, CAIEntityMatrix<T>&	matrix);
	inline	void	linkEntityToMatrix	(const CAIVector&		pos, CAIEntityMatrix<T>&	matrix);
	
	inline	void	linkToWorldMap		(T *entity, const	CAIVectorMirror	&pos, CAIEntityMatrix<T>&	matrix);
	inline	void	linkToWorldMap		(T *entity, const	CAIVector		&pos, CAIEntityMatrix<T>&	matrix);
	
	inline	void	unlinkFromWorldMap()
	{
		_matrixListLink.unlink();
	}
	
protected:
	inline	void	linkEntityToMatrix	(const CAICoord& x, const CAICoord& y, CAIEntityMatrix<T>&	matrix);
private:
	CEntityListLink<T>	_matrixListLink;	// link for the entity matrix used for vision systems
};

#endif
