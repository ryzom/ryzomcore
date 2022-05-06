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

#ifndef __INELLIBRARY_H__
#define __INELLIBRARY_H__

#include <nel/misc/dynloadlib.h>
#include <nel/3d/u_driver.h>
#include <CEGUI/CEGUIRenderer.h>

class CCeguiRendererNelLibrary : public NLMISC::INelLibrary {
        void onLibraryLoaded(bool /* firstTime */) { }
        void onLibraryUnloaded(bool /* lastTime */) { }
};

const char *NELRENDERER_CREATE_PROC_NAME = "createNeLRendererInstance";
typedef CEGUI::Renderer* (*NELRENDERER_CREATE_PROC)(NL3D::UDriver *, bool);

#endif // __INELLIBRARY_H__
