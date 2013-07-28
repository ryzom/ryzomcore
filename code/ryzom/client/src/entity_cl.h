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



#ifndef CL_ENTITY_CL_H
#define CL_ENTITY_CL_H


/////////////
// Include //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/time_nl.h"			// Define the TTime
#include "nel/misc/vector.h"			// Define the CVector
#include "nel/misc/vectord.h"			// Define the CVector
#include "nel/misc/matrix.h"
#include "nel/misc/sheet_id.h"
// Interface 3D
#include "nel/3d/u_instance.h"
#include "nel/3d/u_skeleton.h"
#include "nel/3d/u_visual_collision_entity.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/logic_info.h"
#include "nel/3d/u_particle_system_instance.h"
// Pacs Interface.
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_global_position.h"
// Game_Share
#include "game_share/properties.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/entity_types.h"
#include "game_share/body.h"
#include "game_share/hit_type.h"
#include "game_share/body.h"
#include "game_share/animal_status.h"
#include "game_share/pvp_mode.h"
#include "game_share/pvp_clan.h"
#include "game_share/mount_people.h"
#include "game_share/outpost.h"
// Sheets
#include "client_sheets/ground_fx_sheet.h"
// Client
#include "animation_type.h"
#include "string_manager_client.h"
// Stl
#include <string>
#include <list>
#include <map>

// The update clipped primitive mask
#define RZ_CLIPPED_UPDATE_TIME_MASK 0x7

// Size of the points bars
#define RZ_BARS_LENGTH 127

#define RZ_TIME_TO_BECOME_TRANSPARENT_IN_SECOND	0.25
//
#define TMP_DEBUG_GUIGUI

///////////
// CLASS //
///////////
namespace NL3D
{
	class USkeleton;
	class UPlayList;
	class UParticleSystemInstance;
	class UMaterial;
}

class CEntitySheet;
class CEntityCL;

class CItemSheet;

class CPhysicalDamage;

namespace NLMISC{
class CCDBNodeLeaf;
class CCDBNodeBranch;
}

extern CLFECOMMON::TCLEntityId	SlotUnderCursor;

