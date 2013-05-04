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



#ifndef CL_CHARACTER_CL_H
#define CL_CHARACTER_CL_H


/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
// 3d.
#include "nel/3d/u_play_list.h"
#include "nel/3d/target_anim_ctrl.h"
#include "nel/3d/u_particle_system_instance.h"
// Client.
#include "entity_cl.h"
#include "stage.h"
#include "entity_animation_manager.h"
#include "fx_manager.h"
#include "behaviour_context.h"
#include "attack_info.h"
#include "attached_fx.h"
// Client Sheets
#include "client_sheets/character_sheet.h"
#include "client_sheets/fx_sheet.h"
#include "client_sheets/attack_id_sheet.h"
// std
#include <map>
#include <vector>
// Game Share
#include "game_share/slot_types.h"
#include "game_share/gender.h"
#include "game_share/mode_and_behaviour.h"


extern NL3D::UScene *Scene;

///////////
// CLASS //
///////////
class CCharacterSheet;
class CItemSheet;
class CUserEntity;
class CAnimationFX;
class CItemFXSheet;
class CAttack;
class CGroupInSceneBubble;
class CAttackSheet;
class CProjectileBuild;



/**
 * Class to manage a character.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CCharacterCL : public CEntityCL
{
	friend class CUserEntity;

public:
	/// Stages to describ entity actions/motion.
	CStageSet									_Stages;
	enum { MaxNumAura = 2 };
	enum	TAtkHeight
	{
		AtkUnkownHeight = 0,
		AtkLow,
		AtkMiddle,
		AtkHigh
	};

public:
	NLMISC_DECLARE_CLASS(CCharacterCL);

	/** \name CONSTRUCTOR and DESTRUCTOR
	 * Functions to construct or destroy the class.
	 */
	//@{
	/// Default Constructor
	CCharacterCL();
	/// Default Destructor
	virtual ~CCharacterCL();
	//@}

	/// Build the entity from a sheet.
	virtual bool build(const CEntitySheet *sheet);

	/// \from CEntityCl
	virtual void updatePreCollision(const NLMISC::TTime &time, CEntityCL *target);
	virtual void updatePostCollision(const NLMISC::TTime &time, CEntityCL *target);
	virtual void updateVisible (const NLMISC::TTime &currentTimeInMs, CEntityCL *target);
	virtual void updateSomeClipped (const NLMISC::TTime &currentTimeInMs, CEntityCL *target);
	virtual void updateClipped (const NLMISC::TTime &currentTimeInMs, CEntityCL *target);

	// Get the move for the MOVE channel with the blend.
	double getTheMove(double loopTimeStep, double oldMovingTimeOffset, double         oldMovingTimeOffsetRun) const;
	// Get the move for any channel.
	double getTheMove(double loopTimeStep, double oldMovingTimeOffset, TAnimationType channel               ) const;

	/**
	 * Update the position of the entity after the motion.
	 * \param target : pointer on the current target.
	 */
	virtual void updatePosCombatFloatChanged(CEntityCL * /* target */) {}
	/**
	 * Update the position of the entity after the motion.
	 * \param target : pointer on the current target.
	 */
	virtual void updatePosCombatFloat(double frameTimeRemaining, CEntityCL *target);
	/**
	 * Update the position of the entity after the motion.
	 * \param time : Time for the position of the entity after the motion.
	 * \param target : pointer on the current target.
	 */
	virtual void updatePos(const NLMISC::TTime &currentTimeInMs, CEntityCL *target);
	/** Update the entity after all positions done.
	 */
	virtual void updateVisiblePostPos(const NLMISC::TTime &time, CEntityCL *target);
	/** Update the entity after the render like for the head offset.
	 */
	virtual void updateVisiblePostRender();
	/** Update all the entity after the render visible or not
	 */
	virtual void updateAllPostRender();

	// from CEntityCL
	virtual void computePrimitive();

	/// Get the face instance. Returns NULL if no face.
	virtual SInstanceCL *getFace ();

	/// Update eyes blink. For the moment, called by updatePos.
	void updateBlink(const NLMISC::TTime &currentTimeInMs);

	/// Update system FX
	void updateFX();

	/// Get the entity position and set all visual stuff with it.
	virtual void updateDisplay(CEntityCL *parent = 0);

	/// Display the entity name.
	virtual void displayName();
	/// Display the Hp Modifiers
	virtual void displayModifiers();
	/// Draw Path
	virtual void drawPath();
	/// Draw the selection Box
	virtual void drawBox();

	/// Create in-scene interface
	virtual void buildInSceneInterface ();

	/// Destroy inscene interfaces
	void releaseInSceneInterfaces();

	/** \name HEAD_CODE_CONTROL
	 * Methods to manage the head code control.
	 */
	//@{
	/** Update the head Direction.
	 */
	virtual void updateHeadDirection(CEntityCL *target);
	/// Return the bone Id of the entity Head.
	sint headBone() const {return _HeadBoneId;}
	/// Return the bone Id of the entity pelvis.
	//sint pelvisBone() const {return _PelvisBoneId;}
	/**
	 * Method to get the position of the head (in the world).
	 * \param headPos: will be set with the head position if succeed.
	 * \return 'true' if the param has been updated.
	 * \warning this method do NOT check if there is a skeleton.
	 */
	virtual bool getHeadPos(NLMISC::CVector &headPos);
	//@}

	/// Return the selection box.
	virtual const NLMISC::CAABBox &selectBox();


	/// Set the mode for the entity.
	virtual bool mode(MBEHAV::EMode m);
	void setMode(MBEHAV::EMode m) {_Mode = m; _ModeWanted = m;}


	/** To Inform about another (or self) entity removal (to remove from selection for example).
	 * This will remove the entity from the target.
	 * \param slot : Slot of the entity that will be removed.
	 */
	virtual void slotRemoved(const CLFECOMMON::TCLEntityId &slot);

	enum TOnMove
	{
		OnMoveNone = 0,
		OnMoveForward,
		OnMoveBackward,
		OnMoveLeft,
		OnMoveRight
	};
	enum TOnRot
	{
		OnRotNone = 0,
		OnRotLeft,
		OnRotRight
	};
	enum TOnBigBend
	{
		OnBendNone = 0,
		OnBendLeft,
		OnBendRight
	};
	//
	TOnMove    onMove      (const CAutomatonStateSheet &curAnimState);
	//
	TOnRot     onRotation  (const CAutomatonStateSheet &curAnimState, NLMISC::CVector &dirEndA);
	//
	TOnBigBend onBigBend   (const CAutomatonStateSheet &curAnimState, NLMISC::CVector &dirEndA);
	//
	bool       onBadHeading(const CAutomatonStateSheet &curAnimState);

	/** Force an animation.
	  */
	void setAnim(TAnimStateKey newKey, TAnimStateKey subKey = (TAnimStateKey)CAnimation::UnknownAnim, uint animID=NL3D::UAnimationSet::NotFound);

	/// Set the LOD animation.
	void setAnimLOD(bool changed);

	GSGENDER::EGender getGender() const { return _Gender; } // 0 -> male     1 -> female     2 -> beast
	void setGender(GSGENDER::EGender gender) {_Gender = gender;}

	/// Method to return the attack radius of an entity
	virtual double attackRadius() const;
	/** Return the position the attacker should have to combat according to the attack angle.
	 * \param ang : 0 = the front, >0 and <Pi = left side, <0 and >-Pi = right side.
	 */
	virtual NLMISC::CVectorD getAttackerPos(double ang, double dist) const;
	/// Return true if the opponent is well placed.
	virtual bool isPlacedToFight(const NLMISC::CVectorD &posAtk, const NLMISC::CVector &dirAtk, double attackerRadius) const;

	/** Play an impact on the entity
	 * \param impactType : 0=magic, 1=melee
	 * \param type : see behaviour for spell
	 * \param intensity : see behaviour for spell
	 * \param id : see behaviour for spell
	*/
	virtual void impact(uint impactType, uint type, uint id, uint intensity);
	// from CEntityCL
	virtual void meleeImpact(const CAttackInfo &damage);
	// from CEntityCL
	virtual void magicImpact(uint type, uint intensity);


	/** \name DEBUG
	 * Methods only here for the debug.
	 */
	//@{
	/// Return number of stage remaining.
	virtual uint nbStage();

	/// Return the number of attached FXs remaining to remove.
	virtual uint nbAttachedFXToRemove() {return (uint)_AttachedFXListToRemove.size();}

	/// Set the animation state key.
	bool animationStateKey(TAnimationType channel, TAnimStateId value);

	/// Return the current animation name.
	const std::string &currentAnimationName() const;
	/// Return the current animation set name.
	std::string currentAnimationSetName(TAnimationType animType) const;
	/// Change the entity colors.
	virtual void changeColors(sint userColor, sint hair, sint eyes, sint part);
	/// Display Debug Information.
	virtual void displayDebug(float x, float &y, float lineStep);
	/// Display Debug Information for property stages
	virtual void displayDebugPropertyStages(float x, float &y, float lineStep);
	/// set the NameId for debug in local
	void	debugSetNameId(uint32 nameId) {updateVisualPropertyName(0, nameId);}
	//@}

	/**
	 * \return the sheet of the character
	 */
	const CCharacterSheet *getSheet() const {return _Sheet;}

	// Return true if this entity is a Kami
	virtual bool isKami() const;

	// Return true if this entity has Race set to Unknown
	virtual bool isUnknownRace() const;

	/// TEST
	std::string                                &currentAutomaton()     {return _CurrentAutomaton;}
	std::vector<const CAnimationSet *>         &currentAnimSet()       {return _CurrentAnimSet;}

	void processFrame(const NLMISC::TTime &currentTimeInMs);
	void processStage(CStage &stage);

	CLFECOMMON::TCLEntityId mount() const {return _Mount;}
	CLFECOMMON::TCLEntityId rider() const {return _Rider;}
	void rider(CLFECOMMON::TCLEntityId r) {_Rider = r;}


	/**
	 * \param pos : result given in this variable. Only valid if return 'true'.
	 * \return bool : 'true' if the 'pos' has been filled.
	 */
	virtual bool getChestPos(NLMISC::CVectorD &pos) const;

	/**
	 * \param pos : result given in this variable. Only valid if return 'true'.
	 * \return bool : 'true' if the 'pos' has been filled.
	 */
	virtual bool getNamePos(NLMISC::CVectorD &pos);

	/// Return true if the character is swimming (even if dead).
	bool isSwimming() const {return (_Mode == MBEHAV::SWIM || _Mode == MBEHAV::SWIM_DEATH || _Mode == MBEHAV::MOUNT_SWIM);}
	/// Return true if the character is riding.
	bool isRiding() const {return (_Mode == MBEHAV::MOUNT_NORMAL || _Mode == MBEHAV::MOUNT_SWIM);}
	/// Is the entity in combat.
	bool isFighting() const {return (_Mode == MBEHAV::COMBAT || _Mode == MBEHAV::COMBAT_FLOAT);}
	/// Return true if the character is currently dead.
	virtual bool isDead() const {return (_Mode == MBEHAV::DEATH || _Mode == MBEHAV::SWIM_DEATH);}
	/// Return true if the character is really dead. With no lag because of anim or LCT
	virtual bool isReallyDead() const {return (_TheoreticalMode == MBEHAV::DEATH || _TheoreticalMode == MBEHAV::SWIM_DEATH);}

	const CAutomatonStateSheet *currentState() {return _CurrentState;}
	void currentState(const CAutomatonStateSheet *newState) {_CurrentState = newState;}
	/// Return the entity sheet scale. (return 1.0 if there is any problem).
	virtual float getSheetScale() const;
	// Return the entity collision radius. (return 0.5 if there is any problem).
	float getSheetColRadius() const;
	/// Return the entity scale. (return 1.0 if there is any problem).
	virtual float getScale() const;
	/// Return 'true' is the entity is displayed.
	virtual bool isVisible() const {return true;}
	/// (Re-)Build the playlist (Removing the old one too).
	virtual void buildPlaylist();
	// return vector of ground fxs sorted by ground type, or NULL is ground fxs are not supported for the entity
	virtual const std::vector<CGroundFXSheet> *getGroundFX() const;
	virtual bool supportGroundFX() const { return true; }
	//--------------//
	// ENTITY INFOS //
	//--------------//
	/// Return the entity speed
	virtual double getSpeed() const;

	virtual uint32 getGuildNameID () const { return _GuildNameId; }
	virtual uint64 getGuildSymbol () const { return _GuildSymbol; }

	virtual uint32 getEventFactionID() const { return _EventFactionId; }
	virtual uint16 getPvpMode() const { return _PvpMode; }
	void setPvpMode(uint16 mode) { _PvpMode=mode; }
	virtual uint32 getLeagueID() const { return _LeagueId; }
	void setLeagueID(uint32 league) { _LeagueId=league; }

	virtual uint16 getOutpostId() const { return _OutpostId; }
	virtual OUTPOSTENUMS::TPVPSide getOutpostSide() const { return _OutpostSide; }

	// get scale pos of character
	float getScalePos() const { return _CharacterScalePos; }

	// get scale from reference scale (male fyros). For creatures, returns the same than _CharacterScalePos
	virtual float getScaleRef() const { return _CharacterScalePos; }

	// Return true if this entity is a neutral entity.
	virtual bool isNeutral () const;
	// Return true if this entity is a user's friend.
	virtual bool isFriend () const;
	// Return true if this entity is a user's enemy.
	virtual bool isEnemy () const;
	// Return true if this entity is a user's ally.
	virtual bool isAlly () const;

	/// Return the People for the entity
	virtual EGSPD::CPeople::TPeople people() const;
	virtual void setPeople(EGSPD::CPeople::TPeople people);

	/// *** Ingame interface
	void setBubble (CGroupInSceneBubble *bubble);
	//  *** Ingame interface : get the current bubble assigned
	CGroupInSceneBubble *getBubble () { return _CurrentBubble; }

	// Attach a CAttachedFX  object to that character.
	void	 attachFX(const CAttachedFX::TSmartPtr fx);

	/** Apply the behaviour.
	 * \param behaviour : the behaviour to apply.
	 */
	virtual void applyBehaviour(const CBehaviourContext &behaviour);

	// align an fx to be positionned and oriented like that entity, and put in the same cluster system than this entity
	void alignFX(NL3D::UParticleSystemInstance instance, float scale = 1.f, const NLMISC::CVector &localOffset = NLMISC::CVector::Null) const;

		// align a fx using the given matrix (also add scale & offset) , and put in the same cluster system than this entity
	void alignFX(NL3D::UParticleSystemInstance instance, const NLMISC::CMatrix &matrix, float scale = 1.f, const NLMISC::CVector &localOffset = NLMISC::CVector::Null) const;

	// build a matrix aligned on that entity (includes dir & pos)
	void buildAlignMatrix(NLMISC::CMatrix &dest) const;

	/** Set a new aura fx
	  * If pointer is NULL, then previous aura is shutdown over several frames
	  * If pointer isn't NULL then a new aura is created. Any previously present aura is immediatly deleted.
	  */
	void setAuraFX(uint index, const CAnimationFX *fx);
	/** Set a new link fx.
	  * If there's a previous link, the dispell fx is played to make it disappear
	  * If fx ptr is NULL then no new link fx is created (but dispell is still played)
	  */
	void setLinkFX(const CAnimationFX *fx, const CAnimationFX *dispell);

	// From CEntityCL
	const char *getBoneNameFromBodyPart(BODY::TBodyPart part, BODY::TSide side) const;
	// ...
	virtual bool getBoneHeight(BODY::TBodyPart localisation, BODY::TSide side, float &height) const;
	//
	/// Get the Animation 'State' for an animation channel.
	TAnimStateId				animState (TAnimationType channel) const;

	// retrieve right hand item sheet
	virtual const CItemSheet *getRightHandItemSheet() const;
	virtual const CItemSheet *getLeftHandItemSheet() const;

	// get attack from an attack id (NULL if not found)
	virtual const CAttack *getAttack(const CAttackIDSheet &id) const;

	/// Return the Entity Current Speed.
	virtual double speed() const;

	/// Ask if the entity is sit
	bool isSit() const {return _Mode == MBEHAV::SIT;}

	/// Ask if the entity is afk (a character is never afk but players can be)
	virtual bool isAFK() const {return false;}

	// reset all sound anim id this entity may own
	virtual void resetAllSoundAnimId();

	// manually set/get the job animation state
	void	setAnimJobSpecialisation(uint32 state) {_AnimJobSpecialisation= state;}
	uint32	getAnimJobSpecialisation() const {return _AnimJobSpecialisation;}

	virtual void removeAllAttachedFX();


	void playCastFX(const CAnimationFXSet *afs, uint power);

	// Can the user select this entity by pressing space key ?
	bool	isSelectableBySpace() { return _SelectableBySpace; }

