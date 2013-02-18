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



#include "stdpch.h"
#include "action_phrase_faber.h"
#include "../client_sheets/sbrick_sheet.h"
#include "interface_manager.h"
#include "../sheet_manager.h"
#include "inventory_manager.h"
#include "nel/gui/action_handler.h"
#include "../client_cfg.h"
#include "nel/gui/ctrl_base_button.h"
#include "nel/gui/group_container.h"
#include "../string_manager_client.h"
#include "../net_manager.h"
#include "sbrick_manager.h"
#include "sphrase_manager.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/dbview_bar.h"
#include "skill_manager.h"
#include "game_share/bot_chat_types.h"

using namespace std;
using namespace NLMISC;


// ***************************************************************************
const std::string	FaberPlanDB= "UI:PHRASE:FABER:FABER_PLAN:SHEET";
const std::string	MPFaberDB= "UI:PHRASE:FABER:MP_BUILD";
const std::string	MPSelectionDB= "UI:PHRASE:FABER:MP_SELECT";
const std::string	MPQuantityDb= "UI:PHRASE:FABER:MP_QUANTITY";
const std::string	MPQuantitySelectDb= "UI:PHRASE:FABER:STACK_SELECT";
const std::string	ItemResultSheetDB= "UI:PHRASE:FABER:RESULT_ITEM:SHEET";
const std::string	ItemResultQuantityDB= "UI:PHRASE:FABER:RESULT_ITEM:QUANTITY";
const std::string	ItemResultSheetLevel= "UI:PHRASE:FABER:RESULT_ITEM:QUALITY";
const std::string	ItemResultSheetColor= "UI:PHRASE:FABER:RESULT_ITEM:USER_COLOR";
const std::string	ItemResultSheetClassType= "UI:PHRASE:FABER:RESULT_ITEM:RM_CLASS_TYPE";
const std::string	ItemResultSheetStatType= "UI:PHRASE:FABER:RESULT_ITEM:RM_FABER_STAT_TYPE";
const std::string	FaberPhraseWindow= "ui:interface:phrase_faber_execution";
const std::string	FaberPhraseItemReqLine= FaberPhraseWindow + ":header_opened:item_reqs:item_req_%d";
const std::string	FaberPhraseList= "list";
const std::string	FaberPhraseText= "text";
const std::string	FaberPhraseIcon= "icon";
const std::string	FaberPhraseValidButton= FaberPhraseWindow + ":header_opened:ok_cancel:ok";
const std::string	FaberPhraseFpCtrl= FaberPhraseWindow + ":header_opened:faber_plan";
const std::string	FaberPhraseFpSuccessText= FaberPhraseWindow + ":header_opened:success_text";
const std::string	FaberPhraseMpListModal= "ui:interface:phrase_faber_mp_selection";
const std::string	FaberPhraseMpQuantityModal= "ui:interface:phrase_faber_mp_quantity";
const std::string	FaberPhraseItemResultGroup= FaberPhraseWindow + ":header_opened:item_result";


#define MAX_MP_SELECTION_ENTRIES	256

// ***************************************************************************
CActionPhraseFaber::CActionPhraseFaber()
{
	uint size = MAX_PLAYER_INV_ENTRIES + (MAX_ANIMALINV_ENTRIES * MAX_INVENTORY_ANIMAL) +
		MAX_GUILDINV_ENTRIES + MAX_ROOMINV_ENTRIES;
	_InventoryMirror.resize(size);
	_InventoryObsSetup= false;
	_ExecuteFromItemPlanBrick= NULL;
}


// ***************************************************************************
void			CActionPhraseFaber::fillDBWithMP(const std::string &sheetBase, const CItem &item)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	NLGUI::CDBManager::getInstance()->getDbProp(sheetBase + ":SHEET")->setValue32(item.Sheet.asInt());
	NLGUI::CDBManager::getInstance()->getDbProp(sheetBase + ":QUALITY")->setValue32(item.Quality);
	NLGUI::CDBManager::getInstance()->getDbProp(sheetBase + ":QUANTITY")->setValue32(item.Quantity);
	NLGUI::CDBManager::getInstance()->getDbProp(sheetBase + ":USER_COLOR")->setValue32(item.UserColor);
	NLGUI::CDBManager::getInstance()->getDbProp(sheetBase + ":WEIGHT")->setValue32(item.Weight);
}


// ***************************************************************************
void		CActionPhraseFaber::launchFaberCastWindow(sint32 memoryLine, uint memoryIndex, CSBrickSheet *rootBrick)
{
	// **** Get the ItemSheet for faber plan. NULL => no op.
	if(!rootBrick)
		return;
	// Copy Execution launch
	_ExecuteFromMemoryLine= memoryLine;
	_ExecuteFromMemoryIndex= memoryIndex;
	// no item plan setuped for now
	_ExecuteFromItemPlanBrick= NULL;


	// get the family of item plan (for selection) from the rootBrick. It is stored in the Property0.
	_FaberPlanBrickFamilies.clear();
	if(rootBrick->Properties.size()>0)
	{
		string prop= NLMISC::toUpper(rootBrick->Properties[0].Text);
		vector<string>	strList;
		splitString(prop, " ", strList);
		// The prop Id should be 'FPLAN:'
		if(strList.size()>=2 && strList[0]=="FPLAN:")
		{
			for(uint i=1;i<strList.size();i++)
			{
				BRICK_FAMILIES::TBrickFamily	bfam= BRICK_FAMILIES::toSBrickFamily(strList[i]);
				if(bfam!=BRICK_FAMILIES::Unknown)
					_FaberPlanBrickFamilies.push_back(bfam);
			}
		}
	}
	// if not found, error, cannot choose the faber plan
	if(_FaberPlanBrickFamilies.empty())
	{
		nlwarning("ERROR: The Craft Root %s does not contain a valid FPLAN: property -> Can't select plan to craft",
			rootBrick->Id.toString().c_str() );
		return;
	}


	// **** Hide all widgets, MP Ctrls, and reset DB, until the Plan is not selected
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	// Hide the valid button
	CCtrlBaseButton	*validButton= dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseValidButton));
	if(validButton)
		validButton->setFrozen(true);

	// reset DB, hide the Mps
	uint	itemReqLine;
	for(itemReqLine=0;itemReqLine<MAX_ITEM_REQ_LINE;itemReqLine++)
	{
		// Reset All Mps slots.
		for(uint mpSlot=0;mpSlot<MAX_MP_SLOT;mpSlot++)
		{
			CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:%d:SHEET", MPFaberDB.c_str(), itemReqLine, mpSlot), false);
			if(node)
				node->setValue32(0);
		}

		// Hide item requirements groups per default
		CInterfaceGroup		*itemReqLineGroup= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId( toString(FaberPhraseItemReqLine.c_str(), itemReqLine) ));
		if(itemReqLineGroup)
			itemReqLineGroup->setActive(false);
	}

	// Reset the selected plan
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(FaberPlanDB, false);
	if(node)
		node->setValue32(0);

	// Reset the result item
	node= NLGUI::CDBManager::getInstance()->getDbProp(ItemResultSheetDB, false);
	if(node)
		node->setValue32(0);

	// Hide the ItemResult group
	CInterfaceGroup		*groupMp= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseItemResultGroup));
	if(groupMp)
		groupMp->setActive(false);


	// **** Open the window!
	CGroupContainer		*window= dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseWindow));
	if(window)
	{
		window->setActive(true);

		// Setup the Title with a default text
		ucstring	title= CI18N::get("uiPhraseFaberExecuteNoPlan");
		window->setUCTitle (title);
	}

	// **** setup DB observer!
	// ensure remove (if setuped before), then add
	CCDBNodeBranch	*branch;
	branch= NLGUI::CDBManager::getInstance()->getDbBranch("LOCAL:INVENTORY:BAG");
	if(branch) NLGUI::CDBManager::getInstance()->removeBranchObserver( "LOCAL:INVENTORY:BAG",&_DBInventoryObs);
	if(branch) NLGUI::CDBManager::getInstance()->addBranchObserver( "LOCAL:INVENTORY:BAG",&_DBInventoryObs);

	// and for all pack animals
	uint	i;
	for(i=0;i<MAX_INVENTORY_ANIMAL;i++)
	{
		branch= NLGUI::CDBManager::getInstance()->getDbBranch(toString("LOCAL:INVENTORY:PACK_ANIMAL%d", i));
		if(branch) NLGUI::CDBManager::getInstance()->removeBranchObserver( toString("LOCAL:INVENTORY:PACK_ANIMAL%d", i).c_str(), &_DBInventoryObs);
		if(branch) NLGUI::CDBManager::getInstance()->addBranchObserver( toString("LOCAL:INVENTORY:PACK_ANIMAL%d", i).c_str(), &_DBInventoryObs);
	}

	// Add observers on animal status, cause inventory may become unavailabe during the process
	for(i=0;i<MAX_INVENTORY_ANIMAL;i++)
	{
		node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:STATUS",i), false);
		if(node)
		{
			ICDBNode::CTextId textId;
			node->addObserver(&_DBAnimalObs, textId);
		}
	}

	// Observe skill status change to update success rate
	CSkillManager       *pSM= CSkillManager::getInstance();
	pSM->appendSkillChangeCallback(&_SkillObserver);
}