/**
 * Implementation of NL3D::ILogicInfo
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CEntityLogicInfo3D : public NL3D::ILogicInfo
{
public:
	/// My owner
	CEntityCL		*Self;

public:
	/// retrieve light information for the entity skeleton
	virtual	void		getStaticLightSetup(NLMISC::CRGBA sunAmbient, std::vector<NL3D::CPointLightInfluence> &pointLightList,
		uint8 &sunContribution, NLMISC::CRGBA &localAmbient);
};



/**
 * Interface to manage an Entity in the client side.
 *
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CEntityCL : public NLMISC::IStreamable, public STRING_MANAGER::IStringWaitCallback
{
	friend class CUpdateEntitiesColor;

public:
	struct SInstanceCL
	{
		NL3D::UInstance Current;	// Current instance
		std::string CurrentName;	// Current instance name
		const CItemSheet *FXItemSheet;
		std::vector<NL3D::UInstance> StaticFXs;

		NL3D::UInstance Loading;		// Instance loading (to avoid any artefact)
		std::string LoadingName;		// Loading instance name

		std::string StickPoint;
		sint TextureSet;
		bool ApplyColor;
		bool KeepHiddenWhenLoaded;
		sint ACSkin, ACUser, ACHair, ACEyes;

		// ------------------------------------------------------------------------
		SInstanceCL()
		{
			Current = Loading = NULL;
			FXItemSheet = NULL;
			TextureSet = -1;
			ApplyColor = false;
			KeepHiddenWhenLoaded = false;
			ACSkin = ACUser = ACHair = ACEyes = -1;
			_Scale= NLMISC::CVector(1.f,1.f,1.f);
		}
		~SInstanceCL()
		{
			releaseStaticFXs();
		}


		void selectTextureSet(uint8 value, bool async = true);

		// show every static fxs
		void showStaticFXs();

		// hide every static fxs
		void hideStaticFXs();

		// Create the loading instance. return false if shapeName!="" while still fails to load. else return true.
		bool createLoading(const std::string &shapeName, const std::string &stickPoint=std::string(""), sint texture=-1, bool clearIfFail= true);

		// Apply Colors
		void setColors(sint skin, sint user, sint hair, sint eyes);

		// Create the loading instance from the current instance and copy info from current instance to it
		// If the loading instance already exist do not copy anything from current instance
		NL3D::UInstance createLoadingFromCurrent();

		// Update current instance from loading instance (set loading instance to null)
		// Bind the loading instance to the skeleton
		void updateCurrentFromLoading(NL3D::USkeleton skel);

		void releaseStaticFXs();

		// For Blink
		void	setEmissive(NLMISC::CRGBA emit);
		void	restoreEmissive();

		// For MouseOver player (do it on Current)
		void	makeInstanceTransparent(uint8 opacity, uint8 opacityMin);

		// replace diffuse for all material of this instance (not including alpha)
		void setDiffuse(bool onOff, NLMISC::CRGBA diffuse);

		// Setup the scale for the instance
		void					setScale(const NLMISC::CVector &scale);
		const NLMISC::CVector	&getScale() const {return _Scale;}

	private:
		// Wanted Scale
		NLMISC::CVector		_Scale;
	};

public:
	/// Constructor.
	CEntityCL();
	/// Destructor.
	virtual ~CEntityCL();

	/// Primitive type
	enum TType
	{
		User = 0,
		Player,
		NPC,
		Fauna,
		Entity,
		ForageSource,
		TypeCount
	} Type;

	/// Return the entity Id (persistent as long as the entity is connected) (CLFECOMMON::INVALID_CLIENT_DATASET_INDEX for an invalid one).
	const CLFECOMMON::TClientDataSetIndex &dataSetId() const {return _DataSetId;}
	/// Set the entity Id (persistent as long as the entity is connected) (CLFECOMMON::INVALID_CLIENT_DATASET_INDEX for an invalid one).
	void dataSetId(CLFECOMMON::TClientDataSetIndex dataSet);

	/// Return the sheet Id of the entity.
	const NLMISC::CSheetId &sheetId() const {return _SheetId;}
	/// Set the sheet Id of the entity.
	void sheetId(const NLMISC::CSheetId &id) {_SheetId = id;}

	/// Return the persistent NPC alias of entity (0 if N/A).
	uint32 npcAlias() const {return _NPCAlias; }
	/// Set the persistent NPC alias of the entity.	
	void npcAlias(uint32 alias) {_NPCAlias = alias; }

	/// Method to call to initialize all members of the right class.
	virtual void initialize();

	/// Build the entity from a sheet.
	virtual bool build(const CEntitySheet *sheet) = 0;

	/// Initialize properties of the entity (according to the class).
	virtual void initProperties();
	/** Initialize the primitive. Return 'true' if the primitive has been created.
	 *	clipRadius/clipHeight, specify them if you want special clip values (different from collision ones)
	 */
	bool initPrimitive(float radius, float height, float length, float width, NLPACS::UMovePrimitive::TReaction reactionType, NLPACS::UMovePrimitive::TTrigger triggerType, NLPACS::UMovePrimitive::TCollisionMask occlusionMask, NLPACS::UMovePrimitive::TCollisionMask collisionMask, float clipRadius=0.f, float clipHeight=0.f);

	/**
	 * Update a visual property from the database.
	 * \param gameCycle : when this was sent.
	 * \param prop : the property to udapte.
	 */
	void updateVisualProperty(const NLMISC::TGameCycle &gameCycle, const uint &prop, const NLMISC::TGameCycle &predictedInterval = 0);

	/// Display the entity name.
	virtual void displayName() {}
	/// Display the Hp Modifiers
	virtual void displayModifiers() {}
	/// Draw Path
	virtual void drawPath() {}
	/// Draw the selection Box
	virtual void drawBox() {}

	/** Method called each frame to manage the entity before the collisions detection.
	 * \param time : current time of the frame.
	 * \parem target : pointer on the current entity target.
	 */
	virtual void updatePreCollision(const NLMISC::TTime &/* time */, CEntityCL * /* target */) {}
	/** Method called each frame to manage the entity after the collisions detection.
	 * \param time : current time of the frame.
	 * \parem target : pointer on the current entity target.
	 */
	virtual void updatePostCollision(const NLMISC::TTime &/* time */, CEntityCL * /* target */) {}
	/** Method called each frame to manage the entity after the clipping test if the primitive is visible.
	 * \param time : current time of the frame.
	 * \parem target : pointer on the current entity target.
	 */
	virtual void updateVisible(const NLMISC::TTime &time, CEntityCL *target);
	/** Method called to manage the entity after the clipping test if the primitive is clipped.
	 * This method is called regulary but not at each frame.
	 * \param time : current time of the frame.
	 * \parem target : pointer on the current entity target.
	 */
	virtual void updateSomeClipped(const NLMISC::TTime &time, CEntityCL *target);
	/** Method called to manage the entity after the clipping test if the primitive is clipped.
	 * This method is called at each frame.
	 * \param time : current time of the frame.
	 * \parem target : pointer on the current entity target.
	 */
	virtual void updateClipped(const NLMISC::TTime &time, CEntityCL *target);
	/**
	 * Update the position of the entity after the motion.
	 * \param time : Time for the position of the entity after the motion.
	 * \param target : pointer on the current target.
	 */
	virtual void updatePos(const NLMISC::TTime &/* time */, CEntityCL * /* target */) {}
	/** Update the entity after the render like for the head offset.
	 */
	virtual void updateVisiblePostPos(const NLMISC::TTime &time, CEntityCL *target);
	/** Update the entity after the render like for the head offset.
	 */
	virtual void updateVisiblePostRender() {}
	/** Update all the entity after the render visible or not
	 */
	virtual void updateAllPostRender() {}

	/**
	 * Add an instance to the list of instance composing the entity.
	 * \param shapeName : shape filename.
	 * \param stickPoint : Name of the bone to stick on.
	 * \param texture : texture to use (in multi texture) or -1 for default texture.
	 * \param instIdx : if not CEntityCL::BadIndex, the instance will replace the one at this index.
	 * \return uint32 : index of the instance created, or CEntityCL::BadIndex.
	 */
	uint32 addInstance(const std::string &shapeName, const std::string &stickPoint = "", sint texture = -1, uint32 instIdx = BadIndex);

	// Return true if the primitive is clipped by the camera
	bool clipped (const std::vector<NLMISC::CPlane> &clippingPlanes, const NLMISC::CVector &camPos);

	/** \name NAME
	 * Functions to manipulate the Name.
	 */
	//@{
	/// Return the Name of the entity. There may be a specification in it (guard, trader, etc ...). It is then surrounded by '$'
	const ucstring &getEntityName() const {return _EntityName;}
	/// Return the title from a name. The specification is surrounded by '$', and tells the title of the entity (guard, matis merchant, etc ..)
	static ucstring getTitleFromName(const ucstring &name);
	/// Remove the specification from a name. The specification is surrounded by '$', and tells the title of the entity (guard, matis merchant, etc ..)
	static ucstring removeTitleFromName(const ucstring &name);
	/// Remove the shard from a name (if player from the same shard). The shard is surrounded by (), and tells the incoming shard of the entity (aniro, leanon etc...)
	static ucstring removeShardFromName(const ucstring &name);
	/// Remove both title and shard from name
	static ucstring removeTitleAndShardFromName(const ucstring &name);
	/// Change the entity name.
	void setEntityName(const ucstring &name);
	/// Return a displayable name
	ucstring getDisplayName() const
	{
		return removeTitleAndShardFromName(_EntityName);
	}
	/// Return the Name ID of the entity.
	uint32 getNameId() const {return _NameId;}
	//@}

	/// Return the entity permanent content texture name.
	const std::string &getPermanentStatutIcon() const {return _PermanentStatutIcon;}
	/// Change the entity permanent content texture name.
	void setPermanentStatutIcon(const std::string &name) { _PermanentStatutIcon=name; }


	/// Return the parent slot.
	CLFECOMMON::TCLEntityId parent() {return _Parent;}
	/// Set the parelt slot.
	void parent(CLFECOMMON::TCLEntityId p);
	// Add a new child pointer.
	void addChild(CEntityCL *c);
	// Remove a new child pointer.
	void delChild(CEntityCL *c);


	/// Is the entity able to fly.
	void flyer(bool fly) {_Flyer = fly;}
	bool flyer() {return _Flyer;}

	/**
	 * Set the skeleton for the entity or an empty string to remove a skeleton.
	 * \param filename : file with the skeleton to apply to the entity or empty string to remove a skeleton.
	 * \return USkeleton * : pointer on the skeleton handle or NULL.
	 */
	NL3D::USkeleton *skeleton(const std::string &filename);
	NL3D::USkeleton *skeleton() {return _Skeleton.empty()?NULL:&_Skeleton;}

	void setStateFx(const std::string &name);
	void removeStateFx();
	const std::string &getStateFx() {return _StateFXName;};
	/** To Inform about another (or self) entity removal (to remove from selection for example).
	 * \param slot : Slot of the entity that will be removed.
	 */
	virtual void slotRemoved(const CLFECOMMON::TCLEntityId &/* slot */) {}


	/// Return the current slot for the entity or CLFECOMMON::INVALID_SLOT if the entity is not in any slot.
	const CLFECOMMON::TCLEntityId &slot() const {return _Slot;}
	/// Set the current slot for the entity (CLFECOMMON::INVALID_SLOT for no slot).
	void slot(const CLFECOMMON::TCLEntityId &s) {_Slot = s;}


	/** \name TARGET
	 * Methods to manage the target.
	 */
	//@{
	/// Return the current target of the entity or CLFECOMMON::INVALID_SLOT.
	const CLFECOMMON::TCLEntityId &targetSlot() const {return _TargetSlot;}
	/// Set the current target of the entity (CLFECOMMON::INVALID_SLOT for no target).
	void targetSlot(const CLFECOMMON::TCLEntityId &t) {_TargetSlot = t;}
	/// get the most recent TargeSlot received in updateVisualProperty() (ie wihtout any LCT delay due to _Stages)
	CLFECOMMON::TCLEntityId		getTargetSlotNoLag() const {return _TargetSlotNoLag;}
	/// dir to target
	NLMISC::CVector dirToTarget() const;
	//@}

	/// Set the mode for the entity. return false if it will be impossible to set this mode.
	virtual bool mode(MBEHAV::EMode m) {_Mode = m; return true;}
	/// Return the entity mode.
	virtual MBEHAV::EMode mode() const {return _Mode;}


	/// Return the entity current behaviour.
	MBEHAV::EBehaviour behaviour() const {return _CurrentBehaviour.Behaviour;}

	/**
	 * Show or Hide the entity.
	 * \param s : if 'true' = entity visible, else invisible.
	 */
	void show(bool s);

	void displayable(bool d);
	bool displayable() const {return _Displayable;}


	/** \name POSITION
	 * Functions to manage the entity position.
	 */
	//@{
	/// Return true if the entity has moved since last frame.
	bool hasMoved() const {return _HasMoved;}
	/// Return the last frame position.
	const NLMISC::CVectorD &lastFramePos() const {return _LastFramePos;}
	/// Return the last frame PACS position.
	bool lastFramePACSPos(NLPACS::UGlobalPosition &result) const;
	/// Return the current PACS position.
	bool currentPACSPos(NLPACS::UGlobalPosition &result) const;
	/// Get the entity position(const method).
	const NLMISC::CVectorD &pos() const {return _Position;}
	// Get the direction matrix
	const NLMISC::CMatrix &dirMatrix() const {return _DirMatrix; }
	/// Get a reference on the entity position(method not const).
	NLMISC::CVectorD &pos() {return _Position;}
	/// Get a reference on the entity position(method not const).
	void pos(const NLMISC::CVectorD &vect);
	/// Change the PACS position and the entity position too. This is slow, prefer pacsMove if you can.
	void pacsPos(const NLMISC::CVectorD &vect, const NLPACS::UGlobalPosition &globPos = NLPACS::UGlobalPosition());
	/// Move the PACS position and the entity position too. This is fast. The PACS position will be available after the PACS::evalCollsion call.
	void pacsMove(const NLMISC::CVectorD &vect);
	/**	Update the PACS position after the evalCollision. The entity position is set too. This is fast.
	 *	If the entity position is too far from its PACS position, setGlobalPosition is called.
	 *	After this call, the position.z is valid.
	 */
	virtual void pacsFinalizeMove();
	/// Get the entity position and set all visual stuff with it.
	virtual void updateDisplay(CEntityCL *parent = 0);
	/// Set the cluster system for the current entity and all of its chidren.
	void setClusterSystem(NL3D::UInstanceGroup *cluster);
	// Get the current cluster system
	NL3D::UInstanceGroup *getClusterSystem();
	/// Choose the right cluster according to the entity position.
	void updateCluster();
	/// Snap the entity on the ground using the visual collision manager.
	virtual void snapToGround();
	//@}


	/// Get the vector up of the entity (method const).
	const NLMISC::CVector &up() const {return _Up;}
	/// Set the vector up of the entity and normalize.
	void up(const NLMISC::CVector &vect);

	/// Get the front vector (const method).
	const NLMISC::CVector &front() const {return _Front;}
	/** Change the entity front vector.
	 * \param vect : new vector to use for the front.
	 * \param compute : adjust the param 'vect' to be valid or leave the old front unchanged if impossible.
	 * \param check : warning if the param 'vect' is not valid to be the front (vector Null) even with compute=true.
	 * \param forceTurn : set front even if the entity cannot turn
	 * \return bool : 'true' if the front has been filled, else 'false'.
	 */
	bool front(const NLMISC::CVector &vect, bool compute=true, bool check=true, bool forceTurn=false);
	/// Get the front Yaw angle (const method).
	float	frontYaw() const {return (float)atan2(front().y, front().x);}

	/// Get the entity direction(this method is const).
	const NLMISC::CVector &dir() const {return _Dir;}
	/** Change the entity direction vector.
	 * \param vect : new vector to use for the direction.
	 * \param compute : adjust the param 'vect' to be valid or leave the old direction unchanged if impossible.
	 * \param check : warning if the param 'vect' is not valid to be the direction (vector Null) even with compute=true.
	 * \return bool : 'true' if the direction has been filled, else 'false'.
	 */
	bool dir(const NLMISC::CVector &vect, bool compute=true, bool check=true);

	/**
	 * Method to get the position of the head (in the world).
	 * \param headPos: will be set with the head position if succeed.
	 * \return 'true' if the param has been updated.
	 * \warning this method do NOT check if there is a skeleton.
	 */
	virtual bool getHeadPos(NLMISC::CVector &) {return false;}

	/** \name BOX.
	 * Functions to manage the box around the entity.
	 */
	//@{
	/**
	 * Set the box.
	 * \param box : an axis aligned bounding box to apply to the entity.
	 */
	void box(const NLMISC::CAABBox &box);
	/**
	 * Return a box around the entity.
	 * \return CAABBox : an axis aligned bounding box around the entity.
	 */
	const NLMISC::CAABBox &box() const {return _Aabbox;}
	/// Return the selection box.
	virtual const NLMISC::CAABBox &selectBox() {return _SelectBox;}
	/// Return the local select box (not transformed in world). Scale is included
	virtual const NLMISC::CAABBox &localSelectBox();

	//@}


	/** \name ENTITY PROPERTIES.
	 * Functions to manage the entity properties.
	 */
	//@{
	/**
	 * Return a reference on properties.
	 * \return CProperties : properties of the entity.
	 */
	CProperties			&properties() {return _Properties;}
	const CProperties	&properties() const {return _Properties;}
	//@}


	/** \name COLLISION
	 * Methods to manage the primitive.
	 */
	//@{
	/// Create a primitive for the entity.
	virtual void computePrimitive();
	/// Remove the primitive from PACS
	virtual void removePrimitive();
	/** Get the primitive used for the entity.
	 * \return UMovePrimitive * : poiner on the primitive used for the entity.
	 */
	const NLPACS::UMovePrimitive *getPrimitive() const {return _Primitive;}
	NLPACS::UMovePrimitive *getPrimitive() {return _Primitive;}

	/// Create the collision entity.
	virtual void computeCollisionEntity();
	/// Remove the collision entity.
	void removeCollisionEntity();
	/** Get a pointer on the collision entity (collision with the landscape)
	 * \return pointer on the collision entity
	 */
	NL3D::UVisualCollisionEntity *getCollisionEntity() {return _CollisionEntity;}
	/** Set the collision entity (collision with the landscape)
	 * \param pointer on the collision entity
	 */
	void setCollisionEntity( NL3D::UVisualCollisionEntity	* collisionEntity ) {_CollisionEntity = collisionEntity;}
	//@}

	/// Method to return the attack radius of an entity
	virtual double attackRadius() const;

	/** Return the position the attacker should have to combat according to the attack angle.
	 * \param ang : 0 = the front, >0 and <Pi = left side, <0 and >-Pi = right side.
	 */
	virtual NLMISC::CVectorD getAttackerPos(double ang, double dist) const;
	/// Return true if the opponent is well placed.
	virtual bool isPlacedToFight(const NLMISC::CVectorD &/* posAtk */, const NLMISC::CVector &/* dirAtk */, double /* attackerRadius */) const {return false;}

	/** Play an impact on the entity
	 * \param impactType : 0=magic, 1=melee
	 * \param type : see behaviour for spell
	 * \param intensity : see behaviour for spell
	 * \param id : see behaviour for spell
	*/
	virtual void impact(uint /* impactType */, uint /* type */, uint /* id */, uint /* intensity */) {}
	/** Try to play a melee impact on this entity.
	  */
	virtual void meleeImpact(const CPhysicalDamage &/* damage */) {}
	/** Play the magic impact on the entity
	 * \param type : type of the impact (host/good/neutral).
	 * \param intensity : intensity of the impact.
	 */
	virtual void magicImpact(uint /* type */, uint /* intensity */) {}

	/** \name DEBUG
	 * Methods only here for the debug.
	 */
	//@{
	/// Return number of stage remaining.
	virtual uint nbStage() {return 0;}

	/// Return the number of animation FXs remaining to remove.
	virtual uint nbAnimFXToRemove() {return 0;}

	/// Change the entity colors.
	virtual void changeColors(sint /* userColor */, sint /* hair */, sint /* eyes */, sint /* part */) { }
	/// Display Debug Information.
	virtual void displayDebug(float x, float &y, float lineStep);
	/// Display Debug Information for property stages
	virtual void displayDebugPropertyStages(float x, float &y, float lineStep);
	//@}

	/// TEST
	NL3D::UPlayList *playList() {return _PlayList;}
	NL3D::UPlayList *facePlayList() {return _FacePlayList;}

	/// Get a reference for all instances of the entity.
	std::vector<SInstanceCL> &instances() {return _Instances;}
	NL3D::UInstance			 instance() { return _Instance;}

	/**
	 * \param pos : result given in this variable. Only valid if return 'true'.
	 * \return bool : 'true' if the 'pos' has been filled.
	 */
	virtual bool getNamePos(NLMISC::CVectorD &/* pos */) {return false;}

	virtual bool getChestPos(NLMISC::CVectorD &/* pos */) const {return false;}

	/// Return true if the character is currently dead.
	virtual bool isDead() const {return (_Mode == MBEHAV::DEATH);}
	/// Return true if the character is really dead. With no lag because of anim or LCT
	virtual bool isReallyDead() const {return false;}

	// Add hit points gain/lost by this entity.
	void addHPOutput(sint16 hp, NLMISC::CRGBA color, float dt=0.0f) { if(_HPModifiers.size()<20) _HPModifiers.push_back(CHPModifier(hp,color,dt));}
	void addHPOutput(const ucstring &text, NLMISC::CRGBA color, float dt=0.0f) { if(_HPModifiers.size()<20 && !text.empty()) _HPModifiers.push_back(CHPModifier(text,color,dt));}

	/// Return the entity sheet scale. (return 1.0 if there is any problem).
	virtual float getSheetScale() const {return 1.0f;}
	/// Return the entity collision radius. (return 1.0 if there is any problem).
	virtual float getSheetColRadius() const {return 0.5f;}
	/// Return the entity scale. (return 1.0 if there is any problem).
	virtual float getScale() const {return 1.0;}
	// If entity is a mesh, then returns its default scale (value from export). Returns (1.f, 1.f, 1.f) of not a mesh or skinned
	void  getMeshDefaultScale(NLMISC::CVector &scale) const;

	/// 'True' if the entity is displayed.
	virtual bool isVisible() const {return false;}
	/// (Re-)Build the playlist (Removing the old one too).
	virtual void buildPlaylist() {}


	/// Serialize entity.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	// return vector of ground fxs sorted by ground type, or NULL is ground fxs are not supported for the entity
	virtual const std::vector<CGroundFXSheet> *getGroundFX() const { return NULL; }

	// are ground fx supported by entity ?
	virtual bool supportGroundFX() const { return false; }

	// Was the entity clipped during the last frame ? This is updated during the call CEntityManager::updatePostCamera
	bool getLastClip() const { return _Clipped; }
	void setLastClip(bool lastClip) { _Clipped = lastClip; }

	//--------------//
	// ENTITY INFOS //
	//--------------//

	/// Return the entity speed
	virtual double getSpeed() const {return 0.0;}

	/// Return true if the current position is an indoor position.
	bool indoor() const;

	// Return true if this entity is a user
	bool isUser () const { return Type == User; }

	// Return true if this entity is a neutral entity.
	virtual bool isNeutral () const { return false; }
	// Return true if this entity is a user's friend.
	virtual bool isFriend () const { return false; }
	// Return true if this entity is a user's enemy.
	virtual bool isEnemy () const { return false; }
	// Return true if this entity is a user's ally.
	virtual bool isAlly() const { return false; }
	// Return true if this entity is neutral pvp.
	virtual bool isNeutralPVP() const { return false; }

	/// Return true if this player has the viewing properties of a friend (inscene bars...)
	virtual bool isViewedAsFriend() const { return isNeutral() || isFriend() || isInTeam() || isInSameGuild() || isInSameLeague(); }

	/// Return the People for the entity (unknown by default)
	virtual EGSPD::CPeople::TPeople people() const;
	virtual void setPeople(EGSPD::CPeople::TPeople people);

	// todo handle NPC entities
	// Return true if this entity is a NPC (not a fauna)
	bool isNPC () const { return Type == NPC; }
	
	// Return true if this entity can have missions icons (humanoid NPCs (including Karavan), Kami or Bot Object)
	bool canHaveMissionIcon() const { return isNPC() || isKami() || isUnknownRace(); }

	// Return true if this entity is a Kami
	virtual bool isKami() const { return false; }

	// Return true if this entity has Race set to Unknown
	virtual bool isUnknownRace() const { return false; }

	// Return true if this entity is a fauna (not a NPC)
	bool isFauna () const { return Type == Fauna; }

	// Return true if this entity is a AI (fauna or NPC)
	bool isAI () const { return (Type == Fauna) || (Type == NPC); }

	// Return true if this entity is a forage source
	bool isForageSource() const { return Type == ForageSource; }

	// Return true if this entity is a Player Character
	bool isPlayer () const { return Type == Player; }

	// Return true if this entity is the current user selection. NB: actually test UserEntity->selection() (ie not laggy!)
	bool isTarget () const;

	// Return true if this entity is under the cursor
	bool isUnderCursor () const
	{
		return _Slot == SlotUnderCursor;
	}

	// Is a mission target
	bool isMissionTarget () { return _MissionTarget; }

	// Return true if this entity is in the user group
	bool isInTeam () const { return _IsInTeam; }

	// Return true if this entity is in the user guild
	bool isInSameGuild () const;

	// Return true if this entity is in the user league
	bool isInSameLeague () const;

	// Return true if this entity or the user is in a league
	bool oneInLeague () const;

	// Return true if this entity is a Mount owned by the User
	bool isUserMount () const {return _IsUserMount;}

	// Return true if this entity is a pack animal owned by the User
	bool isUserPackAnimal () const {return _IsUserPackAnimal;}

	// Return info on the guild
	virtual uint32 getGuildNameID () const { return 0; }
	virtual uint64 getGuildSymbol () const { return 0; }

	virtual uint32 getEventFactionID() const { return 0; }
	virtual uint16 getPvpMode() const { return PVP_MODE::None; }
	virtual PVP_CLAN::TPVPClan getPvpClan() const { return PVP_CLAN::None; }
	virtual uint32 getLeagueID() const { return 0; }

	virtual uint16					getOutpostId() const { return 0; }
	virtual OUTPOSTENUMS::TPVPSide	getOutpostSide() const { return OUTPOSTENUMS::UnknownPVPSide; }
	bool					isAnOutpostEnemy() const;
	bool					isAnOutpostAlly() const;

	/// Return the entity title
	const ucstring &getTitle() const
	{
		return _Title;
	}

	/// Return the entity tags
	const ucstring &getTag(uint8 id) const
	{
		if (_Tags.size() > id) {
			return _Tags[id];
		}
		static ucstring empty;
		return empty;
	}

	/// Return the raw unparsed entity title
	const ucstring getTitleRaw() const
	{
		return ucstring(_TitleRaw);
	}

	/// Return true if this entity has a reserved title
	bool hasReservedTitle() const { return _HasReservedTitle; }

	/// Return true if this entity can turn
	bool canTurn() const { return _CanTurn;	}

	/// Get entity color
	NLMISC::CRGBA	getColor () const;

	// Rebuild in scene interfaces
	virtual void buildInSceneInterface ();

	// enable display of in scene interface. This flag is 'anded' with the final value when calling 'mustShowInsceneInterface'
	void enableInSceneInterface(bool enabled) { _InSceneInterfaceEnabled = enabled; }

	// Update the mission target flag
	void updateMissionTarget ();

	// get the type of the ground below the entity
	uint getGroundType() const;

	virtual void makeTransparent(bool t);
	virtual void makeTransparent(float factor);
	virtual void setDiffuse(bool onOff, NLMISC::CRGBA diffuse);


	static NLMISC::CCDBNodeLeaf *getOpacityDBNode();
	static uint32 getOpacityMin();
	static void setOpacityMin(uint32 value);

	// Update the _IsInTeam flag
	void updateIsInTeam ();

	// update for isUserMount() and isUserPackAnimal()
	void updateIsUserAnimal ();

	// if a pack animal, return the animal status. 0 if not a pack animal
	ANIMAL_STATUS::EAnimalStatus	getPackAnimalStatus() const;

	// if a pack animal, return the animal DB index
	bool	getPackAnimalIndexInDB(sint &dbIndex) const;

	// remove all attached fx of that entity (so that they can be reloaded)
	virtual void removeAllAttachedFX() {}

	// get bone name from a part of the body (base implementation provides the base bones names)
	virtual const char *getBoneNameFromBodyPart(BODY::TBodyPart part, BODY::TSide side) const;
	// ...
	virtual bool getBoneHeight(BODY::TBodyPart /* localisation */, BODY::TSide /* side */, float &/* height */) const {return false;}

	// true if the entity position is valid (related to network lag at creation time)
	bool		firstPositionReceived() const {return !_First_Pos;}

	// VISUAL SELECTION

	// Start a new visual selection
	void visualSelectionStart();

	// Stop the visual selection
	void visualSelectionStop();

	// true if the entity allow to cast a shadowMap (eg: false for userentity in some case). Default to ClientCFG value
	virtual bool	canCastShadowMap() const;

	// call when want to update the CastShadowMap flags of skeleton/instance
	void			updateCastShadowMap();

	/// \name Static properties (according to entity type / sheet)
	//@{
	// display the entity in the radar
	bool			getDisplayInRadar() const {return _DisplayInRadar;}
	// name is displayed if (_Sheet->DisplayOSD && DisplayName)
	bool			getDisplayOSDName() const {return _DisplayOSDName;}
	// bars are displayed if (_Sheet->DisplayOSD && DisplayBars)
	bool			getDisplayOSDBars() const {return _DisplayOSDBars;}
	// even if ClientCfg.ShowNameUnderCursor==false, force OSD to display when under cursor (not if _Sheet->DisplayOSD=false)
	bool			getDisplayOSDForceOver() const {return _DisplayOSDForceOver;}
	// the user can traverse this entity after some "force time"
	bool			getTraversable() const {return _Traversable;}
	//@}


	// set ordering layer (for sorting with transparent surfaces)
	void			setOrderingLayer(uint layer);
	// reset all sound anim id this entity may own
	virtual void	resetAllSoundAnimId() {}

	// force to evaluate entity & ancestor anim (and snap to  ground if necessary)
	void forceEvalAnim();

	bool isAsyncLoading() const;
