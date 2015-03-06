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



#ifndef RY_SBRICK_MANAGER_H
#define RY_SBRICK_MANAGER_H

#include "nel/misc/types_nl.h"
#include "../client_sheets/sbrick_sheet.h"
#include "game_share/sabrina_com.h"
#include "game_share/skills.h"
#include "nel/misc/cdb.h"
#include "brick_learned_callback.h"


// ***************************************************************************
/**
 * Manager of Sabrina Bricks.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CSBrickManager
{
public:
	~CSBrickManager();

	///get the singleton 's instance
	static CSBrickManager* getInstance()
	{
		if (!_Instance)
			_Instance = new CSBrickManager;
		return _Instance;
	}

	// release singleton
	static void releaseInstance();

	// Initialize by loading bricks done at init time
	void init();

	// Init the rest done at initInGame time
	void initInGame();

	// Uninit in game data
	void uninitInGame();

	/**
	 * get a brick from its sheetId
	 * param id : the id of the sheet
	 */
	CSBrickSheet *getBrick(const NLMISC::CSheetId &id) const
	{
		uint32 shid = id.getShortId();
		CSBrickSheet *result = NULL;
		if (shid < _BrickVector.size())
			result = _BrickVector[shid];
		//if (!result)
		//	nlwarning("Missing brick '%s'", id.toString().c_str());
		return result;
	}

	/**
	 * \return a sheet id of a brick
	 */
	NLMISC::CSheetId getBrickSheet(uint family, uint index) const;

	/**
	 * \return a family bit set
	 */
	sint64			getKnownBrickBitField(uint family) const;

	/**
	 * \return the DB pointing on the BitField for this family.
	 */
	class NLMISC::CCDBNodeLeaf*	getKnownBrickBitFieldDB(uint family) const;

	/**
	 * \return true if the brick is learn by the player.
	 */
	bool	isBrickKnown(CSBrickSheet *brick) const
	{
		if(!brick)
			return false;
		// IndexInFamily-1 because start at 1 in the sheets
		return ( getKnownBrickBitField(brick->BrickFamily) & ( sint64(1)<<(brick->IndexInFamily-1)) )!=0;
	}
	/// same with sheetId
	bool	isBrickKnown(NLMISC::CSheetId id) const
	{
		return isBrickKnown(getBrick(id));
	}

	/// Get list of all bricks for a family
	const std::vector<NLMISC::CSheetId>		&getFamilyBricks(uint family) const;

	/// Get list of all root bricks
	const std::vector<NLMISC::CSheetId>		&getRootBricks() const {return _Roots;}

	/// Get the SabrinaCom info retriever.
	const CSabrinaCom						&getSabrinaCom() const {return _SabrinaCom;}

	/// Get the Interface Brick (visual only) related to a skill
	NLMISC::CSheetId						getVisualBrickForSkill(SKILLS::ESkills s);

	/// Get the Interface Brick (visual only) used to remove a optional or credit brick
	NLMISC::CSheetId						getInterfaceRemoveBrick();

	/// Modify a list of bricks to keep only the knowns one
	void									filterKnownBricks(std::vector<NLMISC::CSheetId> &bricks);

	/// \name Brick Properties
	// @{
	// get a prop Id from its name.
	uint			getBrickPropId(const std::string &name);
	// Important Ids for properties (to compute cost, range etc...).
	uint			HpPropId;
	uint			SapPropId;
	uint			StaPropId;
	uint			StaWeightFactorId;
	uint			FocusPropId;
	uint			CastTimePropId;
	uint			RangePropId;
	// @}

	// append to the set a callback when a brick is learned
	void			appendBrickLearnedCallback(IBrickLearnedCallback *cb);
	void			removeBrickLearnedCallback(IBrickLearnedCallback *cb);

protected:

	/// Constructor
	CSBrickManager();

	/// Singleton's instance
	static CSBrickManager* _Instance;

	/// Number of families
	uint _NbFamily;

	/// Number of bricks in each family
	std::vector<uint> _NbBricksPerFamily;

	/// Map linking a sheet id to a brick record.
	// std::map<NLMISC::CSheetId,CSBrickSheet*> _Bricks;
	std::vector<CSBrickSheet *> _BrickVector;

	/// Structure storing all bricks. each entry of the vector 
	/// represent a family, described by a vector containing all 
	/// the bricks of the family
	std::vector<std::vector<NLMISC::CSheetId> > _SheetsByFamilies;

	/// Vector of bit fields describing the known bricks of each family
	std::vector<NLMISC::CCDBNodeLeaf*> _FamiliesBits;

	/// List of roots only
	std::vector<NLMISC::CSheetId> _Roots;

	/// Mapper of SkillToBrick
	NLMISC::CSheetId		_VisualBrickForSkill[SKILLS::NUM_SKILLS];

	void			makeRoots();

	void			checkBricks();

	void			makeVisualBrickForSkill();

	// GameShare infos on Sabrina.
	class	CBrickContainer : public CSabrinaCom::IBrickContainer
	{
		virtual sint32		getSabrinaCost(NLMISC::CSheetId id) const;
		virtual float		getSabrinaRelativeCost(NLMISC::CSheetId id) const;
		virtual sint32		getNumParameters(NLMISC::CSheetId id) const;
		virtual BRICK_FAMILIES::TBrickFamily	getBrickFamily(NLMISC::CSheetId id, uint& indexInFamily) const;
		virtual BRICK_TYPE::EBrickType				getBrickType(NLMISC::CSheetId id) const;
		virtual TOOL_TYPE::TCraftingToolType		getFaberPlanToolType(NLMISC::CSheetId id) const;
	};
	CBrickContainer		_BrickContainer;
	CSabrinaCom			_SabrinaCom;

	// Brick Properties
	std::map<std::string, uint>		_BrickPropIdMap;

	void			compileBrickProperties();

	// see getInterfaceRemoveBrick
	NLMISC::CSheetId	_InterfaceRemoveBrick;

	// Observer when a brick is learned
	struct CBrickFamilyObs : public NLMISC::ICDBNode::IPropertyObserver
	{
		CSBrickManager		*Owner;

		virtual void update (NLMISC::ICDBNode *node);
	};
	friend struct CBrickFamilyObs;
	CBrickFamilyObs			_BrickFamilyObs;
	typedef	std::set<IBrickLearnedCallback*>	TBLCBSet;
	TBLCBSet							_BrickLearnedCallbackSet;
};


#endif // NL_SBRICK_MANAGER_H

/* End of sbrick_manager.h */
