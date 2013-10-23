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
#include "nel/misc/xml_auto_ptr.h"
//
#include "group_compas.h"
#include "interface_3d_scene.h"
#include "../entities.h"
#include "nel/gui/action_handler.h"
#include "group_map.h"
#include "../continent.h"
#include "../continent_manager.h"
#include "interface_manager.h"
#include "../time_client.h"
#include "people_interraction.h"
#include "../net_manager.h"
#include "../string_manager_client.h"
#include "view_radar.h"
#include "../client_cfg.h"
// Game share
#include "game_share/mission_desc.h"
#include "game_share/inventories.h"
#include "game_share/animal_type.h"

extern CEntityManager EntitiesMngr;
extern CContinentManager ContinentMngr;
CCompassDialogsManager * CCompassDialogsManager::_Instance = NULL;

using namespace std;
using namespace NLMISC;
using namespace STRING_MANAGER;

// ***************************************************************************
CCompassTarget::CCompassTarget()
{
	_Type = North;
	Name = CI18N::get("uiNorth");
}


// ***************************************************************************
void CCompassTarget::serial(NLMISC::IStream &f)
{
	if (!f.isReading())
	{
		// some position state may not support 'serial'
		if (_PositionState && !_PositionState->canSave())
		{
			CCompassTarget defaultCompassTarget;
			f.serial(defaultCompassTarget);
			return;
		}
	}
	f.serialCheck(NELID("CTAR"));
	f.serialVersion(0);
	f.serial(Pos);
	// for the name, try to save a string identifier if possible, because language may be changed between
	// save & reload
	f.serial(Name);
	std::string language = strlwr(ClientCfg.LanguageCode);
	f.serial(language);
	f.serialEnum(_Type);
	if (_Type == PosTracker)
	{
		CPositionState *ps = _PositionState;
		if (ps)
		{
			nlwarning("ClassName = %s", ps->getClassName().c_str());
		}
		nlwarning("%s poly ptr", f.isReading() ? "reading" : "writing");
		f.serialPolyPtr(ps);
		_PositionState = ps;
	}
	else
	{
		if (f.isReading())
		{
			_PositionState = NULL;
		}
	}
	f.serialCheck(NELID("_END"));
	// if language has been modified, then we are not able to display correctly the name, so just
	// reset the compass to north to avoid incoherency
	if (f.isReading())
	{
		if (strlwr(ClientCfg.LanguageCode) != language)
		{
			*this = CCompassTarget();
		}
	}
}


// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupCompas, std::string, "compas");

CGroupCompas::CGroupCompas(const TCtorParam &param)
:	CGroupContainer(param)
{
	_ArrowShape = NULL;
	std::fill(_TargetTypeColor, _TargetTypeColor + CCompassTarget::NumTypes, CRGBA::White);
	_Target.setType(CCompassTarget::North);
	_Target.Name = CI18N::get("uiNorth");
	_StartBlinkTime = 0;
	_Blinking = false;
	_NewTargetSelectedColor = CRGBA(255, 127, 0);
	_DistView = NULL;
	_RadarView = NULL;
	_RadarRangeView = NULL;
	_RadarPos = 1; // 50 m
	_DynamicTargetPos = NULL;
	_LastDynamicTargetPos = 0xFFFFFFFF;
	_SavedTargetValid = false;
	_TargetSetOnce = false;
}

// ***************************************************************************
CGroupCompas::~CGroupCompas()
{
}


// ***************************************************************************
bool CGroupCompas::wantSerialConfig() const
{
	// NB when the user first start the game, the target will befirst set to 'chiang the strong' when
	// missions are being received for the first time. If his fail to occur, we just don't
	// save anything
	return _TargetSetOnce;
}

// ***************************************************************************
void CGroupCompas::serialConfig(NLMISC::IStream &f)
{
	f.serialVersion(0);
	if(!f.isReading())
	{
		_SavedTarget = _Target;
	}
	f.serial(_SavedTarget);
	_SavedTargetValid = true;
}


