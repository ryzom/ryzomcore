// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2020  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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




#ifndef CL_GROUP_MAP_CL
#define CL_GROUP_MAP_CL

#include "nel/misc/vector_2f.h"
#include "nel/misc/ucstring.h"

#include "../client_sheets/world_sheet.h"

#include "nel/gui/interface_group.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/view_text.h"
#include "animal_position_state.h"
#include "../continent.h"
//


class CContinent;

namespace NLMISC
{
	class CCDBNodeLeaf;
}

namespace NLGUI
{
	class CCtrlQuad;
}

class CWorldSheet;
struct SMap;

namespace NL3D
{
	class UMaterial;
	class UDriver;
	class UTextureFile;
}


#define MISSIONS_DB_PATH "SERVER:MISSIONS"
#define GROUP_MISSIONS_DB_PATH "SERVER:GROUP:MISSIONS"
#define COMPASS_DB_PATH "SERVER:COMPASS"
//
const float RYZOM_MAP_MAX_SCALE = 8.f;


class CLandMarkOptions
{
public:
	std::string			LandMarkTexNormal;
	std::string			LandMarkTexOver;
	std::string			LandMarkTexPushed;
	NLMISC::CRGBA		ColorNormal;
	NLMISC::CRGBA		ColorOver;
	NLMISC::CRGBA		ColorPushed;
	std::string			LandMarkMenu;
public:
	CLandMarkOptions()
	{
		ColorNormal = ColorOver = ColorPushed = NLMISC::CRGBA::White;
	}
};

class CArkPoint
{
public:
	sint32 				x, y;
	std::string			Texture;
	NLMISC::CRGBA		Color;
	std::string			Title;
	std::string			LeftClickAction;
	std::string			LeftClickParam;
	std::string			RightClickAction;
	std::string			RightClickParam;
	std::string			OverClickAction;
	std::string			OverClickParam;

public:
	CArkPoint()
	{
		Color = NLMISC::CRGBA::White;
		x = 0;
		y = 0;
	}
};

/**
 * Display of map and landmarks.
 *
 * There are several coordinate systems :
 *
 * - World coordinates : as usual
 * - Map coordinates : In the [0, 1] range. [0, 0] is the upper left of map, and [1, 1] is the lower right.
 *   the corners of the map in world coordinates are given by their zone names (in the continent sheet)
 * - Screen coordinates :  in pixels
 * - Window coordinates : the same as screen coordinate, but relative to that CInterfaceGroup
 *
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2003
 */

class CGroupMap : public CInterfaceGroup
{
public:

	// external element to be displayed on the map
	struct IDeco
	{
		/** called when the element is added to the map. If the deco is an interface element, it could
		  * add itself to this group child
		  */
		virtual void onAdd(CGroupMap &/* owner */) {}
		virtual void onRemove(CGroupMap &/* owner */) {}
		virtual void onPreRender(CGroupMap &/* owner */) {}
		/** Called when the map has been scrolled or scaled. The deco should update its pos here
		  *
		  */
		virtual void onUpdate(CGroupMap &/* owner */) {}
	};

	REFLECT_EXPORT_START(CGroupMap, CInterfaceGroup)
		REFLECT_STRING("continent", getContinentName, dummySet);
		REFLECT_STRING("texture", getMapTexture, dummySet);
		REFLECT_LUA_METHOD("isIsland", luaIsIsland);
		REFLECT_LUA_METHOD("reload", luaReload);
		REFLECT_STRING("map", getMap, setMap);
	REFLECT_EXPORT_END

	int luaReload(CLuaState &ls);
	int luaIsIsland(CLuaState &ls);

public:
	CGroupMap(const TCtorParam &param);
	virtual ~CGroupMap();
	// Add a decoration to the map. The map will call the 'onAdd' method. When this object is destroyed, it will call the 'onRemove' method
	void addDeco(IDeco *deco);
	// Remove a decoration from the map. This will also call the 'onRemove' method. It is up to the owner to delete it.
	void removeDeco(IDeco *deco);

