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
#include "displayer_visual_entity.h"
#include "displayer_visual_shape.h"
#include "instance.h"
#include "editor.h"
#include "r2_config.h"
//
#include "../entity_cl.h"
#include "../global.h"
#include "../misc.h"
#include "../entities.h"
#include "../pacs_client.h"
#include "nel/misc/cdb_leaf.h"
#include "../interface_v3/interface_manager.h"
#include "entity_sorter.h"
//
#include "verbose_clock.h"
//
#include "nel/misc/sheet_id.h"
#include "nel/misc/line.h"
#include "nel/misc/i18n.h"
#include "nel/misc/command.h"
//
#include "nel/3d/u_bone.h"
#include "nel/3d/u_material.h"
#include "nel/3d/u_particle_system_instance.h"
//
#include "nel/pacs/u_global_retriever.h"
#include "nel/pacs/u_move_primitive.h"
// tmp
#include "nel/3d/scene_user.h"
#include "nel/3d/skeleton_model.h"

#include "../interface_v3/interface_3d_scene.h"
#include "../interface_v3/character_3d.h"
#include "nel/gui/lua_ihm.h"
#include "../pacs_client.h"
#include "../time_client.h"
//
#include "nel/gui/view_renderer.h"
//
#include "../sheet_manager.h"


using namespace NLMISC;
using namespace NL3D;

extern NL3D::UMaterial				GenericMat;

