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



#include "stdpch.h"
//
#include "group_map.h"
#include "interface_manager.h"
#include "../continent_manager.h"
#include "../continent.h"
#include "../zone_util.h"
#include "../user_entity.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/dbgroup_combo_box.h"
#include "../string_manager_client.h"
#include "nel/gui/group_container.h"
#include "nel/gui/action_handler.h"
#include "../dummy_progress.h"
#include "group_compas.h"
#include "../connection.h"
#include "../net_manager.h"
#include "people_interraction.h" // for MaxNumPeopleInTeam
#include "../sheet_manager.h" // for MaxNumPeopleInTeam
#include "../global.h"
#include "nel/gui/ctrl_quad.h"
//
#include "nel/misc/xml_auto_ptr.h"
#include "game_share/mission_desc.h"
#include "game_share/inventories.h"
#include "game_share/animal_type.h"
//
#include "nel/3d/u_material.h"
#include "nel/3d/u_texture.h"
#include "nel/3d/u_scene.h"
#include "../entities.h"
//
#include "nel/misc/vector_2f.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/uv.h"
#include "nel/misc/i18n.h"
#include "../r2/editor.h"
//
#include "game_share/scenario_entry_points.h"

#include <list>

extern CContinentManager ContinentMngr;
extern NL3D::UDriver *Driver;
extern NL3D::UScene *Scene;
extern CEntityManager EntitiesMngr;

using namespace NLMISC;
using NLMISC::CUV;
using NLMISC::CI18N;
using NLMISC::CRGBA;
using NLMISC::CBitMemStream;
using namespace STRING_MANAGER;

// /////// //
// STATICS //
// /////// //
static CGroupMap   *LastClickedMap = NULL;
static CCtrlButton *LastSelectedLandMark = NULL;
static bool		   UseUserPositionForLandMark = false;
static const char  *WIN_LANDMARK_NAME="ui:interface:enter_landmark_name";

// Loaded position of user landmark types
static std::vector<uint>  LoadedPosition;

////////////
// GLOBAL //
////////////
NLMISC::CRGBA	CUserLandMark::_LandMarksColor[CUserLandMark::UserLandMarkTypeCount];



const uint32 ISLAND_PIXEL_PER_METER = 2;

static void setupFromZoom(CViewBase *pVB, CContLandMark::TContLMType t, float fMeterPerPixel);

// popup the landmark name dialog

static void popupLandMarkNameDialog()
{
	// pop the rename dialog
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(WIN_LANDMARK_NAME));
	if (!gc) return;

	gc->setActive(true);
	gc->updateCoords();
	gc->center();
	CWidgetManager::getInstance()->setTopWindow(gc);
	gc->enableBlink(1);
	
	CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(gc->getGroup("eb"));
	if (!eb) return; 
	
	// Load ComboBox for Landmarks & sort entries
	CDBGroupComboBox *cb = dynamic_cast<CDBGroupComboBox *>(gc->getGroup("landmarktypes"));
	cb->sortText();

	if (LastSelectedLandMark)
	{
		CGroupMap *map = dynamic_cast<CGroupMap *>(LastSelectedLandMark->getParent());
		if (!map) return;
		
		const CUserLandMark userLM = map->getUserLandMark(LastSelectedLandMark);

		NLGUI::CDBManager::getInstance()->getDbProp( "UI:TEMP:LANDMARKTYPE" )->setValue8(cb->getTextPos(userLM.Type));
		eb->setInputString(userLM.Title);
	}
	else
	{
		NLGUI::CDBManager::getInstance()->getDbProp( "UI:TEMP:LANDMARKTYPE" )->setValue8(cb->getTextPos(CUserLandMark::Misc));
		eb->setInputString(ucstring());
	}

	CWidgetManager::getInstance()->setCaptureKeyboard(eb);
	eb->setSelectionAll();
}

static void closeLandMarkNameDialog()
{
	LoadedPosition.clear();
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(WIN_LANDMARK_NAME));
	if (!gc) return;
	gc->setActive(false);
}

//============================================================================================================
// CPolyButton
//============================================================================================================

NL3D::UMaterial CGroupMap::CPolyButton::LineMat;

//============================================================================================================
CGroupMap::CPolyButton::CPolyButton()
: CCtrlBase(TCtorParam())
{
	if (Driver != NULL)
	{
		// create material for displaying the button
		if (LineMat.empty())
		{
			// create material for the world map
			LineMat = Driver->createMaterial();
			if (!LineMat.empty())
			{
				LineMat.initUnlit();
				LineMat.setSrcBlend(NL3D::UMaterial::srcalpha);
				LineMat.setDstBlend(NL3D::UMaterial::invsrcalpha);
				LineMat.setBlend(true);
				LineMat.setZFunc(NL3D::UMaterial::always);
				LineMat.setZWrite(false);
				LineMat.setColor(CRGBA::Red);
			}
		}
	}
}


//============================================================================================================
bool CGroupMap::CPolyButton::handleEvent (const NLGUI::CEventDescriptor &event)
{
	if (CCtrlBase::handleEvent(event)) return true;
	if (event.getType() == NLGUI::CEventDescriptor::mouse)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
		{
			if (CWidgetManager::getInstance()->getCapturePointerLeft() != this)
				return false;

			// Set the map !!!
			if (contains(CVector2f((float)eventDesc.getX(), (float)eventDesc.getY())))
			{
				bool bFound = false;
				uint32 i;
				for (i = 0; i < Map->getCurMap()->Children.size(); ++i)
				{
					if (ID == Map->getCurMap()->Children[i].ZoneName)
					{
						bFound = true;
						break;
					}
				}

				CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
				if (bFound)
					Map->setMap(Map->getCurMap()->Children[i].Name);
				return true;
			}
		}

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown)
		{
			if (contains(CVector2f((float)eventDesc.getX(), (float)eventDesc.getY())))
			{
				CWidgetManager::getInstance()->setCapturePointerLeft(this);
				return true;
			}
		}

	}
	return false;
}

//============================================================================================================
void CGroupMap::CPolyButton::updateCoords()
{
	uint32 i, p;
	PolysReal.resize(Polys.size());
	for (p = 0; p < Polys.size(); ++p)
	{
		PolysReal[p].Vertices.resize(Polys[p].Vertices.size());
		for (i = 0; i < Polys[p].Vertices.size(); ++i)
		{
			sint32 x, y;
			CVector2f mapCoords;
			Map->worldToMap(mapCoords, Polys[p].Vertices[i]);
			Map->mapToScreen(x, y, mapCoords);
			PolysReal[p].Vertices[i] = CVector2f((float)x,(float)y);
		}
	}

	ZoneReal.VPoints.resize(Zone.VPoints.size());
	for (i = 0; i < Zone.VPoints.size(); ++i)
	{
		sint32 x, y;
		CVector2f mapCoords;
		Map->worldToMap(mapCoords, Zone.VPoints[i]);
		Map->mapToScreen(x, y, mapCoords);
		ZoneReal.VPoints[i] = NLLIGO::CPrimVector(CVector2f((float)x,(float)y));
	}
}

//============================================================================================================
float CGroupMap::getActualMaxScale() const
{
	return ClientCfg.R2EDEnabled ? _ScaleMaxR2 : _ScaleMax;
}

//============================================================================================================
void CGroupMap::CPolyButton::drawPolyButton()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();

	float oow, ooh;
	rVR.getScreenOOSize(oow, ooh);

	bool bOver = false;
	const std::vector<CCtrlBase*> &rCUP = CWidgetManager::getInstance()->getCtrlsUnderPointer();
	for (uint32 i = 0; i < rCUP.size(); ++i)
		if (rCUP[i] == this)
		{
			bOver = true;
			break;
		}

/*	// Wireframe !!!
	for (uint32 p = 0; p < PolysReal.size(); ++p)
	{
		for (uint32 i = 0; i < PolysReal[p].Vertices.size(); ++i)
		{
			NLMISC::CLine l;
			l.V0 = PolysReal[p].Vertices[i];
			if (i != (PolysReal[p].Vertices.size()-1))
				l.V1 = PolysReal[p].Vertices[i+1];
			else
				l.V1 = PolysReal[p].Vertices[0];

			l.V0.x *= oow;
			l.V0.y *= ooh;
			l.V1.x *= oow;
			l.V1.y *= ooh;

			Driver->drawLine(l, LineMat);
		}
	}
*/

	if (bOver)
		LineMat.setColor(CRGBA(255,255,255,64)); // LineMat.setColor(CRGBA(165,38,6,128));
	else
		return; // LineMat.setColor(CRGBA(255,255,255,64));

	for (uint32 p = 0; p < PolysReal.size(); ++p)
	{
		for (uint32 i = 0; i < (PolysReal[p].Vertices.size()-2); ++i)
		{
			NLMISC::CTriangle t;
			t.V0 = PolysReal[p].Vertices[0];
			t.V1 = PolysReal[p].Vertices[i+1];
			t.V2 = PolysReal[p].Vertices[i+2];

			t.V0.x *= oow;
			t.V0.y *= ooh;
			t.V1.x *= oow;
			t.V1.y *= ooh;
			t.V2.x *= oow;
			t.V2.y *= ooh;

			Driver->drawTriangle(t, LineMat);
		}
	}
}

//============================================================================================================
bool CGroupMap::CPolyButton::build(const NLLIGO::CPrimZone &concavePoly, CGroupMap *pMap, const string &sID)
{
	static int gStat = 0;
	_Id = pMap->getId() + ":polybut" + toString(gStat++);

	Map = pMap;
	ID = sID;
	Zone = concavePoly;

	CPolygon p;
	for (uint32 i = 0; i < concavePoly.VPoints.size(); ++i)
		p.Vertices.push_back(CVector(concavePoly.VPoints[i].x, concavePoly.VPoints[i].y, 0));

	std::list<CPolygon> outputPolygons;
	CMatrix id;
	id.identity();
	if (p.toConvexPolygons (outputPolygons, id))
	{
		std::list<CPolygon>::iterator it = outputPolygons.begin();
		while (it != outputPolygons.end())
		{
			CPolygon2D poly;
			for (uint32 i = 0; i < it->Vertices.size(); ++i)
				poly.Vertices.push_back(CVector2f(it->Vertices[i].x, it->Vertices[i].y));
			Polys.push_back(poly);
			it++;
		}
	}
	else
	{
		nlwarning("cannot convert to convex poly");
		return false;
	}
	return true;
}

//============================================================================================================
bool CGroupMap::CPolyButton::contains(const NLMISC::CVector2f &pos)
{
/* // Not precise
	for (uint32 p = 0; p < PolysReal.size(); ++p)
	{
		if (PolysReal[p].contains(pos))
			return true;
	}
	return false;
*/
	return ZoneReal.contains(pos);
}

//============================================================================================================
// CGroupMap
//============================================================================================================

//============================================================================================================

NLMISC_REGISTER_OBJECT(CViewBase, CGroupMap, std::string, "map");

CGroupMap::CGroupMap(const TCtorParam &param)
: CInterfaceGroup(param)
{
	_Offset.x = 0.f;
	_Offset.y = 0.f;
	_WorldOffset.x = 0.f;
	_WorldOffset.y = 0.f;
	_Scale = 1.f;
	_UserScale = 1.f;
	_MinH = 50;
	_MaxH = 2000;
	//_MinW = 50;
	_MapTF = NULL;
	_PlayerPosMaterial = NULL;
	_PlayerPosTF = NULL;
	_MapTexW = 0;
	_MapTexH = 0;
	_PlayerPosTexW = 0;
	_PlayerPosTexH = 0;
	_MapLoadFailure = false;
	_PlayerPosLoadFailure = false;
	_WorldSheet = dynamic_cast<CWorldSheet*>(SheetMngr.get(CSheetId("ryzom.world")));
	_CurMap = NULL;
	_CurContinent = NULL;
	_Panning = false;
	_HasMoved = false;
	_RightClickLastPos.set(0.f, 0.f);
	// make room for mission targets
	_MissionLM.resize(2 * MAX_NUM_MISSIONS * MAX_NUM_MISSION_TARGETS, 0);
	_MissionTargetTextIDs.resize(2 * MAX_NUM_MISSIONS * MAX_NUM_MISSION_TARGETS, 0);
	_MissionTargetTextReceived.resize(2 * MAX_NUM_MISSIONS * MAX_NUM_MISSION_TARGETS, false);
	_OldPlayerPos.set(0.f, 0.f);
	_PlayerPos.set(0.f, 0.f);
	//
	_TargetLM = NULL;
	_HomeLM = NULL;
	//
	_ScaleMax = 8.f;
	_ScaleMaxR2 = 8.f;
	//
	_TargetPos = NULL;
	_HomePos = NULL;
	//
	_MapX = 0;
	_MapY = 0;
	//
	_MapMode = MapMode_Normal;
	_RespawnSelected = 0;
	_RespawnSelectedBitmap = NULL;
	//
	_WorldToMapDeltaX = 0.f;
	_WorldToMapDeltaY = 0.f;
	//
	_VisibleWorldMin.set(0.f, 0.f);
	_VisibleWorldMax.set(0.f, 0.f);
	//
	_MeterPerPixel = 0.f;
	_FrustumView = NULL;
	_FrustumViewColor = CRGBA(255, 255, 255, 255);
	_FrustumViewColorOver = CRGBA(255, 255, 255, 127);
	_FrustumOverBlendFactor = 0.f;
	_FrustumViewBlendTimeInMs = 300;
	_SelectionAxisH = NULL;
	_SelectionAxisV = NULL;
	//
	_IsIsland = false;
	//
	_PanStartDateInMs = 0;
	_DeltaTimeBeforePanInMs = 0;
	_DeltaPosBeforePan = 0;
}

//============================================================================================================
CGroupMap::~CGroupMap()
{
	if (Driver)
	{
		if (!_MapMaterial.empty())
			Driver->deleteMaterial(_MapMaterial);
		if (!_PlayerPosMaterial.empty())
			Driver->deleteMaterial(_PlayerPosMaterial);
	}
	for(TDecos::iterator it = _Decos.begin(); it != _Decos.end(); ++it)
	{
		(*it)->onRemove(*this);
	}
}


//============================================================================================================
void CGroupMap::addDeco(IDeco *deco)
{
	if (!deco) return;
	if (_Decos.count(deco))
	{
		nlwarning("Deco added twice.");
		return;
	}
	_Decos.insert(deco);
	deco->onAdd(*this);
	deco->onUpdate(*this);
}

//============================================================================================================
void CGroupMap::removeDeco(IDeco *deco)
{
	if (!deco) return;
	TDecos::iterator it = _Decos.find(deco);
	if (it != _Decos.end())
	{
		deco->onRemove(*this);
		_Decos.erase(it);
	}
	else
	{
		nlwarning("Deco not found");
	}
}

//============================================================================================================
/** Load infos about a landmark
  */
static void loadLandmarkInfo(xmlNodePtr cur, const std::string &prefix, CLandMarkOptions &dest)
{
	CXMLAutoPtr ptr;
	ptr = (char*) xmlGetProp( cur, (xmlChar*)(prefix + "_landmark_tex_normal").c_str());
	if (ptr) dest.LandMarkTexNormal = (const char *) ptr;
	ptr = (char*) xmlGetProp( cur, (xmlChar*)(prefix + "_landmark_tex_over").c_str());
	if (ptr) dest.LandMarkTexOver = (const char *) ptr;
	ptr = (char*) xmlGetProp( cur, (xmlChar*)(prefix + "_landmark_tex_pushed").c_str());
	if (ptr) dest.LandMarkTexPushed = (const char *) ptr;
	ptr = (char*) xmlGetProp( cur, (xmlChar*)(prefix + "_landmark_menu").c_str());
	if (ptr) dest.LandMarkMenu = (const char *) ptr;
	ptr = (char*) xmlGetProp( cur, (xmlChar*)(prefix + "_landmark_color_normal").c_str());
	if (ptr) dest.ColorNormal = CCtrlBase::convertColor(ptr);
	ptr = (char*) xmlGetProp( cur, (xmlChar*)(prefix + "_landmark_color_over").c_str());
	if (ptr) dest.ColorOver = CCtrlBase::convertColor(ptr);
	ptr = (char*) xmlGetProp( cur, (xmlChar*)(prefix + "_landmark_color_pushed").c_str());
	if (ptr) dest.ColorPushed = CCtrlBase::convertColor(ptr);
}