	virtual void setActive (bool state);
	virtual void updateCoords();
	virtual	void checkCoords();
	virtual void draw ();
	virtual bool handleEvent (const NLGUI::CEventDescriptor &event);
	virtual bool parse(xmlNodePtr cur, CInterfaceGroup * parentGroup);
	virtual bool getCtrlsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CCtrlBase*> &vICL);

	// Name of the map as filled in the ryzom.world file
	std::string getMap() const;
	void setMap(const std::string &mapName);
	void setMap(SMap *map);

	// return current continent
	std::string getContinentName() const;
	// return currently displayed map texture
	std::string getMapTexture() const;

	// reload current map texture
	void reload();

	// pan the map of the given number of pixels
	void pan(sint32 dx, sint32 dy);

	// center the map on the player
	void centerOnPlayer();
	// center current map on world coords (if not out of map bounds)
	void centerOnWorldPos(const NLMISC::CVector2f &worldPos);

	void setPlayerPos(const NLMISC::CVector2f &p) { _PlayerPos = p; }
	NLMISC::CVector2f getPlayerPos() const { return _PlayerPos; }
	// test if player is currently panning the map
	bool isPanning() const { return _Panning; }
	/** Change the scale. It will be clipped to the max possible value
	  * The center of the scale transformation must be given in the map coordinates.
	  */
	void				setScale(float newScale, const NLMISC::CVector2f &center);
	/** Change the scale. It will be clipped to the max possible value
	  * The center of the scale is the center of current window
	  */
	void				setScale(float newScale);
	//
	float				getScale() const { return _UserScale; }
	/// add a user landmark (returns a pointer on its button).Coordinate are in the current map (not world coordinates)
	CCtrlButton			*addUserLandMark(const NLMISC::CVector2f &pos, const ucstring &title, const CUserLandMark::EUserLandMarkType lmType);
	/// return current continent landmark by its index and type
	CCtrlButton*		getLandmarkCtrl(const std::string &lmType, uint lmIndex) const;
	// remove a user landmark from a pointer on its button
	void				removeUserLandMark(CCtrlButton *button);
	// update a user landmark from a pointer on its button
	void				updateUserLandMark(CCtrlButton *button, const ucstring &newName, const CUserLandMark::EUserLandMarkType lmType);
	// get a user landmark from a pointer on its button
	CUserLandMark			getUserLandMark(CCtrlButton *button) const;
	// get pos on the map of the last right click (in map coords)
	NLMISC::CVector2f		getRightClickLastPos() const { return _RightClickLastPos; }
	// get number of user landmarks
	uint				getNumUserLandMarks() const;
	// get the LandMarksOptions for a given landmark index
	CLandMarkOptions		getUserLandMarkOptions(uint32 lmindex) const;
	// target the given landmark
	void				targetLandmark(CCtrlButton *lm);
	void				targetLandmarkResult(uint32 index);
	// search matching landmark and target it. return true if landmark was targeted
	bool				targetLandmarkByName(const ucstring &search, bool startsWith) const;
	// get the world position of a landmark or return vector Null if not found
	void				getLandmarkPosition(const CCtrlButton *lm, NLMISC::CVector2f &worldPos);

	//Remove and re-create UserLandMarks
	void removeUserLandMarks();
	void updateUserLandMarks();
	void addUserLandMark(const NLMISC::CVector2f &pos, const ucstring &title, NLMISC::CRGBA color);
	void addUserRespawnPoint(const NLMISC::CVector2f &pos);
	void delArkPoints();


	// set landmarks visibility based text query
	void setLandmarkFilter(const std::string &s);

	// set the selection axis pos & visibility
	void				setSelectionAxis(bool active, const NLMISC::CVector2f &worldPos = NLMISC::CVector2f::Null);

	// convert a pos in world to a pos in the window, snapped to the best pixel (-> all elements jump to the next pixel at the same time when the map is panned,
	// avoiding annoying flickering)
	void worldToWindowSnapped(sint32 &px, sint32 &py, const NLMISC::CVector2f &src) const;
	void worldToWindow(NLMISC::CVector2f &dest, const NLMISC::CVector2f &src) const;
	void mapToWindowSnapped(sint32 &px, sint32 &py, const NLMISC::CVector2f &src) const;
	// convert a pos in world to a pos in the map (in the [0, 1] range)
	void worldToMap(NLMISC::CVector2f &dest, const NLMISC::CVector2f &src) const;
	// convert a pos in world to a pos in map (coords are in [0, 1] in the map)
	void mapToWorld(NLMISC::CVector2f &dest, const NLMISC::CVector2f &src) const;
	// convert a pos in window in a pos in screen
	void windowToScreen(sint32 &destX, sint32 &destY, sint32 srcX, sint32 srcY) const;
	// convert a pos in the map to a pos on screen
	void mapToScreen(sint32 &px, sint32 &py, const NLMISC::CVector2f &src) const;
	//
	// convert a pos in the map to a pos relative to the current window (int result)
	void mapToWindow(sint32 &px, sint32 &py, const NLMISC::CVector2f &src) const;
	void mapToWindow(NLMISC::CVector2f &dest, const NLMISC::CVector2f &src) const;
	// convert a pos on screen to a pos in map
	void screenToMap(NLMISC::CVector2f &dest, sint32 px, sint32 py) const;
	// convert a pos on window to a pos in map
	void windowToMap(NLMISC::CVector2f &dest, sint32 px, sint32 py) const;
	float getMeterPerPixel() const { return _MeterPerPixel; }

	//
	const NLMISC::CVector2f &getVisibleWorldMin() const { return _VisibleWorldMin; }
	const NLMISC::CVector2f &getVisibleWorldMax() const { return _VisibleWorldMax; }

	// compute pos of displayed map relative to this group (may be equal are smaller than this group extent
	// depending on the scale.
	void computeMapRectInsideGroup(sint32 &x, sint32 &y, sint32 &w, sint32 &h) const;


	// From CInterfaceElement : scale and offset must persist between virtual desktops
	virtual bool wantSerialConfig() const { return true; }
	// From CInterfaceElement
	virtual void serialConfig(NLMISC::IStream &f);


	// Server set all valid respawn points
	void addRespawnPoints(const CRespawnPointsMsg &rpm);

	// add Ark landscape point
	void addArkPoint(const CArkPoint &point);

	std::string getArkPowoMode() const { return _ArkPowoMode; }
	void setArkPowoMode(const std::string &mode)  { _ArkPowoMode = mode; }
	std::string getArkPowoMapMenu() const { return _ArkPowoMapMenu; }
	void setArkPowoMapMenu(const std::string &menu)  { _ArkPowoMapMenu = menu; }

	bool isInDeathMode() { return _MapMode == MapMode_Death; }

	sint32 getRespawnSelected() const;
	void setRespawnSelected(sint32 nSpawnPointIndex);

	SMap *getCurMap() { return _CurMap; }
	SMap *getParentMap(SMap *map);

	const NLMISC::CVector2f &getWorldOffset() const { return _WorldOffset; }


	bool isIsland() const { return _IsIsland; }

	void updateClosestLandMarkMenu(const std::string &menu, const NLMISC::CVector2f &pos) const;

	// show/hide all user landmarks
	void setUserLandMarkVisible(bool state);
	bool getUserLandMarkVisible() const { return _UserLandMarkVisible; };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
		// A non rectangular button to click on zone of the map
		class CPolyButton : public CCtrlBase
		{
		public:
			CPolyButton();
			virtual bool handleEvent (const NLGUI::CEventDescriptor &event);
			virtual void updateCoords();
			virtual void draw () {}
			void drawPolyButton();
			bool build(const NLLIGO::CPrimZone &concavePoly, CGroupMap *pMap, const std::string &sID);
			bool contains(const NLMISC::CVector2f &pos);
		public:
			NLLIGO::CPrimZone Zone;
			NLLIGO::CPrimZone ZoneReal;
			std::vector<NLMISC::CPolygon2D> Polys;		// Vertices in map pos
			std::vector<NLMISC::CPolygon2D> PolysReal;	// Vertices in screen pos
			CGroupMap *Map;
			std::string ID;
			static NL3D::UMaterial LineMat;
		};
		typedef std::vector<CPolyButton*> TPolyButtonVect;
		// a button representing a location on the map
		class CLandMarkButton : public CCtrlButton
		{
			public:
				NLMISC::CVector2f Pos;
				CContLandMark::TContLMType Type;
				bool					   HandleEvents;
				bool					   SearchMatch;
			public:
				virtual bool handleEvent (const NLGUI::CEventDescriptor& event)
				{
					if (!HandleEvents) return false;
					return CCtrlButton::handleEvent(event);
				}
				virtual	bool		isCapturable() const
				{
					return HandleEvents;
				}
				virtual bool	wantInstantContextHelp() const { return true; }	// from CCtrlBase : avoid delay when the mouse is over
				CLandMarkButton(const TCtorParam &param)
					: CCtrlButton(param)
				{
					Type = CContLandMark::Unknown;
					Pos.set(0.f, 0.f);
					HandleEvents = true;
					SearchMatch = false;
				}
		};
		typedef std::vector<CLandMarkButton*> TLandMarkButtonVect;

		// Text in the map
		class CLandMarkText : public CViewText
		{
			public:
				NLMISC::CVector2f Pos;
				CContLandMark::TContLMType Type;
				bool SearchMatch;

				CLandMarkText(const TCtorParam &param)
					: CViewText(param)
				{
					Type = CContLandMark::Unknown;
					Pos.set(0.f, 0.f);
					SearchMatch = false;
				}
		};
		typedef std::vector<CLandMarkText*> TLandMarkTextVect;

		float getActualMaxScale() const;
