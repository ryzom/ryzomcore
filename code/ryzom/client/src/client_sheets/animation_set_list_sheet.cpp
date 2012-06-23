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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"	// First include for pre-compiled headers.
// Application
#include "automaton_list_sheet.h"
// NeL
#include "nel/misc/smart_ptr.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
#include "nel/misc/variable.h"
// STD
#include <string>

///////////
// USING //
///////////

using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;

NLMISC::CVariable<bool>	VerboseAnimParsing("client","VerboseAnimParsing", "log the animation parsing info", false, 0, true);

////////////
// STATIC //
////////////

const char *CAnimationStateSheet::_AnimStateName[StaticStateCount]=
{
	"idle",
	"run",
	"walk",
	"turn_left",
	"turn_right",
	"emote",
	// OLD CAST
	"cast_good_begin",
	"cast_good_success",
	"cast_good_fail",
	"cast_good_fumble",
	"cast_bad_begin",
	"cast_bad_success",
	"cast_bad_fail",
	"cast_bad_fumble",
	"cast_neutral_begin",
	"cast_neutral_success",
	"cast_neutral_fail",
	"cast_neutral_fumble",
	// CAST
	// Offensive Cast
	"offensive_cast_init",
	"offensive_cast_begin",
	"offensive_cast_loop",
	"offensive_cast_fail",
	"offensive_cast_fumble",
	"offensive_cast_success",
	"offensive_cast_link",
	// Curative Cast
	"curative_cast_init",
	"curative_cast_begin",
	"curative_cast_loop",
	"curative_cast_fail",
	"curative_cast_fumble",
	"curative_cast_success",
	"curative_cast_link",
	// Mixed Cast
	"mixed_cast_init",
	"mixed_cast_begin",
	"mixed_cast_loop",
	"mixed_cast_fail",
	"mixed_cast_fumble",
	"mixed_cast_success",
	"mixed_cast_link",
	// Cast Init
	"cast_acid_init",
	"cast_blind_init",
	"cast_cold_init",
	"cast_elec_init",
	"cast_fear_init",
	"cast_fire_init",
	"cast_healhp_init",
	"cast_mad_init",
	"cast_poison_init",
	"cast_root_init",
	"cast_rot_init",
	"cast_shock_init",
	"cast_sleep_init",
	"cast_slow_init",
	"cast_stun_init",
	// Cast Loop
	"cast_acid_loop",
	"cast_blind_loop",
	"cast_cold_loop",
	"cast_elec_loop",
	"cast_fear_loop",
	"cast_fire_loop",
	"cast_healhp_loop",
	"cast_mad_loop",
	"cast_poison_loop",
	"cast_root_loop",
	"cast_rot_loop",
	"cast_shock_loop",
	"cast_sleep_loop",
	"cast_slow_loop",
	"cast_stun_loop",
	// Cast Fail
	"cast_acid_fail",
	"cast_blind_fail",
	"cast_cold_fail",
	"cast_elec_fail",
	"cast_fear_fail",
	"cast_fire_fail",
	"cast_healhp_fail",
	"cast_mad_fail",
	"cast_poison_fail",
	"cast_root_fail",
	"cast_rot_fail",
	"cast_shock_fail",
	"cast_sleep_fail",
	"cast_slow_fail",
	"cast_stun_fail",
	// Cast End
	"cast_acid_end",
	"cast_blind_end",
	"cast_cold_end",
	"cast_elec_end",
	"cast_fear_end",
	"cast_fire_end",
	"cast_healhp_end",
	"cast_mad_end",
	"cast_poison_end",
	"cast_root_end",
	"cast_rot_end",
	"cast_shock_end",
	"cast_sleep_end",
	"cast_slow_end",
	"cast_stun_end",
	// Attack
	"default atk low",
	"default atk middle",
	"default atk high",
	"powerful atk low",
	"powerful atk middle",
	"powerful atk high",
	"area atk low",
	"area atk middle",
	"area atk high",
	"attack1",
	"attack2",
	"1st_person atk",
	// OTHER
	"impact",
	"death",
	"death_idle",
	"loot_init",
	"loot_end",
	"prospecting_init",
	"prospecting_end",
	"care_init",
	"care_end",
	"use_init",
	"use_begin",
	"use_loop",
	"use_end",
	"stun_begin",
	"stun_loop",
	"stun_end",
	"sit_mode",
	"sit_end",
	"strafe_left",
	"strafe_right",
};