//============================================================================================================
CViewBitmap *CGroupMap::newSelectionAxis(NLMISC::CRGBA color)
{
	CViewBitmap *axis = new CViewBitmap(TCtorParam());
	axis->setActive(false);
	axis->setTexture("blank.tga");
	axis->setModulateGlobalColor(false);
	addView(axis);
	axis->setParent(this);
	axis->setColor(color);
	axis->setScale(true);
	return axis;
}

//============================================================================================================
bool CGroupMap::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	if (!CInterfaceGroup::parse(cur, parentGroup)) return false;
	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"min_h" ));
	if (ptr) fromString((const char*) ptr, _MinH);
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"max_h" );
	if (ptr) fromString((const char*)ptr, _MaxH);
	/*ptr = (char*) xmlGetProp( cur, (xmlChar*)"min_w" );
	if (ptr) fromString((const char*)ptr, _MinW);*/

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"map_mode" );
	if (ptr)
	{
		string sTmp = ptr;
		if (sTmp == "normal")
			_MapMode = MapMode_Normal;
		else if (sTmp == "death")
			_MapMode = MapMode_Death;
		else if (sTmp == "spawn_squad")
			_MapMode = MapMode_SpawnSquad;
	}

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"frustum_view_color" );
	if (ptr)
	{
		_FrustumViewColor = convertColor (ptr);
	}
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"frustum_view_color_over" );
	if (ptr)
	{
		_FrustumViewColorOver = convertColor (ptr);
	}
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"frustum_view_blend_time_in_ms" );
	if (ptr)
	{
		fromString((const char*)ptr, _FrustumViewBlendTimeInMs);
	}


	if (Driver)
	{
		if (_MapMaterial.empty())
		{
			// create material for the world map
			_MapMaterial = Driver->createMaterial();
			if (!_MapMaterial.empty())
			{
				_MapMaterial.initUnlit();
				_MapMaterial.setSrcBlend(NL3D::UMaterial::srcalpha);
				_MapMaterial.setDstBlend(NL3D::UMaterial::invsrcalpha);
				_MapMaterial.setBlend(true);
				_MapMaterial.setZFunc(NL3D::UMaterial::always);
				_MapMaterial.setZWrite(false);
				// Setup stage 0
				_MapMaterial.texEnvOpRGB(0, NL3D::UMaterial::Modulate);
				_MapMaterial.texEnvArg0RGB(0, NL3D::UMaterial::Texture, NL3D::UMaterial::SrcColor);
				_MapMaterial.texEnvArg1RGB(0, NL3D::UMaterial::Diffuse, NL3D::UMaterial::SrcColor);
				_MapMaterial.texEnvOpAlpha(0, NL3D::UMaterial::Modulate);
				_MapMaterial.texEnvArg0Alpha(0, NL3D::UMaterial::Texture, NL3D::UMaterial::SrcAlpha);
				_MapMaterial.texEnvArg1Alpha(0, NL3D::UMaterial::Diffuse, NL3D::UMaterial::SrcAlpha);
				// Setup stage 1
				_MapMaterial.texEnvOpRGB(1, NL3D::UMaterial::Modulate);
				_MapMaterial.texEnvArg0RGB(1, NL3D::UMaterial::Previous, NL3D::UMaterial::SrcColor);
				_MapMaterial.texEnvArg1RGB(1, NL3D::UMaterial::Texture, NL3D::UMaterial::SrcAlpha);
				_MapMaterial.texEnvOpAlpha(1, NL3D::UMaterial::Replace);
				_MapMaterial.texEnvArg0Alpha(1, NL3D::UMaterial::Previous, NL3D::UMaterial::SrcAlpha);
			}
		}
		if (_PlayerPosMaterial.empty())
		{
			// create material for the world map
			_PlayerPosMaterial = Driver->createMaterial();
			if (!_PlayerPosMaterial.empty())
			{
				_PlayerPosMaterial.initUnlit();
				_PlayerPosMaterial.setSrcBlend(NL3D::UMaterial::srcalpha);
				_PlayerPosMaterial.setDstBlend(NL3D::UMaterial::invsrcalpha);
				_PlayerPosMaterial.setBlend(true);
				_PlayerPosMaterial.setZFunc(NL3D::UMaterial::always);
				_PlayerPosMaterial.setZWrite(false);
			}
		}
	}
	// continent landmarks
	loadLandmarkInfo(cur, "continent", _ContinentLMOptions);
	loadLandmarkInfo(cur, "mission", _MissionLMOptions);
	loadLandmarkInfo(cur, "user",   _UserLMOptions);
	loadLandmarkInfo(cur, "target", _TargetLMOptions);
	// home aspect depends on race
	EGSPD::CPeople::TPeople people = EGSPD::CPeople::Fyros;
	if (PlayerSelectedSlot < CharacterSummaries.size())
	{
		people = CharacterSummaries[people].People;
	}
	switch(people)
	{
		case EGSPD::CPeople::Fyros: loadLandmarkInfo(cur, "home_fyros", _HomeLMOptions); break;
		case EGSPD::CPeople::Matis: loadLandmarkInfo(cur, "home_matis", _HomeLMOptions); break;
		case EGSPD::CPeople::Zorai: loadLandmarkInfo(cur, "home_zorai", _HomeLMOptions); break;
		case EGSPD::CPeople::Tryker: loadLandmarkInfo(cur, "home_tryker", _HomeLMOptions); break;
		default: break;
	}
	loadLandmarkInfo(cur, "respawn", _RespawnLMOptions);
	// animal landmark
	loadLandmarkInfo(cur, "animal", _AnimalLMOptions);
	// animal(in stable) landmark
	loadLandmarkInfo(cur, "animal_stable", _AnimalStableLMOptions);
	// animal(dead) landmark
	loadLandmarkInfo(cur, "animal_dead", _AnimalDeadLMOptions);
	// teammate landmark
	loadLandmarkInfo(cur, "teammate", _TeammateLMOptions);

	// player pos
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"player_pos_tex" );
	if (ptr) _PlayerPosTexName = (const char *) ptr;
	// name of compass for targeting
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"compass" );
	if (ptr) _CompassId = (const char *) ptr;

	if (_MapMode == MapMode_Normal)
	{
		// create home, target & respawn landmarks
		_TargetLM = createLandMarkButton(_TargetLMOptions);
		if (_TargetLM)
		{
			_TargetLM->setParent(this);
			addCtrl(_TargetLM);
		}
		_HomeLM = createLandMarkButton(_HomeLMOptions);
		if (_HomeLM)
		{
			_HomeLM->setParent(this);
			addCtrl(_HomeLM);
			_HomeLM->setDefaultContextHelp(NLMISC::CI18N::get("uiHome"));
		}

		// create animals Landmark: pack Animals.
		_AnimalLM.resize(MAX_INVENTORY_ANIMAL);
		for(uint i=0;i<_AnimalLM.size();i++)
		{
			_AnimalLM[i] = createLandMarkButton(_AnimalLMOptions);
			if(_AnimalLM[i])
			{
				_AnimalLM[i]->setParent(this);
				addCtrl(_AnimalLM[i]);
				_AnimalLM[i]->setDefaultContextHelp(NLMISC::CI18N::get(NLMISC::toString("uiPATitleMount%d", i+1)));
			}
		}

		// create teammates Landmark
		_TeammateLM.resize(MaxNumPeopleInTeam);
		for(uint i=0; i < MaxNumPeopleInTeam; i++)
		{
			_TeammateLM[i] = createLandMarkButton(_TeammateLMOptions);
			if(_TeammateLM[i])
			{
				_TeammateLM[i]->setParent(this);
				addCtrl(_TeammateLM[i]);
				_TeammateLM[i]->setDefaultContextHelp(NLMISC::CI18N::get(NLMISC::toString("uittLMTeam%d",i)));
			}
		}
	}

	//
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"scale_max" );
	if (ptr) fromString((const char *) ptr, _ScaleMax);
	//
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"scale_max_r2" );
	if (ptr) fromString((const char *) ptr, _ScaleMaxR2);
	//
	if ((_MapMode == MapMode_Death) || (_MapMode == MapMode_SpawnSquad))
	{
		string stmp = "w_answer_16_valid.tga";
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"respawn_selected" );
		if (ptr) stmp = (const char*)ptr;
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"respawn_button" );
		if (ptr) _RespawnButton = (const char*)ptr;
		_RespawnSelectedBitmap = new CViewBitmap(CViewBase::TCtorParam());
		static int statRP = 0;
		_RespawnSelectedBitmap->setId(getId()+":rp"+toString(statRP++));
		_RespawnSelectedBitmap->setTexture(stmp);
		_RespawnSelectedBitmap->setParent(this);
		_RespawnSelectedBitmap->setParentPosRef(Hotspot_MM);
		_RespawnSelectedBitmap->setPosRef(Hotspot_MM);
		addView(_RespawnSelectedBitmap);

		//CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(_RespawnButton));
		//if (pCB != NULL) pCB->setActive(false);
	}
	nlassert(!_FrustumView);
	CViewBase::TCtorParam param;
	_FrustumView = new CCtrlQuad( param );
	_FrustumView->setActive(false);
	addView(_FrustumView);
	_FrustumView->setParent(this);
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"frustum_view_texture" );

	const char* value;
	if (ptr)
	{
		value = ptr;
	}
	else
	{
		value = "r2_frustum.tga";
	}
	_FrustumView->setTexture(value);
	_FrustumView->setModulateGlobalColor(false);
	//
	nlassert(!_SelectionAxisH);
	nlassert(!_SelectionAxisV);
	CRGBA axisColor = CRGBA(0, 0, 0, 127);
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"selection_axis_color" );
	if (ptr)
	{
		axisColor = convertColor(ptr);
	}
	_SelectionAxisH = newSelectionAxis(axisColor);
	_SelectionAxisV = newSelectionAxis(axisColor);

	// load all islands
	R2::CScenarioEntryPoints &sep = R2::CScenarioEntryPoints::getInstance();
	try
	{
		sep.loadCompleteIslands();
		const R2::CScenarioEntryPoints::TCompleteIslands &completeIslands = sep.getCompleteIslands();
		_Islands.reserve(completeIslands.size());
		for(uint k = 0; k < completeIslands.size(); ++k)
		{
			SMap island;
			island.Name = completeIslands[k].Island;
			island.ContinentName = ""; // no access to world map for now ...
			island.MinX = (float) completeIslands[k].XMin;
			island.MinY = (float) completeIslands[k].YMin;
			island.MaxX = (float) completeIslands[k].XMax;
			island.MaxY = (float) completeIslands[k].YMax;
			// post fix with current season
			island.BitmapName = completeIslands[k].Island + "_sp.tga";
			_Islands.push_back(island);
		}
	}
	catch(const NLMISC::EFile &e)
	{
		nlwarning(e.what());
	}
	return true;
}

//============================================================================================================
void CGroupMap::updateSelectionAxisSize()
{
	if (_SelectionAxisH->getActive())
	{
		_SelectionAxisH->setW(_WReal);
		_SelectionAxisH->setH(1);
		_SelectionAxisV->setW(1);
		_SelectionAxisV->setH(_HReal);
	}
}

//============================================================================================================
void CGroupMap::setSelectionAxis(bool active, const NLMISC::CVector2f &pos /*=NLMISC::CVector2f::Null*/)
{
	_SelectionAxisH->setActive(active);
	_SelectionAxisV->setActive(active);
	if (active)
	{
		sint32 x, y;
		worldToWindowSnapped(x, y, pos);
		_SelectionAxisH->setX(0);
		_SelectionAxisH->setY(y);
		_SelectionAxisV->setX(x);
		_SelectionAxisV->setY(0);
		updateSelectionAxisSize();
		_SelectionAxisH->invalidateCoords();
		_SelectionAxisV->invalidateCoords();
	}
}

//============================================================================================================
bool CGroupMap::getCtrlsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CCtrlBase*> &vICL)
{
	if (!((x >= _XReal) &&
		(x < (_XReal + _WReal))&&
		(y > _YReal) &&
		(y <= (_YReal+ _HReal))))
		return false;
	// test against current clip
	computeCurrentClipContribution(clipX, clipY, clipW, clipH,
								   clipX, clipY, clipW, clipH);

	if (!((x > clipX) &&
		(x < (clipX + clipW))&&
		(y > clipY) &&
		(y < (clipY + clipH))))
		return false;

	bool bFound = false;
	for (uint32 i = 0; i < _PolyButtons.size(); ++i)
	{
		if (_PolyButtons[i]->contains(CVector2f((float)x,(float)y)))
		{
			vICL.push_back(_PolyButtons[i]);
			bFound = true;
		}
	}
	return CInterfaceGroup::getCtrlsUnder(x, y, clipX, clipY, clipW, clipH, vICL);
}

//============================================================================================================
inline void CGroupMap::updateButtonPos(CLandMarkButton &dest) const
{
	sint32 x, y;
	mapToWindowSnapped(x, y, dest.Pos);
	dest.setX(x);
	dest.setY(y);
}


//============================================================================================================
void CGroupMap::updateCoords()
{
	updateSelectionAxisSize();
	//
	CGroupContainer *enclosingContainer = static_cast< CGroupContainer* >( getEnclosingContainer() );

	if (!enclosingContainer || !enclosingContainer->getActive()) return;
	// update position of landmarks
	updateScale();
	// Look if we want to display some details info
	// Show / Hide details with the ratio indicating the meter per pixel of screen
	//
	float denomU = _MapTexW * _Scale;
	_WorldToMapDeltaX = denomU != 0 ? 1.f / denomU : 0.f;
	//

	float denomV = _MapTexH * _Scale;
	_WorldToMapDeltaY = denomV != 0 ? 1.f / denomV : 0.f;
	//
	CInterfaceGroup::updateCoords();
	//
	if (_MapMode == MapMode_Normal)
	{
		//
		float minU, minV, maxU, maxV;
		computeUVRect(minU, minV, maxU, maxV);
		mapToWorld(_VisibleWorldMin, CVector2f(minU,maxV));
		mapToWorld(_VisibleWorldMax, CVector2f(maxU,minV));
		sint32 mapW = std::min(_MapW, _WReal);
		_MeterPerPixel = (_VisibleWorldMax.x - _VisibleWorldMin.x) / mapW;
//		bool newLandMarkShown = false;
		uint i;
		for (i = 0; i < _ContinentLM.size(); ++i)
			setupFromZoom(_ContinentLM[i], _ContinentLM[i]->Type, _MeterPerPixel);
		for (i = 0; i < _ContinentText.size(); ++i)
			setupFromZoom(_ContinentText[i], _ContinentText[i]->Type, _MeterPerPixel);
		//
		updateLandMarkList(_ContinentLM);
		updateLandMarkTextList(_ContinentText);
		updateLandMarkList(_UserLM);
		updateLandMarkList(_MissionLM);
		// target
		if (_TargetLM && _TargetLM->getActive()) updateButtonPos(*_TargetLM);
		// home
		if (_HomeLM && _HomeLM->getActive()) updateButtonPos(*_HomeLM);
		// animals
		updateLandMarkList(_AnimalLM);
		// teammates
		updateLandMarkList(_TeammateLM);

		for (uint32 i = 0; i < _PolyButtons.size(); ++i)
			_PolyButtons[i]->updateCoords();
	}
	//
	updateLandMarkList(_RespawnLM);

	// user decorations
	for(TDecos::iterator it = _Decos.begin(); it != _Decos.end(); ++it)
	{
		(*it)->onUpdate(*this);
	}
	//
	CInterfaceGroup::updateCoords(); // must update coords twice : first is to get map size and meter per pixel -> this allow to know which landmark are visible
	                                 //second is to update (possibly newly visible) landmarks position
}


