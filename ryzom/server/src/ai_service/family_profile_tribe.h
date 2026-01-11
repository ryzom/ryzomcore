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



#ifndef FAMILY_PROFILE_TRIBE_H
#define FAMILY_PROFILE_TRIBE_H

extern NLMISC::CVariable<bool>	LogOutpostDebug;

class	CFamilyProfileTribe;

/// outpost information
class	COutpostInfo
	:public	NLMISC::CRefCount
{
	COutpostInfo	(CFamilyBehavior	*const	familyBehavior, const	CFamilyProfileTribe	*const	familyProfileTribe, const	CNpcZone*const	zoneNpc)
		:_State(ZCSTATE::Tribe)
		,_FamilyBehavior(familyBehavior)
		,_FamilyProfileTribe(familyProfileTribe)
		,_ZoneNpc(zoneNpc)
		,_FightGroupExist(false)
		,_BossGroupExist(false)
		,_ContactGroupExist(false)
	{
	}

public:

	static	NLMISC::CSmartPtr<COutpostInfo>	createOutpost	(CFamilyBehavior	*const	familyBehavior,	const	CFamilyProfileTribe	*const	familyProfileTribe, const	NLMISC::TStringId &outpostName)
	{
		const	CNpcZone	*const	zoneNpc=familyBehavior->getOwner()->lookupNpcZoneByName(/*familyBehavior->getFamily(), */NLMISC::CStringMapper::unmap(outpostName));

		if (	!zoneNpc
			||	!familyProfileTribe
			||	!familyBehavior)
			return	NULL;

		return	new	COutpostInfo(familyBehavior, familyProfileTribe, zoneNpc);
	}

	typedef	std::vector<NLMISC::CDbgPtr<CGroupNpc> >	TGroupList;

	virtual	~COutpostInfo()
	{
		for	(TGroupList::iterator	it=_FightGroup.begin(), itEnd=_FightGroup.end();it!=itEnd;++it)
		{
			NLMISC::CDbgPtr<CGroupNpc>	&dbgPtr=*it;
			if	(!dbgPtr.isNULL())
				dbgPtr->despawnGrp();
		}

		for	(TGroupList::iterator	it=_ContactGroups.begin(), itEnd=_ContactGroups.end();it!=itEnd;++it)
		{
			NLMISC::CDbgPtr<CGroupNpc>	&dbgPtr=*it;
			if	(!dbgPtr.isNULL())
				dbgPtr->despawnGrp();
		}

		if (!_BossGroup.isNULL())
		{
			_BossGroup->despawnGrp();
		}

	}

	void	spawnBoss	();

	void	outpostEvent	(ZCSTATE::TZcState	state);

	void	updateOutPostInfo	();	

	void	fightGroups(bool	exist);
	void	bossGroups(bool	exist);
	void	contactGroups(bool	exist);

	void	addToDespawnGroupList	(CSpawnGroupNpc	*grpNpc)
	{
		grpNpc->activityProfile().setAIProfile(NULL);	//	remove comportment.
		_DespawnList.push_back(&grpNpc->getPersistent());
	}

	void	checkDespawnGroupList	();

private:
	/// The npc zone for the outpost
	NLMISC::CstCDbgPtr<CNpcZone>	_ZoneNpc;

	/// The current outpost state.
	ZCSTATE::TZcState	_State;
	
	TGroupList		_ContactGroups;
	/// Fight Groups.
	TGroupList		_FightGroup;

	TGroupList	_DespawnList;

	/// Pointer on the boss group when present.
	NLMISC::CDbgPtr<CGroupNpc>	_BossGroup;

	NLMISC::CDbgPtr<CFamilyBehavior>		_FamilyBehavior;
	NLMISC::CDbgPtr<CFamilyProfileTribe>	_FamilyProfileTribe;
	
public:
	bool	_FightGroupExist;
	bool	_BossGroupExist;
	bool	_ContactGroupExist;
};

