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



#ifndef CL_CHARACTER_3D_H
#define CL_CHARACTER_3D_H

// ------------------------------------------------------------------------------------------------
#include "game_share/character_summary.h"
#include "game_share/slot_types.h"
#include "nel/3d/u_skeleton.h"
#include "nel/gui/interface_options.h"
#include "interface_options_ryzom.h"
#include "nel/gui/interface_element.h"


// ------------------------------------------------------------------------------------------------
namespace NL3D
{
	class UScene;
	class UInstance;
	class UAnimationSet;
	class USkeleton;
	class UPlayList;
};

// ------------------------------------------------------------------------------------------------
enum TChar3DPart
{
	Char3DPart_Chest = 0,
	Char3DPart_Legs,
	Char3DPart_Arms,
	Char3DPart_Feet,
	Char3DPart_Face,
	Char3DPart_Head,
	Char3DPart_Hands,
	Char3DPart_HandRightItem,
	Char3DPart_HandLeftItem,
	Char3DPart_INVALID
};

#define NB_CHARACTER3D_PARTS 9
#define NB_MORPH_TARGETS 8

// ------------------------------------------------------------------------------------------------
struct SCharacter3DSetup
{
	// A Part of the character body
	// if the name, color or quality changed the instances are rebuilt. It is not for all fxs.
	struct SCharacterPart
	{
		std::string					Name;		// name of the .shape
		std::string					AdvFx;		// advantage fx .ps
		std::vector<std::string>		StatFxNames;	// all the static fx .ps
		std::vector<std::string>		StatFxBones;	// the bones on which to bind the static fx (weird but done like that in CCharacterCL)
		std::vector<NLMISC::CVector>	StatFxOffss;	// the offset on the bone
		sint32						Color;		// a color slot (in the color table of the color slot manager)
		sint32						Quality;	// quality of the item
	};
	// Primary information (if changed instances are rebuilt)
	std::string		Skeleton;					// filename (.skel)
	SCharacterPart	Parts[NB_CHARACTER3D_PARTS];

	sint32			Tattoo;
	sint32			EyesColor;
	sint32			HairColor;

	sint32			AnimPlayed;

	float CharHeight;	// from -1 (smallest) to 1 (tallest)
	float ChestWidth;	// Slim -1  Normal 0  Fat 1
	float ArmsWidth;
	float LegsWidth;
	float BreastSize;

	float MorphTarget[NB_MORPH_TARGETS]; // -100.0 - 100.0 (percentage of the mt)

	// Additionnal information (if changed the instances are not reconstructed)
	EGSPD::CPeople::TPeople	People;
	bool				Male; // false == Female
	bool				LeftHandItemIsShield; // If false this is a weapon if it exists
	bool				HideFace;

	// Constructor (setup a default fyros male)
	SCharacter3DSetup ();

	// Initialize the structure with race_stats form
	void setupDefault (EGSPD::CPeople::TPeople eRace, bool bMale);

	// Initialize the structure from a character summary struct
	void setupFromCharacterSummary (const CCharacterSummary &cs);

	// Initialize the structure from a DataBase Branch (look in cpp to see the structure)
	void setupFromDataBase (const std::string &branchName);

	// Initialize the structure from the SERVER:... DataBase Branch (which is messy)
	void setupFromSERVERDataBase (uint8 entityID = 0);

	// Convert Tool

	// Database Converter Tool
	static void setupDBFromCharacterSummary (const std::string &branchName, const CCharacterSummary &cs);
	static void setupCharacterSummaryFromDB (CCharacterSummary &cs, const std::string &branchName);
	static void setupCharacterSummaryFromSERVERDB (CCharacterSummary &cs, uint8 entityID = 0);

	// Slots enums
	static TChar3DPart convert_VisualSlot_To_Char3DPart (SLOTTYPE::EVisualSlot vs);
	static SLOTTYPE::EVisualSlot convert_Char3DPart_To_VisualSlot (TChar3DPart cp);
	static std::string convert_VisualSlot_To_String (SLOTTYPE::EVisualSlot vs);