// ***************************************************************************
void			CActionPhraseFaber::onCloseFaberCastWindow()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
    CSkillManager       *pSM= CSkillManager::getInstance();

	// No more need to listen inventory change
	CCDBNodeBranch	*branch;
	branch= NLGUI::CDBManager::getInstance()->getDbBranch("LOCAL:INVENTORY:BAG");
	if(branch) branch->removeBranchObserver(&_DBInventoryObs);
	// and for all pack animals
	for(uint i=0;i<MAX_INVENTORY_ANIMAL;i++)
	{
		branch= NLGUI::CDBManager::getInstance()->getDbBranch(toString("LOCAL:INVENTORY:PACK_ANIMAL%d", i));
		if(branch) branch->removeBranchObserver(&_DBInventoryObs);
	}

	// remove observers on animal status, cause inventory may become unavailabe during the process
	for(uint i=0;i<MAX_INVENTORY_ANIMAL;i++)
	{
		CCDBNodeLeaf *node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:STATUS",i), false);
		if(node)
		{
			ICDBNode::CTextId textId;
			node->removeObserver(&_DBAnimalObs, textId);
		}
	}

	pSM->removeSkillChangeCallback(&_SkillObserver);
}


// ***************************************************************************
void			CActionPhraseFaber::fillFaberPlanSelection(const std::string &brickDB, uint maxSelection)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	// fill selection with all bricks of the same family
	uint	i;
	std::vector<CSheetId>		bricks;
	for(i=0;i<_FaberPlanBrickFamilies.size();i++)
	{
		const std::vector<NLMISC::CSheetId>		&famBricks= pBM->getFamilyBricks(_FaberPlanBrickFamilies[i]);
		bricks.insert(bricks.end(), famBricks.begin(), famBricks.end());
	}

	// get only ones known
	pBM->filterKnownBricks(bricks);

	// fill db
	uint	num= min(maxSelection, uint(bricks.size()));
	for(i=0;i<maxSelection;i++)
	{
		if(i<num)
			NLGUI::CDBManager::getInstance()->getDbProp(brickDB + ":" + toString(i) + ":SHEET")->setValue32(bricks[i].asInt());
		else
			NLGUI::CDBManager::getInstance()->getDbProp(brickDB + ":" + toString(i) + ":SHEET")->setValue32(0);
	}
}

// ***************************************************************************
CItemImage		*CActionPhraseFaber::getInvMirrorItemImage(uint slotIndex, uint& invId, uint& indexInInv)
{
	if (slotIndex < MAX_PLAYER_INV_ENTRIES)
	{
		invId = INVENTORIES::bag;
		indexInInv = slotIndex;
		return &getInventory().getBagItem(slotIndex);
	}
	slotIndex -= MAX_PLAYER_INV_ENTRIES;

	if (slotIndex < (MAX_ANIMALINV_ENTRIES * MAX_INVENTORY_ANIMAL))
	{
		uint animal = slotIndex / MAX_ANIMALINV_ENTRIES;
		uint index = slotIndex % MAX_ANIMALINV_ENTRIES;
		invId = INVENTORIES::pet_animal + animal;
		indexInInv = index;
		return &getInventory().getPAItem(animal, index);
	}
	slotIndex -= (MAX_ANIMALINV_ENTRIES * MAX_INVENTORY_ANIMAL);

	if (slotIndex < MAX_GUILDINV_ENTRIES)
	{
		if (getInventory().isInventoryAvailable(INVENTORIES::guild))
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			CCDBNodeBranch *itemBranch = NLGUI::CDBManager::getInstance()->getDbBranch(SERVER_INVENTORY ":GUILD:" + toString(slotIndex));
			static CItemImage image;
			image.build(itemBranch);
			invId = INVENTORIES::guild;
			indexInInv = slotIndex;
			return &image;
		}
		return NULL;
	}
	slotIndex -= MAX_GUILDINV_ENTRIES;

	if (slotIndex < MAX_ROOMINV_ENTRIES)
	{
		if (getInventory().isInventoryAvailable(INVENTORIES::player_room))
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			CCDBNodeBranch *itemBranch = NLGUI::CDBManager::getInstance()->getDbBranch(SERVER_INVENTORY ":ROOM:" + toString(slotIndex));
			static CItemImage image;
			image.build(itemBranch);
			invId = INVENTORIES::player_room;
			indexInInv = slotIndex;
			return &image;
		}
		return NULL;
	}

	return NULL;
}


// ***************************************************************************
bool			CActionPhraseFaber::isMpAvailable(CItemSheet *mpSheet, uint invId, uint slotIndex) const
{
	return mpSheet && mpSheet->Family==ITEMFAMILY::RAW_MATERIAL && getInventory().isInventoryAvailable((INVENTORIES::TInventory)invId);
}