class	CFamilyProfileTribe
		:public	NLMISC::CDbgRefCount<CFamilyProfileTribe>
		,public	IFamilyProfile
{
	/// The zone that is used as camp for the tribe.
	NLMISC::CSmartPtr<const CNpcZone>	_CampZone;	//	smart to have a counter on NpcZone. (Bad but no time to do better).

	/// The contact group in the camp
	NLMISC::CstCDbgPtr<CGroupNpc>		_CampContact;
public:
	std::vector<uint32>	_AggroGroupIds;


	CFamilyProfileTribe	(const	CtorParam	&ctorParam);
	
	virtual	~CFamilyProfileTribe	()
	{
	}

	CFamilyBehavior	*getFamilyBehavior	()
	{
		return	_FamilyBehavior;
	}

		/// The zone that is used as camp for the tribe.
	const	NLMISC::CSmartPtr<const CNpcZone>	&getCampZone()	const
	{
		return	_CampZone;
	}
	void	setCampZone	(const	CNpcZone	*campZone)
	{
		_CampZone=campZone;
	}

	void	spawnBoss(NLMISC::TStringId outpostName);

	void	setDefaultProfile(const	CNpcZone	*const	zone, CGroupNpc	*grp);

	typedef std::map<NLMISC::TStringId, NLMISC::CSmartPtr<COutpostInfo> >	TOutpostContainer;
	TOutpostContainer	_OutpostInfos;

	void	fillOutpostNames(std::vector<NLMISC::TStringId> outpostNames)
	{
		outpostNames.clear();
		for (TOutpostContainer::const_iterator first(_OutpostInfos.begin()), last(_OutpostInfos.end()); first != last; ++first)
			outpostNames.push_back(first->first);
	}


	void	outpostAdd(NLMISC::TStringId outpostName);
	void	outpostRemove(NLMISC::TStringId outpostName);

	void	outpostEvent(NLMISC::TStringId outpostName, ZCSTATE::TZcState	state)
	{
		TOutpostContainer::iterator it(_OutpostInfos.find(outpostName));
		if	(it==_OutpostInfos.end())
			return;

//		nlassert(it != _OutpostInfos.end());		
		it->second->outpostEvent(state);
	};
			
	void	spawnGroup();

	/// The main update for the profile. Called aprox every 10 s (100 ticks)
	void	update();

};
extern	IAiFactory<IFamilyProfile>	*_ProfileTribe;


//---------------------------------------------------------------------------------
// CGrpProfileDynContact
//---------------------------------------------------------------------------------
//
//

class CGrpProfileDynContact
: public CGrpProfileNormal
{
public:
		CGrpProfileDynContact(CProfileOwner *const	owner,	CFamilyProfileTribe	*const	familyProfile, COutpostInfo	*const	outPostInfo,	bool isTheContactGroup=false) 
		:CGrpProfileNormal(owner)
		,_FamilyProfile(familyProfile)
		,_isTheContactGroup(isTheContactGroup)
		,_OutPostInfo(outPostInfo)
	{
		_CurrentZone=_FamilyProfile->getCampZone();
		// this group can't be despawnded
		_Grp->getPersistent().setDiscardable(!isTheContactGroup);
	}

	virtual ~CGrpProfileDynContact()
	{
	}
			
	//---------------------------------------------------------------------------------------------------------
	// virtual routines used to mange behaviour of bot
	// note that this code is responsible for setting the 'nextActivity' bot property and may modify 
	// the 'activityTimer' property - may also set the 'nextTarget' property
	
	// routine called when a bot starts to use a given profile
	// note that bots have a data member called 'void *aiProfileData' reserved for the use of the profile code
	// this data member should be setup here if it is to be used by the profile
	virtual void beginProfile();
	
	// routine called just before bot starts to use a new profile or when a bot dies
	virtual void endProfile();
	// routine called every time the bot is updated (frequency depends on player proximity, etc)
	virtual void updateProfile(uint ticksSinceLastUpdate);

	// routine used to build a debug string for detailed information on a bot's status (with respect to their aiProfile)
	virtual std::string buildDebugString() const;
	
	virtual	AITYPES::TProfiles getAIProfileType () const
	{
		return	AITYPES::ACTIVITY_CONTACT;
	}

	void	gotoZone(const	CNpcZone	*const	zone, const	AITYPES::CPropertySet	&flags);
	
private:
	COutpostInfo	*const			_OutPostInfo;
	CFamilyProfileTribe	*const		_FamilyProfile;
	NLMISC::CstCDbgPtr<CNpcZone>	_CurrentZone;
	const	bool	_isTheContactGroup;
};






