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



#ifndef CL_USER_ENTITY_H
#define CL_USER_ENTITY_H


/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
// 3D Interface.
#include "nel/3d/u_visual_collision_entity.h"
// Client DB
#include "nel/misc/cdb.h"
// Client
#include "player_cl.h"
#include "client_cfg.h"
// Std.
#include <list>
// sp_types
#include "game_share/sp_type.h"
// r2 types
#include "game_share/r2_types.h"


///////////
// USING //
///////////
using NLMISC::CVector;
using NL3D::UVisualCollisionEntity;
//using std::list;


class CItemImage;

///////////
// CLASS //
///////////
/**
 * Class to manage an user entity.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CUserEntity : public CPlayerCL
{
public:
	enum TView
	{
		FirstPV = 0,	// First Person View
		ThirdPV			// Third Person View
	};
	// Move To Actions
	enum TMoveToAction
	{
		None = 0,
		Attack,
		Quarter,
		Loot,
		PickUp,
		ExtractRM,
		TradeItem,
		TradePhrase,
		TradePact,
		Mission,
		DynamicMission,
		StaticMission,
		CreateGuild,
		News,
		TradeTeleport,
		TradeFaction,
		TradeCosmetic,
		Talk,
		CombatPhrase,
		Mount,
		WebPage,
		Outpost,
		BuildTotem,
		MissionRing,
	};

public:
	NLMISC_DECLARE_CLASS(CUserEntity);

	/// Constructor
	CUserEntity();
	/// Destructor
	virtual ~CUserEntity();

	/// Build the entity from a sheet.
	virtual bool build(const CEntitySheet *sheet);

	/**
	 * Apply the motion to the entity.
	 */
	 void applyMotion(CEntityCL *target);

	/**
	 * Apply ForceLook (computed at prec applyMotion).
	 */
	 void applyForceLook();

	/**
	 * Update precollision
	 */
	virtual void updatePreCollision(const NLMISC::TTime &time, CEntityCL *target);

	/**
	 * Update the position of the entity after the motion.
	 * \param target : pointer on the current target.
	 */
	virtual void updatePosCombatFloatChanged(CEntityCL *target);
	/**
	 * Update the position of the entity after the motion.
	 * \param time : Time for the position of the entity after the motion.
	 * \param target : pointer on the current target.
	 */
	virtual void updatePos(const NLMISC::TTime &time, CEntityCL *target);

	/**	Update the PACS position after the evalCollision. The entity position is set too. This is fast.
	 *	If the entity position is too far from its PACS position, setGlobalPosition is called.
	 *	After this call, the position.z is valid.
	 */
	virtual void pacsFinalizeMove();

	/**
	 * Update the sound management to play user steps
	 */
	void	updateSound(const NLMISC::TTime &time);

	virtual void snapToGround();

	/**
	 * 'true' if the user is dead too much and have to be disconnected.
	 */
	bool permanentDeath() const {return _PermanentDeath;}


	/// Method called to change the mode (Combat/Mount/etc.).
	virtual bool mode(MBEHAV::EMode m);
	/// Set the mode for the entity
	virtual MBEHAV::EMode mode() const {return _Mode;}

	/// Return the mount entity if the user is riding, otherwise NULL
	CEntityCL* getMountEntity();

	/// Return the DB entry for the specified user's animal (NULL if not found)
	NLMISC::CCDBNodeBranch *getBeastDBEntry( CLFECOMMON::TClientDataSetIndex uid );

	/** To Inform about an entity removed (to remove from selection for example).
	 * This will remove the entity from the target.
	 * \param slot : Slot of the entity that will be removed.
	 */
	virtual void slotRemoved(const CLFECOMMON::TCLEntityId &slot);

	/** Change the entity selected.
	 * \param slot : slot now selected (CLFECOMMON::INVALID_SLOT for an empty selection).
	 * \warning Can be different from the entity targeted (in combat mode for example).
	 */
	virtual void selection(const CLFECOMMON::TCLEntityId &slot);
	/** Return the entity selected.
	 * \return CLFECOMMON::TCLEntityId : slot currently selected (CLFECOMMON::INVALID_SLOT for an empty selection).
	 * \warning Can be different from the entity targeted (in combat mode for example).
	 */
	virtual const CLFECOMMON::TCLEntityId &selection() const {return _Selection;}

	/** Set the trader (see also trader() and interlocutor())
	 * \param slot : trader slot (CLFECOMMON::INVALID_SLOT if no trader).
	 */
	virtual void trader(const CLFECOMMON::TCLEntityId &slot);
	/** Return the trader entity that will be turned towards the user entity (client-side)
	 * and other stuff such as distance checking (see also interlocutor())
	 * \return CLFECOMMON::TCLEntityId : slot of the trader (CLFECOMMON::INVALID_SLOT if no trader).
	 */
	virtual const CLFECOMMON::TCLEntityId &trader() const {return _Trader;}

	/** Set the current interlocutor entity that will be turned towards the user entity (client-side).
	 * This one is for dyn-chat, while trader() is for bot-chat.
 	 * \param slot : interlocutor slot (CLFECOMMON::INVALID_SLOT if no interlocutor).
	 */
	void interlocutor( const CLFECOMMON::TCLEntityId &slot);
	/** Return the current interlocutor entity that will be turned towards the user entity (client-side).
	 * This one is for dyn-chat, while trader() is for bot-chat.
	 * \return CLFECOMMON::TCLEntityId : slot of the interlocutor (CLFECOMMON::INVALID_SLOT if no interlocutor).
	 */
	const CLFECOMMON::TCLEntityId &interlocutor() const {return _Interlocutor;}

	/// Method to place the user to attack the target and attack.
	void moveToAttack();
	/// Method to attack the target.
	void attack();
	/// Method to disengage the target.
	void disengage();
	/// Method to attack the target, with a special phrase
	void attackWithPhrase();
	/// your current target become the target of your current one.
	void assist();
	/// the entity target in the slot become the target of your current one.
	void assist(uint slot);

	/// Ask fot the client to sit/stand ('true' to sit).
	bool sit(bool s);
	/// Return true if the user can sit.
	bool canSit() const;

	/// Set the user away from keyboard (or not), txt can be customized
	void setAFK(bool b, std::string afkTxt="");

	/// Roll a dice and tell the result around
	void rollDice(sint16 min, sint16 max);

	/// return true if user can engage melee combat, else return false and display system msg
	bool canEngageCombat();


	/** \name VELOCITY
	 * Method to manage the user velocity.
	 */
	//@{
	/**
	 * Set the velocity when the user walks.
	 * \param velocity : new velocity for the user when he walks.
	 */
	inline void walkVelocity(float velocity)
	{
		_WalkVelocity = velocity;
		if(!_Run)
			_CurrentVelocity = _WalkVelocity;
	}
	/**
	 * Get the velocity when the user walks.
	 * \return float : User velocity when he walks.
	 */
	inline float walkVelocity() const {return _WalkVelocity;}
	/**
	 * Set the velocity when the user runs.
	 * \param velocity : new velocity for the user when he runs.
	 */
	inline void runVelocity(float velocity)
	{
		_RunVelocity = velocity;
		if(_Run)
			_CurrentVelocity = _RunVelocity;
	}
	/**
	 * Get the velocity when the user runs.
	 * \return float : User velocity when he runs.
	 */
	inline float runVelocity() const {return _RunVelocity;}

	/**
	 * Get the current velocity of the user.
	 * \return float : Current User velocity.
	 */
	inline float currentVelocity() const {return _CurrentVelocity;}

	/**
	 * Switch velocity between Run and Walk.
	 * \param userRequest : true if user asked it
	 */
	void switchVelocity(bool userRequest = true);

	/**
	  * Test if user is running
	  */
	bool        running() const { return _Run; }
	//@}

	/// Get the front velocity.
	inline float frontVelocity() const {return _FrontVelocity;}
	/// Set the front velocity.
	inline void frontVelocity(float velocity) {_FrontVelocity = velocity;}

	/// Get the lateral velocity.
	inline float lateralVelocity() const {return _LateralVelocity;}
	/// Set the lateral velocity.
	inline void lateralVelocity(float velocity) {_LateralVelocity = velocity;}

	// get the velocity vector of the entity
	NLMISC::CVector getVelocity() const;

	/// Check if the mount is able to run, and force walking mode if not
	void checkMountAbleToRun();

	/// Get Eyes Height.
	float eyesHeight();
	/// Set Eyes Height.
	inline void eyesHeight(float h) {_EyesHeight = h;}

	/// Head Pitch methods
	void	setHeadPitch(double hp);
	double	getHeadPitch() const {return _HeadPitch;}
	void	rotHeadVertically(float ang);

	/// Get the playersheet
	inline const CPlayerSheet *sheet() const {return _Sheet;}


	/// rotate the body on the left or right (front changes).
	void rotate(float ang);


	/// TODO: Ben tests for landscape pacs bug
	const NLPACS::UMovePrimitive	*getMovePrimitive() const { return _Primitive; }

	/// Remove the primitive
	virtual void removePrimitive();
	/// Create a primitive for the entity.
	virtual void computePrimitive();

	/// Remove the check primitive
	void removeCheckPrimitive();

	/// Is the user selectable (first or third person view for example).
	bool selectable() const {return _Selectable;}
	/// Set is the user is selectable or not.
	void selectable(bool s) {_Selectable = s;}

	/// Return if the user is already busy (combat/bo chat/loot/ etc.).
	bool isBusy() const;

	/// Return 'true' if the user is on a mount.
	bool isOnMount() const {return _OnMount;}

	/** \name DEBUG
	 * Methods only here for the debug.
	 */
	//@{
	/// Display Debug Information.
	virtual void displayDebug(float x, float &y, float lineStep);
	//@}

	void updateVisualDisplay();

	/// Return 'true' is an attack animation is currently playing.
	bool isAttacking() const {return _AnimAttackOn;}

	/// Show/Hide the user light.
	void light();

	/// Display dmg/heal numbers above the head.
	virtual void displayModifiers();

	/** \name VIEW
	 * Methods about the view (First/Third Person View).
	 */
	//@{
	/// Change the View (First/Third Person View).
	void  viewMode(TView viewMode, bool changeView=true);
	/// Return the current View Mode.
	TView viewMode() const {return _ViewMode;}
	/// Toggle Camera (First/Third Person)
	void toggleCamera();
	//@}

	/// Return the entity scale. (return 1.0 if there is any problem).
	virtual float getScale() const;
	/// Return 'true' is the entity is displayed.
	virtual bool isVisible() const;

	/// Return true if the user is indoor and the CFG want to force the FPV Indoor.
	bool forceIndoorFPV();
	bool moveTo() const {return (_MoveToSlot != CLFECOMMON::INVALID_SLOT);}
	/// Return true if the User is following an entity.
	bool follow() const {return _FollowMode;}
	/// set true to resetCameraRot if you want that camera rotation follow
	void enableFollow(bool resetCameraRot);
	void disableFollow();
	// when the user request to move the head pitch, stop the "follow mode" to force it
	void stopForceHeadPitchInFollow();
	/// Method to move to someone else. NB: resetAnyMoveTo() is called first
	void moveTo(CLFECOMMON::TCLEntityId slot, double dist, TMoveToAction action);
	/// Method to move to someone else for a mission. NB: resetAnyMoveTo() is called first
	void moveToMission(CLFECOMMON::TCLEntityId slot, double dist, uint32 id);
	/// Method to move to someone else for a ring mission. NB: resetAnyMoveTo() is called first
	void moveToMissionRing(CLFECOMMON::TCLEntityId slot, double dist, uint32 id);
	/** Method to move to someone else, for foraging extraction
	 *	The caller MUST call after CSPhraseManager::clientExecute(), to increment action counter
	 *	NB: resetAnyMoveTo() is called first
	*/
	void moveToExtractionPhrase(CLFECOMMON::TCLEntityId slot, double dist, uint phraseMemoryLine, uint phraseMemorySlot, bool cyclic);
	/** Method to begin a spire construction
	 *	The caller MUST call after CSPhraseManager::clientExecute(), to increment action counter
	 *	NB: resetAnyMoveTo() is called first
	*/
	void moveToTotemBuildingPhrase(CLFECOMMON::TCLEntityId slot, double dist, uint phraseMemoryLine, uint phraseMemorySlot, bool cyclic);
	/// Reset any MoveTo. NB: if the current moveTo is a moveToCombatPhrase() or a moveToExtractionPhrase(), decrement action counter
	void resetAnyMoveTo();
	/** Launch the Action Once the Move is done.
	 * \param CEntityCL * : pointer on the destination entity.
	 * \warning entity pointer must be valid(allocated).
	 */
	void moveToAction(CEntityCL *ent);
	/// Send the position and orientation to the server.
	bool sendToServer(NLMISC::CBitMemStream &out);
	/// Fill the msg to know if the user is well placed to fight.
	bool msgForCombatPos(NLMISC::CBitMemStream &out);
	/// Check the User Position according to the server code.
	void checkPos();
	/// Check a position according to the server code.
	bool testPacsPos(NLMISC::CVectorD& pos);
	/// Teleport the player (remove selection, re-init checkPos, etc.).
	void tp(const NLMISC::CVectorD &dest);
	/// Teleport the player to correct his position.
	void correctPos(const NLMISC::CVectorD &dest);
	/// Skill Up
	void skillUp();
	/// get the level of the player (max of all skills)
	sint getLevel() const;

	/// After a few time, if the user is still in collision with someone else, remove collisions with other entitites.
	void startColTimer();
	/// Called when the user is no more in collision with another entity.
	void stopColTimer();

	/// Make the character transparent if the mouse is under it (params is if we must make it transparent or opaque)
	virtual void makeTransparent(bool t);
	virtual void setDiffuse(bool onOff, NLMISC::CRGBA diffuse);

	/// false if in first person mode
	virtual bool	canCastShadowMap() const;

	/// Return the Entity Current Speed.
	virtual double speed() const;

	/// assert(target). NB: this engage (moveToCombatPhrase) if in melee and to far, or directly launch the action
	void	executeCombatWithPhrase(CEntityCL	*target, uint32 memoryLine, uint32 memoryIndex, bool cyclic);

	/// equip with the last weapon used of with the best weapon that can be found in inventory
	void	autoEquipWithLastUsedWeapons();

	/// save the last weapon(s) used in fight or weapons in hand before tool auto-equip
	void	rememberWeaponsInHand();

	/// overriden beginCast method
	virtual void beginCast(const MBEHAV::CBehaviour &behaviour);

	/// Return the walk speed applicable when riding
	float	getMountWalkVelocity() const { return _MountSpeeds.getWalkSpeed(); }

	/// Return the run speed applicable when riding
	float	getMountRunVelocity() const { return _MountSpeeds.getRunSpeed(); }

	/// \name R2 specific
	// @{
	// R2 char mode (player, dm, anim, edit) NB: automatically affect the camera max distance and player run/walk speed
	void			setR2CharMode(R2::TCharMode mode);
	R2::TCharMode	getR2CharMode() const {return _R2CharMode;}
	// re-update view camera dist max, and walk/run speed according to current R2 char mode
	void			flushR2CharMode() {setR2CharMode(_R2CharMode);}
	// @}
	// When a player control a npc (in a ring shard) then he must adapt his speed on the Npc speed.
	void	updateNpcContolSpeed();
	bool isInNpcControl() const;

	/// cancel current action
	void cancelAllPhrases();

	/// true if current behaviour allows to change front
	bool canChangeFront();

	ucstring getLoginName()
	{
		if (_LoginName == ucstring(""))
			_LoginName = getDisplayName();

		return _LoginName;
	}