namespace R2
{

static bool DisplayR2EntityBoxes = false;


#define BENCH(name) CPreciseClock clock__##name(#name);

// *********************************************************************************************************

//////////////////////////////
//	CDisplayerVisualEntity  //
//////////////////////////////

// *********************************************************************************************************
CDisplayerVisualEntity::CDisplayerVisualEntity()
{
	_Entity  = NULL;
	_InvertedMatrixTouched = true;
	_Angle = 0.f;
	_SelectionDisplayMode = AutoSelection;
	_RegisteredInEditor = false;
	_ClipBlend = -1.f;
	_PlaceHolder = NULL;
	_CreationTimeSwpBufCount = Driver->getSwapBufferCounter();
}

// *********************************************************************************************************
bool CDisplayerVisualEntity::isCreatedThisFrame() const
{
	return _CreationTimeSwpBufCount == Driver->getSwapBufferCounter();
}

// *********************************************************************************************************
CDisplayerVisualEntity::~CDisplayerVisualEntity()
{
	setActive(false);
	nlassert(_PlaceHolder == NULL);
}

// *********************************************************************************************************
void CDisplayerVisualEntity::deletePlaceHolder()
{
	if (_PlaceHolder)
	{
		delete _PlaceHolder;
		_PlaceHolder = NULL;
	}
}

// *********************************************************************************************************
float CDisplayerVisualEntity::getSelectionDecalRadius() const
{
	if (!_Entity) return 1.f; // place holder case
	//H_AUTO(R2_CDisplayerVisualEntity_getSelectionDecalRadius)
	float radius = getCylinderRadius();
	if (radius != -1.f) return radius;
	NLMISC::CAABBox selectBox = getSelectBox();
	return std::max(selectBox.getHalfSize().x, selectBox.getHalfSize().y);
}


// *********************************************************************************************************
void CDisplayerVisualEntity::onSelect(bool selected)
{
	//H_AUTO(R2_CDisplayerVisualEntity_onSelect)
	bool alreadySelected = getDisplayFlag(FlagSelected);
	if (selected && selected != alreadySelected)
	{
		if (_Entity)
		{
			// force to snap entity to ground here, because pos is bad when calling "getSelectionDecalRadius" and
			// local bbox cache is updated with wrong value
			_Entity->snapToGround();
		}
		if (getActualSelectionDisplayMode() == CircleSelection)
		{
			getEditor().addSelectingDecal(getWorldPos(), getSelectionDecalRadius());
		}
	}
	updateMapDeco();
	CDisplayerVisual::onSelect(selected);
}


// *********************************************************************************************************
void CDisplayerVisualEntity::updateVisibility()
{
	//H_AUTO(R2_CDisplayerVisualEntity_updateVisibility)
	if (!getActive() || !_Entity) return; // not active, so ignore 'VisibleFlag'
	bool visible = getActualVisibility();
	//
	_Entity->displayable(visible);
	if (!visible) _Entity->show(false);
	_Entity->enableInSceneInterface(visible);
	_MapDeco.setActive(visible);
	updateWorldMapPresence();

}


// *********************************************************************************************************
void CDisplayerVisualEntity::showSelected()
{
	//H_AUTO(R2_CDisplayerVisualEntity_showSelected)

	if (!_Entity || getActualSelectionDisplayMode() == CircleSelection)
	{
		getEditor().showSelectDecal(getWorldPos(), getSelectionDecalRadius());
	}
	else
	{
		nlassert(getSelectionType() == LocalSelectBox || getSelectionType() == GroundProjected);
		getEditor().showSelectBox(getSelectBox(), getInvertedMatrix().inverted()/*, getBlinkColor(CV_SelectedInstanceColor.get())*/);
	}
}

// *********************************************************************************************************
void CDisplayerVisualEntity::showHighlighted()
{
	//H_AUTO(R2_CDisplayerVisualEntity_showHighlighted)
	if (!_Entity || getActualSelectionDisplayMode() == CircleSelection)
	{
		getEditor().showHighlightDecal(getWorldPos(), getSelectionDecalRadius());
	}
	else
	{
		getEditor().showHighlightBox(getSelectBox(), getInvertedMatrix().inverted()/*, getBlinkColor(CV_FocusedInstanceColor.get())*/);
	}
}

// *********************************************************************************************************
void CDisplayerVisualEntity::setVisualSelectionBlink(bool on, NLMISC::CRGBA color)
{
	//H_AUTO(R2_CDisplayerVisualEntity_setVisualSelectionBlink)
	if (!_Entity) return;
	_Entity->setVisualSelectionBlink(on, getBlinkColor(color));
	for(uint k = 0; k < _Entity->instances().size(); ++k)
	{
		CEntityCL::SInstanceCL &inst = _Entity->instances()[k];
		if (!inst.Current.empty())
		{
			UParticleSystemInstance ps;
			ps.cast(inst.Current);
			if (!ps.empty())
			{
				ps.setUserColor(on ? color : CRGBA::White);
			}
		}
	}
}

// *********************************************************************************************************
void CDisplayerVisualEntity::onPreRender()
{
	if (_PlaceHolder)
	{
		_PlaceHolder->onPreRender();
		if (!_Entity)
		{
			if (getDisplayFlag(FlagSelected))
			{
				setVisualSelectionBlink(true, getBlinkColor(CV_SelectedInstanceColor.get()));
				showSelected();

			}
			else if (getDisplayFlag(FlagHasFocus))
			{
				setVisualSelectionBlink(true, getBlinkColor(CV_FocusedInstanceColor.get()));
				showHighlighted();
			}
			return;
		}
	}

	//H_AUTO(R2_CDisplayerVisualEntity_onPreRender)
	if (!_Entity) return;

	updateVisibility();
	if (!getActualVisibility()) return;

	testNeedZEval();

	// TODO nico : put this code in base class for use by CDisplayerVisualShape
	if (!_Entity->isAsyncLoading())
	{
		if (getActualDisplayMode() == DisplayModeVisible)
		{
			if (getDisplayFlag(FlagSelected))
			{
				setVisualSelectionBlink(true, getBlinkColor(CV_SelectedInstanceColor.get()));
				showSelected();

			}
			else if (getDisplayFlag(FlagHasFocus))
			{
				setVisualSelectionBlink(true, getBlinkColor(CV_FocusedInstanceColor.get()));
				showHighlighted();
			}
			else
			{
				setVisualSelectionBlink(false, getBlinkColor(CV_UnselectedInstanceColor.get()));
			}
			_Entity->setDiffuse(true, CRGBA::White);
			if (_ClipBlend != 0.f)
			{
				uint32 oldOpacity = CEntityCL::getOpacityMin();
				CEntityCL::setOpacityMin(0);
				_Entity->makeTransparent(_ClipBlend);
				CEntityCL::setOpacityMin(oldOpacity);
			}
			else
			{
				_Entity->makeTransparent(false);
			}
		}
		else
		{
			CRGBA col = getDisplayModeColorInScene();
			_Entity->setDiffuse(true, col);
			CRGBA selCol;
			if (getDisplayFlag(FlagSelected))
			{
				selCol = col;
				showSelected();
			}
			else if (getDisplayFlag(FlagHasFocus))
			{
				selCol.R = (uint8) (3 * (uint) col.R / 4);
				selCol.G = (uint8) (3 * (uint) col.G / 4);
				selCol.B = (uint8) (3 * (uint) col.B / 4);
				selCol.A = (uint8) (3 * (uint) col.A / 4);
				showHighlighted();
			}
			else
			{
				selCol.R = col.R >> 1;
				selCol.G = col.G >> 1;
				selCol.B = col.B >> 1;
				selCol.A = col.A >> 1;
			}
			col.A = (uint8) (_ClipBlend * col.A);
			setVisualSelectionBlink(true, getBlinkColor(selCol));
			uint32 oldOpacity = CEntityCL::getOpacityMin();
			CEntityCL::setOpacityMin((uint32) (DEFAULT_ENTITY_MIN_OPACITY * _ClipBlend));
			_Entity->makeTransparent(col.A != 255);
			CEntityCL::setOpacityMin(oldOpacity);
		}
	}
	//
	_MapDeco.setInvalidPosFlag(getDisplayFlag(FlagBadPos));
}


// *********************************************************************************************************
void CDisplayerVisualEntity::drawBBox(const NLMISC::CMatrix &modelMatrix, const NLMISC::CAABBox &bbox, NLMISC::CRGBA colOverZ, NLMISC::CRGBA colUnderZ)
{
	//H_AUTO(R2_CDisplayerVisualEntity_drawBBox)
	//nlwarning("bbox = (%f, %f, %f) - (%f, %f, %f), color = (%d, %d, %d, %d)", bbox.getMin().x, bbox.getMin().y, bbox.getMin().z,
	//												bbox.getMax().x, bbox.getMax().y, bbox.getMax().z, (int) colOverZ.R, (int) colOverZ.G, (int) colOverZ.B, (int) colOverZ.A);
	// for z-precision, work with camera at (0, 0, 0)
	static volatile bool fixMatrixPos = false;
	CMatrix oldViewMatrix = Driver->getViewMatrix();
	Driver->setModelMatrix(modelMatrix);
	CMatrix viewMat = oldViewMatrix;
	if (fixMatrixPos)
	{
		viewMat.setPos(CVector::Null);
	}
	Driver->setViewMatrix(viewMat);
	// draw below zbuffer
	UMaterial::ZFunc oldZfunc = GenericMat.getZFunc();
	bool oldZWrite = GenericMat.getZWrite();
	GenericMat.setZFunc(UMaterial::greater);
	GenericMat.setZWrite(false);
	::drawBox(bbox.getMin(),   bbox.getMax(), colUnderZ );
	GenericMat.setZFunc(oldZfunc);
	GenericMat.setZWrite(oldZWrite);
	::drawBox(bbox.getMin(),   bbox.getMax(), colOverZ);
	Driver->setViewMatrix(oldViewMatrix);
}

// *********************************************************************************************************
void CDisplayerVisualEntity::drawBBox(NLMISC::CRGBA colOverZ, NLMISC::CRGBA colUnderZ)
{
	//H_AUTO(R2_CDisplayerVisualEntity_drawBBox)
	if (!_Entity) return;
	if (_Entity->isAsyncLoading()) return;
	NLMISC::CAABBox bbox;
	switch(getActualSelectionDisplayMode())
	{
		case CircleSelection:
		{
			// draw real local version
			CMatrix modelMatrix = _Entity->dirMatrix();
			modelMatrix.setPos(_Entity->pos().asVector());
			drawBBox(modelMatrix, getEditor().getLocalSelectBox(*_Entity), colOverZ, colUnderZ);
			// draw world version
			colOverZ.avg2(colOverZ, CRGBA::Black);
			colUnderZ.avg2(colUnderZ, CRGBA::Black);
			drawBBox(CMatrix::Identity, getEditor().getSelectBox(*_Entity), colOverZ, colUnderZ);
		}
		break;
		case BoxSelection:
		{
			// uses local version
			CMatrix modelMatrix = _Entity->dirMatrix();
			modelMatrix.setPos(_Entity->pos().asVector());
			drawBBox(modelMatrix, getEditor().getLocalSelectBox(*_Entity), colOverZ, colUnderZ);
		}
		break;
		default:
			nlassert(0);
		break;
	}
}

// *********************************************************************************************************
void CDisplayerVisualEntity::onPostRender()
{
	if (_PlaceHolder)
	{
		// _PlaceHolder->onPostRender();  // do not post render here becauseit would set transparency again, bypassing the changed in 'setClipBlend'
	}
	//H_AUTO(R2_CDisplayerVisualEntity_onPostRender)
	if (!_Entity) return;
	if (!getActualVisibility()) return;

	if (getDisplayFlag(FlagSelected))
	{
		// draw a bbox around the entity
		if (R2::DisplayR2EntityBoxes)
		{
			drawBBox(CRGBA::Green, CRGBA(127, 127, 127, 255));
		}
	}
	else if (getDisplayFlag(FlagHasFocus))
	{
		if (R2::DisplayR2EntityBoxes)
		{
			drawBBox(CRGBA::White, CRGBA(127, 127, 127, 255));
		}
	}
	static volatile bool drawBoneSphere = false;
	if (drawBoneSphere)
	{
		if (_Entity->skeleton())
		{
			CSkeletonModel *sm = _Entity->skeleton()->getObjectPtr();
			if (sm)
			{
				Driver->setModelMatrix(CMatrix::Identity);
				std::vector<CBSphere> boneSpheres;
				sm->getWorldMaxBoneSpheres(boneSpheres);
				for(uint k = 0; k < boneSpheres.size(); ++k)
				{
					drawSphere(boneSpheres[k].Center, boneSpheres[k].Radius, CRGBA::Red);
				}
			}
		}
	}
	CDisplayerVisual::onPostRender();
}

// *********************************************************************************************************
void CDisplayerVisualEntity::setActive(bool active)
{
	//H_AUTO(R2_CDisplayerVisualEntity_setActive)
	if (active && !getActive())
	{
		if (getEditor().getEntitySorter())
		{
			_CreationTimeSwpBufCount = Driver->getSwapBufferCounter();
			_ClipBlend = -1.f;
			getEditor().getEntitySorter()->registerEntityDisplayer(this);
		}
		_RegisteredInEditor = true;
		updateWorldMapPresence();
	}
	if (!active && getActive())
	{
		if (getEditor().getEntitySorter())
		{
			getEditor().getEntitySorter()->unregisterEntityDisplayer(this);
		}
		eraseEntity();
		// in map
		if (_MapDeco.isAddedToMap())
		{
			CGroupMap *gm = CTool::getWorldMap();
			if (gm) gm->removeDeco(&_MapDeco);
		}
		deletePlaceHolder();
		_RegisteredInEditor = false;
	}
}


// *********************************************************************************************************
void CDisplayerVisualEntity::setClipBlend(float amount)
{
	if (_ClipBlend == amount) return;

	//CPreciseClock cl("CDisplayerVisualEntity::setClipBlend");

	_ClipBlend = amount;
	bool visible = _ClipBlend != 0.f;
	if (!_Entity && visible)
	{
		createEntity();
	}
	if (!visible)
	{
		eraseEntity();
	}
	if (amount == 1.f)
	{
		deletePlaceHolder();
	}
	else
	{
		// display the place holder
		if (!_PlaceHolder)
		{
			static volatile float scale = 1.f;
			_PlaceHolder = new CDisplayerVisualShape("r2_entity_place_holder.shape", scale, false);
			R2::setDisplayedInstance(_PlaceHolder, getDisplayedInstance());
			_PlaceHolder->onPostCreate();
			((CDisplayerVisual *) _PlaceHolder)->setActive(true);
			((CDisplayerVisual *) _PlaceHolder)->updateWorldPos();
			_PlaceHolder->onPreRender(); // force to create the instance so we can make it transparent at start
		}
		if (_PlaceHolder && !_PlaceHolder->getMesh().empty())
		{
			makeInstanceTransparent(_PlaceHolder->getMesh(), (uint8) (255.f * (1.f - amount)), amount > 0.5f);
		}
	}
}


// *********************************************************************************************************
void CDisplayerVisualEntity::createEntity()
{
	//H_AUTO(R2_CDisplayerVisualEntity_createEntity)
	// in scene
	nlassert(!_Entity); // framework should never call this twice in a row
	// assume that pos & rot have been set
	uint k = 2;
	for (; k < 255; ++k) // find an empty slot
	{
		if (!EntitiesMngr.entity(k))
		{
			nlwarning("Creating entity in slot %d with sheet %s at position %s with angle %f",
					   (int) k,
					   getSheet().c_str(),
					   toString(getPos().asVector()).c_str(),
					   getAngle());
			_Entity = CEditor::createEntity(k,  NLMISC::CSheetId(getSheet()),  getWorldPos(),  getAngle(), getPermanentStatutIcon());

			break;
		}
	}

	if (!_Entity)
	{
		nlwarning("Entity creation failed. This would normally not happeneds. Sheet '%s', slot %u ", getSheet().c_str(), k);
		return;
	}
	//
	updateName();
	updateEntity();
	_Entity->updateVisible(T1,  NULL);
	_Entity->updatePos(T1,  NULL);
}

// *********************************************************************************************************
void CDisplayerVisualEntity::updateWorldMapPresence()
{
	//H_AUTO(R2_CDisplayerVisualEntity_updateWorldMapPresence)
	if (!getActive()) return;
	// if not in current map then remove
	// if entity not in that map then ignore
	bool inIsland = false;
	CIslandCollision &col = getEditor().getIslandCollision();
	R2::CScenarioEntryPoints::CCompleteIsland	*currIsland = col.getCurrIslandDesc();
	if (currIsland)
	{
		inIsland = currIsland->isIn(getWorldPos2f());
	}
	// in map (displayed only if selectable for now)
	if (getDisplayedInstance()->getSelectable() && inIsland)
	{
		if (!_MapDeco.isAddedToMap())
		{
			CGroupMap *gm = CTool::getWorldMap();
			if (gm)
			{
				_MapDeco.setDisplayedInstance(getDisplayedInstance(), true);
				// retrieve icon from the displayed object (lua code)
				CLuaState &ls = getEditor().getLua();
				std::string texName;
				{
					CLuaStackChecker lsc(&ls);
					if (getDisplayedInstance()->getLuaProjection().callMethodByNameNoThrow("getSelectBarIcon", 0, 1))
					{
						texName = ls.toString(-1);
						ls.pop();
					}
				}
				gm->addDeco(&_MapDeco);
				_MapDeco.setCloseTexture(texName);
				_MapDeco.invalidateCoords();
			}
		}
	}
	else
	{
		if (_MapDeco.isAddedToMap())
		{
			CGroupMap *gm = CTool::getWorldMap();
			if (gm)
			{
				gm->removeDeco(&_MapDeco);
			}
		}
	}
}

// *********************************************************************************************************
bool CDisplayerVisualEntity::getActive() const
{
	return _RegisteredInEditor;
}

// *********************************************************************************************************
void CDisplayerVisualEntity::updateWorldPos()
{
	if (_PlaceHolder)
	{
		((CDisplayerVisual *) _PlaceHolder)->updateWorldPos();
	}
	//H_AUTO(R2_CDisplayerVisualEntity_updateWorldPos)
	CDisplayerVisual::updateWorldPos();
	updateEntityWorldPos();
	snapToGround();
	updateMapDeco();
}

// *********************************************************************************************************
/*void CDisplayerVisualEntity::updateValidPosFlag()
{
	// eval if current pos is valid pacs pos
	if (!GR)
	{
		return;
	}
	NLPACS::UGlobalPosition gpos = GR->retrievePosition(getWorldPos().asVector());
	setDisplayFlag(FlagBadPos,    gpos.InstanceId == -1);
}*/

// *********************************************************************************************************
void CDisplayerVisualEntity::updateMapDeco()
{
	//H_AUTO(R2_CDisplayerVisualEntity_updateMapDeco)
	if (_MapDeco.isAddedToMap())
	{
		CGroupMap *gm = CTool::getWorldMap();
		if (gm)
		{
			_MapDeco.onUpdate(*gm);
			_MapDeco.invalidateCoords();
		}
	}
}

// *********************************************************************************************************
void CDisplayerVisualEntity::onFocus(bool focused)
{
	//H_AUTO(R2_CDisplayerVisualEntity_onFocus)
	CDisplayerVisual::onFocus(focused);
	updateMapDeco();
}

// *********************************************************************************************************
void CDisplayerVisualEntity::eraseEntity()
{
	//H_AUTO(R2_CDisplayerVisualEntity_eraseEntity)
	// in scene
	if (_Entity)
	{
		EntitiesMngr.remove(_Entity->slot(), true);
		_Entity = NULL;
	}
}

// *********************************************************************************************************
void CDisplayerVisualEntity::onCreate()
{
	//H_AUTO(R2_CDisplayerVisualEntity_onCreate)
	CDisplayerVisual::onCreate();
	retrieveAngle();

}

// *********************************************************************************************************
void CDisplayerVisualEntity::onPostCreate()
{
	CDisplayerVisual::onPostCreate();
}

// *********************************************************************************************************
bool CDisplayerVisualEntity::isUserCreated() const
{
	return getDisplayedInstance()->getLuaProjection()["User"]["Select"].toBoolean() == true;
}


// *********************************************************************************************************
void CDisplayerVisualEntity::onAttrModified(const std::string &name, sint32 index)
{
	if (_PlaceHolder)
	{
		_PlaceHolder->onAttrModified(name, index);
	}
	//H_AUTO(R2_CDisplayerVisualEntity_onAttrModified)
	CDisplayerVisual::onAttrModified(name, index);
	if (name == "Angle")
	{
		retrieveAngle();
		updateEntityRot();
		updateMapDeco();
	}
	else
	if (name == "Name")
	{
		updateName();
	}
	else if (name == "GabaritHeight" || name == "GabaritTorsoWidth" || name == "GabaritArmsWidth"
		  || name == "GabaritLegsWidth" || name == "GabaritBreastSize"
		  || name == "HairType" || name == "HairColor" || name == "Tattoo" || name == "EyesColor"
		  || name == "MorphTarget1" || name == "MorphTarget2" || name == "MorphTarget3"	|| name == "MorphTarget4"
		  || name == "MorphTarget5" || name == "MorphTarget6" || name == "MorphTarget7"	|| name == "MorphTarget8"
		  || name == "JacketModel" || name == "TrouserModel" || name == "FeetModel" || name == "HandsModel"
		  || name == "ArmModel" || name == "WeaponRightHand" || name == "WeaponLeftHand"
		  || name == "JacketColor" || name == "ArmColor" || name == "HandsColor"
		  || name == "TrouserColor" || name == "FeetColor")

	{
		updateEntity();
	}
	else if (name == "SheetClient")
	{
		updateRaceAndSex();
	}
	else if (name == "Selectable")
	{
		updateWorldMapPresence();
	}
	/*else if (name == "DisplayMode")
	{
		updateMapDeco();
	}*/
}

// *********************************************************************************************************
void CDisplayerVisualEntity::setDisplayMode(sint32 mode)
{
	//H_AUTO(R2_CDisplayerVisualEntity_setDisplayMode)
	CDisplayerVisual::setDisplayMode(mode);
	updateMapDeco();
}

// *********************************************************************************************************
void CDisplayerVisualEntity::onParentDisplayModeChanged()
{
	//H_AUTO(R2_CDisplayerVisualEntity_onParentDisplayModeChanged)
	updateMapDeco();
}

// *********************************************************************************************************

void CDisplayerVisualEntity::updateRaceAndSex()
{
	//H_AUTO(R2_CDisplayerVisualEntity_updateRaceAndSex)
	std::string sheetClient = (std::string) getString(&getProps(), "SheetClient");

	_Entity = getEditor().createEntity(_Entity->slot(), CSheetId(sheetClient), getPos().asVector(), getAngle());
	updateEntity();

	// update sex
	SPropVisualA vA;
	const string propNameA = toString("SERVER:Entities:E%d:P%d", _Entity->slot(), CLFECOMMON::PROPERTY_VPA);
	CCDBNodeLeaf *leafA = NLGUI::CDBManager::getInstance()->getDbProp(propNameA);
	if (!leafA)
	{
		nlwarning("Can't find DB leaf %s", propNameA.c_str());
		return;
	}
	vA.PropertyA = leafA->getValue64();

	string::size_type loc = sheetClient.find("female", 0);
	vA.PropertySubData.Sex = (uint)(loc != string::npos);

	EntitiesMngr.updateVisualProperty(0, _Entity->slot(), CLFECOMMON::PROPERTY_VPA);
}

// *********************************************************************************************************

void CDisplayerVisualEntity::updateEntity()
{
	{
		//BENCH(updateName)
		//H_AUTO(R2_CDisplayerVisualEntity_updateEntity)
		updateName();
	}

	CCDBNodeLeaf *leafA;
	CCDBNodeLeaf *leafB;
	CCDBNodeLeaf *leafC;
	SPropVisualA vA;
	SPropVisualB vB;
	SPropVisualC vC;
	{
		//BENCH(entitySetup)

		const string propNameA = toString("SERVER:Entities:E%d:P%d", _Entity->slot(), CLFECOMMON::PROPERTY_VPA);
		const string propNameB = toString("SERVER:Entities:E%d:P%d", _Entity->slot(), CLFECOMMON::PROPERTY_VPB);
		const string propNameC = toString("SERVER:Entities:E%d:P%d", _Entity->slot(), CLFECOMMON::PROPERTY_VPC);
		leafA = NLGUI::CDBManager::getInstance()->getDbProp(propNameA);
		leafB = NLGUI::CDBManager::getInstance()->getDbProp(propNameB);
		leafC = NLGUI::CDBManager::getInstance()->getDbProp(propNameC);
		if (!leafA)
		{
			nlwarning("Can't find DB leaf %s", propNameA.c_str());
			return;
		}
		if (!leafB)
		{
			nlwarning("Can't find DB leaf %s", propNameB.c_str());
			return;
		}
		if (!leafC)
		{
			nlwarning("Can't find DB leaf %s", propNameC.c_str());
			return;
		}

		vA.PropertyA = leafA->getValue64();
		vB.PropertyB = leafB->getValue64();
		vC.PropertyC = leafC->getValue64();

		vC.PropertySubData.CharacterHeight	= (uint) getNumber(&getProps(), "GabaritHeight");
		vC.PropertySubData.ArmsWidth		= (uint) getNumber(&getProps(), "GabaritArmsWidth");
		vC.PropertySubData.TorsoWidth		= (uint) getNumber(&getProps(), "GabaritTorsoWidth");
		vC.PropertySubData.LegsWidth		= (uint) getNumber(&getProps(), "GabaritLegsWidth");
		vC.PropertySubData.BreastSize		= (uint) getNumber(&getProps(), "GabaritBreastSize");

		int itemNb = (int) getNumber(&getProps(), "HairType");
		std::string itemFileName;
		if(itemNb>0)
		{
			itemFileName = CSheetId(itemNb).toString();
			vA.PropertySubData.HatModel		= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::HEAD_SLOT);
		}
		else
		{
			vA.PropertySubData.HatModel = 0;
		}
		vA.PropertySubData.HatColor		= (uint) getNumber(&getProps(), "HairColor");
		vC.PropertySubData.Tattoo		= (uint) getNumber(&getProps(), "Tattoo");
		vC.PropertySubData.EyesColor	= (uint) getNumber(&getProps(), "EyesColor");

		vC.PropertySubData.MorphTarget1 = (uint) getNumber(&getProps(), "MorphTarget1");
		vC.PropertySubData.MorphTarget2 = (uint) getNumber(&getProps(), "MorphTarget2");
		vC.PropertySubData.MorphTarget3 = (uint) getNumber(&getProps(), "MorphTarget3");
		vC.PropertySubData.MorphTarget4 = (uint) getNumber(&getProps(), "MorphTarget4");
		vC.PropertySubData.MorphTarget5 = (uint) getNumber(&getProps(), "MorphTarget5");
		vC.PropertySubData.MorphTarget6 = (uint) getNumber(&getProps(), "MorphTarget6");
		vC.PropertySubData.MorphTarget7 = (uint) getNumber(&getProps(), "MorphTarget7");
		vC.PropertySubData.MorphTarget8 = (uint) getNumber(&getProps(), "MorphTarget8");

		itemNb = (int) getNumber(&getProps(), "JacketModel");
		if(itemNb>0)
		{
			itemFileName = CSheetId(itemNb).toString();
			vA.PropertySubData.JacketModel = (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::CHEST_SLOT);
		}
		else
		{
			vA.PropertySubData.JacketModel = 0;
		}

		itemNb = (int) getNumber(&getProps(), "TrouserModel");
		if(itemNb>0)
		{
			itemFileName = CSheetId(itemNb).toString();
			vA.PropertySubData.TrouserModel	= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::LEGS_SLOT);
		}
		else
		{
			vA.PropertySubData.TrouserModel = 0;
		}

