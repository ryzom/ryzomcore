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

/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/path.h"
#include "nel/misc/file.h"
// 3D Interface.
#include "nel/3d/u_scene.h"
#include "nel/3d/u_animation_set.h"
// Client.
#include "auto_anim.h"
// std.
#include <string>
#include <fstream>


///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace NLNET;
using namespace std;


////////////
// EXTERN //
////////////
extern UDriver	*Driver;
extern UScene	*Scene;


// ***************************************************************************
static NL3D::UAnimationSet	*AutoAnimSet= NULL;

// ***************************************************************************
void initAutoAnimation()
{
	/* Load the automatic animations.
	 * Automatics animations are listed in the file auto_animatons_list.txt
	 * Each of this animations are loaded into an animations set
	 * The animation set is gived to the scene as default automatique animation set
	 */
	CIFile file;
	string listFilename = CPath::lookup("auto_animations_list.txt", false, false, false);
	if (listFilename.empty() || !file.open (listFilename))
	{
		nlwarning ("No automatic animation files list (auto_animations_list.txt)");
	}
	else
	{
		nlassert(!AutoAnimSet);
		// Create an animation set
		AutoAnimSet = Driver->createAnimationSet();
		nlassert (AutoAnimSet);

		while (!file.eof())
		{
			// Read a filename
			char line[512];
			file.getline(line, 512);

			// Read the animation file
			string animName = strlwr (CFile::getFilenameWithoutExtension(line));
			uint id = AutoAnimSet->addAnimation (line, animName.c_str ());
			if (id == UAnimationSet::NotFound)
			{
				nlwarning ("Can't load automatic animation '%s'", line);
			}
		}

		// Add the animation set
		AutoAnimSet->build ();
		Scene->setAutomaticAnimationSet (AutoAnimSet);
	}
}

// ***************************************************************************
void releaseAutoAnimation()
{
	// if already created
	if(AutoAnimSet)
	{
		Driver->deleteAnimationSet(AutoAnimSet);
		AutoAnimSet= NULL;
	}
}

