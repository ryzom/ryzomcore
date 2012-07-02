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
#include "nel/georges/u_form_dfn.h"
#include "nel/georges/u_form_loader.h"

///////////
// USING //
///////////

using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;

/////////////
// STATICS //
/////////////

std::vector<std::string> CAutomatonSheet::_MotionStates;
std::vector<std::string> CAutomatonSheet::_GenericStates;
std::vector<std::string> CAutomatonSheet::_ModeStates;
std::vector<std::string> CAutomatonSheet::_AtkStates;
std::vector<std::string> CAutomatonSheet::_SpellStates;
std::vector<std::string> CAutomatonSheet::_OtherStates;

/////////////
// METHODS //
/////////////

// ***************************************************************************
// CAutomatonStateSheet
// ***************************************************************************

//-----------------------------------------------
// CAutomatonStateSheet :
// Constructor.
//-----------------------------------------------
CAutomatonStateSheet::CAutomatonStateSheet()
{
	MoveState				= CAnimationStateSheet::UnknownState;

	NextState				= CAnimationStateSheet::UnknownState;
	Move					= false;
	Rotation				= false;
	Attack					= false;
	Slide					= false;
	AdjustOri				= false;
	MaxAnimDuration			= -1.0f;
	MaxLoop					= 0;
	BreakableOnMove			= false;
	OnMoveForward			= CAnimationStateSheet::UnknownState;
	OnMoveBackward			= CAnimationStateSheet::UnknownState;
	OnMoveLeft				= CAnimationStateSheet::UnknownState;
	OnMoveRight				= CAnimationStateSheet::UnknownState;
	BreakableOnRotation		= false;
	OnLeftRotation			= CAnimationStateSheet::UnknownState;
	OnRightRotation			= CAnimationStateSheet::UnknownState;
	BrkOnBigBend			= false;
	OnBigBendLeft			= CAnimationStateSheet::UnknownState;
	OnBigBendRight			= CAnimationStateSheet::UnknownState;
	OnMinSpeed.Breakable	= false;
	OnMinSpeed.NextStateKey	= CAnimationStateSheet::UnknownState;
	OnMaxSpeed.Breakable	= false;
	OnMaxSpeed.NextStateKey	= CAnimationStateSheet::UnknownState;
	BreakableOnBadHeading	= false;
	OnBadHeadingForward		= CAnimationStateSheet::UnknownState;
	OnBadHeadingBackward	= CAnimationStateSheet::UnknownState;
	OnBadHeadingLeft		= CAnimationStateSheet::UnknownState;
	OnBadHeadingRight		= CAnimationStateSheet::UnknownState;
	BadHeadingMin			= 0.0;
	BadHeadingMax			= 0.0;

	BreakableOnMoveDist		= -1;
	BreakableOnImpact		= false;
	BrkAtDest				= false;

	DirFactor				= 0.0;
	RotFactor				= 1.0;

	ModeConnection.resize(MBEHAV::NUMBER_OF_MODES, false);
	ModeTransition.resize(MBEHAV::NUMBER_OF_MODES, CAnimationStateSheet::UnknownState);

	NextMode				= MBEHAV::UNKNOWN_MODE;

	OnAtk					= CAnimationStateSheet::UnknownState;

	XFactor					= 0.0f;
	YFactor					= 0.0f;
	ZFactor					= 0.0f;
}// CAutomatonStateSheet //

