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



#ifndef RY_GUILD_MANAGER_H
#define RY_GUILD_MANAGER_H

#include "nel/misc/types_nl.h"
#include "obs_huge_list.h"
#include "dbgroup_list_sheet_text.h"
#include "nel/misc/cdb.h"
#include "game_share/guild_grade.h"
#include "game_share/misc_const.h"

// Must be the same as in database.xml
#define MAX_GUILD_MEMBER 256

// ***************************************************************************
struct SGuildMember
{
	uint32		Index;	// Index in the DB
	uint32		NameID;
	ucstring	Name;
	EGSPD::CGuildGrade::TGuildGrade Grade;
	TCharConnectionState			Online;
	uint32		EnterDate;
	/////////////
	SGuildMember()
	{
		Online = ccs_offline;
		NameID = 0;
		Grade = EGSPD::CGuildGrade::Member;
	}
};

// ***************************************************************************
struct SGuild
{
	uint32		NameID;
	ucstring	Name;
	uint64		Icon;
	bool		QuitGuildAvailable;

	//////////////////////
	SGuild()
	{
		NameID = 0;
		Icon = 0;
		QuitGuildAvailable = true;
	}
};

// ***************************************************************************
/**
 * class used to manage the guild of the current player
 * for the moment its used only to regroup access to the good data
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date August 2003
 */
class CGuildManager
{

public:

	/// The singleton 's instance
	static CGuildManager* getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CGuildManager;
		return _Instance;
	}

	static void release()
	{
		if (_Instance != NULL)
			delete _Instance;
		_Instance = NULL;
	}

	/// Destructor
	virtual ~CGuildManager();

	const SGuild &getGuild() { return _Guild; }
	const std::vector<SGuildMember> &getGuildMembers() { return _GuildMembers; }

	enum TSortOrder
	{
		sort_grade,
		START_SORT_ORDER = sort_grade,
		sort_name,
		sort_online,
		END_SORT_ORDER
	};

	void sortGuildMembers(TSortOrder order = sort_grade);

	/// Check if the guild is a proxified guild (not managed on the actual shard)
	bool isProxy();

	/// Called from server (impulse message)
	//void init (const std::vector< std::pair<uint32,uint8> > &NameGrade);

	/// Called from server (if MemberName==0 and MemberGrade==Unknown then remove the entry)
	//void set (uint32 indexMember, uint32 MemberName, uint8 MemberGrade, bool bOnline);

	/// Indicate if the player belongs to a guild
	bool isInGuild();

	/// Tell if we can recruit (invit a player in our guild)
	bool canRecruit();

	/// Indicate if the player belongs to a guild
	bool isLeaderOfTheGuild();

	/// If the player is in a guild get the guild name else return empty
	ucstring getGuildName();

	/// If the player is in a guild get the amount of money the guild owns else return zero
	uint64 getMoney();

	/// If the player is in a guild get the XP the guild owns else return zero
	uint64 getXP();

	/// If the player is in a guild get the guild grade
	EGSPD::CGuildGrade::TGuildGrade getGrade() { return _Grade; }

	// Called once by frame to check if we have to rebuild interface
	void update();

	// Launch ascensor interface
	void launchAscensor();

	// Quit ascensor interface
	void quitAscensor();

	// Launch join proposal interface (when a guild member player wants to invite you in its guild)
	void launchJoinProposal(uint32 phraseID);

	// Close interface
	void quitJoinProposal();

	// When quitting a guild we have to close all guild interfaces
	void closeAllInterfaces();

	// Rebuild guild interface now
	void rebuildInterface();

	// Open the guild window
	void openGuildWindow();

	// Icon manipulations
	// ------------------
	// Icon is designed like this :
	// back image		: 4 bits at pos 0
	// symbol image		: 6 bits at pos 4
	// invert symbol	: 1 bits at pos 10
	// color back 1	R	: 8 bits at pos 11
	// color back 1	G	: 8 bits at pos 19
	// color back 1	B	: 8 bits at pos 27
	// color back 2	R	: 8 bits at pos 35
	// color back 2	G	: 8 bits at pos 43
	// color back 2	B	: 8 bits at pos 51

	static uint64 iconMake (uint8 back, uint8 symb, bool inv, NLMISC::CRGBA col1, NLMISC::CRGBA col2)
	{
		return ((uint64)(back&15)) | (((uint64)(symb&63))<<4) | (((uint64)(inv&1))<<10) |	\
				(((uint64)(col1.R&255))<<11) | (((uint64)(col1.G&255))<<19) | (((uint64)(col1.B&255))<<27) | \
				(((uint64)(col2.R&255))<<35) | (((uint64)(col2.G&255))<<43) | (((uint64)(col2.B&255))<<51);
	}

	static uint8 iconGetBack(uint64 icon) { return (uint8)(icon&15); }
	static uint8 iconGetSymbol(uint64 icon) { return (uint8)(icon>>4)&63; }
	static bool  iconGetInvertSymbol(uint64 icon) { return ((uint8)(icon>>10)&1)==0?false:true; }
	static NLMISC::CRGBA iconGetColor1(uint64 icon) { return NLMISC::CRGBA((uint8)(icon>>11)&255,(uint8)(icon>>19)&255,(uint8)(icon>>27)&255); }
	static NLMISC::CRGBA iconGetColor2(uint64 icon) { return NLMISC::CRGBA((uint8)(icon>>35)&255,(uint8)(icon>>43)&255,(uint8)(icon>>51)&255); }

	static void iconSetBack(uint64 &icon,uint8 n) { icon &= ~((uint64)15); icon |= ((uint64)n&15); }
	static void iconSetSymbol(uint64 &icon,uint8 n) { icon &= ~((uint64)63<<4); icon |= (((uint64)n&63)<<4); }
	static void iconSetInvertSymbol(uint64 &icon,bool inv) { icon &= ~((uint64)1<<10);	uint64 n = inv ? 1 : 0; icon |= n << 10; }
	static void iconSetColor1(uint64 &icon, NLMISC::CRGBA col) { icon &= ~((uint64)255<<11); icon &= ~((uint64)255<<19); icon &= ~((uint64)255<<27);
																icon |= ((uint64)col.R<<11); icon |= ((uint64)col.G<<19); icon |= ((uint64)col.B<<27); }
	static void iconSetColor2(uint64 &icon, NLMISC::CRGBA col) { icon &= ~((uint64)255<<35); icon &= ~((uint64)255<<43); icon &= ~((uint64)255<<51);
																icon |= ((uint64)col.R<<35); icon |= ((uint64)col.G<<43); icon |= ((uint64)col.B<<51); }


