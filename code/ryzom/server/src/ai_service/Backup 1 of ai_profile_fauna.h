/** \file ai_profile_fauna.h
 * 
 * $Id: ai_profile_fauna.h,v 1.8 2005/08/09 12:38:24 vuarand Exp $
 * 
 * This file defines the classes:
 * - CPlanteIdleFaunaProfile
 * - CStaticPlanteIdleFaunaProfile
 * - CAIFaunaActivityBaseSpawnProfile
 * - CWanderFaunaProfile
 * - CStaticWanderFaunaProfile
 * - CGrazeFaunaProfile
 * - CStaticGrazeFaunaProfile
 * - CRestFaunaProfile
 * - CStaticRestFaunaProfile
 * - CStaticFightFaunaProfile
 * - CCorpseFaunaProfileFactory
 * - CEatCorpseFaunaProfile
 * - CStaticEatCorpseFaunaProfile
 * - CCuriosityFaunaProfile
 * - CStaticCuriosityFaunaProfile
 */

#ifndef AI_PROFILE_FAUNA_H
#define AI_PROFILE_FAUNA_H

#include "profile.h"		// for CAIBaseProfile
#include "path_behaviors.h"	// for CPathPosition
#include "ai_bot_fauna.h"	// for CCorpseFaunaProfile
#include "ai_grp_fauna.h"
#include "ai_mgr_fauna.h"

class CSpawnBotFauna;
class CMovementMagnet;

// ---------------------------------------------------------------------------
// Debug defines
// ---------------------------------------------------------------------------
// COMPACT_POS_WARNINGS compress flooding warnings concerning path problems.
// Positions where the problems occures are stored and displayed and cleared
// every minute.
// :TODO: /!\ As it cannot be tested without long-time run with several
// players the following define can be commented to restore previous behavior.
#define COMPACT_POS_WARNINGS 1

class CFaunaProfileFloodLogger
{
public:
	typedef std::map<std::string, int> TLogPositions;
	
public:
	CFaunaProfileFloodLogger(int period)
	: logLastTick(0)
	, logPeriod(period)
	{
	}
	
public:
	TLogPositions logPositions;
	int logLastTick;
	int logPeriod;
};

//////////////////////////////////////////////////////////////////////////////
// CPlanteIdleFaunaProfile                                                  //
//////////////////////////////////////////////////////////////////////////////

class CPlanteIdleFaunaProfile
: public CAIBaseProfile
{
public:
	CPlanteIdleFaunaProfile(CProfileOwner* owner);
	
	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ACTIVITY_PLANTIDLE; }
	virtual std::string getOneLineInfoString() const;
	//@}
	
protected:
	CSpawnBotFauna* _Bot;
};

//////////////////////////////////////////////////////////////////////////////
// CAIFaunaActivityBaseSpawnProfile                                         //
//////////////////////////////////////////////////////////////////////////////

class CAIFaunaActivityBaseSpawnProfile
: public CAIBaseProfile
, public IMouvementMagnetOwner
{
public:
	CAIFaunaActivityBaseSpawnProfile(CProfileOwner* owner);
	
	virtual NLMISC::CSmartPtr<CMovementMagnet> const& getMovementMagnet() const;
	
protected:
	NLMISC::CSmartPtr<CMovementMagnet> _MovementMagnet;
	CPathPosition _PathPos;
	bool _OutOfMagnet;
};

//////////////////////////////////////////////////////////////////////////////
// CWanderFaunaProfile                                                      //
//////////////////////////////////////////////////////////////////////////////

class CWanderFaunaProfile
: public CAIFaunaActivityBaseSpawnProfile
{
public:
	CWanderFaunaProfile(CProfileOwner* owner);
	
	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ACTIVITY_WANDERING; }
	virtual std::string getOneLineInfoString() const;
	//@}
	
protected:
	RYAI_MAP_CRUNCH::TAStarFlag _DenyFlags;
	CSpawnBotFauna* _Bot;
	double _magnetDist; ///< distance from bot to his magnet at last move
	static CFaunaProfileFloodLogger _FloodLogger;
};

//////////////////////////////////////////////////////////////////////////////
// CGrazeFaunaProfile                                                       //
//////////////////////////////////////////////////////////////////////////////

class CGrazeFaunaProfile
: public CAIFaunaActivityBaseSpawnProfile
{
public:
	CGrazeFaunaProfile(CProfileOwner* owner);
	
	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ACTIVITY_GRAZING; }
	virtual std::string getOneLineInfoString() const;
	//@}
	
protected:
	CSpawnBotFauna*	_Bot;
private:
	CAITimer			_CycleTimer;
	uint			_CycleTimerBaseTime;
	bool			_ArrivedInZone;	
	double			_magnetDist; ///< distance from bot to his magnet at last move
	RYAI_MAP_CRUNCH::TAStarFlag	_DenyFlags;
	static CFaunaProfileFloodLogger _FloodLogger;
};

//////////////////////////////////////////////////////////////////////////////
// CRestFaunaProfile                                                        //
//////////////////////////////////////////////////////////////////////////////

class CRestFaunaProfile
: public CAIFaunaActivityBaseSpawnProfile
{
public:
	CRestFaunaProfile(CProfileOwner* owner);
	
	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ACTIVITY_RESTING; }
	virtual std::string getOneLineInfoString() const;
	//@}
	
protected:
	CSpawnBotFauna*	_Bot;