//-----------------------------------------------
// build :
// Build the sheet from an external script.
//-----------------------------------------------
void CAutomatonStateSheet::build(const NLGEORGES::UFormElm &item, const string &baseKeyStr)
{
	string stateName;

	// PROPERTIES
	// Get state parameter 'move'.
	item.getValueByName(Move,					string(baseKeyStr + ".move").c_str());
	// Get state parameter 'rotation'.
	item.getValueByName(Rotation,				string(baseKeyStr + ".rotation").c_str());
	// Get state parameter 'attack'.
	item.getValueByName(Attack,					string(baseKeyStr + ".attack").c_str());
	// Get state parameter 'slide'.
	item.getValueByName(Slide,					string(baseKeyStr + ".slide").c_str());
	// Get state parameter 'adjust orientation'.
	item.getValueByName(AdjustOri,				string(baseKeyStr + ".adjust orientation").c_str());
	// Max Animation Duration if there is a destination.
	item.getValueByName(MaxAnimDuration,		string(baseKeyStr + ".max duration").c_str());
	// Max Loop of the same kind of animation.
	item.getValueByName(MaxLoop,				string(baseKeyStr + ".max loop").c_str());

	// DEFAULT NEXT
	// Get state parameter 'default next'.
	item.getValueByName(stateName,				string(baseKeyStr + ".default next").c_str());
	NextState = CAnimationStateSheet::getAnimationStateId(stateName);
	// Get the Next Mode
	item.getValueByName(stateName,				string(baseKeyStr + ".next automaton").c_str());
	NextMode = MBEHAV::stringToMode(stateName);

	// ON MOVE
	// Get state parameter 'on move foreward'.
	item.getValueByName(stateName,				string(baseKeyStr + ".on move forward").c_str());
	OnMoveForward = CAnimationStateSheet::getAnimationStateId(stateName);
	// Get state parameter 'on move backward'.
	item.getValueByName(stateName,				string(baseKeyStr + ".on move backward").c_str());
	OnMoveBackward = CAnimationStateSheet::getAnimationStateId(stateName);
	// Get state parameter 'on move left'.
	item.getValueByName(stateName,				string(baseKeyStr + ".on move left").c_str());
	OnMoveLeft = CAnimationStateSheet::getAnimationStateId(stateName);
	// Get state parameter 'on move right'.
	item.getValueByName(stateName,				string(baseKeyStr + ".on move right").c_str());
	OnMoveRight = CAnimationStateSheet::getAnimationStateId(stateName);
	// ON ROTATION
	// State on rotation to the left
	item.getValueByName(stateName,				string(baseKeyStr + ".on rotation left").c_str());
	OnLeftRotation  = CAnimationStateSheet::getAnimationStateId(stateName);
	// State on rotation to the right
	item.getValueByName(stateName,				string(baseKeyStr + ".on rotation right").c_str());
	OnRightRotation = CAnimationStateSheet::getAnimationStateId(stateName);
	// ON BIG BEND
	item.getValueByName(stateName,				string(baseKeyStr + ".on big bend left").c_str());
	OnBigBendLeft  = CAnimationStateSheet::getAnimationStateId(stateName);
	item.getValueByName(stateName,				string(baseKeyStr + ".on big bend right").c_str());
	OnBigBendRight = CAnimationStateSheet::getAnimationStateId(stateName);
	// ON SPEED
	// Get state parameter 'on min speed'.
	item.getValueByName(stateName,				string(baseKeyStr + ".on min speed").c_str());
	OnMinSpeed.NextStateKey = CAnimationStateSheet::getAnimationStateId(stateName);
	// Get state parameter 'on max speed'.
	item.getValueByName(stateName,				string(baseKeyStr + ".on max speed").c_str());
	OnMaxSpeed.NextStateKey = CAnimationStateSheet::getAnimationStateId(stateName);
	// ON BAD HEADING
	item.getValueByName(stateName,				string(baseKeyStr + ".on bad heading forward").c_str());
	OnBadHeadingForward  = CAnimationStateSheet::getAnimationStateId(stateName);
	item.getValueByName(stateName,				string(baseKeyStr + ".on bad heading backward").c_str());
	OnBadHeadingBackward = CAnimationStateSheet::getAnimationStateId(stateName);
	item.getValueByName(stateName,				string(baseKeyStr + ".on bad heading left").c_str());
	OnBadHeadingLeft     = CAnimationStateSheet::getAnimationStateId(stateName);
	item.getValueByName(stateName,				string(baseKeyStr + ".on bad heading right").c_str());
	OnBadHeadingRight    = CAnimationStateSheet::getAnimationStateId(stateName);
	item.getValueByName(BadHeadingMin,			string(baseKeyStr + ".bad heading min").c_str());
	item.getValueByName(BadHeadingMax,			string(baseKeyStr + ".bad heading max").c_str());

	// ON IMPACT
	item.getValueByName (BreakableOnImpact,		string(baseKeyStr + ".breakable on impact").c_str());
	// AT DESTINATION
	item.getValueByName (BrkAtDest,				string(baseKeyStr + ".brk at destination").c_str());
	// X,Y,Z FACTOR
	item.getValueByName (XFactor,				string(baseKeyStr + ".x factor").c_str());
	item.getValueByName (YFactor,				string(baseKeyStr + ".y factor").c_str());
	item.getValueByName (ZFactor,				string(baseKeyStr + ".z factor").c_str());

	// MODES
	for(uint mode = 0; mode<MBEHAV::NUMBER_OF_MODES; ++mode)
	{
		string animset;
		animset = NLMISC::strlwr(MBEHAV::modeToString((MBEHAV::EMode)mode));
		if(animset != "unknown_mode")
		{
			string resultTransition;
			item.getValueByName(resultTransition, (baseKeyStr + "." + animset + " mode transition").c_str());
			if(resultTransition.empty()==false)
			{
				ModeConnection[mode] = true;
				ModeTransition[mode] = CAnimationStateSheet::getAnimationStateId(resultTransition);
			}
		}
	}

	item.getValueByName (DirFactor,				string(baseKeyStr + ".dir factor").c_str());
	item.getValueByName (RotFactor,				string(baseKeyStr + ".rot factor").c_str());

	// State on atk.
	item.getValueByName (stateName,				string(baseKeyStr + ".on atk").c_str());
	OnAtk = CAnimationStateSheet::getAnimationStateId(stateName);

	// Is the state breakable on Move.
	BreakableOnMove = ((OnMoveForward  != CAnimationStateSheet::UnknownState)
					|| (OnMoveBackward != CAnimationStateSheet::UnknownState)
					|| (OnMoveLeft     != CAnimationStateSheet::UnknownState)
					|| (OnMoveRight    != CAnimationStateSheet::UnknownState));
	// Is the state breakable on Rotation.
	BreakableOnRotation = ((OnLeftRotation != CAnimationStateSheet::UnknownState) || (OnRightRotation != CAnimationStateSheet::UnknownState));
	// Is the state breakable on BigBend.
	BrkOnBigBend = ((OnBigBendLeft != CAnimationStateSheet::UnknownState) || (OnBigBendRight != CAnimationStateSheet::UnknownState));
	// Min/Max Speed
	OnMinSpeed.Breakable = (OnMinSpeed.NextStateKey != CAnimationStateSheet::UnknownState);
	OnMaxSpeed.Breakable = (OnMaxSpeed.NextStateKey != CAnimationStateSheet::UnknownState);
	// Is the state breakable on Bad Heading.
	BreakableOnBadHeading = ((OnBadHeadingForward  != CAnimationStateSheet::UnknownState)
	                      || (OnBadHeadingBackward != CAnimationStateSheet::UnknownState)
	                      || (OnBadHeadingLeft     != CAnimationStateSheet::UnknownState)
	                      || (OnBadHeadingRight    != CAnimationStateSheet::UnknownState));
	// Compute the matrix for the state.
	Matrix.rotateZ((float)(DirFactor*Pi/180.0));

}// build //

