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
#include "nel/misc/smart_ptr.h"
#include "family_profile.h"
#include "continent.h"
#include "ai_grp_fauna.h"
#include "ai_grp_npc.h"
#include "ai_mgr_fauna.h"
#include "ai_mgr_npc.h"
#include "group_profile.h"
#include "family_behavior.h"
#include "family_profile_tribe.h"

#include "continent_inline.h"

using namespace std;
using namespace NLMISC;
using namespace AITYPES;
using namespace RYAI_MAP_CRUNCH;

CAiFactory<IFamilyProfile, CFamilyProfileTribe>	_singleProfileTribe;
IAiFactory<IFamilyProfile>	*_ProfileTribe=&_singleProfileTribe;



//CPropertyId	act_fz_spawn("activity_fauna_spawn");
//
//CPropertyId	act_fz_food_herb("activity_fauna_food_herb");
//CPropertyId	act_fz_food_carn("activity_fauna_food_carn");
//CPropertyId	act_fz_rest_herb("activity_fauna_rest_herb");
//CPropertyId	act_fz_rest_carn("activity_fauna_rest_carn");
//
//CPropertyId	act_nz_harvest("activity_npc_harvest");
//CPropertyId	act_nz_ambush("activity_npc_ambush");
//CPropertyId	act_nz_rest("activity_npc_rest");
//CPropertyId	act_nz_outpost("activity_npc_outpost");
//CPropertyId	act_nz_spawn("activity_npc_spawn");
//CPropertyId	act_nz_outpost_def("activity_npc_outpost_def");
//CPropertyId	act_nz_outpost_atk("activity_npc_outpost_atk");
//CPropertyId	act_nz_kami_wander("activity_npc_kami_wander");
//CPropertyId	act_nz_escort("activity_npc_escort");		
//CPropertyId	act_nz_convoy("activity_npc_convoy");
//CPropertyId	act_nz_contact("activity_npc_contact");
//CPropertyId	act_nz_fight("activity_npc_fight");
//
//CPropertyId	act_nz_contact_camp("activity_npc_contact_camp");
//CPropertyId	act_nz_contact_outpost("activity_npc_contact_outpost");
//CPropertyId	act_nz_contact_city("activity_npc_contact_city");
////	CPropertyId	act_nz_contact_city("activity_npc_fight_boss");	problem ?
//CPropertyId	act_nz_fight_boss("act_nz_fight_boss");
//
//CPropertyId	act_fz_food_kitin("act_fz_food_kitin");
//CPropertyId	act_fz_food_kitin_invasion("act_fz_food_kitin_invasion");
//CPropertyId	act_fz_rest_kitin_invasion("act_fz_rest_kitin_invasion");
//CPropertyId	act_fz_food_degen("act_fz_food_degen");
//CPropertyId	act_fz_plant("act_fz_plant");
//CPropertyId	act_fz_rest_kitin("act_fz_rest_kitin");	
//CPropertyId	act_fz_rest_degen("act_fz_rest_degen");

// Todo:
//
//			+faire gaffe à l'init du CGrpProfileFollowRoute dans le cadre statique .. :\ (adapter?)//
//			(done)+ dynCamping n'a pas une bonne current zone, -> à mettre.
//			(done)+ problème d'ordre des points pour le suivit de chemin.
//			+ vérifier que les comportements statiques fonctionnent toujours avec le nouveau code (régression).
//
//	- Choisir le comportement au spawn des groupes.
//	- Mettre des activités dans les évènements d'OutPosts. (pas de chianlie..).
//	- Vérifer la logique des comportements (pas de trous).
//	- Vérifier que currentZone est bien géré dans les classes.
//
//	- Death report / OutPosts.
//	- getGeometry (forced by the unconceptualized finite state machine script implementation legacy in the leveldesign :( ).
//
//	- mettre un coup de boule dans le mur pour se calmer.