// ***************************************************************************
void		CActionPhraseFaber::validateFaberPlanSelection(CSBrickSheet *itemPlanBrick)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();


	// **** Get the ItemSheet for faber plan. NULL => no op.
	if(!itemPlanBrick)
		return;
	_ExecuteFromItemPlanBrick= itemPlanBrick;


	// TestYoyo
	/*for(uint tam=0;tam<_ExecuteFromItemPlanBrick->FaberPlan.ItemPartMps.size();tam++)
	{
		_ExecuteFromItemPlanBrick->FaberPlan.ItemPartMps[tam].Quantity= 20;
	}
	_ExecuteFromItemPlanBrick->FaberPlan.FormulaMps.resize(2);
	_ExecuteFromItemPlanBrick->FaberPlan.FormulaMps[0].ItemRequired= CSheetId("m0152chdca01.sitem");
	_ExecuteFromItemPlanBrick->FaberPlan.FormulaMps[0].Quantity= 13;
	_ExecuteFromItemPlanBrick->FaberPlan.FormulaMps[1].ItemRequired= CSheetId("m0691chdca01.sitem");
	_ExecuteFromItemPlanBrick->FaberPlan.FormulaMps[1].Quantity= 25;
	*/


	// the num of itempPart/specific items to setup
	_MPBuildNumItemPartReq= min((uint)MAX_ITEM_REQ_LINE, (uint)_ExecuteFromItemPlanBrick->FaberPlan.ItemPartMps.size());
	_MPBuildNumSpecificItemReq= min(((uint)MAX_ITEM_REQ_LINE-_MPBuildNumItemPartReq), (uint)_ExecuteFromItemPlanBrick->FaberPlan.FormulaMps.size());
	_MPBuildNumTotalItemReq= _MPBuildNumItemPartReq + _MPBuildNumSpecificItemReq;


	// Setup the selected plan
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(FaberPlanDB, false);
	if(node)
		node->setValue32(_ExecuteFromItemPlanBrick->Id.asInt());

	// Setup the result item
	node= NLGUI::CDBManager::getInstance()->getDbProp(ItemResultSheetDB, false);
	if(node)
		node->setValue32(itemPlanBrick->FaberPlan.ItemBuilt.asInt());

	// Setup the result quantity (for stacked items)
	node= NLGUI::CDBManager::getInstance()->getDbProp(ItemResultQuantityDB, false);
	if(node)
		node->setValue32(itemPlanBrick->FaberPlan.NbItemBuilt);

	// Show the ItemResult group
	CInterfaceGroup		*groupMp= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseItemResultGroup));
	if(groupMp)
		groupMp->setActive(true);


	// **** reset the mpBuild
	// For all item required.
	uint	itemReqLine;
	for(itemReqLine=0;itemReqLine<_MPBuildNumTotalItemReq;itemReqLine++)
	{
		CMPBuild	&mpBuild= _MPBuild[itemReqLine];

		// Type of requirement?
		// First go the ItemPart reqs
		if(itemReqLine<_MPBuildNumItemPartReq)
		{
			uint	itemPartId= itemReqLine;
			mpBuild.RequirementType= CMPBuild::ItemPartReq;
			mpBuild.FaberTypeRequired= _ExecuteFromItemPlanBrick->FaberPlan.ItemPartMps[itemPartId].FaberTypeFilter;
			mpBuild.QuantityReq= _ExecuteFromItemPlanBrick->FaberPlan.ItemPartMps[itemPartId].Quantity;
		}
		// Then go the Specific item reqs
		else
		{
			uint	itemSpecificId= itemReqLine - _MPBuildNumItemPartReq;
			mpBuild.RequirementType= CMPBuild::SpecificItemReq;
			mpBuild.SpecificItemRequired= _ExecuteFromItemPlanBrick->FaberPlan.FormulaMps[itemSpecificId].ItemRequired;
			mpBuild.QuantityReq= _ExecuteFromItemPlanBrick->FaberPlan.FormulaMps[itemSpecificId].Quantity;
		}

		// Reset the quantity setuped for this line
		mpBuild.NumMpSlot= 0;
		for(uint mpSlot=0;mpSlot<MAX_MP_SLOT;mpSlot++)
		{
			mpBuild.Id[mpSlot]= 0;	// useless, but for consistency
			mpBuild.QuantitySelected[mpSlot]= 0;
		}
	}
	// reset other to 0 also
	for(;itemReqLine<MAX_ITEM_REQ_LINE;itemReqLine++)
	{
		_MPBuild[itemReqLine].reset();
	}


	// **** First clear and copy the inventory to local struct
	uint	i;
	for(i=0;i<_InventoryMirror.size();i++)
	{
		_InventoryMirror[i].reset();
	}

	uint		invId = 0;
	uint		indexInInv = 0;
	// Run all the inventories.
	for(i=0;i<_InventoryMirror.size();i++)
	{
		CItemImage	*itemImage= getInvMirrorItemImage(i, invId, indexInInv);
		bool bLockedByOwner = itemImage && itemImage->getLockedByOwner();
		// item found and not locked?
		if(itemImage)
		{
			// setup the origin
			_InventoryMirror[i].InventoryId= invId;
			_InventoryMirror[i].IdInInventory= indexInInv;

			// The item must be a mp
			CSheetId	sheetId= CSheetId(itemImage->getSheetID());
			CItemSheet	*mpSheet= dynamic_cast<CItemSheet*>(SheetMngr.get(sheetId));
			if( isMpAvailable(mpSheet, invId, i) && !bLockedByOwner)
			{
				_InventoryMirror[i].Sheet= sheetId;
				_InventoryMirror[i].Quality= itemImage->getQuality();
				_InventoryMirror[i].Quantity= itemImage->getQuantity();
				_InventoryMirror[i].UserColor= itemImage->getUserColor();
				_InventoryMirror[i].Weight= itemImage->getWeight();
				// Bkup original quantity from inventory
				_InventoryMirror[i].OriginalQuantity= _InventoryMirror[i].Quantity;
				_InventoryMirror[i].LockedByOwner= bLockedByOwner;
			}
		}
	}


	// **** show ItemParts according to plan.
	// Hide the valid button
	CCtrlBaseButton	*validButton= dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseValidButton));
	if(validButton)
		validButton->setFrozen(true);

	// reset DB, show/hide the Mps
	for(itemReqLine=0;itemReqLine<MAX_ITEM_REQ_LINE;itemReqLine++)
	{
		CMPBuild	&mpBuild= _MPBuild[itemReqLine];

		// Reset All Mps slots.
		for(uint mpSlot=0;mpSlot<MAX_MP_SLOT;mpSlot++)
		{
			CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:%d:SHEET", MPFaberDB.c_str(), itemReqLine, mpSlot), false);
			if(node)
				node->setValue32(0);
		}

		// Setup item requirement groups
		CInterfaceGroup		*itemReqLineGroup= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId( toString(FaberPhraseItemReqLine.c_str(), itemReqLine) ));
		if(itemReqLineGroup)
		{
			if( itemReqLine<_MPBuildNumTotalItemReq )
			{
				itemReqLineGroup->setActive(true);

				// Set as Text the required MP FaberType or Specific item
				CViewText	*viewText= dynamic_cast<CViewText*>(itemReqLineGroup->getView(FaberPhraseText));
				if(viewText)
				{
					ucstring	text;
					if(mpBuild.RequirementType==CMPBuild::ItemPartReq)
					{
						text= CI18N::get("uihelpFaberMpHeader");
						strFindReplace(text, "%f", RM_FABER_TYPE::toLocalString(mpBuild.FaberTypeRequired) );
					}
					else if(mpBuild.RequirementType==CMPBuild::SpecificItemReq)
					{
						text= STRING_MANAGER::CStringManagerClient::getItemLocalizedName(mpBuild.SpecificItemRequired);
					}
					else
					{
						nlstop;
					}
					viewText->setText( text );
				}

				// Set as Icon the required MP FaberType / or Sheet Texture (directly...)
				CViewBitmap	*viewBmp= dynamic_cast<CViewBitmap*>(itemReqLineGroup->getView(FaberPhraseIcon));
				if(viewBmp)
				{
					if(mpBuild.RequirementType==CMPBuild::ItemPartReq)
					{
						// texture name in config.xml
						viewBmp->setTexture(CWidgetManager::getInstance()->getParser()->getDefine( RM_FABER_TYPE::toIconDefineString(mpBuild.FaberTypeRequired) ));
					}
					else if(mpBuild.RequirementType==CMPBuild::SpecificItemReq)
					{
						// NB: the texture is scaled, so it's ok to put the item 40x40 texture
						const CItemSheet	*itemSheet= dynamic_cast<const CItemSheet*>(SheetMngr.get(mpBuild.SpecificItemRequired));
						if(itemSheet)
							viewBmp->setTexture(itemSheet->getIconMain());
						else
							viewBmp->setTexture(std::string());
					}
					else
					{
						nlstop;
					}
				}

				// update the EmptySlot
				updateEmptySlot(itemReqLine, itemReqLineGroup);

				// setup item required quantity view
				updateQuantityView(itemReqLine);
			}
			else
			{
				itemReqLineGroup->setActive(false);
			}
		}

	}

	// **** Setup the new window title
	CGroupContainer		*window= dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseWindow));
	if(window)
	{
		// Setup the Title with the item built
		ucstring	title= CI18N::get("uiPhraseFaberExecute");
		strFindReplace(title, "%item", STRING_MANAGER::CStringManagerClient::getItemLocalizedName(_ExecuteFromItemPlanBrick->FaberPlan.ItemBuilt) );
		window->setUCTitle (title);
	}


	// **** result view
	updateItemResult();
}

// ***************************************************************************
void			CActionPhraseFaber::resetSelection()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	for(uint i=0;i<MAX_MP_SELECTION_ENTRIES;i++)
	{
		NLGUI::CDBManager::getInstance()->getDbProp(MPSelectionDB+ ":" + toString(i) + ":SHEET")->setValue32(0);
	}
}

// ***************************************************************************
void			CActionPhraseFaber::fillSelection(const std::vector<uint> &mps)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	uint	num= min(uint(MAX_MP_SELECTION_ENTRIES), uint(mps.size()));
	for(uint i=0;i<MAX_MP_SELECTION_ENTRIES;i++)
	{
		if(i<num && mps[i]<_InventoryMirror.size())
		{
			CItem	&item= _InventoryMirror[mps[i]];
			fillDBWithMP(MPSelectionDB+ ":" + toString(i), item);
		}
		else
			NLGUI::CDBManager::getInstance()->getDbProp(MPSelectionDB+ ":" + toString(i) + ":SHEET")->setValue32(0);
	}
}


// ***************************************************************************
void			CActionPhraseFaber::filterSelectionItemPart(std::vector<uint> &mps, RM_FABER_TYPE::TRMFType itemPartFilter, ITEM_ORIGIN::EItemOrigin originFilter)
{
	// Unknown => no fitler
	if(itemPartFilter==RM_FABER_TYPE::Unknown)
		return;

	std::vector<uint>	res;
	res.reserve(mps.size());

	for(uint i=0;i<mps.size();i++)
	{
		// get the item sheet
		const CItemSheet	*itemSheet= dynamic_cast<const CItemSheet*>(SheetMngr.get(_InventoryMirror[mps[i]].Sheet));
		// test itemPartFilter match.
		if(itemSheet)
		{
			if(itemSheet->canBuildItemPart(itemPartFilter, originFilter))
			{
				res.push_back(mps[i]);
			}
		}
	}

	mps= res;
}


