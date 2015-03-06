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




#ifndef CL_MISC_H
#define CL_MISC_H


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/quat.h"
#include "nel/misc/rgba.h"
#include "nel/3d/u_instance.h"
#include "nel/3d/u_driver.h"
#include "game_share/slot_types.h"
#include "game_share/mode_and_behaviour.h"


////////////
// DEFINE //
////////////
#define _MOVIE_SHOOTER_ON_		//

// They want profile functions in final version
#define _PROFILE_ON_

///////////
// CLASS //
///////////
namespace NL3D
{
	class UInstance;
	class UScene;
}

namespace NLMISC
{
	class CNoiseValue;
	class CVector;
	class CBitmap;
}

struct SPropVisualA;
struct SPropVisualB;
class CPlayerSheet;

enum TLightGroup
{
	LightGroupAlways = 0,
	LightGroupDay,
	LightGroupNight,
	LightGroupFireworks,
};

class CSeeds;

//////////////
// FUNCTION //
//////////////
double keepIn_NegPi_Pi(double angle);

double angleBetween2Vect(const NLMISC::CVectorD &from, const NLMISC::CVectorD &to);

/**
 * \param face : pointer on the face to make up (must not be null).
 * \param idMakeUp : index of the make-up to apply.
 * \warning This function does not check if 'face'  is valid.
 */
void makeUp(NL3D::UInstance face, sint idMakeUp);

/**
 * qStart is the quaterion at t=0,
 * qEnd is the quaterion at t=TAnimEnd,
 * time is between 0 and 1. (1=> TAnimEnd)
 */
NLMISC::CQuat applyRotationFactor(NLMISC::CQuat in, float rotFactor, NLMISC::CQuat qStart, NLMISC::CQuat qEnd, float time);

/** Compute the angle between a source and a destination using the shortest way.
 * \param from : angle between -Pi and Pi;
 * \param to : angle between -Pi and Pi;
 */
double computeShortestAngle(double from, double to);

/** Return the animset base name corresponding to the mode.
 * \param mode : the mode to convert.
 * \param result : this will be filed with the mode animset name.
 * \return bool : 'true' if 'result' is filled, 'false' if left untouched.
 */
bool mode2Anim(MBEHAV::EMode mode, std::string &result);

/** Compute the animation set to use according to weapons, mode and race.
 * \param animSet : result pointer.
 * \param mode : the mode.
 * \param animSetBaseName : basic name to construc the complet name of the animSet.
 * \param leftHand : animSet name for the left hand.
 * \param rightHand : animSet name for the right hand.
 * \param lookAtItemsInHands : compute animset according to items in hands or not.
 * \return bool : 'true' if the new animation set is the right one. 'false' if the one choosen is not the right one.
 */
bool computeAnimSet(const class CAnimationSet *&animSet, MBEHAV::EMode mode, const std::string &animSetBaseName, const class CItemSheet *itemLeftHand, const CItemSheet *itemRightHand, bool lookAtItemsInHands);

/** Create a file with the current state of the client (good to report a bug).
 */
void dump(const std::string &name);

/** Create a file with the current state of the client (good to report a bug).
 */
void loadDump(const std::string &name);

/** Get the lod character id from the scene, according to LodCharacterName. Cached.
 *	-1 if id not found.
 */
sint getLodCharacterId(NL3D::UScene &scene, const std::string &lodCharacterName);

CItemSheet *getItem(const struct CGenderInfo &genderInfo, SLOTTYPE::EVisualSlot slot);
sint getColorIndex(const CGenderInfo &genderInfo, SLOTTYPE::EVisualSlot slot);
SPropVisualA buildPropVisualA(const CGenderInfo &genderInfo);
SPropVisualB buildPropVisualB(const CGenderInfo &genderInfo);

// Test whether user color is supported for a given visual slot
bool isUserColorSupported(const CPlayerSheet &playerSheet, SLOTTYPE::EVisualSlot vs);
SPropVisualA buildPropVisualA(const CPlayerSheet &playerSheet);
SPropVisualB buildPropVisualB(const CPlayerSheet &playerSheet);

