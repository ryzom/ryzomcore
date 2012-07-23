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
#include "r2_config.h"


using namespace NLMISC;

namespace R2
{

CConfigVarRGBA	CV_MapEntityHighlightColor("MapEntityHighlightColor", CRGBA(255, 255, 255, 127));
CConfigVarRGBA	CV_MapEntitySelectColor("MapEntitySelectColor", CRGBA(255, 0, 0, 255));
CConfigVarRGBA	CV_MapEntityFrozenColor("MapEntityFrozenColor", CRGBA(0,  255,  255,  80));
CConfigVarRGBA	CV_MapEntityLockedColor("MapEntityLockedColor", CRGBA(127,  0,  200,  80));
CConfigVarRGBA	CV_ArrayInstanceColor("ArrayInstanceColor", CRGBA(255, 255, 255, 127));
CConfigVarString CV_MapEntitySelectTexture("MapEntitySelectTexture", "r2_icon_select.tga");
CConfigVarString CV_MapEntityDefaultTexture("MapEntityDefaultTexture", "brick_default.tga");
CConfigVarString CV_MapEntityFarTexture("MapEntityFarTexture", "r2_icon_far.tga");
CConfigVarFloat  CV_MapEntityFarArrowSize("MapEntityFarArrowSize", 10.f);
CConfigVarString CV_MapEntitySmallTexture("MapEntitySmallTexture", "r2_icon_map_entity_small.tga");
CConfigVarString CV_MapEntitySmallHighlightTexture("MapEntitySmallHighlightTexture", "r2_icon_map_entity_small_highlight.tga");
CConfigVarString CV_MapEntityOrientTexture("MapEntityOrientTexture", "r2_icon_map_entity_orient.tga");
CConfigVarFloat  CV_MapEntityOrientOriginDist("MapEntityOrientOriginDist", 10.f);
CConfigVarFloat  CV_MapEntityOrientBlendTimeInMs("MapEntityOrientBlendTimeInMs", 300.f);
CConfigVarFloat  CV_MapEntityOrientOriginDistSmall("MapEntityOrientOriginDistSmall", 8.f);
CConfigVarFloat  CV_MapEntityCloseDist("MapEntityCloseDist", 4.0f);

//
CConfigVarString CV_MapGlowStarTexture("MapGlowStarTexture", "r2_glow_star.tga");
CConfigVarFloat  CV_MapGlowStarSize("MapGlowStarSize", 10.f);
CConfigVarFloat	CV_MapGlowStarSpeed[2] =
{
	CConfigVarFloat("MapGlowStarSpeed1", 0.50f),
	CConfigVarFloat("MapGlowStarSpeed2", -0.60f)
};
//
CConfigVarFloat CV_FloatingShapeRefScale("FloatingShapeRefScale", 1.f);
CConfigVarFloat CV_RegionFadeTimeInMs("RegionFadeTimeInMs", 300.f);
//
CConfigVarRGBA CV_FocusedRegionColor("FocusedRegionColor", CRGBA(63,  127,  255,  100));
CConfigVarRGBA CV_SelectedRegionColor("SelectedRegionColor", CRGBA(192,  127,  64,  100));
CConfigVarRGBA CV_UnselectedRegionColor("UnselectedRegionColor", CRGBA(0,  0,  255,  80));
CConfigVarRGBA CV_FrozenRegionColor("FrozenRegionColor", CRGBA(0,  255,  255,  80));
CConfigVarRGBA CV_LockedRegionColor("LockedRegionColor", CRGBA(127,  0,  200,  80));
//
CConfigVarRGBA CV_FocusedInstanceColor("FocusedInstanceColor", CRGBA(200,  32,  64, 127));
CConfigVarRGBA CV_SelectedInstanceColor("SelectedInstanceColor", CRGBA(127,  127,  127));
CConfigVarRGBA CV_UnselectedInstanceColor("UnselectedInstanceColor", CRGBA(0,  0,  0));
CConfigVarRGBA CV_FrozenInstanceColor("FrozenInstanceColor", CRGBA(0,  255,  255,  80));
CConfigVarRGBA CV_LockedInstanceColor("LockedInstanceColor", CRGBA(127,  0,  200,  80));
//
CConfigVarSInt32 CV_MapAutoPanBorder("MapAutoPanBorder", 10);
CConfigVarSInt32 CV_MapAutoPanDeltaInMs("MapAutoPanDeltaInMs", 300);
CConfigVarSInt32 CV_MapAutoFastPanDeltaInMs("MapAutoPanFastDeltaInMs", 150);
CConfigVarSInt32 CV_MapAutoPanSpeedInPixels("MapAutoPanSpeedInPixels", 10);
CConfigVarSInt32 CV_MapAutoFastPanNumTicks("MapAutoFastPanNumTicks", 6);
//
CConfigVarString	CV_FootStepMapTexture("FootStepMapTexture", "r2_map_foot_steps.tga");
CConfigVarString	CV_FootStepDecalTexture("FootStepDecalTexture", "r2_foot_steps.tga");
CConfigVarFloat		CV_FootStepMapWidth("FootStepMapWidth", 3.f);
CConfigVarFloat		CV_FootStepDecalUScale("FootStepDecalUScale", 1.5f);
CConfigVarFloat		CV_FootStepDecalWidth("FootStepDecalWidth", 0.15f);
CConfigVarString	CV_WanderDecalTexture("WanderDecalTexture", "r2_wander.tga");
CConfigVarFloat		CV_WanderDecalSize("WanderDecalSize", 0.55f);
CConfigVarRGBA		CV_FootStepDecalSelectedColor("FootStepSelectedColor", CRGBA(255, 255, 255, 255));
CConfigVarRGBA		CV_FootStepDecalHiddenColor("FootStepHiddenColor", CRGBA(0, 0, 255, 80));
CConfigVarRGBA		CV_FootStepDecalFocusedColor("FootStepFocusedColor", CRGBA(255, 255, 255, 127));
CConfigVarRGBA		CV_FootStepMapSelectedColor("FootStepMapSelectedColor", CRGBA(255, 255, 255, 255));
CConfigVarRGBA		CV_FootStepMapHiddenColor("FootStepMapHiddenColor", CRGBA(0, 0, 255, 80));
CConfigVarRGBA		CV_FootStepMapFocusedColor("FootStepMapFocusedColor", CRGBA(255, 255, 255, 127));
//
CConfigVarFloat		CV_AutoGroupMaxDist("AutoGroupMaxDist", 5.f);
//
CConfigVarRGBA		CV_InaccessiblePosColor0("InaccessiblePosColor0", CRGBA(255, 0, 0, 255));
CConfigVarRGBA		CV_InaccessiblePosColor1("InaccessiblePosColor1", CRGBA(200, 217, 0, 255));
CConfigVarFloat		CV_InaccessiblePosAnimDurationInMS("InaccessiblePosAnimDurationInMS", 500);
//
CConfigVarFloat		CV_DecalTopBlendStartDist("DecalTopBlendStartDist", 3.f);
CConfigVarFloat		CV_DecalBottomBlendStartDist("DecalBottomBlendStartDist", 1.f);
CConfigVarFloat		CV_DecalBlendLength("DecalBlendLength", 1.5f);

CConfigVarString	CV_MapEntityInvalidTexture("MapEntityInvalidTexture", "r2_icon_map_invalid.tga");
CConfigVarString	CV_MapEntityInvalidTextureSmall("MapEntityInvalidTextureSmall", "r2_icon_map_invalid_small.tga");



} // R2
