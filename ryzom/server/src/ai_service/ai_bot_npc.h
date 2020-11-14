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

#ifndef RYAI_BOT_NPC_H
#define RYAI_BOT_NPC_H

#include "game_share/msg_ais_egs_gen.h"
#include "ai_bot.h"
#include "owners.h"
#include "npc_description_msg.h"
#include "states.h"
#include "ai_player.h"

class CBotNpc;
class CNpc;
class CSpawnGroupNpc;
class CBotPlayer;
class CPlayerControlNpc;

//////////////////////////////////////////////////////////////////////////////
// CSpawnBotNpc                                                             //
//////////////////////////////////////////////////////////////////////////////

class CSpawnBotNpc
: public NLMISC::CDbgRefCount<CSpawnBotNpc>
, public CSpawnBot
, public CPetOwner
{
public:
	CSpawnBotNpc(TDataSetRow const& row, CBot& owner, NLMISC::CEntityId const& id, float radius, uint32 level, RYAI_MAP_CRUNCH::TAStarFlag denyFlags);
	
	void update(uint32 ticks);
	
	CSpawnGroupNpc& spawnGrp() const;
	
	// Accessors for NPC dynamic parameters -----------------------------
	void setCurrentChatProfile(CNpcChatProfileImp* chatProfile);

	void updateChat(CAIState const* state);
	
	virtual void processEvent(CCombatInterface::CEvent const& event);
	
	CAIEntityPhysical& getPhysical() { return *this; }
	
	/// @name Chat parameter management 
	//@{
	void beginBotChat(CBotPlayer* plr);
	void endBotChat(CBotPlayer* plr);
	void beginDynChat() { ++_NbCurrentDynChats; }
	void endDynChat() { --_NbCurrentDynChats; }
	//@}
	
	// Return the active bot chats
	std::vector<CBotPlayer*>& getActiveChats() { return _ActiveChats; }
	
	// Return the number of dyn chats (these chats are not managed by the AIS)
	uint getNbActiveDynChats() { return _NbCurrentDynChats; }
	
	/// Dispatching message to EGS to describe chat possibilities
	void sendInfoToEGS() const;
	
	virtual std::vector<std::string> getMultiLineInfoString() const;
	
	CBotNpc& getPersistent() const;
	
	virtual RYZOMID::TTypeId getRyzomType() const { return RYZOMID::npc; }
	
	virtual	bool isBotAttackable() const;
	
	virtual void propagateAggro() const;
	
	float getReturnDistCheck() const;

	void setPlayerController(CBotPlayer* player);

	void setFacing(CAngle theta);

	void setUserModelId(const std::string &id);

	std::string getUserModelId();

	void setCustomLootTableId(const std::string &id);

	std::string getCustomLootTableId();

	void setPrimAlias(uint32 alias);
	uint32 getPrimAlias() const;

private:
	std::vector<CBotPlayer*> _ActiveChats;	// vector of ptrs to players currently chatting with bot
	float _OldHpPercentage; // Fix for HP triggers
	
	CNpcChatProfileImp	_CurrentChatProfile;
	sint32				_NbCurrentDynChats;
	NLMISC::CSmartPtr<CPlayerControlNpc>	_PlayerController;
	uint32 _FacingTick;
	CAngle	_FacingTheta;

	std::string	_UserModelId;
	std::string _CustomLootTableId;
	uint32		_PrimAlias;
};

//////////////////////////////////////////////////////////////////////////////
// CBotNpcSheet                                                             //
//////////////////////////////////////////////////////////////////////////////

