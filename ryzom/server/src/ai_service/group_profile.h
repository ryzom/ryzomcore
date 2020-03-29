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




#ifndef GROUP_PROFILE_H
#define GROUP_PROFILE_H

#include "continent.h"
#include "ai_profile_npc.h"
#include "timer.h"

//----------------------------------------------------------------------------
// CGrpProfileDynCamping
//----------------------------------------------------------------------------

class CGrpProfileDynCamping
: public CSlaveProfile
{
public:
	CGrpProfileDynCamping(CProfileOwner* owner, CNpcZone const* const zone);
			
	/// @name CAIBaseProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::MOVE_CAMPING; }
	virtual std::string getOneLineInfoString() const;
	//@}
	
	/// @name CSlaveSpawnProfile implementation
	//@{
	void addBot(CBot* bot) { }
	void removeBot(CBot* bot) { }
	//@}
	
	bool timeOut() const;
	CNpcZone const* currentZone() const;
	
private:
	NLMISC::CSmartPtr<CNpcZone const> _CurrentZone;

	RYAI_MAP_CRUNCH::CWorldPosition _CampPos;
	CAITimer _EndOfCamping;
};

inline
CGrpProfileDynCamping::CGrpProfileDynCamping(CProfileOwner* owner, CNpcZone const* const zone)
: CSlaveProfile(owner)
, _CurrentZone(zone)
{
}

inline
bool CGrpProfileDynCamping::timeOut() const
{
	return _EndOfCamping.test();
}

inline
CNpcZone const* CGrpProfileDynCamping::currentZone() const
{
	return	_CurrentZone;
}

//----------------------------------------------------------------------------
// CGrpProfileDynWaitInZone
//----------------------------------------------------------------------------

class CGrpProfileDynWaitInZone
: public CGrpProfileNormal
{
public:
	CGrpProfileDynWaitInZone(CProfileOwner* owner, CNpcZone const* const npcZone);
	
	/// @name CAIBaseProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ZONE_WAIT; }
	virtual std::string getOneLineInfoString() const;
	//@}
	
	CNpcZone const* currentZone() const;
	
private:
	NLMISC::CSmartPtr<const	CNpcZone> _CurrentZone;
	/// where to wait.
	RYAI_MAP_CRUNCH::CWorldPosition WaitPos;
	/// Date of beginning of wait
	uint32 StartOfWait;
};

inline
CGrpProfileDynWaitInZone::CGrpProfileDynWaitInZone(CProfileOwner* owner, CNpcZone const* const npcZone) 
: CGrpProfileNormal(owner)
, _CurrentZone(npcZone)
{
}

inline
CNpcZone const* CGrpProfileDynWaitInZone::currentZone() const
{
	return _CurrentZone;
}

//---------------------------------------------------------------------------------
// CGrpProfileTestDyn
//---------------------------------------------------------------------------------

class CGrpProfileDynFollowPath
: public CMoveProfile
{
	static int _InstanceCounter;
public:
	CGrpProfileDynFollowPath(CProfileOwner* owner);
	CGrpProfileDynFollowPath(CProfileOwner* owner, CNpcZone const* const start, CNpcZone const* const end, AITYPES::CPropertySet const& zoneFilter);
	virtual ~CGrpProfileDynFollowPath();
	
	/// @name CAIBaseProfile implementation
	//@{
	virtual	void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual	void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType () const { return AITYPES::MOVE_DYN_FOLLOW_PATH; }
	virtual std::string getOneLineInfoString() const;
	//@}
	
	/// @name CSlaveSpawnProfile implementation
	//@{
	void addBot(CBot* bot);
	void removeBot(CBot* bot);
	//@}
	
	/// @name CMoveSpawnProfile implementation
	//@{
	virtual	CPathCont* getPathCont(CBot const* bot);
	//@}
	
	/// Overload for IDynFollowPath interface
	void setPath(CNpcZone const* const start, CNpcZone const* const end, AITYPES::CPropertySet const& zoneFilter);
	void calcPath();
	
	bool destinationReach() const;
	
	CNpcZone const* currentZone() const;
	
private:
	NLMISC::CSmartPtr<CNpcZone const>	_StartZone;	///< smartPtr to inc refcount (cheat)
	NLMISC::CSmartPtr<CNpcZone const>	_EndZone;	///< smartPtr to inc refcount (cheat)
	
	NLMISC::CstCDbgPtr<CNpcZone>	_CurrentZone;
	NLMISC::CstCDbgPtr<CRoad>		_CurrentRoad;
	AITYPES::CPropertySet			_ZoneFilter;
	
	/// CSmartPtr to have a counter on each road (bad way, but no time to spend rewriting kernel classes).
	std::vector<NLMISC::CSmartPtr<CRoad> >	_Path;
	uint32		_PathCursor;
	
	bool		_hasChanged;	//	to avoid to remake calculation every calls.
	
	CProfilePtr	_FollowRoute;
};

inline
CGrpProfileDynFollowPath::CGrpProfileDynFollowPath(CProfileOwner* owner)
: CMoveProfile(owner)
{
	++_InstanceCounter;
	nldebug("CGrpProfileDynFollowPath: %u instances", _InstanceCounter);
	_hasChanged = false;
}

inline
CGrpProfileDynFollowPath::CGrpProfileDynFollowPath(CProfileOwner* owner, CNpcZone const* const start, CNpcZone const* const end, AITYPES::CPropertySet const& zoneFilter)
: CMoveProfile(owner)
{
	++_InstanceCounter;
	
	_hasChanged = false;
	setPath(start, end, zoneFilter);
}

inline
CGrpProfileDynFollowPath::~CGrpProfileDynFollowPath()
{
	--_InstanceCounter;
}

inline
CNpcZone const* CGrpProfileDynFollowPath::currentZone() const
{
	return _CurrentZone;
}

inline
CPathCont* CGrpProfileDynFollowPath::getPathCont(CBot const* bot)
{
	nlassert(_FollowRoute.getAIProfile() != NULL);
	return	NLMISC::safe_cast<CMoveProfile*>(_FollowRoute.getAIProfile())->getPathCont(bot);
}

inline
void CGrpProfileDynFollowPath::addBot(CBot* bot)
{
	if( _FollowRoute.getAIProfile() == NULL)
		return;
	
	CMoveProfile*moveProfile=NLMISC::type_cast<CMoveProfile*>(_FollowRoute.getAIProfile());
#if !FINAL_VERSION
	nlassert(moveProfile);
#endif
	if (moveProfile)
		moveProfile->addBot(bot);
}

inline
void CGrpProfileDynFollowPath::removeBot(CBot* bot)
{
	if( _FollowRoute.getAIProfile() == NULL)
		return;
	
	CMoveProfile*moveProfile=NLMISC::type_cast<CMoveProfile*>(_FollowRoute.getAIProfile());
#if !FINAL_VERSION
	nlassert(moveProfile);
#endif
	if (moveProfile)
		moveProfile->removeBot(bot);
}

#endif // GROUP_PROFILE_H
