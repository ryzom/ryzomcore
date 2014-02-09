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
#include "stage.h"
#include "game_share/entity_types.h"


///////////
// USING //
///////////
using namespace std;


/////////////
// METHODS //
/////////////
//-----------------------------------------------
// CStage :
// Constructor.
//-----------------------------------------------
CStage::CStage()
{
	_Time = 0.0;
	_PredictedInterval = 0;
	// default to full impact
	_LCTImpact= 256;
}// CStage //

//-----------------------------------------------
// addProperty :
// Add a property in the stage(and may replace the old one if wanted).
// \param property : property to add.
// \param value : value of the property.
// \param replace : 'true' to replace the old property if there is one.
//-----------------------------------------------
void CStage::addProperty(uint property, sint64 value, bool replace)
{
	// Try to insert a new property.
	pair<TStage::iterator, bool> result = _Stage.insert(make_pair(property, value));
	if(!result.second)
	{
		// Replace the property if wanted.
		if(replace)
		{
			TStage::iterator itProp = _Stage.find(property);
			if(itProp == _Stage.end())
				nlwarning("CStage::addProperty: Cannot insert the property '%d' but cannot find it too.", property);
			else
				(*itProp).second = value;
		}
	}
}// addProperty //

//-----------------------------------------------
// removeProperty :
// Remove the selected property from the stage.
// \param property : property to remove from this stage.
//-----------------------------------------------
void CStage::removeProperty(uint property)
{
	_Stage.erase(property);
}// removeProperty //


//-----------------------------------------------
// Return a pair according to the property asked. First element (the bool), indicate if the value (second element) is valid.
// \param uint prop : property asked (to get its value).
// \return pair<bool, sint64> : '.first' == 'true' if the property exist so value is valid. '.second' is the value if '.first' == 'true'.
//-----------------------------------------------
std::pair<bool, sint64> CStage::property(uint prop) const
{
	// Create the result.
	std::pair<bool, sint64> result;

	// Search for the property
	TStage::const_iterator it = _Stage.find(prop);
	if(it != _Stage.end())
	{
		// Property exist -> update the result.
		result.first = true;
		result.second = (*it).second;
	}
	// Property dose not exist -> value is not valid.
	else
		result.first = false;

	// Return the result.
	return result;
}// property //

//-----------------------------------------------
// getPos :
// Get the position in the stage or return false.
// \param pos : will be filled with the position or left untouched.
// \return bool : true if pos has been filled.
//-----------------------------------------------
bool CStage::getPos(NLMISC::CVectorD &pos) const
{
	// Get the X value.
	pair<bool, sint64> resultX = property(CLFECOMMON::PROPERTY_POSX);
	if(resultX.first == false)
		return false;

	// Get the Y value.
	pair<bool, sint64> resultY = property(CLFECOMMON::PROPERTY_POSY);
	if(resultY.first == false)
	{
		nlwarning("CStage:getPos: there is property X in the Stage but no property Y.");
		return false;
	}

	// Get the Z value.
	pair<bool, sint64> resultZ = property(CLFECOMMON::PROPERTY_POSZ);
	if(resultZ.first == false)
	{
		nlwarning("CStage:getPos: there are properties X and Y in the Stage but no property Z.");
		return false;
	}

	// Convert it into a CVectorD
	pos.x = (double)((sint32)resultX.second)/1000.0f;
	pos.y = (double)((sint32)resultY.second)/1000.0f;
	pos.z = (double)((sint32)resultZ.second)/1000.0f;
	// Position filled.
	return true;
}// getPos //

//-----------------------------------------------
// serial :
// Serialize entities.
//-----------------------------------------------
void CStage::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(_Time);
	f.serialCont(_Stage);
	f.serial(_PredictedInterval);
}// serial //