		itemNb = (int) getNumber(&getProps(), "FeetModel");
		if(itemNb>0)
		{
			itemFileName = CSheetId(itemNb).toString();
			vB.PropertySubData.FeetModel = (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::FEET_SLOT);
		}
		else
		{
			vB.PropertySubData.FeetModel = 0;
		}

		itemNb = (int) getNumber(&getProps(), "HandsModel");
		if(itemNb>0)
		{
			itemFileName = CSheetId(itemNb).toString();
			vB.PropertySubData.HandsModel = (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::HANDS_SLOT);
		}
		else
		{
			vB.PropertySubData.HandsModel = 0;
		}

		itemNb = (int) getNumber(&getProps(), "ArmModel");
		if(itemNb>0)
		{
			itemFileName = CSheetId(itemNb).toString();
			vA.PropertySubData.ArmModel	= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::ARMS_SLOT);
		}
		else
		{
			vA.PropertySubData.ArmModel = 0;
		}

		itemNb = (int) getNumber(&getProps(), "WeaponRightHand");
		if(itemNb>0)
		{
			itemFileName = CSheetId(itemNb).toString();
			vA.PropertySubData.WeaponRightHand = (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::RIGHT_HAND_SLOT);
		}
		else
		{
			vA.PropertySubData.WeaponRightHand = 0;
		}

		itemNb = (int) getNumber(&getProps(), "WeaponLeftHand");
		if(itemNb>0)
		{
			itemFileName = CSheetId(itemNb).toString();
			vA.PropertySubData.WeaponLeftHand = (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::LEFT_HAND_SLOT);
		}
		else
		{
			vA.PropertySubData.WeaponLeftHand = 0;
		}

		vA.PropertySubData.JacketColor		= (uint) getNumber(&getProps(), "JacketColor");
		vA.PropertySubData.TrouserColor		= (uint) getNumber(&getProps(), "TrouserColor");
		vB.PropertySubData.FeetColor		= (uint) getNumber(&getProps(), "FeetColor");
		vB.PropertySubData.HandsColor		= (uint) getNumber(&getProps(), "HandsColor");
		vA.PropertySubData.ArmColor			= (uint) getNumber(&getProps(), "ArmColor");
	}

	{
		//BENCH(entityApply)
		// Set the database
		leafA->setValue64(vA.PropertyA);
		leafB->setValue64(vB.PropertyB);
		leafC->setValue64(vC.PropertyC);

		// Force to update properties
		EntitiesMngr.updateVisualProperty(0, _Entity->slot(), CLFECOMMON::PROPERTY_VPA);
		EntitiesMngr.updateVisualProperty(0, _Entity->slot(), CLFECOMMON::PROPERTY_VPB);
		EntitiesMngr.updateVisualProperty(0, _Entity->slot(), CLFECOMMON::PROPERTY_VPC);
	}
}