map<string, TAnimStateId>	CAnimationStateSheet::_StringToAnimStateId;
vector<string>				CAnimationStateSheet::_AnimStateIdToString;
CStaticStringMapper			CAnimationStateSheet::LodCharAnim;

NLMISC::CStaticStringMapper	CAnimationSheet::AnimNames;
NLMISC::CStaticStringMapper	CAnimationSheet::FxNames;


/////////////
// METHODS //
/////////////



// ***************************************************************************
// CAnimationSheet
// ***************************************************************************

//-----------------------------------------------
// Constructor
//-----------------------------------------------
CAnimationSheet::CAnimationSheet()
{
	IdAnim = AnimNames.emptyId();
	ApplyCharacterScalePosFactor= true;

	HeadControlable = false;
	Reverse		= false;
	HideAtEndAnim = false;
	VirtualRot		= 0.0;

	IdFX = FxNames.emptyId();

	// no restriction by default
	JobRestriction= 0;
	RaceRestriction= EGSPD::CPeople::Unknown;
}// CAnimationSheet //

//-----------------------------------------------
// build
//-----------------------------------------------
void CAnimationSheet::build(const NLGEORGES::UFormElm &item)
{
	// Get the animation filename.
	string animName;
	if(item.getValueByName(animName, "filename"))
	{
		// Info to log the animation name if there is a pb.
		if (VerboseAnimParsing)
			nlinfo("    Insert animation '%s'.", animName.c_str());
		IdAnim = AnimNames.add(animName);
	}
	else
		nlwarning("key 'filename' not found.");
	// get ApplyCharacterScalePosFactor
	bool bTmp;
	if(!item.getValueByName(bTmp, "apply_char_scale_pos"))
		nlwarning("CAnimation:build: key 'apply_char_scale_pos' not found.");
	ApplyCharacterScalePosFactor = bTmp;
	// get the fx associated.
	string fx;
	if(!item.getValueByName(fx, "fx"))
		nlwarning("CAnimation:build: key 'fx' not found.");
	IdFX = FxNames.add(fx);
	// Does the animation take control of the head ?
	if(!item.getValueByName(bTmp, "head controlable"))
		nlwarning("CAnimation:build: key 'head controlable' not found.");
	HeadControlable = bTmp;
	// Will be the animation played from the start or the end.
	if(!item.getValueByName(bTmp, "reverse"))
		nlwarning("CAnimation:build: key 'reverse' not found.");
	Reverse = bTmp;
	// In case of an animation that is a rotation but with no rotation (like characters on a mount).
	if(!item.getValueByName(VirtualRot, "virtual rotation"))
		nlwarning("CAnimation:build: key 'virtual rotation' not found.");
	// Do we need to hide the entity at the end of the animation ? (useful for Kami).
	if(!item.getValueByName(bTmp, "hide_at_end_anim"))
		nlwarning("CAnimation:build: key 'hide_at_end_anim' not found.");
	HideAtEndAnim = bTmp;

	// build standard fx
	FXSet.buildWithPrefix(item, "fx_set.");

	// Next Animations
	for(uint i=0;i<7;i++)
	{
		sint8 next = -1;
		uint16 nextWeight = 1;
		if(!item.getValueByName(next, toString("next%d",i+1).c_str()))
			nlwarning("CAnimation:build: key 'next%d' not found.", i+1);
		if(next >= 0)
		{
			Next.push_back(next);

			if(!item.getValueByName(nextWeight, toString("next%d weight", i+1).c_str()))
			{
				nlwarning("CAnimation:build: key 'next%d weight' not found.", i+1);
			}
			NextWeight.push_back(nextWeight);
		}
	}

	// Restrictions
	uint32		uTmp= 0;
	if(!item.getValueByName(uTmp, "Job Restriction"))
		nlwarning("CAnimation:build: key 'Job Restriction' not found.");
	JobRestriction= uTmp;
	string		sTmp;
	if(!item.getValueByName(sTmp, "Race Restriction"))
		nlwarning("CAnimation:build: key 'Race Restriction' not found.");
	RaceRestriction= EGSPD::CPeople::fromString(sTmp);

}// build //