// ***************************************************************************
void			CActionPhraseFaber::filterSelectionItemSpecific(std::vector<uint> &mps, NLMISC::CSheetId specificItemWanted)
{
	std::vector<uint>	res;
	res.reserve(mps.size());

	// if unknown sheetid, no match
	if(specificItemWanted==NLMISC::CSheetId::Unknown)
	{
		mps.clear();
		return;
	}

	for(uint i=0;i<mps.size();i++)
	{
		// get the item sheet
		const CItemSheet	*itemSheet= dynamic_cast<const CItemSheet*>(SheetMngr.get(_InventoryMirror[mps[i]].Sheet));
		// test sheetid match.
		if(itemSheet)
		{
			if(itemSheet->Id == specificItemWanted)
			{
				res.push_back(mps[i]);
			}
		}
	}

	mps= res;
}


// ***************************************************************************
void		CActionPhraseFaber::startMpSelection(uint itemReqLine, uint mpSlot)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// get the ctrlSlot
	CDBCtrlSheet		*ctrlSlot= NULL;
	CInterfaceGroup		*itemReqLineGroup= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId( toString(FaberPhraseItemReqLine.c_str(), itemReqLine) ));
	if(itemReqLineGroup)
	{
		CDBGroupListSheet	*listSheet= dynamic_cast<CDBGroupListSheet*>(itemReqLineGroup->getGroup(FaberPhraseList));
		if(listSheet)
			ctrlSlot= listSheet->getSheet(mpSlot);
	}
	if(!ctrlSlot)
		return;

	// get the mpBuild setup
	nlassert(itemReqLine<MAX_ITEM_REQ_LINE);
	CMPBuild	&mpBuild= _MPBuild[itemReqLine];

	// If the slot selected is already filled, Launch the MP Quantity selection modal
	if(mpSlot<mpBuild.NumMpSlot)
	{
		// fill the sheet info
		uint	invSlot= mpBuild.Id[mpSlot];
		CItem	item= _InventoryMirror[invSlot];
		fillDBWithMP(MPQuantitySelectDb, item);

		// compute the maximum quantity possible to fill
		uint	maxQuantity= getMaxQuantityChange(itemReqLine, mpSlot);

		// set the max quantity as the default quantity to set up.
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(MPQuantitySelectDb + ":CUR_QUANTITY",  false);
		if(node)	node->setValue32(maxQuantity);
		node= NLGUI::CDBManager::getInstance()->getDbProp(MPQuantitySelectDb + ":MAX_QUANTITY",  false);
		if(node)	node->setValue32(maxQuantity);

		// bkup for validation
		_MpSelectionItemReqLine= itemReqLine;
		_MpChangeQuantitySlot= mpSlot;

		// Setup the text with value by default
		CInterfaceGroup		*quantityModal= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseMpQuantityModal));
		if(quantityModal)
		{
			CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(quantityModal->getGroup("eb"));
			if (eb)
			{
				CWidgetManager::getInstance()->setCaptureKeyboard(eb);
				eb->setInputString(toString(maxQuantity));
				eb->setSelectionAll();
			}
		}

		// launch the modal
		CWidgetManager::getInstance()->enableModalWindow(ctrlSlot, quantityModal);
	}
	// else select new MP
	else
	{
		// For All the inventory
		vector<uint>	selectMps;
		for(uint i=0;i<_InventoryMirror.size();i++)
		{
			// If still some MP on this stack, and if not already selected, add to selection
			if(_InventoryMirror[i].Quantity>0 && (_InventoryMirror[i].Selected&(1<<itemReqLine))==0 )
			{
				selectMps.push_back(i);
			}
		}

		// Filter the selection whether it is an itemPart or specificItem reqiurement
		if(mpBuild.RequirementType==CMPBuild::ItemPartReq)
		{
			CItemSheet	*itemBuilt= dynamic_cast<CItemSheet*>(SheetMngr.get(_ExecuteFromItemPlanBrick->FaberPlan.ItemBuilt));
			ITEM_ORIGIN::EItemOrigin	itemOrigin= itemBuilt? itemBuilt->ItemOrigin : ITEM_ORIGIN::UNKNOWN;
			filterSelectionItemPart(selectMps, mpBuild.FaberTypeRequired, itemOrigin);
		}
		else if(mpBuild.RequirementType==CMPBuild::SpecificItemReq)
		{
			filterSelectionItemSpecific(selectMps, mpBuild.SpecificItemRequired);
		}
		else
		{
			nlstop;
		}

		// Reset the DB selection
		resetSelection();
		fillSelection(selectMps);

		// Bkup Selection for Validate later
		_MpSelectionItemReqLine= itemReqLine;
		_MpCurrentSelection= selectMps;

		// Open the Selection Window.
		CWidgetManager::getInstance()->enableModalWindow(ctrlSlot, FaberPhraseMpListModal);
	}
}

// ***************************************************************************
void		CActionPhraseFaber::validateMpSelection(uint selectId)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if(selectId>=_MpCurrentSelection.size())
	{
		CWidgetManager::getInstance()->disableModalWindow();
		return;
	}

	// get which MP of the inventory we have selected
	uint	newInvSlot= _MpCurrentSelection[selectId];

	// get the build
	uint	itemReqLine= _MpSelectionItemReqLine;
	CMPBuild	&mpBuild= _MPBuild[itemReqLine];

	// Select the quantity to peek from this inventory slot: get max possible
	sint	quantity= mpBuild.QuantityReq - getTotalQuantitySetuped(itemReqLine);
	nlassert(quantity>0);
	quantity= min((sint32)quantity, _InventoryMirror[newInvSlot].Quantity);

	// it may be possible (by update DB and error) that selected slot is no more usable => just quit
	if(quantity<=0)
		return;

	// And Remove (virtually) item stack from this slot
	_InventoryMirror[newInvSlot].Quantity-= quantity;
	// mark as selected for this itemReqLine, so can no more select it
	_InventoryMirror[newInvSlot].Selected|= 1<<itemReqLine;

	// update the build
	nlassert(mpBuild.NumMpSlot<MAX_MP_SLOT);
	mpBuild.Id[mpBuild.NumMpSlot]= newInvSlot;
	mpBuild.QuantitySelected[mpBuild.NumMpSlot]= quantity;
	mpBuild.NumMpSlot++;

	// Update The Execution View
	CItem	item= _InventoryMirror[newInvSlot];
	item.Quantity= quantity;
	fillDBWithMP(toString("%s:%d:%d", MPFaberDB.c_str(), itemReqLine, mpBuild.NumMpSlot-1), item);

	// update the empty slot
	updateEmptySlot(itemReqLine);

	// update quantity view
	updateQuantityView(itemReqLine);

	// update the validateButton
	updateValidButton();

	// update the item result
	updateItemResult();

	// must hide the modal window which had open us. NB: must be done here because next,
	// we'll open the MP quantity selection
	CWidgetManager::getInstance()->disableModalWindow();

	// **** when all is correctly ended, open the quantity selection
	// NB: just enable this code, if you want this feature
	//startMpSelection(itemReqLine, mpBuild.NumMpSlot-1);
}

// ***************************************************************************
void		CActionPhraseFaber::validateMpSelectQuantity()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// get current execution context of the validate
	uint	itemReqLine= _MpSelectionItemReqLine;
	nlassert(itemReqLine<MAX_ITEM_REQ_LINE);
	CMPBuild	&mpBuild= _MPBuild[itemReqLine];
	uint	mpSlot= _MpChangeQuantitySlot;
	nlassert(mpSlot<mpBuild.NumMpSlot);
	uint	invSlot= mpBuild.Id[mpSlot];
	nlassert(invSlot<_InventoryMirror.size());
	nlassert(_InventoryMirror[invSlot].Selected & (1<<itemReqLine));

	// get the quantity selected
	uint	quantitySelected= 0;
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(MPQuantitySelectDb + ":CUR_QUANTITY",  false);
	if(node)	quantitySelected= node->getValue32();

	// maximize (if error)
	quantitySelected= min(quantitySelected, getMaxQuantityChange(itemReqLine, mpSlot));

	// if the new quantity is 0
	if(quantitySelected==0)
	{
		// special: remove the mp slot from list
		deleteMpSlot(itemReqLine, mpSlot);
	}
	else
	{
		// restore old quantity into inventory
		_InventoryMirror[invSlot].Quantity+= mpBuild.QuantitySelected[mpSlot];
		// And then Remove (virtually) new item stack from this slot
		_InventoryMirror[invSlot].Quantity-= quantitySelected;

		// update the build
		mpBuild.QuantitySelected[mpSlot]= quantitySelected;

		// Update The Execution View
		CItem	item= _InventoryMirror[invSlot];
		item.Quantity= quantitySelected;
		fillDBWithMP(toString("%s:%d:%d", MPFaberDB.c_str(), itemReqLine, mpSlot), item);
	}

	// update the empty slot
	updateEmptySlot(itemReqLine);

	// update quantity view
	updateQuantityView(itemReqLine);

	// update the valid button
	updateValidButton();

	// update the item result
	updateItemResult();

	// hide the Modal Quantity selection
	CWidgetManager::getInstance()->disableModalWindow();
}