void	COutpostInfo::checkDespawnGroupList	()
{

	while (_DespawnList.size()>0)
	{
		CGroupNpc	*grpNpc=_DespawnList.back();		
		_DespawnList.pop_back();

		{
			TGroupList::iterator	it=_FightGroup.begin();
			const	TGroupList::iterator	itEnd=_FightGroup.end();
			
			for	(;(it!=itEnd) && ((*it).ptr()!=grpNpc);++it);
			
			if	(it!=itEnd)
			{
				(*it)->despawnGrp();
				_FightGroup.erase(it);
				continue;
			}

		}

		{
			TGroupList::iterator	it=_ContactGroups.begin();
			const	TGroupList::iterator	itEnd=_ContactGroups.end();

			for	(;(it!=itEnd) && ((*it).ptr()!=grpNpc);++it);
			
			if	(it!=itEnd)
			{
				(*it)->despawnGrp();
				_ContactGroups.erase(it);
				continue;
			}
			
		}
#ifdef NL_DEBUG
		nlassert(true==false);	//	unknown group.
#endif
	}

}

//	Not Implemented.
void	COutpostInfo::spawnBoss	()
{
}

void	COutpostInfo::outpostEvent	(ZCSTATE::TZcState	state)
{
	if	(_ZoneNpc.isNULL())
		return;

	_State=state;

	//	sets group presence in the outpost.
	switch	(_State)
	{
	case ZCSTATE::Tribe:	//	no charge
		fightGroups(false);
		bossGroups(false);
		contactGroups(true);
		break;

	case ZCSTATE::TribeInWar:	//	tribe got the outpost, tribe is in war
	case ZCSTATE::GuildInWar:	//	Guild got the outpost, Guild is in war
		fightGroups(true);
		bossGroups(true);
		contactGroups(false);
		break;

	case ZCSTATE::GuildInPeace:	//	Guild got the outpost, Guild is in peace
		fightGroups(false);
		bossGroups(false);
		contactGroups(false);
		break;
	}

}

void	COutpostInfo::fightGroups(bool	exist)
{
	return;
//	TODO
//	if	(exist)
//	{
//		if	(_FightGroupExist)	//	if no groups, spawn them ..
//			return;
//
//		CCellZone	&cellZone=_FamilyBehavior->getOwner();
//
//		for	(uint i=0; i<10; ++i)
//		{
//			// look for a fight group
//			const	CGroupDesc	*const	gd = cellZone.getOwner()->getProportionalGroupDesc(_FamilyBehavior, act_nz_fight, act_nz_fight_boss);
//			if	(!gd)
//				continue;
//
//			// set the npc of the group attackable
//			gd->setAttackable(true);
//
//			// look for a spawn point
//			const	CNpcZone	*const	spawnZone = cellZone.lookupNpcZone(/*_FamilyBehavior->getFamily(),*/ act_nz_spawn);
//			if	(!spawnZone)
//				continue;
//
//			// Look for a defense zone
//			const	CNpcZone	*const	defZone = cellZone.lookupNpcZone(/*_FamilyBehavior->getFamily(),*/ act_nz_outpost_def);
//			if	(!defZone)
//				continue;
//
//			CGroupNpc		*const	grp	=	_FamilyProfileTribe->createNpcGroup(spawnZone,gd);
//			if	(!grp)
//				continue;
//			
//			grp->setDiscardable			(false);
//			// this group will run !
//			grp->mergeProfileParameter	(CProfileParameters::TProfileParameter("running", "", 0));
//
//			CGrpProfileDynFight	*dynFight=new	CGrpProfileDynFight(grp->getSpawnObj(),_FamilyProfileTribe, spawnZone, this);
//			grp->getSpawnObj()->activityProfile().setAIProfile	(dynFight);
//			dynFight->gotoZone(defZone, act_nz_outpost_atk+act_nz_contact_camp);
//			_FightGroup.push_back(grp);
//		}
//
//	}
//	else
//	{
//		if	(!_FightGroupExist)
//			return;
//
//		for	(TGroupList::iterator	it=_FightGroup.begin(), itEnd=_FightGroup.end();it!=itEnd;++it)
//		{			
//			NLMISC::CDbgPtr<CGroupNpc>	&dbgPtr=*it;
//#ifdef NL_DEBUG
//			nlassert(!dbgPtr.isNULL());
//#endif
//			CGrpProfileDynFight	*dynFight=safe_cast<CGrpProfileDynFight*>(dbgPtr->getSpawnObj()->activityProfile().getAIProfile());
//			dynFight->gotoZone(_FamilyProfileTribe->getCampZone(),CPropertySet());
//		}
//
//	}
//	_FightGroupExist=exist;
}