//============================================================================================================
void CGroupMap::worldToMap(NLMISC::CVector2f &dest,const NLMISC::CVector2f &src) const
{
	dest.x = _MapMinCorner.x != _MapMaxCorner.x ? (src.x - _MapMinCorner.x) / (_MapMaxCorner.x - _MapMinCorner.x) : _MapMinCorner.x;
	dest.y = _MapMinCorner.y != _MapMaxCorner.y ? 1.f - (src.y - _MapMinCorner.y) / (_MapMaxCorner.y - _MapMinCorner.y) : 1.f - _MapMinCorner.y;
}

//============================================================================================================
void CGroupMap::mapToWorld(NLMISC::CVector2f &dest,const NLMISC::CVector2f &src) const
{
	dest.x = src.x * (_MapMaxCorner.x - _MapMinCorner.x) + _MapMinCorner.x;
	dest.y = (1.f - src.y) *  (_MapMaxCorner.y - _MapMinCorner.y) +  _MapMinCorner.y;
}


/*
//============================================================================================================
void CGroupMap::mapToScreen(sint32 &px, sint32 &py, const NLMISC::CVector2f &src) const
{
	px = (sint32) (_XReal + (src.x - (_PlayerPos.x + _Offset.x)) * _MapTexW * _Scale);
	py = (sint32) (_YReal + _HReal - (src.y - (_PlayerPos.y + _Offset.y)) * _MapTexH * _Scale);
}

//============================================================================================================
void CGroupMap::mapToWindow(sint32 &px, sint32 &py, const NLMISC::CVector2f &src) const
{
	px = (sint32) ((src.x - (_PlayerPos.x + _Offset.x)) * _MapTexW * _Scale);
	py = (sint32) (_HReal - (src.y - (_PlayerPos.y + _Offset.y)) * _MapTexH * _Scale);
}
*/

//============================================================================================================
void CGroupMap::worldToWindow(NLMISC::CVector2f &dest, const NLMISC::CVector2f &src) const
{
	worldToMap(dest, src);
	mapToWindow(dest, dest);
}

//============================================================================================================
void CGroupMap::worldToWindowSnapped(sint32 &px,sint32 &py, const NLMISC::CVector2f &src) const
{
	NLMISC::CVector2f snappedPos;
	snappedPos.x = _WorldToMapDeltaX != 0.f ? (src.x - fmodf(src.x, _WorldToMapDeltaX)) : src.x;
	snappedPos.y = _WorldToMapDeltaY != 0.f ? (src.y - fmodf(src.y, _WorldToMapDeltaY)) : src.y;
	worldToMap(snappedPos, snappedPos);
	mapToWindow(px, py, snappedPos);
}

//============================================================================================================
void CGroupMap::mapToWindowSnapped(sint32 &px,sint32 &py, const NLMISC::CVector2f &src) const
{
	NLMISC::CVector2f snappedPos;
	snappedPos.x = _WorldToMapDeltaX != 0.f ? (src.x - fmodf(src.x, _WorldToMapDeltaX)) : src.x;
	snappedPos.y = _WorldToMapDeltaY != 0.f ? (src.y - fmodf(src.y, _WorldToMapDeltaY)) : src.y;
	mapToWindow(px, py, snappedPos);
}

//============================================================================================================
void CGroupMap::mapToScreen(sint32 &px, sint32 &py, const NLMISC::CVector2f &src) const
{
	px = (sint32) (_XReal + _MapX + (src.x - (_PlayerPos.x + _Offset.x)) * _MapTexW * _Scale);
	py = (sint32) (_YReal + _MapY + _HReal - (src.y - (_PlayerPos.y + _Offset.y)) * _MapTexH * _Scale);
}

//============================================================================================================
void CGroupMap::windowToScreen(sint32 &destX, sint32 &destY, sint32 srcX, sint32 srcY) const
{
	destX = srcX + _XReal;
	destY = srcY + _YReal;
}

//============================================================================================================
void CGroupMap::mapToWindow(sint32 &px, sint32 &py, const NLMISC::CVector2f &src) const
{
	px = (sint32) ((src.x - (_PlayerPos.x + _Offset.x)) * _MapTexW * _Scale) + _MapX;
	py = (sint32) (_HReal - (src.y - (_PlayerPos.y + _Offset.y)) * _MapTexH * _Scale) + _MapY;
}

//============================================================================================================
void CGroupMap::mapToWindow(NLMISC::CVector2f &dest,const NLMISC::CVector2f &src) const
{
	dest.x = ((src.x - (_PlayerPos.x + _Offset.x)) * _MapTexW * _Scale) + (float) _MapX;
	dest.y = (sint32) (_HReal - (src.y - (_PlayerPos.y + _Offset.y)) * _MapTexH * _Scale) + (float) _MapY;
}

//============================================================================================================
void CGroupMap::screenToMap(NLMISC::CVector2f &dest, sint32 px, sint32 py) const
{
	if (_MapTexW == 0 || _MapTexH == 0)
	{
		dest.set(0.f, 0.f);
		return;
	}
	dest.x = (px - (_XReal + _MapX)) / (_MapTexW * _Scale) + _PlayerPos.x + _Offset.x;
	dest.y = (_YReal + _HReal + _MapY - py) / (_MapTexH * _Scale) + _PlayerPos.y + _Offset.y;
}

//============================================================================================================
void CGroupMap::windowToMap(NLMISC::CVector2f &dest,sint32 px,sint32 py) const
{
	if (_MapTexW == 0 || _MapTexH == 0)
	{
		dest.set(0.f, 0.f);
		return;
	}
	dest.x = ((px + 0.5f) -  _MapX) / (_MapTexW * _Scale) + _PlayerPos.x + _Offset.x;
	dest.y = (_HReal + _MapY - (py - 0.5f)) / (_MapTexH * _Scale) + _PlayerPos.y + _Offset.y;
}

//============================================================================================================
void CGroupMap::updateLMPosFromDBPos(CLandMarkButton *dest ,sint32 px, sint32 py)
{
	nlassert(dest);
	if (px == 0 && py == 0)
	{
		dest->setActive(false);
	}
	else
	{
		NLMISC::CVector2f worldPos(px * 0.001f, py * 0.001f);
		NLMISC::CVector2f mapPos;
		worldToMap(mapPos, worldPos);
		if (mapPos.x != dest->Pos.x)
		{
			dest->Pos.x = mapPos.x;
			dest->invalidateCoords();
		}
		if (mapPos.y != dest->Pos.y)
		{
			dest->Pos.y = mapPos.y;
			dest->invalidateCoords();
		}
		dest->setActive(true);
	}
}
//============================================================================================================
// create mission position state object from base db path
static CSmartPtr<CNamedEntityPositionState> buildMissionPositionState(CInterfaceManager *im, const std::string &baseDBpath, uint missionIndex, uint targetIndex)
{
	nlassert(im);
	CCDBNodeLeaf *name = NLGUI::CDBManager::getInstance()->getDbProp(baseDBpath + NLMISC::toString(":%d:TARGET%d:TITLE", (int) missionIndex, (int) targetIndex));
	CCDBNodeLeaf *x = NLGUI::CDBManager::getInstance()->getDbProp(baseDBpath + NLMISC::toString(":%d:TARGET%d:X", (int) missionIndex, (int) targetIndex));
	CCDBNodeLeaf *y = NLGUI::CDBManager::getInstance()->getDbProp(baseDBpath +NLMISC::toString(":%d:TARGET%d:Y", (int) missionIndex, (int) targetIndex));
	CSmartPtr<CNamedEntityPositionState> ps = new CNamedEntityPositionState;
	ps->build(name, x, y);
	return ps;
}

//============================================================================================================
void CGroupMap::checkCoords()
{
	if (!_Active) return;
	updatePlayerPos();
	if (!_MapTF && !_MapLoadFailure)
	{
		loadMap();
		invalidateCoords();
	}
	updateContinentInfo();

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// **** update landmark for missions from the database
	if (_MissionPosStates.empty()) // pointers on db node initialised ?
	{
		_MissionPosStates.resize(2 * MAX_NUM_MISSIONS * MAX_NUM_MISSION_TARGETS); // 2 is because of group missions
		uint targetIndex = 0;
		for(uint k = 0; k < MAX_NUM_MISSIONS; ++k)
		{
			for(uint l = 0; l <MAX_NUM_MISSION_TARGETS; ++l)
			{
				_MissionPosStates[targetIndex++] = buildMissionPositionState(pIM, MISSIONS_DB_PATH, k, l);
				_MissionPosStates[targetIndex++] = buildMissionPositionState(pIM, GROUP_MISSIONS_DB_PATH, k, l);
			}
		}
		// also fill ptr for special landmarks (target, home & respawn)
		_TargetPos = NLGUI::CDBManager::getInstance()->getDbProp(COMPASS_DB_PATH ":TARGET");
		_HomePos = NLGUI::CDBManager::getInstance()->getDbProp(COMPASS_DB_PATH ":HOME_POINT");
	}
	//
//	bool mustInvalidateCoords = false;
	for(uint k = 0; k < 2 * MAX_NUM_MISSIONS * MAX_NUM_MISSION_TARGETS; ++k)
	{
		sint32 nameID = _MissionPosStates[k]->getNameNode()->getValue32();
		if (nameID != _MissionTargetTextIDs[k])
		{
			_MissionTargetTextIDs[k] = nameID;
			if (_MissionTargetTextIDs[k] == 0)
			{
				if (_MissionLM[k] != NULL)
				{
					delCtrl(_MissionLM[k]);
					_MissionLM[k] = NULL;
				}
			}
			else
			{
				// create a new button if necessary
				if (_MissionLM[k] == NULL)
				{
					_MissionLM[k] = createLandMarkButton(_MissionLMOptions);
					_MissionLM[k]->setParent(this);
					addCtrl(_MissionLM[k]);
				}
			}
			// text must be received
			_MissionTargetTextReceived[k] = false;
		}
		if (_MissionLM[k])
		{
			// update text if needed
			if (!_MissionTargetTextReceived[k])
			{
				ucstring result;
				if (STRING_MANAGER::CStringManagerClient::instance()->getDynString(_MissionTargetTextIDs[k], result))
				{
					_MissionLM[k]->setDefaultContextHelp(result);
					_MissionTargetTextReceived[k] = true;
				}
			}
		}
	}

	// **** retrieve pos of target and update it, or hide it if there's no target
	if (_TargetLM)
	{
		sint32 px = (sint32) (_TargetPos->getValue64() >> 32);
		sint32 py = _TargetPos->getValue32();
		updateLMPosFromDBPos(_TargetLM, px, py);
		if (_TargetLM->getActive())
		{
			if (UserEntity)
			{
				if (UserEntity->selection() != CLFECOMMON::INVALID_SLOT && UserEntity->selection() != 0)
				{
					CEntityCL *sel = EntitiesMngr.entity(UserEntity->selection());
					if (sel)
					{
						_TargetLM->setDefaultContextHelp(NLMISC::CI18N::get("uiTargetTwoPoint") + sel->removeTitleAndShardFromName(sel->getEntityName()));
					}
				}
			}
		}
	}

	// **** retrieve pos of home and update it, or hide it if there's no target
	if (_HomeLM)
	{
		sint32 px = (sint32) (_HomePos->getValue64() >> 32);
		sint32 py = _HomePos->getValue32();
		updateLMPosFromDBPos(_HomeLM, px, py);
	}

	// **** retrieve pos of respawn and update it, or hide it if there's no target
	uint i;
	if (_RespawnPos.size() < _RespawnLM.size())
	{
		for (i = (uint)_RespawnPos.size(); i < _RespawnLM.size(); i++)
		{
			delCtrl(_RespawnLM[i]);
			_RespawnLM[i] = NULL;
		}
	}
	_RespawnLM.resize(_RespawnPos.size(), NULL);
	for(i = 0; i < _RespawnPos.size(); ++i)
	{
		if (_RespawnLM[i] == NULL)
		{
			_RespawnLM[i] = createLandMarkButton(_RespawnLMOptions);
			_RespawnLM[i]->setId(this->getId() + ":rplm_" + NLMISC::toString(i));
			_RespawnLM[i]->setParent(this);
			if (_MapMode == MapMode_SpawnSquad)
				_RespawnLM[i]->setDefaultContextHelp(NLMISC::CI18N::get("uiSquadSpawnPoint") + NLMISC::toString(" %u", i+1));
			else
			{
				if (isIsland())
				{
					_RespawnLM[i]->setDefaultContextHelp(NLMISC::CI18N::get("uiR2EntryPoint"));
				}
				else
				{
					_RespawnLM[i]->setDefaultContextHelp(NLMISC::CI18N::get("uiRespawnPoint"));
				}
				_RespawnLM[i]->HandleEvents = R2::getEditor().getMode() != R2::CEditor::EditionMode;
			}
			addCtrl(_RespawnLM[i]);
		}
		if (_RespawnLM[i])
			updateLMPosFromDBPos(_RespawnLM[i], _RespawnPos[i].x, _RespawnPos[i].y);
	}
	if ((_MapMode == MapMode_Death) || (_MapMode == MapMode_SpawnSquad))
	{
		if (_RespawnPosReseted)
		{
			_RespawnPosReseted = false;
			// Reset selection
			_RespawnSelected = 0;
			if (_MapMode == MapMode_Death)
				NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:RESPAWN_PT")->setValue32(_RespawnSelected);
			else if (_MapMode == MapMode_SpawnSquad)
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:SQUAD_RESPAWN_PT")->setValue32(_RespawnSelected);
			if (_RespawnLM.size() > 0)
				_RespawnSelectedBitmap->setParentPos(_RespawnLM[_RespawnSelected]);
		}
		else
		{
			_RespawnSelected = getRespawnSelected();
			if (_RespawnLM.size() > 0)
				_RespawnSelectedBitmap->setParentPos(_RespawnLM[_RespawnSelected]);
		}
	}

	// **** Animal Landmarks
	// initialize DB if not done
	if(_AnimalPosStates.empty() && !_AnimalLM.empty())
	{
		_AnimalPosStates.resize(_AnimalLM.size());
		// pack animals
		nlassert(_AnimalPosStates.size()<=MAX_INVENTORY_ANIMAL);
		for(uint i=0;i<MAX_INVENTORY_ANIMAL;i++)
		{
			_AnimalPosStates[i] = new CAnimalPositionState;
			_AnimalPosStates[i]->build(NLMISC::toString("SERVER:PACK_ANIMAL:BEAST%d",i));
		}
	}
	// update DB pos & tooltip text
	for(i=0;i<_AnimalLM.size();i++)
	{
		if( _AnimalLM[i] )
		{
			if (_IsIsland)
			{
				_AnimalLM[i]->setActive(false);
			}
			else
			{
				_AnimalLM[i]->setActive(true);
				// update texture from animal status
				CInterfaceManager	*pIM= CInterfaceManager::getInstance();
				CCDBNodeLeaf	*statusNode = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d", i) + ":STATUS", false);
				if (statusNode && ANIMAL_STATUS::isInStable((ANIMAL_STATUS::EAnimalStatus)statusNode->getValue32()) )
				{
					_AnimalLM[i]->setTexture(_AnimalStableLMOptions.LandMarkTexNormal);
					_AnimalLM[i]->setScale(true); // hardcoded because chosen icon is too big (bad...)
				}
				else
				if (statusNode && ANIMAL_STATUS::isDead((ANIMAL_STATUS::EAnimalStatus)statusNode->getValue32()) )
				{
					_AnimalLM[i]->setTexture(_AnimalDeadLMOptions.LandMarkTexNormal);
					_AnimalLM[i]->setScale(true); // hardcoded because chosen icon is too big (bad...)
				}
				else
				{
					_AnimalLM[i]->setTexture(_AnimalLMOptions.LandMarkTexNormal);
				}

				// update tooltip text
				ANIMAL_TYPE::EAnimalType at;
				at = (ANIMAL_TYPE::EAnimalType)NLGUI::CDBManager::getInstance()->getDbProp("SERVER:PACK_ANIMAL:BEAST"+toString(i)+":TYPE")->getValue32();
				string sPrefix;
				switch(at)
				{
					default:
					case ANIMAL_TYPE::All:	  sPrefix = "uiPATitleMount";  break;
					case ANIMAL_TYPE::Mount:  sPrefix = "uiPATitleMount";  break;
					case ANIMAL_TYPE::Packer: sPrefix = "uiPATitlePacker"; break;
					case ANIMAL_TYPE::Demon:  sPrefix = "uiPATitleDemon";  break;
				}
				_AnimalLM[i]->setDefaultContextHelp(NLMISC::CI18N::get(sPrefix+toString(i+1)));

				// update pos
				sint32	px, py;
				_AnimalPosStates[i]->getPos(px, py);
				updateLMPosFromDBPos(_AnimalLM[i], px, py);
			}
		}
	}

	// **** Teammate Landmarks
	// initialize DB if not done
	if(_TeammatePosStates.empty() && !_TeammateLM.empty())
	{
		_TeammatePosStates.resize(MaxNumPeopleInTeam);
		for(uint i=0; i < MaxNumPeopleInTeam; i++)
		{
			_TeammatePosStates[i] = new CTeammatePositionState;
			_TeammatePosStates[i]->build(NLMISC::toString("SERVER:GROUP:%d",i));
		}
	}
	// update DB pos.
	for(i=0; i < _TeammateLM.size(); i++)
	{

		if (_TeammateLM[i])
		{		
			sint32	px, py;

			if (_TeammatePosStates[i]->getPos(px, py))
			{
				CInterfaceManager *im = CInterfaceManager::getInstance();
				uint32 val = NLGUI::CDBManager::getInstance()->getDbProp(NLMISC::toString("SERVER:GROUP:%d:NAME",i))->getValue32();
				STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
				ucstring res;

				if (pSMC->getString(val,res))
				{
						res = CEntityCL::removeTitleAndShardFromName(res);
						_TeammateLM[i]->setDefaultContextHelp(res);
				}
			}
			updateLMPosFromDBPos(_TeammateLM[i], px, py);
		}
	}

	// update position for mission landmarks
	// TODO : try to factor behaviour between anim / treamate / missions (they are all dynamic)
	// TODO : make just one list of landmark with a CCompassTarget would be better ?
	for(i=0; i < _MissionPosStates.size(); i++)
	{
		if (_MissionLM[i])
		{
			sint32	px, py;
			if (_MissionPosStates[i]->getPos(px, py))
			{
				updateLMPosFromDBPos(_MissionLM[i], px, py);
			}
		}
	}


	// **** check if player has moved
	if (!EntitiesMngr.entities().empty())
	{
		if (EntitiesMngr.entity(0))
		{
			const NLMISC::CVectorD &playerPos = EntitiesMngr.entity(0)->pos();
			CVector2f newPlayerPos((float) playerPos.x, (float) playerPos.y);
			if (newPlayerPos != _OldPlayerPos)
			{
				_OldPlayerPos = newPlayerPos;
				invalidateCoords();
			}
		}
	}
	CInterfaceGroup::checkCoords();
}