// ***************************************************************************
void		CActionPhraseFaber::validateExecution()
{
	// the plan has must been selected
	nlassert(_ExecuteFromItemPlanBrick);

	// Build the list of MP in Bag.
	vector<CFaberMsgItem>	mpItemPartList;
	vector<CFaberMsgItem>	specificItemList;

	// Run all the current Build execution
	for(uint itemReqLine=0;itemReqLine<_MPBuildNumTotalItemReq;itemReqLine++)
	{
		CMPBuild	&mpBuild= _MPBuild[itemReqLine];
		// For all slot setuped.
		for(uint mpSlot=0;mpSlot<mpBuild.NumMpSlot;mpSlot++)
		{
			CFaberMsgItem	item;

			uint	invSlot= mpBuild.Id[mpSlot];
			nlassert(invSlot<_InventoryMirror.size());
			item.setInvId(INVENTORIES::TInventory(_InventoryMirror[invSlot].InventoryId));
			item.IndexInInv= _InventoryMirror[invSlot].IdInInventory;
			item.Quantity= mpBuild.QuantitySelected[mpSlot];

			if(mpBuild.RequirementType==CMPBuild::ItemPartReq)
				mpItemPartList.push_back(item);
			else if(mpBuild.RequirementType==CMPBuild::SpecificItemReq)
				specificItemList.push_back(item);
			else
			{
				nlstop;
			}
		}
	}

	// display next craft action, and Send message to server
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();
	pPM->executeCraft(_ExecuteFromMemoryLine, _ExecuteFromMemoryIndex,
		_ExecuteFromItemPlanBrick->Id.asInt(), mpItemPartList, specificItemList);

	// Open the Interface to get the crafted item
	CTempInvManager::getInstance()->open(TEMP_INV_MODE::Craft);

	// NO more Close the Execution window (allow refaber quick)
	/*CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CInterfaceElement	*window= CWidgetManager::getInstance()->getElementFromId(FaberPhraseWindow);
	if(window)
		window->setActive(false);
	*/

}


// ***************************************************************************
uint			CActionPhraseFaber::getTotalQuantitySetuped(uint itemReqLine) const
{
	nlassert(itemReqLine<MAX_ITEM_REQ_LINE);
	nlassert(_MPBuild[itemReqLine].NumMpSlot<=MAX_MP_SLOT);

	uint	ret= 0;
	for(uint i=0;i<_MPBuild[itemReqLine].NumMpSlot;i++)
	{
		ret+= _MPBuild[itemReqLine].QuantitySelected[i];
	}

	return ret;
}


// ***************************************************************************
uint			CActionPhraseFaber::getMaxQuantityChange(uint itemReqLine, uint mpSlot) const
{
	nlassert(itemReqLine<MAX_ITEM_REQ_LINE);
	nlassert(mpSlot<_MPBuild[itemReqLine].NumMpSlot);

	uint	invSlot= _MPBuild[itemReqLine].Id[mpSlot];
	nlassert(invSlot<_InventoryMirror.size());
	CItem	item= _InventoryMirror[invSlot];

	// This is the quantity already selected for this itemReqLine, + rest in inventory
	uint	maxQuantity= _MPBuild[itemReqLine].QuantitySelected[mpSlot] + item.Quantity;
	// maximize with the rest of quantity with have to setup (remove us btw)
	maxQuantity= min(maxQuantity, _MPBuild[itemReqLine].QuantityReq -
		(getTotalQuantitySetuped(itemReqLine) - _MPBuild[itemReqLine].QuantitySelected[mpSlot]) );

	return maxQuantity;
}


// ***************************************************************************
void			CActionPhraseFaber::updateEmptySlot(uint itemReqLine, CInterfaceGroup *itemReqLineGroup)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if(!itemReqLineGroup)
		itemReqLineGroup= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId( toString(FaberPhraseItemReqLine.c_str(), itemReqLine) ));
	if(!itemReqLineGroup)
		return;

	// get the list sheet and ctrlButton.
	CDBGroupListSheet	*listSheet= dynamic_cast<CDBGroupListSheet*>(itemReqLineGroup->getGroup(FaberPhraseList));
	if(!listSheet)
		return;

	// NB: forceValidity calls invalidateCoords() if state change => dont "clear then set".

	// All Setuped?
	bool	allSetuped= getTotalQuantitySetuped(itemReqLine) >= _MPBuild[itemReqLine].QuantityReq;

	// button no more needed?
	if(allSetuped)
	{
		// Reset all ForceValid
		for(uint i=0;i<MAX_MP_SLOT;i++)
			listSheet->forceValidity(i, false);
	}
	else
	{
		// Reset all ForceValid
		for(uint i=0;i<MAX_MP_SLOT;i++)
		{
			listSheet->forceValidity(i, i==_MPBuild[itemReqLine].NumMpSlot);
		}
	}

	// Special for Specific Item requirement. Setup grayed item for the last empty slot
	for(uint i=0;i<MAX_MP_SLOT;i++)
	{
		CMPBuild	&mpBuild= _MPBuild[itemReqLine];

		// *** Fill the empty DB.
		if(i>=mpBuild.NumMpSlot)
		{
			CItem	item;
			// If Specfific requirement and just the last one, don't leave empty
			if(!allSetuped && i==mpBuild.NumMpSlot && mpBuild.RequirementType== CMPBuild::SpecificItemReq)
			{
				item.Sheet= mpBuild.SpecificItemRequired;
			}
			fillDBWithMP(toString("%s:%d:%d", MPFaberDB.c_str(), itemReqLine, i), item);
		}

		// *** Grayed,NoQuantity,NoQuality for the last slot of a specific requirement
		CDBCtrlSheet	*ctrl= listSheet->getSheet(i);
		if(ctrl)
		{
			if(i==mpBuild.NumMpSlot && mpBuild.RequirementType== CMPBuild::SpecificItemReq)
			{
				ctrl->setUseQuality(false);
				ctrl->setUseQuantity(false);
				ctrl->setGrayed(true);
			}
			else
			{
				ctrl->setUseQuality(true);
				ctrl->setUseQuantity(true);
				ctrl->setGrayed(false);
			}
		}
	}
}


// ***************************************************************************
void		CActionPhraseFaber::updateQuantityView(uint itemReqLine)
{
	nlassert(itemReqLine<MAX_ITEM_REQ_LINE);

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:SELECTED", MPQuantityDb.c_str(), itemReqLine), false);
	if(node)
		node->setValue32(getTotalQuantitySetuped(itemReqLine));
	node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:REQUIRED", MPQuantityDb.c_str(), itemReqLine), false);
	if(node)
		node->setValue32(_MPBuild[itemReqLine].QuantityReq);
}


// ***************************************************************************
void			CActionPhraseFaber::updateValidButton()
{
	// Check For All MPSlot: If All Ok, then can validate!
	bool	canValid= true;

	// can validate only if the Plan has been selected
	if(_ExecuteFromItemPlanBrick)
	{
		// Run all the current Build execution
		for(uint itemReqLine=0;itemReqLine<_MPBuildNumTotalItemReq;itemReqLine++)
		{
			canValid= canValid && getTotalQuantitySetuped(itemReqLine)==_MPBuild[itemReqLine].QuantityReq;
		}
	}
	else
		canValid= false;

	// unfreeze if valid
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCtrlBaseButton		*validButton= dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseValidButton));
	if(validButton)		validButton->setFrozen(!canValid);
}


// ***************************************************************************
void			CActionPhraseFaber::deleteMpSlot(uint itemReqLine, uint mpSlot)
{
	nlassert(itemReqLine<MAX_ITEM_REQ_LINE);
	CMPBuild	&mpBuild= _MPBuild[itemReqLine];
	nlassert(mpSlot<mpBuild.NumMpSlot);
	uint	invSlot= mpBuild.Id[mpSlot];
	nlassert(invSlot<_InventoryMirror.size());
	nlassert(_InventoryMirror[invSlot].Selected & (1<<itemReqLine));
	// NB: possible that mpBuild.QuantitySelected[mpSlot]==0 (call from removeMpSlotThatUseInvSlot())

	// restore quantity into inventory
	_InventoryMirror[invSlot].Quantity+= mpBuild.QuantitySelected[mpSlot];
	// no more selected for this itemReqLine, so can select it now
	_InventoryMirror[invSlot].Selected&= ~(1<<itemReqLine);

	// update the build by shifting in memory
	uint	i;
	for(i=mpSlot;i<mpBuild.NumMpSlot-1;i++)
	{
		mpBuild.Id[i]= mpBuild.Id[i+1];
		mpBuild.QuantitySelected[i]= mpBuild.QuantitySelected[i+1];
	}
	mpBuild.NumMpSlot--;

	// update the execution view (just what needed)
	for(i=mpSlot;i<mpBuild.NumMpSlot;i++)
	{
		CItem	item= _InventoryMirror[mpBuild.Id[i]];
		item.Quantity= mpBuild.QuantitySelected[i];
		fillDBWithMP(toString("%s:%d:%d", MPFaberDB.c_str(), itemReqLine, i), item);
	}

	// reset the empty slot!
	CItem	item;
	fillDBWithMP(toString("%s:%d:%d", MPFaberDB.c_str(), itemReqLine, mpBuild.NumMpSlot), item);
}