protected:
	enum { BadIndex = 0xFFFFFFFF };

	// Entity Id (CLFECOMMON::INVALID_CLIENT_DATASET_INDEX for an invalid one)
	CLFECOMMON::TClientDataSetIndex	_DataSetId;
	// Sheet Id of the entity.
	NLMISC::CSheetId				_SheetId;
	// Persistent NPC Alias of the entity
	uint32							_NPCAlias;
	// Local DB Branch for this entity
	class NLMISC::CCDBNodeBranch			*_DBEntry;
	// Playlist
	NL3D::UPlayList					*_PlayList;
	NL3D::UPlayList					*_FacePlayList;
	// Collision entity used to test collision with landscape
	NL3D::UVisualCollisionEntity	*_CollisionEntity;
	// Entity skeleton
	NL3D::USkeleton					_Skeleton;
	/// Slot of the entity.
	CLFECOMMON::TCLEntityId			_Slot;
	// Slot of the target or CLFECOMMON::INVALID_SLOT if there is no target.
	CLFECOMMON::TCLEntityId			_TargetSlot;
	// same, but see getTargetSlotNoLag()
	CLFECOMMON::TCLEntityId			_TargetSlotNoLag;

	// Temp Debug GUIGUI
#ifdef TMP_DEBUG_GUIGUI
	// Theoretical Position
	NLMISC::CVectorD				_TheoreticalPosition;
	// Theoretical Orientation (last orientation received).
	float							_TheoreticalOrientation;