//-----------------------------------------------
// serial :
// Serialize a CAutomatonStateSheet.
//-----------------------------------------------
void CAutomatonStateSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialEnum(MoveState);
	f.serialEnum(NextState);

	f.serial    (Move);
	f.serial    (Rotation);
	f.serial    (Attack);
	f.serial    (Slide);
	f.serial    (AdjustOri);
	f.serial    (MaxAnimDuration);

	f.serial    (MaxLoop);

	f.serial    (BreakableOnMove);
	f.serial    (BreakableOnMoveDist);
	f.serialEnum(OnMoveForward);
	f.serialEnum(OnMoveBackward);
	f.serialEnum(OnMoveLeft);
	f.serialEnum(OnMoveRight);

	f.serial    (BreakableOnRotation);
	f.serialEnum(OnLeftRotation);
	f.serialEnum(OnRightRotation);

	f.serial    (BrkOnBigBend);
	f.serialEnum(OnBigBendLeft);
	f.serialEnum(OnBigBendRight);

	f.serial    (OnMinSpeed);
	f.serial    (OnMaxSpeed);

	f.serial    (BreakableOnBadHeading);
	f.serialEnum(OnBadHeadingForward);
	f.serialEnum(OnBadHeadingBackward);
	f.serialEnum(OnBadHeadingLeft);
	f.serialEnum(OnBadHeadingRight);
	f.serial    (BadHeadingMin);
	f.serial    (BadHeadingMax);

	f.serialCont(ModeConnection);
	// Mode Transition
	uint32 size;
	if (f.isReading ())
	{
		f.serial (size);
		ModeTransition.resize (size);
	}
	else
	{
		size = (uint32)ModeTransition.size ();
		f.serial (size);
	}
	for (uint i = 0; i < size; i++)
		f.serialEnum(ModeTransition[i]);

	f.serialEnum(NextMode);
	f.serial    (DirFactor);
	f.serial    (RotFactor);

	f.serialEnum(OnAtk);

	f.serial    (BreakableOnImpact);
	f.serial    (BrkAtDest);

	f.serial    (XFactor);
	f.serial    (YFactor);
	f.serial    (ZFactor);

	f.serial    (Matrix);

}// serial //