// *********************************************************************************************************
void CDisplayerVisualEntity::updateEntityWorldPos()
{
	//H_AUTO(R2_CDisplayerVisualEntity_updateEntityWorldPos)
	_InvertedMatrixTouched = true;
	if (!_Entity) return;
	_Entity->pacsPos(getWorldPos());
	_Entity->updatePos(T1, NULL);
	_Entity->updateVisible(T1, NULL);
}

// *********************************************************************************************************
void CDisplayerVisualEntity::updateName()
{
	//H_AUTO(R2_CDisplayerVisualEntity_updateName)
	if (!_Entity) return;

	std::string name = getString(&getProps(), "Name");
	ucstring ucName;
	ucName.fromUtf8(name);
	if (ucName.empty())
	{
		ucName = CI18N::get("uiR2EDNoName");
	}

	std::string actName=std::string("");

	// If entity is in an additionnal act, then postfix its name with the name of the act
	if (getDisplayedInstance()->getParentAct() != getEditor().getBaseAct())
	{
		R2::CObjectTable* act = getDisplayedInstance()->getParentAct()->getObjectTable();
		R2::CObjectTable* acts = (CObjectTable *)(act->getParent());
		sint actNb = -1;
		for(uint i=0; i<acts->getSize(); i++)
		{
			if(acts->getValue(i)->equal(act))
			{
				actNb = i;
				break;
			}
		}
		std::string firstPart;
		if(actNb>0)
			firstPart = CI18N::get("uiR2EDDefaultActTitle").toString() + " " + NLMISC::toString(actNb);

		if (act->isString("Name"))
			actName = act->toString("Name");
		else
			actName = act->toString("Title"); //obsolete

		if(actName!=firstPart && actName!="")
			actName = firstPart+":"+actName;
	}
	else
	{
		actName = CI18N::get("uiR2EDBaseAct").toString();
	}

	actName = NLMISC::toString(" [%s]", actName.c_str());

	ucstring ucActName;
	ucActName.fromUtf8(actName);
	ucName += ucActName;

	{
		//BENCH(setEntityName)
		_Entity->setEntityName(ucName);
	}
	{
		//BENCH(buildInSceneInterface)
		_Entity->buildInSceneInterface();
	}
}

