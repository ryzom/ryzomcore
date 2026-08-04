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



#ifndef CL_PLAYER_CL_H
#define CL_PLAYER_CL_H


/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
// 3d
#include "nel/3d/u_point_light.h"
// Client.
#include "character_cl.h"
// Client Sheets
#include "client_sheets/race_stats_sheet.h"
// Game Share
#include "game_share/people.h"


///////////
// CLASS //
///////////
class CPlayerSheet;

/**
 * Class to manage a player.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CPlayerCL : public CCharacterCL
{
public:
	NLMISC_DECLARE_CLASS(CPlayerCL);

	/// Constructor
	CPlayerCL();
	/// Destructor
	virtual ~CPlayerCL() NL_OVERRIDE;

	/// Build the entity from a sheet.
	virtual bool build(const CEntitySheet *sheet) NL_OVERRIDE;

	/// Method to return the attack radius of an entity
	virtual double attackRadius() const NL_OVERRIDE;
	/** Return the position the attacker should have to combat according to the attack angle.
	 * \param ang : 0 = the front, >0 and <Pi = left side, <0 and >-Pi = right side.
	 */
	virtual NLMISC::CVectorD getAttackerPos(double ang, double dist) const NL_OVERRIDE;

	/** \name DEBUG
	 * Methods only here for the debug.
	 */
	//@{
	/// Display Debug Information.
	virtual void displayDebug(float x, float &y, float lineStep) NL_OVERRIDE;
	//@}

	/// Return the People for the entity.
	virtual EGSPD::CPeople::TPeople people() const NL_OVERRIDE;
	virtual void setPeople(EGSPD::CPeople::TPeople people) NL_OVERRIDE;

	/// Return a pointer on the sheet used to create this player.
	const CRaceStatsSheet *playerSheet() const {return _PlayerSheet;}

	// from CEntityCL
	virtual void computePrimitive() NL_OVERRIDE;

	/// Return the entity scale. (return 1.0 if there is any problem).
	virtual float getScale() const NL_OVERRIDE;
	// return vector of ground fxs sorted by ground type, or NULL is ground fxs are not supported for the entity
	virtual const std::vector<CGroundFXSheet> *getGroundFX() const NL_OVERRIDE;
	virtual bool supportGroundFX() const NL_OVERRIDE { return true; }

	// Return true if this entity is a neutral entity.
	virtual bool isNeutral () const NL_OVERRIDE;
	// Return true if this entity is a user's friend.
	virtual bool isFriend () const NL_OVERRIDE;
	// Return true if this entity is a user's enemy.
	virtual bool isEnemy () const NL_OVERRIDE;
	// Return true if this entity is a user's ally.
	virtual bool isAlly() const NL_OVERRIDE;
	// Return true if this entity is neutral pvp.
	virtual bool isNeutralPVP() const NL_OVERRIDE;

	/// Ask if the entity is afk (a character is never afk but players can be)
	virtual bool isAFK() const NL_OVERRIDE {return (_Mode == MBEHAV::REST || properties().afk());}

	/// Return true if this player is in the same faction as the user's (except if neutral)
	bool isFromSameNonNeutralPvpClanAsUser() const;

	// From CEntityCL
	const char *getBoneNameFromBodyPart(BODY::TBodyPart part, BODY::TSide side) const NL_OVERRIDE;

	// retrieve right hand item sheet
	virtual const CItemSheet *getRightHandItemSheet() const NL_OVERRIDE;
	virtual const CItemSheet *getLeftHandItemSheet() const NL_OVERRIDE;

	virtual const CAttack *getAttack(const CAttackIDSheet &id) const NL_OVERRIDE;

	virtual float getScaleRef() const NL_OVERRIDE;

protected:
	/// Pointer on the Sheet with basic parameters.
	const CPlayerSheet		*_Sheet;
	/// Pointer on the Sheet with basic parameters.
	const CRaceStatsSheet	*_PlayerSheet;
	/// Player Face
	SInstanceCL				_Face;
	/// Default Look
	std::string				_DefaultChest;
	std::string				_DefaultLegs;
	std::string				_DefaultArms;
	std::string				_DefaultHands;
	std::string				_DefaultFeet;
	std::string				_DefaultHair;
	sint32					_HairColor;
	sint32					_EyesColor;
	/// 'true' while the entity is not ready to be displayed.
	bool					_WaitForAppearance;
	// AsyncTexturing: true if all instances are not loaded.
	bool					_PlayerCLAsyncTextureLoading;
	// Is the light On or Off.
	bool					_LightOn;
	// Light
	NL3D::UPointLight		_Light;

	std::string _CacheSkeletonShapeName;

protected:
	// Return the automaton type of the entity (homin, creature, etc.)
	virtual std::string automatonType() const NL_OVERRIDE;

	// Initialize the graphic for the player.
	void init3d();

	/// Initialize properties of the entity (according to the class).
	virtual void initProperties() NL_OVERRIDE;

	/// Set the equipmenent worn.
	void equip(SLOTTYPE::EVisualSlot slot, const std::string &shapeName, const CItemSheet *item = nullptr);
	/// Compute the equipmenent worn.
	void equip(SLOTTYPE::EVisualSlot slot, uint index, uint color);

	/// Compute the animation set to use according to weapons, mode and race.
	virtual void computeAnimSet() NL_OVERRIDE;

	/// Update the Visual Property A
	virtual void updateVisualPropertyVpa(const NLMISC::TGameCycle &gameCycle, const sint64 &prop) NL_OVERRIDE;
	/// Update the Visual Property B
	virtual void updateVisualPropertyVpb(const NLMISC::TGameCycle &gameCycle, const sint64 &prop) NL_OVERRIDE;
	/// Update the Visual Property C
	virtual void updateVisualPropertyVpc(const NLMISC::TGameCycle &gameCycle, const sint64 &prop) NL_OVERRIDE;
	/// Update the Visual Property PVP Mode (need special imp for player because of PVP consider)
	virtual void updateVisualPropertyPvpMode(const NLMISC::TGameCycle &gameCycle, const sint64 &prop) NL_OVERRIDE;

	// Get The Entity Skin
	virtual sint skin() const NL_OVERRIDE;

	/// Update blink
	virtual SInstanceCL *getFace () NL_OVERRIDE;

	// Draw the name.
	virtual void drawName(const NLMISC::CMatrix &mat) NL_OVERRIDE;

	/** \name 3D System
	 * Methods to manage basics 3D systems
	 */
	//@{
	/** update the display of the AsyncTexture of the entity. called in updateDisplay()
	 *	Deriver: See CPlayerCL implementation
	 *	\return distance from entity to camera computed (helper for deriver)
	 */
	virtual	float		updateAsyncTexture() NL_OVERRIDE;

	/// Update the Lod Texture When needed
	virtual	void		updateLodTexture() NL_OVERRIDE;
	//@}
	/// Return the basic max speed for the entity in meter per sec
	virtual double getMaxSpeed() const NL_OVERRIDE;

	// Read/Write Variables from/to the stream.
	virtual void readWrite(NLMISC::IStream &f) NL_OVERRIDE;
	// To call after a read from a stream to re-initialize the entity.
	virtual void load() NL_OVERRIDE;

	/// Return name position on Z axis defined in sheet
	virtual float getNamePosZ() const NL_OVERRIDE;

	// virtual for special PlayerCL _Face mgt
	virtual void doSetVisualSelectionBlink(bool bOnOff, NLMISC::CRGBA emitColor) NL_OVERRIDE;

};


#endif // CL_PLAYER_CL_H

/* End of player_cl.h */
