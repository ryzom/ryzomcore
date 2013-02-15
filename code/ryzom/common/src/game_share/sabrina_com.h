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



#ifndef NL_SABRINA_COM_H
#define NL_SABRINA_COM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "skills.h"
#include "crafting_tool_type.h"
#include "brick_types.h"
#include "brick_families.h"
#include <vector>


// ***************************************************************************
/**
 * Common Client/Server class to get related "Sabrina" infos
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CSabrinaCom
{
public:
	class	IBrickContainer
	{
	public:
		IBrickContainer() {}
		virtual 			~IBrickContainer() {}

		/// get the sabrina cost for a given sbrick
		virtual sint32		getSabrinaCost(NLMISC::CSheetId id) const =0;

		/// get the relative sabrina cost for a given sbrick
		virtual float		getSabrinaRelativeCost(NLMISC::CSheetId is) const =0;

		/// get the number of parameters required for a given sbrick
		virtual sint32		getNumParameters(NLMISC::CSheetId id) const =0;

		/// get the brick type of this brick
		virtual BRICK_FAMILIES::TBrickFamily	getBrickFamily(NLMISC::CSheetId id, uint& indexInFamily) const =0;

		/// get the brick type of this brick
		virtual BRICK_TYPE::EBrickType		getBrickType(NLMISC::CSheetId id) const =0;

		/// get the associated tooltype of this faber brick (unknown if not a faber brick)
		virtual TOOL_TYPE::TCraftingToolType		getFaberPlanToolType(NLMISC::CSheetId id) const =0;
	};

public:
	/// Constructor. must set the necessary IBrickContainer implementation. owned but not deleted.
	CSabrinaCom(IBrickContainer *bc);
	~CSabrinaCom();

	/// get the positive and negative cost of a Sabrina Phrase.
	void				getPhraseCost(const std::vector<NLMISC::CSheetId> &bricks, uint32 &pos, uint32 &neg) const;

	/// get the maximum positive cost of a Sabrina Phrase.
	uint32				getPhraseMaxBrickCost(const std::vector<NLMISC::CSheetId> &bricks) const;

	/// For a specific phrase and a specific index in it, give the cost of mandatory/optional/credit and all its parameters (neg or pos)
	sint32				getPhraseBrickAndParamCost(const std::vector<NLMISC::CSheetId> &bricks, uint brickIndex) const;

	/// For a specific phrase and a specific index in it, give the relative cost of mandatory/optional/credit and all its parameters (neg or pos)
	float				getPhraseBrickAndParamRelativeCost(const std::vector<NLMISC::CSheetId> &bricks, uint brickIndex) const;


	/** For complex rules of Mandatory exclusion, fill this method.
	 *	\param phraseBricks array of bricks of the phrase currently composed in the client. Note that the client fill with 0 mandatories
	 *		which have not been selected (consider 0 params for unselected mandatory bricks)
	 *	\param mandatoryBricks list of bricks that the client have detected for possible selection. remove any you don't want.
	 */
	void				filterMandatoryComposition(const std::vector<NLMISC::CSheetId> &phraseBricks, std::vector<NLMISC::CSheetId> &mandatoryBricks) const;

	/// For Faber.
	TOOL_TYPE::TCraftingToolType		getPhraseFaberPlanToolType(const std::vector<NLMISC::CSheetId> &phraseBricks) const;

	/// For Display. Return the brick (should be in phrase) used to display the phrase as icon
	NLMISC::CSheetId	getPhraseBestDisplayBrick(const std::vector<NLMISC::CSheetId> &phraseBricks) const;

	/** For Display. Return true if the main icon of the brick is in the IconOver slot
	 * (when Icon used by another icon). Return iconOver2NotSuitableForActionDisplay as well.
	 */
	bool				isMainDisplayIconInOverSlot(const NLMISC::CSheetId &brickId, bool& iconOver2NotSuitableForActionDisplay) const;

private:
	IBrickContainer			*_BC;
};


#endif // NL_SABRINA_COM_H

/* End of sabrina_com.h */
