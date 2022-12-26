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
#include "family_behavior.h"
#include "game_share/fame.h"
#include "continent.h"
#include "ai_instance.h"
#include "ai_grp_npc.h"
#include "ai_grp_fauna.h"

#include "continent_inline.h"
#include "dyn_grp_inline.h"

using namespace MULTI_LINE_FORMATER;

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace AITYPES;



CFamilyBehavior::CFamilyBehavior(CCellZone *owner, const	CGroupFamily	*grpFamily)
: CChild<CCellZone>(owner)
,_BaseLevel(0)
,_EffectiveLevel(0)
,_PlayerEffect(0)
,_CurrentLevel(0)
,_TheoricalLevel(0)
,_LastUpdateTime(CTimeInterface::gameCycle()+CAIS::rand32(100))
,_UpdatePeriod(1)
,_GrpFamily(grpFamily)
{
	// create the family behavior
	_FamilyProfile = IFamilyProfile::createFamilyProfile(grpFamily->profileName(),IFamilyProfile::CtorParam(this));
	
	_ManagerNpc		=	new	CMgrNpc(this, 0, getName()+":npc_manager", "");
	_ManagerFauna	=	new	CMgrFauna(this, 0, getName()+":fauna_manager", "");
	_ManagerNpc->spawn();
	_ManagerFauna->spawn();
	for	(uint32	i=0;i<4;i++	)
	{
		_Modifier[i]=1;
	}
	// tmp nico 
	/*
	nlinfo("creating new family beahviour, activities : ");
	nlinfo("FOOD");
	{
		std::set<NLMISC::TStringId> &props = grpFamily->getProfileProperty("food").properties();
		std::set<NLMISC::TStringId>::iterator it;
		for (it = props.begin(); it != props.end(); ++it)
		{
			nlinfo(NLMISC::CStringMapper::unmap(*it).c_str());
		}
	}
	nlinfo("REST");
	{
		std::set<NLMISC::TStringId> &props = grpFamily->getProfileProperty("rest").properties();
		std::set<NLMISC::TStringId>::iterator it;
		for (it = props.begin(); it != props.end(); ++it)
		{
			nlinfo(NLMISC::CStringMapper::unmap(*it).c_str());
		}
	}
	*/

}

std::string	CFamilyBehavior::getName() const
{
	return	_GrpFamily->getName();
}


CAIInstance* CFamilyBehavior::getAIInstance() const
{
	return getOwner()->getAIInstance();
}


const	std::string	&getLevelString	(const	uint32	&levelIndex)
{
	static	std::string	s0("0-25");
	static	std::string	s1("25-50");
	static	std::string	s2("50-75");
	static	std::string	s3("75-100");
	static	std::string	invalid("InvalidLevel");

	switch	(levelIndex)
	{
	case 0:
		return	s0;
	case 1:
		return	s1;
	case 2:
		return	s2;
	case 3:
		return	s3;
	default:
		return	invalid;
	}
}

uint32	CFamilyBehavior::energyScale	(uint32 levelIndex)	const
{
	if (levelIndex==~0)
		levelIndex=getLevelIndex();
	
	return	(uint32)(_GrpFamily->levelEnergyValue(levelIndex)*(double)_Modifier[levelIndex]*(double)AITYPES::ENERGY_SCALE);
}

