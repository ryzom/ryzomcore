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



#ifndef CL_OBS_HUGE_LIST_H
#define CL_OBS_HUGE_LIST_H

#include "dbgroup_list_sheet.h"
#include "game_share/trade_slot_type.h"
#include "game_share/mission_desc.h"
#include "trade_common.h"

// the DB branch contening items to buy
#define DB_BRANCH_FOR_ITEM_FOR_MONEY	"SERVER:TRADING"
#define DB_BRANCH_FOR_ITEM_FOR_MISSION	"SERVER:ITEMS_FOR_MISSIONS"
#define DB_BRANCH_FOR_ASCENSOR			"SERVER:ASCENSOR"
#define DB_BRANCH_FOR_MISSIONS			"SERVER:CHOOSE_MISSIONS"
#define DB_LOCAL_BRANCH_FOR_ITEM_FOR_MONEY		"LOCAL:TRADING"
#define DB_LOCAL_BRANCH_FOR_ITEM_FOR_MISSION	"LOCAL:ITEMS_FOR_MISSIONS"
#define DB_LOCAL_BRANCH_FOR_ASCENSOR			"LOCAL:ASCENSOR"
#define DB_LOCAL_BRANCH_FOR_MISSIONS			"LOCAL:CHOOSE_MISSIONS"


// ****************************************************************************
/** Database observer to update incrementaly a list of items
 * Class synchronizing huge list of leaves from server to local
 * This is done through a small window in the server database of 8 (for the moment) elements
 * Each elements can be composed of several leaves
 * This is done through an intermediairy structure to prevent updating local database with page that
 * should not be here (ie 1,2 are received and we receive 4. If we update directly we will have pop and displacement
 * which can be confusing for the player, for example if he has selected an item in page 4.
 *
 * For the moment there are 4 differents lists
 *
 * MISSIONS (in the screen to choose the mission, with an icon for each mission category)
 *		ICON
 *		TEXT
 *      DETAIL_TEXT
 *		PREREQ_STATE
 *
 * ASCENSOR for the teleport stuff in ascensors
 * 		ICON
 * 		NAME
 *
 * TRADING for the bot chat trade
 * 		SHEET
 * 		QUALITY
 *		WEIGHT
 *		INFO_VERSION
 * 		PRICE
 * 		SLOT_TYPE
 *
 * ITEMS_FOR_MISSIONS for the missions (in bot chat)
 * 		SHEET
 * 		QUALITY
 * 		SLOT_TYPE
 * 		LOGIC_TEXT_ID
 * 		DESC_TEXT_ID
 *
 *
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date August 2003
 */
class CHugeListObs : public NLMISC::ICDBNode::IPropertyObserver
{
public:

	enum ListType
	{
		Ascensor = 0,
		Trading,
		ItemForMissions,
		Missions,
		ListTypeCount
	};

	CHugeListObs(ListType category = ListTypeCount);
	~CHugeListObs();

	// Init the category. This must be called (no more that once) before any other method (or the ctor must provide the category)
	void setListType(ListType listType);

	// Set the current mission type, for special msg. Call before start
	void	setMissionClientType(MISSION_DESC::TClientMissionType mType) {_MType= mType;}

	// Used For Phrase Trading only, true to allow phrase update and client fill (only if server agree)
	void	allowPhrasePriceUpdateAndClientFill(bool state) {_PhrasePriceUpdateAndMaybeClientFill= state;}

	// Clean local db if botchat que the botchat session id
	void start();

	// Check whether download has finished
	bool isDownloadComplete() const { return _DownloadComplete; }

	uint16 getSessionID () { nlassert((uint) _Category < ListTypeCount); return _CurrentSessionNb; }
	void incrementSessionID() { nlassert((uint) _Category < ListTypeCount); ++_CurrentSessionNb; }

	// true if the current trading Phrases are filled by client.
	bool	isPhraseClientFill() const {return _PhraseClientFill;}

private:

	// pointer to db entry for one item
	class CItemDBLeaves
	{
	public:
		NLMISC::CCDBNodeLeaf *GuildIcon;	// valid only for ascensor
		NLMISC::CCDBNodeLeaf *GuildName;	// valid only for ascensor
		//
		NLMISC::CCDBNodeLeaf *SlotType;
		NLMISC::CCDBNodeLeaf *Quality;
		NLMISC::CCDBNodeLeaf *SheetIDOrSkill;
		//
		NLMISC::CCDBNodeLeaf *LogicTextID;	// valid if the item is to be obtained for a mission
		NLMISC::CCDBNodeLeaf *DescTextID;	// valid if the item is to be obtained for a mission
		//
		NLMISC::CCDBNodeLeaf *Price;		// valid if the item is to be obtained with money
		//
		NLMISC::CCDBNodeLeaf *MissionText;
		NLMISC::CCDBNodeLeaf *MissionDetailText;
		NLMISC::CCDBNodeLeaf *MissionIcon;
		NLMISC::CCDBNodeLeaf *MissionPreReqState;
		//	items only
		NLMISC::CCDBNodeLeaf *Weight;
		NLMISC::CCDBNodeLeaf *NameId;
		NLMISC::CCDBNodeLeaf *InfoVersion;
		NLMISC::CCDBNodeLeaf *UserColor;
		NLMISC::CCDBNodeLeaf *Enchant;
		NLMISC::CCDBNodeLeaf *RMClassType;
		NLMISC::CCDBNodeLeaf *RMFaberStatType;
		NLMISC::CCDBNodeLeaf *PrerequisitValid;
		// items Resale only
		NLMISC::CCDBNodeLeaf *Quantity;
		NLMISC::CCDBNodeLeaf *PriceRetire;
		NLMISC::CCDBNodeLeaf *ResaleTimeLeft;
		NLMISC::CCDBNodeLeaf *VendorNameId;
		NLMISC::CCDBNodeLeaf *FactionType;
		NLMISC::CCDBNodeLeaf *FactionPointPrice;
		NLMISC::CCDBNodeLeaf *SellerType;
	public:
		CItemDBLeaves() :	GuildIcon(NULL),
							GuildName(NULL),
							SlotType(NULL),
							Quality(NULL),
							SheetIDOrSkill(NULL),
							LogicTextID(NULL),
							DescTextID(NULL),
							Price(NULL),
							MissionText(NULL),
							MissionDetailText(NULL),
							MissionIcon(NULL),
							MissionPreReqState(NULL),
							Weight(NULL),
							NameId(NULL),
							InfoVersion(NULL),
							UserColor(NULL),
							Enchant(NULL),
							RMClassType(NULL),
							RMFaberStatType(NULL),
							Quantity(NULL),
							PriceRetire(NULL),
							ResaleTimeLeft(NULL),
							VendorNameId(NULL),
							FactionType(NULL),
							FactionPointPrice(NULL),
							SellerType(NULL)
		{}
	};
	//
	virtual void update(NLMISC::ICDBNode *node);

	void updateUIItemPage(uint index);

	// init db entry pointers
	bool init();

	// tool fct
	void setPriceInDB(uint32 value, const std::string &ls, const std::string &ms, const std::string &bs, const std::string &vbs);

private:
	//
	ListType		_Category;

	bool			_Init;
	CItemDBLeaves	_Items[TRADE_PAGE_NUM_ITEMS];
	sint16			_CurrentSessionNb;
	NLMISC::CCDBNodeLeaf	*_Session;
	NLMISC::CCDBNodeLeaf	*_PageID;
	NLMISC::CCDBNodeLeaf	*_HasNext;

	bool			 _DownloadComplete;

	/** Temporary structure to store pages
	  * Managment of items (for trade)
	  * Items are received a pages at a time from the server.
	  * This struct mimic one db entry for an item in the trade (see database.xml).
	  * A a slot is empty if SlotType == StandardBehaviour and SheetID == 0
	  */
	struct CItem
	{

		TRADE_SLOT_TYPE::TTradeSlotType	SlotType;
		uint16							Quality;
		uint32							SheetIDOrSkill;
		uint32 LogicTextID;				// Valid if the item is to be obtained as a mission reward
		uint32 DescTextID;				// Valid if the item is to be obtained as a mission reward
		uint32 Price;					// Valid for standard items (not to be obtained with a mission)
		uint64 GuildIcon;	// Valid in Ascensor
		uint32 GuildName;	// Valid in Ascensor
		uint32 MissionText;			// Valid in Missions
		uint32 MissionDetailText;	// Valid in Missions
		uint32 MissionIcon;			// Valid in Missions
		uint32 MissionPreReqState;	// Valid in Missions
		uint32 Weight;		// Valid for items
		uint32 NameId;		// Valid for items
		uint32 InfoVersion; // Valid for items
		sint32 UserColor;   // Valid for items
		uint32 Enchant;
		uint32 RMClassType;
		uint32 RMFaberStatType;
		uint32 PrerequisitValid;
		uint32 PriceRetire;			// Valid for Resale items
		uint32 ResaleTimeLeft;		// Valid for Resale items
		uint32 VendorNameId;		// Valid for Resale items
		uint32 FactionType;			// Valid for Resale items
		uint32 FactionPointPrice;	// Valid for Resale items
		uint32 Quantity;			// Valid for Resale items
		uint32 SellerType;			// Valid for Resale items
	};

	// a page of items
	struct CItemPage
	{
		bool	   PageLoaded; // was the page loaded ?
		bool       PageVisible;
		CItem	   Items[TRADE_PAGE_NUM_ITEMS];
		CItemPage() : PageLoaded(false), PageVisible(false) {}
	};


	typedef std::vector<CItemPage> TItemVect;

	TItemVect						_ItemsPages;
	CDBCtrlSheet::TSheetCategory	_ItemCategory;
	MISSION_DESC::TClientMissionType _MType;
	uint32							_FirstPageNotUpdated;

	// For Phrase price update
	bool							_PhrasePriceUpdateAndMaybeClientFill;
	class IBrickLearnedCallback		*_PhrasePriceCB;
	friend class CPhrasePriceCB;
	void		recomputeAllPhrasePrices();

	// For Phrase Generic Fill
	void		clientGenerateAllPhrases();
	bool							_PhraseClientFill;			// if both client and server said "client fill phrase"
	uint32							_PhraseClientFillFlags;
	uint32							_PhraseClientFillRace;
	uint32							_PhraseClientFillNumPhrase;
	NLMISC::CCDBNodeLeaf					*_RoleMasterFlagDB;
	NLMISC::CCDBNodeLeaf					*_RoleMasterRaceDB;
};

#endif