//-----------------------------------------------
// serial
//-----------------------------------------------
void CAnimationSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// serialize the animation name to be able to compute the animation when loading.
	AnimNames.serial(f, IdAnim);

	bool bTmp;
	bTmp = ApplyCharacterScalePosFactor;
	f.serial(bTmp);
	ApplyCharacterScalePosFactor = bTmp;

	FxNames.serial(f, IdFX);

	bTmp = HeadControlable;
	f.serial(bTmp);
	HeadControlable = bTmp;

	f.serial(VirtualRot);

	// serial standard fx
	f.serial(FXSet);

	bTmp = Reverse;
	f.serial(bTmp);
	Reverse = bTmp;

	bTmp = HideAtEndAnim;
	f.serial(bTmp);
	HideAtEndAnim = bTmp;

	f.serialCont(Next);
	f.serialCont(NextWeight);

	f.serial(JobRestriction);
	f.serialEnum(RaceRestriction);
}// serial //

// ***************************************************************************
// CAnimationStateSheet
// ***************************************************************************

//-----------------------------------------------
// getAnimationStateId :
// Return an anim id from its name.
//-----------------------------------------------
TAnimStateId CAnimationStateSheet::getAnimationStateId (const std::string &stateName)
{
	if (stateName.empty ())
		return UnknownState;

	std::map<std::string, TAnimStateId>::const_iterator ite = _StringToAnimStateId.find (stateName);
	if (ite == _StringToAnimStateId.end())
	{
		uint i;
		for (i=0; i<StaticStateCount; i++)
		{
			if (stateName == _AnimStateName[i])
				break;
		}
		if (i>=StaticStateCount)
			i = std::max ((uint)_AnimStateIdToString.size (), (uint)StaticStateCount);

		if (i >= _AnimStateIdToString.size ())
			_AnimStateIdToString.resize (i+1);

		_AnimStateIdToString[i] = stateName;
		_StringToAnimStateId.insert (make_pair(stateName, (TAnimStateId)i));
		return (TAnimStateId)i;
	}
	return ite->second;
}// getAnimationStateId //

//-----------------------------------------------
// getAnimationStateName :
// Return a name from an anim id.
//-----------------------------------------------
const string &CAnimationStateSheet::getAnimationStateName (TAnimStateId id)
{
	static std::string unknownString = "Unknown";
	if (id == UnknownState)
		return unknownString;
	else
	{
		if((uint32)id >= _AnimStateIdToString.size())
		{
			nlwarning("ASS:getAnimationStateName: id(%d) out of range (size=%u)", _AnimStateIdToString.size());
			return unknownString;
		}
		return _AnimStateIdToString[id];
	}
}// getAnimationStateName //


//-----------------------------------------------
// build
//-----------------------------------------------
void CAnimationStateSheet::build(const NLGEORGES::UFormElm &item)
{
	// Erase the animation vector
	Animations.clear ();

	// Check parameter
	if (State == CAnimationStateSheet::UnknownState)
		return;

	// Get animations.
	const UFormElm *elmt = 0;
	if(!item.getNodeByName(&elmt, "animations"))
		nlwarning("CAnimationState:build: the node 'animations' is not referenced.");
	// If the array is not empty (in fact exist).
	if(elmt)
	{
		// Get the array size
		uint arraySize;
		elmt->getArraySize(arraySize);
		// If there is at least 1 animation.
		if(arraySize > 0)
		{
			// Reserve the vector
			Animations.reserve (arraySize);
			// Get all animation for the State.
			for(uint i = 0; i<arraySize; ++i)
			{
				const UFormElm *animElmt;
				elmt->getArrayNode(&animElmt, i);
				if(animElmt)
				{
					// Create and initialize the Animation.
					CAnimationSheet anim;
					anim.build(*animElmt);

					// Check animation ID (warnings are in animation.cpp).
					if(anim.IdAnim != CAnimationSheet::AnimNames.emptyId())
					{
						// Insert the animation.
						Animations.push_back (anim);
					}
				}
				else
					nlwarning("CAnimationState::build : element (%d) should be here in 'animations'.");
			}
		}
	}
	// Get LodCharacterAnimation
	string sTmp;
	if(!item.getValueByName(sTmp, "LodCharacterAnimation"))
		nlwarning("CAnimationState:build: the node 'LodCharacterAnimation' is not referenced.");
	IdLodCharacterAnimation = LodCharAnim.add(sTmp);

	// Is Object Visible ?
	bool bTmp;
	if(!item.getValueByName(bTmp, "Display Objects"))
		nlwarning("CAnimationState:build: the node 'Display Objects' is not referenced.");
	DisplayObjects = bTmp;

	// Get MeleeImpactDelay (relevant only for melee attack states)
	if(!item.getValueByName(MeleeImpactDelay, "MeleeImpactDelay"))
		nlwarning("CAnimationState:build: the node 'MeleeImpactDelay' is not referenced.");

}// build //


