/**
 * \file NeLResourceProvider.h
 * \date January 2005
 * \author Matt Raykowski
 * \author Henri Kuuste
 */

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


/************************************************************************
	purpose:	Interface for main Nevrax Engine GUI renderer class

	For use with GUI Library:
	Crazy Eddie's GUI System (http://crayzedsgui.sourceforge.net)
    Copyright (C)2004 Paul D Turner (crayzed@users.sourceforge.net)

	This file contains code that is specific to NeL (http://www.nevrax.org)
*************************************************************************/

#ifndef __NELRESOURCEPROVIDER_H__
#define __NELRESOURCEPROVIDER_H__

// standard includes
#include <set>
#include <list>

// CEGUI includes
#include "CEGUIBase.h"
#include "CEGUIRenderer.h"
#include "CEGUITexture.h"
#include "CEGUISystem.h"

// NeL includes
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/debug.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/common.h>
#include <nel/misc/events.h>
#include <nel/misc/fast_mem.h>
#include <nel/misc/config_file.h>
#include <nel/misc/system_info.h>
#include <nel/misc/mem_displayer.h>

#include <nel/3d/u_scene.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_texture.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_particle_system_instance.h>

#ifdef NL_OS_WINDOWS
#ifdef NEL_CEGUIRENDERER_EXPORTS
#define DLLSPEC __declspec(dllexport)
#else //NEL_CEGUI_RENDERER_EXPORTS
#define DLLSPEC __declspec(dllimport)
#endif // NEL_CEGUI_RENDERER_EXPORTS
#else // NL_OS_WINDOWS
#define DLLSPEC 
#endif // NL_OS_WINDOWS

// Start of CEGUI namespace section
namespace CEGUI
{
	class DLLSPEC NeLResourceProvider : public ResourceProvider
	{
	public:
		NeLResourceProvider();
		~NeLResourceProvider();

		void loadRawDataContainer(const String& filename, RawDataContainer& output, const String& resourceGroup);
	};
}; // end namespace CEGUI

#endif // end __NELRESOURCEPROVIDER_H__