protected:
	/** A worn item and its attached fxs.
	  * The item instance itself is managed by _Instances
	  */
	class CWornItem
	{
	public:
		const CItemSheet			  *Sheet;        // sheet of item, or NULL if not used
		NL3D::UInstance				  Trail;        // trail linked to that fx
		NL3D::UParticleSystemInstance AdvantageFX;	 // adantage fx linked to that item
	public:
		CWornItem()
		{
			Sheet = NULL;
		}
		// release (but not remove the mesh, it is managed by _Instances)
		void release()
		{
			Sheet = NULL;
			releaseFXs();
		}
		// init item fxs
		void initFXs(SLOTTYPE::EVisualSlot visualSlot, NL3D::UInstance parent);
		// release item fxs
		void releaseFXs();
		/** trigger fxs associated with an attack
		  * \param intensity in [1, 5]
		  */
		void startAttackFX(NL3D::USkeleton skeleton, uint intensity, SLOTTYPE::EVisualSlot visualSlot, bool activateTrail);
		// stop attack fx
		void stopAttackFX();
		// start/stop the advantage fx (advantage fx appears on a weapon that is mastered by user)
		void enableAdvantageFX(NL3D::UInstance parent);
		/** set size of trail (value clamped to [0, 15]). This will map to the values given in the item sheet.
		  * 0 mean that there's no trail
		  */
		void setTrailSize(uint size);
	};

	// the various category of animated fx list
	enum TAttachedFXList
	{
		FXListToRemove,
		FXListCurrentAnim,
		FXListAuto   // the list is determined by the sheet of the fx
	};

	static const uint8							_BadHairIndex;

	/// Stages to describ entity actions/motion.