private:
	CAITimer			_CycleTimer;
	uint			_CycleTimerBaseTime;
	bool			_ArrivedInZone;
	double			_magnetDist;	// distance from bot to his magnet at last move
	RYAI_MAP_CRUNCH::TAStarFlag	_DenyFlags;
	static CFaunaProfileFloodLogger _FloodLogger;
};

//////////////////////////////////////////////////////////////////////////////
// CEatCorpseFaunaProfile                                                   //
//////////////////////////////////////////////////////////////////////////////

class CEatCorpseFaunaProfile
: public CAIBaseProfile
{
public:	
	CEatCorpseFaunaProfile(CProfileOwner* owner, TDataSetRow const& corpse, RYAI_MAP_CRUNCH::TAStarFlag flag);
	
	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ACTIVITY_EAT_CORPSE; }
	virtual std::string getOneLineInfoString() const;
	//@}
	
	TDataSetRow		_eated;
protected:
	CSpawnBotFauna*	_Bot;
private:
	bool			_atGoodDist;
	CAITimer			_eatTimer;
	CPathPosition	_PathPos;
	CPathCont		_PathCont;
};

//////////////////////////////////////////////////////////////////////////////
// CCuriosityFaunaProfile                                                   //
//////////////////////////////////////////////////////////////////////////////

class CCuriosityFaunaProfile
: public CAIBaseProfile
{
public:
	CCuriosityFaunaProfile(CProfileOwner* owner, TDataSetRow const& player, RYAI_MAP_CRUNCH::TAStarFlag flag);
	
	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ACTIVITY_CURIOSITY; }
	virtual std::string getOneLineInfoString() const;
	//@}

	TDataSetRow		_player;
protected:
	CSpawnBotFauna*	_Bot;
private:
	bool			_atGoodDist;
	CAITimer			_curiosityTimer;
	CPathPosition	_PathPos;
	CPathCont		_PathCont;
	uint32			_addCuriosityTime;
	bool			_TooFar;
	RYAI_MAP_CRUNCH::TAStarFlag	_Flag;
};

//////////////////////////////////////////////////////////////////////////////
// CCorpseFaunaProfile                                                      //
//////////////////////////////////////////////////////////////////////////////

class CCorpseFaunaProfile
: public CAIBaseProfile
{
public:
	CCorpseFaunaProfile(CProfileOwner* owner);
	
	virtual void beginProfile();
	
	virtual void endProfile() { }
	
	virtual void updateProfile(uint ticksSinceLastUpdate) { }
	
	virtual std::string getOneLineInfoString() const { return NLMISC::toString("corpse fauna profile"); }
	
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ACTIVITY_CORPSE; }
	
	bool eated() const { return _Eated; }
	void setEated(bool eated) { _Eated = eated; }
	
	void setEater(bool eater) { _HaveEater = eater; }
	bool haveEater() const { return _HaveEater; }
	
protected:
	CSpawnBotFauna* _Bot;
	
private:
	bool _HaveEater;
	bool _Eated;
};

/****************************************************************************/
/* Profile factories                                                        */
/****************************************************************************/

//- Simple profile factories -------------------------------------------------

// CPlanteIdleFaunaProfileFactory
typedef CAIGenericProfileFactory<CPlanteIdleFaunaProfile> CPlanteIdleFaunaProfileFactory;

// CWanderFaunaProfileFactory
typedef CAIGenericProfileFactory<CWanderFaunaProfile> CWanderFaunaProfileFactory;

// CGrazeFaunaProfileFactory
typedef CAIGenericProfileFactory<CGrazeFaunaProfile> CGrazeFaunaProfileFactory;

// CRestFaunaProfileFactory
typedef CAIGenericProfileFactory<CRestFaunaProfile> CRestFaunaProfileFactory;

// CCorpseFaunaProfileFactory
typedef CAIGenericProfileFactory<CCorpseFaunaProfile> CCorpseFaunaProfileFactory;

//- Complex profile factories ------------------------------------------------

// CStaticFightFaunaProfile
class CFightFaunaProfileFactory
: public IAIProfileFactory
{
public:
	NLMISC::CSmartPtr<IAIProfile> createAIProfile(CProfileOwner* owner)
	{
		return NULL;
	}
};

// CStaticEatCorpseFaunaProfile
class CEatCorpseFaunaProfileFactory
: public IAIProfileFactory
{
public:
	NLMISC::CSmartPtr<IAIProfile> createAIProfile(CProfileOwner* owner)
	{
	#ifdef NL_DEBUG
		nlassert(false);
	#endif
		return	NULL;
	}
};

// CStaticCuriosityFaunaProfile
class CCuriosityFaunaProfileFactory
: public IAIProfileFactory
{
public:
	NLMISC::CSmartPtr<IAIProfile> createAIProfile(CProfileOwner* owner)
	{
#ifdef NL_DEBUG
		nlassert(false);
#endif
		return	NULL;
	}
};

//- Profile factories singletons ---------------------------------------------

extern CPlanteIdleFaunaProfileFactory PlanteIdleFaunaProfileFactory;
extern CWanderFaunaProfileFactory WanderFaunaProfileFactory;
extern CGrazeFaunaProfileFactory GrazeFaunaProfileFactory;
extern CRestFaunaProfileFactory RestFaunaProfileFactory;
extern CFightFaunaProfileFactory FightFaunaProfileFactory;
extern CCorpseFaunaProfileFactory CorpseFaunaProfileFactory;
extern CEatCorpseFaunaProfileFactory EatCorpseFaunaProfileFactory;
extern CCuriosityFaunaProfileFactory CuriosityFaunaProfileFactory;

#endif