//---------------------------------------------------------------------------------
// CGrpProfileDynHarvest
//---------------------------------------------------------------------------------


class CGrpProfileDynHarvest: public CGrpProfileNormal
{
public:
		CGrpProfileDynHarvest(CProfileOwner *const	owner,	CFamilyProfileTribe	*const	familyProfile, const	CNpcZone	*const	harvestZone, const	CNpcZone	*const	currentZone)
		:CGrpProfileNormal(owner)
		,_FamilyProfile(familyProfile)
		,_HarvestZone(harvestZone)
	{
		_CurrentZone=currentZone;
	}

	virtual ~CGrpProfileDynHarvest()
	{
	}
			
	//---------------------------------------------------------------------------------------------------------
	// virtual routines used to mange behaviour of bot
	// note that this code is responsible for setting the 'nextActivity' bot property and may modify 
	// the 'activityTimer' property - may also set the 'nextTarget' property
	
	// routine called when a bot starts to use a given profile
	// note that bots have a data member called 'void *aiProfileData' reserved for the use of the profile code
	// this data member should be setup here if it is to be used by the profile
	virtual void beginProfile();
	
	// routine called just before bot starts to use a new profile or when a bot dies
	virtual void endProfile();
	// routine called every time the bot is updated (frequency depends on player proximity, etc)
	virtual void updateProfile(uint ticksSinceLastUpdate);

	void	checkTargetsAround	();
		
	// routine used to build a debug string for detailed information on a bot's status (with respect to their aiProfile)
	virtual std::string buildDebugString() const;
	
	virtual	AITYPES::TProfiles getAIProfileType () const
	{
		return	AITYPES::ACTIVITY_HARVEST;
	}
	
private:
	CAITimer	_checkTargetTimer;

	CFamilyProfileTribe	*const		_FamilyProfile;
	NLMISC::CstCDbgPtr<CNpcZone>	_CurrentZone;
	NLMISC::CstCDbgPtr<CNpcZone>	const	_HarvestZone;
};


//---------------------------------------------------------------------------------
// CGrpProfileDynHarvest
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// CGrpProfileDynFight
//---------------------------------------------------------------------------------
//
//

class CGrpProfileDynFight: public CGrpProfileNormal
{
public:
		CGrpProfileDynFight(CProfileOwner *const	owner,	CFamilyProfileTribe	*const	familyProfile, const	CNpcZone*const	zone, COutpostInfo	*const	outPostInfo)
		:CGrpProfileNormal(owner)
		,_FamilyProfile(familyProfile)
		,_CurrentZone(zone)
		,_OutPostInfo(outPostInfo)
	{
	}

	virtual ~CGrpProfileDynFight()
	{
	}
			
	//---------------------------------------------------------------------------------------------------------
	// virtual routines used to mange behaviour of bot
	// note that this code is responsible for setting the 'nextActivity' bot property and may modify 
	// the 'activityTimer' property - may also set the 'nextTarget' property
	
	// routine called when a bot starts to use a given profile
	// note that bots have a data member called 'void *aiProfileData' reserved for the use of the profile code
	// this data member should be setup here if it is to be used by the profile
	virtual void beginProfile();
	
	// routine called just before bot starts to use a new profile or when a bot dies
	virtual void endProfile();
	// routine called every time the bot is updated (frequency depends on player proximity, etc)
	virtual void updateProfile(uint ticksSinceLastUpdate);

	// routine used to build a debug string for detailed information on a bot's status (with respect to their aiProfile)
	virtual std::string buildDebugString() const;
	
	virtual	AITYPES::TProfiles getAIProfileType () const
	{
		return	AITYPES::ACTIVITY_FIGHT;
	}

	void	gotoZone(const	CNpcZone	*const	zone, const	AITYPES::CPropertySet	&flags);
	
private:
	COutpostInfo	*const			_OutPostInfo;
	CFamilyProfileTribe	*const		_FamilyProfile;
	NLMISC::CstCDbgPtr<CNpcZone>	_CurrentZone;
};

#endif