// Draw a Box from 2 vectors.
void drawBox(const NLMISC::CVector &vMin, const NLMISC::CVector &vMax, const NLMISC::CRGBA &color);

// Draw a Wired Sphere
void drawSphere(const NLMISC::CVector &center, float radius, const NLMISC::CRGBA &color);

void getSeedsFromDB(CSeeds &dest);


/** Change a 'direction' vector.
 * \param vectToChange : the vector to change.
 * \param vect : new vector to use.
 * \param compute : adjust the param 'vect' to be valid or leave the old one unchanged if impossible.
 * \param check : warning if the param 'vect' is not valid (vector Null) even with compute=true.
 * \return bool : 'true' if the vectToChange has been filled, else 'false'.
 */
bool setVect(NLMISC::CVector &vectToChange, const NLMISC::CVector &vect, bool compute, bool check);


// read color from client cfg system info colors
NLMISC::CRGBA interpClientCfgColor(const ucstring &src, ucstring &dest);
// Get the category from the string (src="&SYS&Who are you?" and dest="Who are you?" and return "SYS"), if no category, return "SYS"
std::string getStringCategory(const ucstring &src, ucstring &dest, bool alwaysAddSysByDefault = true);
// Get the category from the string (src="&SYS&Who are you?" and dest="Who are you?" and return "SYS"), if no category, return ""
std::string getStringCategoryIfAny(const ucstring &src, ucstring &dest);

// Number of shortcut
#define RYZOM_MAX_SHORTCUT 20

// Number of render filters
enum TFilter3d
{
	FilterMeshNoVP=0,
	FilterMeshVP,
	FilterFXs,
	FilterLandscape,
	FilterVegetable,
	FilterSkeleton,
	FilterWater,
	FilterCloud,
	FilterCoarseMesh,
	FilterSky,
	RYZOM_MAX_FILTER_3D,
};

// compare 2 ucstring s0 and s1, without regard to case. give start and size for sequence p0
sint ucstrnicmp(const ucstring &s0, uint p0, uint n0, const ucstring &s1);

/** Compute a non-continuous noise with uniform repartition in [0, 1], with the given noise object
  * By default repartition is not uniform for noise
  */
float computeUniformNoise(const NLMISC::CNoiseValue &nv, const NLMISC::CVector &pos);


// ***************************************************************************
void computeCurrentFovAspectRatio(float &fov, float &ar);

/** draw a disc in a bitmap
  * \TODO : find a place for this in NLMISC
  */
void drawDisc(NLMISC::CBitmap &dest, float x, float y, float radius, const NLMISC::CRGBA &color, bool additif = false, uint numSegs = 127);


// helper : get pointer to the current sky scene (new sky code, or fallback on old sky scene for continent that do not use the new system)
NL3D::UScene *getSkyScene();

// set emissive on all material of an instance (alpha is left unmodified)
void setEmissive(NL3D::UInstance instance, const NLMISC::CRGBA &color);

// set diffuse on all material of an instance (not including alpha)
void setDiffuse(NL3D::UInstance instance, bool onOff, const NLMISC::CRGBA &color);

// make an instance transparent (uses values from the shape material as a reference)
void makeInstanceTransparent(NL3D::UInstance	&inst, uint8 opacity, bool disableZWrite);


// change the video mode, possibly centering the software mouse cursor if going to fullscreen
void setVideoMode(const NL3D::UDriver::CMode &mode);

// Get the current color depth (8, 16, or 32). In windowed mode, this will be the desktop color depth, in fullscreen mode, the color depth of the framebuffer.
uint getCurrentColorDepth();

// get maximized
bool isWindowMaximized();

// get all supported video modes
sint getRyzomModes(std::vector<NL3D::UDriver::CMode> &videoModes, std::vector<std::string> &stringModeList);

#endif // CL_MISC_H

/* End of misc.h */