void	CFamilyBehavior::displayLogOld	(CStringWriter	&stringWriter, bool detailled)
{
/*
	string	res="in "+getCellZone()->getAliasFullName();
	res+=", GroupFamily"+getName();
	res+="\t NrjLvl="+toString(getLevelIndex());
	res+=":"+getLevelString(getLevelIndex());
	res+="("+toString(effectiveLevel()/(float)ENERGY_SCALE);
	res+=") CurNrjScale="+toString(_CurrentLevel/(float)ENERGY_SCALE);
	res+=" FinalNrjScale["+getLevelString(getLevelIndex());
	res+="]="+toString(energyScale()/(float)ENERGY_SCALE);
	res+=" (NrjScale="+toString(_GrpFamily->levelEnergyValue(getLevelIndex()));
	res+="*Modifier="+toString(_Modifier[getLevelIndex()]);
	res+=") Theorical="+toString(_TheoricalLevel/(float)ENERGY_SCALE);
	
	stringWriter.append(res);
	
	if (!detailled)
		return;
	
	for	(uint32 i=0;i<4;i++)
	{
		stringWriter.append("		> "+getLevelString(i)+" \t: FinalEnergyScale "
			+toString(energyScale(i)/(float)ENERGY_SCALE)
			+" (EnergyScale="+toString(_GrpFamily->levelEnergyValue(i))
			+" Modifier="+toString(_Modifier[i])
			+")");
	}
	
*/
	string const& celZon = getCellZone()->getAliasFullName();
	string const& grpFam = getName();
	string const& lvlIdx = toString(getLevelIndex());
	string const& lvlIdxStr = getLevelString(getLevelIndex());
	string const& effLvl = toString(effectiveLevel()/(float)ENERGY_SCALE);
	string const& curLvl = toString(_CurrentLevel/(float)ENERGY_SCALE);
	string const& finLvl = toString(energyScale()/(float)ENERGY_SCALE);
	string const& teoLvl = toString(_TheoricalLevel/(float)ENERGY_SCALE);
	string const& lvlNrgVal = toString(_GrpFamily->levelEnergyValue(getLevelIndex()));
	string const& modifier = toString(_Modifier[getLevelIndex()]);
	
	string log = 
		"in "+celZon+", GroupFamily"+grpFam+"\t NrjLvl="+lvlIdx+":"+lvlIdxStr+"("+effLvl+") CurNrjScale="+curLvl
		+" FinalNrjScale["+lvlIdxStr+"]="+finLvl+" (NrjScale="+lvlNrgVal+"*Modifier="+modifier+") Theorical="+teoLvl;
	
	stringWriter.append(log);
	
	if (!detailled)
		return;
	
	for	(uint32 i=0;i<4;i++)
	{
		stringWriter.append("		> "+getLevelString(i)+" \t: FinalEnergyScale "
			+toString(energyScale(i)/(float)ENERGY_SCALE)
			+" (EnergyScale="+toString(_GrpFamily->levelEnergyValue(i))
			+" Modifier="+toString(_Modifier[i])
			+")");
	}
}

void CFamilyBehavior::displayLogHeaders(CStringWriter& stringWriter, int index, bool detailled, std::vector<size_t> widths)
{
	vector<string> cols(9, "");
	cols[0] = "CellZone";
	cols[1] = "GroupFamily";
	cols[2] = "Levels for";
	cols[3] = "effective";
	cols[4] = "current";
	cols[5] = "final";
	cols[6] = "theorical";
	cols[7] = "value";
	cols[8] = "modifier";
	
	for (size_t j=0; j<cols.size(); ++j)
		for (size_t i=cols[j].length(); i<widths[j]; ++i)
			cols[j] += " ";
	
	string log = string("| ")+cols[0]+" | "+cols[1]+" | "+cols[2]+" | "+cols[3]+" | "+cols[4]+" | "+cols[5]+" | "+cols[6]+" | "+cols[7]+" | "+cols[8]+" |";
	stringWriter.append(log);
}

void CFamilyBehavior::displayLogLine(CStringWriter& stringWriter, int index, bool detailled, std::vector<size_t> widths)
{
	vector<string> cols(9, "");
	
	for (size_t j=0; j<cols.size(); ++j)
		for (size_t i=cols[j].length(); i<widths[j]; ++i)
			cols[j] += "-";
	
	string log  = string("+-")+cols[0]+"-+-"+cols[1]+"-+-"+cols[2]+"-+-"+cols[3]+"-+-"+cols[4]+"-+-"+cols[5]+"-+-"+cols[6]+"-+-"+cols[7]+"-+-"+cols[8]+"-+";
	stringWriter.append(log);
}