// ***************************************************************************
void CGroupCompas::updateCoords()
{
	CGroupContainer::updateCoords();

	// Set the compas
	if (_ArrowShape == NULL)
	{
		CInterfaceElement *element = getElement (_Id+":arrow3d");
		if (element)
		{
			// 3d scene
			CInterface3DScene *scene = dynamic_cast<CInterface3DScene *>(element);
			if (scene)
			{
				element = scene->getElement (scene->getId()+":arrow");
				if (element)
				{
					// Shape
					_ArrowShape = dynamic_cast<CInterface3DShape *>(element);
				}
			}
		}
	}

	// Get a pointer on the dist view
	if (_DistView == NULL)
	{
		CInterfaceElement *element = getElement(_Id+":dist");
		if (element)
			_DistView = dynamic_cast<CViewText*>(element);
	}

	if (_RadarView == NULL)
	{
		CInterfaceElement *element = getElement(_Id+":visuel:radar");
		if (element)
			_RadarView = dynamic_cast<CViewRadar*>(element);
	}

	if (_RadarRangeView == NULL)
	{
		CInterfaceElement *element = getElement(_Id+":visuel:range");
		if (element)
			_RadarRangeView = dynamic_cast<CViewText*>(element);
	}
}

// ***************************************************************************
void CGroupCompas::blink()
{
	_Blinking = true;
	_StartBlinkTime = TimeInSec;
}

// ***************************************************************************
NLMISC::CVector2f CGroupCompas::getNorthPos(const NLMISC::CVector2f &userPos) const
{
	return NLMISC::CVector2f(userPos.x, userPos.y + 1.f);
}

// ***************************************************************************
void CGroupCompas::draw()
{
	if ((uint) _Target.getType() >= CCompassTarget::NumTypes) return;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	//
	const NLMISC::CVectorD &userPosD = UserEntity->pos();
	NLMISC::CVector userPos((float) userPosD.x, (float) userPosD.y, (float) userPosD.z);
	NLMISC::CVector2f targetPos(0.f, 0.f);
	// if a position tracker is provided, use it
	CCompassTarget displayedTarget = _Target;

	switch(_Target.getType())
	{
		case CCompassTarget::North:
			targetPos.set(userPos.x, userPos.y + 1.f);
		break;
		case CCompassTarget::PosTracker:
		{
			sint32 x = 0, y = 0;
			if (_Target.getPositionState())
			{
				if (_Target.getPositionState()->getPos(x, y))
				{
					targetPos.set(x * 0.001f, y * 0.001f);
				}
			}
			if (x == 0 && y == 0)
			{
				displayedTarget = CCompassTarget();
				targetPos = getNorthPos(userPos);
			}
		}
		break;
		case CCompassTarget::Selection:
		{
			if (UserEntity->selection() != CLFECOMMON::INVALID_SLOT && UserEntity->selection() != 0)
			{
				CEntityCL *sel = EntitiesMngr.entity(UserEntity->selection());
				if (sel != NULL)
				{
					_Target.Name = sel->removeTitleAndShardFromName(sel->getEntityName());
					if (_Target.Name.empty())
					{
						_Target.Name = sel->getTitle();
					}
					targetPos.set((float)sel->pos().x, (float)sel->pos().y);
				}
			}
			else
			{
				displayedTarget = CCompassTarget();
				targetPos = getNorthPos(userPos);
			}
		}
		break;
		case CCompassTarget::Home:
		{
			// get pos
			CCDBNodeLeaf *pos = NLGUI::CDBManager::getInstance()->getDbProp(COMPASS_DB_PATH ":HOME_POINT");
			sint32 px = (sint32) (pos->getValue64() >> 32);
			sint32 py = pos->getValue32();
			if (px != 0 || py != 0)
			{
				targetPos.set(px * 0.001f, py * 0.001f);
			}
			else
			{
				displayedTarget = CCompassTarget();
				targetPos = getNorthPos(userPos);
			}
		}
		break;
		default:
			targetPos = _Target.Pos;
		break;
	}

	CRGBA color = _TargetTypeColor[displayedTarget.getType()];

	// apply blinking color if there's one
	if (_Blinking)
	{
		float dt = (float) (TimeInSec - _StartBlinkTime);
		if (dt >= COMPASS_BLINK_TIME)
		{
			_Blinking = false;
		}
		else
		{
			float blinkAmount;
			if (dt > COMPASS_BLINK_TIME * 0.5f)
			{
				blinkAmount = 2.f * (COMPASS_BLINK_TIME - dt);
			}
			else
			{
				blinkAmount = 2.f * dt;
			}
			color.blendFromui(color, _NewTargetSelectedColor, (uint8) (blinkAmount * 255.f));
		}
	}

	// todo hulud add here guild and bots locations

	if (_ArrowShape)
	{
		// Set the color
		if (_ArrowShape->getShape().getNumMaterials()>0)
			_ArrowShape->getShape().getMaterial(0).setDiffuse(color);

		// Set angle
		const CVector &front = UserEntity->front();
		float myAngle = (float)atan2 (front.y, front.x);
		float deltaX = targetPos.x - userPos.x;
		float deltaY = targetPos.y - userPos.y;
		float targetAngle = (float)atan2 (deltaY, deltaX);
		_ArrowShape->setRotZ ((float)(targetAngle - myAngle)*180.f/(float)Pi);

		// Set the distance text
		if (_DistView)
		{
			bool active = (ContinentMngr.cur() && !ContinentMngr.cur()->Indoor);
			if (_DistView->getActive() != active)
				_DistView->setActive (active);

			if (active)
			{
				float dist = (float)sqrt(deltaX*deltaX+deltaY*deltaY);

				// The text
				char message[50];
				ucstring  distText;
				if (displayedTarget.getType() != CCompassTarget::North)
				{
					if (dist > 999.0f)
					{
						smprintf (message, 50, "%.1f ", dist/1000.0f);
						distText = ucstring (message) + CI18N::get("uiKilometerUnit");
					}
					else
					{
						smprintf (message, 50, "%.0f ", dist);
						distText = ucstring (message) + CI18N::get("uiMeterUnit");
					}
					distText = distText + " - " + displayedTarget.Name;
				}
				else
				{
					distText = displayedTarget.Name;
				}
				if (_DistViewText != distText)
				{
					_DistView->setText(distText);
					_DistViewText = distText;
				}
			}
		}
	}

	CCtrlBase *toolTip = getCtrl("tt");
	if (toolTip)
	{
		ucstring text;
		if (displayedTarget.getType() != CCompassTarget::North)
			toolTip->setDefaultContextHelp(CI18N::get("uittCompassDistance"));
		else
			toolTip->setDefaultContextHelp(text);
	}

	if (displayedTarget.Name != _CurrTargetName)
	{
		_CurrTargetName = displayedTarget.Name;
	}
	CGroupContainer::draw ();
}