//-----------------------------------------------
// getModeConnection:
// Return if there is a connection with the mode in parameter from the current automaton and filled the transition to use.
//-----------------------------------------------
bool CAutomatonStateSheet::getModeConnection(MBEHAV::EMode mode, TAnimStateKey &transition) const
{
	if(mode<0 || (uint)mode >ModeConnection.size())
		return false;

	// If there is a transition -> fill the result.
	if(ModeConnection[mode])
		transition = ModeTransition[mode];
	return ModeConnection[mode];

}// getModeConnection //

// ***************************************************************************
// CAutomatonSheet
// ***************************************************************************

//-----------------------------------------------
//-----------------------------------------------
void CAutomatonSheet::build(const NLGEORGES::UFormElm &item)
{
	// READ DFNs
	// Read "automaton_motion_states.dfn"
	if(_MotionStates.empty())
		CAutomatonListSheet::readDfnStringArray(_MotionStates,  "automaton_motion_states.dfn");
	// Read "automaton_generic_states.dfn"
	if(_GenericStates.empty())
		CAutomatonListSheet::readDfnStringArray(_GenericStates, "automaton_generic_states.dfn");
	// Read "automaton_mode_states.dfn"
	if(_ModeStates.empty())
		CAutomatonListSheet::readDfnStringArray(_ModeStates,    "automaton_mode_states.dfn");
	// Read "automaton_atk_states.dfn"
	if(_AtkStates.empty())
		CAutomatonListSheet::readDfnStringArray(_AtkStates,     "automaton_atk_states.dfn");
	// Read "automaton_spell_states.dfn"
	if(_SpellStates.empty())
		CAutomatonListSheet::readDfnStringArray(_SpellStates,   "automaton_spell_states.dfn");
	// Read "automaton_other_states.dfn"
	if(_OtherStates.empty())
		CAutomatonListSheet::readDfnStringArray(_OtherStates,   "automaton_other_states.dfn");

	// Get the move dist.
	double moveDist;
	if(!item.getValueByName(moveDist, "move dist"))
	{
		nlwarning("'move dist' not found.");
		moveDist = -1;
	}
	// IDLE
	// Create the automaton state.
	CAutomatonStateSheet mas;
	// Get the state name.
	mas.MoveState = CAnimationStateSheet::getAnimationStateId("idle");
	// Set the move dist.
	mas.BreakableOnMoveDist = moveDist;
	const string baseKeyStr = "idle";
	mas.build(item, baseKeyStr);
	// Try to Insert the new automaton state.
	TAutomatonStateSheets::iterator itStates = _States.find(mas.MoveState);
	// The state does not already exist -> Insert
	if(itStates == _States.end())
		_States.insert(make_pair(mas.MoveState, mas));
	// The state already exist -> Not Insert
	else
		nlwarning("The state '%s' already exists.", CAnimationStateSheet::getAnimationStateName(mas.MoveState).c_str());
	// OTHER
	buildStateSet(item, _MotionStates,  "motion",  moveDist);
	buildStateSet(item, _GenericStates, "generic", moveDist);
	buildStateSet(item, _ModeStates,    "mode",    moveDist);
	buildStateSet(item, _AtkStates,     "atk",     moveDist);
	buildStateSet(item, _SpellStates,   "spell",   moveDist);
	buildStateSet(item, _OtherStates,   "other",   moveDist);
}// build //