void	COutpostInfo::bossGroups(bool	exist)	//	not implemented yet.
{
	_BossGroupExist=exist;
}

void	COutpostInfo::contactGroups(bool exist)	//	contact groups ..
{
//	TODO
//	if	(exist)
//	{
//		if	(_ContactGroupExist)
//			return;
//
//		// try to create a contact group
//		breakable
//		{
//			const	CGroupDesc *gd = _FamilyBehavior->getOwner()->getOwner()->getProportionalGroupDesc(_FamilyBehavior, act_nz_contact+act_nz_contact_camp, act_nz_escort+act_nz_convoy+act_nz_fight_boss);
//			if (!gd)
//				break;
//
//			CGroupNpc	*const	grp=_FamilyProfileTribe->createNpcGroup(_FamilyProfileTribe->getCampZone(),gd);
//			if (!grp)
//				break;
//
//			grp->getSpawnObj()->activityProfile().setAIProfile(new	CGrpProfileDynContact(grp->getSpawnObj(),	_FamilyProfileTribe,	this,	true));
//			_ContactGroups.push_back(grp);
//		}
//
//		//	2 other contacts
//		for	(uint32	i=0;i<2;i++)
//		{
//			// try to create a contact group
//			const	CGroupDesc *gd = _FamilyBehavior->getOwner()->getOwner()->getProportionalGroupDesc(_FamilyBehavior, act_nz_contact+act_nz_contact_outpost, act_nz_escort+act_nz_convoy);
//			if	(!gd)
//				continue;
//			
//			CGroupNpc	*const	grp=_FamilyProfileTribe->createNpcGroup(_FamilyProfileTribe->getCampZone(),gd);
//			if	(!grp)
//				continue;
//			
//			grp->getSpawnObj()->activityProfile().setAIProfile(new	CGrpProfileDynContact(grp->getSpawnObj(),	_FamilyProfileTribe,	this));
//			_ContactGroups.push_back(grp);
//
//		}
//
//	}
//	else
//	{
//		if	(!_ContactGroupExist)
//			return;
//
//		for	(TGroupList::iterator	it=_ContactGroups.begin(), itEnd=_ContactGroups.end();it!=itEnd;++it)
//		{			
//			NLMISC::CDbgPtr<CGroupNpc>	&dbgPtr=*it;
//			CGrpProfileDynFight	*dynFight=safe_cast<CGrpProfileDynFight*>(dbgPtr->getSpawnObj()->activityProfile().getAIProfile());
//			dynFight->gotoZone(_FamilyProfileTribe->getCampZone(),CPropertySet());
//		}
//
//	}
//	_ContactGroupExist=exist;
}


void	COutpostInfo::updateOutPostInfo	()
{
	checkDespawnGroupList	();
}



CFamilyProfileTribe::CFamilyProfileTribe	(const	CtorParam	&ctorParam)
	:IFamilyProfile(ctorParam)
{
	// choose a camp zone.
	// need to choose a camp zone
	static	CPropertyId	act_nz_rest("activity_rest");
	_CampZone = _FamilyBehavior->getOwner()->lookupNpcZone(_FamilyBehavior->getFamilyTag()+act_nz_rest, _FamilyBehavior->grpFamily()->getSubstitutionId());


	//	get params.
	const	NLMISC::CVirtualRefCount*const	param=ctorParam.familyBehavior()->grpFamily()->getProfileParams("aggro_groups");
	const	CAggroGroupContainer *const	aggroGroupContainer=type_cast<const	CAggroGroupContainer*>(param);
	if	(aggroGroupContainer)
		_AggroGroupIds=aggroGroupContainer->aggroGroupIds;
}



