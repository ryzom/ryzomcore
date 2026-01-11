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



#include "stdpch.h"

#include "lod_character_user_manager.h"
#include "nel/3d/u_scene.h"
#include "nel/misc/debug.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/path.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"

// Client
#include "sheet_manager.h"



///////////
// USING //
///////////
using namespace NLMISC;
using namespace NLGEORGES;
using namespace NL3D;
using namespace std;


////////////
// EXTERN //
////////////
extern UScene	*Scene;


// ***************************************************************************
CLodCharacterUserManager	LodCharacterUserManager;


// ***************************************************************************
CLodCharacterUserManager::CLodCharacterUserManager()
{
}


// ***************************************************************************
bool			CLodCharacterUserManager::addLodShapeBank(const std::string &filename)
{
	// Scene must exist
	nlassert(Scene);

	try
	{
		// load and add the file to the main scene
		Scene->loadCLodShapeBank(filename);
	}
	catch(const Exception &e)
	{
		nlwarning(e.what());
		return false;
	}

	return true;
}


// ***************************************************************************
void			CLodCharacterUserManager::init()
{
	// Scene must exist
	nlassert(Scene);


	// add player LodShapeBank.
	addLodShapeBank("characters.clodbank");
	// add creatures LodShapeBank.
	addLodShapeBank("fauna.clodbank");

}