// ***************************************************************************

bool CGroupCompas::handleEvent (const NLGUI::CEventDescriptor &event)
{
	if (event.getType() == NLGUI::CEventDescriptor::mouse)
	{
		const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
		//
		if ((_RadarView != NULL) && (_RadarRangeView != NULL))
		{
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousewheel)
			{
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				_RadarPos = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:RADARZOOM")->getValue32();
				if (eventDesc.getWheel() > 0)
				{
					// Zoom out
					if (_RadarPos > 0) _RadarPos--;
				}
				else
				{
					// Zoom in
					if (_RadarPos < 3) _RadarPos++;
				}

				NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:RADARZOOM")->setValue32(_RadarPos);
			}
		}
	}

	return CGroupContainer::handleEvent(event);
}


// ***************************************************************************

void CGroupCompas::setTarget(const CCompassTarget &target)
{
	if (_LastDynamicTargetPos != 0xFFFFFFFF)
	{
		_LastDynamicTargetPos = 0xFFFFFFFF;
		// ask server for no dynamic target update
		CBitMemStream out;
		if (GenericMsgHeaderMngr.pushNameToStream("TARGET:COMPASS_NOT_DYNAMIC", out))
		{
			NetMngr.push(out);
			//nlinfo("impulseCallBack : TARGET:COMPASS_NOT_DYNAMIC sent");
		}
		else
			nlwarning(" unknown message name 'TARGET:COMPASS_NOT_DYNAMIC");
	}

	_Target = target;
	_TargetSetOnce = true;
}


// ***************************************************************************