void	CFamilyProfileTribe::setDefaultProfile(const	CNpcZone	*const	zone, CGroupNpc	*grp)
{
#ifdef NL_DEBUG
	nlassert(grp);
#endif
	static	CPropertyId	act_nz_harvest("activity_harvest");
	CSpawnGroupNpc	*const	spawnGrp=grp->getSpawnObj();
	const	CNpcZone	*const	dest = getFamilyBehavior()->getOwner()->lookupNpcZone(act_nz_harvest, getFamilyBehavior()->grpFamily()->getSubstitutionId());
	if	(	!dest
		||	dest==zone)
	{
		spawnGrp->activityProfile().setAIProfile(new CGrpProfileDynWaitInZone(spawnGrp,zone));
		return;
	}
	spawnGrp->activityProfile().setAIProfile(new CGrpProfileDynHarvest(spawnGrp,this,dest,zone));
}

void	CFamilyProfileTribe::outpostAdd(NLMISC::TStringId outpostName)
{
	if	(_OutpostInfos.find(outpostName) != _OutpostInfos.end())
	{
		return;
	}

	if	(LogOutpostDebug)
		nldebug("OUTPOST: adding outpost '%s' to tribe '%s' control in '%s'",
			CStringMapper::unmap(outpostName).c_str(),
			_FamilyBehavior->getName().c_str(),
			_FamilyBehavior->getOwner()->getAliasFullName().c_str());

	
	CSmartPtr<COutpostInfo>	outPost=COutpostInfo::createOutpost(_FamilyBehavior,this,outpostName);
	if	(!outPost)
	{
		static map<CFamilyProfileTribe*, set<TStringId> > warnOnce;
		if	(warnOnce[this].find(outpostName) == warnOnce[this].end())
		{
			nlwarning("OUTPOST: no zone found for outpost '%s'", CStringMapper::unmap(outpostName).c_str());
			warnOnce[this].insert(outpostName);
		}
		return;
	}
	_OutpostInfos.insert(make_pair(outpostName, outPost));
}

void	CFamilyProfileTribe::outpostRemove(NLMISC::TStringId outpostName)
{
	TOutpostContainer::iterator it(_OutpostInfos.find(outpostName));
	if	(it==_OutpostInfos.end())
		return;

//	nlassert(it != _OutpostInfos.end());

	if (LogOutpostDebug)
		nldebug("OUTPOST: Removing outpost '%s' to tribe 'ss' control in '%s'",
			CStringMapper::unmap(outpostName).c_str(),
			/*_FamilyBehavior->getFamily().getFamilyName().c_str(),*/
			_FamilyBehavior->getOwner()->getAliasFullName().c_str());

	_OutpostInfos.erase(it);
}

void	CFamilyProfileTribe::spawnBoss(NLMISC::TStringId outpostName)
{
	TOutpostContainer::iterator it(_OutpostInfos.find(outpostName));
	if	(it==_OutpostInfos.end())
		return;

//	nlassert(it != _OutpostInfos.end());
	it->second->spawnBoss();
}

void	CFamilyProfileTribe::spawnGroup()
{
	H_AUTO(FamilySpawnTribe)
	static	CPropertyId	act_nz_spawn("activity_spawn");
		
	if	(getCampZone().isNull())
		return;

	static	CPropertyId	act_nz_rest("activity_rest");
	const	CNpcZone	*const	spawn = _FamilyBehavior->getOwner()->lookupNpcZone(/*_FamilyBehavior->getFamily(),*/ act_nz_spawn+act_nz_rest, _FamilyBehavior->grpFamily()->getSubstitutionId());
	if	(!spawn)
		return;

	
	static	CPropertyId	act_nz_escort("activity_escort");		
	static	CPropertyId	act_nz_contact("activity_contact");
	static	CPropertyId	act_nz_fight_boss("act_nz_fight_boss");
	
//	CCellZone	&cellZone=_FamilyBehavior->getOwner();
	const	CGroupDesc<CGroupFamily>	*const	gd = _FamilyBehavior->grpFamily()->getProportionalGroupDesc(_FamilyBehavior, CPropertySet(), act_nz_contact+act_nz_fight_boss+act_nz_escort);

	if	(!gd)
		return;

	// set the npc of the group attackable
//	gd->setAttackable(true);
	// set the npc of the group vulnerable
//	gd->setVulnerable(true);

	const	CGroupNpc	*const	grp=createNpcGroup(spawn,gd);

	if	(!grp)
		return;

	grp->getSpawnObj()->spawnBotOfGroup();
}


