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



#ifndef NL_DBGROUP_BUILD_PHRASE_H
#define NL_DBGROUP_BUILD_PHRASE_H

#include "nel/misc/types_nl.h"
#include "nel/gui/interface_group.h"
#include "game_share/sphrase_com.h"
#include "game_share/brick_types.h"
#include "game_share/skills.h"


namespace NLGUI
{
	class CCtrlBaseButton;
	class CViewText;
	class CViewBitmap;
	class CGroupEditBox;
}


// ***************************************************************************
class	CDBCtrlSheet;
class	CSBrickSheet;

// ***************************************************************************
/**
 * Widget to handle building sentence.
 *	A Main Brick is a Root/Mandatory/Optional/Credit brick. ie everything but a Param
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CDBGroupBuildPhrase : public CInterfaceGroup
{
public:
	enum	{MaxParam= 4};
	static const std::string	BrickSelectionModal;
	static const std::string	BrickSelectionViewNotUsed;
	static const std::string	BrickSelectionDB;
	static const std::string	BrickOptionalMenu;
	static const std::string	BrickMandatoryAHRightClick;
	static const std::string	BrickBuildDB;
	enum	{MaxSelection= 256};
	enum	{MaxBricks= 64};


	/** A Brick slot.
	 *	NB: A Slot can exist at runtime only if Brick and Back are setuped.
	 */
	class	CSlot
	{
	public:
		CDBCtrlSheet			*Brick;
		CViewBitmap				*Back;
		bool					Valid;
		// The Parameter families setuped for this slot (ie if != from Brick->ParameterFamilies, must update display)
		std::vector<uint16>		ViewParamFamilies;

		// reset display.
		void			reset();

		// set default with a family

		CSlot()
		{
			Brick= NULL;
			Back= NULL;
			Valid= false;
		}
	};

	/** Descriptor of a word of the builded sentence
	 *	NB: A Word can exist at runtime only if all his slots can exist (Brick and Back are setuped.)
	 *	But for Credits...
	 */
	class	CWord
	{
	public:
		CSlot					Slot;
		CSlot					ParamSlot[MaxParam];
		// the total number of parameters (params and their possible son)
		uint32					NumTotalParams;
		bool					ParamError;
		// The Cost View for this line.
		CViewText				*CostView;
		CViewText				*CreditView;
		CViewText				*InfoView;

		// reset display.
		void			reset();
		// reset only all params
		void			resetParams();
		// enable disaply and set the brick
		void			setBrick(uint32 sheetId);
		void			setParamBrick(uint param, uint32 sheetId);
		// for optional delete, copy state from another word
		void			copySetup(const CWord &w);

		CWord()
		{
			NumTotalParams= 0;
			ParamError= false;
			CostView= NULL;
			CreditView= NULL;
			InfoView= NULL;
		}
	};

public:

	/// Constructor
	CDBGroupBuildPhrase(const TCtorParam &param);
	virtual ~CDBGroupBuildPhrase();

	/// CInterfaceGroup Interface
	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
	virtual void updateCoords ();
	virtual void draw ();

	// Get the Validate Button if possible
	CCtrlBaseButton		*getValidateButton() const {return _ValidateButton;}

	// clear the phrase
	void			clearBuildingPhrase();
	// call after clearBuildingPhrase(), to copy a phrase, or empty phrase to init new Name.
	void			startComposition(const CSPhraseCom &phrase);

	// build the current composed phrase (result of the edition).
	void			buildCurrentPhrase(CSPhraseCom &newPhrase);

	/** For edition, can set a brickType filter for the root. if !Unknown, only root of this type will match
	 *	Second and more filter are ORed with the root brick, if unknown, not ored.
	 */
	void			setRootBrickTypeFilter(BRICK_TYPE::EBrickType	rootBtFilter,
		BRICK_TYPE::EBrickType	rootBtFilter2= BRICK_TYPE::UNKNOWN,
		BRICK_TYPE::EBrickType	rootBtFilter3= BRICK_TYPE::UNKNOWN,
		BRICK_TYPE::EBrickType	rootBtFilter4= BRICK_TYPE::UNKNOWN);

	/// \name Brick Selection
	// @{
	void			fillSelectionRoot();
	// if index==0, you must call fillSelectionRoot(). It can be a Mandatory/Optional/Credit
	void			fillSelectionMain(uint index);
	void			fillSelectionParam(uint index, uint paramIndex);
	void			fillSelectionNewOp();
	void			fillSelectionNewCredit();
	void			validateRoot(const CSBrickSheet *sheet);
	// if index==0, you must call validateRoot()
	void			validateMain(uint index, const CSBrickSheet *sheet);
	void			validateParam(uint index, uint paramIndex, const CSBrickSheet *sheet);
	void			validateNewOpCredit(const CSBrickSheet *sheet);
	// NB: the index is the index of the brick
	void			deleteOpCredit(uint index);
	// @}

	/// Call this when the name or the root brick change, to update the view of the spell edited only.
	void			updateSpellView();

	/// Call this whill update all the view
	void			updateAllDisplay();