// ***************************************************************************
// ***************************************************************************
// Handlers
// ***************************************************************************
// ***************************************************************************


static	CActionPhraseFaber	*ActionPhraseFaber = NULL;


// ***************************************************************************
// This expr is used only for define in phrase.xml.
DECLARE_INTERFACE_CONSTANT(getPhraseMPSelectionMax, MAX_MP_SELECTION_ENTRIES)


// ***************************************************************************
class	CHandlerPhraseFaberSelectMP : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CDBCtrlSheet	*ctrl= dynamic_cast<CDBCtrlSheet*>(pCaller);
		if(!ctrl)
			return;

		// get itemReqLine to Modify
		uint	itemReqLine;
		fromString(getParam(Params, "item_req"), itemReqLine);
		// get mpSlot edited
		uint	mpSlot= ctrl->getIndexInDB();

		if (ActionPhraseFaber == NULL) ActionPhraseFaber = new CActionPhraseFaber;
		ActionPhraseFaber->startMpSelection(itemReqLine, mpSlot);
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseFaberSelectMP, "phrase_faber_select_mp");


// ***************************************************************************
class	CHandlerPhraseFaberValidateMP : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CDBCtrlSheet	*ctrl= dynamic_cast<CDBCtrlSheet*>(pCaller);
		if(!ctrl)
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			CWidgetManager::getInstance()->disableModalWindow();
			return;
		}

		// get the selected MP.
		uint	selectMP= ctrl->getIndexInDB();

		if (ActionPhraseFaber == NULL) ActionPhraseFaber = new CActionPhraseFaber;
		ActionPhraseFaber->validateMpSelection(selectMP);
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseFaberValidateMP, "phrase_faber_validate_mp");


// ***************************************************************************
class	CHandlerPhraseFaberValidate : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (ActionPhraseFaber == NULL) ActionPhraseFaber = new CActionPhraseFaber;
		ActionPhraseFaber->validateExecution();
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseFaberValidate, "phrase_faber_validate");


// ***************************************************************************
class	CHandlerPhraseFaberValidateOnEnter : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// get the button
		CInterfaceManager		*pIM= CInterfaceManager::getInstance();
		CCtrlBaseButton			*button= dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseValidButton));

		// Ok, button found. test if active.
		if( button && !button->getFrozen() )
		{
			// Act as if the player click on this button
			CAHManager::getInstance()->runActionHandler("phrase_faber_validate", button );
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseFaberValidateOnEnter, "phrase_faber_validate_on_enter");


// ***************************************************************************
class	CHandlerPhraseFaberSelectMpQuantity : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (ActionPhraseFaber == NULL) ActionPhraseFaber = new CActionPhraseFaber;
		ActionPhraseFaber->validateMpSelectQuantity();
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseFaberSelectMpQuantity, "phrase_faber_select_mp_quantity");



// ***************************************************************************
void		launchFaberCastWindow(sint32 memoryLine, uint memoryIndex, CSBrickSheet *rootBrick)
{
	if (ActionPhraseFaber == NULL) ActionPhraseFaber = new CActionPhraseFaber;
	ActionPhraseFaber->launchFaberCastWindow(memoryLine, memoryIndex, rootBrick);
}

// ***************************************************************************
void		fillFaberPlanSelection(const std::string &brickDB, uint maxSelection)
{
	if (ActionPhraseFaber == NULL) ActionPhraseFaber = new CActionPhraseFaber;
	ActionPhraseFaber->fillFaberPlanSelection(brickDB, maxSelection);
}

// ***************************************************************************
void		validateFaberPlanSelection(CSBrickSheet *itemPlanBrick)
{
	if (ActionPhraseFaber == NULL) ActionPhraseFaber = new CActionPhraseFaber;
	ActionPhraseFaber->validateFaberPlanSelection(itemPlanBrick);
}

// ***************************************************************************
void		closeFaberCastWindow()
{
	if (ActionPhraseFaber == NULL) return;
	CGroupContainer	*window= dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseWindow));
	if(window && window->getActive())
		window->setActive(false);
}


// ***************************************************************************
class	CHandlerPhraseFaberOnClose : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (ActionPhraseFaber == NULL) ActionPhraseFaber = new CActionPhraseFaber;
		ActionPhraseFaber->onCloseFaberCastWindow();
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseFaberOnClose, "phrase_faber_on_close");


// ***************************************************************************
// ***************************************************************************
// Management of Change in Inventory
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CActionPhraseFaber::removeMpSlotThatUseInvSlot(uint invSlot, uint quantityToRemove)
{
	if(quantityToRemove==0)
		return;

	// remove from all mpSlots
	for(uint itemReqLine=0;itemReqLine<_MPBuildNumTotalItemReq;itemReqLine++)
	{
		CMPBuild	&mpBuild= _MPBuild[itemReqLine];
		for(uint mpSlot=0;mpSlot<mpBuild.NumMpSlot;)
		{
			// if this mpSlot use the invSlot.
			if(mpBuild.Id[mpSlot]==invSlot)
			{
				// then remove stack mp. If slot not enough
				uint	removeSlotQuantity;
				removeSlotQuantity= min(quantityToRemove, mpBuild.QuantitySelected[mpSlot]);
				mpBuild.QuantitySelected[mpSlot]-= removeSlotQuantity;
				quantityToRemove-= removeSlotQuantity;

				// if slot completely removed, then remove it totaly
				if(mpBuild.QuantitySelected[mpSlot]==0)
				{
					// NB: nothing is restored to inventory since mpBuild.QuantitySelected[mpSlot] is reseted before
					deleteMpSlot(itemReqLine, mpSlot);
				}
				else
				{
					// just change this DB view
					CItem	item= _InventoryMirror[invSlot];
					item.Quantity= mpBuild.QuantitySelected[mpSlot];
					fillDBWithMP(toString("%s:%d:%d", MPFaberDB.c_str(), itemReqLine, mpSlot), item);

					// go to next slot
					mpSlot++;
				}

				// if ok, all req quantity removed, quit
				if(quantityToRemove==0)
					return;
			}
			else
				mpSlot++;
		}
	}
}