/// The main update for the profile. Called aprox every 10 s (100 ticks)
void	CFamilyProfileTribe::update()
{
	H_AUTO(FamilyTribeUpdate)

	// update outposts
	TOutpostContainer::iterator first(_OutpostInfos.begin()), last(_OutpostInfos.end());
	for	(; first != last; ++first)
		first->second->updateOutPostInfo();
}


/************************************************************************/
/* Profiles                                                             */
/************************************************************************/

//---------------------------------------------------------------------------------
// CGrpProfileDynContact
//---------------------------------------------------------------------------------

void	CGrpProfileDynContact::beginProfile()
{
	static	CPropertyId	act_nz_rest("activity_rest");
	static	CPropertyId	act_nz_outpost("activity_outpost");
	_Grp->movingProfile().setAIProfile(new CGrpProfileDynFollowPath(_Grp, _CurrentZone, _FamilyProfile->getCampZone(), act_nz_outpost + act_nz_rest));
}

// routine called just before bot starts to use a new profile or when a bot dies
void	CGrpProfileDynContact::endProfile()
{
}

// routine called every time the bot is updated (frequency depends on player proximity, etc)

//	there's a coherence problem about the activity.
void	CGrpProfileDynContact::updateProfile(uint ticksSinceLastUpdate)
{
	
	// this is a contact group, special treatment
	switch	(_Grp->movingProfile().getAIProfileType())
	{
	case	ZONE_WAIT:
		// we must be in a city or in an outpost, just check the date to return to camp.
		if	(CTimeInterface::timeOfDay()!=CRyzomTime::nightfall)
			break;
		{
			//	go back to the camp.
			CGrpProfileDynWaitInZone const* const waitProfile = safe_cast<CGrpProfileDynWaitInZone*>(_Grp->movingProfile().getAIProfile());
			_Grp->movingProfile().setAIProfile(new CGrpProfileDynFollowPath(_Grp, waitProfile->currentZone(), _FamilyProfile->getCampZone(), CPropertySet()));
		}
		break;

	case	MOVE_CAMPING:
		// the group is camping, if the day rise, send them to an outpost or city
		if	(CTimeInterface::timeOfDay()!=CRyzomTime::dawn)
			break;

		{
			static	CPropertyId	act_nz_outpost("activity_outpost");
			// time to go work childrens! 
			const	CNpcZone *dest =	_FamilyProfile->getFamilyBehavior()->getOwner()->lookupNpcZone(/*_FamilyProfile->getFamilyBehavior()->getFamily(),*/ act_nz_outpost, _FamilyProfile->getFamilyBehavior()->grpFamily()->getSubstitutionId());
			if	(dest)
			{
				CGrpProfileDynCamping const* const campingProfile = safe_cast<CGrpProfileDynCamping*>(_Grp->movingProfile().getAIProfile());
				_Grp->movingProfile().setAIProfile(new CGrpProfileDynFollowPath(_Grp, campingProfile->currentZone(), dest, CPropertySet()));
			}
		}
		break;

	case	MOVE_DYN_FOLLOW_PATH:
		{
			CGrpProfileDynFollowPath* profile = NLMISC::safe_cast<CGrpProfileDynFollowPath*>(_Grp->movingProfile().getAIProfile());

			//	if arrived.
			if	(!profile->destinationReach())
				break;
			
			if	(!_OutPostInfo->_ContactGroupExist)
			{
				_OutPostInfo->addToDespawnGroupList(_Grp);
				return;
			}

			// ok, we are at the road end, what should we do now ?
			if	(_isTheContactGroup)
				_Grp->activityProfile().setAIProfile(new CGrpProfileDynCamping(_Grp,profile->currentZone()));
			else
				_Grp->activityProfile().setAIProfile(new CGrpProfileDynWaitInZone(_Grp,profile->currentZone()));
		}
		break;
	default:
#ifdef NL_DEBUG
			nlassert(true==false);
#endif
		break;
	}

	CGrpProfileNormal::updateProfile(ticksSinceLastUpdate);
}