class CBotNpcSheet
: public AISHEETS::CCreatureProxy
{
public:
	CBotNpcSheet(AISHEETS::ICreatureCPtr const& sheet)
	: AISHEETS::CCreatureProxy(sheet)
	, _LeftItem(NLMISC::CSheetId::Unknown)
	, _RightItem(NLMISC::CSheetId::Unknown)
	, _ColorHead(0)
	, _ColorArms(0)
	, _ColorHands(0)
	, _ColorBody(0)
	, _ColorLegs(0)
	, _ColorFeets(0)
	{
		reset();
	}
	
	///@name ICreature overloads
	//@{
	virtual uint8 ColorHead() const { return _ColorHead; }
	virtual uint8 ColorArms() const { return _ColorArms; }
	virtual uint8 ColorHands() const { return _ColorHands; }
	virtual uint8 ColorBody() const { return _ColorBody; }
	virtual uint8 ColorLegs() const { return _ColorLegs; }
	virtual uint8 ColorFeets() const { return _ColorFeets; }
	
	virtual NLMISC::CSheetId const& LeftItem() const
	{
		if (_LeftItem!=NLMISC::CSheetId::Unknown)
			return _LeftItem;
		else
			return this->CCreatureProxy::LeftItem();
	}
	virtual NLMISC::CSheetId const& RightItem() const
	{
		if (_RightItem!=NLMISC::CSheetId::Unknown)
			return _RightItem;
		else
			return this->CCreatureProxy::RightItem();
	}
	//@}
	
	///@name Setters
	//@{
	void setColorHead(uint8 val) { _ColorHead = val; }
	void setColorArms(uint8 val) { _ColorArms = val; }
	void setColorHands(uint8 val) { _ColorHands = val; }
	void setColorBody(uint8 val) { _ColorBody = val; }
	void setColorLegs(uint8 val) { _ColorLegs = val; }
	void setColorFeets(uint8 val) { _ColorFeets = val; }
	
	void setLeftItem(NLMISC::CSheetId const& val) { _LeftItem = val; }
	void setRightItem(NLMISC::CSheetId const& val) { _RightItem = val; }
	//@}

	void reset()
	{
		if (_Sheet)
		{
			_LeftItem = _Sheet->LeftItem();
			_RightItem = _Sheet->RightItem();
			_ColorHead = _Sheet->ColorHead();
			_ColorArms = _Sheet->ColorArms();
			_ColorHands = _Sheet->ColorHands();
			_ColorBody = _Sheet->ColorBody();
			_ColorLegs = _Sheet->ColorLegs();
			_ColorFeets = _Sheet->ColorFeets();
		}
	}
		
private:	
	NLMISC::CSheetId	_LeftItem;
	NLMISC::CSheetId	_RightItem;
	uint8				_ColorHead;
	uint8				_ColorArms;
	uint8				_ColorHands;	
	uint8				_ColorBody;
	uint8				_ColorLegs;
	uint8				_ColorFeets;
};
typedef NLMISC::CSmartPtr<CBotNpcSheet> CBotNpcSheetPtr;
typedef NLMISC::CSmartPtr<CBotNpcSheet const> CBotNpcSheetCPtr;

//////////////////////////////////////////////////////////////////////////////
// CBotNpc                                                                  //
//////////////////////////////////////////////////////////////////////////////