	static uint64 getDB (const std::string &name);
	static void setDB (const std::string &name, uint64 val);
private:

	void setupFromCS_ModelCol (SLOTTYPE::EVisualSlot s, sint32 model, sint32 col);

};

// ------------------------------------------------------------------------------------------------
class CCharacter3D : public CReflectableRefPtrTarget
{

public:

	CCharacter3D();
	~CCharacter3D();

	bool init (NL3D::UScene *pScene);
	void setup (const SCharacter3DSetup &newSetup);
	SCharacter3DSetup getCurrentSetup () { return _CurrentSetup; }

	void setPos (float x, float y, float z);			// reference point : Feet
	void setClusterSystem (NL3D::UInstanceGroup *pIG);
	void setRotEuler (float rx, float ry, float rz);

	void getPos (float &x, float &y, float &z)	{ x = _CurPosX; y = _CurPosY; z = _CurPosZ; }
	void getHeadPos (float &x, float &y, float &z);
	void getRotEuler (float &rx, float &ry, float &rz) { rx = _CurRotX; ry = _CurRotY; rz = _CurRotZ; }

	void setAnim (uint animID);

	void animate (double globalTime); // Should be called each frame

	EGSPD::CPeople::TPeople	getPeople() { return _CurrentSetup.People; }
	void					setPeople(EGSPD::CPeople::TPeople	people) { _CurrentSetup.People = people; }

	bool	getSex() { return _CurrentSetup.Male; }
	void	setSex(bool male) { _CurrentSetup.Male = male; }

	// get skeleton
	NL3D::USkeleton getSkeleton() { return _Skeleton; }

	void copyAnimation(bool copyAnim) { _CopyAnim=copyAnim; }

private:

	void resetAnimation (NL3D::UAnimationSet *animSet);
	void animblink (double globalTime);
	NLMISC::CVector getBonePos (const std::string &boneName);

	// Internal stuff
	void setSkeleton (const std::string &filename);
	void createInstance (TChar3DPart i, const SCharacter3DSetup::SCharacterPart &part);
	void bindToSkeleton (TChar3DPart i);
	uint32 peopleToSkin (EGSPD::CPeople::TPeople p) const;

	void	disableFaceMorphAndBlinks();

private:

	NL3D::UScene					*_Scene; // reference to the scene containing the character
	//
	NL3D::UPlayListManager			*_PlayListManager;
	NL3D::UAnimationSet				*_AnimationSet;	// reference to the option AnimationSet
	//
	NL3D::UPlayList					*_PlayList;
	NL3D::UPlayList					*_FacePlayList;

	std::vector<COptionsAnimationSet::CAnim>		_AnimMale;
	std::vector<COptionsAnimationSet::CAnim>		_AnimFemale;

	// Other instances
	NL3D::UInstance					_Instances[NB_CHARACTER3D_PARTS];

	struct SInstanceFx
	{
		NL3D::UInstance					AdvantageFx;
		std::vector<NL3D::UInstance>	StaticFx;
	};
	SInstanceFx						_InstancesFx[NB_CHARACTER3D_PARTS];

	// Skeleton
	NL3D::USkeleton					_Skeleton;

	// Root is the parent of skeleton (needed because skeleton receive animation values)
	NL3D::UTransform				_Root;

	NL3D::UInstanceGroup			*_ClusterSystem;

	float							_CurPosX, _CurPosY, _CurPosZ;
	float							_CurRotX, _CurRotY, _CurRotZ;

	// The animated pelvis pos
	NLMISC::CVector					_PelvisPos;
	float							_CustomScalePos;

	double							_NextBlinkTime;

	// Cache information
	SCharacter3DSetup				_CurrentSetup;

	bool							_CopyAnim;
};

#endif // CL_CHARACTER_3D_H