void	CGrpProfileDynContact::gotoZone(const	CNpcZone	*const	zone, const	CPropertySet	&flags)
{
	_Grp->movingProfile().setAIProfile(new CGrpProfileDynFollowPath(_Grp, _CurrentZone, zone, flags));
}

string	CGrpProfileDynContact::buildDebugString()	const
{
	return	string();
}

//---------------------------------------------------------------------------------
// CGrpProfileDynHarvest
//---------------------------------------------------------------------------------


void	CGrpProfileDynHarvest::beginProfile()
{
	static	CPropertyId	act_nz_rest("activity_rest");
	static	CPropertyId	act_nz_harvest("activity_harvest");
	static	CPropertyId	act_nz_outpost("activity_outpost");
	_Grp->movingProfile().setAIProfile(new CGrpProfileDynFollowPath(_Grp, _CurrentZone, _HarvestZone, act_nz_outpost + act_nz_rest));
}

// routine called just before bot starts to use a new profile or when a bot dies
void	CGrpProfileDynHarvest::endProfile()
{
}

// routine called every time the bot is updated (frequency depends on player proximity, etc)


void	CGrpProfileDynHarvest::checkTargetsAround	()
{
	if	(!_checkTargetTimer.test())
		return;

	_checkTargetTimer.set(10+CAIS::rand16(2));	//	every 11 seconds.

	if	(_FamilyProfile->_AggroGroupIds.size()==0)
		return;

	//	check if we have a player property.
	{
		const	std::vector<uint32>	&aggroList=_FamilyProfile->_AggroGroupIds;
		if	(std::find(aggroList.begin(), aggroList.end(), AISHEETS::CSheets::getInstance()->playerGroupIndex())==aggroList.end())
			return;
	}
	
	CAIVision<CPersistentOfPhysical>	Vision;
	
	breakable
	{
		CAIVector	centerPos;
		if	(!_Grp->calcCenterPos(centerPos))	// true if there's some bots in the group.
			break;

		const	uint32	playerRadius=	uint(30);	//	_AggroRange);
		const	uint32	botRadius=uint(0);		//	_AggroRange);
		
		const	uint32	minRadius=playerRadius>botRadius?botRadius:playerRadius;
		
		Vision.updateBotsAndPlayers(_Grp->getPersistent().getAIInstance(), centerPos, playerRadius, botRadius);
	}

	{
		const std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> > &players = Vision.players();
		
		std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >::const_iterator	first(players.begin()), last(players.end());
		for (; first != last; ++first)
		{
			const	CPersistentOfPhysical*const	player = (*first);
			const	CAIEntityPhysical*const		ep = player->getSpawnObj();
			if	(	!ep
				||	!ep->isAlive()
				||	ep->currentHitPoints()<=0.f	)
				continue;

			const	CRootCell	*const	rootCell=ep->wpos().getRootCell();
			if	(	rootCell
				&&	rootCell->getFlag()!=0	)	//	Safe Zone ?
				continue;
			
			_Grp->setAggroMinimumFor(ep->dataSetRow(), 0.5f, false);
		}
	}
}

