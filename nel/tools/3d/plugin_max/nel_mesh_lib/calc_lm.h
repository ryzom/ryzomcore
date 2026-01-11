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

#ifndef NL_CALC_LM_H
#define NL_CALC_LM_H

// ***********************************************************************************************

#include "nel/misc/rgba.h"

#include <string>
#include <set>

// ***********************************************************************************************

#define MAXLIGHTMAPSIZE		1024

// ***********************************************************************************************

#define NEL3D_APPDATA_LM			41654684
#define NEL3D_APPDATA_LM_ANIMATED_LIGHT	(NEL3D_APPDATA_LM+1)		// The name of the animation used by light
#define NEL3D_APPDATA_LM_ANIMATED	(NEL3D_APPDATA_LM+2)			// 1 if this light give an animation, 0 else
#define NEL3D_APPDATA_LM_LIGHT_GROUP	(NEL3D_APPDATA_LM+3)		// The light group. 0 : global, 1 : day, 2 : night etc..

#define NEL3D_APPDATA_LM_ANIMATED_LIGHT_DEFAULT	""
#define NEL3D_APPDATA_LM_LIGHT_GROUP_DEFAULT 0

// ***********************************************************************************************
extern void convertToWorldCoordinate	(NL3D::CMesh::CMeshBuild *pMB, 
										NL3D::CMeshBase::CMeshBaseBuild *pMBB, 
										NLMISC::CVector &translation = NLMISC::CVector(0.0f, 0.0f, 0.0f));

extern NLMISC::CMatrix getObjectToWorldMatrix	(NL3D::CMesh::CMeshBuild *pMB, 
												NL3D::CMeshBase::CMeshBaseBuild *pMBB);

// ***********************************************************************************************
// Light representation from max
struct SLightBuild
{
	std::string		Name;
	std::string		AnimatedLight;
	uint			LightGroup;
	enum EType { LightAmbient, LightPoint, LightDir, LightSpot };
	EType			Type;
	NLMISC::CVector Position;				// Used by LightPoint and LightSpot
	NLMISC::CVector Direction;				// Used by LightSpot and LightDir
	float			rRadiusMin, rRadiusMax;	// Used by LightPoint and LightSpot
	float			rHotspot, rFallof;		// Used by LightSpot
	NLMISC::CRGBA	Ambient;
	NLMISC::CRGBA	Diffuse;
	NLMISC::CRGBA	Specular;
	bool			bCastShadow;
	bool			bAmbientOnly;			// true if LightPoint|LightSpot|LightDir with GetAmbientOnly().
	float			rMult;

	NLMISC::CBitmap ProjBitmap;				// For projector (bitmap)
	NLMISC::CMatrix mProj;					// For projector (matrix)

	float			rDirRadius;				// Accel for directionnal lights : Radius of the 
											// cylinder passing trough the bounding sphere of the 
											// object under consideration

	std::set<std::string> setExclusion;		// List of object name excluded by this light

	float			rSoftShadowRadius;		// The radius max used when calculating soft shadows
	float			rSoftShadowConeLength;	// The distance between vertex and cylinder beginning
	
	// -------------------------------------------------------------------------------------------

	SLightBuild ();
	bool canConvertFromMaxLight (INode *node, TimeValue tvTime);
	void convertFromMaxLight (INode *node,TimeValue tvTime);

};

// ***********************************************************************************************
/** retrieve all the light flagged "lightmap" in the scene
 *	/param visibleOnly if false, all lights are retrieved, else only light not hidden are retrieved
 */
void getLightmapLightBuilds( std::vector<SLightBuild> &lights, TimeValue tvTime, Interface& ip, bool visibleOnly= false );


#endif // NL_CALC_LM_H

/* End of calc_lm.h */