//-----------------------------------------------
// serial
//-----------------------------------------------
void CAnimationStateSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Vector of animation.
	f.serialCont(Animations);
	// Animation State Id;
	f.serial(State);
	string sStateName;
	if (!f.isReading()) sStateName = getAnimationStateName((TAnimStateSheetId)State);
	f.serial(sStateName);
	// The name of the animation for Lod Character.
	LodCharAnim.serial(f, IdLodCharacterAnimation);
	// Are the objects in hand visible ?
	f.serial(DisplayObjects);
	// MeleeImpactDelay
	f.serial(MeleeImpactDelay);

	// update statics maps
	if (f.isReading())
	{
		_StringToAnimStateId.insert(pair<string, TAnimStateSheetId>(sStateName, (TAnimStateSheetId)State));
		if (_AnimStateIdToString.size() <= State)
			_AnimStateIdToString.resize(State+1);
		_AnimStateIdToString[State] = sStateName;
	}

}// serial //


// ***************************************************************************
// CAnimationSetSheet
// ***************************************************************************

//-----------------------------------------------
// build :
// build the list from a georges file.
//-----------------------------------------------
void CAnimationSetSheet::build(const NLGEORGES::UFormElm &rootElmt)
{
	// Clear the animation state array
	AnimationStates.clear ();

	// Bool to know if there is a need to compute speedToRun or Walk?
	IsWalkEssential = true;
	IsRunEssential = true;

	// Get all animation states.
	uint structSize;
	rootElmt.getStructSize(structSize);

	// Reserve some space
	AnimationStates.reserve (structSize);

	for(uint i = 0; i < structSize; ++i)
	{
		// Get the state name.
		string stateName;
		if(rootElmt.getStructNodeName(i, stateName))
		{
			// "defaultHeadControl" is not a state.
			if(stateName == "defaultHeadControl")
				continue;

			// Push a debug information.
			if (VerboseAnimParsing)
					nlinfo("%2d state '%s' :", i, stateName.c_str());

			const UFormElm *elmt = 0;
			if(rootElmt.getNodeByName(&elmt, stateName.c_str()))
			{
				bool animPresent = false;
				if(elmt)
				{
					// Get the state id
					TAnimStateId stateId = CAnimationStateSheet::getAnimationStateId (stateName);

					if (stateId != CAnimationStateSheet::UnknownState)
					{
						// Resize the array
						if ((uint)stateId >= AnimationStates.size ())
							AnimationStates.resize (stateId+1);
						// Already exist ?
						if (AnimationStates[stateId].State == CAnimationStateSheet::UnknownState)
						{
							AnimationStates[stateId].State = stateId;
							AnimationStates[stateId].build(*elmt);
							animPresent = !AnimationStates[stateId].Animations.empty();
						}
						else
							nlwarning("CAnimationSet::build : Insertion failed state '%s' already exit.", stateName.c_str());
					}
					else
						nlwarning("CAnimationSet::build : Bad Animation State ID. -> state '%s' not inserted.", stateName.c_str());
				}

				// Is the animation needed ?
				bool essential = true;
				const string essentialKey = stateName + string(".Essential");
				if(!rootElmt.getValueByName(essential, essentialKey.c_str()))
					nlwarning("CAnimationSet:build: the node '%s' is not referenced.", string(stateName + string(".Essential")).c_str());
				if(essential)
				{
					if(!animPresent)
						if (VerboseAnimParsing)
							nlinfo("This STATE NEED an ANIMATION.");
				}
				else
				{
					if(essentialKey == "walk.Essential")
						IsWalkEssential = false;
					else if(essentialKey=="run.Essential")
						IsRunEssential = false;
				}
			}
			else
				nlwarning("Cannot get the state '%s' (element '%d').", stateName.c_str(), i);
		}
		// Push a warning to debug.
		else
			nlwarning("Cannot get the name of the element '%d'.", i);
	}
}// build //

