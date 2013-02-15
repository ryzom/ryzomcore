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



#ifndef CL_PLAYER_R2_CL_H
#define CL_PLAYER_R2_CL_H


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
 * Class to manage a R2 player.
 * \author Typhaine Le Gallo
 * \author Nevrax France
 * \date 2005
 */
class CPlayerR2CL : public CCharacterCL
{
public:
	NLMISC_DECLARE_CLASS(CPlayerR2CL);

	/// Constructor
	CPlayerR2CL();
	/// Destructor
	virtual ~CPlayerR2CL();

	/// Build the entity from a sheet.
	virtual bool build(const CEntitySheet *sheet);

	/// Method to return the attack radius of an entity
	virtual double attackRadius() const;
	/** Return the position the attacker should have to combat according to the attack angle.
	 * \param ang : 0 = the front, >0 and <Pi = left side, <0 and >-Pi = right side.
	 */
	virtual NLMISC::CVectorD getAttackerPos(double ang, double dist) const;

	/** \name DEBUG
	 * Methods only here for the debug.
	 */
	//@{
	/// Display Debug Information.
	virtual void displayDebug(float x, float &y, float lineStep);
	//@}

	/// Return the People for the entity.
	//virtual EGSPD::CPeople::TPeople people() const;
	//virtual void setPeople(EGSPD::CPeople::TPeople people);

	/// Return a pointer on the sheet used to create this player.
	//const CRaceStatsSheet *playerSheet() const {return _PlayerSheet;}

	/// Return the entity scale. (return 1.0 if there is any problem).
	virtual float getScale() const;
	// return vector of ground fxs sorted by ground type, or NULL is ground fxs are not supported for the entity
	//virtual const std::vector<CGroundFXSheet> *getGroundFX() const;
	virtual bool supportGroundFX() const { return true; }

	/// Return true if this player is in the same faction as the user's (except if neutral)
	bool isFromSameNonNeutralPvpClanAsUser() const;

	// From CEntityCL
	//const char *getBoneNameFromBodyPart(BODY::TBodyPart part, BODY::TSide side) const;

	// retrieve right hand item sheet
	virtual const CItemSheet *getRightHandItemSheet() const;
	virtual const CItemSheet *getLeftHandItemSheet() const;

	//virtual const CAttack *getAttack(const CAttackIDSheet &id) const;

	virtual float getScaleRef() const;

	// from CEntityCL
	void makeTransparent(bool t);
	virtual void setDiffuse(bool onOff, NLMISC::CRGBA diffuse);

protected:
	/// Pointer on the Sheet with basic parameters.
	//const CPlayerSheet		*_Sheet;
	/// Pointer on the Sheet with basic parameters.
	//const CRaceStatsSheet	*_PlayerSheet;
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
	//virtual std::string automatonType() const;

	// Initialize the graphic for the player.
	void init3d();

	/// Initialize properties of the entity (according to the class).
	virtual void initProperties();

	/// Set the equipmenent worn.
	void equip(SLOTTYPE::EVisualSlot slot, const std::string &shapeName, const CItemSheet *item = 0);
	/// Compute the equipmenent worn.
	void equip(SLOTTYPE::EVisualSlot slot, uint index, uint color);

	/// Update the Visual Property A
	virtual void updateVisualPropertyVpa(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update the Visual Property B
	virtual void updateVisualPropertyVpb(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	/// Update the Visual Property C
	virtual void updateVisualPropertyVpc(const NLMISC::TGameCycle &gameCycle, const sint64 &prop);
	// Get The Entity Skin
	//virtual sint skin() const;

	/// Update blink
	virtual SInstanceCL *getFace ();

	// Draw the name.
	virtual void drawName(const NLMISC::CMatrix &mat);

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
	/// Return the basic max speed for the entity in meter per sec
	virtual double getMaxSpeed() const;

	// Read/Write Variables from/to the stream.
	virtual void readWrite(class NLMISC::IStream &f) throw(NLMISC::EStream);
	// To call after a read from a stream to re-initialize the entity.
	virtual void load();

	/// Return name position on Z axis defined in sheet
	//virtual float getNamePosZ() const;

	// virtual for special PlayerCL _Face mgt
	virtual void doSetVisualSelectionBlink(bool bOnOff, NLMISC::CRGBA emitColor);

	CGenderInfo * getGenderInfo();

};


#endif // CL_PLAYER_R2_CL_H

/* End of player_r2_cl.h */