// *********************************************************************************************************
void CDisplayerVisualEntity::updatePermanentStatutIcon(const string & textureName)
{
	//H_AUTO(R2_CDisplayerVisualEntity_updatePermanentStatutIcon)
	if (!_Entity) return;
	setPermanentStatutIcon(textureName);
	_Entity->setPermanentStatutIcon(textureName);
	_Entity->buildInSceneInterface();
}

// *********************************************************************************************************
void CDisplayerVisualEntity::onPostHrcMove()
{
	//H_AUTO(R2_CDisplayerVisualEntity_onPostHrcMove)
	CDisplayerVisual::onPostHrcMove();
	// update the name, because act may have changed
	updateName();
}

// *********************************************************************************************************
void CDisplayerVisualEntity::updateEntityRot()
{
	//H_AUTO(R2_CDisplayerVisualEntity_updateEntityRot)
	_InvertedMatrixTouched = true;
	if (!_Entity) return;
	float angle = getAngle();
	_Entity->front(CVector((float)cos(angle), (float)sin(angle), 0.f), true, true, true);
	_Entity->dir(_Entity->front(), false, false);
	NLPACS::UMovePrimitive *prim = _Entity->getPrimitive();
	if (prim && prim->getPrimitiveType() == NLPACS::UMovePrimitive::_2DOrientedBox)
	{
		prim->setOrientation(angle, dynamicWI);
	}

}