//============================================================================================================
static void setupFromZoom(CViewBase *pVB, CContLandMark::TContLMType t, float fMeterPerPixel)
{
	bool active = false;
	// Icon
	if ((t == CContLandMark::Capital) && (fMeterPerPixel <= 5.0f))	active = true;
	if ((t == CContLandMark::Village) && (fMeterPerPixel <= 4.0f))	active = true;
	if ((t == CContLandMark::Outpost) && (fMeterPerPixel <= 4.0f))	active = true;
	if ((t == CContLandMark::Stable)  && (fMeterPerPixel <= 3.5f))	active = true;
	// Just a text
	if ((t == CContLandMark::Region)  && (fMeterPerPixel <= 50.0f))	active = true;
	if ((t == CContLandMark::Place)   && (fMeterPerPixel <= 6.0f))	active = true;
	if ((t == CContLandMark::Street)  && (fMeterPerPixel <= 2.0f))	active = true;
	if (t == CContLandMark::Unknown)								active = true;
	pVB->setActive(active);
}


//============================================================================================================
void CGroupMap::computeUVRect(float &minU, float &minV, float &maxU, float &maxV) const
{
	minU = _Offset.x + _PlayerPos.x;
	minV = _Offset.y + _PlayerPos.y;
	// delta U & V for a pixel on screen
	float deltaU = 1.f / favoid0(_MapTexW * _Scale);
	float deltaV = 1.f / favoid0(_MapTexH * _Scale);
	//
	if (_MapX < 0)
	{
		minU -= deltaU * _MapX;
	}
	//
	if (_MapY > 0)
	{
		minV += deltaV * _MapY;
	}
	// ensure that UV displacement results in moves pixel by pixel on screen (landmarks move pixel by pixel, so the map may appear not to follow them because it moves smoothly)
	minU -= fmodf(minU, deltaU);
	minV -= fmodf(minV, deltaV);
	//
	sint32 mapW = std::min(_MapW, _WReal);
	sint32 mapH = std::min(_MapH, _HReal);
	//
	maxU = minU + ((float) mapW / favoid0(_Scale * (float) _MapTexW));
	maxV = minV + ((float) mapH / favoid0(_Scale * (float) _MapTexH));
}


//============================================================================================================
void CGroupMap::computeFrustumQuad(CQuad &fruQuad) const
{

	CVector2f winPos;
	const NLMISC::CVector &camPos = MainCam.getMatrix().getPos();
	worldToWindow(winPos, camPos);
	fruQuad.V0.set(winPos.x, winPos.y, 0.f);
	CVector projectedFront = MainCam.getMatrix().getJ();
	projectedFront.z = 0.f;
	if (projectedFront.norm() <= 0.01f)
	{
		projectedFront = MainCam.getMatrix().getK();
		projectedFront.z = 0.f;
	}
	CVector front = projectedFront.normed();
	CVector right(- front.y, front.x, 0.f);
	const NL3D::CFrustum &fru = MainCam.getFrustum();
	float farRight = fru.Right * (fru.Far / fru.Near);
	float farLeft  = fru.Left * (fru.Far / fru.Near);
	worldToWindow(winPos, camPos  + farRight * right + fru.Far * front);;
	fruQuad.V1.set(winPos.x, winPos.y, 0.f);
	worldToWindow(winPos, camPos + farLeft * right + fru.Far * front);;
	fruQuad.V3.set(winPos.x, winPos.y, 0.f);
	CVector middle = 0.5f * (fruQuad.V1 + fruQuad.V3);
	fruQuad.V2 = fruQuad.V0 + 2.f * (middle - fruQuad.V0);
}

//============================================================================================================
void CGroupMap::draw()
{
	// user decorations
	for(TDecos::iterator it = _Decos.begin(); it != _Decos.end(); ++it)
	{
		(*it)->onPreRender(*this);
	}
	// TMP TMP for debug
	static volatile bool clearAll = false;
	if (clearAll)
	{
		removeLandMarks(_ContinentLM);
		removeLandMarks(_MissionLM);
		removeLandMarks(_AnimalLM);
		removeLandMarks(_TeammateLM);
		for (uint k = 0; k < _ContinentText.size(); ++k)
		delView(_ContinentText[k]);
		_ContinentText.clear();
		for (uint k = 0; k < _PolyButtons.size(); ++k)
		delCtrl(_PolyButtons[k]);
		_PolyButtons.clear();
	}

	sint32 oldSciX, oldSciY, oldSciW, oldSciH;
	makeNewClip (oldSciX, oldSciY, oldSciW, oldSciH);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CViewRenderer &vr = *CViewRenderer::getInstance();
	uint8 alpha = CWidgetManager::getInstance()->getGlobalColorForContent().A;
	updateScale();

	// No Op if screen minimized
	if(vr.isMinimized())
		return;

	// compute scissors
	uint32 sw, sh;
	vr.getScreenSize(sw, sh);

	NL3D::CScissor oldScissor;
	oldScissor = Driver->getScissor();
	sint32 sciX, sciY, sciW, sciH;
	vr.getClipWindow(sciX, sciY, sciW, sciH);
	NL3D::CScissor newScissor;
	newScissor.X = sciX / (float) sw;
	newScissor.Width = sciW / (float) sw;
	newScissor.Y = sciY / (float) sh;
	newScissor.Height = sciH / (float) sh;
	//
	if (!_PlayerPosTF && !_PlayerPosLoadFailure)
	{
		loadPlayerPos();
	}
	if (!_MapLoadFailure)
	{
		if (_MapTexW != 0 && _MapTexH != 0)
		{
			float minU, minV, maxU, maxV;
			computeUVRect(minU, minV, maxU, maxV);
			// draw the map
			sint32 mapW = std::min(_MapW, _WReal);
			sint32 mapH = std::min(_MapH, _HReal);
			// Display fog of war map
			CVector2f fowMin(0,0), fowMax(0,0);
			float fowURatio = 0.0f;
			float fowVRatio = 0.0f;
			// Convert u,v (from map coords) to world coordinates
			CVector2f worldMin, worldMax;
			mapToWorld(worldMin, CVector2f(minU,minV));
			mapToWorld(worldMax, CVector2f(maxU,maxV));

			if (_CurContinent != NULL)
			{
				// Convert world coordinate to map coordinate in the fog of war map
				CFogOfWar &rFOW = _CurContinent->FoW;
				if (rFOW.getMaxX() != rFOW.getMinX()) fowMin.x = (worldMin.x - rFOW.getMinX()) / (rFOW.getMaxX() - rFOW.getMinX());
				if (rFOW.getMaxY() != rFOW.getMinY()) fowMin.y = (worldMin.y - rFOW.getMinY()) / (rFOW.getMaxY() - rFOW.getMinY());
				if (rFOW.getMaxX() != rFOW.getMinX()) fowMax.x = (worldMax.x - rFOW.getMinX()) / (rFOW.getMaxX() - rFOW.getMinX());
				if (rFOW.getMaxY() != rFOW.getMinY()) fowMax.y = (worldMax.y - rFOW.getMinY()) / (rFOW.getMaxY() - rFOW.getMinY());
				if ((fowMin.x >= 0) && (fowMin.x <= 1) &&
					(fowMin.y >= 0) && (fowMin.y <= 1) &&
					(fowMax.x >= 0) && (fowMax.x <= 1) &&
					(fowMax.y >= 0) && (fowMax.y <= 1))
				{
					fowURatio = (float)rFOW.getMapWidth() / favoid0((float)rFOW.getRealWidth());
					fowVRatio = (float)rFOW.getMapHeight() / favoid0((float)rFOW.getRealHeight());
				}

			}

			vr.drawCustom(_XReal + std::max(_MapX, (sint32) 0), _YReal - std::min(_MapY, (sint32) 0), mapW, mapH,
						  CUV(minU * _URatio, minV * _VRatio), CUV(maxU * _URatio, maxV * _VRatio),
						  CUV(fowMin.x * fowURatio, fowMin.y * fowVRatio), CUV(fowMax.x * fowURatio, fowMax.y * fowVRatio),
						  CRGBA(255, 255, 255, alpha),
						  _MapMaterial );

			// Look if we want to display some details info
			// Show / Hide details with the ratio indicating the meter per pixel of screen
			/*
			{
				float fMeterPerPixel = (worldMax.x - worldMin.x) / mapW;

				uint32 i;
				for (i = 0; i < _ContinentLM.size(); ++i)
					setupFromZoom(_ContinentLM[i], _ContinentLM[i]->Type, fMeterPerPixel);
				for (i = 0; i < _ContinentText.size(); ++i)
					setupFromZoom(_ContinentText[i], _ContinentText[i]->Type, fMeterPerPixel);
			}
			*/


			/* if (_MapW < _WReal || _MapH < _HReal)
				vr.drawWiredQuad(_XReal + _MapX, _YReal - _MapY, _MapW, _MapH, CRGBA(0, 0, 0, alpha));*/
		}
	}

	// if editor is in edition mode, draw the frustum instead (for better visibility)
	CQuad fruQuad;
	CTriangle fruTri;

	CRGBA fruColor(0, 0, 0);

	if (_FrustumView)
	{
		if (R2::getEditor().getMode() == R2::CEditor::EditionMode)
		{
			static volatile bool wantFrustum = true;
			if (wantFrustum)
			{
				computeFrustumQuad(fruQuad);
				_FrustumView->setActive(true);
				//_FrustumView->setAdditif(true);
				_FrustumView->setQuad(fruQuad);
				_FrustumView->updateCoords();
				// handle mouse over
				if (CWidgetManager::getInstance()->getPointer())
				{
					sint32 originX, originY;
					getCorner(originX, originY, getPosRef());
					CVector delta((float) originX, (float) originY, 0.f);
					fruTri = CTriangle(fruQuad.V0, fruQuad.V1, fruQuad.V2);
					CVector mousePos((float) CWidgetManager::getInstance()->getPointer()->getXReal(), (float) CWidgetManager::getInstance()->getPointer()->getYReal(), 0.f);
					mousePos -= delta;
					CVector dummyHit;
					float deltaBlend = DT / (0.001f * (float) _FrustumViewBlendTimeInMs);
					bool hit = fruTri.intersect(mousePos + CVector::K, mousePos - CVector::K, dummyHit, CPlane(0.f, 0.f, 0.f, 1.f));
					NLMISC::incrementalBlend(_FrustumOverBlendFactor, hit ? 1.f : 0.f, deltaBlend);
					fruColor.blendFromui(_FrustumViewColor, _FrustumViewColorOver, (uint8) (255 * _FrustumOverBlendFactor));
				}
				else
				{
					fruColor = _FrustumViewColor;
				}
				//
				_FrustumView->setColorRGBA(fruColor);
			}
		}
		else
		{
			_FrustumView->setActive(false);
		}
	}

	//Driver->setScissor(oldScissor);
	// let parent view draw continent, mission and user locations
	CInterfaceGroup::draw();
	// force the flush
	vr.flush();


	Driver->setScissor(newScissor);

	if (R2::getEditor().getMode() != R2::CEditor::EditionMode)
	{
		// Draw the player TODO : replace with a CViewQuad
		if (!_PlayerPosLoadFailure)
		{
			// draw rotated bitmap for player position
			const NLMISC::CVector &front3f = UserEntity->front();
			NLMISC::CVector2f front;
			NLMISC::CVector2f right;
			front.set(front3f.x, front3f.y);
			right.set(front3f.y, - front3f.x);
			// compute corners
			sint32 spx, spy;
			mapToScreen(spx, spy, _PlayerPos);
			NLMISC::CVector center;
			center.set((float) spx, (float) spy, 0.f);
			NLMISC::CQuadColorUV quv;
			quv.V0 = center - 0.5f * (float) _PlayerPosTexW * right - 0.5f * (float) _PlayerPosTexH * front;
			quv.V1 = center + 0.5f * (float) _PlayerPosTexW * right - 0.5f * (float) _PlayerPosTexH * front;
			quv.V2 = center + 0.5f * (float) _PlayerPosTexW * right + 0.5f * (float) _PlayerPosTexH * front;
			quv.V3 = center - 0.5f * (float) _PlayerPosTexW * right + 0.5f * (float) _PlayerPosTexH * front;
			quv.Uv0.set(0.f, 1.f);
			quv.Uv1.set(1.f, 1.f);
			quv.Uv2.set(1.f, 0.f);
			quv.Uv3.set(0.f, 0.f);
			quv.Color0 = quv.Color1 = quv.Color2 = quv.Color3 = CRGBA(255, 255, 255, alpha);
			quv.V0.x /= (float) sw;
			quv.V0.y /= (float) sh;
			quv.V1.x /= (float) sw;
			quv.V1.y /= (float) sh;
			quv.V2.x /= (float) sw;
			quv.V2.y /= (float) sh;
			quv.V3.x /= (float) sw;
			quv.V3.y /= (float) sh;
			Driver->drawQuads(&quv, 1, _PlayerPosMaterial);
		}
	}

	// draw border of frustum
	if (_FrustumView)
	{
		if (R2::getEditor().getMode() == R2::CEditor::EditionMode)
		{
			if (_FrustumMaterial.empty())
			{
				// create material for the world map
				_FrustumMaterial = Driver->createMaterial();
				if (!_FrustumMaterial.empty())
				{
					_FrustumMaterial.initUnlit();
					_FrustumMaterial.setZWrite(false);
					_FrustumMaterial.setZFunc(NL3D::UMaterial::always);
					_FrustumMaterial.setBlend (true);
					_FrustumMaterial.setBlendFunc (NL3D::UMaterial::srcalpha, NL3D::UMaterial::invsrcalpha);
					_FrustumMaterial.setDoubleSided();
				}
			}
			if (!_FrustumMaterial.empty())
			{
				fruColor.A /= 2;
				_FrustumMaterial.setColor(fruColor);
				//Driver->setScissor(newScissor);
				NL3D::UDriver::TPolygonMode oldPolygonMode = Driver->getPolygonMode();
				Driver->setPolygonMode(NL3D::UDriver::Line);
				//
				CTriangle fruTri;
				fruTri.V0.set((fruQuad.V0.x + _XReal) / sw, (fruQuad.V0.y + _YReal) / sh, 0.f);
				fruTri.V1.set((fruQuad.V1.x + _XReal) / sw, (fruQuad.V1.y + _YReal) / sh, 0.f);
				fruTri.V2.set((fruQuad.V3.x + _XReal) / sw, (fruQuad.V3.y + _YReal) / sh, 0.f);
				Driver->drawTriangle(fruTri, _FrustumMaterial);
				Driver->setPolygonMode(oldPolygonMode);
			}
		}
	}

	// Draw all poly buttons
	for (uint32 i = 0; i < _PolyButtons.size(); ++i)
	{
		_PolyButtons[i]->drawPolyButton();
	}

	// Restore Old Scissor
	Driver->setScissor(oldScissor);

	restoreClip (oldSciX, oldSciY, oldSciW, oldSciH);
}