protected:
	class CSpeedFactor : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		/// Initialize
		void init();
		/// Release
		void release();
		/// Return the speed factor.
		float getValue() const {return _Value;}
		virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) {f.serial(_Value);}
	protected:
		/// Method called when the ping message is back.
		virtual void update(NLMISC::ICDBNode* leaf);
	private:
		float _Value;
	};

	class CMountHunger
	{
	public:
		/// Initialize
		void init();
		/// Release
		void release();
		/// Return true if the mount can run. Precondition: UserEntity->isRiding().
		bool canRun() const;
		virtual void serial(class NLMISC::IStream &/* f */) throw(NLMISC::EStream) {}
	};

	class CMountSpeeds : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		/// Initialize
		void init();
		/// Release
		void release();
		/// Return the walk speed when riding
		float getWalkSpeed() const { return _WalkSpeed; }
		/// Return the run speed when riding
		float getRunSpeed() const { return _RunSpeed; }
	protected:
		/// Method called when the value is changed
		virtual void update(NLMISC::ICDBNode* leaf);
	private:
		float _WalkSpeed;
		float _RunSpeed;
	};

	/// Speed Factor to use for the user sent by the server to respect magical spells
	CSpeedFactor				_SpeedFactor;
	/// The current maximum speed of the mount (if any)
	CMountHunger				_MountHunger;
	/// An observer of the current mount speeds
	CMountSpeeds				_MountSpeeds;
	/// Velocity : Front and Lateral
	float						_FrontVelocity;
	float						_LateralVelocity;
	/// Head Pitch
	double						_HeadPitch;
	/// Height of the eyes (camera).
	float						_EyesHeight;
	/// 'True' if the user is running else is walking.
	bool						_Run;
	/// 'True' if the user & mount should run but are walking because of the mount unability to run
	bool						_RunWhenAble;
	/// Speed when the user walks.
	float						_WalkVelocity;
	/// Speed when the user runs.
	float						_RunVelocity;
	/// Current User Velocity.
	float						_CurrentVelocity;
	/// Slot Currently Selected (can be different from _TargetSlot).
	CLFECOMMON::TCLEntityId		_Selection;
	/// Trader Slot (bot chat)
	CLFECOMMON::TCLEntityId		_Trader;
	/// Interlocutor Slot (dyn chat)
	CLFECOMMON::TCLEntityId		_Interlocutor;
	bool						_Selectable;
	bool						_PermanentDeath;
	bool						_FollowMode;
	/// Last position validated by the Check system
	NLMISC::CVectorD			_LastPositionValidated;
	/// Last Validated Global Position.
	NLPACS::UGlobalPosition		_LastGPosValidated;
	/// Last Global Position sent to the server (in fact it's a normal position sent).
	NLPACS::UGlobalPosition		_LastGPosSent;
	/// Primitive used to check if the server will accept the current move.
	NLPACS::UMovePrimitive		*_CheckPrimitive;

	CLFECOMMON::TCLEntityId		_MoveToSlot;
	double						_MoveToDist;
	TMoveToAction				_MoveToAction;
	uint						_MoveToPhraseMemoryLine; // used for extraction as well
	uint						_MoveToPhraseMemorySlot; // used for extraction phrase as well
	bool						_MoveToPhraseCyclic;
	uint32						_MoveToMissionId;		 // Used for both mission option and mission ring
	/// Time in MS when the User started beiing in collision with anything that avoid him to do an Action (and still is).
	sint64						_MoveToColStartTime;


	/// CSkill points observer
	class CSkillPointsObserver : public NLMISC::ICDBNode::IPropertyObserver
	{
	public :
		uint	SpType;

		/// From ICDBNode::IPropertyObserver
		virtual void update(NLMISC::ICDBNode* node );
	};
	CSkillPointsObserver		_SkillPointObs[EGSPD::CSPType::EndSPType];

	class CInvisibleObserver : public NLMISC::ICDBNode::IPropertyObserver
	{
	public :
		virtual void update(NLMISC::ICDBNode* node);
	};
	CInvisibleObserver			_InvisibleObs;

	/// Fame observer
	class CFameObserver : public NLMISC::ICDBNode::IPropertyObserver
	{
	public :
		uint32	FactionIndex;

		/// From ICDBNode::IPropertyObserver
		virtual void update(NLMISC::ICDBNode* node );
	};
	std::vector<CFameObserver *>		_FamesObs;