private:

	///////////////////////
	// MULTIMAP HANDLING //
	///////////////////////

		// Logical information to display the map
		CWorldSheet			*_WorldSheet;
		SMap				*_CurMap;
		CContinent			*_CurContinent;	// the last continent for which the map was displayed (can be NULL if world)
		NLMISC::CVector2f	_MapMinCorner; // In world coordinates
		NLMISC::CVector2f	_MapMaxCorner;
		std::string			_MapTexture; // currently displayed map texture

		bool				_IsIsland;  // true if current map is an island (island bitmap need not to be raised to the next
									  // power of 2

		TPolyButtonVect		_PolyButtons;

	//////////////////////////////
	// MAP & PLAYER POS TEXTURE //
	//////////////////////////////

		std::string			_PlayerPosTexName;
		// min height of the window
		sint32				_MinH;
		//sint32			_MinW;
		// offset of the map view (offset is in uv coords)
		NLMISC::CVector2f	_Offset;
		float				_UserScale;			// user wanted scale
		float				_Scale;				// actual scale for drawing
		float				_ScaleMax;
		float				_ScaleMaxR2;
		float				_MeterPerPixel;
		//
		float				_WorldToMapDeltaX;
		float				_WorldToMapDeltaY;
		//
		NLMISC::CVector2f   _VisibleWorldMin;
		NLMISC::CVector2f   _VisibleWorldMax;
		// continent map material
		NL3D::UMaterial		_MapMaterial;
		// continent map texture
		NL3D::UTextureFile	*_MapTF;
		// player pos map material
		NL3D::UMaterial		_PlayerPosMaterial;
		// player pos map texture
		NL3D::UTextureFile	*_PlayerPosTF;
		NL3D::UMaterial		_FrustumMaterial;

		// continent map dimensions
		uint32				_MapTexW;			// map texture width in pixels
		uint32				_MapTexH;			// map texture height in pixels
		uint32				_PlayerPosTexW;		// playerpos texture width in pixels
		uint32				_PlayerPosTexH;		// playerpos texture height in pixels
		float				_URatio;			// == _MapTexW / nextPowerOf2(_MapTexW)
		float				_VRatio;			// == _MapTexH / nextPowerOf2(_MapTexH)
		bool				_MapLoadFailure;	// load failure for the map
		bool				_PlayerPosLoadFailure; // load failure for the player pos sprite

		NLMISC::CVector2f	_PlayerPos;			// player pos ranging from 0.f to 1.f in the map
		NLMISC::CVector2f	_OldPlayerPos;		// old player pos in world

		// position of map relative to its parent. Maybe 0 unless the map is centred
		sint32				_MapX;
		sint32				_MapY;
		sint32				_MapW;
		sint32				_MapH;

		std::string			_ArkPowoMode;
		std::string			_ArkPowoMapMenu;
		NLMISC::CRGBA		_FrustumViewColor;
		NLMISC::CRGBA		_FrustumViewColorOver;
		float				_FrustumOverBlendFactor;
		uint				_FrustumViewBlendTimeInMs;

		// selection axis
		CViewBitmap			*_SelectionAxisH;
		CViewBitmap			*_SelectionAxisV;



	////////////
	// EVENTS //
	////////////

		NLMISC::CVector2f	_RightClickLastPos;
		bool				_Panning;					// does the user currently 'pan' the map ?
		bool				_HasMoved;
		sint64				_PanStartDateInMs;
		sint64				_DeltaTimeBeforePanInMs;
		uint				_DeltaPosBeforePan;
		// Panning
		sint32				_StartXForPaning;			// start pos for panning (in map)
		sint32				_StartYForPaning;
		NLMISC::CVector2f	_StartWorldOffsetForPaning;	// World Offset Panning
		NLMISC::CVector2f	_WorldOffset;

	///////////////
	// LANDMARKS //
	///////////////

		// if false, user landmarks are not drawn
		bool				_UserLandMarkVisible;

		// landmarks of continent
		TLandMarkButtonVect	_ContinentLM;
		TLandMarkTextVect	_ContinentText;
		// landmarks from user
		TLandMarkButtonVect	_UserLM;
		// landmarks for mission (one for each db entry)
		TLandMarkButtonVect	_MissionLM;
		// landmark for target
		CLandMarkButton		*_TargetLM;
		// landmark for home (user flat)
		CLandMarkButton		*_HomeLM;
		// landmark for animals
		TLandMarkButtonVect	_AnimalLM;
		// landmark for teammates
		TLandMarkButtonVect	_TeammateLM;

		//
		CLandMarkOptions	_ContinentLMOptions;
		CLandMarkOptions	_MissionLMOptions;
		CLandMarkOptions	_UserLMOptions;
		CLandMarkOptions    _TargetLMOptions;
		CLandMarkOptions    _HomeLMOptions;
		CLandMarkOptions    _AnimalLMOptions;
		CLandMarkOptions    _AnimalStableLMOptions;
		CLandMarkOptions    _AnimalDeadLMOptions;
		CLandMarkOptions    _TeammateLMOptions;

		//
		// last texts id for missions targets
		std::vector<sint32> _MissionTargetTextIDs;
		// have the texts been received for mission targets ?
		std::vector<bool>	_MissionTargetTextReceived;
		// ptr on db leaf for coordinates of special landmarks
		NLMISC::CCDBNodeLeaf		*_TargetPos;
		NLMISC::CCDBNodeLeaf		*_HomePos;
		// Animals State for landMarks
		std::vector<NLMISC::CSmartPtr<CAnimalPositionState> >	_AnimalPosStates;
		// Teammate State for landMarks
		std::vector<NLMISC::CSmartPtr<CTeammatePositionState> >	_TeammatePosStates;
		// Mission State for landMarks
		std::vector<NLMISC::CSmartPtr<CNamedEntityPositionState> >	_MissionPosStates;

		// ui id  of compass for targetting
		std::string		_CompassId;

		// user decorations
		typedef std::set<IDeco *> TDecos;
		TDecos _Decos;

		// filter keywords
		std::vector<ucstring> _LandmarkFilter;
		struct SMatchedLandmark
		{
			SMatchedLandmark(const NLMISC::CVector2f pos, const ucstring &title, CLandMarkOptions opts)
				: Pos(pos), Title(title), Options(opts)
			{}
			NLMISC::CVector2f Pos;
			ucstring Title;

			CLandMarkOptions Options;
		};
		std::vector<SMatchedLandmark> _MatchedLandmarks;

	//////////////////////
	// Respawn handling //
	// //////////////// //

		enum TMapMode
		{
			MapMode_Normal = 0,
			MapMode_Death,
			MapMode_SpawnSquad
		};

		TMapMode			_MapMode;
		std::vector<CArkPoint> _ArkPoints;
		CLandMarkOptions	_RespawnLMOptions;
		// landmark for respawn
		TLandMarkButtonVect	_RespawnLM;
		sint32				_RespawnSelected;
		CViewBitmap			*_RespawnSelectedBitmap;
		std::string			_RespawnButton;
		// Positions are coming from server
		std::vector<CRespawnPointsMsg::SRespawnPoint> _RespawnPos;
		bool			_RespawnPosReseted;
		CCtrlQuad			*_FrustumView; // frustum on map for R2 editor

	// r2 islands
	std::vector<SMap>	_Islands;

	// guard against recursive calls
	bool				_LuaLoadMapEntered;