//============================================================================================================
bool CGroupMap::handleEvent(const NLGUI::CEventDescriptor &event)
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	// if R2 editor editor is on, give it a chance to handle the event first
	if (ClientCfg.R2EDEnabled && R2::getEditor().getCurrentTool())
	{
		bool handled = false;
		bool panEnd = false;
		// handle standard clicks
		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
			panEnd = eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup && _Panning && _HasMoved;
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup && !panEnd)
			{
				//if (CWidgetManager::getInstance()->getCapturePointerLeft() == this)
				// NB : don't test capture of mouse here, because
				// some R2 tool may begin outside of this window
				// example : clicking in the palette window and doing a drag-and-drop to the
				// minimap
				{
					handled = R2::getEditor().getCurrentTool()->onMouseLeftButtonUp();
					if (!handled)
					{
						handled = R2::getEditor().getCurrentTool()->onMouseLeftButtonClicked();
					}
				}
				if (!handled && R2::getEditor().getMode() == R2::CEditor::EditionMode)
				{
					if (R2::getEditor().getCurrentTool())
					{
						if (!R2::getEditor().getCurrentTool()->getPreviousToolClickEndFlag())
						{
							if (CWidgetManager::getInstance()->getCapturePointerLeft() == this)
							{
								// unselected unless tool has been changed before last mouse left up (happens when one's finish a route using double click -> should not unselect then)
								R2::getEditor().setSelectedInstance(NULL);
							}
						}
					}
				}
			}
			else if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightup)
			{
				if (CWidgetManager::getInstance()->getCapturePointerRight() == this)
				{
					if (isIn(eventDesc.getX(), eventDesc.getY()))
					{
						NLMISC::CVector2f clickPos;
						screenToMap(clickPos, eventDesc.getX(), eventDesc.getY());
						if (clickPos.x >= 0.f &&
							clickPos.y >= 0.f &&
							clickPos.x <= 1.f &&
							clickPos.y <= 1.f
						   )
						{
							handled = R2::getEditor().getCurrentTool()->onMouseRightButtonClicked();
						}
					}
				}
			}
		}
		if (!panEnd)
		{
			handled = handled || R2::getEditor().getCurrentTool()->handleEvent(event);
		}

		if (handled)
		{
			_Panning = false;
			_HasMoved = false;
			return true;
		}
	}

	for (uint32 i = 0; i < _PolyButtons.size(); ++i)
		if (_PolyButtons[i]->handleEvent(event))
			return true;

	// left button can be used to 'pan' the map
	// mouse wheel can be used to zoom in/out
	if (event.getType() == NLGUI::CEventDescriptor::mouse)
	{
		if (!_Active)
			return false;
		const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
		{
			if (CWidgetManager::getInstance()->getCapturePointerLeft() != this)
			{
				return false;
			}
			_Panning = false;
			_HasMoved = false;
			return true;
		}

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightdown)
		{
			CWidgetManager::getInstance()->setCapturePointerRight(this);
			return true;
		}

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown)
		{
			if (isIn(eventDesc.getX(), eventDesc.getY()))
			{
				CWidgetManager::getInstance()->setCapturePointerLeft(this);
				_StartXForPaning = eventDesc.getX();
				_StartYForPaning = eventDesc.getY();
				_StartWorldOffsetForPaning = _WorldOffset;

				// Clamp the start WorldOffset on the Move start. NB: not clamped on a scale
				computeOffsets();
				_StartWorldOffsetForPaning.x= _Offset.x * (_MapMaxCorner.x - _MapMinCorner.x);
				_StartWorldOffsetForPaning.y= _Offset.y * (_MapMaxCorner.y - _MapMinCorner.y);

				_PanStartDateInMs = T1;
				/** If in editor, and current tool is a creation tool, only handle panning after a small delta pos / delta time
				  * for better ergonomy
				  */
				if (ClientCfg.R2EDEnabled && R2::getEditor().getCurrentTool() &&
					R2::getEditor().getCurrentTool()->isCreationTool()
				   )
				{
					_DeltaPosBeforePan = 2;
					_DeltaTimeBeforePanInMs = 300;
				}
				else
				{
					// handle panning immediately
					_DeltaPosBeforePan = 0;
					_DeltaTimeBeforePanInMs = 0;
				}
				_Panning = true;
				return true;
			}
		}

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousemove)
		{
			if (CWidgetManager::getInstance()->getCapturePointerLeft() != this || !_Panning)
				return CInterfaceGroup::handleEvent(event);

			if (_MapTexW != 0 && _MapTexH != 0)
			{
				if (_HasMoved || (T1 - _PanStartDateInMs) > _DeltaTimeBeforePanInMs ||
					(uint) abs(eventDesc.getX() - _StartXForPaning) > _DeltaPosBeforePan ||
					(uint) abs(eventDesc.getY() - _StartYForPaning) > _DeltaPosBeforePan)
				{
					_HasMoved = true;
					NLMISC::CVector2f	dWorld;
					dWorld.x= - (_MapMaxCorner.x - _MapMinCorner.x) * (eventDesc.getX() - _StartXForPaning) / (_MapTexW * _Scale);
					dWorld.y= + (_MapMaxCorner.y - _MapMinCorner.y) * (eventDesc.getY() - _StartYForPaning) / (_MapTexH * _Scale);
					_WorldOffset= _StartWorldOffsetForPaning + dWorld;

					// Clamp the WorldOffset on a Move. NB: not clamped on a scale
					computeOffsets();
					_WorldOffset.x= _Offset.x * (_MapMaxCorner.x - _MapMinCorner.x);
					_WorldOffset.y= _Offset.y * (_MapMaxCorner.y - _MapMinCorner.y);

					computeOffsets();
					invalidateCoords();
				}
			}
			return true;
		}


		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousewheel && !_Panning)
		{
			sint32 wheel = eventDesc.getWheel();
			float newScale = _UserScale;
			while (wheel != 0)
			{
				if (wheel < 0)
				{
					newScale *= 0.7f;
					if (newScale < 1.f)
					{
						newScale = 1.f;
						break;
					}
					++ wheel;
				}
				else
				{
					newScale *= 1.3f;
					float realScale = computeRealScaleFromUserScale(newScale);
					if (realScale > getActualMaxScale())
					{
						newScale = computeUserScaleFromRealScale(getActualMaxScale());
						break;
					}
					-- wheel;
				}
			}
			NLMISC::CVector2f mapPos;
			screenToMap(mapPos, eventDesc.getX(), eventDesc.getY());
			setScale(newScale, mapPos);
			return true;
		}

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightup)
		{
			// convert click pos into map pos
			if (_MapTexW == 0 || _MapTexH == 0)
			{
				_RightClickLastPos.set(0.f, 0.f);
			}
			else
			{
				NLMISC::CVector2f clickPos;
				screenToMap(clickPos, eventDesc.getX(), eventDesc.getY());
				if (clickPos.x < 0.f ||
					clickPos.y < 0.f ||
					clickPos.x > 1.f ||
					clickPos.y > 1.f
				   )
				{
					return false;
				}
				_RightClickLastPos = clickPos;
				return CInterfaceGroup::handleEvent(event);
			}
		}
	}
	if (event.getType() == NLGUI::CEventDescriptor::system)
	{
		NLGUI::CEventDescriptorSystem &es = (NLGUI::CEventDescriptorSystem &) event;
		if (es.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::activecalledonparent)
		{
			bool visible = getActive();
			if (visible)
			{
				CInterfaceElement *curr = this->getParent();
				while (curr)
				{
					if (!curr->getActive())
					{
						visible = false;
						break;
					}
					curr = curr->getParent();
				}
			}
			if (visible)
			{
				checkCoords();
			}
			else
			{
				unloadMap();
			}
		}
	}

	return CInterfaceGroup::handleEvent(event);
}

//============================================================================================================
void CGroupMap::pan(sint32 dx, sint32 dy)
{
	if (_MapTexW * _Scale != 0.f) _WorldOffset.x += dx * (_MapMaxCorner.x - _MapMinCorner.x) / (_MapTexW * _Scale);
	if (_MapTexH * _Scale != 0.f) _WorldOffset.y -= dy * (_MapMaxCorner.y - _MapMinCorner.y) / (_MapTexH * _Scale);
	computeOffsets();
	_WorldOffset.x= _Offset.x * (_MapMaxCorner.x - _MapMinCorner.x);
	_WorldOffset.y= _Offset.y * (_MapMaxCorner.y - _MapMinCorner.y);
	invalidateCoords();
}

//============================================================================================================
void CGroupMap::setActive(bool active)
{
	bool visible = active;
	CInterfaceElement *curr = this->getParent();
	while (curr)
	{
		if (!curr->getActive())
		{
			visible = false;
			break;
		}
		curr = curr->getParent();
	}
	if (!visible)
	{
		unloadMap();
	}
	CInterfaceGroup::setActive(active);
	if (visible)
	{
		checkCoords();
	}
}

//============================================================================================================
void CGroupMap::loadPlayerPos()
{
	if (_PlayerPosLoadFailure) return;
	_PlayerPosLoadFailure = true;
	if (_PlayerPosMaterial.empty())
		return;
	std::string fullName = NLMISC::CPath::lookup(_PlayerPosTexName, false, false);
	if (fullName.empty())
	{
		nlwarning("Can't player pos texture %s", _PlayerPosTexName.c_str());
		return;
	}
	uint32 w, h;
	NLMISC::CIFile bm;
	if (bm.open(fullName))
	{
		try
		{
			NLMISC::CBitmap::loadSize(fullName, w, h);
		}
		catch(const NLMISC::Exception &e)
		{
			nlwarning(e.what());
			return;
		}
	}
	else
	{
		nlwarning("Can't open player pos texture %s", fullName.c_str());
		return;
	}
	_PlayerPosTF = Driver->createTextureFile(fullName);
	if (!_PlayerPosTF) return;
	_PlayerPosLoadFailure = false;
	_PlayerPosTF->setWrapS(NL3D::UTexture::Clamp);
	_PlayerPosTF->setWrapT(NL3D::UTexture::Clamp);
	_PlayerPosTexW = w;
	_PlayerPosTexH = h;
	_PlayerPosMaterial.setTexture(_PlayerPosTF);
}

//============================================================================================================
void CGroupMap::loadMap()
{
	_MapLoadFailure = true;
	if (!_CurMap) return;
	const std::string &mapName = _CurMap->BitmapName;
	std::string fullName = NLMISC::CPath::lookup(mapName, false, false);
	if (fullName.empty())
	{
		nlwarning("Can't find map %s", mapName.c_str());
		return;
	}
	uint32 w, h;
	NLMISC::CIFile bm;
	if (bm.open(fullName))
	{
		try
		{
			NLMISC::CBitmap::loadSize(fullName, w, h);
		}
		catch(const NLMISC::Exception &e)
		{
			nlwarning(e.what());
			return;
		}
	}
	else
	{
		nlwarning("Can't open map %s", mapName.c_str());
		return;
	}
	_MapTF = Driver->createTextureFile(fullName);
	if (_MapTF)
	{
		_MapMaterial.setTexture(0, _MapTF);
		_MapTF->setWrapS(NL3D::UTexture::Clamp);
		_MapTF->setWrapT(NL3D::UTexture::Clamp);
		_MapTF->setEnlargeCanvasNonPOW2Tex(true);
		if (_IsIsland)
		{
			// island are already pow2 textures
			// compute the real size
			// (1 pixel per metter, hardcoded for now)
			_MapTexW = (uint32) (ISLAND_PIXEL_PER_METER * (_CurMap->MaxX - _CurMap->MinX));
			_MapTexH = (uint32) (ISLAND_PIXEL_PER_METER * (_CurMap->MaxY - _CurMap->MinY));
			if (_MapTexW > w)
			{
				nlwarning("Island real width > texture width, snapshot not up-to-date ? (island = %s)", _CurMap->Name.c_str());
				_MapTexW = w;
			}
			if (_MapTexH > h)
			{
				nlwarning("Island real height > texture height, snapshot not up-to-date ? (island = %s)", _CurMap->Name.c_str());
				_MapTexH = h;
			}
			_URatio = w != 0 ? (float) _MapTexW / w : 0.f;
			_VRatio = h != 0 ? (float) _MapTexH / h : 0.f;
		}
		else
		{
			// must raise to next power of 2 & scale uvs accordingly
			_MapTexW = w;
			_MapTexH = h;
			_URatio = (float) w / (float) NLMISC::raiseToNextPowerOf2(w);
			_VRatio = (float) h / (float) NLMISC::raiseToNextPowerOf2(h);
		}
		_MapLoadFailure = false;
	}
}

//============================================================================================================
void CGroupMap::unloadMap()
{
	// remove texture from the material
	if (_MapTF) Driver->deleteTextureFile(_MapTF);
	_MapTF = NULL;
	if (_PlayerPosTF) Driver->deleteTextureFile(_PlayerPosTF);
	_PlayerPosTF = NULL;
	_MapMaterial.setTexture(0,NULL);
	_MapMaterial.setTexture(1,NULL);
	_MapTexW = 0;
	_MapTexH = 0;
	_MapLoadFailure = false;
}

//============================================================================================================
void CGroupMap::updateContinentInfo()
{
	return;
}


//============================================================================================================
void CGroupMap::setMap(SMap *map)
{
	nlassert(map);
	if ((_CurMap != NULL) && (_CurMap->Name == map->Name)) return;

	// Unload and reset all stuff
	unloadMap();

	_CurMap = map;
	_CurContinent = ContinentMngr.get(_CurMap->ContinentName);
	_MapMinCorner.x = _CurMap->MinX;
	_MapMinCorner.y = _CurMap->MinY;
	_MapMaxCorner.x = _CurMap->MaxX;
	_MapMaxCorner.y = _CurMap->MaxY;
	if (_MapMinCorner.x > _MapMaxCorner.x) std::swap(_MapMinCorner.x, _MapMaxCorner.x);
	if (_MapMinCorner.y > _MapMaxCorner.y) std::swap(_MapMinCorner.y, _MapMaxCorner.y);

	loadMap();
	centerOnPlayer();
	centerOnPlayer();
	invalidateCoords();
	createContinentLandMarks();

	if (_CurContinent != NULL)
		_MapMaterial.setTexture(1, _CurContinent->FoW.Tx);
	else
		_MapMaterial.setTexture(1, NULL);

	// disable the map_back button for islands (islands can't be seen on the world map)
	CInterfaceGroup *gc = getParentContainer();
	if (gc)
	{
		CCtrlBase *mapBack = gc->getCtrl("map_back");
		if (mapBack) mapBack->setActive(!_IsIsland);
	}

	centerOnPlayer();
	setScale(0);
}

