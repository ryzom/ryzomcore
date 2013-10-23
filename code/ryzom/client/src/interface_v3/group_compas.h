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




#ifndef CL_GROUP_COMPAS_HELP_H
#define CL_GROUP_COMPAS_HELP_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2f.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_menu.h"
#include "animal_position_state.h"

class CViewRadar;

// time for the compass to blink (to indicate that player has clicked on it)
const float COMPASS_BLINK_TIME = 0.3f;

/**
 * Target of a compass
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2003
 */

class CCompassTarget
{
public:
	enum TType	{ North = 0, Selection, Home, Respawn, ContinentLandMark, UserLandMark, PosTracker, NumTypes };
	NLMISC::CVector2f		Pos;		// Used for static target (ie not the current selection, a team member ...)
	ucstring				Name;
	CCompassTarget();
	TType					 getType() const { return _Type; }
	void                     setType(TType type) { if (type == _Type) return; setPositionState(NULL); _Type = type; }
	// returns position tracker (if type is 'PosTracker')
	CPositionState			*getPositionState() { return _PositionState; }
	void					 setPositionState(CPositionState	*ps) {if (ps != _PositionState) _PositionState = ps; if (ps) _Type = PosTracker; }
	//
	void					 serial(NLMISC::IStream &f);
private:
	TType							  _Type;
	NLMISC::CSmartPtr<CPositionState> _PositionState; // use smart pointer for copy/dtor convenience
};


// helper function to build compass targets
bool	buildCompassTargetFromTeamMember(CCompassTarget &ct, uint teamMemberId);
bool	buildCompassTargetFromAnimalMember(CCompassTarget &ct, uint animalMemberId);


/**
 * Compas group
 *
 * Default target is north
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2003
 */
class CGroupCompas : public CGroupContainer
{
	friend class CHandlerChangeCompas;
public:


	// Constructor
	CGroupCompas(const TCtorParam &param);
	~CGroupCompas();

	// From CInterfaceElement
	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
	virtual void updateCoords();
	virtual void draw();
	virtual bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);

	void setTarget(const CCompassTarget &target);
	const CCompassTarget &getTarget() const { return _Target; }

	// force the compass to blink (for example to indicate that a new target has been selected)
	void blink();


	virtual bool wantSerialConfig() const;
	// config serialization will save the current compass direction
	virtual void serialConfig(NLMISC::IStream &f);

	bool			isSavedTargetValid() const { return _SavedTargetValid; }
	CCompassTarget &getSavedTarget() { return _SavedTarget; }

private:
	// The arrow shape
	class CInterface3DShape			*_ArrowShape;
	CCompassTarget					_Target;
	bool							_TargetSetOnce;
	CCompassTarget					_SavedTarget;
	bool							_SavedTargetValid;

	NLMISC::CCDBNodeLeaf					*_DynamicTargetPos;
	uint32							_LastDynamicTargetPos;

	// Color for each type of target
	NLMISC::CRGBA					_TargetTypeColor[CCompassTarget::NumTypes];

	// Color when a new target has been selected
	NLMISC::CRGBA					_NewTargetSelectedColor;

	bool	_Blinking;
	double  _StartBlinkTime;
	ucstring _CurrTargetName;

	// The dist text
	CViewText	*_DistView;
	ucstring	_DistViewText;

	CViewRadar	*_RadarView;
	CViewText	*_RadarRangeView;
	uint32		_RadarPos;

	NLMISC::CVector2f getNorthPos(const NLMISC::CVector2f &userPos) const;
};

/**
 * Compas menu group
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2003
 */
class CGroupCompasMenu : public CGroupMenu
{
public:

	// Constructor
	CGroupCompasMenu(const TCtorParam &param);
	~CGroupCompasMenu();

	// parse
	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parent=NULL);

	// From CInterfaceElement
	virtual void setActive (bool state);

	// name of the target compass for that menu
	std::string _TargetCompass;

	// current locations for displayed menu
	std::vector<CCompassTarget> Targets;
};

/**
 * singleton used to store dialog compass targets
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2005
 */
class CCompassDialogsManager
{
public:
	/// entry in the dialog
	struct CCompassDialogsEntry
	{
		CCompassDialogsEntry(sint32 x,sint32 y,	uint32 text )
			:X(x), Y(y), Text(text){}

		sint32 X;
		sint32 Y;
		uint32 Text;
	};
	static CCompassDialogsManager & getInstance()
	{
		if ( _Instance == NULL )
			_Instance = new CCompassDialogsManager;
		return *_Instance;
	}

	const std::vector<CCompassDialogsEntry> & getEntries() { return _Entries; }

	void addEntry( sint32 x, sint32 y, uint32 text )
	{
		_Entries.push_back( CCompassDialogsEntry(x,y,text) );
	}

	void removeEntry(uint32 text);


private:
	friend class CCompassDialogsStringCallback;

	CCompassDialogsManager(){}
	~CCompassDialogsManager(){}

	static CCompassDialogsManager * _Instance;
	std::vector<CCompassDialogsEntry> _Entries;
};

#endif // CL_GROUP_COMPAS_HELP_H
