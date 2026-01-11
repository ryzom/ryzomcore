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

#ifndef R2_CONFIG_H
#define R2_CONFIG_H


#include "config_var.h"

// access to config vars
namespace R2
{



extern CConfigVarRGBA	CV_MapEntityHighlightColor;
extern CConfigVarRGBA	CV_MapEntitySelectColor;
extern CConfigVarRGBA	CV_MapEntityFrozenColor;
extern CConfigVarRGBA	CV_MapEntityLockedColor;
extern CConfigVarRGBA	CV_ArrayInstanceColor;
extern CConfigVarString CV_MapEntitySelectTexture;
extern CConfigVarString CV_MapEntityDefaultTexture;
extern CConfigVarString CV_MapEntityFarTexture;
extern CConfigVarFloat  CV_MapEntityFarArrowSize;
extern CConfigVarString CV_MapEntitySmallTexture;
extern CConfigVarString CV_MapEntitySmallHighlightTexture;
extern CConfigVarString CV_MapEntityOrientTexture;
extern CConfigVarFloat  CV_MapEntityOrientOriginDist;
extern CConfigVarFloat  CV_MapEntityOrientBlendTimeInMs;
extern CConfigVarFloat  CV_MapEntityOrientOriginDistSmall;
extern CConfigVarFloat  CV_MapEntityCloseDist;
extern CConfigVarString CV_MapEntityInvalidTexture;
extern CConfigVarString CV_MapEntityInvalidTextureSmall;
//
extern CConfigVarString CV_MapGlowStarTexture;
extern CConfigVarFloat  CV_MapGlowStarSize;
extern CConfigVarFloat	CV_MapGlowStarSpeed[2];
extern CConfigVarFloat  CV_FloatingShapeRefScale;
//
extern CConfigVarFloat CV_RegionFadeTimeInMs;
extern CConfigVarRGBA CV_FocusedRegionColor;
extern CConfigVarRGBA CV_SelectedRegionColor;
extern CConfigVarRGBA CV_UnselectedRegionColor;
extern CConfigVarRGBA CV_FrozenRegionColor;
extern CConfigVarRGBA CV_LockedRegionColor;
//
extern CConfigVarRGBA CV_FocusedInstanceColor;
extern CConfigVarRGBA CV_SelectedInstanceColor;
extern CConfigVarRGBA CV_UnselectedInstanceColor;
extern CConfigVarRGBA CV_FrozenInstanceColor;
extern CConfigVarRGBA CV_LockedInstanceColor;
//
extern CConfigVarSInt32 CV_MapAutoPanBorder;
extern CConfigVarSInt32 CV_MapAutoPanDeltaInMs;
extern CConfigVarSInt32 CV_MapAutoFastPanDeltaInMs;
extern CConfigVarSInt32 CV_MapAutoPanSpeedInPixels;
extern CConfigVarSInt32 CV_MapAutoFastPanNumTicks;
//
extern CConfigVarString CV_FootStepMapTexture;
extern CConfigVarFloat	CV_FootStepMapWidth;
extern CConfigVarString CV_FootStepDecalTexture;
extern CConfigVarFloat	CV_FootStepDecalUScale;
extern CConfigVarFloat	CV_FootStepDecalWidth;
extern CConfigVarString	CV_WanderDecalTexture;
extern CConfigVarFloat	CV_WanderDecalSize;
extern CConfigVarRGBA	CV_FootStepMapHiddenColor;
extern CConfigVarRGBA	CV_FootStepMapFocusedColor;
extern CConfigVarRGBA	CV_FootStepMapSelectedColor;
extern CConfigVarRGBA	CV_FootStepDecalHiddenColor;
extern CConfigVarRGBA	CV_FootStepDecalFocusedColor;
extern CConfigVarRGBA	CV_FootStepDecalSelectedColor;
//
extern CConfigVarFloat	CV_AutoGroupMaxDist;
//
extern CConfigVarRGBA	CV_InaccessiblePosColor0;
extern CConfigVarRGBA	CV_InaccessiblePosColor1;
extern CConfigVarFloat  CV_InaccessiblePosAnimDurationInMS;
//
extern CConfigVarFloat  CV_DecalTopBlendStartDist; // distance at which the color starts to fade for player & selection decals
extern CConfigVarFloat  CV_DecalBottomBlendStartDist; // distance at which the color starts to fade for player & selection decals
extern CConfigVarFloat  CV_DecalBlendLength; // length for decal color fading (player & selection decals)



} // R2


#endif