//============================================================================================================
void CGroupMap::setMap(const string &mapName)
{
	if ((_CurMap != NULL) && (_CurMap->Name == mapName)) return;

	// Unload and reset all stuff
	unloadMap();

	_IsIsland = false;

	// Acquire new map
	uint32 i;
	for (i = 0; i < _WorldSheet->Maps.size(); ++i)
		if (_WorldSheet->Maps[i].Name == mapName)
			break;

	if (i == _WorldSheet->Maps.size())
	{
		nlwarning("Unknown map to set : %s", mapName.c_str());
		return;
	}
	setMap(&_WorldSheet->Maps[i]);

	centerOnPlayer();
	setScale(0);
}

//============================================================================================================
void CGroupMap::centerOnPlayer()
{
	if (_MapTexW == 0 || _MapTexH == 0) return;

	// Ensure good scale computed
	updateScale();

	// Here, in some case (init, if the map is not displayed), scale is 0. Avoid div by 0.
	float	lx= (_MapTexW * _Scale);
	float	ly= (_MapTexH * _Scale);
	if(lx==0.f) lx= 0.01f;
	if(ly==0.f) ly= 0.01f;
	_WorldOffset.x = (_MapMaxCorner.x - _MapMinCorner.x) * (_MapX - _WReal * 0.5f) / lx;
	_WorldOffset.y = (_MapMaxCorner.y - _MapMinCorner.y) * (- _MapY - _HReal * 0.5f) / ly;

	computeOffsets();
	invalidateCoords();
}

//============================================================================================================
void CGroupMap::setScale(float newUserScale, const NLMISC::CVector2f &/* center */)
{
	NLMISC::clamp(newUserScale, 1.f, computeUserScaleFromRealScale(getActualMaxScale()));

	_UserScale = newUserScale;
	computeOffsets();
	//fitWindow();
	invalidateCoords();
}

//============================================================================================================
void CGroupMap::setScale(float newScale)
{
	sint32 centerX = (2 * (_XReal + _MapX) + std::min(_WReal, _MapW)) / 2;
	sint32 centerY = (2 * (_YReal + _MapY) + std::min(_HReal, _MapH)) / 2;
	NLMISC::CVector2f mapCoords;
	screenToMap(mapCoords, centerX, centerY);
	setScale(newScale, mapCoords);
}


//============================================================================================================
void CGroupMap::updateLandMarkList(TLandMarkButtonVect &lmVect)
{
	uint numLM = (uint)lmVect.size();
	for(uint k = 0; k < numLM; ++k)
	{
		CLandMarkButton *lmb = lmVect[k];
		if (lmb)
		{
			updateButtonPos(*lmb);
		}
	}
}

//============================================================================================================
void CGroupMap::updateLandMarkTextList(TLandMarkTextVect &lmVect)
{
	uint numLM = (uint)lmVect.size();
	for(uint k = 0; k < numLM; ++k)
	{
		CLandMarkText *lmt = lmVect[k];
		if (lmt != NULL)
		{
			sint32 x, y;
			mapToWindowSnapped(x, y, lmt->Pos);
			lmt->setX(x);
			lmt->setY(y);
		}
	}
}

//============================================================================================================
void CGroupMap::removeLandMarks(TLandMarkButtonVect &lm)
{
	uint numLM = (uint)lm.size();
	for(uint k = 0; k < numLM; ++k)
	{
		if (lm[k])
		{
			delCtrl(lm[k]);
		}
	}
	lm.clear();
}

//============================================================================================================
void CGroupMap::createLMWidgets(const std::vector<CContLandMark> &lms)
{
	for (uint32 k = 0; k < lms.size(); ++k)
	{
		const CContLandMark &rCLM =lms[k];

		NLMISC::CVector2f mapPos;
		worldToMap(mapPos, rCLM.Pos);

		const ucstring ucsTmp(CStringManagerClient::getPlaceLocalizedName(rCLM.TitleTextID));
		// Add button if not a region nor a place
		if ((rCLM.Type != CContLandMark::Region) && (rCLM.Type != CContLandMark::Place) &&
			(rCLM.Type != CContLandMark::Street))
		{
			if (rCLM.Type != CContLandMark::Stable)
				addLandMark(_ContinentLM, mapPos, ucsTmp, _ContinentLMOptions);
			else
				addLandMark(_ContinentLM, mapPos, CI18N::get("uiStable"), _ContinentLMOptions);
			_ContinentLM.back()->Type = rCLM.Type;
		}
		else // just add a text
		{
			CLandMarkText *pNewText = new CLandMarkText(CViewBase::TCtorParam());
			pNewText->setText(ucsTmp);
			pNewText->Pos = mapPos;
			pNewText->setParent(this);
			pNewText->setParentPosRef(Hotspot_BL);
			pNewText->setPosRef(Hotspot_MM);
			if (rCLM.Type == CContLandMark::Region)
				pNewText->setFontSize(16);

			pNewText->setColor(CRGBA(255,255,255,255));
			pNewText->setShadow(true);
			pNewText->setShadowColor(CRGBA(0,0,0,255));
			pNewText->setModulateGlobalColor(false);
			pNewText->Type = rCLM.Type;
			_ContinentText.push_back(pNewText);
			addView(pNewText);
		}

		// If the name of the landmark is used as a click zone name of the current map, create a polybutton
		bool bFound = false;
		for (uint i = 0; i < _CurMap->Children.size(); ++i)
		{
			if (rCLM.TitleTextID == _CurMap->Children[i].ZoneName)
			{
				bFound = true;
				break;
			}
		}
		if (bFound)
		{
			CPolyButton *pPB = new CPolyButton;
			pPB->build(rCLM.Zone, this, rCLM.TitleTextID);
			_PolyButtons.push_back(pPB);
			addCtrl(pPB);
		}
	}
}

//============================================================================================================
void CGroupMap::createContinentLandMarks()
{
	uint32 k;

	if (_MapMode != MapMode_Normal) return;
	if (_CurMap == NULL) return;

	// Remove all
	removeLandMarks(_ContinentLM);
	for (k = 0; k < _ContinentText.size(); ++k)
		delView(_ContinentText[k]);
	_ContinentText.clear();
	removeLandMarks(_UserLM);
	for (k = 0; k < _PolyButtons.size(); ++k)
		delCtrl(_PolyButtons[k]);
	_PolyButtons.clear();

	// World map special case
	if (_CurMap->Name == "world")
	{
		createLMWidgets(ContinentMngr.WorldMap);
		invalidateCoords();
		return;
	}

	if (_CurContinent == NULL) return;

	// Continent Landmarks
	createLMWidgets(_CurContinent->ContLandMarks);
	uint nbUserLandMarks = std::min( uint(_CurContinent->UserLandMarks.size()), CContinent::getMaxNbUserLandMarks());
	// User Landmarks
	for(k = 0; k < nbUserLandMarks; ++k)
	{
		NLMISC::CVector2f mapPos;
		worldToMap(mapPos, _CurContinent->UserLandMarks[k].Pos);

		addLandMark(_UserLM, mapPos, _CurContinent->UserLandMarks[k].Title, getUserLandMarkOptions(k));
	}
	invalidateCoords();
}

static void hideTeleportButtonsInPopupMenuIfNotEnoughPriv()
{
	bool showTeleport = (hasPrivilegeDEV() || hasPrivilegeSGM() || hasPrivilegeGM() || hasPrivilegeVG() || hasPrivilegeSG() || hasPrivilegeEM() || hasPrivilegeEG());
	CInterfaceManager *im = CInterfaceManager::getInstance();

	CInterfaceElement *ie = CWidgetManager::getInstance()->getElementFromId("ui:interface:map_menu:teleport");
	if(ie) ie->setActive(showTeleport);

	ie = CWidgetManager::getInstance()->getElementFromId("ui:interface:land_mark_menu:lmteleport");
	if(ie) ie->setActive(showTeleport);

	ie = CWidgetManager::getInstance()->getElementFromId("ui:interface:user_land_mark_menu:lmteleport");
	if(ie) ie->setActive(showTeleport);
}


//============================================================================================================
void CGroupMap::updateUserLandMarks()
{
	uint32 k;

	if (_MapMode != MapMode_Normal) return;
	if (_CurMap == NULL || _CurMap->Name == "world" || _CurContinent == NULL) return;

	// Remove all
	removeLandMarks(_UserLM);

	// Re create User Landmarks
	uint nbUserLandMarks = std::min( uint(_CurContinent->UserLandMarks.size()), CContinent::getMaxNbUserLandMarks());
	for(k = 0; k < nbUserLandMarks; ++k)
	{
		NLMISC::CVector2f mapPos;
		worldToMap(mapPos, _CurContinent->UserLandMarks[k].Pos);

		addLandMark(_UserLM, mapPos, _CurContinent->UserLandMarks[k].Title, getUserLandMarkOptions(k));
	}
	invalidateCoords();

	hideTeleportButtonsInPopupMenuIfNotEnoughPriv();
}


//============================================================================================================
CGroupMap::CLandMarkButton *CGroupMap::createLandMarkButton(const CLandMarkOptions &options)
{
 	CLandMarkButton *lmb = new CLandMarkButton(CViewBase::TCtorParam());
	static int statFool = 0;
	lmb->setId(this->getId()+":lm"+toString(statFool++));
	lmb->setTexture(options.LandMarkTexNormal);
	lmb->setTextureOver(options.LandMarkTexOver);
	lmb->setTexturePushed(options.LandMarkTexPushed);
	lmb->setType(CCtrlButton::PushButton);
	lmb->setActionOnLeftClick("land_mark_selected");
	lmb->setColor(options.ColorNormal);
	lmb->setColorOver(options.ColorOver);
	lmb->setColorPushed(options.ColorPushed);
	lmb->setModulateGlobalColorAll(false);
	if (!options.LandMarkMenu.empty())
	{
		lmb->setActionOnRightClick("active_menu");
		lmb->setParamsOnRightClick(NLMISC::toString("menu=%s", options.LandMarkMenu.c_str()));
	}
	lmb->setPosRef(Hotspot_MM);
	return lmb;
}
//============================================================================================================
void CGroupMap::updateLandMarkButton(CLandMarkButton *lmb, const CLandMarkOptions &options)
{
	lmb->setTexture(options.LandMarkTexNormal);
	lmb->setTextureOver(options.LandMarkTexOver);
	lmb->setTexturePushed(options.LandMarkTexPushed);

	lmb->setColor(options.ColorNormal);
	lmb->setColorOver(options.ColorOver);
	lmb->setColorPushed(options.ColorPushed);
}

//============================================================================================================
void CGroupMap::addLandMark(TLandMarkButtonVect &destList, const NLMISC::CVector2f &pos, const ucstring &title, const CLandMarkOptions &options)
{
	// create a new button and add it to the list
	CLandMarkButton *lmb = createLandMarkButton(options);
	lmb->setParent(this);
	lmb->Pos = pos;
	lmb->setDefaultContextHelp(title);
	destList.push_back(lmb);
	addCtrl(lmb);
}

//============================================================================================================
CCtrlButton *CGroupMap::addUserLandMark(const NLMISC::CVector2f &pos, const ucstring &title, const CUserLandMark::EUserLandMarkType lmType)
{
	if (_CurContinent == NULL) return NULL;
	nlassert(_CurContinent->UserLandMarks.size() == _UserLM.size());
	// add the landmark in the current continent (for later save)
	// keep pos in world
	CUserLandMark ulm;
	mapToWorld(ulm.Pos, pos);
	ulm.Title = title;
	ulm.Type = (uint8)lmType;
	_CurContinent->UserLandMarks.push_back(ulm);

	// add a landmark with a menu to remove it
	addLandMark(_UserLM, pos, title, getUserLandMarkOptions((uint32)_CurContinent->UserLandMarks.size() - 1));

	// Save the config file each time a user landmark is created
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	uint8 currMode = pIM->getMode();
	pIM->saveConfig ("save/interface_" + PlayerSelectedFileName + ".icfg");
	if (currMode != pIM->getMode())
	{
		pIM->setMode(currMode);
	}
	return _UserLM.back();
}

//============================================================================================================
void CGroupMap::removeUserLandMark(CCtrlButton *button)
{
	if (_CurContinent == NULL) return;
	nlassert(_CurContinent->UserLandMarks.size() >= _UserLM.size());
	for(uint k = 0; k < _UserLM.size(); ++k)
	{
		if (_UserLM[k] == button)
		{
			delCtrl(_UserLM[k]);
			_UserLM.erase(_UserLM.begin() + k);
			_CurContinent->UserLandMarks.erase(_CurContinent->UserLandMarks.begin() + k);

			if (_CurContinent->UserLandMarks.size() > _UserLM.size())
			{
				// if user has over the limit landmarks, then rebuild visible markers
				updateUserLandMarks();
			}

			return;
		}
	}
}

//============================================================================================================
void CGroupMap::updateUserLandMark(CCtrlButton *button, const ucstring &newTitle, const CUserLandMark::EUserLandMarkType lmType)
{
	if (_CurContinent == NULL) return;
	nlassert(_CurContinent->UserLandMarks.size() >= _UserLM.size());
	for(uint k = 0; k < _UserLM.size(); ++k)
	{
		if (_UserLM[k] == button)
		{
			_CurContinent->UserLandMarks[k].Title =  newTitle;
			_CurContinent->UserLandMarks[k].Type =  (uint8)lmType;

			updateLandMarkButton(_UserLM[k], getUserLandMarkOptions(k));
			button->setDefaultContextHelp(newTitle);
			return;
		}
	}
}

//============================================================================================================
CUserLandMark CGroupMap::getUserLandMark(CCtrlButton *button) const
{
	CUserLandMark ulm;
	if (_CurContinent == NULL) return ulm;
	nlassert(_CurContinent->UserLandMarks.size() >= _UserLM.size());
	for(uint k = 0; k < _UserLM.size(); ++k)
	{
		if (_UserLM[k] == button)
		{
			return _CurContinent->UserLandMarks[k];
		}
	}

	
	return ulm;
}

//============================================================================================================
uint CGroupMap::getNumUserLandMarks() const
{
	if (_CurContinent == NULL) return 0;
	return (uint)_CurContinent->UserLandMarks.size();
}
//============================================================================================================
CLandMarkOptions CGroupMap::getUserLandMarkOptions(uint32 lmindex) const
{
	if (_CurContinent == NULL || _CurContinent->UserLandMarks.size() < lmindex) 
		return _UserLMOptions;

	CLandMarkOptions clmo(_UserLMOptions);
	clmo.ColorNormal = clmo.ColorOver = clmo.ColorPushed = _CurContinent->UserLandMarks[lmindex].getColor();

	return clmo;

}


//============================================================================================================
void CGroupMap::updatePlayerPos()
{
	if (_MapMode != MapMode_SpawnSquad)
	{
		if (EntitiesMngr.entities().empty()) return;
		if (!EntitiesMngr.entity(0)) return;
		const NLMISC::CVectorD &playerPosD = EntitiesMngr.entity(0)->pos();
		NLMISC::CVector pos((float) playerPosD.x, (float) playerPosD.y, (float) playerPosD.z);
		// convert player pos into pos in map
		worldToMap(_PlayerPos, pos);
		// if player isn't in is last map anymore, see if in another island
		bool isInX = false;
		bool isInY = false;
		if(_CurMap)
		{
			isInX = (_CurMap->MinX <= pos.x) && (pos.x <= _CurMap->MaxX);
			isInY = (_CurMap->MinY <= pos.y) && (pos.y <= _CurMap->MaxY);
		}
		if(!isInX || !isInY)
		{
			_IsIsland = false;
			for(uint k = 0; k < _Islands.size(); ++k)
			{
				if (pos.x >= _Islands[k].MinX &&
					pos.y >= _Islands[k].MinY &&
					pos.x <= _Islands[k].MaxX &&
					pos.y <= _Islands[k].MaxY)
				{
					_IsIsland = true;
					setMap(&_Islands[k]);
					break;
				}
			}
		}
	}
}