// *********************************************************************************************************
void CDisplayerVisualEntity::retrieveAngle()
{
	//H_AUTO(R2_CDisplayerVisualEntity_retrieveAngle)
	_Angle = (float) getNumber(&getProps(), "Angle");
}

// *********************************************************************************************************
std::string CDisplayerVisualEntity::getSheet() const
{
	//H_AUTO(R2_std_getSheet)
	std::string sheet =  getString(&getProps(), "SheetClient");
	if (sheet.empty())
	{
		nlwarning("Sheet not found, defaulting to fyros.race_stats");
		return "fyros.race_stats";
	}
	return sheet;
}


// *********************************************************************************************************
std::string CDisplayerVisualEntity::getVisualProperties() const
{
	//H_AUTO(R2_std_getVisualProperties)
	if( !_Entity )
		return "";

	const std::string propNameA = toString("SERVER:Entities:E%d:P%d", _Entity->slot(), CLFECOMMON::PROPERTY_VPA);
	const std::string propNameB = toString("SERVER:Entities:E%d:P%d", _Entity->slot(), CLFECOMMON::PROPERTY_VPB);
	const std::string propNameC = toString("SERVER:Entities:E%d:P%d", _Entity->slot(), CLFECOMMON::PROPERTY_VPC);

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

	uint64 uVPA = leafA->getValue64();
	uint64 uVPB = leafB->getValue64();
	uint64 uVPC = leafC->getValue64();

	const std::string strVPABC = NLMISC::toString( "VPA:%016.16"NL_I64"x\nVPB:%016.16"NL_I64"x\nVPC:%016.16"NL_I64"x", uVPA, uVPB, uVPC );

	return strVPABC;
}