//-----------------------------------------------
//-----------------------------------------------
void CAutomatonSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont(_States);
}// serial //

//-----------------------------------------------
//-----------------------------------------------
const CAutomatonStateSheet *CAutomatonSheet::state(const TAnimStateKey &key) const
{
	// Search the key.
	TAutomatonStateSheets::const_iterator itStates = _States.find(key);
	if(itStates != _States.end())
		return &((*itStates).second);
	else
		return 0;
}// state //

//-----------------------------------------------
//-----------------------------------------------
void CAutomatonSheet::buildStateSet(const NLGEORGES::UFormElm &item, const std::vector<std::string> &states, const std::string &setName, double moveDist)
{
	// Read the form
	for(uint i=0; i<states.size(); ++i)
	{
		// Create the automaton state.
		CAutomatonStateSheet mas;
		// Get the state name.
		mas.MoveState = CAnimationStateSheet::getAnimationStateId(states[i]);
		// Set the move dist.
		mas.BreakableOnMoveDist = moveDist;

		const string baseKeyStr = setName + " states." + states[i];
		mas.build(item, baseKeyStr);

		// Try to Insert the new automaton state.
		TAutomatonStateSheets::iterator itStates = _States.find(mas.MoveState);
		// The state does not already exist -> Insert
		if(itStates == _States.end())
			_States.insert(make_pair(mas.MoveState, mas));
		// The state already exist -> Not Insert
		else
			nlwarning("The state '%s' already exists.", CAnimationStateSheet::getAnimationStateName(mas.MoveState).c_str());
	}
}// buildStateSet //

// ***************************************************************************
// CAutomatonListSheet
// ***************************************************************************

//-----------------------------------------------
// CAutomatonListSheet :
// Constructor.
//-----------------------------------------------
CAutomatonListSheet::CAutomatonListSheet()
{
	// Initialize the type.
	Type = CEntitySheet::AUTOMATON_LIST;

}

CAutomatonListSheet::~CAutomatonListSheet()
{
	std::map<std::string, CAutomatonSheet*>::iterator it = Automatons.begin(), itEnd = Automatons.end();
	while( it != itEnd )
		delete (*it++).second;
	Automatons.clear();
}