void CFamilyBehavior::displayLog(CStringWriter& stringWriter, int index, bool detailled, std::vector<size_t> widths)
{
	vector<string> cols(9, "");
	cols[0] = getCellZone()->getAliasFullName();
	cols[1] = getName();
	cols[2] = toString(getLevelIndex()) + ":" + getLevelString(getLevelIndex());
	cols[3] = toString(effectiveLevel()/(float)ENERGY_SCALE);
	cols[4] = toString(_CurrentLevel/(float)ENERGY_SCALE);
	cols[5] = toString(energyScale()/(float)ENERGY_SCALE);
	cols[6] = toString(_TheoricalLevel/(float)ENERGY_SCALE);
	cols[7] = toString(_GrpFamily->levelEnergyValue(getLevelIndex()));
	cols[8] = toString(_Modifier[getLevelIndex()]);
	
	for (size_t j=0; j<cols.size(); ++j)
		for (size_t i=cols[j].length(); i<widths[j]; ++i)
			cols[j] += " ";
	
	string log = string("| ")+cols[0]+" | "+cols[1]+" | "+cols[2]+" | "+cols[3]+" | "+cols[4]+" | "+cols[5]+" | "+cols[6]+" | "+cols[7]+" | "+cols[8]+" |";
	
	stringWriter.append(log);
	/*
	if (!detailled)
		return;
	
	for	(uint32 i=0;i<4;i++)
	{
		stringWriter.append("		> "+getLevelString(i)+" \t: FinalEnergyScale "
				+toString(energyScale(i)/(float)ENERGY_SCALE)
				+" (EnergyScale="+toString(_GrpFamily->levelEnergyValue(i))
				+" Modifier="+toString(_Modifier[i])
				+")");
	}
	*/
}

void CFamilyBehavior::checkLogHeadersWidths(std::vector<size_t>& widths, int index, bool detailled)
{
	vector<string> cols(9, "");
	cols[0] = "CellZone";
	cols[1] = "GroupFamily";
	cols[2] = "Levels for";
	cols[3] = "effective";
	cols[4] = "current";
	cols[5] = "final";
	cols[6] = "theorical";
	cols[7] = "value";
	cols[8] = "modifier";
	
	for (size_t j=0; j<cols.size(); ++j)
		widths[j] = std::max(widths[j], cols[j].length());
}

void CFamilyBehavior::checkLogWidths(std::vector<size_t>& widths, int index, bool detailled)
{
	vector<string> cols(9, "");
	cols[0] = getCellZone()->getAliasFullName();
	cols[1] = getName();
	cols[2] = toString(getLevelIndex()) + ":" + getLevelString(getLevelIndex());
	cols[3] = toString(effectiveLevel()/(float)ENERGY_SCALE);
	cols[4] = toString(_CurrentLevel/(float)ENERGY_SCALE);
	cols[5] = toString(energyScale()/(float)ENERGY_SCALE);
	cols[6] = toString(_TheoricalLevel/(float)ENERGY_SCALE);
	cols[7] = toString(_GrpFamily->levelEnergyValue(getLevelIndex()));
	cols[8] = toString(_Modifier[getLevelIndex()]);
	
	for (size_t j=0; j<cols.size(); ++j)
		widths[j] = std::max(widths[j], cols[j].length());
}

std::string CFamilyBehavior::getIndexString()	const
{	
	return getOwner()->getIndexString()+toString(":fb%u", getChildIndex());
}

std::string	CFamilyBehavior::getOneLineInfoString() const
{
	return std::string("Family behaviour '") + getName() + "'";
}

std::vector<std::string> CFamilyBehavior::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	
	pushTitle(container, "CFamilyBehavior");
	pushEntry(container, "id=" + getIndexString());
	container.back() += " name=" + getName();
	pushFooter(container);
	
	
	return container;
}

