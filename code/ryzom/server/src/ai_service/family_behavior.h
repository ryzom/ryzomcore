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



#ifndef FAMILY_BEHAVIOR_H
#define FAMILY_BEHAVIOR_H

#include "ai_share/ai_types.h"
#include "child_container.h"
#include "manager_parent.h"
#include "family_profile.h"
#include "ai_mgr_npc.h"
#include "ai_mgr_fauna.h"
#include "time_interface.h"

class	CCellZone;


class	CLevelEnergy : public	NLMISC::CDbgRefCount<CLevelEnergy>
{
public:
	CLevelEnergy	()
	{
		for	(uint32 i=0;i<4;i++)
		{
			_LevelEnergyValue[i]=1.f;
		}
	}
	CLevelEnergy	(const	CLevelEnergy	&other)
	{
		for	(uint32 i=0;i<4;i++)
		{
			_LevelEnergyValue[i]=other._LevelEnergyValue[i];
		}
	}

	inline	void	setLevelEnergyValue	(const	double	&v, const	uint32 &index)
	{
#ifdef NL_DEBUG
		nlassert(index<=3);
#endif
		if (index>3)
			return;

		_LevelEnergyValue[index]=v;
	}

	inline	double	levelEnergyValue	(const	uint32	&levelIndex)	const
	{
#ifdef NL_DEBUG
		nlassert(levelIndex<=3);
#endif
		if	(levelIndex>3)
			return	0;

		return	_LevelEnergyValue[levelIndex];
	}

//	protected:
private:
	double	_LevelEnergyValue[4];
};

/** Bot Family master behavior.
 *	This class contains state and ai profile for a family inside a cellzone.
 *	Also, there is a
 */

class CGroupFamily;

