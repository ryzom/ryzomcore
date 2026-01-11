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



#ifndef NL_FORAGE_SOURCE_CL_H
#define NL_FORAGE_SOURCE_CL_H

#include "nel/misc/types_nl.h"
#include "fx_cl.h"


/**
 * <Class description>
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CForageSourceCL : public CFxCL
{
public:

	enum TFSBarIndex { FSBTime, FSBQuantiy, FSBD, FSBE, FSBK, NbFSBarIndices };

	static NLMISC::CRGBA SafeSourceColor;

	/// Constructor
	CForageSourceCL();

	/// Destructor
	virtual ~CForageSourceCL();

	/// Build the entity from a sheet.
	virtual bool build( const CEntitySheet *sheet );

	/// Rebuild in scene interfaces
	virtual void buildInSceneInterface ();

	/// Called when clipped out
	virtual void updateClipped (const NLMISC::TTime &currentTimeInMs, CEntityCL *target);

	/// Method called each frame to manage the entity after the clipping test if the primitive is visible.
	virtual void updateVisible(const NLMISC::TTime &time, CEntityCL *target);

	/// Update the entity after all positions done.
	virtual void updateVisiblePostPos(const NLMISC::TTime &time, CEntityCL *target);

	/// Return true if at least an extraction is in progress
	bool	isExtractionInProgress() const { return _IsExtractionInProgress; }

	/// Return the knowledge level [0..2]
	uint8	getKnowledge() const { return (_KnowledgeLevel & 0x80) != 0 ? 0 : _KnowledgeLevel; }

	/// Return the icon filename (valid if the knowledge > 0, otherwise returns NULL)
	const std::string	*getKnowledgeIcon() const { return _IconFilename; }

	/// Return the value of the K (max=127)
	uint8	getKamiAngerBar() const { return _BarDestValues[4]; }

	/// Return true if the source is in "extra time"
	bool	isInExtraTime() const { return _BarDestValues[0] < _ExtraTime; }

	/// Return true if the source is in "prospection extra time" (not bonus extra time or normal extra time)
	bool	isInProspectionExtraTime() const { return  _BarDestValues[0] < _ExtraTime - _InclBonusExtraTime; }

	/// Return the current quantity available in the source
	uint8	getCurrentQuantity() const { return _CurrentQuantity; }

	void resetVP();

	void displayInscenePos();

	/// Current Display Value for bars
	uint8								getTimeBar() const {return _TimeBar;}
	uint8								getQuantityBar() const {return _QuantityBar;}
	uint8								getDBar() const {return _DBar;}
	uint8								getEBar() const {return _EBar;}

	bool isSafe() { return _SafeSource; }

protected:

	/// Initialize properties of the entity (according to the class).
	virtual void initProperties();

	// Update Entity Visual Property B
	//virtual void updateVisualPropertyVpb(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);

	/// Update Entity Bars
	virtual void updateVisualPropertyBars(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Visual FX
	virtual void updateVisualPropertyVisualFX(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Name.
	virtual void updateVisualPropertyName         (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Target.
	virtual void updateVisualPropertyTarget       (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Orientation.
	virtual void updateVisualPropertyOrient       (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);

	/// Display the modifiers
	virtual void displayModifiers();

	/// Destroy inscene interfaces
	void releaseInSceneInterfaces();

	/// Inscene interface
	class CGroupInSceneUserInfo			*_InSceneUserInterface;

private:

	/// Knowledge icon (valid only if _KnowledgeLevel > 0, otherwise NULL)
	const std::string					*_IconFilename;

	/// Destination values for bars
	uint8								_BarDestValues[NbFSBarIndices];

	/// Current value for bars (except the K bar)
	float								_BarCurrentValues[NbFSBarIndices-1];

	/// Current Display Value for bars
	uint8								_TimeBar;
	uint8								_QuantityBar;
	uint8								_DBar;
	uint8								_EBar;

	/// When an extraction is in progress, display all bars
	bool								_IsExtractionInProgress;

	/// If the source is safe display its explosion bar green
	bool								_SafeSource;

	/// 0=nothing 1=group 2=family. Bit 7 set means "knowledge info not received yet" (by visual fx)
	uint8								_KnowledgeLevel;

	/// Memorize the last received explosion bit
	uint8								_LastExplosionSwitch;

	/// The prospector slot (received in target)
	uint8								_ProspectorSlot;

	/// When the source is in "extra time", change the display of the time bar
	uint8								_ExtraTime;

	/// When the source is in "bonus extra time", change the display of the time bar
	uint8								_InclBonusExtraTime;

	/// First received value of quantity
	uint8								_InitialQuantity;

	/// Current quantity
	uint8								_CurrentQuantity;
};


#endif // NL_FORAGE_SOURCE_CL_H

/* End of forage_source_cl.h */
