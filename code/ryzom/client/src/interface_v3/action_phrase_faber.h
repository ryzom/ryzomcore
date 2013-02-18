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



#ifndef NL_ACTION_PHRASE_FABER_H
#define NL_ACTION_PHRASE_FABER_H

#include "nel/misc/types_nl.h"
#include "inventory_manager.h"
#include "game_share/rm_family.h"
#include "game_share/brick_families.h"
#include "game_share/item_origin.h"
#include "skill_change_callback.h"


// ***************************************************************************
class	CSBrickSheet;


// ***************************************************************************
// There is at max 10 requirement line: 5 item part, and 5 formula items
#define	MAX_ITEM_REQ_LINE		10
#define	MAX_MP_SLOT				36


// ***************************************************************************
/**
 * Faber Execution Manager
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CActionPhraseFaber
{
public:

	/// Constructor
	CActionPhraseFaber();

	/// open the window
	void		launchFaberCastWindow(sint32 memoryLine, uint memoryIndex, CSBrickSheet *rootBrick);
	/// called when the window is closed
	void		onCloseFaberCastWindow();


	/// Fill the Faber Plan selection DB (no window opened)
	void		fillFaberPlanSelection(const std::string &brickDB, uint maxSelection);
	/// Called when the user has selected the Plan. copy bag. the itemPlanBrick must have "FaberPlan" good infos.
	void		validateFaberPlanSelection(CSBrickSheet *itemPlanBrick);


	/// Called when the user click on a slot (full or empty)
	void		startMpSelection(uint itemReqLine, uint mpSlot);
	/// Called when the user validate click on a mp slot
	void		validateMpSelection(uint selectId);
	/// Called when the user selected the mp quantity to change
	void		validateMpSelectQuantity();


	/// Validate the execution
	void		validateExecution();



private:
	struct	CItem
	{
		// Item Origin
		uint16					InventoryId;		// INVENTORIES::TInventory
		uint16					IdInInventory;
		// Item State
		NLMISC::CSheetId		Sheet;
		sint32					Quality;
		sint32					Quantity;
		sint32					UserColor;
		sint32					Weight;
		// BitField to know which itemReqLine has selected this Item
		uint					Selected;
		// This is the original quantity in inventory
		sint32					OriginalQuantity;
		bool                    LockedByOwner;

		CItem() : Sheet(0)
		{
			Quality= 0;
			Quantity= 0;
			UserColor= 0;
			Weight= 0;
			Selected= 0;
			OriginalQuantity= 0;
		}

		void		reset()
		{
			InventoryId= 0;
			IdInInventory= 0;
			Sheet= NLMISC::CSheetId::Unknown;
			Quality= 0;
			Quantity= 0;
			UserColor= 0;
			Weight= 0;
			Selected= 0;
			OriginalQuantity= 0;
		}
	};

	// The observer in case of skill change
	class CSkillObserver : public ISkillChangeCallback
	{
	public:
		virtual void onSkillChange();
	};
	CSkillObserver          _SkillObserver;
	friend class CSkillObserver;

	// For Selection/Validate
	uint					_MpSelectionItemReqLine;
	std::vector<uint>		_MpCurrentSelection;
	// For Change Quantity/Validate
	uint					_MpChangeQuantitySlot;

	// the Launched Action
	sint32					_ExecuteFromMemoryLine;
	uint					_ExecuteFromMemoryIndex;
	CSBrickSheet			*_ExecuteFromItemPlanBrick;
	// The required brick family for the rootBrick
	std::vector<BRICK_FAMILIES::TBrickFamily>	_FaberPlanBrickFamilies;

	// The Inventory manipulated.
	std::vector<CItem>		_InventoryMirror;
	bool					_InventoryObsSetup;
	class CDBInventoryObs : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(NLMISC::ICDBNode* node);
	};
	CDBInventoryObs			_DBInventoryObs;
	friend class			CDBInventoryObs;
	// The animals Status
	class CDBAnimalObs : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(NLMISC::ICDBNode* node);
	};
	CDBAnimalObs			_DBAnimalObs;
	friend class			CDBAnimalObs;


	// The Current MP Construction for a Line of Item Requirement
	class CMPBuild
	{
	public:
		// Different Type of requirement per line
		enum	TReqLineType
		{
			ItemPartReq=0,	// The line requires MPs that can craft a particular ItemPart
			SpecificItemReq,		// The line requires specific MP
			NumReqLineType
		};

	public:
		TReqLineType				RequirementType;
		// Bkup from BrickPlan
		RM_FABER_TYPE::TRMFType		FaberTypeRequired;		// valid if RequirementType==ItemPartReq
		NLMISC::CSheetId			SpecificItemRequired;	// valid if RequirementType==SpecificItemReq
		// For each MpSlot, the Inventory index (points to _InventoryMirror), and the quantity selected
		uint				Id[MAX_MP_SLOT];
		// For each MpSlot, the quantity selected
		uint				QuantitySelected[MAX_MP_SLOT];
		// The quantity required for this line
		uint				QuantityReq;
		// The number of mpSlot setuped (ie where quantity!=0)
		uint				NumMpSlot;

	public:
		CMPBuild()
		{
			reset();
		}

		void	reset()
		{
			RequirementType= ItemPartReq;
			FaberTypeRequired= RM_FABER_TYPE::Unknown;
			SpecificItemRequired= NLMISC::CSheetId::Unknown;
			QuantityReq= 0;
			NumMpSlot= 0;

			// For all slot, reset the quantities setuped
			for(uint mpSlot=0;mpSlot<MAX_MP_SLOT;mpSlot++)
			{
				QuantitySelected[mpSlot]= 0;
				Id[mpSlot]= 0;	// useless, but for consistency
			}
		}
	};
	CMPBuild				_MPBuild[MAX_ITEM_REQ_LINE];
	uint					_MPBuildNumItemPartReq;			// The number of different itempart lines requirement
	uint					_MPBuildNumSpecificItemReq;		// The number of different specific items lines requirement
	uint					_MPBuildNumTotalItemReq;		// Total of requirement

	void			fillDBWithMP(const std::string &sheetBase, const CItem &item);

	void			resetSelection();
	void			fillSelection(const std::vector<uint> &mps);
	void			filterSelectionItemPart(std::vector<uint> &mps, RM_FABER_TYPE::TRMFType itemPartFilter, ITEM_ORIGIN::EItemOrigin originFilter);
	void			filterSelectionItemSpecific(std::vector<uint> &mps, NLMISC::CSheetId specificItemWanted);

	uint			getMaxQuantityChange(uint itemReqLine, uint mpSlot) const;
	uint			getTotalQuantitySetuped(uint itemReqLine) const;

	void			updateEmptySlot(uint itemReqLine, CInterfaceGroup *itemReqLineGroup=NULL);
	void			updateQuantityView(uint itemReqLine);
	void			updateValidButton();

	void			deleteMpSlot(uint itemReqLine, uint mpSlot);

	// When the inventory is modified, must do some checks on the current faber execution
	void			onInventoryChange();
	void			removeMpSlotThatUseInvSlot(uint invSlot, uint quantityToRemove);

	// from an index in _InventoryMirror, get the ItemImage
	CItemImage		*getInvMirrorItemImage(uint slotIndex, uint& invId, uint& indexInInv);
	bool			isMpAvailable(CItemSheet *mpSheet, uint invId, uint slotIndex) const;

	void			updateItemResult();
};


// ***************************************************************************
// Called when click a Faber phrase
extern void		launchFaberCastWindow(sint32 memoryLine, uint memoryIndex, CSBrickSheet *rootBrick);
// Called when select a Faber plan
extern void		fillFaberPlanSelection(const std::string &brickDB, uint maxSelection);
// Called when the Faber plan is selected
extern void		validateFaberPlanSelection(CSBrickSheet *itemPlanBrick);
// Called when something needs to close the crafting window (does nothing if not open)
extern void		closeFaberCastWindow();

#endif // NL_ACTION_PHRASE_FABER_H

/* End of action_phrase_faber.h */