// ***************************************************************************
void		CActionPhraseFaber::onInventoryChange()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	uint	i;
	bool	displayChange= false;

	// If the Faber Plan has not yet been selected, then must not check _InventoryMirror, since not initialized
	if(_ExecuteFromItemPlanBrick==NULL)
		return;

	// Run all the Bag
	uint invId = 0;
	uint indexInInv = 0;
	for(i=0;i<_InventoryMirror.size();i++)
	{
		CItemImage	*itemImage= getInvMirrorItemImage(i, invId, indexInInv);

		if(itemImage)
		{
			CSheetId	sheetId= CSheetId(itemImage->getSheetID());
			CItemSheet	*mpSheet= dynamic_cast<CItemSheet*>(SheetMngr.get(sheetId));
			CItem		newInvItem;

			bool bLockedByOwner = itemImage->getLockedByOwner();

			// The item must be a mp, and the item must be available and unlocked
			if( isMpAvailable(mpSheet, invId, i) && !bLockedByOwner)
			{
				newInvItem.Sheet= sheetId;
				newInvItem.Quality= itemImage->getQuality();
				newInvItem.Quantity= itemImage->getQuantity();
				newInvItem.UserColor= itemImage->getUserColor();
				newInvItem.Weight= itemImage->getWeight();
				newInvItem.OriginalQuantity= newInvItem.Quantity;
				newInvItem.LockedByOwner = bLockedByOwner;
			}

			/* There is 5 cases:
				- no changes => no op.
				- new/unlocked Mp on a empty or non Mp slot. Easy, just add.
				- old Mp removed (not same sheetId/quality/userColor/locked)
				- old Mp with quantity changed to be greater
				- old Mp with quantity changed to be smaller
			*/

			CItem		&curInvItem= _InventoryMirror[i];

			// Bkup Id in newInvItem (for ope= correctness)
			newInvItem.InventoryId= curInvItem.InventoryId;
			newInvItem.IdInInventory= curInvItem.IdInInventory;

			// If the item was not a mp
			if(_InventoryMirror[i].Sheet==CSheetId::Unknown)
			{
				// if now it is, easy, just add if not locked
				if(newInvItem.Sheet!=CSheetId::Unknown && !newInvItem.LockedByOwner)
					curInvItem= newInvItem;
			}
			// else must test change or remove
			else
			{
				bool	sameMp;
				sameMp=	curInvItem.Sheet == newInvItem.Sheet &&
						curInvItem.Quality == newInvItem.Quality &&
						curInvItem.UserColor == newInvItem.UserColor &&
						curInvItem.LockedByOwner == newInvItem.LockedByOwner;

				// if the Mp was deleted from this slot, delete it from all faber execution
				if(!sameMp)
				{
					// remove all from current execution
					removeMpSlotThatUseInvSlot(i, curInvItem.OriginalQuantity);

					// replace (with nothing or new different Mp)
					curInvItem= newInvItem;

					// mpSlot may have been deleted
					displayChange= true;
				}
				// test change of quantity
				else
				{
					// if the quantity is the same, no op!
					if(curInvItem.OriginalQuantity!=newInvItem.OriginalQuantity)
					{
						// if the quantity is now greater, its easy
						if(newInvItem.OriginalQuantity > curInvItem.OriginalQuantity)
						{
							// just add the difference to the original and current setuped quantity
							uint32	diff= newInvItem.OriginalQuantity - curInvItem.OriginalQuantity;
							curInvItem.OriginalQuantity+= diff;
							curInvItem.Quantity+= diff;
						}
						else
						{
							// complex, must remove the quantity that has changed
							uint32	diff= curInvItem.OriginalQuantity - newInvItem.OriginalQuantity;
							// try first to remove it from remaining quantity
							if(curInvItem.Quantity>=(sint32)diff)
							{
								// no change to current mpSlots!
								curInvItem.Quantity-= diff;

								// must close the selection modal if opened
								displayChange= true;
							}
							// The remaining quantity is not enough, must also remove from mpSlot that use it!
							else
							{
								uint32	toRemoveFromSlot= diff - curInvItem.Quantity;
								curInvItem.Quantity= 0;
								// remove all needed to current mp slot.
								removeMpSlotThatUseInvSlot(i, toRemoveFromSlot);

								// mpSlot may have been deleted
								displayChange= true;
							}

							// bkup new original quantity
							curInvItem.OriginalQuantity= newInvItem.OriginalQuantity;
						}
					}
				}
			}
		}
	}

	// must update display?
	if(displayChange)
	{
		for(uint itemReqLine= 0;itemReqLine<_MPBuildNumTotalItemReq;itemReqLine++)
		{
			updateEmptySlot(itemReqLine);
			updateQuantityView(itemReqLine);
		}
		updateValidButton();

		// close selection modals if they are opened
		CInterfaceGroup		*quantityModal= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseMpQuantityModal));
		if(quantityModal && CWidgetManager::getInstance()->getModalWindow()==quantityModal)
			CWidgetManager::getInstance()->disableModalWindow();
		CInterfaceGroup		*listModal= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseMpListModal));
		if(listModal && CWidgetManager::getInstance()->getModalWindow()==listModal)
			CWidgetManager::getInstance()->disableModalWindow();

		// update item result
		updateItemResult();
	}
}

// ***************************************************************************
void	CActionPhraseFaber::CDBInventoryObs::update(ICDBNode * /* node */)
{
	if (ActionPhraseFaber == NULL) ActionPhraseFaber = new CActionPhraseFaber;
	ActionPhraseFaber->onInventoryChange();
}

// ***************************************************************************
void	CActionPhraseFaber::CDBAnimalObs::update(ICDBNode * /* node */)
{
	if (ActionPhraseFaber == NULL) ActionPhraseFaber = new CActionPhraseFaber;
	ActionPhraseFaber->onInventoryChange();
}



