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



#ifndef AI_PROFILE_NPC_H
#define AI_PROFILE_NPC_H

#include "profile.h"
#include "ai_bot_npc.h"
#include "ai_grp_npc.h"
#include "ai_generic_fight.h"

extern bool simulateBug(int bugId);

// Forard declarations
class CSpawnGroupNpc;

//////////////////////////////////////////////////////////////////////////////
// Free variables and functions                                             //
//////////////////////////////////////////////////////////////////////////////

extern bool ai_profile_npc_VerboseLog;

//////////////////////////////////////////////////////////////////////////////
// CBotProfileMoveTo                                                        //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileMoveTo
: public CAIBaseProfile
, public NLMISC::IDbgPtrData
{
public:
	CBotProfileMoveTo(AITYPES::TVerticalPos verticalPos, RYAI_MAP_CRUNCH::CWorldPosition const& dest, CProfileOwner* owner);

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual	AITYPES::TProfiles getAIProfileType () const { return AITYPES::BOT_MOVE_TO; }
	virtual std::string getOneLineInfoString() const;
	//@}

	bool destinationReach()	const;

	CFollowPath::TFollowStatus		_Status;
	AITYPES::TVerticalPos			_VerticalPos;
	RYAI_MAP_CRUNCH::CWorldPosition	_Dest;
	CPathCont		_PathCont;
	CPathPosition	_PathPos;
	CAIVector		_LastPos;

protected:
	NLMISC::CDbgPtr<CSpawnBotNpc>	_Bot;
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFollowPos                                                     //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileFollowPos
: public CAIBaseProfile
, public NLMISC::IDbgPtrData
{
public:
	CBotProfileFollowPos(CPathCont* pathCont, CProfileOwner* owner);
	CBotProfileFollowPos(CBotProfileFollowPos	const& other);

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::BOT_FOLLOW_POS; }
	virtual std::string getOneLineInfoString() const;
	//@}

	void setMaxSpeeds(float walkSpeed, float runSpeed);
	void setStop(bool stop);

	CFollowPath::TFollowStatus	_Status;
	CPathPosition		_PathPos;

protected:
	NLMISC::CDbgPtr<CSpawnBotNpc>	_Bot;

private:
	NLMISC::CDbgPtr<CPathCont>	_PathCont;

	// maximum speed to use (for group with different creatures)
	float	_MaxWalkSpeed;
	float	_MaxRunSpeed;

	// flag to stop temporary stop the movement
	bool	_Stop;
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileWanderBase                                                    //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileWanderBase
: public CAIBaseProfile
{
public:
	CBotProfileWanderBase();

	void setTimer(uint32 ticks);
	bool testTimer() const;

protected:
	CAITimer _Timer;
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileStandAtPos                                                    //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileStandAtPos
: public CBotProfileWanderBase
, public NLMISC::IDbgPtrData
{
public:
	CBotProfileStandAtPos(CProfileOwner* owner);

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::BOT_STAND_AT_POS; }
	virtual std::string getOneLineInfoString() const;
	//@}

protected:
	NLMISC::CDbgPtr<CSpawnBotNpc>	_Bot;
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileForage                                                        //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileForage
: public CBotProfileWanderBase
, public NLMISC::IDbgPtrData
{
public:
	CBotProfileForage(CProfileOwner* owner);
	virtual ~CBotProfileForage();

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual	AITYPES::TProfiles getAIProfileType () const { return AITYPES::BOT_FORAGE; }
	virtual std::string getOneLineInfoString() const;
	//@}

	void setOldSheet();

protected:
	NLMISC::CDbgPtr<CSpawnBotNpc> _Bot;

	CAITimer _ForageTimer;
	AISHEETS::ICreatureCPtr _OldSheet;
	bool _TemporarySheetUsed;
};

//////////////////////////////////////////////////////////////////////////////
// CActivityProfile                                                         //
//////////////////////////////////////////////////////////////////////////////

class CActivityProfile
: public CAIBaseProfile
{
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileNormal                                                        //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileNormal
: public CActivityProfile
{
public:
	CGrpProfileNormal(CProfileOwner* owner);

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual	AITYPES::TProfiles getAIProfileType () const { return AITYPES::ACTIVITY_NORMAL; }
	virtual std::string getOneLineInfoString() const;
	//@}

	bool isGroupFighting() const;
	void setGroupFighting(bool groupFighting);

protected:
	NLMISC::CDbgPtr<CSpawnGroupNpc>	_Grp;
	bool _GroupFighting;
};

//////////////////////////////////////////////////////////////////////////////
// CSlaveProfile                                                            //
//////////////////////////////////////////////////////////////////////////////
// :TODO: Understand following comment, and act accordingly
// every bot add or remove must be done from a spawn/despawn/die event (and not this way!)

class CSlaveProfile
: public CAIBaseProfile
{
public:
	CSlaveProfile(CProfileOwner* owner);

	/// @name IAIProfile partial implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	//@}

	virtual	void addBot(CBot* bot) = 0;
	virtual	void removeBot(CBot* bot) = 0;

protected:
	NLMISC::CDbgPtr<CSpawnGroup> _Grp;
};

//////////////////////////////////////////////////////////////////////////////
// CMoveProfile                                                             //
//////////////////////////////////////////////////////////////////////////////

class CMoveProfile
: public CSlaveProfile
{
public:
	CMoveProfile(CProfileOwner* owner) ;

	virtual	CPathCont* getPathCont(CBot const* bot) = 0;

	virtual void resumeBot(CBot const* bot);

protected:
	float _MaxRunSpeed;
	float _MaxWalkSpeed;
};

//////////////////////////////////////////////////////////////////////////////
// CFightProfile                                                            //
//////////////////////////////////////////////////////////////////////////////

class CFightProfile
: public CSlaveProfile
{
public:
	CFightProfile(CProfileOwner* owner);

	virtual std::vector<CBot*>& npcList() = 0;
	virtual bool stillHaveEnnemy() const = 0;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileGoToPoint                                                     //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileGoToPoint
: public CMoveProfile
{
public:
	/// @name Constructors, destructor
	//@{
	CGrpProfileGoToPoint(CProfileOwner* owner, RYAI_MAP_CRUNCH::CWorldPosition const& startPos, RYAI_MAP_CRUNCH::CWorldPosition const& endPos, bool dontSendEvent = false);
	virtual ~CGrpProfileGoToPoint();
	//@}

	enum TShapeType
	{
		SHAPE_NOTHING,
		SHAPE_RECTANGLE,
	};
	class CBotFollower
	{
	public:
		CBotFollower();
		virtual ~CBotFollower();
		void setBotAtDest(bool atDest = true);
		const bool& isBotAtDest() const;

	private:
		bool _BotAtDest;
	};

	/// @name CMoveProfile implementation
	//@{
	virtual CPathCont* getPathCont(CBot const* bot);
	//@}

	/// @name CSlaveProfile implementation
	//@{
	virtual void addBot(CBot* bot);
	virtual void removeBot(CBot* bot);
	//@}

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::MOVE_GOTO_POINT; }
	virtual std::string getOneLineInfoString() const;
	virtual void stateChangeProfile();
	virtual void resumeProfile();
	//@}

	bool profileTerminated() const;

protected:
	void setDirection(bool forward);
	bool getDirection();


	void setCurrentDestination(RYAI_MAP_CRUNCH::CWorldPosition const& dest);

	void calcRatios();

	void stopNpc(bool stop);

private:
	void assignGeometryFromState();

	typedef	std::map<CBot*, CBotFollower>	TBotFollowerMap;
	TBotFollowerMap	_NpcList;

	bool		_ValidPosInit;
	bool		_FollowForward;
	bool		_ProfileTerminated;
	bool		_MustCalcRatios;
	TShapeType	_Shape;
	RYAI_MAP_CRUNCH::CWorldPosition _StartPos;
	RYAI_MAP_CRUNCH::CWorldPosition _EndPos;

	/// flag to temporary stop the bots
	bool		_StopNpc;

	// rectangle shape related vars.
	float	_XSize;
	float	_YSize;
	float	_Ratio;
	double	_Cx, _Cy;
	uint32	_NbRange;
	uint32	_NbLines;
	uint32	_NbBotInNormalShape;
	uint32	_Rest;

	uint32		_GeomIndex;
	CPathCont	_PathCont;
	CAIVector	_GlobalOrient;

	AITYPES::TVerticalPos _VerticalPos;
	bool _DontSendEvent;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileFollowRoute                                                   //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileFollowRoute
: public CMoveProfile
{
public:
	CGrpProfileFollowRoute(CProfileOwner* owner, std::vector<CShape::TPosition> const& geometry, AITYPES::TVerticalPos const& verticalPos, bool dontSendEvent = false);
	CGrpProfileFollowRoute(CProfileOwner* owner);

	virtual ~CGrpProfileFollowRoute();

	enum TShapeType
	{
		SHAPE_NOTHING,
			SHAPE_RECTANGLE,
	};

	class CBotFollower
	{
	public:
		CBotFollower();
		virtual ~CBotFollower();
		void setBotAtDest(bool atDest = true);
		const bool& isBotAtDest() const;

	private:
		bool _BotAtDest;
	};

	void setDirection(bool forward);
	bool getDirection();

	void resumeProfile();

	CPathCont* getPathCont(CBot const* bot);

	void addBot(CBot* bot);
	void removeBot(CBot* bot);

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::MOVE_FOLLOW_ROUTE; }
	virtual std::string getOneLineInfoString() const;
	//@}

	bool profileTerminated() const;

	void setCurrentValidPos(AITYPES::TVerticalPos verticalPos);

	void calcRatios();

	virtual void stateChangeProfile();

	void stopNpc(bool stop);

private:
	void assignGeometryFromState();

	typedef	std::map<CBot*, CBotFollower>	TBotFollowerMap;
	TBotFollowerMap	_NpcList;

	bool		_ValidPosInit;
	bool		_FollowForward;
	bool		_ProfileTerminated;
	bool		_MustCalcRatios;
	TShapeType	_Shape;

	/// flag to temporary stop the bots
	bool		_StopNpc;

	// rectangle shape related vars.
	float	_XSize;
	float	_YSize;
	float	_Ratio;
	double	_Cx, _Cy;
	uint32	_NbRange;
	uint32	_NbLines;
	uint32	_NbBotInNormalShape;
	uint32	_Rest;

	uint32		_GeomIndex;
	CPathCont	_PathCont;
	CAIVector	_GlobalOrient;

	bool _GeometryComeFromState;
	std::vector<CShape::TPosition> const* _Geometry;
	AITYPES::TVerticalPos _VerticalPos;
	bool _DontSendEvent;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileStandOnVertices                                               //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileStandOnVertices
: public CMoveProfile
{
public:
	// utility class
	class CBotPositionner : public NLMISC::CRefCount
	{
	public:
		CBotPositionner(RYAI_MAP_CRUNCH::TAStarFlag	flags);
		CBotPositionner(uint32 geomIndex, RYAI_MAP_CRUNCH::TAStarFlag flags);
		void setBotAtDest(bool atDest = true);
		bool isBotAtDest() const;

		CPathCont _PathCont;
		uint32 _GeomIndex;
	private:
		bool _BotAtDest;
	};

	CGrpProfileStandOnVertices(CProfileOwner* owner);
	~CGrpProfileStandOnVertices();

	CPathCont	*getPathCont	(const	CBot *bot);

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::MOVE_STAND_ON_VERTICES; }
	virtual std::string getOneLineInfoString() const;
	//@}
	void resumeProfile();

	void setCurrentValidPos	(CAIStatePositional *grpState);

	void addBot(CBot* bot);
	void removeBot(CBot* bot);

private:
	typedef		std::map<const CBot*,NLMISC::CSmartPtr<CBotPositionner> >	TNpcBotPositionnerMap;
	TNpcBotPositionnerMap			_NpcList;
	bool							_Finished;
	RYAI_MAP_CRUNCH::TAStarFlag		_DenyFlags;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileWander                                                        //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileWander
: public CMoveProfile
{
public:
	CGrpProfileWander(CProfileOwner* owner);
	CGrpProfileWander(CProfileOwner* owner, CNpcZone const* const npcZone);
	virtual ~CGrpProfileWander();

	void setBotStandProfile(AITYPES::TProfiles	botStandProfileType, IAIProfileFactory* botStandProfileFactory);

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::MOVE_WANDER; }
	virtual std::string getOneLineInfoString() const;
	//@}

	void		addBot			(CBot*	bot);
	void		removeBot		(CBot*	bot);
	CPathCont*	getPathCont		(CBot const*	bot);

	void	stateChangeProfile();

	void	affectZoneFromStateMachine();
	void	resetDestinationReachedData();

	void	setTimer(uint32 ticks) { _Timer.set(ticks); }
	bool	testTimer()	const { return _Timer.test(); }

	CNpcZone const* currentZone() const { return _NpcZone; }

private:

	/// the profile type to apply to bot standing between two deplacement
	AITYPES::TProfiles _BotStandProfileType;
	/// the profile factory to apply to bot standing between two deplacement
	IAIProfileFactory*_BotStandProfileFactory;
	/// a flag to force social interaction for the bots
	bool _Social;

	NLMISC::CstCDbgPtr<CPlaceRandomPos> _RandomPos;
	NLMISC::CSmartPtr<const CNpcZone> _NpcZone;
	std::vector<bool>	_NpcDestinationReached;
	bool _DestinationReachedAll;
	bool _DestinationReachedFirst;

	CAITimer _Timer;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileWanderNoPrim                                                  //
//////////////////////////////////////////////////////////////////////////////

/// This profile shouldn't require data primitive for wander zone
class CGrpProfileWanderNoPrim
: public CMoveProfile
{
public:
//	CGrpProfileWanderNoPrim(CProfileOwner* owner);
	CGrpProfileWanderNoPrim(CProfileOwner* owner, NLMISC::CSmartPtr<CNpcZonePlaceNoPrim> const& npcZone);
	virtual ~CGrpProfileWanderNoPrim();

	void setBotStandProfile(AITYPES::TProfiles	botStandProfileType, IAIProfileFactory* botStandProfileFactory);

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::MOVE_WANDER; }
	virtual std::string getOneLineInfoString() const;
	//@}

	void		addBot			(CBot*	bot);
	void		removeBot		(CBot*	bot);
	CPathCont*	getPathCont		(CBot const*	bot);

	void	stateChangeProfile();

//	void	affectZoneFromStateMachine();

	void	setTimer(uint32 ticks) { _Timer.set(ticks); }
	bool	testTimer()	const { return _Timer.test(); }

//	CNpcZone const* currentZone() const { return _NpcZone; }

private:

	/// the profile type to apply to bot standing between two deplacement
	AITYPES::TProfiles _BotStandProfileType;
	/// the profile factory to apply to bot standing between two deplacement
	IAIProfileFactory*_BotStandProfileFactory;
	/// a flag to force social interaction for the bots
	bool _Social;

//	NLMISC::CstCDbgPtr<CPlaceRandomPos> _RandomPos;
	NLMISC::CSmartPtr<CNpcZonePlaceNoPrim> _NpcZone;

	CAITimer _Timer;
};

class CGrpProfileFollowPlayer : 
public CMoveProfile
{
public:
	CGrpProfileFollowPlayer(CProfileOwner* owner, TDataSetRow const& playerRow, uint32 dispersionRadius);
	virtual ~CGrpProfileFollowPlayer() {}

	void setBotStandProfile(AITYPES::TProfiles	botStandProfileType, IAIProfileFactory* botStandProfileFactory);

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile() {}
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::BOT_FOLLOW_POS; }
	virtual std::string getOneLineInfoString() const { return std::string("follow_player group profile"); }
	//@}

	void	stateChangeProfile() {}
	bool	destinationReach() const;

	void		addBot			(CBot*	bot) {}
	void		removeBot		(CBot*	bot) {}
	CPathCont*	getPathCont		(CBot const*	bot) { return NULL; };


protected:
private:
	/// the profile type to apply to bot standing between two deplacement
	AITYPES::TProfiles _BotStandProfileType;
	/// the profile factory to apply to bot standing between two deplacement
	IAIProfileFactory*_BotStandProfileFactory;

	CFollowPath::TFollowStatus	_Status;
	CPathPosition	_PathPos;
	CPathCont		_PathCont;
	CAIVector		_LastPos;

	TDataSetRow	_PlayerRow;
	uint32      _DispersionRadius;
};


//////////////////////////////////////////////////////////////////////////////
// CGrpProfileIdle                                                          //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileIdle
: public CMoveProfile
{
public:
	CGrpProfileIdle(CProfileOwner* owner);
	virtual ~CGrpProfileIdle();

	class CBotPositionner
	{
	public:
		CBotPositionner();
		virtual ~CBotPositionner();
	};

	CPathCont* getPathCont(CBot const* bot);
	virtual void beginProfile();
	void resumeProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);

	void addBot(CBot* bot);
	void removeBot(CBot* bot);

	virtual std::string getOneLineInfoString() const;
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::MOVE_IDLE; }

private:
	typedef	std::map<CBot*,CBotPositionner>	TNpcBotPositionnerMap;
	TNpcBotPositionnerMap _NpcList;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileBandit                                                        //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileBandit
: public CGrpProfileNormal
{
public:
	CGrpProfileBandit(CProfileOwner* owner);
	virtual ~CGrpProfileBandit();

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType () const { return AITYPES::ACTIVITY_BANDIT; }
	virtual std::string getOneLineInfoString() const;
	//@}

private:
	uint32 _AggroRange;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileEscorted                                                      //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileEscorted
: public CGrpProfileNormal
{
public:
	CGrpProfileEscorted(CProfileOwner* owner);
	virtual ~CGrpProfileEscorted();

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType () const { return AITYPES::ACTIVITY_ESCORTED; }
	virtual std::string getOneLineInfoString() const;
	//@}

	void stateChangeProfile();

protected:
	bool _EscortTeamInRange;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileGuard                                                         //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileGuard
: public CGrpProfileNormal
{
public:
	CGrpProfileGuard(CProfileOwner* owner);
	virtual ~CGrpProfileGuard();

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ACTIVITY_GUARD; }
	virtual std::string getOneLineInfoString() const;
	//@}

private:
	CAIVector _CenterPos;
	uint32 _AggroRange;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileGuardEscorted                                                 //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileGuardEscorted
: public CGrpProfileNormal
{
public:
	CGrpProfileGuardEscorted(CProfileOwner* owner);
	virtual ~CGrpProfileGuardEscorted();

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ACTIVITY_GUARD_ESCORTED; }
	virtual std::string getOneLineInfoString() const;
	//@}

	void stateChangeProfile();

protected:
	NLMISC::CSmartPtr<CGrpProfileGuard>    _GuardProfile;
	NLMISC::CSmartPtr<CGrpProfileEscorted> _EscortedProfile;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileFaction                                                       //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileFaction
: public CGrpProfileNormal
{
public:
	CGrpProfileFaction(CProfileOwner* owner);
	virtual ~CGrpProfileFaction();

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ACTIVITY_FACTION; }
	virtual std::string getOneLineInfoString() const;
	//@}

	virtual void aggroEntity(CAIEntityPhysical const* entity);
	void checkTargetsAround();
	void noAssist() { bNoAssist = true; }

	static std::string scriptFactionToFameFaction(std::string name);
	static std::string fameFactionToScriptFaction(std::string name);

	static bool scriptFactionToFameFactionGreaterThan(std::string name);
	static sint32 scriptFactionToFameFactionValue(std::string name);


private:
	CAITimer	_checkTargetTimer;
	bool		bNoAssist;

private:
	static AITYPES::CPropertySet _FameFactions;
	static void initFameFactions();
	static bool entityHavePartOfFactions(CAIEntityPhysical const* entity, AITYPES::CPropertySetWithExtraList<TAllianceId> const& factions);
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileSquad                                                         //
// The Manager Parent of the Manager of this Grp class is COutpost          //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileSquad
: public CGrpProfileFaction
{
public:
	CGrpProfileSquad(CProfileOwner* owner);
	virtual ~CGrpProfileSquad();

	/// @name IAIProfile implementation
	//@{
	virtual void beginProfile();
	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::ACTIVITY_SQUAD; }
	virtual std::string getOneLineInfoString() const;
	//@}

	virtual void aggroEntity(CAIEntityPhysical const* entity);

	NLMISC::CSmartPtr<CAIPlace const> buildFirstHitPlace(TDataSetRow const& aggroBot);

protected:

	/// Return the outpost to which the squad belongs, or NULL if not found
	COutpost* getDefendedOutpost() { return dynamic_cast<COutpost*>(_Grp->getPersistent().getOwner()->getOwner()); }
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileFight                                                    //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileFight
: public CFightProfile
, public CFightOrganizer
{
	// TODO : set this parameters thrue the group, no hardcode them
	enum
	{

		CHECK_AROUND_PERIOD = 50, ///< Check every 5s
			CHECK_AROUND_RADIUS = 60, ///< Check 60 meters around
	};
public:
	CGrpProfileFight(CProfileOwner *owner);
	virtual ~CGrpProfileFight();

	virtual void beginProfile();
	void	endProfile();

	void	addBot	(CBot *bot);
	void	removeBot	(CBot *bot);

	//////////////////////////////////////////////////////////////////////////
	/// @name CFightOrganizer
	//@{
	virtual void setFight(CSpawnBot* bot, CAIEntityPhysical* ennemy);
	virtual void setHeal(CSpawnBot* bot, CAIEntityPhysical* target);
	virtual void setNoFight(CSpawnBot* bot);
	virtual void setFlee(CSpawnBot* bot, CAIVector& fleeVect);
	virtual void setReturnAfterFight(CSpawnBot* bot);
	//@}

	bool stillHaveEnnemy	() const;

	virtual	void	updateProfile(uint ticksSinceLastUpdate);

	virtual	std::string	getOneLineInfoString() const;

	virtual	AITYPES::TProfiles getAIProfileType() const { return AITYPES::FIGHT_NORMAL; }

	std::vector<CBot*>	&npcList();

private:
	std::vector<CBot*> _NpcList;
	/// A timer to check the friend groups around and set their aggro list if they are not fighting
	CAITimer _CheckAround;
	bool _WasRunning;
};

/****************************************************************************/
/* Profile factories                                                        */
/****************************************************************************/

//- Simple profile factories (based on generic factory) ----------------------

// CBotProfileStandAtPos
typedef CAIGenericProfileFactory<CBotProfileStandAtPos> CBotProfileStandAtPosFactory;

// CBotProfileForage
typedef CAIGenericProfileFactory<CBotProfileForage> CBotProfileForageFactory;

// CGrpProfileNormal
typedef CAIGenericProfileFactory<CGrpProfileNormal> CGrpProfileNormalFactory;

// CGrpProfileFollowRoute
typedef CAIGenericProfileFactory<CGrpProfileFollowRoute> CGrpProfileFollowRouteFactory;

// CGrpProfileWander
typedef CAIGenericProfileFactory<CGrpProfileWander> CGrpProfileWanderFactory;

// CGrpProfileGuardFactory
typedef CAIGenericProfileFactory<CGrpProfileGuard> CGrpProfileGuardFactory;

// CGrpProfileEscortedFactory
typedef CAIGenericProfileFactory<CGrpProfileEscorted> CGrpProfileEscortedFactory;

// CGrpProfileGuardEscortedFactory
typedef CAIGenericProfileFactory<CGrpProfileGuardEscorted> CGrpProfileGuardEscortedFactory;

// CGrpProfileFactionFactory
typedef CAIGenericProfileFactory<CGrpProfileFaction> CGrpProfileFactionFactory;

// CGrpProfileSquadFactory
typedef CAIGenericProfileFactory<CGrpProfileSquad> CGrpProfileSquadFactory;

// CGrpProfileStandOnVerticesFactory
typedef CAIGenericProfileFactory<CGrpProfileStandOnVertices> CGrpProfileStandOnVerticesFactory;

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileNoChange                                                      //
//////////////////////////////////////////////////////////////////////////////

/// No change special profile
class CGrpProfileNoChangeFactory
: public IAIProfileFactory
{
public:
	NLMISC::CSmartPtr<IAIProfile> createAIProfile(CProfileOwner* owner)
	{
		return NULL;
	}
};
RYAI_DECLARE_PROFILE_FACTORY(CGrpProfileNoChangeFactory);

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileBanditFactory                                                 //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileBanditFactory
: public IAIProfileFactory
{
public:
	NLMISC::CSmartPtr<IAIProfile> createAIProfile(CProfileOwner* owner);
	static float getDefaultBanditAggroRange();
private:
	static float _DefaultAggroRange;
};

//- Singleton profiles (stateless ones) --------------------------------------

extern CBotProfileStandAtPosFactory BotProfileStandAtPosFactory;
extern CBotProfileForageFactory BotProfileForageFactory;

#endif