//============================================================================================================
/*
void CGroupMap::fitWindow()
{

	if (!EntitiesMngr.entity(0)) return;
	CGroupContainer *parentCont = this->getEnclosingContainer();
	if (!parentCont) return;
	CCtrlBase::updateCoords();
	const NLMISC::CVectorD &playerPosD = EntitiesMngr.entity(0)->pos();
	NLMISC::CVector pos((float) playerPosD.x, (float) playerPosD.y, (float) playerPosD.z);
	// convert player pos into pos in map
	worldToMap(_PlayerPos, pos);
	// x coord
	if (_MapTexW != 0)
	{
		float mapWidthOnScreen = floorf(_MapTexW * _Scale);
		sint32 diffWReal = parentCont->getWReal() - _WReal;
		if ((float)_WReal >= mapWidthOnScreen)
		{
			_Offset.x = - _PlayerPos.x;
			// _W is given by parent, so must update parent W
			if ((float)_WReal > mapWidthOnScreen)
			{
				parentCont->setW((sint32) mapWidthOnScreen + diffWReal);
			}
		}
		else
		{
			NLMISC::clamp(_Offset.x, - _PlayerPos.x, - _PlayerPos.x + 1.f - _WReal / ((float) _MapTexW * _Scale));
		}
		sint32 maxW = (sint32) mapWidthOnScreen;
		sint32 minW = std::min((sint32) mapWidthOnScreen, _MinW);
		parentCont->setMaxW(maxW + diffWReal);
		parentCont->setMinW(minW + diffWReal);
		parentCont->setPopupMaxW(maxW + diffWReal);
		parentCont->setPopupMinW(minW + diffWReal);
		if ((float)_WReal < minW)
		{
			parentCont->setW((sint32) mapWidthOnScreen + diffWReal);
		}
	}
	// y coord
	if (_MapTexH != 0)
	{
		//sint32 diffHReal = parentCont->getHReal() - _HReal;
		float mapHeightOnScreen = floorf(_MapTexH * _Scale);
		// parentCont->setPopupMaxH((sint32) mapHeightOnScreen + diffHReal);
		// parentCont->setPopupMinH((sint32) std::min(_MinH, (sint32) mapHeightOnScreen) + diffHReal);

		if ((float)_HReal >= mapHeightOnScreen)
		{
			_Offset.y = - _PlayerPos.y;
			if ((float)_HReal > mapHeightOnScreen)
			{
				_H = (sint32) mapHeightOnScreen;
			}
		}
		else
		{
			_H = std::max(_H, std::min(_MinH, (sint32) mapHeightOnScreen));
			NLMISC::clamp(_Offset.y, - _PlayerPos.y, - _PlayerPos.y + 1.f - _H / ((float) mapHeightOnScreen));
		}
	}
}
*/

//============================================================================================================
void CGroupMap::evalMapOffset(float userScale, float &scale, sint32 &x, sint32 &y) const
{
	if (_MapTexW == 0 || _MapTexH == 0)
	{
		x = 0;
		y = 0;
		return;
	}
	float scaleX = (float) _WReal / (float) _MapTexW;
	float scaleY = (float) _HReal / (float) _MapTexH;
	scale = std::min(scaleX, scaleY) * userScale;
	// center the map if needed
	sint32 w = (sint32) (scale * _MapTexW);
	x = (_WReal - w) / 2;
	sint32 h = (sint32) (scale * _MapTexH);
	y = (h - _HReal) / 2;
}

//============================================================================================================
float CGroupMap::computeRealScaleFromUserScale(float userScale) const
{
	float scaleX = (float) _WReal / (float) _MapTexW;
	float scaleY = (float) _HReal / (float) _MapTexH;
	return std::min(scaleX, scaleY) * userScale;//
}

//============================================================================================================
float CGroupMap::computeUserScaleFromRealScale(float realScale) const
{
	float scaleX = (float) _WReal / (float) _MapTexW;
	float scaleY = (float) _HReal / (float) _MapTexH;
	if (scaleX <= 0.f || scaleY <= 0.f) return 1.f;
	return realScale / std::min(scaleX, scaleY);
}

//============================================================================================================
void CGroupMap::updateScale()
{
	if (_MapTexW == 0 || _MapTexH == 0)
	{
		_Scale = _UserScale;
		return;
	}
	else
	{
		_Scale = std::min(getActualMaxScale(), computeRealScaleFromUserScale(_UserScale));

		// center the map if needed
		sint32 w = (sint32) (_Scale * _MapTexW);
		_MapW = w;
		_MapX = (_WReal - w) / 2;
		sint32 h = (sint32) (_Scale * _MapTexH);
		_MapY = (h - _HReal) / 2;
		_MapH = h;
	}
	computeOffsets();
}

//=========================================================================================================
void CGroupMap::computeMapRectInsideGroup(sint32 &x, sint32 &y, sint32 &w, sint32 &h) const
{
	x = std::max((sint32) 0, _MapX);
	w = std::min(_WReal, _MapX + _MapW);
	y = std::max((sint32) 0, _MapY);
	h = std::min(_HReal, _MapY + _MapH);
	y = std::max((sint32) 0, (_HReal - _MapY));
}

//=========================================================================================================
void CGroupMap::computeOffsets()
{
	if (_MapTexW == 0 || _MapTexH == 0)
	{
		_WorldOffset.set(0.f, 0.f);
		_Offset.set(0.f, 0.f);
		return;
	}

	// avoid div by 0 (nb: duno if those values can be negative...)
	float	lx= _MapMaxCorner.x - _MapMinCorner.x;
	float	ly= _MapMaxCorner.y - _MapMinCorner.y;
	if(lx==0.f) lx= 0.01f;
	if(ly==0.f) ly= 0.01f;
	_Offset.x= _WorldOffset.x / lx;
	_Offset.y= _WorldOffset.y / ly;

	float startCornerX = _Offset.x + _PlayerPos.x;
	if (_MapX < 0) startCornerX -= _MapX  / (_MapTexW * _Scale);
	float startCornerY = _Offset.y + _PlayerPos.y;
	if (_MapY > 0) startCornerY += _MapY / (_MapTexH * _Scale);
	float endCornerX = startCornerX;
	float endCornerY = startCornerY;
	NLMISC::clamp(endCornerX, 0.f, 1.f - std::min(_MapW, _WReal) / ((float) _MapTexW * _Scale));
	NLMISC::clamp(endCornerY, 0.f, 1.f - std::min(_MapH, _HReal) / ((float) _MapTexH * _Scale));
	_Offset.x += endCornerX - startCornerX;
	_Offset.y += endCornerY - startCornerY;
}

//=========================================================================================================
void CGroupMap::targetLandmark(CCtrlButton *lm)
{
	if (!lm) return;
	// continent landmarks
	CCompassTarget ct;
	bool found = false;
	TLandMarkButtonVect::iterator it = std::find(_ContinentLM.begin(), _ContinentLM.end(),lm);
	{
		if (it != _ContinentLM.end())
		{
			ct.setType(CCompassTarget::ContinentLandMark);
			(*it)->getContextHelp(ct.Name);
			mapToWorld(ct.Pos, (*it)->Pos);
			found = true;
		}
	}
	if (!found)
	{
		// mission landmarks
		it = std::find(_MissionLM.begin(), _MissionLM.end(),lm);
		{
			if (it != _MissionLM.end())
			{
				ct.setPositionState(_MissionPosStates[it - _MissionLM.begin()]);
				(*it)->getContextHelp(ct.Name);
				mapToWorld(ct.Pos, (*it)->Pos);
				found = true;
			}
		}
	}
	if (!found)
	{
		// user landmarks
		it = std::find(_UserLM.begin(), _UserLM.end(),lm);
		{
			if (it != _UserLM.end())
			{
				ct.setType(CCompassTarget::UserLandMark);
				(*it)->getContextHelp(ct.Name);
				mapToWorld(ct.Pos, (*it)->Pos);
				found = true;
			}
		}
	}
	// home
	if (!found)
	{
		if (lm == _HomeLM)
		{
			// pos irrelevant for home, recomputed at each frame
			ct.setType(CCompassTarget::Home);
			ct.Name = CI18N::get("uiHome");
			found = true;
		}
	}
	// respawn point
	if (!found)
	{
		// if in island -> no effect when choosing entry point for now
		it = std::find(_RespawnLM.begin(), _RespawnLM.end(),lm);
		{
			if (it != _RespawnLM.end())
			{
				if (!isIsland())
				{
					ct.setType(CCompassTarget::Respawn);
					(*it)->getContextHelp(ct.Name);
					mapToWorld(ct.Pos, (*it)->Pos);
					found = true;
				}
				// pos irrelevant for respawn point, recomputed at each frame
				if ((_MapMode == MapMode_Death) || (_MapMode == MapMode_SpawnSquad))
				{
					_RespawnSelectedBitmap->setParentPos(*it);
					string sTmp = (*it)->getId();
					sTmp = sTmp.substr(sTmp.rfind('_')+1, sTmp.size());
					fromString(sTmp, _RespawnSelected);
					CInterfaceManager *pIM = CInterfaceManager::getInstance();
					if (_MapMode == MapMode_Death)
						NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:RESPAWN_PT")->setValue32(_RespawnSelected);
					else if (_MapMode == MapMode_SpawnSquad)
					{
						NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:SQUAD_RESPAWN_PT")->setValue32(_RespawnSelected);
						// Close window containing the map
						CInterfaceGroup *pGrp = CWidgetManager::getInstance()->getWindow(this);
						if (pGrp != NULL) pGrp->setActive(false);
					}
					invalidateCoords();
				}
			}
		}
	}
	// target
	if (!found)
	{
		if (lm == _TargetLM)
		{
			// pos irrelevant for respwan, recomputed at each frame
			ct.setType(CCompassTarget::Selection);
			found = true;
		}
	}
	// animals
	if (!found)
	{
		// animals landmarks
		if(_AnimalLM.size() == _AnimalPosStates.size())
		{
			for(uint i=0;i<_AnimalLM.size();i++)
			{
				if(_AnimalLM[i]==lm)
				{
					_AnimalLM[i]->getContextHelp(ct.Name);
					// copy The Animal Pos retriever into the compass
					ct.setPositionState(_AnimalPosStates[i]);
					found = true;
				}
			}
		}
	}
	// teammates
	if (!found)
	{
		// teammates landmarks
		if(_TeammateLM.size() == _TeammatePosStates.size())
		{
			for(uint i=0;i<_TeammateLM.size();i++)
			{
				if(_TeammateLM[i]==lm)
				{
					_TeammateLM[i]->getContextHelp(ct.Name);
					// copy The Animal Pos retriever into the compass
					ct.setPositionState(_TeammatePosStates[i]);
					found = true;
				}
			}
		}
	}

	if (found)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupCompas *gc = dynamic_cast<CGroupCompas *>(CWidgetManager::getInstance()->getElementFromId(_CompassId));
		if (gc)
		{
			gc->setActive(true);
			gc->setTarget(ct);
			gc->blink();
			CWidgetManager::getInstance()->setTopWindow(gc);
		}
	}
}

//=========================================================================================================
void CGroupMap::getLandmarkPosition(const CCtrlButton *lm, NLMISC::CVector2f &worldPos)
{
	if (!lm) return;

	// continent landmarks
	CCompassTarget ct;
	TLandMarkButtonVect::iterator it = std::find(_ContinentLM.begin(), _ContinentLM.end(),lm);
	{
		if (it != _ContinentLM.end())
		{
			mapToWorld(worldPos, (*it)->Pos);
			return;
		}
	}

	// mission landmarks
	it = std::find(_MissionLM.begin(), _MissionLM.end(),lm);
	{
		if (it != _MissionLM.end())
		{
			mapToWorld(worldPos, (*it)->Pos);
			return;
		}
	}

	// user landmarks
	it = std::find(_UserLM.begin(), _UserLM.end(),lm);
	{
		if (it != _UserLM.end())
		{
			mapToWorld(worldPos, (*it)->Pos);
			return;
		}
	}

	// respawn point
	// if in island -> no effect when choosing entry point for now
	it = std::find(_RespawnLM.begin(), _RespawnLM.end(),lm);
	if (it != _RespawnLM.end())
	{
		if (!isIsland())
		{
			mapToWorld(worldPos, (*it)->Pos);
			return;
		}
	}

	worldPos = NLMISC::CVector2f::Null;
}

//=========================================================================================================
void CGroupMap::addRespawnPoints(const CRespawnPointsMsg &rpm)
{
	_RespawnPosReseted = false;
	if (rpm.NeedToReset)
	{
		_RespawnPosReseted = true;
		_RespawnPos.clear();
	}
	for (uint32 i = 0; i < rpm.RespawnPoints.size(); ++i)
		_RespawnPos.push_back(rpm.RespawnPoints[i]);
	// Ensure there is at least one respawn point
//	nlassert(_RespawnPos.size()>0);

	// Choose the good map ! (select the first respawn point and check for first matching bounding box map
	if (_MapMode != MapMode_Death) return;
	if (_RespawnPos.size() == 0) return;

	CWorldSheet *pWS = dynamic_cast<CWorldSheet*>(SheetMngr.get(CSheetId("ryzom.world")));
	if (pWS == NULL) return;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	if (pIM == NULL) return;

	NLMISC::CVector2f rpWorldPos(_RespawnPos[0].x * 0.001f, _RespawnPos[0].y * 0.001f);

	for (uint32 i = 0; i < pWS->Maps.size(); ++i)
	{
		SMap &rMap = pWS->Maps[i];
		if (rMap.ContinentName.empty()) continue;

		if ((rpWorldPos.x >= rMap.MinX) &&
			(rpWorldPos.x <= rMap.MaxX) &&
			(rpWorldPos.y >= rMap.MinY) &&
			(rpWorldPos.y <= rMap.MaxY))
		{
			setMap(rMap.Name);
			break;
		}
	}
}

//=========================================================================================================
void CGroupMap::serialConfig(NLMISC::IStream &f)
{
	sint ver = f.serialVersion(3);
	f.serial(_UserScale);
	if (ver == 0)
	{
		_UserScale = 1.f;
	}

	if (ver < 2)
	{
		// dummy read the old _Offset, and reset to 0.
		f.serial(_Offset);
		_WorldOffset.set(0.f, 0.f);
		_Offset.set(0.f, 0.f);
	}
	else
	{
		f.serial(_WorldOffset);
	}

	// Avoid bad saved WorldOffset
	if(f.isReading())
	{
		if(!isValidDouble(_WorldOffset.x) || !isValidDouble(_WorldOffset.y))
			_WorldOffset.set(0,0);
	}

	// In version 3 we do not read the _H anymore
	if (ver < 3)
	{
		f.serial(_H);
		// Yoyo: patch to avoid old "respawn map hid at second time" bug
		if (f.isReading())
			if (_H < 10)
				_H = 10;
	}
}

//=========================================================================================================
sint32 CGroupMap::getRespawnSelected() const
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf *pNL = NULL;
	if (_MapMode == MapMode_Death)
		pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:RESPAWN_PT",false);
	else if (_MapMode == MapMode_SpawnSquad)
		pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:SQUAD_RESPAWN_PT",false);
	if (pNL != NULL)
		return pNL->getValue32();
	return 0;
}

//=========================================================================================================
void CGroupMap::setRespawnSelected(sint32 nSpawnPointIndex)
{
	if (_RespawnPos.size() == 0) return;
	if (nSpawnPointIndex < 0) return;
	if ((uint32)nSpawnPointIndex >= _RespawnPos.size()) return;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf *pNL = NULL;
	if (_MapMode == MapMode_Death)
		pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:RESPAWN_PT",false);
	else if (_MapMode == MapMode_SpawnSquad)
		pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:SQUAD_RESPAWN_PT",false);
	if (pNL != NULL)
		pNL->setValue32(nSpawnPointIndex);
	_RespawnSelected = nSpawnPointIndex;
	_RespawnPosReseted = false;
}


//=========================================================================================================
SMap *CGroupMap::getParentMap(SMap *map)
{
	if (map == NULL) return NULL;

	for (uint32 i = 0; i < _WorldSheet->Maps.size(); ++i)
	{
		bool bFound = false;
		SMap *pM = &_WorldSheet->Maps[i];
		for (uint32 j = 0; j < pM->Children.size(); ++j)
		{
			if (pM->Children[j].Name == map->Name)
			{
				bFound = true;
				break;
			}
		}
		if (bFound)
			return pM;
	}
	return NULL;
}