protected:
	bool						_GroupValid;
	// In this array comes in Order Root/Mandatories/Optionals/Credits
	std::vector<CWord>			_MainWords;
	CCtrlBaseButton				*_ValidateButton;
	CViewText					*_TextPhraseDesc;
	// The number of mandatories, not including the root.
	uint32						_NumMandatories;
	// The number of activated optional
	uint32						_NumOptionals;
	// The number of activated credit
	uint32						_NumCredits;

	bool						_Setuped;

	// the color of an unknown color brick
	sint32						_TextureIdSlotDisabled;

	// filter for the root
	enum	{MaxRootBrickTypeFilter= 4};
	BRICK_TYPE::EBrickType		_RootBrickTypeFilter[MaxRootBrickTypeFilter];

protected:
	void			setupBuildSentence();

	// reset the sentence with a sheet.
	void			resetSentence(sint32 rootSheetId);
	void			updateDisplayFromRootBrick();

	// update the parameters of the indexed main brick
	void			updateParams(uint index);
	// reset the parameter hierarchy of the indexed main brick.
	void			updateParamHrc(uint index);
	void			updateNewButtons();
	void			updateAllDisplay(const CSPhraseCom &phrase);

	// fill array of bricks possible for a new optional brick.
	void			fillNewOptionalBricks(std::vector<NLMISC::CSheetId> &bricks);
	void			fillNewCreditBricks(std::vector<NLMISC::CSheetId> &bricks);
	void			resetSelection();
	void			fillSelection(const std::vector<NLMISC::CSheetId> &bricks);
	void			filterKnownBricks(std::vector<NLMISC::CSheetId> &bricks);
	void			filterBrickExclusion(std::vector<NLMISC::CSheetId> &bricks, uint16 indexToSkip = 0xffff);
	void			filterFamilySetuped(std::vector<uint16> &families);
	void			filterBrickSetuped(std::vector<NLMISC::CSheetId> &bricks);
	/// Filter bricks so only one that are compatible with skill of the current setuped one are filtered.
	void			filterSkillSetuped(std::vector<NLMISC::CSheetId> &bricks, bool checkOptional, bool checkCredit, sint avoidCheckIndex= -1);
	/// for special root edition
	void			filterRootBrickType(std::vector<NLMISC::CSheetId> &bricks);
	void			filterRootPossibles(std::vector<NLMISC::CSheetId> &bricks);

	// return the current number of main brick setuped. at least 1 with the root
	uint			getNumMainBricks() const {return 1+_NumMandatories+_NumOptionals+_NumCredits;}

	// get the current rootBrick
	const CSBrickSheet	*getRootBrick();

protected:
	CGroupEditBox				*_UserSentenceName;
	sint						_NewSpellNumber;
	// The spell icon builded.
	CDBCtrlSheet				*_SpellView;

	// For weapon restriction
	std::vector<SKILLS::ESkills>	_WeaponSkills;
};


#endif // NL_DBGROUP_BUILD_PHRASE_H

/* End of dbgroup_build_phrase.h */