class CFamilyBehavior
: public NLMISC::CDbgRefCount<CFamilyBehavior>
, public NLMISC::CRefCount
, public CChild<CCellZone>
, public IManagerParent
, public CServiceEvent::CHandler
, public CAIEntity
{
 public:
	std::string getIndexString() const;
	std::string getManagerIndexString(const CManager *child) const;

	CFamilyBehavior(CCellZone *owner, const	CGroupFamily	*grpFamily);	//	;const	std::string	&name, const	NLMISC::TStringId	&profileName)

	virtual	~CFamilyBehavior()
	{
		_FamilyProfile	=	NULL;
		_ManagerNpc		=	NULL;
		_ManagerFauna	=	NULL;
	}

	CMgrNpc* mgrNpc()
	{
	#ifdef NL_DEBUG
		nlassert(_ManagerNpc);
	#endif
		return _ManagerNpc;
	}

	CMgrFauna* mgrFauna()
	{
	#ifdef NL_DEBUG
		nlassert(_ManagerFauna);
	#endif
		return _ManagerFauna;
	}

	bool isFamilyProfileValid()	const
	{
		return _FamilyProfile!=NULL;
	}

	IFamilyProfile* familyProfile()	const
	{
		return	_FamilyProfile;
	}

	void	updateManagers();
	void	update(uint32 nbTicks);

	void displayLogOld(CStringWriter& stringWriter, bool detailled=false);
	void displayLog(CStringWriter& stringWriter, int index, bool detailled, std::vector<size_t> widths);
	void checkLogWidths(std::vector<size_t>& widths, int index, bool detailled);
	static void displayLogLine(CStringWriter& stringWriter, int index, bool detailled, std::vector<size_t> widths);
	static void displayLogHeaders(CStringWriter& stringWriter, int index, bool detailled, std::vector<size_t> widths);
	static void checkLogHeadersWidths(std::vector<size_t>& widths, int index, bool detailled);

//	const	AITYPES::TPopulationFamily &getFamily()	const	{	return _Family;	}
	const	AITYPES::CPropertyId &getFamilyTag() const
	{
		return _FamilyTag;
	}

	void	serviceEvent	(const	CServiceEvent	&info);

	uint	getStaticFameIndex()
	{
		return _Faction;
	}

//	NLMISC::CMustConsume<bool>	CFamilyBehavior::getActivities	(AITYPES::CPropertyId &food, AITYPES::CPropertyId &rest, bool &plante, const	CGroupDesc*const	gd)	const;
	void	getActivities	(AITYPES::CPropertySet &food, AITYPES::CPropertySet &rest/*, *bool &plante,	const	CGroupDesc*const	gd*/)	const;
	void	getNpcFlags(AITYPES::CPropertySet &flags);


//		NLMISC::CMustConsume<bool>	getActivities	(AITYPES::TZoneActivity &food, AITYPES::TZoneActivity &rest, bool &plante, const	CGroupDesc*const	gd)	const;

	const	uint32	&baseLevel	()	const
	{
		return	_BaseLevel;
	}

	uint32	getLevelIndex	()	const
	{
		//	check energy level
		if (effectiveLevel()<AITYPES::ENERGY_LEVEL_1)	//	<0.5
		{
			if (effectiveLevel()<AITYPES::ENERGY_LEVEL_0)	//	<0.25
				return	0;
			else	//	>=0.25 <0.5
				return	1;
		}
		else	//	>=0.5
		{
			if (effectiveLevel()<AITYPES::ENERGY_LEVEL_2)	//	>=0.5 <0.75
				return	2;
			else	//	>=0.75
				return	3;
		}

	}

	const	uint32	&effectiveLevel	()	const
	{
		return	_EffectiveLevel;
	}

	void	setBaseLevel	(const	uint32	&baseLevel)
	{
		_BaseLevel=baseLevel;
	}

	void	setEffectiveLevel	(const	uint32	&effectiveLevel)
	{
		_EffectiveLevel=effectiveLevel;
	}

	/// Fill a vector of outpost id name assigned to tribe
	void	fillOutpostNames(std::vector<NLMISC::TStringId> outpostNames);
	/// Add an outpost for the tribe (nb : the family must be a tribe)
	void	outpostAdd(NLMISC::TStringId outpostName);
	/// Remove an from the tribe
	void	outpostRemove(NLMISC::TStringId outpostName);

	void	outpostEvent(NLMISC::TStringId outpostName, ZCSTATE::TZcState	state);

	void	spawnBoss(NLMISC::TStringId outpostName);

	uint32	energyScale	(uint32 levelIndex=std::numeric_limits<uint32>::max())	const;

	void	setModifier	(const	float	&value, const	uint32	&index)
	{
		_Modifier[index]=value;
	}

	float	getModifier	(const	uint32	&index)	const
	{
		return	_Modifier[index];
	}

	bool	needUpdate	()	const
	{
		return	getDt()>_UpdatePeriod;
	}
	NLMISC::TGameCycle	getDt	()	const
	{
		return	CTimeInterface::gameCycle()-_LastUpdateTime;
	}

	CGroupNpc	*createNpcGroup(const CNpcZone	*const	zone, const	CGroupDesc<CGroupFamily>	*const	groupDesc);

	uint32	_TheoricalLevel;

	virtual std::string getName() const;

	const	NLMISC::CDbgPtr<CGroupFamily>	&grpFamily()	const
	{
		return	_GrpFamily;
	}

	bool spawn();
	bool despawn();

	virtual std::string	getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;

private:

 	CAIInstance *getAIInstance() const;
	virtual	void	addEnergy		(uint32	energy);
	virtual	void	removeEnergy	(uint32	energy);

	void	groupDead(CGroup *grp);

	 // overloads for IManagerParent virtuals
	 CAIInstance	&getAiInstance() const;

	 inline	CCellZone	*getCellZone()
	 {
		 return		getOwner();
	 }


	 // If the family is a tribe, the property describe the name of the family (witch is the tribe name for tribe family)
	 AITYPES::CPropertyId				_FamilyTag;
//	 AITYPES::TPopulationFamily			_Family;
	 uint32								_Faction;
	 NLMISC::CSmartPtr<IFamilyProfile>	_FamilyProfile;

	 /// A list of group to delete at the next master update.
	 std::vector<NLMISC::CDbgPtr<CGroup> >	_GroupToDelete;


	/// The base level. Come from a random at start of cycle.
	uint32	_BaseLevel;
	/// The effective level, computed from base level at start of cycle.
	uint32	_EffectiveLevel;
	/// The cumulated players actions effect.
	uint32	_PlayerEffect;
	/// The current energie level represented by the living population
	/** The potential energy.
	 *	This is a very special one. Each time a new group is spawn, the
	 *	global energy value is added to this var.
	 *	The value of this var always decay during time.
	 *	The value is added to CurrentLevel before compare to EffectiveLevel.
	 *	This avoid spawning lots of unneeded group because the
	 *	spawn of the bots are delayed so they dont give value
	 *	in CurrentLevel.
	 */
	uint32	_CurrentLevel;

	NLMISC::CSmartPtr<CMgrNpc>		_ManagerNpc;
	NLMISC::CSmartPtr<CMgrFauna>	_ManagerFauna;

//	NLMISC::CstCDbgPtr<CLevelEnergy>	_energyScale;
	float	_Modifier[4];

	NLMISC::TGameCycle	_LastUpdateTime;
	uint32				_UpdatePeriod;

	NLMISC::CDbgPtr<CGroupFamily>	_GrpFamily;
};

#endif // FAMILY_BEHAVIOR_H