// ***************************************************************************
void	CActionPhraseFaber::updateItemResult()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	uint	i;

	// level is the min level of MP
	sint32	minLevel= INT_MAX;
	// Stat is computed like server
	float	statArray[RM_FABER_STAT_TYPE::NumRMStatType];
	float	statCount[RM_FABER_STAT_TYPE::NumRMStatType];
	uint64	itemStatBF= 0;
	for(i=0;i<RM_FABER_STAT_TYPE::NumRMStatType;i++)
	{
		statArray[i]= 0;
		statCount[i]= 0;
	}
	// Color stat (for armour)
	uint32		bestItemColor[RM_COLOR::NumColors];
	for(i=0;i<RM_COLOR::NumColors;i++)
		bestItemColor[i]= 0;
	// Stat energy is computed like server
	float	statEnergy= 0;


	// **** Parse all Bricks of the phrase executed, to get min level
	// take The brick with the lowest CR_RECOMMENDED
	uint		phraseSlot= pPM->getMemorizedPhrase(_ExecuteFromMemoryLine, _ExecuteFromMemoryIndex);
	const CSPhraseCom	&phrase= pPM->getPhrase(phraseSlot);
	uint32		recommendedPropId= pBM->getBrickPropId("cr_recommended");
	for(i=0;i<phrase.Bricks.size();i++)
	{
		CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
		if(brick)
		{
			for(uint j=0;j<brick->Properties.size();j++)
			{
				// if a CR_RECOMMENDED propId
				if(brick->Properties[j].PropId == recommendedPropId)
				{
					// minimze the level
					minLevel= min(minLevel, sint32(brick->Properties[j].Value));
				}
			}
		}
	}


	// **** Parse all MPs setuped, to compute level and stats
	uint	totalItemPartMPReq= 0;
	uint	totalItemPartMPSetuped= 0;
	for(i=0;i<_MPBuildNumTotalItemReq;i++)
	{
		CMPBuild		&mpBuild= _MPBuild[i];

		// --- ItemPart requirement?
		if(mpBuild.RequirementType==CMPBuild::ItemPartReq)
		{
			// For all slots setuped
			uint	nSlot= min((uint)MAX_MP_SLOT, mpBuild.NumMpSlot);
			for(uint j=0;j<nSlot;j++)
			{
				// Try to get the MP in this slot
				CItemSheet		*mp= dynamic_cast<CItemSheet*>(SheetMngr.get(_InventoryMirror[mpBuild.Id[j]].Sheet));
				if(mp && mp->canBuildItemPart(mpBuild.FaberTypeRequired))
				{
					// minimize level
					minLevel= min(_InventoryMirror[mpBuild.Id[j]].Quality, minLevel);

					// Increment stat for each of this MP selected
					const CItemSheet::CMpItemPart	&mpIP= mp->getItemPart(mpBuild.FaberTypeRequired);

					// append to the stats
					uint	numMps= mpBuild.QuantitySelected[j];
					for(uint k=0;k<RM_FABER_STAT_TYPE::NumRMStatType;k++)
					{
						if(RM_FABER_STAT_TYPE::isStatRelevant(mpBuild.FaberTypeRequired, (RM_FABER_STAT_TYPE::TRMStatType)k))
						{
							// %age to 0-1.
							statArray[k]+= numMps * (mpIP.Stats[k]/100.f);
						}
					}

					// Same for total energy
					statEnergy+= numMps * (mp->Mp.StatEnergy/100.f);

					// Increment color stat
					if(mp->Mp.MpColor>=0 && mp->Mp.MpColor<RM_COLOR::NumColors)
					{
						bestItemColor[mp->Mp.MpColor]+= numMps;
					}

					// Total MP setuped
					totalItemPartMPSetuped+= numMps;
				}
			}

			// get all stat for this item, and count MP req per stat
			for(uint k=0;k<RM_FABER_STAT_TYPE::NumRMStatType;k++)
			{
				// if item part 'i' affect stat 'k'
				if(RM_FABER_STAT_TYPE::isStatRelevant(mpBuild.FaberTypeRequired, (RM_FABER_STAT_TYPE::TRMStatType)k))
				{
					// StatPerItemPart ored in StatPerItem
					itemStatBF|= uint64(1)<<k;
					// Total Num MP per stat
					statCount[k]+= mpBuild.QuantityReq;
				}
			}

			// Total MP Req
			totalItemPartMPReq+= mpBuild.QuantityReq;
		}

		// --- Specific Item requirement?
		else if(mpBuild.RequirementType==CMPBuild::SpecificItemReq)
		{
			// For all slots setuped
			uint	nSlot= min((uint)MAX_MP_SLOT, mpBuild.NumMpSlot);
			for(uint j=0;j<nSlot;j++)
			{
				// Try to get the MP in this slot
				CItemSheet		*mp= dynamic_cast<CItemSheet*>(SheetMngr.get(_InventoryMirror[mpBuild.Id[j]].Sheet));
				if(mp->Id == mpBuild.SpecificItemRequired)
				{
					// minimize level
					minLevel= min(_InventoryMirror[mpBuild.Id[j]].Quality, minLevel);

					// Formula 's Specific MPs don't impact on stats.
				}
			}
		}
	}

	// Mean stat
	for(i=0;i<RM_FABER_STAT_TYPE::NumRMStatType;i++)
	{
		if(statCount[i])
			statArray[i]/= statCount[i];
		clamp(statArray[i], 0.f, 1.f);
	}

	// Mean stat energy
	if(totalItemPartMPReq)
	{
		statEnergy/= totalItemPartMPReq;
		clamp(statEnergy, 0.f, 1.f);
	}

	// As in server, stretch the stats.
	// Add the special bonus ONLY when all MPs are setuped, for clearness
	RM_FABER_STAT_TYPE::stretchItemStats(statArray, itemStatBF, totalItemPartMPSetuped>=totalItemPartMPReq);


	// **** setup Level
	if(minLevel==INT_MAX)
		minLevel= 0;
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(ItemResultSheetLevel, false);
	if(node)
		node->setValue32(minLevel);


	// **** change success rate too
	CViewText	*successView= dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseFpSuccessText));
	if(successView)
	{
		ucstring	text= CI18N::get("uiPhraseFaberSuccessRate");
		// Get the success rate of the related phrase
		uint		phraseSlot= pPM->getMemorizedPhrase(_ExecuteFromMemoryLine, _ExecuteFromMemoryIndex);

		sint32 craftSuccessModifier = 0;
		CCDBNodeLeaf	*nodeCSM= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:CRAFT", false);
		if(nodeCSM)
		{
			craftSuccessModifier = nodeCSM->getValue32();
		}
		// With the faber plan skill
		sint		success= pPM->getCraftPhraseSuccessRate(pPM->getPhrase(phraseSlot), _ExecuteFromItemPlanBrick->getSkill(), minLevel);
		string		successStr;
		if( craftSuccessModifier == 0 )
		{
			successStr = toString("@{FFFF}") + toString(success);
		}
		else
		if( craftSuccessModifier > 0 ) // bonus
		{
			successStr = "@{0F0F}" + toString(success+craftSuccessModifier)
							+ "@{FFFF}("
							+ toString( success )
							+ "@{0F0F} + "
							+ toString( craftSuccessModifier )
							+ "@{FFFF})";
		}
		else
		{
			successStr = "@{E42F}" + toString(success+craftSuccessModifier)
				+ "@{FFFF}("
				+ toString( success )
				+ "@{E42F} - "
				+ toString( craftSuccessModifier )
				+ "@{FFFF})";
		}
		strFindReplace(text, "%success", successStr );
		successView->setTextFormatTaged(text);
	}


	// **** setup Color
	// Same than server code (NB: beige==1 per default)
	uint	maxNumColor = 0;
	uint	dominanteColor = 1;
	for(i = 0; i < RM_COLOR::NumColors; ++i )
	{
		if( bestItemColor[i] > maxNumColor )
		{
			maxNumColor = bestItemColor[i];
			dominanteColor = i;
		}
	}
	node= NLGUI::CDBManager::getInstance()->getDbProp(ItemResultSheetColor, false);
	if(node)
		node->setValue32(dominanteColor);


	// **** Get Stat validity
	uint64	itemStatFinalUsageBF= 0;
	// Some stat (magic protection and magic resist) are finaly used in the item only for the best ones
	itemStatFinalUsageBF= RM_FABER_STAT_TYPE::getStatFinalValidity(statArray, itemStatBF);


	// **** Stats
	CInterfaceGroup		*groupMp= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(FaberPhraseItemResultGroup));
	if(groupMp)
	{
		// default: hide all
		for(i=0;i<RM_FABER_STAT_TYPE::NumRMStatType;i++)
		{
			// get the stat group
			CInterfaceGroup		*groupStat= dynamic_cast<CInterfaceGroup*>(groupMp->getElement(groupMp->getId()+toString(":stat%d",i) ));
			if(groupStat)
				groupStat->setActive(false);
		}
		// enable only one that are relevant for this item
		uint	groupIndex= 0;
		for(i=0;i<RM_FABER_STAT_TYPE::NumRMStatType;i++)
		{
			RM_FABER_STAT_TYPE::TRMStatType		statType= RM_FABER_STAT_TYPE::TRMStatType(i);

			// if this stat is not relevant for the item, don't display it!
			if( (itemStatBF&(uint64(1)<<i)) == 0)
				continue;

			// Is the stat finaly used? (magic protection for instance may not be)
			bool	finalyUsed= (itemStatFinalUsageBF&(uint64(1)<<i)) != 0;
			CRGBA	usageColor= finalyUsed?(CRGBA::White):CRGBA(128,128,128);

			// get the next stat group
			CInterfaceGroup		*groupStat= dynamic_cast<CInterfaceGroup*>(groupMp->getElement(groupMp->getId()+toString(":stat%d",groupIndex) ));
			if(groupStat)
			{
				groupStat->setActive(true);
				// fill text and bar according to stat
				CViewText	*statTitle= dynamic_cast<CViewText*>(groupStat->getElement(groupStat->getId()+":text" ));
				CDBViewBar	*statValueBar= dynamic_cast<CDBViewBar*>(groupStat->getElement(groupStat->getId()+":bar" ));
				CViewText	*statValueText= dynamic_cast<CViewText*>(groupStat->getElement(groupStat->getId()+":textstat" ));
				CCtrlBase	*statToolTip= dynamic_cast<CCtrlBase*>(groupStat->getElement(groupStat->getId()+":tt" ));
				uint	sv= uint(statArray[i]*100);
				if(statTitle)
				{
					statTitle->setText(RM_FABER_STAT_TYPE::toLocalString(statType));
					statTitle->setColor(usageColor);
				}
				if(statValueBar)
				{
					statValueBar->setValue(sv);
					statValueBar->setColor(usageColor);
				}
				if(statValueText)
				{
					statValueText->setText(toString(sv)+"/100");
					statValueText->setColor(usageColor);
				}
				if(statToolTip)
				{
					if(finalyUsed)
					{
						// display something only for magic/protect stat
						if( RM_FABER_STAT_TYPE::isMagicResistStat(RM_FABER_STAT_TYPE::TRMStatType(i)) ||
							RM_FABER_STAT_TYPE::isMagicProtectStat(RM_FABER_STAT_TYPE::TRMStatType(i)) )
							statToolTip->setDefaultContextHelp(CI18N::get("uiFaberStatActive"));
						else
							statToolTip->setDefaultContextHelp(ucstring());
					}
					else
						statToolTip->setDefaultContextHelp(CI18N::get("uiFaberStatGrayed"));
				}
			}

			groupIndex++;
		}
	}


	// **** BestStat (for text over)
	node= NLGUI::CDBManager::getInstance()->getDbProp(ItemResultSheetStatType, false);
	if(node)
	{
		float bestStatValue =-1.0f;
		RM_FABER_STAT_TYPE::TRMStatType bestStat = RM_FABER_STAT_TYPE::NumRMStatType;
		for( i = 0; i < RM_FABER_STAT_TYPE::NumRMStatType; ++i )
		{
			// if this stat is not relevant for the item, don't use it!
			if( (itemStatBF&(uint64(1)<<i)) == 0)
				continue;
			float value = statArray[i];
			if( value > bestStatValue )
			{
				bestStatValue = value;
				bestStat = (RM_FABER_STAT_TYPE::TRMStatType)i;
			}
		}
		// Setup DB
		node->setValue32(bestStat);
	}


	// **** ClassType (for text over)
	node= NLGUI::CDBManager::getInstance()->getDbProp(ItemResultSheetClassType, false);
	if(node)
	{
		// Setup DB
		node->setValue32(RM_CLASS_TYPE::getItemClass((uint32)(100.0f * statEnergy)));
	}

}

/* Handle change of skill -> recompute success rate */
void CActionPhraseFaber::CSkillObserver::onSkillChange()
{
	if (ActionPhraseFaber == NULL) ActionPhraseFaber = new CActionPhraseFaber;
	// Dont update if the plan has not yet been selected
	if(ActionPhraseFaber->_ExecuteFromItemPlanBrick==NULL)
		return;
	ActionPhraseFaber->updateItemResult();
}