// *********************************************************************************************************
/*NLMISC::CVector CDisplayerVisualEntity::evalLinkPoint(bool leader) const
{*/
	//if (!_Entity) return CVector::Null;
	/*
	static volatile bool alwaysSnap = false;
	if (_Entity->getLastClip() || alwaysSnap)
	{
		_Entity->snapToGround();
	}
	if (_Entity->skeleton())
	{
		sint boneId = _Entity->skeleton()->getBoneIdByName(leader ? "Bip01 Head" : "Bip01 Spine1");
		if (boneId != -1)
		{
			if (!_Entity->skeleton()->isBoneComputed(boneId))
			{
				_Entity->skeleton()->forceComputeBone(boneId);
			}
			NL3D::UBone bone = _Entity->skeleton()->getBone(boneId);
			const NLMISC::CMatrix &worldMat = bone.getLastWorldMatrixComputed();
			return worldMat.getPos();
		}
	}
	*/
	//TMP : must solve this : bone pos isn't good until entity is fully loaded
/*	return _Entity->pos();
}*/

// *********************************************************************************************************
bool CDisplayerVisualEntity::getLastClip() const
{
	//H_AUTO(R2_CDisplayerVisualEntity_getLastClip)
	if (!_Entity)
	{
		return _PlaceHolder == NULL; // is there's a place holder consider entity to be not clipped
	}
	if (getActualSelectionDisplayMode() == BoxSelection) return false;
	return _Entity->getLastClip();
}


// *********************************************************************************************************
NLMISC::CAABBox CDisplayerVisualEntity::getSelectBox() const
{
	//H_AUTO(R2_NLMISC_CAABBox )
	if (!_Entity)
	{
		if (_PlaceHolder)
		{
			return _PlaceHolder->getSelectBox();
		}
		return CDisplayerVisual::getSelectBox();
	}
	if (getSelectionType() == ISelectableObject::LocalSelectBox ||
		getSelectionType() == ISelectableObject::GroundProjected
	   )
	{
		if (_Entity->isAsyncLoading())
		{
			NLMISC::CAABBox result;
			result.setCenter(CVector::Null);
			result.setHalfSize(CVector::Null);
			return result;
		}
		return getEditor().getLocalSelectBox(*_Entity);
	}
	else
	{
		return getEditor().getSelectBox(*_Entity);
	}
}

// *********************************************************************************************************
float CDisplayerVisualEntity::preciseIntersectionTest(const NLMISC::CVector &worldRayStart, const NLMISC::CVector &worldRayDir) const
{
	if (!_Entity)
	{
		if (_PlaceHolder)
		{
			return _PlaceHolder->preciseIntersectionTest(worldRayStart, worldRayDir);
		}
		return 0.f;
	}
	//H_AUTO(R2_CDisplayerVisualEntity_preciseIntersectionTest)
	return CEditor::preciseEntityIntersectionTest(*_Entity, worldRayStart, worldRayDir);
}