//	there's a coherence problem about the activity.
void	CGrpProfileDynHarvest::updateProfile(uint ticksSinceLastUpdate)
{
	static	CPropertyId	act_nz_harvest("activity_harvest");
	CProfilePtr	&movingProfile=_Grp->movingProfile();

	// this is a contact group, special treatment
	switch	(movingProfile.getAIProfileType())
	{
	//	Camping -> Moving(Wandering(Harvesting))
	case	MOVE_CAMPING:
		{
			// the group is resting in the camp, check activity timeout
			CGrpProfileDynCamping const* const profile = safe_cast<CGrpProfileDynCamping*>(movingProfile.getAIProfile());
			if	(!profile->timeOut())
				break;

			// send the group to a harvest point
			const	CNpcZone	*const	dest = _FamilyProfile->getFamilyBehavior()->getOwner()->lookupNpcZone(/*_FamilyProfile->getFamilyBehavior()->getFamily(),*/ act_nz_harvest, _FamilyProfile->getFamilyBehavior()->grpFamily()->getSubstitutionId());

			if (dest==profile->currentZone())	// are we already at the right place ?
			{
				//	then lets camp again ..
				_Grp->movingProfile().setAIProfile(new CGrpProfileDynCamping(_Grp,dest));
			}
			else
			{
				// send the group to an harvest site.
				movingProfile.setAIProfile(new CGrpProfileDynFollowPath(_Grp, profile->currentZone(), dest, CPropertySet()));
			}
			break;
		}
		break;

	//	Moving -> Wandering(Harvesting)|Camping
	case	MOVE_DYN_FOLLOW_PATH:
		{			
			CGrpProfileDynFollowPath const* const fp = safe_cast<CGrpProfileDynFollowPath*>(movingProfile.getAIProfile());
			
			if	(!fp->destinationReach())
				break;

			if	(fp->currentZone()==_FamilyProfile->getCampZone())
			{
				// the group just enter the camp, do a camping for some time
				_Grp->movingProfile().setAIProfile(new CGrpProfileDynCamping(_Grp,fp->currentZone()));
				break;
			}

			// ok the group has reach a destination that is not the camp ..
			// lets forage ..
			// set the group in wander/forage mode (with zone in param)
			CGrpProfileWander *profile=new	CGrpProfileWander(_Grp,fp->currentZone());
			_Grp->movingProfile().setAIProfile(profile);

			// do some foraging
			profile->setBotStandProfile(BOT_FORAGE, &BotProfileForageFactory);
			profile->setTimer(60*10+CAIS::rand32(30*10));	// between 1 to 1:30 minute long ..
		}
		break;

	//	Harvesting -> Moving(Camping)
	case	MOVE_WANDER:
		{
			// this is a harvest group doing foraging
			const	CGrpProfileWander	* const	profile=safe_cast<CGrpProfileWander*>(movingProfile.getAIProfile());
			if	(!profile->testTimer())
				break;

			// send the group to a harvest point
			const	CNpcZone	*dest=NULL;
			
			while	(	!dest
					||	dest==profile->currentZone())
			{
				if	(CAIS::rand32(9)==0)
					dest=_FamilyProfile->getCampZone();
				else
					dest=_FamilyProfile->getFamilyBehavior()->getOwner()->lookupNpcZone(/*_FamilyProfile->getFamilyBehavior()->getFamily(),*/ act_nz_harvest, _FamilyProfile->getFamilyBehavior()->grpFamily()->getSubstitutionId());
			};

			// send the group to the camp.
			movingProfile.setAIProfile(new CGrpProfileDynFollowPath(_Grp, profile->currentZone(), dest, CPropertySet()));
		}
		break;

	default:	//	don't think we have to be there ..
#ifdef NL_DEBUG
			nlassert(true==false);
#endif
		break;
	}
	checkTargetsAround	();		
	CGrpProfileNormal::updateProfile(ticksSinceLastUpdate);
}

string	CGrpProfileDynHarvest::buildDebugString()	const
{
	return	string();
}



//---------------------------------------------------------------------------------
// CGrpProfileDynFight
//---------------------------------------------------------------------------------

void	CGrpProfileDynFight::beginProfile()
{
	static	CPropertyId	act_nz_rest("activity_rest");
	static	CPropertyId	act_nz_harvest("activity_harvest");
	static	CPropertyId	act_nz_outpost("activity_outpost");
	
	const	CNpcZone	*const	dest = _FamilyProfile->getFamilyBehavior()->getOwner()->lookupNpcZone(/*_FamilyProfile->getFamilyBehavior()->getFamily(),*/ act_nz_harvest, _FamilyProfile->getFamilyBehavior()->grpFamily()->getSubstitutionId());
	_Grp->movingProfile().setAIProfile(new CGrpProfileDynFollowPath(_Grp, _CurrentZone, dest, act_nz_outpost + act_nz_rest + act_nz_harvest));
}