bool CGroupCompas::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	// Call parent
	CGroupContainer::parse (cur, parentGroup);

	// Look for the arrow shape
	_ArrowShape = NULL;
	//
	CXMLAutoPtr ptr;
	ptr = xmlGetProp (cur, (xmlChar*)"north_color");
	if (ptr) _TargetTypeColor[CCompassTarget::North] = convertColor(ptr);
	//
	ptr = xmlGetProp (cur, (xmlChar*)"selection_color");
	if (ptr) _TargetTypeColor[CCompassTarget::Selection] = convertColor(ptr);
	//
	ptr = xmlGetProp (cur, (xmlChar*)"home_color");
	if (ptr) _TargetTypeColor[CCompassTarget::Home] = convertColor(ptr);
	//
	ptr = xmlGetProp (cur, (xmlChar*)"respawn_color");
	if (ptr) _TargetTypeColor[CCompassTarget::Respawn] = convertColor(ptr);
	//
	ptr = xmlGetProp (cur, (xmlChar*)"continent_landmark_color");
	if (ptr) _TargetTypeColor[CCompassTarget::ContinentLandMark] = convertColor(ptr);
	//
	ptr = xmlGetProp (cur, (xmlChar*)"mission_landmark_color");
	if (ptr) _TargetTypeColor[CCompassTarget::PosTracker] = convertColor(ptr); // TODO : see if more colors are needed for animal, mission target, etc.
	                                                                           // for now same color for dyanmic tracking of position
	//
	ptr = xmlGetProp (cur, (xmlChar*)"user_landmark_color");
	if (ptr) _TargetTypeColor[CCompassTarget::UserLandMark] = convertColor(ptr);
	//
	ptr = xmlGetProp (cur, (xmlChar*)"new_target_selected_color");
	if (ptr) _NewTargetSelectedColor = convertColor(ptr);
	//

	_DynamicTargetPos = NLGUI::CDBManager::getInstance()->getDbProp(COMPASS_DB_PATH ":TARGET");

	return true;
}

// ***************************************************************************
bool	buildCompassTargetFromTeamMember(CCompassTarget &ct, uint teamMemberId)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	CCDBNodeLeaf	*entityNode = NLGUI::CDBManager::getInstance()->getDbProp(toString(TEAM_DB_PATH ":%d:UID", teamMemberId), false);
	CCDBNodeLeaf	*nameNode = NLGUI::CDBManager::getInstance()->getDbProp(toString(TEAM_DB_PATH ":%d:NAME", teamMemberId), false);
	if (nameNode && nameNode->getValueBool() && entityNode && entityNode->getValue32()!=0 && nameNode)
	{
		CSmartPtr<CTeammatePositionState> tracker = new CTeammatePositionState;
		tracker->build(toString(TEAM_DB_PATH ":%d", teamMemberId));
		ct.setPositionState(tracker);

		CStringManagerClient *pSMC = CStringManagerClient::instance();
		ucstring name;
		if (pSMC->getString(nameNode->getValue32(), name))
			ct.Name = CEntityCL::removeTitleAndShardFromName(name); // TODO : dynamic support for name
		else
			ct.Name = CI18N::get("uiNotReceived");
		return true;
	}

	return false;
}


// ***************************************************************************
// 0..MAX_INVENTORY_ANIMAL
 bool	buildCompassTargetFromAnimalMember(CCompassTarget &ct, uint animalMemberId)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	string			dbBase;
	if(animalMemberId<MAX_INVENTORY_ANIMAL+1)
	{
		ANIMAL_TYPE::EAnimalType at;
		at = (ANIMAL_TYPE::EAnimalType)NLGUI::CDBManager::getInstance()->getDbProp("SERVER:PACK_ANIMAL:BEAST"+toString(animalMemberId-1)+":TYPE")->getValue32();
		string sPrefix;
		switch(at)
		{
			default:
			case ANIMAL_TYPE::All:	  sPrefix = "uiPATitleMount";  break;
			case ANIMAL_TYPE::Mount:  sPrefix = "uiPATitleMount";  break;
			case ANIMAL_TYPE::Packer: sPrefix = "uiPATitlePacker"; break;
			case ANIMAL_TYPE::Demon:  sPrefix = "uiPATitleDemon";  break;
		}

		dbBase= toString("SERVER:PACK_ANIMAL:BEAST%d", animalMemberId-1);
		// +1, its normal, see en.uxt
		ct.Name= CI18N::get(sPrefix+toString(animalMemberId));
	}
	else
		return false;

	// get if present or not
	CCDBNodeLeaf	*statusNode = NLGUI::CDBManager::getInstance()->getDbProp(dbBase + ":STATUS", false);
	if (statusNode && ANIMAL_STATUS::isSpawned((ANIMAL_STATUS::EAnimalStatus)statusNode->getValue32()) )
	{
		CSmartPtr<CAnimalPositionState> tracker = new CAnimalPositionState;
		tracker->build(dbBase);
		ct.setPositionState(tracker);
		return true;
	}

	return false;
}


// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupCompasMenu, std::string, "menu_compas");

CGroupCompasMenu::CGroupCompasMenu(const TCtorParam &param)
:	CGroupMenu(param)
{
}

// ***************************************************************************

CGroupCompasMenu::~CGroupCompasMenu()
{
}