//	CStageSet									_Stages;


	/// Sound ids of the current animations (-1 if none set)
	std::vector<NLSOUND::TSoundAnimId>			_SoundId;
	/// The sound context for animation for this character
	NLSOUND::CSoundContext						_SoundContext;

	/// Subsidiary Key for the Animation State (emote).
	TAnimStateId								_SubStateKey;
	/// Pointer on the current Anim Set.
	std::vector<const CAnimationSet *>			_CurrentAnimSet;

	double										_LastFrameTime;

	/// Lod Animation ??. false by default
	bool										_LodCharacterAnimEnabled;
	/// Time for lod animation
	double										_LodCharacterAnimTimeOffset;
	/// Which playList AnimSlot drive the LodCharacterAnimation. MOVE, by default.
	uint										_LodCharacterMasterAnimSlot;

	// value to scale the "pos" channel of the animation of the character.
	float										_CharacterScalePos;

	// First Position.
	NLMISC::CVectorD							_FirstPos;
	// Time of the first Position.
	double										_FirstTime;
	// Distance to the first stage.
	double										_DistToFirst;
	// Time at which the first run animation should start in order to avoid pop (computed in updateStages())
	// It is used to accelerate animations that are followed by positions
	double										_RunStartTimeNoPop;


	// Position of the destination.
	NLMISC::CVectorD							_DestPos;
	// Time of the destiantion.
	double										_DestTime;
	// Distance to the destination or 0 if destination reached (in meter).
	double										_DistToDest;

	/// Position that was in the last stage with a position reached.
	NLMISC::CVectorD							_OldPos;
	/// Time of the last stage pos reached.
	double										_OldPosTime;


	/// Is it a male ? female ? could be a beast :p
	GSGENDER::EGender							_Gender;

	/// Id of the Bone used to display the name.
	sint										_NameBoneId;
	/// Used to force the bone to be computed (but in CLod form)
	NL3D::UTransform							_NameTransform;
	/// If in Clod form, the name pos is computed with DeltaPos
	mutable float								_NameCLodDeltaZ;
	enum {NameCLodDeltaZNotComputed= -1000000};

	/// Id of the Bone used for the Middle of torso.
	sint										_ChestBoneId;

	/// Pointers on items for visible slots.
	std::vector<CWornItem>						_Items;
	/// Instances Index for visible slots.
	uint32										_HeadIdx;
	uint32										_FaceIdx;
	/// Should objects in hands be displayed ?
	bool										_ObjectsVisible;

