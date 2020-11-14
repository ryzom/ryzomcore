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



#ifndef FAMILY_PROFILE_H
#define FAMILY_PROFILE_H

#include "nel/misc/smart_ptr.h"

#include "ai_share/ai_types.h"
#include "server_share/mission_messages.h"
#include "game_share/zc_shard_common.h"
#include "ai_factory.h"

class CFamilyBehavior;
//class IGroup;

class CGroupNpc;
class CNpcZone;
class CGroupFamily;
class IGroupDesc;
template <class FamilyT>
class CGroupDesc;

/** Interface class for family profile
*/
class IFamilyProfile :
	public NLMISC::CDbgRefCount<IFamilyProfile>,
	public NLMISC::CRefCount
{
protected:
	NLMISC::CDbgPtr<CFamilyBehavior>	_FamilyBehavior;
public:

	class CtorParam
	{
	public:
		CtorParam(CFamilyBehavior *familyBehavior)
			:_familyBehavior(familyBehavior)
		{
#ifdef NL_DEBUG
			nlassert(familyBehavior);
#endif
		}
		virtual ~CtorParam()
		{}
		const	CFamilyBehavior *familyBehavior	()	const
		{
			return	_familyBehavior;
		}
	protected:
		friend	class	IFamilyProfile;
	private:
		CFamilyBehavior *_familyBehavior;
	};


	IFamilyProfile	(const	CtorParam	&ctorParam)
		:_FamilyBehavior(ctorParam._familyBehavior)
	{
	}

	virtual	~IFamilyProfile	()
	{
	}
	
	virtual	void	setDefaultProfile(const	CNpcZone	*const	zone, CGroupNpc	*grp)	{}

	/// Spawn group.
	virtual void	spawnGroup() =0;

	/// The main update for the profile. Called aprox every 10 s (100 ticks)
	virtual void update() =0;

	/// Fill a vector of outpost id name assigned to tribe
	virtual void fillOutpostNames(std::vector<NLMISC::TStringId> outpostNames)	{}
	/// Add an outpost for the tribe (nb : the family must be a tribe)
	virtual void outpostAdd(NLMISC::TStringId outpostName) {}
	/// Remove an from the tribe
	virtual void outpostRemove(NLMISC::TStringId outpostName) {}

	virtual	void outpostEvent(NLMISC::TStringId outpostName, ZCSTATE::TZcState	state)	{}

	virtual	void spawnBoss(NLMISC::TStringId outpostName)	{}

	CGroupNpc	*createNpcGroup(const	CNpcZone	*const	zone, const	CGroupDesc<CGroupFamily>	*const	groupDesc);

	/** Factory method to create profile instance.
	 *	NB : you are responsible to delete the profile.
	 */	
	static	IFamilyProfile*	createFamilyProfile(const	NLMISC::TStringId	&profileNamer, const	IFamilyProfile::CtorParam&	ctorParam);
};


class	CFamilyProfileFactory
	:	public	CAiFactoryContainer<IFamilyProfile, NLMISC::TStringId>	//	TFamilyTag>
{
public:
	CFamilyProfileFactory();
	
	static CAiFactoryContainer<IFamilyProfile, NLMISC::TStringId> &instance();
	
	virtual	~CFamilyProfileFactory();

	static	IFamilyProfile*	createFamilyProfile(const NLMISC::TStringId	&keyWord, const	IFamilyProfile::CtorParam&	ctorParam);
};



#endif // FAMILY_PROFILE_H