#endif // TMP_DEBUG_GUIGUI

	// Current entity position.
	NLMISC::CVectorD				_Position;
	// Useful to limit some noise on positions.
	NLMISC::CVectorD				_PositionLimiter;
	// Last Frame Position
	NLMISC::CVectorD				_LastFramePos;
	// Last Frame PACS Position
	NLPACS::UGlobalPosition			_LastFramePACSPos;
	// Current mode
	MBEHAV::EMode					_Mode;
	// Theoretical Current Mode (could be different from the current mode).
	MBEHAV::EMode					_TheoreticalMode;
	// Current behaviour
	MBEHAV::CBehaviour				_CurrentBehaviour;
	// Flags to know what is possible to do with the entity (selectable, liftable, etc.).
	CProperties						_Properties;
	// Current Name for the entity
	ucstring						_EntityName;
	// Current entity title
	ucstring						_Title;
	// Current entity tags
	std::vector<ucstring>			_Tags;
	// Current entity title string id
	ucstring						_TitleRaw;
	// Current permanent content symbol for the entity
	std::string						_PermanentStatutIcon;
	// Has reserved title?
	bool							_HasReservedTitle;

	// Extended Name
	ucstring						_NameEx;
	// String ID
	uint32							_NameId;
	// Primitive used for the collision in PACS
	NLPACS::UMovePrimitive			*_Primitive;
	// 3D Logic info for light request.
	CEntityLogicInfo3D				_LogicInfo3D;
	// Box around the entity.
	NLMISC::CAABBox					_Aabbox;
	// Sphere around the entity for 3D clipping (Local to Entity Pos).
	float							_ClipRadius;	// Radius of the clip sphere
	float							_ClipDeltaZ;	// DeltaZ of the sphere center to ground (different from clipradius)

	/// Parent Slot or CLFECOMMON::INVALID_SLOT if there is no parent.
	CLFECOMMON::TCLEntityId			_Parent;
	/// List of children.
	typedef std::list<CEntityCL *>	TChildren;
	TChildren						_Children;
	/// 3D mesh if the entity has no skeleton.
	NL3D::UInstance					_Instance;

	/// Meshes to bind to the skeleton if the entity has one.
	std::vector<SInstanceCL>		_Instances;

	// Orientation of the entity.
	NLMISC::CVector					_Front;
	// Entity Up.
	NLMISC::CVector					_Up;
		// Angle to be linked to the target.
	double							_TargetAngle;
	// Current direction for the entity.
	NLMISC::CVector					_Dir;
	// Matrix of the entity direction.
	NLMISC::CMatrix					_DirMatrix;
	// The final PACS position
	NLMISC::CVectorD				_FinalPacsPos;
	/// The last successfully retrieved position as a vector
	NLMISC::CVectorD				_LastRetrievedPosition;
	/// The last successfully retrieved position as a pacs position
	NLPACS::UGlobalPosition			_LastRetrievedPacsPosition;
	// Box around the entity.
	NLMISC::CAABBox					_SelectBox;
	// Local selection box
	NLMISC::CAABBox					_LocalSelectBox;
	// List of modifiers taken by this entity.
	class CHPModifier
	{
	public:
		CHPModifier() {}
		virtual ~CHPModifier() {}
		CHPModifier (sint16 value, NLMISC::CRGBA color, float dt) : Value(value), Color(color), DeltaT(dt) {}
		CHPModifier (const ucstring &text, NLMISC::CRGBA color, float dt) : Text(text), Color(color), DeltaT(dt) {}

		sint16			Value;		// If Text.empty(), take the Value
		ucstring		Text;
		NLMISC::CRGBA	Color;
		float			DeltaT;
	};
	std::list<CHPModifier>			_HPModifiers;
	//
	class HPMD : public CHPModifier
	{
	public:
		double	Time;
		// DeltaZ between pos() and namePos(). computed only one time
		float	DeltaZ;
		HPMD()
		{
			DeltaZ= -FLT_MAX;
		}
	};
	std::list<HPMD>				_HPDisplayed;

	// The transparency factor
	float _TranspFactor; // 0 - opaque 1 - transparent

	// Entities color
	static NLMISC::CRGBA		_EntitiesColor[TypeCount];
	static NLMISC::CRGBA		_DeadColor;
	static NLMISC::CRGBA		_TargetColor;
	static NLMISC::CRGBA		_GroupColor;
	static NLMISC::CRGBA		_GuildColor;
	static NLMISC::CRGBA		_UserMountColor;
	static NLMISC::CRGBA		_UserPackAnimalColor;
	// colors for PvP
	static NLMISC::CRGBA		_PvpEnemyColor;
	static NLMISC::CRGBA		_PvpNeutralColor;
	static NLMISC::CRGBA		_PvpAllyInTeamColor;
	static NLMISC::CRGBA		_PvpAllyInLeagueColor;
	static NLMISC::CRGBA		_PvpAllyColor;
	// colors for GM players
	static NLMISC::CRGBA		_GMTitleColor[CHARACTER_TITLE::EndGmTitle - CHARACTER_TITLE::BeginGmTitle + 1];

	/// invalid gm title id
	static uint8				_InvalidGMTitleCode;

	// true if all instances are not loaded.
	bool							_AsyncTextureLoading     : 1;
	// true if the lod Texture Need to be recomputed
	bool							_LodTextureDirty         : 1;
	// True as long as the entity has not received any position.
	bool							_First_Pos               : 1;
	//
	bool							_MountRdy                : 1;	// obsolete
	// Id the entity a flyer (NO SNAP TO GROUND)
	bool							_Flyer                   : 1;
	// Set global position not done for this entity
	bool							_SetGlobalPositionDone   : 1;
	// Snap to ground not done for this entity
	bool							_SnapToGroundDone        : 1;
	// Do we have to display or not this entity.
	bool							_Displayable             : 1;
	// Was the entity visible during the last frame ? This is updated during the call to CEntityManager::updatePostCamera
	bool							_Clipped                 : 1;
	// Has the entity moved this fram ?
	bool							_HasMoved                : 1;
	// The entity is a mission target
	bool							_MissionTarget           : 1;
	// The entity is in the team of the player
	bool							_IsInTeam				 : 1;
	// The entity is the User's mount
	bool							_IsUserMount			 : 1;
	// The entity is a User's pack animal
	bool							_IsUserPackAnimal		 : 1;
	// Indicate that some of _Instances have enableCastShadowMap(true) set
	bool							_SomeInstanceCastShadowMap : 1;
	// Is this the First Position Managed ?
	bool							_FirstPosManaged         : 1;

	// display the entity in the radar
	bool							_DisplayInRadar			 : 1;
	// name is displayed if (_Sheet->DisplayOSD && DisplayName)
	bool							_DisplayOSDName			 : 1;
	// bars are displayed if (_Sheet->DisplayOSD && DisplayBars)
	bool							_DisplayOSDBars			 : 1;
	// even if ClientCfg.ShowNameUnderCursor==false, force OSD to display when under cursor (not if _Sheet->DisplayOSD=false)
	bool							_DisplayOSDForceOver	 : 1;
	// the user can traverse this entity after some "force time"
	bool							_Traversable			 : 1;
	// The entity can turn
	bool							_CanTurn				 : 1;
	// The entity must not be clipped by camera
	bool							_ForbidClipping			 : 1;
	// bkup state for setVisualSelectionBlink
	bool							_VisualSelectionBlinked	 : 1;
	// OSD enabled ?
	bool							_InSceneInterfaceEnabled : 1;

	// The groundType cache
	mutable uint32					_GroundTypeCache;
	mutable NLMISC::CVectorD		_GroundTypeCachePos;

	NL3D::UParticleSystemInstance	_SelectionFX;
	NL3D::UParticleSystemInstance	_MouseOverFX;
	NL3D::UParticleSystemInstance	_StateFX;
	std::string						_StateFXName;

	/// gamemaster title code of the entity, if any
	uint							_GMTitle;

	// Shadow Stuff
	float							_ShadowMapZDirClamp;
	float							_ShadowMapMaxDepth;
	sint64							_ShadowMapPropertyLastUpdate;

	// Visual selection stuff
	sint64							_VisualSelectionTime;

	// for localSelectBox() computing
	sint64										_LastLocalSelectBoxComputeTime;

	static NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _OpacityMinNodeLeaf;
	static NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _ShowReticleLeaf;