////// HEAD
	/// Id of the Bone for the Head. -1 if no bone.
	sint										_HeadBoneId;
	// Id of the Pelvis Bone. -1 if no bone
	//sint										_PelvisBoneId;
	/// Offset between the entity position and its head.
	NLMISC::CVector								_HeadOffset;
	/// true if the head offset has been computed at least once
	bool										_HeadOffsetComputed;


////// ANIMATION
	/// Used to adjust the rotation of the animation to finish with the perfect orientation.
	double										_RotationFactor;
	/// This is the direction the entity will have at the end of the animation.
	NLMISC::CVector								_DirEndAnim;
	/// ...
	double										_RotAngle;
	/// Automaton currently in use.
	std::string									_CurrentAutomaton;
	/// Pointer on the current state.
	const CAutomatonStateSheet					*_CurrentState;
	/// The mode the entity should already be but still not the current one.
	MBEHAV::EMode								_ModeWanted;
	/// Number of frame remaining for the blend.
	sint										_BlendRemaining;
	/// Automaton currently in use.
	std::string									_OldAutomaton;
	/// Rotation at the end of the last animation for the blend.
	NLMISC::CQuat								_OldRotQuat;
	/// Scale for the skeleton according to the gabarit.
	float										_CustomScalePos;
	/// Next blink time
	NLMISC::TTime								_NextBlinkTime;
	/// Number of loop of the curren animation.
	uint32										_NbLoopAnim;
	bool										_MaxLoop;
	///
	NL3D::CTargetAnimCtrl						_TargetAnimCtrl;
	/// The job specialisation of the character (0 by default). visual prop from server
	uint32										_AnimJobSpecialisation;

	/** List of animation FXs currently being played. Those FXs are shutdown as soon as an animation switch occurs.
	  * When theyareshutdown,theit emitter are stopped, and they are put in _RemoveAnimFXList,waiting for deletion
      */
	std::list<CAttachedFX::TSmartPtr>			_AttachedFXListForCurrentAnim;
	/// List of attached to remove as soon as possible (when there are no particles left)
	std::list<CAttachedFX::TSmartPtr>			_AttachedFXListToRemove;

	std::list<CAttachedFX::CBuildInfo>			_AttachedFXListToStart;

	CAttachedFX::TSmartPtr						_AuraFX[MaxNumAura]; // special case for aura
	CAttachedFX::TSmartPtr						_LinkFX;             // special case for link

	class CStaticFX : public NLMISC::CRefCount
	{
	public:
		CAnimationFX            AF; // ideally this field should be per sheet, not per instance, but simpler this way
		CAttachedFX::TSmartPtr  FX;
	};

	NLMISC::CSmartPtr<CStaticFX>				_StaticFX;			  // special case for static fx


	/// FX from the right hand activated.
	bool										_RightFXActivated;
	/// FX from the left hand activated.
	bool										_LeftFXActivated;

	CLFECOMMON::TCLEntityId						_Mount;
	CLFECOMMON::TCLEntityId						_TheoreticalMount;
	CLFECOMMON::TCLEntityId						_Rider;
	CLFECOMMON::TCLEntityId						_TheoreticalRider;
	MOUNT_PEOPLE::TMountPeople					_OwnerPeople;

	bool										_IsThereAMode;

	// if 0(default), LCT is fully applied, else this is the ServerTick when the LCT impact decrease
	NLMISC::TGameCycle							_StartDecreaseLCTImpact;

	// this time is the estimated time of the first important stage (excluding the first one)
	double										_ImportantStepTime;

	// The Colour of the entity hair.
	sint8										_HairColor;
	// The Colour of the entity Eyes.
	sint8										_EyesColor;
	//
	uint8										_HairIndex;
	//
	bool										_LookRdy;

	double										_Speed;
	double										_RunFactor;

	/// *** Ingame interface
	class CGroupInSceneUserInfo					*_InSceneUserInterface;
	class CGroupInSceneBubble					*_CurrentBubble;
	uint32										_GuildNameId;
	uint64										_GuildSymbol;
	uint32										_EventFactionId;
	uint16										_PvpMode;
	PVP_CLAN::TPVPClan							_PvpClan;
	uint32										_LeagueId;
	uint16										_OutpostId;
	OUTPOSTENUMS::TPVPSide						_OutpostSide;

	// Can the user select this entity by pressing space key ?
	bool										_SelectableBySpace;

	// current attack (attacks include magic/melee/range/creature_attack)
	const	CAttack								*_CurrentAttack;
	CAttackIDSheet								 _CurrentAttackID;
	CAttackInfo									 _CurrentAttackInfo;

	bool										_HideSkin;

	// for selectBox() computing
	sint64										_LastSelectBoxComputeTime;

	float										_CustomScale;

	/// Pointer on the Sheet with basic parameters.
	const CCharacterSheet						*_Sheet;