class CBotNpc
: public CBot
, public CKeyWordOwner
{
public:
	friend class CSpawnBotNpc; // allows CSpawnBotNpc to acceed his definition.
	
public:
	CBotNpc(CGroup* owner, CAIAliasDescriptionNode* alias = NULL);
	CBotNpc(CGroup* owner, uint32 alias, std::string const& name);
	virtual ~CBotNpc();
	
	void init();
	
	RYZOMID::TTypeId getRyzomType() const { return RYZOMID::npc; }
	
	CSpawnBotNpc* getSpawn();
	CSpawnBotNpc const* getSpawn() const;
	
	CAIS::CCounter& getSpawnCounter();
	
	void calcSpawnPos(RYAI_MAP_CRUNCH::CWorldMap const& worldMap);
	
	void getSpawnPos(CAIVector& triedPos, RYAI_MAP_CRUNCH::CWorldPosition& pos, RYAI_MAP_CRUNCH::CWorldMap const& worldMap, CAngle& spawnTheta);
	
	CSpawnBot* getSpawnBot(TDataSetRow const& row, NLMISC::CEntityId const& id, float radius);
	
	// spawn & despawn --------------------------------------------------
	virtual bool spawn();
	virtual void despawnBot();
	
	bool reSpawn(bool sendMessage = true);
	
	//------------------------------------------------------------
	// accessing the parent mgr, group, etc 
	CGroupNpc& grp() const;
	
	// Carried equipment management -------------------------------------
	void equipmentInit();
	void equipmentAdd(std::string const& kit);
	
	// Write accessors for NPC base parameters --------------------------
	void setStartPos(double x, double y, float theta, AITYPES::TVerticalPos verticalPos);
	
	void setColour(uint8 colour);
	void setColours(std::string colours);

	void setVisualProperties(std::string input);

	inline void setMaxHitRangeForPlayer(float maxHitRange) { _MaxHitRangeForPC = maxHitRange; }
//	void setMissionStepIconHidden(bool hide) { _MissionIconFlags.IsMissionStepIconDisplayable = !hide; }
//	void setMissionGiverIconHidden(bool hide) { _MissionIconFlags.IsMissionGiverIconDisplayable = !hide; }
	
	// Read accessors for NPC base parameters ---------------------------
	NLMISC::CSheetId getStats() const { return getSheet()->SheetId(); }
	CAIPos const& getStartPos() const { return _StartPos; }
	AITYPES::TVerticalPos getStartVerticalPos() const { return _VerticalPos; }
	
	// Callback called on state change of parent group (including changes to/from punctual states)
	
	// the update routine (delegates control to the _aiProfile object) --
	//	void update(uint32 ticks);
	
	// do nothing for 'ticks' ticks
	// return number of ticks of movement time left after destination reached (or 0)


	void sendVPA();
	void sendVisualProperties();
	
	void newChat();
	
	NLMISC::CSmartPtr<CNpcChatProfileImp> const& getChat() const { return _ChatProfile; }
	void setUserModelId(const std::string &userModelId);
	std::string getUserModelId();

	void setCustomLootTableId(const std::string &customLootTableId);
	std::string getCustomLootTableId();

	void setPrimAlias(uint32 alias);
	uint32 getPrimAlias() const;
	
//	void fillDescriptionMsg(CNpcBotDescriptionImp& msg) const;
	void fillDescriptionMsg(RYMSG::TGenNpcDescMsg& msg) const;
	
	
	virtual std::string	getOneLineInfoString() const { return std::string("NPC bot '") + getName() + "'"; }
	
	virtual AISHEETS::ICreatureCPtr getSheet() const { return _Sheet.getPtr(); }

	virtual void setSheet(AISHEETS::ICreatureCPtr const& sheet);

	virtual bool isSheetValid() const
	{
		return _Sheet!=NULL && _Sheet->isValid();
	}
	
	void setOutpostSide(OUTPOSTENUMS::TPVPSide side) { _OutpostSide = side; }
	
	virtual bool getFaunaBotUseBotName() const;
	
protected:
	virtual void sheetChanged();
	bool finalizeSpawnNpc();
	virtual void initAdditionalMirrorValues();
		
protected:
	// stuff supplied by CAIBot ------------------
	// sheet (.creature) for game parameters
	// position & orientation
	// mode and behaviour
	
	// A static bot object with a fauna sheet must not display SheetName but botobject name
	bool _FaunaBotUseBotName;

//	struct
//	{
//		/// Allows to hide icons for current missions steps having interactions with this NPC
//		bool		IsMissionStepIconDisplayable : 1;
//
//		/// Allows to hide icons for missions proposed by this NPC
//		bool		IsMissionGiverIconDisplayable : 1;
//	} _MissionIconFlags;

	// look parameters ---------------------------
	bool			_Hat;
	
	bool			_useVisualProperties; // true => use VisualPropertyA, B, C instead of alternate VPA
	uint64			_VisualPropertyA;
	uint64			_VisualPropertyB;
	uint64			_VisualPropertyC;
	
	CBotNpcSheetPtr	_Sheet;
	
	
	// spawn place -------------------------------
	CAIPos _StartPos;
	
	std::vector<NLMISC::CSheetId> _LootList;

	// static chat parameters --------------------
	NLMISC::CSmartPtr<CNpcChatProfileImp> _ChatProfile;
	
	// Outpost side
	OUTPOSTENUMS::TPVPSide _OutpostSide;

	// Max range a PC can hit this bot
	float _MaxHitRangeForPC;

	//userModelId used for this bot
	std::string _UserModelId;

	std::string _CustomLootTableId;

	uint32 _PrimAlias;
};

#endif
