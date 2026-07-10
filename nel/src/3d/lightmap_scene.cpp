// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "std3d.h"

#include "nel/3d/lightmap_scene.h"

#include "nel/misc/file.h"

using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************
// CLightmapLight
// ***************************************************************************

CLightmapLight::CLightmapLight()
{
	LightGroup = 0;
	Type = LightPoint;
	Position.set(0.f, 0.f, 0.f);
	Direction.set(1.f, 0.f, 0.f);
	rRadiusMin = 1.f;
	rRadiusMax = 2.f;
	rHotspot = 0.f;
	rFallof = 0.f;
	Ambient.set(0, 0, 0, 0);
	Diffuse.set(0, 0, 0, 0);
	Specular.set(0, 0, 0, 0);
	bCastShadow = false;
	bAmbientOnly = false;
	rMult = 1.f;
	mProj.identity();
	rSoftShadowRadius = 1.4f;
	rSoftShadowConeLength = 15.f;
	rDirRadius = 0.f;
}

// ***************************************************************************
void CLightmapLight::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serial(Name);
	f.serial(AnimatedLight);
	f.serial(LightGroup);
	f.serialEnum(Type);
	f.serial(Position);
	f.serial(Direction);
	f.serial(rRadiusMin);
	f.serial(rRadiusMax);
	f.serial(rHotspot);
	f.serial(rFallof);
	f.serial(Ambient);
	f.serial(Diffuse);
	f.serial(Specular);
	f.serial(bCastShadow);
	f.serial(bAmbientOnly);
	f.serial(rMult);

	// Projector bitmap: raw RGBA pixels (mipmaps rebuilt on read)
	{
		uint32 w, h;
		if (f.isReading())
		{
			f.serial(w);
			f.serial(h);
			if (w && h)
			{
				ProjBitmap.resize(w, h, CBitmap::RGBA);
				f.serialBuffer(&ProjBitmap.getPixels()[0], w * h * 4);
				ProjBitmap.buildMipMaps();
			}
			else
			{
				ProjBitmap.reset();
			}
		}
		else
		{
			// Only an RGBA projector is ever stored (empty when no projector)
			bool haveProj = (ProjBitmap.getWidth() != 0 && ProjBitmap.getHeight() != 0
				&& ProjBitmap.PixelFormat == CBitmap::RGBA);
			w = haveProj ? ProjBitmap.getWidth() : 0;
			h = haveProj ? ProjBitmap.getHeight() : 0;
			f.serial(w);
			f.serial(h);
			if (w && h)
				f.serialBuffer(&ProjBitmap.getPixels()[0], w * h * 4);
		}
	}
	f.serial(mProj);

	f.serialCont(setExclusion);

	f.serial(rSoftShadowRadius);
	f.serial(rSoftShadowConeLength);
}

// ***************************************************************************
// CLightmapSceneMesh
// ***************************************************************************

void CLightmapSceneMesh::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serial(NodeName);
	f.serial(MeshBuild);
	f.serial(BaseBuild);
}

// ***************************************************************************
// CLightmapReceiverGeom
// ***************************************************************************

CLightmapReceiverGeom::CLightmapReceiverGeom()
{
	DistMax = 1000.f;
	BlendLength = 0.f;
	SlotFlags = 0;
	LumelSizeMul = 1.f;
	LmcEnabled = false;
	for (uint i = 0; i < 3; ++i)
	{
		LmcAmbient[i] = NLMISC::CRGBA::Black;
		LmcDiffuse[i] = NLMISC::CRGBA::White;
	}
	FirstMaterial = 0;
}

// ***************************************************************************
void CLightmapReceiverGeom::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serial(NodeName);
	f.serial(MeshBuild);

	f.serial(DistMax);
	f.serial(BlendLength);
	f.serial(SlotFlags);

	f.serial(LumelSizeMul);
	f.serial(LmcEnabled);
	for (uint i = 0; i < 3; ++i)
	{
		f.serial(LmcAmbient[i]);
		f.serial(LmcDiffuse[i]);
	}

	f.serial(FirstMaterial);

	f.serialCont(ExcludeOccluders);
}

// ***************************************************************************
// CLightmapReceiver
// ***************************************************************************

CLightmapReceiver::CLightmapReceiver()
{
	MultiLod = false;
	StaticLod = true;
	WantMrm = false;
	MrmNLods = 11;
	MrmDivisor = 20;
	MrmSkinReduction = 1; // CMRMParameters::SkinReductionMax
	MrmDistanceFinest = 5.f;
	MrmDistanceMiddle = 30.f;
	MrmDistanceCoarsest = 200.f;
	AnimatedMaterials = false;
	AutoAnim = false;
	DistMax = 1000.f;
	CoarseOutput = false;
}

// ***************************************************************************
void CLightmapReceiver::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serial(NodeName);
	f.serial(BaseBuild);
	f.serialCont(Geoms);

	f.serial(MultiLod);
	f.serial(StaticLod);
	f.serial(WantMrm);
	f.serial(MrmNLods);
	f.serial(MrmDivisor);
	f.serial(MrmSkinReduction);
	f.serial(MrmDistanceFinest);
	f.serial(MrmDistanceMiddle);
	f.serial(MrmDistanceCoarsest);

	f.serialCont(MaterialNames);
	f.serial(AnimatedMaterials);
	f.serial(AutoAnim);
	f.serial(DistMax);

	f.serial(CoarseOutput);
}

// ***************************************************************************
// CLightmapScene
// ***************************************************************************

void CLightmapScene::serial(NLMISC::IStream &f)
{
	// File magic + container version
	f.serialCheck(NELID("SMLN")); // "NLMS" on disk
	(void)f.serialVersion(0);

	f.serial(ProjectName);
	f.serialCont(Lights);
	f.serialCont(Occluders);
	f.serialCont(Receivers);
}

// ***************************************************************************
void CLightmapScene::save(const std::string &path)
{
	COFile f;
	if (!f.open(path))
		throw EFileNotOpened(path);
	serial(f);
	f.close();
}

// ***************************************************************************
void CLightmapScene::load(const std::string &path)
{
	CIFile f;
	if (!f.open(path))
		throw EFileNotOpened(path);
	serial(f);
	f.close();
}

} // NL3D

/* End of lightmap_scene.cpp */