std::string CFamilyBehavior::getManagerIndexString(const CManager *child) const
{
	if	(child == _ManagerNpc)
		return getIndexString() + ":mnpc";

	if	(child == _ManagerFauna)
		return getIndexString() + ":mfauna";

	return getIndexString() + ":munknown";
}

void CFamilyBehavior::updateManagers()
{
	// update the manager
//	NLMEMORY::CheckHeap(true);
	mgrNpc()->update();
	mgrFauna()->update();
//	NLMEMORY::CheckHeap(true);
}


void	CFamilyBehavior::getNpcFlags(AITYPES::CPropertySet &flags)
{
	flags = grpFamily()->getProfileProperty(string("npc"));
}

void	CFamilyBehavior::getActivities	(CPropertySet &food, CPropertySet &rest/*,bool &plante, const	CGroupDesc<CGroupFamily>*const	gd*/)	const
{
	food=grpFamily()->getProfileProperty(string("food"));
	rest=grpFamily()->getProfileProperty(string("rest"));
}

extern CVariable<TGameCycle>	DynamicMaxUpdatePeriod;
void	CFamilyBehavior::update(uint32 nbTicks)
{
	// calcs _UpdatePeriod to avoid pingpong problems ..
	breakable
	{
		if	(energyScale()==0)
		{
			_UpdatePeriod=1+DynamicMaxUpdatePeriod;
			break;
		}

		double	delta=((double)((sint32)energyScale()-(sint32)_TheoricalLevel))/((double)energyScale());
		clamp(delta,0.0,1.0);
		delta=1.0-delta;
		delta*=delta;	//	^3
		delta*=delta;
		delta*=delta;
		_UpdatePeriod=1+(uint32)(delta*DynamicMaxUpdatePeriod);
	}

	CManager	*Manager;
	{
		IAliasCont *cont0, *cont1;
		cont0 = mgrNpc()->getAliasCont(AITypeGrp);
		cont1 = mgrFauna()->getAliasCont(AITypeGrp);
		Manager=(cont0->size()>cont1->size())?NLMISC::safe_cast<CManager*>(mgrNpc()):NLMISC::safe_cast<CManager*>(mgrFauna());
	}
	

	// TODO : reactivate group deletion
	// delete group that are dead
	while	(!_GroupToDelete.empty())
	{
//	NLMEMORY::CheckHeap(true);
		CGroup	*const	grp=(CGroup*)_GroupToDelete.back();
		_GroupToDelete.pop_back();
		grp->getManager().getAliasCont(AITypeGrp)->removeChildByIndex(grp->getChildIndex());
//	NLMEMORY::CheckHeap(true);
	}


	// check to despawn groups that are no more valid in current energy or season
	breakable
	{
		H_AUTO(FamilyDespawnGroup)
		
		const IGroupDesc	*gd = NULL;
		CGroup	*grp=NULL;
		
		breakable
		{
			const	uint32	nbGroups=Manager->groups().size();

			if	(nbGroups==0)
				break;

			grp = Manager->getGroup(CAIS::rand16(nbGroups));
			if	(!grp)
				break;

			CDynGrpBase	*const	grpDynBase=grp->getGrpDynBase();
#if !FINAL_VERSION
			nlassert(grpDynBase!=NULL);
#endif
			if	(	grpDynBase
				||	grpDynBase->getDiscardable())
				gd=grpDynBase->getGroupDesc();

			break;
		}

		if	(	!grp
			||	!gd
			||	!grp->isSpawned())
			break;

//		add a check if group is valid related to used regions flags to know if we need to despawn it

		bool	alreadyDespawned=false;
		breakable
		{
			CGrpFauna	*const	grpFauna=dynamic_cast<CGrpFauna*>(grp);
			if (!grpFauna)
				break;

			const	CFaunaZone *faunaZone;
			CPropertySet	food, rest;
//			bool	plante;
			getActivities	(food, rest/*, plante, gd*/);
//			{
//#if	!FINAL_VERSION
//				nlwarning("there is a problem with getActivities for %s", gd->getFullName().c_str());
//#endif
//				break;
//			}

			const CAIPlace	*place=grpFauna->places()[CGrpFauna::EAT_PLACE];
			place=NLMISC::safe_cast<const CAIRefPlaceXYR *>(place)->getZone();
			faunaZone=NLMISC::safe_cast<const	CFaunaZone *>(place);
			if	(!faunaZone)
			{
#if !FINAL_VERSION
				nlassert(faunaZone);
#endif
				break;
			}

			if	(!faunaZone->haveActivity(food))
			{
				grp->getSpawnObj()->despawnBots(true);	//	not ok, despawn this group ..
				alreadyDespawned=true;
				break;
			}

			place=grpFauna->places()[CGrpFauna::EAT_PLACE];
			place=NLMISC::safe_cast<const CAIRefPlaceXYR *>(place)->getZone();
			faunaZone=NLMISC::safe_cast<const	CFaunaZone *>(place);
			if	(!faunaZone)
			{
#if !FINAL_VERSION
				nlassert(faunaZone);
#endif
				break;
			}
			
			if	(!faunaZone->haveActivity(rest))
			{
				grp->getSpawnObj()->despawnBots(true);	//	not ok, despawn this group ..
				alreadyDespawned=true;
				break;
			}

		}		
		// deals with npcs dyn groups
		breakable
		{
			CGroupNpc	*const	grpNpc=dynamic_cast<CGroupNpc *>(grp);
			if (!grpNpc)
				break;

			const	CNpcZone *npcZone = grpNpc->getSpawnZone();
			CPropertySet	flags;
			getNpcFlags(flags);			
			
			if	(!npcZone || !npcZone->properties().containsAllOf(flags))
			{
				// must despawn  group
				grp->getSpawnObj()->despawnBots(true);	//	not ok, despawn this group ..
				alreadyDespawned=true;
				break;
			}			

		}
		if	(alreadyDespawned)
			break;
		
		if	(gd->getWeightForEnergy(getLevelIndex())!=0)
			break;

		const	EGSPD::CSeason::TSeason	season=CTimeInterface::season();

		if	(	season<EGSPD::CSeason::Invalid	//	if valid season
			&&	gd->isValidForSeason(season)	)
			break;												//	no reason to despawn

		if	(!alreadyDespawned)
			grp->getSpawnObj()->despawnBots(false);	//	not ok, despawn this group ..
	}
	
	// check for spawning new group to equilibrate energy level.
	breakable
	{
		H_AUTO(FamilyGroup)
		if	(_TheoricalLevel< energyScale())
		{
			H_AUTO(FamilyGroupeSpawn)
			// need to spawn some group ?
			if	(_FamilyProfile)
				_FamilyProfile->spawnGroup();

			break;
		}
		// or check for despawning

		if	(_TheoricalLevel <= (energyScale()+(uint32)(0.01*(double)ENERGY_SCALE)))
			break;

		CGroup	*grp=NULL;
		const IGroupDesc	*gd = NULL;
		// need to despawn some group ?

		{
			H_AUTO(FamilyGroupDespawn)
				
				// try to despawn some group in the manager
			const	uint32 start = CAIS::rand16(Manager->groups().size());

			grp = Manager->getGroup(start);
			if	(!grp)
				grp	= Manager->groups().getNextValidChild(grp);
			
			if (grp)
			{
				CDynGrpBase	*const	grpDynBase=grp->getGrpDynBase();
				if (	grpDynBase
					&&	grp->getSpawnObj()
					&&	grpDynBase->getDiscardable())
				{
					gd=grpDynBase->getGroupDesc();
				}
								
			}
			else
			{
				Manager->groups().setChildSize(start);	//	There's no group after start, so we can resize the group.
			}
			
		}
		

		if	(grp && gd)
		{
			H_AUTO(FamilyGroupDespawn)

			if ((_TheoricalLevel - gd->groupEnergyValue()) >= energyScale())
			{
				// ok, we can despawn this group
				grp->despawnBots(false);
			}

		}
		
	}

	{
		H_AUTO(FamilyProfileUpdate);
		CFollowPathContext fpcFamilyProfileUpdate("FamilyProfileUpdate");

		// update the family profile (if any)
		if (_FamilyProfile)
			_FamilyProfile->update();
	}

}