///////////////
// CStageSet //
//-----------------------------------------------
// addStage :
// Try to add a new property for the stage corresponding to the Game Cycle.
// May also add a new stage if there is no stage for the Game Cycle.
// \warning If the property already exist, this method does not change the value.
// \warning If the Game Cycle is before the first element, this method do not try to add a stage but will try to add a property to the first one.
// \param TGameCycle gameCycle : This is the Key for the stage to search where to add the property.
// \param uint property : property to add.
// \param uint64 value : value of the property.
// \return CStage * : pointer on the stage or 0.
//-----------------------------------------------
CStage *CStageSet::addStage(NLMISC::TGameCycle gameCycle, uint property, sint64 value)
{
	// Try to find the element in the map.
	TStageSet::iterator it;
	bool replace;

	// If there is at least 1 stage.
	if(!_StageSet.empty())
	{
		// Get the Game Cycle for the first stage.
		NLMISC::TGameCycle gcFirst = (*_StageSet.begin()).first;
		// New Stage and before the first one.
		if(gameCycle < gcFirst)	// Could be egal if we do not replace the property in the case the property already exist too.
		{
			// Game Cycle to early -> put the property in the first one instead (but do not replace the property in the first 1 if already exist).
			it = _StageSet.begin();
			replace = false;
		}
		// The property will not be inserted before the first one.
		else
		{
			// Try to insert a new stage.
			pair<TStageSet::iterator, bool> result = _StageSet.insert(make_pair(gameCycle, CStage()));
			if(!result.second)
			{
				TStageSet::iterator itFind = _StageSet.find(gameCycle);
				if(itFind == _StageSet.end())
				{
					nlwarning("CStageSet::addStage: Cannot insert a stage for the gamecycle '%d' but cannot find too.", gameCycle);
					return 0;
				}

				// Set the iterator.
				it = itFind;
				replace = true;
			}
			else
			{
				// Set the iterator.
				it = result.first;
				replace = true;
			}
		}
	}
	// There is no stage for the moment.
	else
	{
		// Try to insert a new stage.
		pair<TStageSet::iterator, bool> result = _StageSet.insert(make_pair(gameCycle, CStage()));
		if(!result.second)
		{
			nlwarning("CStageSet::addStage : Strange, map is empty but cannot insert a new element in the map.");
			return 0;
		}

		// Set the iterator (can be a new one or an existing one.
		it = result.first;
		replace = true;
	}

	// Add the property in the stage.
	(*it).second.addProperty(property, value, replace);

	// Return a pointer on the stage.
	return &((*it).second);
}// addStage //

//-----------------------------------------------
// addStage :
// Try to add a new property for the stage corresponding to the Game Cycle.
// May also add a new stage if there is no stage for the Game Cycle.
// \warning If the property already exist, this method does not change the value.
// \warning If the Game Cycle is before the first element, this method do not try to add a stage but will try to add a property to the first one.
// \param TGameCycle gameCycle : This is the Key for the stage to search where to add the property.
// \param uint property : property to add.
// \param uint64 value : value of the property.
// \return CStage * : pointer on the stage or 0.
//-----------------------------------------------
CStage *CStageSet::addStage(NLMISC::TGameCycle gameCycle, uint property, sint64 value, NLMISC::TGameCycle predictedI)
{
	// Add a new stage if not already present
	CStage *stage = addStage(gameCycle, property, value);
	// Set the predicted interval.
	if(stage)
	{
		stage->predictedInterval(predictedI);
		return stage;
	}
	else
		return 0;
}// addStage //




//-----------------------------------------------
// serial :
// Serialize entities.
//-----------------------------------------------
void CStageSet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Serialize the map.
	f.serialCont(_StageSet);
}// serial //


//-----------------------------------------------
// removePosWithNoMode :
// Remove Positions except for those with a mode.
//-----------------------------------------------
void CStageSet::removePosWithNoMode()
{
	TStageSet::iterator it = _StageSet.begin();
	while(it != _StageSet.end())
	{
		// Get the reference on the current stage
		CStage &stage = (*it).second;
		// If there is no mode is the stage
		if(stage.isPresent(CLFECOMMON::PROPERTY_MODE) == false)
		{
			// Remove the position
			stage.removeProperty(CLFECOMMON::PROPERTY_POSX);
			stage.removeProperty(CLFECOMMON::PROPERTY_POSY);
			stage.removeProperty(CLFECOMMON::PROPERTY_POSZ);
		}
		// Next Stage
		++it;
	}
}// removePosWithNoMode //