private:
	void loadPlayerPos();
	void loadMap();
	void unloadMap();
	//
	void updateContinentInfo();
	/** update a list of landmarks
	  * This update their position on the map depending on offset and scale
	  */
	void updateLandMarkList(TLandMarkButtonVect &lm);
	void updateLandMarkTextList(TLandMarkTextVect &lm);
	void updateMatchedLandmarks();
	//
	void removeLandMarks(TLandMarkButtonVect &lm);
	/** create landmarks from the continent (and remove previous ones)
	  * this includes fixed and user landmarks
	  */
	void createContinentLandMarks();
	void createLMWidgets(const std::vector<CContLandMark> &lms);

	// add a landmark in a list
	void addLandMark(TLandMarkButtonVect &destList, const NLMISC::CVector2f &pos, const ucstring &title, const CLandMarkOptions &options);
	// Create a landmark button, but do not add it to this group
	CLandMarkButton *createLandMarkButton(const CLandMarkOptions &options);
	// Create a Ark landmark button, but do not add it to this group
	CLandMarkButton *createArkPointButton(const CArkPoint &point);

	// update a landmark button
	void updateLandMarkButton(CLandMarkButton *lmb, const CLandMarkOptions &options);

	// Test title against landmark filter
	bool filterLandmark(const ucstring &title) const;
	bool filterLandmark(const ucstring &title, const std::vector<ucstring> filter, bool startsWith = false) const;

	// return closest landmark which matches (case insensitive) search string
	// center position must be in world coordindates
	CLandMarkButton* findClosestLandmark(const NLMISC::CVector2f &center, const ucstring &search, bool startsWith, const TLandMarkButtonVect &landmarks, float &closest) const;
	CLandMarkText* findClosestLandmark(const NLMISC::CVector2f &center, const ucstring &search, bool startsWith, const TLandMarkTextVect &landmarks, float &closest) const;

	// update the scale depending on the window size and the user scale
	void updateScale();

	// compute real scale from user scale (this takes in account the size of the window)
	float computeRealScaleFromUserScale(float userScale) const;

	// compute the user scale needed to reach the given real scale on screen (this takes in account the size of the window)
	float computeUserScaleFromRealScale(float realScale) const;

	// eval map offset from its parent depending on scale
	void evalMapOffset(float userScale, float &scale, sint32 &x, sint32 &y) const;

	// compute _Offset from the _WorldOffset. Then clamp so that region outside the map can't be seen
	void computeOffsets(const NLMISC::CVector2f &centerPos);
	void computeOffsets();

	void updatePlayerPos();
	// clamp offsets & change window size if necessary
	//void fitWindow();
	//
	void updateButtonPos(CLandMarkButton &dest) const;
	// Update a landmark position from position given from the db as two int32's (before they're divided by 1000)
	void updateLMPosFromDBPos(CLandMarkButton *dest, sint32 x, sint32 y);
	// compute uv rect for current map (not including cropping)
	void computeUVRect(float &minU, float &minV, float &maxU, float &maxV) const;

	void updateSelectionAxisSize();
	CViewBitmap *newSelectionAxis(NLMISC::CRGBA color);
	void computeFrustumQuad(NLMISC::CQuad &fruQuad) const;
};

#endif