// ***************************************************************************
bool CGroupCompasMenu::parse(xmlNodePtr cur, CInterfaceGroup *parent /*=NULL*/)
{
	if (!CGroupMenu::parse(cur, parent)) return false;
	CXMLAutoPtr compass((const char*) xmlGetProp (cur,  (xmlChar*)"compass"));
	if (compass) _TargetCompass = (const char *) compass;
	return true;
}

// Helper for sorting landmarks
static inline bool UserLandMarksSortPredicate(const CUserLandMark& lm1, const CUserLandMark& lm2)
{
	return toLower(lm1.Title) < toLower(lm2.Title);
}

// ***************************************************************************
// Called when we activate the compass menu
void CGroupCompasMenu::setActive (bool state)
{
	if (state && getRootMenu())
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		uint	k;


		// **** First clear entries
		// get first SubMenu.
		uint numLines = getRootMenu()->getNumLines();
		uint firstSubMenu = numLines;
		for(k = 0; k < numLines; ++k)
		{
			if (getRootMenu()->getSubMenu(k))
			{
				firstSubMenu = k;
				break;
			}
		}
		// remove lines until first sub menu
		for(k = 0; k < firstSubMenu; ++k)
		{
			getRootMenu()->removeLine(0);
		}

		// clear all possible targets
		Targets.clear();

		// Get the 3 sub menus now (since not so many lines)
		CGroupSubMenu	*missionSubMenu= NULL;
		CGroupSubMenu	*landMarkSubMenu= NULL;
		CGroupSubMenu	*teamSubMenu= NULL;
		CGroupSubMenu	*animalSubMenu= NULL;
		CGroupSubMenu	*dialogsSubMenu= NULL;
		sint			missionLineIndex= 0;
		sint			landMarkLineIndex= 0;
		sint			teamLineIndex= 0;
		sint			animalLineIndex= 0;
		sint			dialogLineIndex= 0;
		numLines = getRootMenu()->getNumLines();
		for(k=0;k<numLines;k++)
		{
			CGroupSubMenu	*subMenu= getRootMenu()->getSubMenu(k);
			if(subMenu)
			{
				if(getRootMenu()->getLineId(k)=="mission")
					missionLineIndex= k, missionSubMenu= subMenu;
				else if(getRootMenu()->getLineId(k)=="land_mark")
					landMarkLineIndex= k, landMarkSubMenu= subMenu;
				else if(getRootMenu()->getLineId(k)=="team")
					teamLineIndex= k, teamSubMenu= subMenu;
				else if(getRootMenu()->getLineId(k)=="animal")
					animalLineIndex= k, animalSubMenu= subMenu;
				else if(getRootMenu()->getLineId(k)=="dialogs")
					dialogLineIndex= k, dialogsSubMenu= subMenu;
			}
		}


		// **** append North/Target etc...
		{
			uint lineIndex = 0;
			CCompassTarget ct;
			// North
			ct.setType(CCompassTarget::North);
			ct.Name = CI18N::get("uiNorth");
			Targets.push_back(ct);
			getRootMenu()->addLineAtIndex(lineIndex ++, ct.Name, "set_compas", toString ("compass=%s|id=%d|menu=%s", _TargetCompass.c_str(), (int) Targets.size() - 1, _Id.c_str()));
			// Home
			CCDBNodeLeaf *pos = NLGUI::CDBManager::getInstance()->getDbProp(COMPASS_DB_PATH ":HOME_POINT");
			sint32 px = (sint32) (pos->getValue64() >> 32);
			sint32 py = pos->getValue32();
			if (px != 0 || py != 0)
			{
				ct.setType(CCompassTarget::Home);
				ct.Name = CI18N::get("uiHome");
				Targets.push_back(ct);
				getRootMenu()->addLineAtIndex(lineIndex ++,  ct.Name, "set_compas", toString ("compass=%s|id=%d|menu=%s", _TargetCompass.c_str(), (int) Targets.size() - 1, _Id.c_str()));
			}
			// Respawn
			pos = NLGUI::CDBManager::getInstance()->getDbProp(COMPASS_DB_PATH ":BIND_POINT");
			px = (sint32) (pos->getValue64() >> 32);
			py = pos->getValue32();
			if (px != 0 || py != 0)
			{
				ct.setType(CCompassTarget::Respawn);
				ct.Name = CI18N::get("uiRespawn");
				Targets.push_back(ct);
				getRootMenu()->addLineAtIndex(lineIndex ++,  ct.Name, "set_compas", toString ("compass=%s|id=%d|menu=%s", _TargetCompass.c_str(), (int) Targets.size() - 1, _Id.c_str()));
			}

			// As of 6/5/2007 : The option to point the selection is always proposed even if no slot is currently targeted
			//if (UserEntity->selection() != CLFECOMMON::INVALID_SLOT && UserEntity->selection() != 0)
			//{
				/*CEntityCL *entity = EntitiesMngr.entity(UserEntity->selection());
				if (entity != NULL)
				{*/
					//ucstring targetName = CI18N::get("uiTargetTwoPoint") + entity->removeTitleAndShardFromName(entity->getEntityName());
					ucstring targetName = CI18N::get("uiTarget");
					ct.setType(CCompassTarget::Selection);
					ct.Name = targetName;
					Targets.push_back(ct);
					getRootMenu()->addLineAtIndex(lineIndex ++, targetName, "set_compas", toString ("compass=%s|id=%d|menu=%s", _TargetCompass.c_str(), (int) Targets.size() - 1, _Id.c_str()));
				/*}
			}*/

			// increment Menu line indexes.
			missionLineIndex+= lineIndex;
			landMarkLineIndex+= lineIndex;
			teamLineIndex+= lineIndex;
			animalLineIndex+= lineIndex;
			dialogLineIndex+= lineIndex;
		}

		// **** Append Mission in a SubMenu
		// Mission landmarks
		if(missionSubMenu)
		{
			// clear this sub menu
			missionSubMenu->reset();

			// and add each entry
			bool	selectable= false;
			for(uint m = 0; m < 2; ++m) // deals with solo & group missions
			{
				std::string baseDbPath = m == 0 ? MISSIONS_DB_PATH : GROUP_MISSIONS_DB_PATH;
				for(k = 0; k < MAX_NUM_MISSIONS; ++k)
				{
					for(uint l = 0; l <MAX_NUM_MISSION_TARGETS; ++l)
					{
						CCDBNodeLeaf *textIDLeaf = NLGUI::CDBManager::getInstance()->getDbProp(baseDbPath + toString(":%d:TARGET%d:TITLE", (int) k, (int) l), false);
						if (textIDLeaf)
						{
							ucstring name;
							if (CStringManagerClient::instance()->getDynString(textIDLeaf->getValue32(), name))
							{
								CCDBNodeLeaf *leafPosX= NLGUI::CDBManager::getInstance()->getDbProp(baseDbPath +  toString(":%d:TARGET%d:X", (int) k, (int) l), false);
								CCDBNodeLeaf *leafPosY = NLGUI::CDBManager::getInstance()->getDbProp(baseDbPath +  toString(":%d:TARGET%d:Y", (int) k, (int) l), false);
								if (leafPosX && leafPosY)
								{
									CCompassTarget ct;
									CSmartPtr<CNamedEntityPositionState> tracker = new CNamedEntityPositionState;
									tracker->build(textIDLeaf, leafPosX, leafPosY);
									ct.setPositionState(tracker);
									ct.Name = name;
									Targets.push_back(ct);
									missionSubMenu->addLine(ct.Name, "set_compas", toString("compass=%s|id=%d|menu=%s", _TargetCompass.c_str(), (int) Targets.size() - 1, _Id.c_str()));
									selectable= true;
								}
							}
						}
					}
				}
			}
			// Not Empty?
			getRootMenu()->setSelectable(missionLineIndex, selectable);
			getRootMenu()->setGrayedLine(missionLineIndex, !selectable);
		}

		// **** Append LandMarks in a SubMenu
		if(landMarkSubMenu)
		{
			uint numLMLines = landMarkSubMenu->getNumLines();
			uint firstLMSubMenu = numLMLines;
			for(k = 0; k < numLMLines; ++k)
			{
				if (landMarkSubMenu->getSubMenu(k))
				{
					firstLMSubMenu = k;
					break;
				}
			}
			// remove lines until first sub menu
			for(k = 0; k < firstLMSubMenu; ++k)
			{
				landMarkSubMenu->removeLine(0);
			}

			// Get the userlm sub menus now (before adding the continent lm)
			std::vector<CGroupSubMenu*>	landMarkSubMenus;
			landMarkSubMenus.resize(CUserLandMark::UserLandMarkTypeCount);

			for(k=0;k<landMarkSubMenus.size();k++)
			{
				landMarkSubMenus[k] = landMarkSubMenu->getSubMenu(k);
				landMarkSubMenus[k]->reset();
			}

			CContinent *currCont = ContinentMngr.cur();
			bool	selectable= false;
			if (currCont)
			{
				uint contLandMarkIndex = 0;
				// Continent landmarks
				for(k = 0; k < currCont->ContLandMarks.size(); ++k)
				if ((currCont->ContLandMarks[k].Type == CContLandMark::Capital) ||
					(currCont->ContLandMarks[k].Type == CContLandMark::Village))
				{
					CCompassTarget ct;
					ct.setType(CCompassTarget::ContinentLandMark);
					ct.Pos = currCont->ContLandMarks[k].Pos;
					ct.Name = CStringManagerClient::getPlaceLocalizedName(currCont->ContLandMarks[k].TitleTextID);
					Targets.push_back(ct);
					landMarkSubMenu->addLineAtIndex(contLandMarkIndex++, ct.Name, "set_compas", toString("compass=%s|id=%d|menu=%s", _TargetCompass.c_str(), (int) Targets.size() - 1, _Id.c_str()));
					selectable= true;
				}
				// separator?
				if (currCont->ContLandMarks.size() != 0 && currCont->UserLandMarks.size() != 0)
				{
					landMarkSubMenu->addSeparatorAtIndex(contLandMarkIndex++);
				}
				// User landmarks
				uint nbUserLandMarks = std::min( uint(currCont->UserLandMarks.size()), CContinent::getMaxNbUserLandMarks() );

				// Sort the landmarks
				std::vector<CUserLandMark> sortedLandmarks(currCont->UserLandMarks);
				std::sort(sortedLandmarks.begin(), sortedLandmarks.end(), UserLandMarksSortPredicate);

				for(k = 0; k < nbUserLandMarks; ++k)
				{
					if (sortedLandmarks[k].Type < CUserLandMark::UserLandMarkTypeCount)
					{
						CCompassTarget ct;
						ct.setType(CCompassTarget::UserLandMark);
						ct.Pos = sortedLandmarks[k].Pos;
						ct.Name = sortedLandmarks[k].Title;
						Targets.push_back(ct);
						landMarkSubMenus[sortedLandmarks[k].Type]->addLine(ct.Name, "set_compas", toString("compass=%s|id=%d|menu=%s", _TargetCompass.c_str(), (int) Targets.size() - 1, _Id.c_str()));
						selectable= true;
					}
				}

				// Not Empty?
				for(k=0;k<landMarkSubMenus.size();k++)
				{
					landMarkSubMenu->setHiddenLine(k + contLandMarkIndex, landMarkSubMenus[k]->getNumLines() <= 0);
				}

			}

			// Not Empty?
			getRootMenu()->setSelectable(landMarkLineIndex, selectable);
			getRootMenu()->setGrayedLine(landMarkLineIndex, !selectable);
		}

		// **** Append Team Members in a SubMenu
		if(teamSubMenu)
		{
			// clear this sub menu
			teamSubMenu->reset();

			bool	selectable= false;
			for (k = 0; k < MaxNumPeopleInTeam; k++)
			{
				CCompassTarget ct;
				if (buildCompassTargetFromTeamMember(ct, k))
				{
					Targets.push_back(ct);
					teamSubMenu->addLine(ct.Name, "set_compas", toString("compass=%s|id=%d|menu=%s", _TargetCompass.c_str(), (int) Targets.size() - 1, _Id.c_str()));
					selectable= true;
				}
			}

			// Not Empty?
			getRootMenu()->setSelectable(teamLineIndex, selectable);
			getRootMenu()->setGrayedLine(teamLineIndex, !selectable);
		}

		// **** Append Animal Members in a SubMenu
		if(animalSubMenu)
		{
			// clear this sub menu
			animalSubMenu->reset();

			bool	selectable= false;
			for (k = 0; k < 1 + MAX_INVENTORY_ANIMAL+1; k++)
			{
				CCompassTarget ct;
				if (buildCompassTargetFromAnimalMember(ct, k))
				{
					Targets.push_back(ct);
					animalSubMenu->addLine(ct.Name, "set_compas", toString("compass=%s|id=%d|menu=%s", _TargetCompass.c_str(), (int) Targets.size() - 1, _Id.c_str()));
					selectable= true;
				}
			}

			// Not Empty?
			getRootMenu()->setSelectable(animalLineIndex, selectable);
			getRootMenu()->setGrayedLine(animalLineIndex, !selectable);
		}

		// **** Append Dialogs Members in a SubMenu
		if(dialogsSubMenu)
		{
			// clear this sub menu
			dialogsSubMenu->reset();
			bool	selectable= false;

			// and add each entry
			const std::vector<CCompassDialogsManager::CCompassDialogsEntry> & entries = CCompassDialogsManager::getInstance().getEntries();
			const uint size = (uint)entries.size();
			for( uint i = 0; i < size; ++i)
			{
				CCompassTarget ct;
				if (CStringManagerClient::instance()->getDynString(entries[i].Text, ct.Name))
				{
					CSmartPtr<CDialogEntityPositionState> tracker = new CDialogEntityPositionState( i );
					ct.setPositionState(tracker);
					Targets.push_back(ct);
					dialogsSubMenu->addLine(ct.Name, "set_compas", toString("compass=%s|id=%d|menu=%s", _TargetCompass.c_str(), (int) Targets.size() - 1, _Id.c_str()));
					selectable= true;
				}
			}
			// Not Empty?
			getRootMenu()->setSelectable(dialogLineIndex, selectable);
			getRootMenu()->setGrayedLine(dialogLineIndex, !selectable);
		}
	}

	CGroupMenu::setActive (state);
}