// *********************************************************************************************************
void CDisplayerVisualEntity::snapToGround()
{
	//H_AUTO(R2_CDisplayerVisualEntity_snapToGround)
	if (!_Entity) return;
	_Entity->snapToGround();
	// change local z
	_WorldPos.z = _Entity->pos().z;
}

// *********************************************************************************************************
const NLMISC::CMatrix &CDisplayerVisualEntity::getInvertedMatrix() const
{
	//H_AUTO(R2_NLMISC_CMatrix )
	if (!_InvertedMatrixTouched) return _InvertedMatrix;
	if (_Entity)
	{
		_InvertedMatrix = _Entity->dirMatrix();
		_InvertedMatrix.setPos(_Entity->pos());
		_InvertedMatrix.invert();
		_InvertedMatrixTouched = false;
	}
	else if (_PlaceHolder)
	{
		return _PlaceHolder->getInvertedMatrix();
	}
	return _InvertedMatrix;
}

// *********************************************************************************************************
sint32 CDisplayerVisualEntity::getSlotEntity() const
{
	//H_AUTO(R2_CDisplayerVisualEntity_getSlotEntity)
	if(_Entity)
	{
		return (sint32)_Entity->slot();
	}
	return -1;
}

// *********************************************************************************************************
int CDisplayerVisualEntity::luaUpdateName(CLuaState &ls)
{
	//H_AUTO(R2_CDisplayerVisualEntity_luaUpdateName)
	const char *funcName = "updateName";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	updateName();
	return 0;
}

// *********************************************************************************************************
int CDisplayerVisualEntity::luaUpdatePermanentStatutIcon(CLuaState &ls)
{
	//H_AUTO(R2_CDisplayerVisualEntity_luaUpdatePermanentStatutIcon)
	const char *funcName = "updatePermanentStatutIcon";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	updatePermanentStatutIcon(ls.toString(1));
	return 0;
}

// *********************************************************************************************************
bool CDisplayerVisualEntity::isInProjection(const NLMISC::CVector2f &pos) const
{
	//H_AUTO(R2_CDisplayerVisualEntity_isInProjection)
	if (getActualSelectionDisplayMode() == CircleSelection) return false;
	nlassert(getSelectionType() == LocalSelectBox || getSelectionType() == GroundProjected);
	CVector localPos = getInvertedMatrix() * pos;
	NLMISC::CAABBox selectBox = getSelectBox();
	CVector pmin = selectBox.getMin();
	CVector pmax = selectBox.getMax();
	if (localPos.x < pmin.x || localPos.x > pmax.x ||
		localPos.y < pmin.y || localPos.y > pmax.y) return false;
	return true;
}

// *********************************************************************************************************
ISelectableObject::TSelectionType CDisplayerVisualEntity::getSelectionType() const
{
	if (!_Entity && _PlaceHolder) return _PlaceHolder->getSelectionType();
	//H_AUTO(R2_ISelectableObject_TSelectionType )
	// If box selection was explicitly asked (not auto mode), uses ground selection only
	if (_SelectionDisplayMode == BoxSelection) return ISelectableObject::GroundProjected;
	// if there's a cylinder primitive, use world box selection, else use local box (for obstacle like barriers)
	return getActualSelectionDisplayMode() == BoxSelection ? ISelectableObject::LocalSelectBox : ISelectableObject::WorldSelectBox;
}

// *********************************************************************************************************
float CDisplayerVisualEntity::getCylinderRadius() const
{
	//H_AUTO(R2_CDisplayerVisualEntity_getCylinderRadius)
	if (!_Entity) return -1.f;
	const NLPACS::UMovePrimitive *prim = _Entity->getPrimitive();
	if (!prim) return -1;
	if (prim->getPrimitiveType() == NLPACS::UMovePrimitive::_2DOrientedCylinder)
	{
		return prim->getRadius();
	}
	else
	{
		return -1;
	}
}

// *********************************************************************************************************
CDisplayerVisualEntity::TSelectionDisplayMode CDisplayerVisualEntity::getActualSelectionDisplayMode() const
{
	//H_AUTO(R2_CDisplayerVisualEntity_TSelectionDisplayMode )
	// unless the display mode was explicitly given, uses the following rule
	// If the collision primitive is a rectangle, always display selection as a rectangle on ground
	// If the collision primitive is a cylinder, use that cylinder to display that selection on the ground, unless
	// it is too big. In this case ,uses a box instead
	static volatile float maxRadiusForCircleSelection = 2.f;
	switch(_SelectionDisplayMode)
	{
		case AutoSelection:
		{
			float radius = getCylinderRadius();
			if (radius == -1.f || radius >= maxRadiusForCircleSelection) return BoxSelection;
			return CircleSelection;
		}
		break;
		case BoxSelection:
			return BoxSelection;
		break;
		case CircleSelection:
			return CircleSelection;
		break;
		default:
			nlassert(0);
		break;
	}
	return CircleSelection;
}

// *********************************************************************************************************
bool CDisplayerVisualEntity::maxVisibleEntityExceeded() const
{
	return _ClipBlend != 1.f && _ClipBlend >= 0.f;
}


} // R2

NLMISC_COMMAND(toggleR2EntityBoxes, "toggle display bounding boxes for selected R2 entities", "")
{
	if (!args.empty()) return false;
	R2::DisplayR2EntityBoxes = !R2::DisplayR2EntityBoxes;
	return true;
}

NLMISC_COMMAND(showR2EntityBoxes, "display bounding boxes for selected R2 entities", "")
{
	if (!args.empty()) return false;
	R2::DisplayR2EntityBoxes = true;
	return true;
}

NLMISC_COMMAND(hideR2EntityBoxes, "display bounding boxes for selected R2 entities", "")
{
	if (!args.empty()) return false;
	R2::DisplayR2EntityBoxes = false;
	return true;
}