protected:
	/**
	 * Change the box position.
	 * \param pos contains the new box position.
	 */
	void posBox(const NLMISC::CVector &pos);

	/// Update Entity Position.
	virtual void updateVisualPropertyPos          (const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */, const NLMISC::TGameCycle &/* pI */) {}
	/// Update Entity Orientation.
	virtual void updateVisualPropertyOrient       (const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Update Entity Behaviour.
	virtual void updateVisualPropertyBehaviour    (const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Update Entity Name.
	virtual void updateVisualPropertyName         (const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Update Entity Target.
	virtual void updateVisualPropertyTarget       (const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Update Entity Mode.
	virtual void updateVisualPropertyMode         (const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Update Entity Visual Property A
	virtual void updateVisualPropertyVpa          (const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Update Entity Visual Property B
	virtual void updateVisualPropertyVpb          (const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Update Entity Visual Property C
	virtual void updateVisualPropertyVpc          (const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Update Entity Mount
	virtual void updateVisualPropertyEntityMounted(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Update Entity Rider
	virtual void updateVisualPropertyRiderEntity  (const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Update Entity Bars
	virtual void updateVisualPropertyBars			(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	virtual void updateVisualPropertyGuildSymbol	(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	virtual void updateVisualPropertyGuildNameID	(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	virtual void updateVisualPropertyEventFactionID	(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	virtual void updateVisualPropertyPvpMode		(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	virtual void updateVisualPropertyPvpClan		(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	virtual void updateVisualPropertyOwnerPeople	(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	virtual void updateVisualPropertyOutpostInfos	(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}


	/// Update Entity State
	virtual void updateVisualPropertyStatus       (const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Target lists
	virtual void updateVisualPropertyTargetList(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */, uint /* listIndex */) {}
	/// Visual FX
	virtual void updateVisualPropertyVisualFX(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */) {}
	/// Update vprop contextual attributes
	virtual void updateVisualPropertyContextual(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);

	/// Return true if the in-scene interface must be shown
	bool		mustShowInsceneInterface( bool enabledInSheet ) const;

	/** Return the instance from its index.
	 * \param idx : index of the instance.
	 * \return SInstanceCL * : pointer on the instance associated to the index or 0 if idx has no instance.
	 */
	SInstanceCL	*idx2Inst(uint idx);

	// Initialize the Object with this function for all constructors.
	void init();

	// hide the entity Skin (all entity instances).
	void hideSkin();

	// called each frame to update some shadowmap properties according to position of player
	void			updateShadowMapProperties();

	// virtual for special PlayerCL _Face mgt
	virtual void doSetVisualSelectionBlink(bool bOnOff, NLMISC::CRGBA emitColor);


public:
	// Make the selection blinking
	void setVisualSelectionBlink(bool bOnOff, NLMISC::CRGBA emitColor)
	{
		// if already disabled, no need
		// NB: cannot do this if both true, since emitColor may not be the same (emitColor caching not interesting...)
		if(!bOnOff && !_VisualSelectionBlinked)
			return;
		doSetVisualSelectionBlink(bOnOff, emitColor);
		// cache
		_VisualSelectionBlinked= bOnOff;
	}
/** \name 3D System
	 * Methods to manage basics 3D systems
	 */
	//@{
	/** update the display of the AsyncTexture of the entity. called in updateDisplay()
	 *	Deriver: See CPlayerCL implementation
	 *	\return distance from entity to camera computed (helper for deriver)
	 */
	virtual	float		updateAsyncTexture();
	/// Update the Lod Texture When needed
	virtual	void		updateLodTexture();
	//@}

	// Read/Write Variables from/to the stream.
	virtual void readWrite(class NLMISC::IStream &f) throw(NLMISC::EStream);
	// To call after a read from a stream to re-initialize the entity.
	virtual void load();

private:

	// Override for string reception callback
	virtual void onStringAvailable(uint stringId, const ucstring &value);

};


#endif // CL_ENTITY_CL_H

/* End of entity_cl.h */
