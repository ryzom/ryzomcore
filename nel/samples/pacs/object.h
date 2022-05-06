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

#ifndef NL_OBJECT_H
#define NL_OBJECT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_global_position.h"
#include "nel/3d/u_instance.h"

// External classes

namespace NLNET
{
class CPacsClient;
};

namespace NLPACS
{
class UMoveContainer;
class UMovePrimitive;
class UGlobalPosition;
};

namespace NL3D
{
class UScene;
};

/**
 * object for the test
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CObjectDyn
{
public:

	/// Box constructor
	CObjectDyn (double width, double depth, double height, double orientation, const NLMISC::CVectorD& pos,
		const NLMISC::CVectorD& speed, bool obstacle, NLPACS::UMoveContainer &container, NL3D::UScene &scene, 
		NLPACS::UMovePrimitive::TReaction reaction, NLPACS::UMovePrimitive::TTrigger trigger, 
		uint8 worldImage, uint8 nbImage, uint8 insertWorldImage);

	/// Cylinder constructor
	CObjectDyn (double radius, double height, const NLMISC::CVectorD& pos, const NLMISC::CVectorD& speed, 
		bool obstacle, NLPACS::UMoveContainer &container, NL3D::UScene &scene, 
		NLPACS::UMovePrimitive::TReaction reaction, NLPACS::UMovePrimitive::TTrigger trigger, 
		uint8 worldImage, uint8 nbImage, uint8 insertWorldImage);

	/// Set position
	void			setPos (const NLMISC::CVectorD& pos);
	void			setGlobalPos (NLPACS::UGlobalPosition& gpos, NLMISC::CVectorD& pos, uint8 worldimage);

	/// Set position
	void	setSpeed (const NLMISC::CVectorD& speed);

	/// Set position
	const NLMISC::CVectorD&	getPos () const
	{
		return _Position;
	}

	/// Set position
	const NLMISC::CVectorD&	getSpeed () const
	{
		return _Speed;
	}

	/// Simulate
	void			tryMove (double deltaTime, NLPACS::UMoveContainer &container, uint8 worldImage);
	void			doMove (double deltaTime, uint8 worldImage);

	/// Remove object from container
	void			remove (NLPACS::UMoveContainer &container, NL3D::UScene &scene);

	bool						Freezed;
	
private:

	bool						_TryMove;
	NLMISC::CVectorD			_Position;
	NLMISC::CVectorD			_TryPosition;
	NLMISC::CVectorD			_Speed;
	NLPACS::UMovePrimitive		*_MovePrimitive;
	NL3D::UInstance				_Instance;
};


#endif // NL_OBJECT_H

/* End of object.h */