//-----------------------------------------------
// build :
// Build the sheet from an external script.
//-----------------------------------------------
void CAutomatonListSheet::build(const NLGEORGES::UFormElm &rootList)
{
	NLGEORGES::UFormLoader *formLoader = UFormLoader::createLoader();

	const UFormElm *list = 0;
	rootList.getNodeByName(&list, "list");
	if(list)
	{
		CSmartPtr<UForm> form = 0;

		uint size;
		list->getArraySize(size);
		for(uint i = 0; i<size; ++i)
		{
			string automatonType;
			if(list->getArrayValue(automatonType, i))
			{
				// 1 automaton per Mode
				for(uint mode = 0; mode<MBEHAV::NUMBER_OF_MODES; ++mode)
				{
					// Get the Mode Name
					string modeName = NLMISC::strlwr(MBEHAV::modeToString((MBEHAV::EMode)mode));
					// Compute the automaton name
					string filename = NLMISC::strlwr(automatonType) + "_" + modeName + ".automaton";
					// Push some information
					nlinfo("loading automaton '%s'.", filename.c_str());
					// Load the automaton's form.
					form = formLoader->loadForm(filename.c_str());
					if(form)
					{
						// Build the automaton.
						CAutomatonSheet *pAutomatonSheet = new CAutomatonSheet;
						if (pAutomatonSheet != NULL)
						{
							pAutomatonSheet->build(form->getRootNode());
							Automatons.insert(make_pair(filename, pAutomatonSheet));
						}
						else
							nlwarning("EAM::init: cannot create a CMovingAutomaton.");
					}
					else
						nlwarning("EAM::init: cannot load form '%s'.", filename.c_str());
				}
			}
			else
				nlwarning("EAM::init: Cannot get the value form the %d element.", i);
		}
	}
	else
		nlwarning("EAM::init: Cannot find the key 'list'.");

	UFormLoader::releaseLoader(formLoader);

}// build //

//-----------------------------------------------
// serial :
// Serialize character sheet into binary data file.

// version 9 : added stick mode to locate impact point
// version 8 : added projectile delay
// ..
// version 2 : added new animation fx sheets
// version 1 : base  version

//-----------------------------------------------
void CAutomatonListSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading())
	{
		uint32 nNb;
		f.serial(nNb);

		for (uint32 i = 0; i < nNb; ++i)
		{
			string sTmp;
			f.serial(sTmp);
			CAutomatonSheet *pAS = new CAutomatonSheet;
			f.serial(*pAS);
			Automatons.insert(make_pair(sTmp,pAS));
		}
	}
	else
	{
		uint32 nNb = (uint32)Automatons.size();
		f.serial(nNb);
		map<string, CAutomatonSheet*>::iterator it = Automatons.begin();
		while (it != Automatons.end())
		{
			string sTmp = it->first;
			f.serial(sTmp);
			f.serial(*(it->second));
			it++;
		}
	}
}// serial //

//-----------------------------------------------
// readDfnStringArray :
// read a dfn composed of an array of string
//-----------------------------------------------
void CAutomatonListSheet::readDfnStringArray(std::vector<std::string> &states, const std::string &dfnName)
{
	// Read "automaton_generic_states.dfn"
	nlinfo("Reading '%s'.", dfnName.c_str());
	NLGEORGES::UFormLoader *formLoader = UFormLoader::createLoader();
	CSmartPtr<UFormDfn> dfn = formLoader->loadFormDfn(dfnName.c_str());
	if(dfn)
	{
		uint nbStates = dfn->getNumEntry();
		nlinfo("  Nb States : %d", nbStates);
		for(uint i=0; i<nbStates; ++i)
		{
			string stateName;
			if(dfn->getEntryName(i, stateName))
			{
				nlinfo("    '%s'", stateName.c_str());
				states.push_back(stateName);
			}
			else
				nlwarning("Cannot Get the Name for the entry '%d'.", i);
		}
	}
	else
		nlwarning("Cannot load the dfn '%s'.", dfnName.c_str());
	UFormLoader::releaseLoader(formLoader);
}// readDfnStringArray //