//-----------------------------------------------
// serial :
// serialize to disk or from disk the content of this class
//-----------------------------------------------
void CAnimationSetSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Name);
	f.serialCont(AnimationStates);
	f.serial(IsWalkEssential);
	f.serial(IsRunEssential);
}// serial //

// ***************************************************************************
// CAnimationSetListSheet
// ***************************************************************************

//-----------------------------------------------
// CAnimationSetListSheet
//-----------------------------------------------
CAnimationSetListSheet::CAnimationSetListSheet()
{
	Type = CEntitySheet::ANIMATION_SET_LIST;
}// CAnimationSetListSheet //

//-----------------------------------------------
// build :
// build the list from a georges file.
//-----------------------------------------------
void CAnimationSetListSheet::build(const NLGEORGES::UFormElm &item)
{
	vector<string> modes;
	CAutomatonListSheet::readDfnStringArray(modes, "animset_mode.dfn");

	vector<string> types;
	CAutomatonListSheet::readDfnStringArray(types, "animset_type.dfn");

	NLGEORGES::UFormLoader *formLoader = UFormLoader::createLoader();

	// Get the root.
	const UFormElm *pAnimsetList = 0;
	if(item.getNodeByName(&pAnimsetList, "list"))
	{
		if(pAnimsetList)
		{
			NLMISC::CSmartPtr<NLGEORGES::UForm> smartPtr;
			// Read the array with all kind of entities.
			uint size, nTotalSize = 0;
			pAnimsetList->getArraySize(size);
			for(uint i=0; i<size; ++i)
			{
				// Get the entity name
				string entityName;
				if(!pAnimsetList->getArrayNodeName(entityName, i))
				{
					nlwarning("Cannot get the entity name.");
					continue;
				}
				// Get the filename
				string filename;
				if(!pAnimsetList->getArrayValue(filename, i))
				{
					nlwarning("Cannot get the filename.");
					continue;
				}
				// Check if the filename is not empty.
				if(filename.empty())
				{
					nlwarning("filename empty.");
					continue;
				}
				// Check if the filename is valid.
				CSmartPtr<UForm> fileForm = formLoader->loadForm(filename.c_str());
				if(fileForm == 0)
				{
					nlwarning("Invalid Form : %s", filename.c_str());
					continue;
				}
				// Compute all animset for the entity.
				bool useful;
				uint32 nSize = 0;
				for(uint j=0; j<modes.size(); ++j)
				for(uint k=0; k<types.size(); ++k)
					if(fileForm->getRootNode().getValueByName(useful, string(modes[j]+ "." + types[k]).c_str()))
					{
						if(useful)
							nSize++;
					}

				AnimSetList.resize(nTotalSize+nSize);
				nSize = 0;

				for(uint j=0; j<modes.size(); ++j)
				for(uint k=0; k<types.size(); ++k)
					if(fileForm->getRootNode().getValueByName(useful, string(modes[j]+ "." + types[k]).c_str()))
					{
						if(useful)
						{
							// Compute the animset name
							string animset = toString("%s_%s%s", entityName.c_str(), modes[j].c_str(), types[k].c_str());
							// Build
							CAnimationSetSheet animationSet;
							AnimSetList[nTotalSize+nSize].Name = animset + ".animation_set";
							CSmartPtr<UForm> fileFormAS = formLoader->loadForm(AnimSetList[nTotalSize+nSize].Name.c_str());
							if (fileFormAS != NULL)
							{
								AnimSetList[nTotalSize+nSize].build(fileFormAS->getRootNode());
								nSize++;
							}
							else
								nlwarning("Invalid Form : %s.", AnimSetList[nTotalSize+nSize].Name.c_str());
						}
					}

				nTotalSize += nSize;
			}
		}
	}
	else
		nlwarning("Cannot get the Node from the Key 'list'.");

	UFormLoader::releaseLoader(formLoader);
}// build //

//-----------------------------------------------
// serial :
// serialize to disk or from disk the content of this class
//-----------------------------------------------
void CAnimationSetListSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont(AnimSetList);
}// serial //
