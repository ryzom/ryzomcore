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



#ifndef NL_LOD_CHARACTER_USER_MANAGER_H
#define NL_LOD_CHARACTER_USER_MANAGER_H

#include "nel/misc/types_nl.h"
#include <map>


// ***************************************************************************
/**
 * Manager of Lod characters for entities.
 *	Use Scene.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLodCharacterUserManager
{
public:

	/// Constructor
	CLodCharacterUserManager();

	/** init, reseting manager, and loading default shapeBank
	 *	It load and setup "homins.lod_character_desc" too
	 */
	void			init();

	/// Add a .clodbank. nlwarning and return false if not found. Added to "Scene".
	bool			addLodShapeBank(const std::string &filename);


// *****************
private:

};


extern	CLodCharacterUserManager	LodCharacterUserManager;


#endif // NL_LOD_CHARACTER_USER_MANAGER_H

/* End of lod_character_user_manager.h */