protected:
	// Adjust the Predicted Interval to fix some errors according to the distance.
	NLMISC::TGameCycle adjustPI(float x, float y, float z, const NLMISC::TGameCycle &pI);

	// Look at items in hands to change the animation set.
	bool modeWithHiddenItems() const;

	/** Returns true if current behaviour triggers an attack end.
      * (when projectiles & impact fx are created)
	  */
	bool isCurrentBehaviourAttackEnd() const;

	/** Build attack infos from current behaviour
	  * This convert any attack encoded in current behaviour (magic/melee/range/creature_attack)
	  * into a common representation (CAttackInfo)
	  */
	void buildAttackInfo(CAttackInfo &dest);

	/** perform current attack
	  * list of targets if given by the behaviour context
	  * (where projectiles & impact fx are reated)
	  */
	void performCurrentAttackEnd(const CBehaviourContext &bc, bool directOffensifSpell, std::vector<double> &targetHitDates, TAnimStateKey animForCombat);

	// update current attack from the current behaviour (this update the "_Attack" field)
	void updateCurrentAttack();


	/** Create part of current attack (a single projectile or/and impact, total attack may contain severals)
	  * IMPORTANT: the instance on which this method is invoked is NOT always the caster (secondary missiles can be generated after main target has been reached)
	  * NB : target stick mode isn't filled, for this, should use computeTargetStickMode instead
	  * NB : once fully filled, the CProjectileBuild object should be inserted in the projectile manager
	  */
	bool createCurrentAttackEndPart(CProjectileBuild &destPB,
		                            const CAttack *currentAttack,
		                            const CCharacterCL &target,
								    const CFXStickMode *sm,
									double spawnDate,
									double hitDate,
									bool playImpactFX,
									bool playImpactAnim,
									bool magicResist,
									bool mainImpactIsLocalised,
									const CAttackInfo &attackInfo,
									const NLMISC::CVector &additionnalOffset = NLMISC::CVector::Null
								   );


	// Return the automaton type of the entity (homin, creature, etc.)
	virtual std::string automatonType() const;
	// Compute the current automaton for the entity.
	void computeAutomaton();

	// Return the atk height.
	TAtkHeight getAttackHeight(CEntityCL *target, BODY::TBodyPart localisation, BODY::TSide side) const;

