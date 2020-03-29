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



#ifndef RYAI_GENERIC_FIGHT_H
#define	RYAI_GENERIC_FIGHT_H

#include "profile.h"
#include "sheets.h"
#include "path_behaviors.h"


// Forward declarations
class CAIEntityPhysical;
class CModEntityPhysical;
class CBotPlayer;

// a big bad global var !
extern CAIEntityPhysical	*TempSpeaker;
extern CBotPlayer			*TempPlayer;
extern float getDistBetWeen(CAIEntityPhysical& creat1, CAIEntityPhysical& creat2);

template <class TVectorSrc, class TVectorDst>
float buildDecalage(TVectorSrc const& src, TVectorDst const& dst, CAIVector* decalage)
{
	if (decalage!=NULL)
	{
		CAIVector& vect = *decalage;
		vect = dst;
		vect -= src;
		float normXY = (float)vect.norm();
		float normZ = (dst.h()-src.h())/1000.f;
		// :KLUDGE: This is a dirty dirty hack
		// As z precision is 2m, entities can be near contact but considered 2m away if they are on differents PACS planes.
		// So we bring back all z dists 2m closer to 0.
		if (normZ > 2.f) normZ -= 2.f;
		else if (normZ < -2.f) normZ += 2.f;
		else normZ = 0.f;
		// End of hack
		float normXYZ = (float)sqrt(normXY*normXY + normZ*normZ);
		return normXYZ;
	}
	else
	{
		CAIVector vect;
		return buildDecalage(src, dst, &vect);
	}
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFightHeal                                                     //
//////////////////////////////////////////////////////////////////////////////
class CBotProfileFightHeal
: public CAIBaseProfile
{
public:
	CBotProfileFightHeal() : CAIBaseProfile() { }
	virtual void eventTargetKilled() = 0;
	virtual void noMoreTarget() = 0;
	
public:
	/// @name Fight ranges/distances parameters
	//@{
	static float fightDists[AISHEETS::FIGHTCFG_MAX];
	static float fightDefaultMinRange;
	static float fightDefaultMaxRange;
	static float fightMeleeMinRange;
	static float fightMeleeMaxRange;
	static float fightRangeMinRange;
	static float fightRangeMaxRange;
	static float fightMixedMinRange;
	static float fightMixedMaxRange;
	static float giveUpDistance;
	static bool  fleeUnreachableTargets;
	//@}
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFight                                                         //
//////////////////////////////////////////////////////////////////////////////

///	Generic fight profile class
class CBotProfileFight
: public CBotProfileFightHeal
, public NLMISC::IDbgPtrData
{
public:
	/// @name Constructors, destructor
	//@{
	CBotProfileFight(CProfileOwner* owner, CAIEntityPhysical* ennemy);
	virtual ~CBotProfileFight();
	//@}
	
	/// @name Event handlers
	//@{
	virtual void eventBeginFight() = 0;
	virtual void eventTargetKilled() = 0;
	//@}
	
	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual	void resumeProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual std::string getOneLineInfoString() const;
	//@}
	
	virtual void noMoreTarget() = 0;
	
public:
	/// @name Accessors
	//@{
	bool isHitting() const { return _Bot->isHitting(); }
	bool atAttackDist() const { return _AtAttackDist; }
	virtual AITYPES::TProfiles getAIProfileType() const { return AITYPES::BOT_FIGHT; }
	//@}

protected:
	NLMISC::CDbgPtr<CSpawnBot>			_Bot;
	NLMISC::CRefPtr<CAIEntityPhysical>	_Ennemy;
	
	bool			_Engaged;
	
	CPathPosition	_PathPos;
	CPathCont		_PathCont;
	
	bool			_RangeCalculated;
	bool			_UseFightConfig;
	float			_RangeMin;
	float			_RangeMax;
	
	bool			_AtAttackDist;
	
	bool			_SearchAlternativePath;
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileHeal                                                          //
//////////////////////////////////////////////////////////////////////////////

///	Generic fight profile class
class CBotProfileHeal
: public CBotProfileFightHeal
{
public:
	/// @name Constructors, destructor
	//@{
	CBotProfileHeal(TDataSetRow const& row, CProfileOwner* owner);
	virtual ~CBotProfileHeal();
	//@}
	
	/// @name Event handlers
	//@{
//	virtual void eventBeginFight() = 0;
	virtual void eventTargetKilled() { nlwarning("eventTargetKilled called on a heal profile"); }
	//@}
	
	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual	void resumeProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual std::string getOneLineInfoString() const;
	//@}
	
	virtual void noMoreTarget() = 0;
	
public:
	/// @name Accessors
	//@{
	bool isHitting() const { return (_Bot->getActionFlags()&RYZOMACTIONFLAGS::Attacks)!=0; }
	bool atAttackDist() const { return _AtAttackDist; }
	virtual AITYPES::TProfiles getAIProfileType() const { return AITYPES::BOT_HEAL; }
	//@}
	
protected:
	NLMISC::CDbgPtr<CSpawnBot>	_Bot;
	
	bool			_Engaged;
	
	CPathPosition	_PathPos;
	CPathCont		_PathCont;
	TDataSetRow		_Row;
	
	bool			_RangeCalculated;
	bool			_UseFightConfig;
	float			_RangeMin;
	float			_RangeMax;
	
	bool			_AtAttackDist;
	
	bool			_SearchAlternativePath;
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFlee                                                          //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileFlee
: public CAIBaseProfile
{
public:
	/// @name Constructor and destructor
	//@{
	CBotProfileFlee(CProfileOwner *owner);
	virtual ~CBotProfileFlee();
	//@}
	
	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile() { }
	virtual	AITYPES::TProfiles getAIProfileType () const { return AITYPES::BOT_FLEE; }
	virtual std::string getOneLineInfoString() const;
	//@}
	
private:
	RYAI_MAP_CRUNCH::CDirection		_LastDir;
	RYAI_MAP_CRUNCH::CMapPosition	_LastStartPos;
	
	RYAI_MAP_CRUNCH::TAStarFlag		_DenyFlags;
	
	CPathPosition					_PathPos;
	CPathCont						_fightFleePathContainer;
	
public:
	/// @name Flee ranges/distances parameters
	//@{
	static float giveUpDistanceUnreachable;
	//@}
	
protected:
	NLMISC::CDbgPtr<CSpawnBot>	_Bot;
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileReturnAfterFight                                              //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileFollowPos;

class CBotProfileReturnAfterFight
: public CAIBaseProfile
{
public:
	/// @name Constructor and destructor
	//@{
	CBotProfileReturnAfterFight(CProfileOwner *owner);
	virtual ~CBotProfileReturnAfterFight();
	//@}
	
	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType () const { return AITYPES::BOT_RETURN_AFTER_FIGHT; }
	virtual std::string getOneLineInfoString() const;
	virtual void stateChangeProfile();
	//@}
	
private:
	/*
	RYAI_MAP_CRUNCH::CDirection		_LastDir;
	RYAI_MAP_CRUNCH::CMapPosition	_LastStartPos;
	
	RYAI_MAP_CRUNCH::TAStarFlag		_DenyFlags;
	
	CPathPosition					_PathPos;
	CPathCont						_fightFleePathContainer;
	*/
//	CPathCont	_PathCont;
//	NLMISC::CSmartPtr<CBotProfileFollowPos> _MoveProfile;
	
public:
	/// @name Flee ranges/distances parameters
	//@{
//	static float giveUpDistanceUnreachable;
	//@}
	
protected:
	NLMISC::CDbgPtr<CSpawnBot>	_Bot;
};

//////////////////////////////////////////////////////////////////////////////
// CFightOrganizer                                                          //
//////////////////////////////////////////////////////////////////////////////

class CFightOrganizer
{
public:
	CFightOrganizer();
	
	template <class TContainerIterator>
	void reorganize(TContainerIterator const& first, TContainerIterator const& last)
	{
		H_AUTO(CFightOrganizer_reorganize);
		_HaveEnnemy = false;
		
		for	(TContainerIterator it=first, itEnd=last;it!=itEnd;++it)
		{
			CBot* bot = (*it);
			if (!reorganizeIteration(bot))
				break;
		}
	}
	
	/// @name Virtual interface
	//@{
	virtual void setFight(CSpawnBot* bot, CAIEntityPhysical* ennemy) = 0;
	virtual void setHeal(CSpawnBot* bot, CAIEntityPhysical* target) = 0;
	virtual void setNoFight(CSpawnBot* bot) = 0;
	virtual void setFlee(CSpawnBot* bot, CAIVector& fleeVect) = 0;
	virtual void setReturnAfterFight(CSpawnBot* bot) = 0;
	//@}
	
private:
	bool healIteration(CBot* bot, CBot* otherBot);
	bool reorganizeIteration(CBot* bot);
	
protected:
	bool	_HaveEnnemy;
};

/****************************************************************************/
/* Profile factories                                                        */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFightFactory                                                  //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileFightFactory
: public IAIProfileFactory
{
public:
	NLMISC::CSmartPtr<IAIProfile> createAIProfile(CProfileOwner* owner)
	{
		nlassert(false);
		return NULL;
	}
};
RYAI_DECLARE_PROFILE_FACTORY(CBotProfileFightFactory);

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFleeFactory                                                   //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileFleeFactory
: public IAIProfileFactory
{
public:
	NLMISC::CSmartPtr<IAIProfile> createAIProfile(CProfileOwner* owner)
	{
		nlassert(false);
		return NULL;
	}
};
RYAI_DECLARE_PROFILE_FACTORY(CBotProfileFleeFactory);

#endif
