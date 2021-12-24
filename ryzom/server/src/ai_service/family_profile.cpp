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
#include "family_profile.h"
#include "continent.h"
#include "ai_place_xyr.h"
#include "ai_grp_fauna.h"
#include "ai_grp_npc.h"
#include "ai_mgr_fauna.h"
#include "ai_mgr_npc.h"
#include "group_profile.h"
#include "family_behavior.h"
//	#include "family_profile_tribe.h"

#include "continent_inline.h"

//extern bool LogOutpostDebug;
extern NLMISC::CVariable<bool>	LogOutpostDebug;

#include "dyn_grp_inline.h"

using namespace std;
using namespace NLMISC;
using namespace AITYPES;


CGroupNpc	*IFamilyProfile::createNpcGroup(const CNpcZone	*const	zone, const	CGroupDesc<CGroupFamily>	*const	groupDesc)
{
	CGroupNpc	*grp=_FamilyBehavior->createNpcGroup(zone, groupDesc);
	if (grp)
		setDefaultProfile(zone, grp);
	return	grp;
}


class	CFamilyProfileKitin : public IFamilyProfile
{
public:
	CFamilyProfileKitin	(const	IFamilyProfile::CtorParam	&ctorParam)
		:IFamilyProfile(ctorParam)
	{
	}
	virtual	~CFamilyProfileKitin()
	{
	}

	void	spawnGroup()
	{
		H_AUTO(FamilySpawnKitin)

		const	CGroupDesc<CGroupFamily> *gd = _FamilyBehavior->grpFamily()->getProportionalGroupDesc(_FamilyBehavior, CPropertySet(), CPropertySet());
		if	(!gd)
			return;

		CGrpFauna *grp = gd->createFaunaGroup(_FamilyBehavior);
		if	(!grp)
			return;

//		nldebug("DYN: Grp '%p' spawned for fauna", grp);
		grp->getSpawnObj()->spawnBotOfGroup();
	}

	/// The main update for the profile. Called aprox every 10 s (100 ticks)
	void update()
	{
	}

};


class CFamilyProfileFauna : public IFamilyProfile
{
public:
	CFamilyProfileFauna	(const	IFamilyProfile::CtorParam	&ctorParam)
		:IFamilyProfile(ctorParam)
	{
	}
	virtual	~CFamilyProfileFauna()
	{
	}

	void	spawnGroup()
	{
		H_AUTO(FamilySpawnFauna)

		const	CGroupDesc<CGroupFamily> *const	gd = _FamilyBehavior->grpFamily()->getProportionalGroupDesc(_FamilyBehavior, CPropertySet(), CPropertySet());
		if	(!gd)
			return;

		CGrpFauna *const	grp = gd->createFaunaGroup(_FamilyBehavior);
		if	(!grp)
			return;

//		nldebug("DYN: Grp '%p' spawned for fauna", grp);
		grp->getSpawnObj()->spawnBotOfGroup();
	}

	/// The main update for the profile. Called aprox every 10 s (100 ticks)
	void update()
	{
	}

};


class CFamilyProfileNpc : public IFamilyProfile
{
public:
	CFamilyProfileNpc(const	IFamilyProfile::CtorParam	&ctorParam)
		:IFamilyProfile(ctorParam)
	{
	}
	virtual	~CFamilyProfileNpc()
	{
	}

	void	spawnGroup()
	{
		H_AUTO(FamilySpawnNpc)	
		
		AITYPES::CPropertySet	flags;
		_FamilyBehavior->getNpcFlags(flags);		

		const CNpcZone	*spawn = _FamilyBehavior->getOwner()->lookupNpcZone(flags, _FamilyBehavior->grpFamily()->getSubstitutionId());
		if	(!spawn)
			return;					
		const	CGroupDesc<CGroupFamily>	*const	gd = _FamilyBehavior->grpFamily()->getProportionalGroupDesc(_FamilyBehavior, CPropertySet(), CPropertySet());

		if	(!gd)
			return;		

		const	CGroupNpc	*const	grp=createNpcGroup(spawn, gd);

		if	(!grp)
			return;

		grp->getSpawnObj()->spawnBotOfGroup();
	}

	/// The main update for the profile. Called aprox every 10 s (100 ticks)
	void update()
	{
	}

};

CAiFactory<IFamilyProfile, CFamilyProfileFauna>	_singleProfileFauna;
IAiFactory<IFamilyProfile>	*_ProfileFauna=&_singleProfileFauna;

CAiFactory<IFamilyProfile, CFamilyProfileKitin>	_singleProfileKitin;
IAiFactory<IFamilyProfile>	*_ProfileKitin=&_singleProfileKitin;

CAiFactory<IFamilyProfile, CFamilyProfileNpc>	    _singleProfileNpc;
IAiFactory<IFamilyProfile>	*_ProfileNpc=&_singleProfileNpc;


extern	IAiFactory<IFamilyProfile>	*_ProfileTribe;	//	in another cpp.

NL_ISO_TEMPLATE_SPEC CAiFactoryContainer<IFamilyProfile, TStringId> *CAiFactoryContainer<IFamilyProfile, TStringId>::_Instance = NULL;

CFamilyProfileFactory::CFamilyProfileFactory()
{
	registerFactory(CStringMapper::map("groupFamilyProfileFauna"),	_ProfileFauna);
	registerFactory(CStringMapper::map("groupFamilyProfileKitin"),	_ProfileKitin);
	registerFactory(CStringMapper::map("groupFamilyProfileTribe"),	_ProfileTribe);
	registerFactory(CStringMapper::map("groupFamilyProfileNpc"),	_ProfileNpc);
}

CFamilyProfileFactory::~CFamilyProfileFactory()
{
}



CAiFactoryContainer<IFamilyProfile, TStringId> &CFamilyProfileFactory::instance()
{
	if (!_Instance)
	{
		_Instance = new CFamilyProfileFactory();
	}
	return *_Instance;
}

IFamilyProfile*	CFamilyProfileFactory::createFamilyProfile(const	TStringId	&keyWord, const	IFamilyProfile::CtorParam&	ctorParam)
{

	breakable
	{
		IAiFactory<IFamilyProfile>	*const	familyProfile=instance().getFactory(keyWord);
		
		if	(!familyProfile)
			break;
		
		IFamilyProfile	*const	profile=familyProfile->createObject(ctorParam);
		
		if	(!profile)
			break;
		
		return	profile;
	}
	nlwarning("DYN: createProfile no profile available for %s", NLMISC::CStringMapper::unmap(keyWord).c_str());
	return	NULL;
}	

IFamilyProfile*	IFamilyProfile::createFamilyProfile(const	TStringId	&profileName, const	IFamilyProfile::CtorParam&	ctorParam)
{
	return	CFamilyProfileFactory::createFamilyProfile(profileName, ctorParam);
}