// ***************************************************************************
// Called from a compass menu
class CHandlerSetCompas : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		std::string compassID = getParam(sParams, "compass");
		std::string menuID = getParam(sParams, "menu");
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupCompas *gc = dynamic_cast<CGroupCompas *>(CWidgetManager::getInstance()->getElementFromId(compassID));
		if (!gc) return;
		CGroupCompasMenu *gcm = dynamic_cast<CGroupCompasMenu *>(CWidgetManager::getInstance()->getElementFromId(menuID));
		if (!gcm) return;
		int index;
		std::string id = getParam(sParams, "id");
		if (!fromString(id, index)) return;
		if ((uint) index < (uint) gcm->Targets.size())
		{
			gc->setTarget(gcm->Targets[index]);
			gc->setActive(true);
			gc->blink();
			CWidgetManager::getInstance()->setTopWindow(gc);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerSetCompas, "set_compas");


// ***************************************************************************
// Called from a Team member menu
class CHandlerSetTeamCompas : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// get the compass window
		std::string compassID = getParam(sParams, "compass");

		// Get the team member index
		CPeopleList *list;
		uint	peopleIndex;
		if (PeopleInterraction.getPeopleFromCurrentMenu(list, peopleIndex))
		{
			if (list == &PeopleInterraction.TeamList) // check for good list
			{
				// If can CompassTarget this team member
				CCompassTarget	ct;
				if(buildCompassTargetFromTeamMember(ct, peopleIndex))
				{
					CGroupCompas	*gc= dynamic_cast<CGroupCompas*>(CWidgetManager::getInstance()->getElementFromId(compassID));
					if(gc)
					{
						gc->setTarget(ct);
						gc->setActive(true);
						gc->blink();
						CWidgetManager::getInstance()->setTopWindow(gc);
					}
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerSetTeamCompas, "set_team_compas");

// ***************************************************************************
class CHandlerSetCompassNorth : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		std::string compassID = getParam(sParams, "compass");
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupCompas *gc = dynamic_cast<CGroupCompas *>(CWidgetManager::getInstance()->getElementFromId(compassID));
		if (!gc) return;
		gc->setTarget(CCompassTarget());
	}
};
REGISTER_ACTION_HANDLER( CHandlerSetCompassNorth, "set_compass_north");


class CCompassDialogsStringCallback : public IStringWaitCallback
{
	virtual void onDynStringAvailable(uint /* stringId */, const ucstring &value)
	{
		uint size = (uint)CCompassDialogsManager::getInstance()._Entries.size();
		for ( uint i = 0; i < size; i++)
		{
			ucstring name;
			if ( CStringManagerClient::instance()->getDynString(CCompassDialogsManager::getInstance()._Entries[i].Text, name) )
			{
				if ( value == name )
				{
					CCompassDialogsManager::getInstance()._Entries[i] = CCompassDialogsManager::getInstance()._Entries.back();
					CCompassDialogsManager::getInstance()._Entries.pop_back();
					break;
				}
			}
		}

	}
};

void CCompassDialogsManager::removeEntry(uint32 text)
{
	CCompassDialogsStringCallback * callback = new CCompassDialogsStringCallback;
	CStringManagerClient::instance()->waitDynString( text,callback );
}