private:

	void initForDebug();

	// Rebuild (at next update) this is called internally by the observer on DB
	void rebuildBasic() { _NeedRebuild = true; }
	// Need to rebuild Members at next update (+all). this is called internally by the observer on DB
	void rebuildBasicAndMembers()  { _NeedRebuild = _NeedRebuildMembers = true; }


	// Database management stuff
	class CDBObs : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(NLMISC::ICDBNode* node);
	};
	class CDBObsMembers : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(NLMISC::ICDBNode* node);
	};

	CDBObs			_DBObs;
	CDBObsMembers	_DBObsMembers;
	friend class CDBObs;
	friend class CDBObsMembers;

	void initDBObservers();

	// need rebuild data?
	bool _NeedRebuild;
	bool _NeedRebuildMembers;
	// need update (typically names, after rebuild done)
	bool _NeedUpdate;
	bool _NeedUpdateMembers;

private:

	/// Constructor
	CGuildManager();

	/// Singleton's instance
	static CGuildManager*		_Instance;

	// Does the local player belongs to a guild ?
	bool						_InGuild;
	// Name Description and icon of the local player guild
	SGuild						_Guild;
	// Grade of local player giving administration rights
	// Leader : bear | invite | xp | (kick/set rank) members, bearer, recruiter, officer, high officer or Leader
	// High Officer : bear | invite | xp | (kick/set rank) members, bearer, recruiter or officer
	// Officer : bear | invite | kick members, recruiter or bearer
	// Recruiter : invite
	// Bearer : bear
	// Member : nothing
	// bear=can bear the banner, invite=can invite other players to the guild, xp= can spend XP guild on role masters
	// kick=can kick a player out of the guild, set rank=set the rank of a guild member
	EGSPD::CGuildGrade::TGuildGrade		_Grade;
	// Guild Members of the guild the local player belong to
	std::vector<SGuildMember>	_GuildMembers;

	// Lift handling
	CHugeListObs Ascensors;

	// flag set to true when EGS says the player has joined the guild
	bool		_NewToTheGuild;

	// Join Proposal handling
	uint32		_JoinPropPhraseID;
	ucstring	_JoinPropPhrase;
	bool		_JoinPropUpdate;
};


// ***************************************************************************
class CDBGroupListAscensor : public CDBGroupListSheetText
{
public:
	// A child node
	struct	CSheetChildAscensor : public CDBGroupListSheetText::CSheetChild
	{
		enum TAscensorEntryType
		{
			LiftTypeExit,
			LiftTypeGuild,
			LiftTypeGuildAnnexe
		};

		bool Setuped;
		uint Index;
		sint32 SecondSheetIdCache;

		virtual void init(CDBGroupListSheetText *pFather, uint index);
		virtual bool isInvalidated(CDBGroupListSheetText *pFather);
		virtual void updateViewText(CDBGroupListSheetText * /* pFather */) { }
		virtual bool isSheetValid(CDBGroupListSheetText *pFather);
	};

	CDBGroupListAscensor(const TCtorParam &param) : CDBGroupListSheetText(param)
	{
		_CheckCoordAccelerated = false; // isInvalidated called each frame
	}

	virtual CSheetChild *createSheetChild() { return new CSheetChildAscensor; }
};



#endif // RY_GUILD_MANAGER_H

/* End of guild_manager.h */