// routine called just before bot starts to use a new profile or when a bot dies
void	CGrpProfileDynFight::endProfile()
{
}

// routine called every time the bot is updated (frequency depends on player proximity, etc)

//	there's a coherence problem about the activity.
void	CGrpProfileDynFight::updateProfile(uint ticksSinceLastUpdate)
{
	static	CPropertyId	act_nz_outpost("activity_outpost");
	CProfilePtr	&movingProfile=_Grp->movingProfile();
	
	// this is a contact group, special treatment
	switch	(movingProfile.getAIProfileType())
	{
	//	Camping -> Moving(Wandering(Fighting))
	case	MOVE_CAMPING:
		{
			// the group is resting in the camp, check activity timeout
			CGrpProfileDynCamping const* const profile = safe_cast<CGrpProfileDynCamping*>(movingProfile.getAIProfile());
#ifdef NL_DEBUG
			nlassert(_CurrentZone==profile->currentZone());
#endif
			if	(!profile->timeOut())
				break;

			// send the group to a harvest point
			const	CNpcZone	*const	dest = _FamilyProfile->getFamilyBehavior()->getOwner()->lookupNpcZone(/*_FamilyProfile->getFamilyBehavior()->getFamily(),*/ act_nz_outpost, _FamilyProfile->getFamilyBehavior()->grpFamily()->getSubstitutionId());
			// send the group to an harvest site.
			movingProfile.setAIProfile(new CGrpProfileDynFollowPath(_Grp, profile->currentZone(), dest, CPropertySet()));
			break;
		}							
		break;

	//	Moving -> Wandering(Fighting)|Camping
	case	MOVE_DYN_FOLLOW_PATH:
		{			
			CGrpProfileDynFollowPath const* const fp = safe_cast<CGrpProfileDynFollowPath*>(movingProfile.getAIProfile());
			
			_CurrentZone=fp->currentZone();

			if	(!fp->destinationReach())
				break;

			if	(!_OutPostInfo->_FightGroupExist)
			{
				_OutPostInfo->addToDespawnGroupList(_Grp);
				return;
			}

			if	(fp->currentZone()==_FamilyProfile->getCampZone())
			{
				// the group just enter the camp, do a camping for some time
				_Grp->movingProfile().setAIProfile(new CGrpProfileDynCamping(_Grp,fp->currentZone()));
				break;
			}

			// ok the group has reach a destination that is not the camp ..
			// lets forage ..
			// set the group in wander/forage mode (with zone in param)
			CGrpProfileWander* profile = new CGrpProfileWander(_Grp,fp->currentZone());
			_Grp->movingProfile().setAIProfile(profile);

			profile->setTimer(30*60*10+CAIS::rand32(15*50*10));
		}
		break;

	//	Fighting -> Moving(Camping)
	case	MOVE_WANDER:
		{
			// this is a harvest group doing foraging
			CGrpProfileWander const* const profile = safe_cast<CGrpProfileWander*>(movingProfile.getAIProfile());
#ifdef NL_DEBUG
			nlassert(_CurrentZone==profile->currentZone());
#endif

			if	(!profile->testTimer())
				break;

			// send the group to the camp.
			movingProfile.setAIProfile(new CGrpProfileDynFollowPath(_Grp, profile->currentZone(), _FamilyProfile->getCampZone(), CPropertySet()));
		}
		break;

	default:	//	don't think we have to be there ..
#ifdef NL_DEBUG
			nlassert(true==false);
#endif
		break;
	}
	CGrpProfileNormal::updateProfile(ticksSinceLastUpdate);
}


void	CGrpProfileDynFight::gotoZone(const	CNpcZone	*const	zone, const	CPropertySet	&flags)
{
	_Grp->movingProfile().setAIProfile(new CGrpProfileDynFollowPath(_Grp, _CurrentZone, zone, flags));
}

string	CGrpProfileDynFight::buildDebugString()	const
{
	return	string();
}