////// FX


	/** Stop all attached FXs linked to current anim (this wait until FX have no more particle, after what they are effectively deleted)
	  * \param stopLoopingFX If set to true, looping fx are also stopped. A looping fx has RepeatMode == Loop in its sheet.
	  */
	void stopAttachedFXForCurrrentAnim(bool stopLoopingFX);

	/** start items attack fxs
	* \param intensity tranges from 1 to 5
	*/
	void startItemAttackFXs(bool activateTrails, uint intensity);

	// stop items attack fxs
	void stopItemAttackFXs();

	/** Compute some bone id (name, chest, head...)
	 * \warning This method do not check the bone is valid, nor there is a Scene.
	 */
	void computeSomeBoneId();


	/// Create the play list for this entity.
	void createPlayList();

	/** Compute each part of the visual equipement of the character
	 * \param slot: structure of the equipement.
	 * \param visualSlot: visual slot used by this item.
	 * \param instIdx : if not CEntityCL::BadIndex, replace the equipement instance at this index.
	 * \return uint32 : index of the instance or CEntityCL::BadIndex if there were a problem.
	 * \todo GUIGUI : find a better choice to avoid all these slotType tests
	 */
	uint32 buildEquipment(const CCharacterSheet::CEquipment &slot, SLOTTYPE::EVisualSlot visualSlot, sint color = -1, uint32 instIdx = CEntityCL::BadIndex);

	/// Compute the animation set to use according to weapons, mode and race.
	virtual void computeAnimSet();

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
	/// Update the NPC Alternative Look Property.
	virtual void updateVisualPropertyVpa           (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	virtual void updateVisualPropertyVpb           (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Mount
	virtual void updateVisualPropertyEntityMounted (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Rider
	virtual void updateVisualPropertyRiderEntity   (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update Entity Bars
	virtual void updateVisualPropertyBars          (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	virtual void updateVisualPropertyGuildSymbol	(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	virtual void updateVisualPropertyGuildNameID	(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	virtual void updateVisualPropertyEventFactionID	(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	virtual void updateVisualPropertyPvpMode		(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	virtual void updateVisualPropertyPvpClan		(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	virtual void updateVisualPropertyOwnerPeople	(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	virtual void updateVisualPropertyOutpostInfos	(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);

	/// Update Entity Status
	virtual void updateVisualPropertyStatus        (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	// Update target list
	virtual void updateVisualPropertyTargetList	   (const NLMISC::TGameCycle &gameCycle, const sint64 &prop, uint listIndex);
	// Update tvisual fw
	virtual void updateVisualPropertyVisualFX      (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update vprop contextual attributes
	virtual void updateVisualPropertyContextual	   (const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	// Get The Entity Skin
	virtual sint skin() const;

	/// Get the direction that should have the character at the end of the animation.
	const NL3D::CVector &dirEndAnim() {return _DirEndAnim;}
	/// Set the direction that should have the character at the end of the animation.
	void dirEndAnim(const NL3D::CVector &vect);

	/**
	 * Add an instance to the list of instance composing the entity.
	 * \param shapeName : shape filename.
	 * \param stickPoint : Name of the bone to stick on.
	 * \param texture : texture to use (in multi texture) or -1 for default texture.
	 * \param instIdx : if not CEntityCL::BadIndex, the instance will replace the one at this index.
	 * \return uint32 : index of the instance created, or CEntityCL::BadIndex.
	 */
	uint32 addColoredInstance(const std::string &shapeName, const std::string &stickPoint = "", sint texture = -1, uint32 instIdx = BadIndex, sint color = -1);

	/// Initialize properties of the entity (according to the class).
	virtual void initProperties();

	/// Method to Flag the character as dead and do everything needed.
	virtual void setDead();
	/// Method to Flag the character as alive and do everything needed.
	virtual void setAlive();

	///
	double computeTimeStep(const double &currentTime);

	///
	double computeSpeed();
	/**
	 * Compute and return the speed factor to apply to the animation.
	 * \param speedToDest : evaluted speed to destination.
	 * \return double : the speed factor to use for the current animation.
	 */
	double computeSpeedFactor(double speedToDest);

	///
	void updateAnimationState();
	///
	double computeMotion(const double &oldMovingTimeOffset, TAnimationType channel = MOVE) const;

	/// Apply stage modifications.
	bool applyStage(CStage &stage);
	/// Apply The Current Stage (first stage).
	bool applyCurrentStage();
	/// Apply all stages to the first stage with a position.
	void applyAllStagesToFirstPos();


	virtual void beginCast(const MBEHAV::CBehaviour &behaviour);
	void endCast(const MBEHAV::CBehaviour &behaviour, const MBEHAV::CBehaviour &lastBehaviour);


	/// Play the time step for the loop and truncate to End Anim if Time Step too big.
	void playToEndAnim(const double &startTimeOffset, double &length);

	/// Call this method to give a time for each stage, compute distance to destination and some more information.
	void updateStages();

	/**
	 * Manage Events that could be created by the animation (like sound).
	 * \param startTime : time to start processing events from the current animation.
	 * \param stopTime : time to stop processing events from the current animation.
	 */
	void animEventsProcessing(double startTime, double stopTime);

	void applyColorSlot(SInstanceCL &instance, sint skin, sint userColor, sint hair, sint eyes);

	/// Return the current max speed for the entity in meter per sec (!= swim, != mount, etc.)
	virtual double getMaxSpeed() const;
	// Draw the name.
	virtual void drawName(const NLMISC::CMatrix &mat);

	/// Set the Distance from the current entity position to the First Position.
	void   dist2FirstPos(double d2FP);
	/// Return the Distance from the current entity position to the First Position.
	double dist2FirstPos() const;
	/// Set the Distance from the current entity position to the Destination.
	void   dist2Dest(double d2D);
	/// Return the Distance from the current entity position to the Destination.
	double dist2Dest() const;
	/// Set the Entity Current Speed.
	void   speed(double s);
	/// Set the Factor between Walk & Run
	void   runFactor(double factor);
	/// Get the Factor between Walk & Run
	double runFactor() const;
	/// Set the Animation 'Offset' for an animation channel.
	void						animOffset(TAnimationType channel, double offset);
	/// Get the Animation 'Offset' for an animation channel.
	double						animOffset(TAnimationType channel) const;
	/// Set the Animation 'State' for an animation channel.
	void						animState (TAnimationType channel, TAnimStateId state);
	/// Set the Animation 'Index' in the 'State' for an animation channel.
	void						animIndex (TAnimationType channel, CAnimation::TAnimId index);
	/// Get the Animation 'Index' in the 'State' for an animation channel.
	CAnimation::TAnimId			animIndex (TAnimationType channel) const;
	/// Set the Animation 'Id' among all the animations for an animation channel.
	void						animId    (TAnimationType channel, uint id);
	/// Get the Animation 'Id' among all the animations for an animation channel.
	uint						animId    (TAnimationType channel) const;


	// Read/Write Variables from/to the stream.
	virtual void readWrite(class NLMISC::IStream &f) throw(NLMISC::EStream);
	// To call after a read from a stream to re-initialize the entity.
	virtual void load();

	void	showOrHideBodyParts( bool objectsVisible );
	bool	objectsVisible() const { return _ObjectsVisible; };

	/// Return name position on Z axis defined in sheet
	virtual float getNamePosZ() const;

private:
	static const std::string					_EmptyString;
	const CCharacterSheet						*_ClothesSheet;
	uint32										_RHandInstIdx;
	uint32										_LHandInstIdx;
	// Return 'true' if the animation should be played from the end to the start.
	std::vector<bool>							_AnimReversed;
	// Animation 'Offset' for each channel.
	std::vector<double>							_AnimOffset;
	// Animation 'State' for each channel.
	std::vector<TAnimStateId>					_AnimState;
	// Animation 'Index' in the 'State' for each channel (-1 if none set).
	std::vector<CAnimation::TAnimId>			_AnimIndex;
	// Animation 'Id' among all the animations for each channel.
	std::vector<uint>							_AnimId;

	struct CNamePosHistory
	{
		// ctor
		CNamePosHistory() : LastNamePosZ(0.f), LastBonePosZ(0.f) {}

		bool isInitialized() const { return LastNamePosZ != 0.f; }

		float LastNamePosZ;
		float LastBonePosZ;
	};

	CNamePosHistory								_NamePosHistory;

private:
	/// Call it at the end of the current animation to choose the next one.
	void endAnimTransition();

	/** Return the shape pointer from tha item and according to the character gender.
	 * \param itemSheet : reference on the item sheet.
	 * \return string & : reference on the shape name.
	 */
	std::string shapeFromItem(const CItemSheet &itemSheet) const;

	/// Create the instance from an item
	uint32 createItemInstance(const CItemSheet &itemSheet, uint32 instIdx, SLOTTYPE::EVisualSlot visualSlot, const std::string &bindBone, sint8 texture, sint color);

	/** Return if the impact must be played.
	 * \param anim : pointer on the current animation (MUST NOT BE NULL).
	 * \param currentTime : current time in the animation.
	 * \param triggerName : name of the trigger to check.
	 * \param isActive : read (and can change) the state.
	 * \param timeFactor : when to activate the impact if there is no track (value has to be between 0 and 1 to be valid).
	 * \return bool : true if the trigger is valid.
	 * \warning This method does not check if the animation is Null.
	 */
	bool beginImpact(NL3D::UAnimation *anim, NL3D::TAnimationTime currentTime, const std::string &triggerName, bool &isActive, float timeFactor);
	/** FX played on the weapon when there is an impact.
	 * \param index : index of the fx.
	 * \param attackPower : power of the current attack.
	 */
	void weaponImpactFX(sint index, uint attackPower);

	// update attached fxs
	void updateAttachedFX();


	// Attach a CAttachedFX  object to that character.
	void	 attachFXInternal(const CAttachedFX::TSmartPtr fx, TAttachedFXList wantedList);

	//  attach an externally created fx in the given list (alive fx or shutting down fxs)
	void attachFX(NL3D::UParticleSystemInstance instance,
		          const CAnimationFX *sheet,
				  const CFXStickMode *stickMode,
				  TAttachedFXList targetList,
				  float timeOut = FX_MANAGER_DEFAULT_TIMEOUT,
				  const NLMISC::CVector &stickOffset = NLMISC::CVector::Null,
				  uint maxNumAnimCount = 0,
				  uint8 targeterSlot = CLFECOMMON::INVALID_SLOT,
				  const NLMISC::CMatrix *staticMatrix = NULL
				 );


	// apply visual fx from packed property (include auras and links)
	virtual void applyVisualFX(sint64 prop);
	/// Helper fct : remove any obsolete reference in a CAnimFX list to an entity  slot that has just been removed.
	static void updateAnimFXListForSlotRemoved(std::list<CAttachedFX::TSmartPtr> &fxList, const CLFECOMMON::TCLEntityId &slotRemoved);

	// compute the best cast ray when creating a projectile (for guard towers)
	void computeBestCastRay(CEntityCL		   &targetEntity,
		                    const CFXStickMode &targetStickMode,
							NLMISC::CVector    &castWorldOrigin,
							NLMISC::CVector    &castWorldPos,
							NLMISC::CVector    &worldOffsetToCasterPivot) const;

	// build cast fxs for static object (such as a guard tower ...)
	void buildStaticObjectCastFX(const NLMISC::CVector &castWorldOrigin,
		                         NLMISC::CVector &castWorldPos,
								 const CAttackSheet &sheet,
								 uint intensity
								);

	// Compute the stick mode on the target for an attack
	void computeTargetStickMode(const CAttackSheet &sheet, const CAttackInfo &attackInfo, CFXStickMode &dest, CEntityCL &target);

	// remove attached fxs that depends on another slot when it has been removed
	void updateAttachedFXListForSlotRemoved(std::list<CAttachedFX::TSmartPtr> &fxList, const CLFECOMMON::TCLEntityId &slotRemoved);

	// apply the flying HP related to behaviour
	void applyBehaviourFlyingHPs(const CBehaviourContext &bc, const MBEHAV::CBehaviour &behaviour, const std::vector<double> &targetHitDates);

protected:
	// retrieve attack from an id and a list of attack lists filenames
	const CAttack *getAttack(const CAttackIDSheet &id, const std::vector<NLMISC::TSStringId> &attackList) const;
	void releaseStaticFX();
	void initStaticFX();
};


#endif // CL_CHARACTER_CL_H

/* End of character_cl.h */
