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
#include "nel/3d/u_instance_material.h"
#include "sky_material_setup.h"

using namespace NL3D;

H_AUTO_DECL(RZ_SkyMaterialSetup)

//===================================================================================
void CSkyMaterialSetup::buildFromInstance(NL3D::UInstance instance, uint stage)
{
	H_AUTO_USE(RZ_SkyMaterialSetup)
	nlassert(stage < 2);
	if (instance.empty())
	{
		Setup.clear();
		return;
	}
	Setup.clear();
	CTexInfo ti;
	// take each texture at the given stage that is a texture file
	for(uint k = 0; k < instance.getNumMaterials(); ++k)
	{
		UInstanceMaterial im = instance.getMaterial(k);
		uint currStage;
		if (im.getLastTextureStage() >= (sint) stage) currStage = stage;
		else currStage = 0;
		if (im.isTextureFile(currStage))
		{
			ti.MatNum = k;
			ti.TexName = im.getTextureFileName(currStage);
			Setup.push_back(ti);
		}
	}
}

//===================================================================================
void CSkyMaterialSetup::applyToInstance(NL3D::UInstance instance, uint stage, bool skipFirstMaterial /*= false*/)
{
	H_AUTO_USE(RZ_SkyMaterialSetup)
	nlassert(stage < 2);
	if (instance.empty()) return;
	for(uint k = 0; k < Setup.size(); ++k)
	{
		if (skipFirstMaterial && Setup[k].MatNum == 0) continue;
		if (Setup[k].MatNum < instance.getNumMaterials())
		{
			NL3D::UInstanceMaterial im = instance.getMaterial(Setup[k].MatNum);
			uint currStage;
			if (im.getLastTextureStage() >= (sint) stage) currStage = stage;
			else currStage = 0;
			if (!im.isTextureFile(currStage)) continue;
			if (NLMISC::nlstricmp(Setup[k].TexName, im.getTextureFileName(currStage)) != 0) // must change name ?
			{
				im.setTextureFileName(Setup[k].TexName, currStage);
			}
		}
	}
}