/////////////////////
// ACTION HANDLERS //
/////////////////////

//=========================================================================================================
// A land mark button has been pushed
class CAHLandMarkSelected : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* params */)
	{
		CCtrlButton *button = dynamic_cast<CCtrlButton *>(pCaller);
		if (!button) return;
		// Select the landmark as the current target
		CGroupMap *map = dynamic_cast<CGroupMap *>(button->getParent());
		if (!map) return;
		map->targetLandmark(button);
	}
};
REGISTER_ACTION_HANDLER(CAHLandMarkSelected, "land_mark_selected");

//=========================================================================================================
// Remove a user landmark
class CAHRemoveUserLandMark : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* params */)
	{
		CCtrlButton *button = dynamic_cast<CCtrlButton *>(pCaller);
		if (!button) return;
		CGroupMap *map = dynamic_cast<CGroupMap *>(button->getParent());
		if (!map) return;
		map->removeUserLandMark(button);
		// close the rename window & create window
		closeLandMarkNameDialog();
		LastSelectedLandMark = NULL;
	}
};
REGISTER_ACTION_HANDLER(CAHRemoveUserLandMark, "remove_user_landmark");

//=========================================================================================================
// Rename a user land mark
class CAHRenameUserLandMark : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* params */)
	{
		LastSelectedLandMark = dynamic_cast<CCtrlButton *>(pCaller);
		if (!LastSelectedLandMark) return;

		popupLandMarkNameDialog();
	}
};
REGISTER_ACTION_HANDLER(CAHRenameUserLandMark, "rename_user_landmark");


//=========================================================================================================
// Validate user landmark name
class CAHValidateUserLandMarkName : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* params */)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CInterfaceGroup *ig = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(WIN_LANDMARK_NAME));
		if (!ig) return;
		CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(ig->getGroup("eb"));
		if (!eb) return;
		ig->setActive(false);
		
		CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(WIN_LANDMARK_NAME));
		if (!gc) return;
		// Retrieve ComboBox to get the position(ordered landmark type) of the selected item
		CDBGroupComboBox *cb = dynamic_cast<CDBGroupComboBox *>(gc->getGroup("landmarktypes"));

		CUserLandMark::EUserLandMarkType landMarkType = CUserLandMark::Misc;
		sint8 nLandMarkType = cb->getTextId( CDBManager::getInstance()->getDbProp( "UI:TEMP:LANDMARKTYPE" )->getValue8());
		if (nLandMarkType>=0 && nLandMarkType<=CUserLandMark::UserLandMarkTypeCount)
		{
			landMarkType = (CUserLandMark::EUserLandMarkType)nLandMarkType;
		}

		if (LastSelectedLandMark)
		{
			CGroupMap *map = dynamic_cast<CGroupMap *>(LastSelectedLandMark->getParent());
			if (!map) return;
			// update existing landmark
			map->updateUserLandMark(LastSelectedLandMark, eb->getInputString(), landMarkType);
		}
		else
		{
			// create a new landmark
			if (!LastClickedMap) return;
			if( UseUserPositionForLandMark )
			{
				LastClickedMap->addUserLandMark(LastClickedMap->getPlayerPos(), eb->getInputString(), landMarkType);
			}
			else
			{
				LastClickedMap->addUserLandMark(LastClickedMap->getRightClickLastPos(), eb->getInputString(), landMarkType);
			}
			LastClickedMap->invalidateCoords();
		}
	}
};
REGISTER_ACTION_HANDLER(CAHValidateUserLandMarkName, "validate_user_landmark_name");

//=========================================================================================================
void createUserLandMark(CCtrlBase * /* pCaller */, const string &/* params */)
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	// pop the rename dialog
	LastClickedMap = dynamic_cast<CGroupMap *>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
	if (LastClickedMap->isInDeathMode()) return;
	if (LastClickedMap->getNumUserLandMarks() >= CContinent::getMaxNbUserLandMarks() )
	{
		// too many landmark, can't create
		im->displaySystemInfo(CI18N::get("uiNoMoreLandMarks"), "CHK");
		return;
	}
	LastSelectedLandMark = NULL;
	popupLandMarkNameDialog();
}

// create a new landmark after giving its name
class CAHCreateUserLandMark : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &params)
	{
		UseUserPositionForLandMark = false;
		createUserLandMark(pCaller,params);
	}
};
REGISTER_ACTION_HANDLER(CAHCreateUserLandMark, "create_user_landmark");

// create a new landmark at user position after giving its name
class CAHCreateUserLandMarkAtUserPos: public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &params)
	{
		UseUserPositionForLandMark = true;
		createUserLandMark(pCaller,params);
	}
};
REGISTER_ACTION_HANDLER(CAHCreateUserLandMarkAtUserPos, "create_user_landmark_at_user_pos");


//=========================================================================================================
// zoom in the map
class CAHMapZoomIn : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &params)
	{
		std::string map = getParam(params, "map");
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupMap *gm = dynamic_cast<CGroupMap *>(CWidgetManager::getInstance()->getElementFromId(map));
		if (!gm) return;
		NLMISC::CVector2f center;
		gm->windowToMap(center, gm->getWReal() / 2, gm->getHReal() / 2);
		gm->setScale(gm->getScale() * 1.3f, center);
	}
};
REGISTER_ACTION_HANDLER(CAHMapZoomIn, "map_zoom_in");

//=========================================================================================================
// zoom out the map
class CAHMapZoomOut : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &params)
	{
		std::string map = getParam(params, "map");
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupMap *gm = dynamic_cast<CGroupMap *>(CWidgetManager::getInstance()->getElementFromId(map));
		if (!gm) return;
		NLMISC::CVector2f center;
		gm->windowToMap(center, gm->getWReal() / 2, gm->getHReal() / 2);
		gm->setScale(gm->getScale() * 0.7f, center);
	}
};
REGISTER_ACTION_HANDLER(CAHMapZoomOut, "map_zoom_out");

//=========================================================================================================
// center map on the player
class CAHMapCenter : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &params)
	{
		std::string map = getParam(params, "map");
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupMap *gm = dynamic_cast<CGroupMap *>(CWidgetManager::getInstance()->getElementFromId(map));
		if (!gm) return;
		gm->centerOnPlayer();
	}
};
REGISTER_ACTION_HANDLER(CAHMapCenter, "map_center");

//=========================================================================================================
// return to the parent map
class CAHMapBack : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &params)
	{
		std::string map = getParam(params, "map");
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupMap *pGM = dynamic_cast<CGroupMap *>(CWidgetManager::getInstance()->getElementFromId(map));
		if (pGM == NULL) return;
		SMap *pMap = pGM->getParentMap(pGM->getCurMap());
		if (pMap != NULL)
			pGM->setMap(pMap->Name);
	}
};
REGISTER_ACTION_HANDLER(CAHMapBack, "map_back");

//=========================================================================================================
// valid the respawn location selected
class CAHRespawnMapValid : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &params)
	{
		std::string map = getParam(params, "map");
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupMap *gm = dynamic_cast<CGroupMap *>(CWidgetManager::getInstance()->getElementFromId(map));
		if (!gm) return;
		if (gm->getRespawnSelected() == -1) return;

		CBitMemStream out;
		const string msgName = "DEATH:ASK_RESPAWN";
		if (!GenericMsgHeaderMngr.pushNameToStream(msgName, out))
		{
			nlwarning ("don't know message name %s", msgName.c_str());
		}
		else
		{
			uint16 respawnIndex = (uint16)gm->getRespawnSelected();
			out.serial(respawnIndex);
			NetMngr.push(out);
			//nlinfo("impulseCallBack : %s %d sent", msgName.c_str(), respawnIndex);
		}
		/*
			Yoyo: NO!!! don't do this!! leave the DB Player MBEHAV Mode drive this (laggy but correct)....
			Else Errors arise if the client MBEHAV Mode never reset to Normal==1 (for any lag reason)
			The following scenario else could cause a bug:
				- the player is dead, the respawn map is activated
				- the player click "respawn"
				- in the buggy version we close the window immediatly (no lag)
				- the server receive the ASK respawn, resapwn the player, and change the mode to NORMAL==1
				- the server decide that the PLAYER DIES AUTOMATICALLY in the same server tick (gingo in town for instance! :)  )
					NB: even if the DIE arise in following frame, lag and packet loss etc... can still cause problems
				- the server then reset the mode to DEATH
				- the client never receive the change Mode: 0-->1-->0 (always 0), hence the window is not reopened
				- the user cannot reswpan....
			Instead, I chose to hide the timer text in map.xml
		*/
		/*sint64 val = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:CURRENT_SERVER_TICK")->getValue64();
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:RESPAWN:END_DATE")->setValue64(val+10*10);
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:OPEN_RESPAWN_AT_TIME")->setValue64(0);
		// must hide the window which contains this map, not the map itself!!
		CInterfaceGroup		*rootWindow= gm->getRootWindow();
		if(rootWindow)	rootWindow->setActive(false);
		*/
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:RESPAWN:MSG_SENT")->setValue64(1);
	}
};
REGISTER_ACTION_HANDLER(CAHRespawnMapValid, "respawn_map_valid");


//=========================================================================================================
// right click on the map
class CAHWorldMapRightClick : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &params)
	{
		std::string map = getParam(params, "map");
		CInterfaceManager *im = CInterfaceManager::getInstance();

		hideTeleportButtonsInPopupMenuIfNotEnoughPriv();

		CGroupMap *gm = dynamic_cast<CGroupMap *>(CWidgetManager::getInstance()->getElementFromId(map));
		if (!gm) return;
		if (!gm->isIsland())
		{
			CAHManager::getInstance()->runActionHandler("active_menu", pCaller, "menu=ui:interface:map_menu");
		}
		else
		{
			CAHManager::getInstance()->runActionHandler("active_menu", pCaller, "menu=ui:interface:map_menu_island");
		}
	}
};
REGISTER_ACTION_HANDLER(CAHWorldMapRightClick, "world_map_right_click")

//=========================================================================================================
// A land mark button has been pushed
class CAHLandMarkTeleport : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* params */)
	{
		CCtrlButton *button = dynamic_cast<CCtrlButton *>(pCaller);
		if (!button) return;
		// Select the landmark as the current target
		CGroupMap *map = dynamic_cast<CGroupMap *>(button->getParent());
		if (!map) return;
		NLMISC::CVector2f pos;
		map->getLandmarkPosition(button, pos);

		// Check if the pos is ok
		if(pos == NLMISC::CVector2f::Null) return;

		closeLandMarkNameDialog();
		// Remove the selection.
		UserEntity->selection(CLFECOMMON::INVALID_SLOT);
		// Remove the target.
		UserEntity->targetSlot(CLFECOMMON::INVALID_SLOT);

		nlinfo("LM teleport to %f,%f", pos.x, pos.y);
		ICommand::execute(toString("a Position %f,%f", pos.x, pos.y), *InfoLog);
	}
};
REGISTER_ACTION_HANDLER(CAHLandMarkTeleport, "land_mark_teleport");


//=========================================================================================================
// Teleport player to the given location
class CAHMapTeleport : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* params */)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupMap   *clickedMap = dynamic_cast<CGroupMap *>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		closeLandMarkNameDialog();
		NLMISC::CVector2f pos = clickedMap->getRightClickLastPos();
		clickedMap->mapToWorld(pos, pos);
		// Remove the selection.
		UserEntity->selection(CLFECOMMON::INVALID_SLOT);
		// Remove the target.
		UserEntity->targetSlot(CLFECOMMON::INVALID_SLOT);

		nlinfo("teleport to %f,%f", pos.x, pos.y);
		ICommand::execute(toString("a Position %f,%f", pos.x, pos.y), *InfoLog);

	}
};
REGISTER_ACTION_HANDLER(CAHMapTeleport, "map_teleport");

//=========================================================================================================
// update LandMarks Colors
class CUpdateLandMarksColor : public IActionHandler{public:	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)	
{		
	CInterfaceManager *pIM = CInterfaceManager::getInstance();		

	CUserLandMark::_LandMarksColor[CUserLandMark::Misc] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:MISC")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Tribe] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:TRIBE")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Bandit] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:BANDIT")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Citizen] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:CITIZEN")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Fauna] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:FAUNA")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::FaunaExcel] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:FAUNAEXCEL")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::FaunaSup] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:FAUNASUP")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Forage] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:FORAGE")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::ForageExcel] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:FORAGEEXCEL")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::ForageSup] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:FORAGESUP")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Sap] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:SAP")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Amber] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:AMBER")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Node] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:NODE")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Fiber] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:FIBER")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Bark] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:BARK")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Seed] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:SEED")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Shell] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:SHELL")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Resin] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:RESIN")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Wood] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:WOOD")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Oil] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:OIL")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Mission] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:MISSION")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Food] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:FOOD")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Construction] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:CONSTRUCTION")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Goo] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:GOO")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Insect] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:INSECT")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Kitin] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:KITIN")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Nocive] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:NOCIVE")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Preservative] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:PRESERVATIVE")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Passage] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:PASSAGE")->getValueRGBA();
	CUserLandMark::_LandMarksColor[CUserLandMark::Teleporter] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:LANDMARK:COLORS:TELEPORTER")->getValueRGBA();


	
	CGroupMap *pGM = dynamic_cast<CGroupMap *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:map:content:map_content:actual_map"));		
	if (pGM == NULL) return;		
	pGM->updateUserLandMarks();	

}};
REGISTER_ACTION_HANDLER (CUpdateLandMarksColor, "update_landmarks_color");


////////////////////
// DEBUG COMMANDS //
////////////////////


#if !FINAL_VERSION

NLMISC_COMMAND( testMapHome, "Debug : test display of home on map", "" )
{
	if (!args.empty()) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp(COMPASS_DB_PATH ":HOME_POINT")->setValue64((((sint64) 4787 * 1000) << 32 )| (sint64) (uint32) ((sint64) -3661 * 1000));
	return true;
}
/*
NLMISC_COMMAND( testMapRespawn, "Debug : test display of respawn point on map", "" )
{
	if (!args.empty()) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp(COMPASS_DB_PATH ":BIND_POINT")->setValue64((((sint64) 4687 * 1000) << 32 )| (sint64) (uint32) ((sint64) -3561 * 1000));
	return true;
}
*/
NLMISC_COMMAND( testRespawn, "Debug : test respawn map", "" )
{
	if (!args.empty()) return false;

	CRespawnPointsMsg rpm;
	rpm.NeedToReset = true;
	rpm.RespawnPoints.push_back(CRespawnPointsMsg::SRespawnPoint(4150*1000,-4350*1000));
	rpm.RespawnPoints.push_back(CRespawnPointsMsg::SRespawnPoint(4640*1000,-4320*1000));
	rpm.RespawnPoints.push_back(CRespawnPointsMsg::SRespawnPoint(4100*1000,-4120*1000));
	rpm.RespawnPoints.push_back(CRespawnPointsMsg::SRespawnPoint(4050*1000,-4200*1000));
	rpm.RespawnPoints.push_back(CRespawnPointsMsg::SRespawnPoint(4200*1000,-4150*1000));
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupMap *pMap = dynamic_cast<CGroupMap*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:respawn_map:content:map_content:actual_map"));
	if (pMap == NULL)
	{
		nlwarning("problem cannot find ui:interface:respawn_map:content:map_content:actual_map");
		return false;
	}
	pMap->addRespawnPoints(rpm);


	pMap = dynamic_cast<CGroupMap*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:map:content:map_content:actual_map"));
	if (pMap == NULL)
	{
		nlwarning("problem cannot find ui:interface:map:content:map_content:actual_map");
		return false;
	}
	pMap->addRespawnPoints(rpm);

/*
	CInterfaceManager *im = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp(COMPASS_DB_PATH ":BIND_POINT")->setValue64((((sint64) 4687 * 1000) << 32 )| (sint64) (uint32) ((sint64) -3561 * 1000));
*/
	return true;
}

NLMISC_COMMAND( setMap, "Debug : test respawn map", "" )
{
	if (args.size() != 1) return false;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupMap *pMap = dynamic_cast<CGroupMap*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:map:content:map_content:actual_map"));
	if (pMap != NULL)
		pMap->setMap(args[0]);

	return true;
}

#endif