protected:
	/// Initialize properties of the entity (according to the class).
	virtual void initProperties();

	/// Update Entity Position.
	virtual void updateVisualPropertyPos           (const NLMISC::TGameCycle &gameCycle, const sint64 &prop, const NLMISC::TGameCycle &pI);
	/// Update Entity Orientation.
	virtual void updateVisualPropertyOrient        (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Behaviour.
	virtual void updateVisualPropertyBehaviour     (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Name.
	virtual void updateVisualPropertyName          (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Target.
	virtual void updateVisualPropertyTarget        (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Mode.
	virtual void updateVisualPropertyMode          (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Visual Property A
	virtual void updateVisualPropertyVpa           (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Visual Property B
	virtual void updateVisualPropertyVpb           (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Visual Property C
	virtual void updateVisualPropertyVpc           (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Mount
	virtual void updateVisualPropertyEntityMounted (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Rider
	virtual void updateVisualPropertyRiderEntity   (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Target lists
	virtual void updateVisualPropertyTargetList	   (const NLMISC::TGameCycle &gameCycle, const sint64 &prop, uint listIndex);
	/// visual fx
	virtual void updateVisualPropertyVisualFX	   (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// PVP Mode
	virtual void updateVisualPropertyPvpMode	   (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Pvp Outpost
	virtual void updateVisualPropertyOutpostInfos  (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Pvp Clan
	virtual void updateVisualPropertyPvpClan	   (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);


	/// Apply the behaviour for the user.
	virtual void applyBehaviour(const CBehaviourContext &behaviour);
	/// Method to Flag the character as dead and do everything needed.
	virtual void setDead();

	/// Mount the mount in _Mount
	void	setOnMount();

	/** Return the current max speed for the entity in meter per sec
	 * The method return a max according to the speed factor (given by the server)
	 * It's also return a value according to the landscape (water)
	 * Also managed mounts
	 */
	virtual double getMaxSpeed() const;

	/// Read/Write Variables from/to the stream.
	virtual void readWrite(class NLMISC::IStream &f) throw(NLMISC::EStream);
	/// To call after a read from a stream to re-initialize the entity.
	virtual void load();


	/// Extract RM from a select forage source
	void extractRM();


	/// Build a Totem
	void buildTotem();

private:
	/// TO know if the user is on a mount at the moment.
	bool _OnMount;
	/// Is the attack animation is currently playing.
	bool _AnimAttackOn;
	/// Current View Mode (First/Third Person View).
	TView _ViewMode;
	/// 'true' if the collisions between the user and other entities are removed.
	bool	_ColRemoved;
	/// 'true' if the User is in collision with someone else.
	bool	_ColOn;
	/// Time in MS when the User started beiing in collision with another entity (and still is).
	sint64	_ColStartTime;

	/// Force Look also modify the head pitch in First person
	bool		_FollowForceHeadPitch;
	/// The Head Offset to follow is kept here.
	float		_FollowHeadOffset;

	/// Check if the user is not already well placed.
	void moveToCheckStartDist(CLFECOMMON::TCLEntityId slot, double dist, TMoveToAction action);

	/// forceLook an entity (follow or moveTo)
	CLFECOMMON::TCLEntityId		_ForceLookSlot;
	void forceLookEntity(const NLMISC::CVectorD &dir2targ, bool updateHeadPitch, bool start = false);
	void startForceLookEntity(CLFECOMMON::TCLEntityId slot);

	/** Method to move to someone else, for special melee combat
	 *	The caller MUST call after CSPhraseManager::clientExecute(), to increment action counter
	 *	NB: resetAnyMoveTo() is called first
	 */
	void moveToCombatPhrase(CLFECOMMON::TCLEntityId slot, double dist, uint phraseMemoryLine, uint phraseMemorySlot, bool phraseCyclic);

	/// For executeCombatWithPhrase
	CLFECOMMON::TCLEntityId		_LastExecuteCombatSlot;

	/// R2: to know in which mode is the current entity (affect camera view and run speed)
	R2::TCharMode		_R2CharMode;

	/// snapshot of a CItemImage
	struct CItemSnapshot
	{
		uint32 Sheet;
		uint16 Quality;
		uint16 Quantity;
		uint8  UserColor;
		uint32 Price;
		uint32 Weight;
		uint32 NameId;
		uint8  InfoVersion;

		CItemSnapshot()
		{
			Sheet = 0;
			Quality = 0;
			Quantity = 0;
			UserColor = 0;
			Price = 0;
			Weight = 0;
			NameId = 0;
			InfoVersion = 0;
		}

		CItemSnapshot( const CItemImage& i );
	};
	/// previous items in hand before they have been changed by an auto-equip due to an action (ex: forage)
	CItemSnapshot			_PreviousRightHandItem;
	CItemSnapshot			_PreviousLeftHandItem;

	ucstring _LoginName;
};

/// Out game received position
extern NLMISC::CVectorD UserEntityInitPos;
extern NLMISC::CVector	UserEntityInitFront;
extern CUserEntity		*UserEntity;

/// char and account time properties
extern uint32		CharFirstConnectedTime;
extern uint32		CharPlayedTime;

/// Max distance between player and extracted forage source
extern const double MaxExtractionDistance;

#endif // CL_USER_ENTITY_H

/* End of user_entity.h */
