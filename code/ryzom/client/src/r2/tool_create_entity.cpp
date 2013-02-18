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
#include "editor.h"
#include "tool_create_entity.h"
//
#include "nel/misc/vectord.h"
#include "nel/misc/i18n.h"
//
#include "../client_sheets/character_sheet.h"
//
#include "../entity_cl.h"
#include "../player_r2_cl.h"
#include "../sheet_manager.h"
#include "nel/gui/lua_ihm.h"
#include "nel/misc/cdb_leaf.h"
#include "../interface_v3/interface_manager.h"
#include "dmc/palette.h"
#include "displayer_visual.h"
#include "r2_config.h"
#include "verbose_clock.h"
#include "entity_sorter.h"
//
#include "nel/gui/ctrl_base_button.h"
//
#include "game_share/player_visual_properties.h"
#include "game_share/visual_slot_manager.h"

using namespace NLPACS;
using namespace NLMISC;
using namespace std;

namespace R2
{

// ***************************************************************
CToolCreateEntity::~CToolCreateEntity()
{
	clearArray();
}

// ***************************************************************
void CToolCreateEntity::cancel()
{
	CToolChoosePos::cancel();
	clearArray();
}

// ***************************************************************
CToolCreateEntity::CToolCreateEntity(uint ghostSlot, const std::string &paletteId, bool arrayMode) : CToolChoosePos(ghostSlot)
{
	_PaletteId = paletteId;
	if (!arrayMode)
	{
		enableMultiPos();
	}
	_CreateState = CreateSingle;
	_ArrayOrigin.set(0.f, 0.f, 0.f);
	_ArrayEnd.set(0.f, 0.f, 0.f);
	_ArrayDefaultAngle = 0.f;
	if (arrayMode)
	{
		CObject *paletteNode = getEditor().getDMC().getPaletteElement(paletteId);
		if (paletteNode)
		{
			std::string sheetClient = getString(paletteNode, "SheetClient");
			if (isBotObjectSheet(CSheetId(sheetClient)))
			{
				_CreateState = ChooseArrayOrigin;
			}
		}
	}
	_ArrayWantedAction = ArrayActionNone;
}

// ***************************************************************
void CToolCreateEntity::updateInvalidCursorOnUI()
{
	//H_AUTO(R2_CToolCreateEntity_updateInvalidCursorOnUI)
	// set the default cursor unless the mouse is on the palette
	const std::vector<CInterfaceGroup *> &groups = CWidgetManager::getInstance()->getGroupsUnderPointer();
	for(uint k = 0; k < groups.size(); ++k)
	{
		if (groups[k]->getId() == "ui:interface:r2ed_palette") // hardcoded for now ...
		{
			setMouseCursor(_CursValid);
			return;
		}
	}
	setMouseCursor(DEFAULT_CURSOR);
}

// ***************************************************************
void CToolCreateEntity::commit(const NLMISC::CVector &createPosition, float createAngle)
{
	//H_AUTO(R2_CToolCreateEntity_commit)
	if (_CreateState == ChooseArrayOrigin)
	{
		if (!getEditor().verifyRoomLeft(0, 1))
		{

			CLuaManager::getInstance().executeLuaScript("r2:checkStaticQuota(1)");
			return;
		}
		setContextHelp(CI18N::get("uiR2EDDrawArrayContextHelp"));
		_CreateState = DrawArray;
		_ArrayDefaultAngle = createAngle;
		_ArrayOrigin = createPosition;
		_ArrayEnd = createPosition;
		updateArray(getGhost());
		removeGhostSlot();
		return;
	}

	CEntityCL *ghost = getGhost();
	if (!ghost) return;

	cloneEntityIntoScenario(ghost,
							createPosition,
							createAngle,
							true, /* new action */
							false /* create ghost */);

	if (isMultiPos() && isShiftDown())
	{
		// prevent newly created ghost to be removed twice ...
		removeGhostSlot();
		// re set this tool to generate a new entity look
		CAHManager::getInstance()->runActionHandler("r2ed_create_entity", NULL, "PaletteId=" + _PaletteId);
	}
}

// ***************************************************************
bool CToolCreateEntity::isBotObjectSheet(const NLMISC::CSheetId &sheetId) const
{
	//H_AUTO(R2_CToolCreateEntity_isBotObjectSheet)
	const CCharacterSheet *charSheet = dynamic_cast<const CCharacterSheet *>(SheetMngr.get(sheetId));
	if (charSheet)
	{
		std::string botobjectsPaletteRoot = "palette.entities.botobjects";
		return (_PaletteId.substr(0, botobjectsPaletteRoot.size()) == botobjectsPaletteRoot);
	}
	return false;
}


// ***************************************************************
std::string CToolCreateEntity::cloneEntityIntoScenario(CEntityCL *clonee,
												const NLMISC::CVector &createPosition,
												float createAngle,
												bool  newAction,
												bool  createGhost
											   )
{
	//H_AUTO(R2_CToolCreateEntity_cloneEntityIntoScenario)
	if (!clonee) return "";
	std::string instanceId;
	bool isBotObject = isBotObjectSheet(clonee->sheetId());

	if (!getEditor().verifyRoomLeft(isBotObject ? 0 : 1, isBotObject ? 1 : 0)) { return ""; }

	std::string className;
	// if class is given in the palette node, then use it. Default to 'Npc' else
	CObject *paletteNode = getDMC().getPaletteElement(_PaletteId);
	if (paletteNode && paletteNode->findIndex("Class") != -1)
	{
		className = getString(paletteNode, "Class");
	}
	if (className.empty())
	{
		className = "Npc";
	}

	ucstring readableName;
	// retrieve name from the palette id
	CLuaState &ls = getEditor().getLua();
	getEditor().getEnv()["PaletteIdToTranslation"][_PaletteId].push();
	if (ls.isString(-1))
	{
		readableName.fromUtf8(ls.toString(-1));
	}
	if (readableName.empty())
	{
		// if no name found then give a default one
		readableName = CI18N::get(isBotObject ? "uiR2EDNameBotObject" : "uiR2EDNameNPC");
	}

	// except for creatures, posfix the name with a number
	std::string creaturePaletteRoot = "palette.entities.creatures";
	if (_PaletteId.substr(0, creaturePaletteRoot.size()) != creaturePaletteRoot)
	{
		readableName = getEditor().genInstanceName(readableName);
	}
	else
	{
		className = "NpcCreature";

		// is Plant
		std::string sheetClient = getString(paletteNode, "SheetClient");
		getEditor().getLua().push(sheetClient);
		if (getEditor().getEnv().callMethodByNameNoThrow("isNPCPlant", 1, 1))
		{
			CLuaObject result(getEditor().getLua());
			bool isPlant = result.toBoolean();
			if (isPlant)
				className = "NpcPlant";
		}
	}

	if (newAction)
	{
		getDMC().newAction(NLMISC::CI18N::get("uiR2EDCreateAction") + readableName);
	}
	// send network commands to create entity on server
	std::auto_ptr<CObject> desc(getDMC().newComponent(className));

	if (desc.get())
	{
		// TMP FIX : if the created entity is a custom npc, then retrieve look from the clonee visual properties
		if (className == "NpcCustom")
		{
			SPropVisualA vA;
			SPropVisualB vB;
			SPropVisualC vC;
			const string propNameA = toString("SERVER:Entities:E%d:P%d", clonee->slot(), CLFECOMMON::PROPERTY_VPA);
			const string propNameB = toString("SERVER:Entities:E%d:P%d", clonee->slot(), CLFECOMMON::PROPERTY_VPB);
			const string propNameC = toString("SERVER:Entities:E%d:P%d", clonee->slot(), CLFECOMMON::PROPERTY_VPC);
			CCDBNodeLeaf *leafA = NLGUI::CDBManager::getInstance()->getDbProp(propNameA);
			CCDBNodeLeaf *leafB = NLGUI::CDBManager::getInstance()->getDbProp(propNameB);
			CCDBNodeLeaf *leafC = NLGUI::CDBManager::getInstance()->getDbProp(propNameC);
			if (!leafA)
			{
				nlwarning("Can't find DB leaf %s", propNameA.c_str());
				return "";
			}
			if (!leafB)
			{
				nlwarning("Can't find DB leaf %s", propNameB.c_str());
				return "";
			}
			if (!leafC)
			{
				nlwarning("Can't find DB leaf %s", propNameC.c_str());
				return "";
			}

			vA.PropertyA = leafA->getValue64();
			vB.PropertyB = leafB->getValue64();
			vC.PropertyC = leafC->getValue64();
			nlassert(desc->isTable());
			CObjectTable *props = (CObjectTable *) desc.get();

			props->set("GabaritHeight",     (double)vC.PropertySubData.CharacterHeight);
			props->set("GabaritTorsoWidth", (double)vC.PropertySubData.TorsoWidth);
			props->set("GabaritArmsWidth",  (double)vC.PropertySubData.ArmsWidth);
			props->set("GabaritLegsWidth",  (double)vC.PropertySubData.LegsWidth);
			props->set("GabaritBreastSize", (double)vC.PropertySubData.BreastSize);

			props->set("HairColor", (double)vA.PropertySubData.HatColor);
			props->set("Tattoo",    (double)vC.PropertySubData.Tattoo);
			props->set("EyesColor", (double)vC.PropertySubData.EyesColor);

			props->set("MorphTarget1", (double)vC.PropertySubData.MorphTarget1);
			props->set("MorphTarget2", (double)vC.PropertySubData.MorphTarget2);
			props->set("MorphTarget3", (double)vC.PropertySubData.MorphTarget3);
			props->set("MorphTarget4", (double)vC.PropertySubData.MorphTarget4);
			props->set("MorphTarget5", (double)vC.PropertySubData.MorphTarget5);
			props->set("MorphTarget6", (double)vC.PropertySubData.MorphTarget6);
			props->set("MorphTarget7", (double)vC.PropertySubData.MorphTarget7);
			props->set("MorphTarget8", (double)vC.PropertySubData.MorphTarget8);

			props->set("Sex", (double)vA.PropertySubData.Sex);

			CVisualSlotManager * vsManager = CVisualSlotManager::getInstance();
			NLMISC::CSheetId * sheetId = NULL;

			if(vA.PropertySubData.HatModel == 0)
			{
				props->set("HatModel", 0);
			}
			else
			{
				sheetId = vsManager->index2Sheet((uint32)vA.PropertySubData.HatModel, SLOTTYPE::HEAD_SLOT);
				if (sheetId)
				{
					props->set("HairType",  (double)sheetId->asInt());
				}
			}

			if(vA.PropertySubData.JacketModel == 0)
			{
				props->set("JacketModel", 0);
			}
			else
			{
				sheetId = vsManager->index2Sheet((uint32)vA.PropertySubData.JacketModel, SLOTTYPE::CHEST_SLOT);
				if (sheetId)
				{
					props->set("JacketModel",  (double)sheetId->asInt());
				}
			}

			if(vA.PropertySubData.TrouserModel == 0)
			{
				props->set("TrouserModel", 0);
			}
			else
			{
				sheetId = vsManager->index2Sheet((uint32)vA.PropertySubData.TrouserModel, SLOTTYPE::LEGS_SLOT);
				if (sheetId)
				{
					props->set("TrouserModel",  (double)sheetId->asInt());
				}
			}

			if(vB.PropertySubData.FeetModel == 0)
			{
				props->set("FeetModel", 0);
			}
			else
			{
				sheetId = vsManager->index2Sheet((uint32)vB.PropertySubData.FeetModel, SLOTTYPE::FEET_SLOT);
				if (sheetId)
				{
					props->set("FeetModel",  (double)sheetId->asInt());
				}
			}

			if(vB.PropertySubData.HandsModel == 0)
			{
				props->set("HandsModel", 0);
			}
			else
			{
				sheetId = vsManager->index2Sheet((uint32)vB.PropertySubData.HandsModel, SLOTTYPE::HANDS_SLOT);
				if (sheetId)
				{
					props->set("HandsModel",  (double)sheetId->asInt());
				}
			}

			if(vA.PropertySubData.ArmModel == 0)
			{
				props->set("ArmModel", 0);
			}
			else
			{
				sheetId = vsManager->index2Sheet((uint32)vA.PropertySubData.ArmModel, SLOTTYPE::ARMS_SLOT);
				if (sheetId)
				{
					props->set("ArmModel",  (double)sheetId->asInt());
				}
			}

			double weaponRH=0, weaponLH=0;
			if(vA.PropertySubData.WeaponRightHand == 0)
			{
				props->set("WeaponRightHand", 0);
			}
			else
			{
				sheetId = vsManager->index2Sheet((uint32)vA.PropertySubData.WeaponRightHand, SLOTTYPE::RIGHT_HAND_SLOT);
				if (sheetId)
				{
					weaponRH = (double)sheetId->asInt();
				}
				props->set("WeaponRightHand",  weaponRH);
			}

			if(vA.PropertySubData.WeaponLeftHand == 0)
			{
				props->set("WeaponLeftHand", 0);
			}
			else
			{
				sheetId = vsManager->index2Sheet((uint32)vA.PropertySubData.WeaponLeftHand, SLOTTYPE::LEFT_HAND_SLOT);
				if (sheetId)
				{
					weaponLH = (double)sheetId->asInt();
				}
				props->set("WeaponLeftHand",  weaponLH);
			}

			props->set("JacketColor", (double)vA.PropertySubData.JacketColor);
			props->set("TrouserColor", (double)vA.PropertySubData.TrouserColor);
			props->set("FeetColor", (double)vB.PropertySubData.FeetColor);
			props->set("HandsColor", (double)vB.PropertySubData.HandsColor);
			props->set("ArmColor", (double)vA.PropertySubData.ArmColor);

			CPlayerR2CL * player = (CPlayerR2CL*)dynamic_cast<CPlayerR2CL*>(clonee);
			if(player != NULL)
			{
				std::string race, gender, sheetClient;
				switch(player->people())
				{
				case EGSPD::CPeople::Fyros:
					sheetClient = "basic_fyros_";
					race = "Fyros";
					break;

				case EGSPD::CPeople::Matis:
					sheetClient = "basic_matis_";
					race = "Matis";
					break;

				case EGSPD::CPeople::Tryker:
					sheetClient = "basic_tryker_";
					race = "Tryker";
					break;

				case EGSPD::CPeople::Zorai:
					sheetClient = "basic_zorai_";
					race = "Zorai";
					break;

				default:
					nlwarning("CToolCreateEntity::commit unknown people");
				}
				switch(player->getGender())
				{
				case GSGENDER::female:
					sheetClient = sheetClient+"female.creature";
					gender = "female";
					break;

				case GSGENDER::male:
					sheetClient = sheetClient+"male.creature";
					gender = "male";
					break;

				default:
					nlwarning("CToolCreateEntity::commit unknown gender");
				}

				props->set("SheetClient", sheetClient);

				// random name
				getEditor().getLua().push(race);
				getEditor().getLua().push(gender);
				if (getEditor().getEnv().callMethodByNameNoThrow("randomNPCName", 2, 1))
				{
					CLuaObject result(getEditor().getLua());
					std::string name = result.toString();
					props->set("Name", name);
				}
			}

			getEditor().getLua().push(getString(paletteNode, "Equipment"));
			getEditor().getLua().push(weaponRH);
			getEditor().getLua().push(weaponLH);
			getEditor().getLua().push(getString(paletteNode, "Sheet"));
			getEditor().getLua().push(getString(paletteNode, "SheetModel"));
			if (getEditor().getEnv().callMethodByNameNoThrow("searchSheet", 5, 1))
			{
				CLuaObject result(getEditor().getLua());
				std::string sheet = result.toString();
				props->set("Sheet", sheet);
			}
			else
			{
				nlwarning("SearchSheet failed : Palette Id = %s", _PaletteId.c_str());
				return "";
			}
		}
		else
		{
			desc->set("Name", readableName.toUtf8());
		}

		desc->set("Base", _PaletteId);
		desc->setObject("Position", buildVector(CVectorD(createPosition)));
		desc->set("Angle", createAngle);
		//desc->set("Name", readableName.toUtf8());

		instanceId = getString(desc.get(), "InstanceId");
		if (!instanceId.empty())
		{
			if (!createGhost)
			{
				// selection asked when instance is created
				getEditor().setCookie(instanceId, "Select", true);
			}
			else
			{
				getEditor().setCookie(instanceId, "GhostDuplicate", true);
			}
		}

		// send creation command
		// tmp : static npc counter
		// add in component list of default feature
		if (getEditor().getDefaultFeature())
		{
			std::string targetInstanceId;
			// if object is a bot object, it is considered to be permanent content
			// and should be created in the base act
			CInstance *targetAct =  isBotObject ? getEditor().getBaseAct() : getEditor().getCurrentAct();
			if (!targetAct)
			{
				nlwarning("Can't find act when creating an entity");
			}
			else
			{
				if (_AutoGroup.getGroupingCandidate())
				{
					nlassert(!createGhost); // either autogroup or arraymode, both at the same time not supported
					_AutoGroup.group(desc.get(), createPosition);
				}
				else
				{
					// create standalone
					desc->setGhost(createGhost);
					getDMC().requestInsertNode(getEditor().getDefaultFeature(targetAct)->getId(),
											   "Components",
											   -1,
											   "",
											   desc.get());
				}
			}
		}
	}
	return instanceId;
}

// ***************************************************************
void CToolCreateEntity::onActivate()
{
	//H_AUTO(R2_CToolCreateEntity_onActivate)
	setContextHelp(CI18N::get("uiR2EDToolCreateEntity"));
}

// ***************************************************************
void CToolCreateEntity::updateBeforeRender()
{
	//H_AUTO(R2_CToolCreateEntity_updateBeforeRender)
	if (_CreateState != DrawArray)
	{
		CToolChoosePos::updateBeforeRender();
		_AutoGroup.update(_CreatePosition, _PaletteId, _Valid && !isCtrlDown());
		setContextHelp(CI18N::get(_AutoGroup.getGroupingCandidate() ? "uiR2EDToolCreateEntityAutoGroup" : "uiR2EDToolCreateEntity"));
		return;
	}
	setContextHelp(CI18N::get("uiR2EDDrawArrayContextHelp"));
	// update for array mode
	bool valid = true;
	sint32 mouseX,  mouseY;
	getMousePos(mouseX,  mouseY);
	if (!isInScreen(mouseX,  mouseY) || (isMouseOnUI() && !isMouseOnWorldMap()))
	{
		valid = false;
		_ArrayEnd = _ArrayOrigin;
	}
	//
	CTool::CWorldViewRay worldViewRay;
	computeWorldViewRay(mouseX,  mouseY,  worldViewRay);
	//
	CVector entityPos; // the pos where the ghost will be shown
	CVector inter;     // intersection of view ray with landscape
	_ValidArray = true;
	TRayIntersectionType rayIntersectionType = computeLandscapeRayIntersection(worldViewRay,  inter);
	switch(rayIntersectionType)
	{
		case NoIntersection:
			_ValidArray  = false;
			_ArrayEnd = _ArrayOrigin;
		break;
		case ValidPacsPos:
		case InvalidPacsPos:
			_ArrayEnd = inter;
		break;
	}
	for (uint k = 0; valid && k < _ArrayElements.size(); ++k)
	{
		if (_ArrayElements[k])
		{
			if (_ArrayElements[k]->getDisplayFlag(CDisplayerVisual::FlagBadPos))
			{
				_ValidArray  = false;
			}
		}
	}
	CGroupMap *worldMap = getWorldMap();
	if (worldMap) worldMap->setSelectionAxis(_ValidArray);
	setMouseCursor(_ValidArray ? _CursValid.c_str() : _CursInvalid.c_str());
}

// ***************************************************************
void CToolCreateEntity::updateAfterRender()
{
	//H_AUTO(R2_CToolCreateEntity_updateAfterRender)
	if (_CreateState != DrawArray)
	{
		CToolChoosePos::updateAfterRender();
		return;
	}
	switch(_ArrayWantedAction)
	{
		case ArrayActionNone:
		break;
		case ArrayActionValidate:
		{
			commitArray();
			CTool::TSmartPtr hold(this);
			CAHManager::getInstance()->runActionHandler("r2ed_create_entity", NULL, "PaletteId="+_PaletteId);
			return;
		}
		break;
		case ArrayActionCancel:
		{
			CTool::TSmartPtr hold(this);
			cancel();
			getEditor().setCurrentTool(NULL);
			return;
		}
		break;
	}
	updateArray(NULL);
}


// ***************************************************************
bool CToolCreateEntity::onMouseLeftButtonClicked()
{
	//H_AUTO(R2_CToolCreateEntity_onMouseLeftButtonClicked)
	if (_CreateState != DrawArray)
	{
		return CToolChoosePos::onMouseLeftButtonClicked();
	}
	if (_ValidArray)
	{
		_ArrayWantedAction = ArrayActionValidate;
	}
	return true;
}

// ***************************************************************
bool CToolCreateEntity::onMouseRightButtonClicked()
{
	//H_AUTO(R2_CToolCreateEntity_onMouseRightButtonClicked)
	if (_CreateState != DrawArray)
	{
		return CToolChoosePos::onMouseRightButtonClicked();
	}
	_ArrayWantedAction = ArrayActionCancel;
	return true;
}

// ***************************************************************
void CToolCreateEntity::clearArray()
{
	//H_AUTO(R2_CToolCreateEntity_clearArray)
	for (uint k = 0; k < _ArrayElements.size(); ++k)
	{
		if (_ArrayElements[k])
		{
			getEditor().getDMC().requestEraseNode(_ArrayElements[k]->getDisplayedInstance()->getId(), "", -1);
		}
	}
}

// ***************************************************************
void CToolCreateEntity::updateArray(CEntityCL *clonee)
{
	//H_AUTO(R2_CToolCreateEntity_updateArray)
	if (!clonee)
	{
		nlassert(!_ArrayElements.empty());
		nlassert(_ArrayElements[0] != NULL);
		clonee = _ArrayElements[0]->getEntity();
		if (!clonee)
		{
			return;
		}
	}

	CVector extent = _ArrayEnd - _ArrayOrigin;
	uint arraySize = 1;
	float arrayStepLength = 1.f;
	if (!_ArrayElements.empty() && _ArrayElements[0])
	{
		arrayStepLength = 2.f * _ArrayElements[0]->getSelectionDecalRadius();
		arraySize = (uint) floorf(extent.norm() / arrayStepLength) + 1;
		arraySize = std::min(arraySize, (uint) 16);
	}
	while (!getEditor().verifyRoomLeft(0, arraySize))
	{
		-- arraySize;
		if (arraySize == 0)
		{
			TSmartPtr hold(this);
			cancel();
			return;
		}
	}
	_ArrayElements.resize(std::max(arraySize, uint(_ArrayElements.size())));
	CVector delta = arrayStepLength * extent.normed();
	float angle = _ArrayDefaultAngle;
	if (arraySize > 1)
	{
		angle = - (float) atan2(extent.x, extent.y);
	}
	bool newEntityCreated = false;
	uint numCreatedEntity = 0;
	for (uint k = 0; k < _ArrayElements.size(); ++k)
	{
		CVector pos = _ArrayOrigin + (float) k * delta;
		if (!_ArrayElements[k])
		{
			if (k < arraySize)
			{
				nlwarning("NEW ENTITY");
				// create new element
				std::string instanceId = cloneEntityIntoScenario(clonee,
																 pos,
																 angle,
																 false, /*new action*/
																 true /*create ghost*/);
				CInstance *inst = getEditor().getInstanceFromId(instanceId);
				if (inst)
				{
					_ArrayElements[k] = dynamic_cast<CDisplayerVisualEntity *>(inst->getDisplayerVisual());
					if (_ArrayElements[k])
					{
						_ArrayElements[k]->setDisplayMode(CDisplayerVisual::DisplayModeArray);
					}
				}
				newEntityCreated = true;
				++ numCreatedEntity;
			}
		}

		if (_ArrayElements[k])
		{
			bool active = k < arraySize;
			// do a kind of 'reserve' on the list of entities : don't delete entities in excess, but hide them instead
			if (active != _ArrayElements[k]->getActive())
			{
				_ArrayElements[k]->setActive(active);
				if (active)
				{
					newEntityCreated = true;
					_ArrayElements[k]->setDisplayMode(CDisplayerVisual::DisplayModeArray);
				}
			}
			if (active)
			{
				// update pos & angle
				TInstanceId	instanceId = _ArrayElements[k]->getDisplayedInstance()->getId();
				CVector worldPos = _ArrayElements[k]->getWorldPos();
				if (pos != worldPos)
				{
					CObject *newPos = buildVector(pos, _ArrayElements[k]->getDisplayedInstance()->getPosInstanceId());
					getEditor().getDMC().requestSetNode(instanceId, "Position", newPos);
					delete newPos;
				}
				if (angle != _ArrayElements[k]->getAngle())
				{
					CObjectNumber *angleObject = new CObjectNumber(angle);
					getEditor().getDMC().requestSetNode(instanceId, "Angle", angleObject);
					delete angleObject;
				}
			}
		}
	}
	if (newEntityCreated)
	{
		nlwarning("Num created entity = %d", numCreatedEntity);
		getEditor().getEntitySorter()->clipEntitiesByDist();
	}
}

// ***************************************************************
void CToolCreateEntity::commitArray()
{
	//H_AUTO(R2_CToolCreateEntity_commitArray)
	for (uint k = 0; k < _ArrayElements.size(); ++k)
	{
		if (_ArrayElements[k])
		{
			cloneEntityIntoScenario(_ArrayElements[k]->getEntity(),
									_ArrayElements[k]->getWorldPos(),
									_ArrayElements[k]->getAngle(),
									k == 0, /*new action*/
									false /*create ghost*/);
		}
	}
	clearArray();
	getEditor().getDMC().flushActions();
}

// ***************************************************************
bool CToolCreateEntity::stopAfterCommit() const
{
	//H_AUTO(R2_CToolCreateEntity_stopAfterCommit)
	return _CreateState == CreateSingle;
}



// ***************************************************************
class CAHR2EDToggleDrawArray : public IActionHandler
{
	virtual void execute(CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CCtrlBaseButton *but = dynamic_cast<CCtrlBaseButton *>(pCaller);
		if (but)
		{
			NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:R2_DRAW_ARRAY")->setValueBool(but->getPushed());
			CToolCreateEntity *tce = dynamic_cast<CToolCreateEntity *>(getEditor().getCurrentTool());
			if (tce)
			{
				CAHManager::getInstance()->runActionHandler("r2ed_create_entity", NULL, "PaletteId=" + tce->getPaletteId());
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CAHR2EDToggleDrawArray, "r2ed_toggle_draw_array");




} // R2