void CFamilyBehavior::fillOutpostNames(std::vector<NLMISC::TStringId> outpostNames)
{
	if	(_FamilyProfile)
		_FamilyProfile->fillOutpostNames(outpostNames);
}


void CFamilyBehavior::outpostAdd(NLMISC::TStringId outpostName)
{
	if	(_FamilyProfile)
		_FamilyProfile->outpostAdd(outpostName);
}
void CFamilyBehavior::outpostRemove(NLMISC::TStringId outpostName)
{
	if	(_FamilyProfile)
		_FamilyProfile->outpostRemove(outpostName);
}

void CFamilyBehavior::outpostEvent(NLMISC::TStringId outpostName, ZCSTATE::TZcState	state)
{
	if	(_FamilyProfile)
		_FamilyProfile->outpostEvent(outpostName,state);
}

void CFamilyBehavior::spawnBoss(NLMISC::TStringId outpostName)
{
	if	(_FamilyProfile)
		_FamilyProfile->spawnBoss(outpostName);
}


void	CFamilyBehavior::groupDead(CGroup *grp)
{
#ifdef NL_DEBUG
	for	(uint32 i=0;i<_GroupToDelete.size();i++)
	{
		nlassert(_GroupToDelete[i].ptr() != grp);
	}
#endif
	// ok, we can delete this group
	_GroupToDelete.push_back(grp);
}

