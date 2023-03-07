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



#ifndef NL_FORAGE_PROGRESS_H
#define NL_FORAGE_PROGRESS_H

#include "game_item_manager/game_item.h"


class CCharacter;
class CHarvestSource;

/**
 * Forage extraction progress for one player
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CForageProgress
{
	NL_INSTANCE_COUNTER_DECL(CForageProgress);
public:


	/// Constructor
	CForageProgress( const NLMISC::CSheetId& material, const TDataSetRow& sourceRowId, SKILLS::ESkills usedSkill, sint32 initialFocus )
	{
		reset( material, sourceRowId, initialFocus );
		_UsedSkill = usedSkill;
	}

	/// Reset
	void						reset( const NLMISC::CSheetId& material, const TDataSetRow& sourceRowId, sint32 initialFocus );

	/// Add the result of an extraction action
	void						fillFromExtraction( float quantity, float quality, CCharacter *player );

	/// Add the result of a care action
	void						fillFromCare( bool isUseful ) { _HasCastedUsefulCare |= isUseful; }

	/// Give forage result and XP. Return true if there is some raw material to take.
	bool						giveForageResult( CCharacter *player, const CHarvestSource *source );

	/// Return true if the session is finished and the result can be taken
	bool						resultCanBeTaken() const { return _ResultCanBeTaken; }

	/// Return the material sheet id
	const NLMISC::CSheetId&		material() const { return _Material; }

	/// Return the source dataset row
	const TDataSetRow&			sourceRowId() const { return _SourceRowId; }

	/// Return the amount
	uint16						amount() const { return (uint16)_Amount; }

	/// Return the quality
	uint16						quality() const { return (uint16)_Quality; }

	/// Return the amount of focus the player has when the extraction started
	sint32						initialFocus() const { return _InitialFocus; }

	/// Return true if the player has executed at least one useful care action
	bool						hasCastedUsefulCare() const { return _HasCastedUsefulCare; }

protected:

	/// Report the accumulated XP (currently only 1 skill can be used). Assumes extractor and source not null.
	void						reportXP( CCharacter *extractor, const CHarvestSource *source );

	/// Report the accumulated XP to a participant
	void						reportXPTo( CCharacter *player, float factor, CGameItemPtr toolUsed );

private:

	/// Raw material
	NLMISC::CSheetId			_Material;

	/// The source dataset row
	TDataSetRow					_SourceRowId;

	/// Quantity of material obtained so far
	float						_Amount;

	/// Average quality of material obtained so far
	float						_Quality;

	/// Skill used to forage
	SKILLS::ESkills				_UsedSkill;

	/// Amount of focus the player has when the extraction started
	sint32						_InitialFocus;

	/// Forage tool used
//	CGameItemRefPtr				_ForageToolUsed;

	/// True at the end of a successful extraction session
	bool						_ResultCanBeTaken;

	/// True if the player has executed at least one useful care action
	bool						_HasCastedUsefulCare;
};


#endif // NL_FORAGE_PROGRESS_H

/* End of forage_progress.h */