void	CFamilyBehavior::addEnergy	(uint32	energy)
{
	_CurrentLevel	+=	energy;
}

void	CFamilyBehavior::removeEnergy	(uint32	energy)
{
#ifdef NL_DEBUG
	nlassert(_CurrentLevel>=energy);	
#endif
	_CurrentLevel	-=	energy;
}


void	CFamilyBehavior::serviceEvent	(const	CServiceEvent	&info)
{
	mgrNpc()->serviceEvent	(info);
	mgrFauna()->serviceEvent	(info);
}


CGroupNpc	*CFamilyBehavior::createNpcGroup(const CNpcZone	*const	zone, const	CGroupDesc<CGroupFamily>	*const	groupDesc)
{
//	const TPopulationFamily	&family=getFamily()
//	if	(	family.FamilyTag == family_fauna_herbivore
//		||	family.FamilyTag == family_fauna_carnivore
//		||	family.FamilyTag == family_flora
//		||	family.FamilyTag == family_kitin
//		||	family.FamilyTag == family_kitin_invasion
//		||	family.FamilyTag == family_degen
//		||	family.FamilyTag == family_goo	)
//	{
//		nlwarning("CRegion::createGroup can't create a npc group for family '%s', energy level %f in region '%s'", family.getFamilyName().c_str(), effectiveLevel(), getOwner()->getOwner()->getAliasFullName().c_str());
//		return	NULL;
//	}

	CGroupNpc	*grp=groupDesc->createNpcGroup	(mgrNpc(), zone->midPos());

	if (grp)
	{
		grp->initDynGrp		(groupDesc, this);
		grp->setSpawnZone(zone);
	}
	return	grp;
}

bool CFamilyBehavior::spawn()
{
	// Spawn dyn NPCs
	_ManagerNpc->spawn();
	// Spawn dyn fauna
	_ManagerFauna->spawn();
	// We should check individual errors, but fake success here :)
	return true;
}

bool CFamilyBehavior::despawn()
{
	// Despawn dyn NPCs
	_ManagerNpc->despawnMgr();
	// Despawn dyn fauna
	_ManagerFauna->despawnMgr();
	// We should check individual errors, but fake success here :)
	return true;
}
