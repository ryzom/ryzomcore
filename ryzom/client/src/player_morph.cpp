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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "stdpch.h"
#include "player_morph.h"
#include "player_cl.h"
#include "user_entity.h"
#include "string_manager_client.h"
#include "sheet_manager.h"
#include "entity_animation_manager.h"
#include "client_cfg.h"
#include "misc.h"
#include "interface_v3/action_handler_tools.h"
#include "pacs_client.h"
#include "nel/3d/u_scene.h"
#include "nel/gui/db_manager.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_list.h"
#include "nel/gui/ctrl_text_button.h"
#include "nel/gui/view_text.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/parser.h"
#include "nel/misc/i18n.h"
#include "nel/misc/file.h"

using namespace NLMISC;
using namespace NL3D;
using namespace NLGUI;
using namespace std;

extern UScene *Scene;
extern CEntityAnimationManager *EAM;

// Last abilities the server sent for the user's morph. They can arrive before the MORPH_SHEET visual
// property, so the panel is built once both name the same creature.
static uint32 MorphAbilitiesSheet = 0;
static vector<CSheetId> MorphActions;
static vector<string> MorphLabels;
static bool MorphAttackable = false;

// the moods a creature shows on its own, which the morph offers as buttons; NORMAL stands again
static const MBEHAV::EMode MorphMoods[] = { MBEHAV::REST, MBEHAV::EAT, MBEHAV::ALERT, MBEHAV::HUNGRY, MBEHAV::NORMAL };
static const char *MorphMoodLabels[] = { "uiMorphMoodRest", "uiMorphMoodEat", "uiMorphMoodAlert", "uiMorphMoodHungry", "uiMorphMoodNormal" };

static bool isMorphMood(MBEHAV::EMode mode)
{
	return mode == MBEHAV::EAT || mode == MBEHAV::REST || mode == MBEHAV::ALERT || mode == MBEHAV::HUNGRY;
}

// the server ends a mood with the form, so a new creature is checked standing normally, also when its
// form arrives before the mode
static MBEHAV::EMode getMorphCheckMode(MBEHAV::EMode mode)
{
	return isMorphMood(mode) ? MBEHAV::NORMAL : mode;
}

// the creature has an animation set and an automaton for this mode
static bool hasMorphMode(const CCharacterSheet &sheet, MBEHAV::EMode mode)
{
	const CAnimationSet *animSet = NULL;
	return ::computeAnimSet(animSet, mode == MBEHAV::SIT ? MBEHAV::REST : mode, sheet.getAnimSetBaseName(), NULL, NULL, false) &&
		EAM->mState(sheet.getAutomaton() + "_" + toLowerAscii(MBEHAV::modeToString(mode)) + ".automaton", CAnimationStateSheet::Idle);
}

static const CCharacterSheet *checkMorphSheet(uint32 formId, MBEHAV::EMode mode)
{
	if (!Scene || !EAM || ClientCfg.Light)
		return NULL;
	const CCharacterSheet *sheet = dynamic_cast<const CCharacterSheet *>(SheetMngr.get(CSheetId(formId)));
	if (!sheet || sheet->Race < EGSPD::CPeople::Creature || sheet->Race >= EGSPD::CPeople::EndCreature ||
		sheet->getSkelFilename().empty() || sheet->Body.getItem().empty())
		return NULL;
	if (!hasMorphMode(*sheet, mode))
		return NULL;
	USkeleton skeleton = Scene->createSkeleton(sheet->getSkelFilename());
	if (skeleton.empty())
		return NULL;
	Scene->deleteSkeleton(skeleton);
	// the body is an item reference like in CCharacterCL::buildEquipment, or a shape file
	string shape = sheet->Body.getItem();
	string ext = CFile::getExtension(shape);
	if (ext == "item" || ext == "sitem")
	{
		CSheetId itemId;
		const CItemSheet *item = itemId.buildSheetId(toLowerAscii(shape)) ?
			dynamic_cast<const CItemSheet *>(SheetMngr.get(itemId)) : NULL;
		if (!item)
			return NULL;
		shape = item->getShape();
	}
	UInstance body = Scene->createInstance(shape);
	if (body.empty())
		return NULL;
	Scene->deleteInstance(body);
	return sheet;
}

void prepareMorph(uint32 sheet)
{
	if (!UserEntity || !checkMorphSheet(sheet, getMorphCheckMode(UserEntity->mode())))
	{
		nlwarning("Morph: cannot load creature %u. Current appearance unchanged.", sheet);
		return;
	}
	sendMsgToServer("COMMAND:MORPH_READY", sheet);
}

void setMorphAbilities(uint32 sheet, const vector<CSheetId> &actions, const vector<string> &labels, bool attackable)
{
	MorphAbilitiesSheet = sheet;
	MorphActions = actions;
	MorphLabels = labels;
	MorphAttackable = attackable;
	updateMorphPanel();
}

// returns the width the button takes in the list
static sint32 addMorphButton(CGroupList &list, const string &id, const string &label, const string &handler, const string &handlerParams)
{
	vector<pair<string, string> > params;
	params.push_back(make_pair(string("id"), id));
	params.push_back(make_pair(string("label"), CI18N::get(label)));
	params.push_back(make_pair(string("handler"), handler));
	params.push_back(make_pair(string("handler_params"), handlerParams));
	CInterfaceGroup *button = CWidgetManager::getInstance()->getParser()->createGroupInstance("player_morph_button", list.getId(), &params[0], (uint)params.size());
	if (!button)
		return 0;
	list.addChild(button);
	CCtrlTextButton *cast = dynamic_cast<CCtrlTextButton *>(button->getCtrl("cast"));
	if (cast)
	{
		cast->updateCoords();
		button->setW(cast->getW() + 8);
	}
	return button->getW();
}

void updateMorphPanel()
{
	CWidgetManager *wm = CWidgetManager::getInstance();
	CGroupContainer *panel = dynamic_cast<CGroupContainer *>(wm->getElementFromId("ui:interface:player_morph"));
	if (!panel)
		return;
	if (!UserEntity || !UserEntity->isMorphed() || UserEntity->getSheet()->Id.asInt() != MorphAbilitiesSheet)
	{
		panel->setActive(false);
		return;
	}
	CGroupList *list = dynamic_cast<CGroupList *>(panel->getGroup("content:abilities"));
	CGroupList *moods = dynamic_cast<CGroupList *>(panel->getGroup("content:moods"));
	if (!list || !moods)
		return;
	list->deleteAllChildren();
	sint32 abilitiesWidth = 0;
	for (uint i = 0; i < MorphActions.size(); ++i)
		abilitiesWidth += addMorphButton(*list, toString("ability%u", i), MorphLabels[i], "morph_ability", toString(MorphActions[i].asInt()));
	if (MorphActions.empty())
		list->addTextChild(CI18N::get("uiMorphNoAbilities"));
	moods->deleteAllChildren();
	sint32 moodsWidth = 0;
	for (uint i = 0; i < sizeofarray(MorphMoods); ++i)
	{
		if (hasMorphMode(*UserEntity->getSheet(), MorphMoods[i]) && (moodsWidth != 0 || MorphMoods[i] != MBEHAV::NORMAL))
			moodsWidth += addMorphButton(*moods, toString("mood%u", i), MorphMoodLabels[i], "morph_mood", toString((uint)MorphMoods[i]));
	}
	CViewText *pvp = dynamic_cast<CViewText *>(panel->getView("content:pvp"));
	if (pvp)
	{
		pvp->setText(CI18N::get(MorphAttackable ? "uiMorphAttackableOn" : "uiMorphAttackableOff"));
		pvp->updateCoords();
	}
	// the abilities scroll, the few moods always fit
	sint32 width = max(max(sint32(260), min(sint32(520), abilitiesWidth + 16)), moodsWidth + 16);
	if (pvp)
		width = max(width, pvp->getW() + 16);
	panel->setW(width);
	panel->getGroup("content")->setW(width);
	// a horizontal list sizes itself to its buttons and shows them up to its max width
	list->setMaxW(width - 16);
	moods->setMaxW(width - 16);
	CInterfaceElement *scroll = panel->getCtrl("content:scroll");
	if (scroll)
	{
		scroll->setW(width - 16);
		scroll->setActive(abilitiesWidth > width - 16);
	}
	panel->setTitle(STRING_MANAGER::CStringManagerClient::getCreatureLocalizedName(UserEntity->getSheet()->Id));
	panel->setActive(true);
	panel->open();
}

class CHandlerMorphAbility : public IActionHandler
{
	void execute(CCtrlBase *, const string &params)
	{
		uint32 action;
		if (UserEntity && UserEntity->isMorphed() && fromString(params, action))
			sendMsgToServer("COMMAND:MORPH_ACTION", action);
	}
};
REGISTER_ACTION_HANDLER(CHandlerMorphAbility, "morph_ability");

class CHandlerMorphMood : public IActionHandler
{
	void execute(CCtrlBase *, const string &params)
	{
		uint8 mode;
		if (UserEntity && UserEntity->isMorphed() && fromString(params, mode))
			sendMsgToServer("COMMAND:MORPH_MODE", mode);
	}
};
REGISTER_ACTION_HANDLER(CHandlerMorphMood, "morph_mood");

void endMorphMood()
{
	// afk ends on its own
	if (UserEntity && UserEntity->isMorphed() && !UserEntity->isAFK() && isMorphMood(UserEntity->mode()) &&
		UserEntity->mode(MBEHAV::NORMAL))
		sendMsgToServer("COMMAND:MORPH_MODE", (uint8)MBEHAV::NORMAL);
}

void CPlayerCL::updateVisualPropertyMorphSheet(const TGameCycle &gameCycle, const sint64 &prop)
{
	_MorphSheetId = (uint32)prop;
	// VPA supplies the original gender and appearance before the first morph.
	if (_WaitForAppearance || !Scene || ClientCfg.Light)
		return;

	const CCharacterSheet *sheet = NULL;
	if (_MorphSheetId != 0)
	{
		sheet = checkMorphSheet(_MorphSheetId, getMorphCheckMode(_Mode));
		if (!sheet)
		{
			nlwarning("PL:morph:%u: cannot load creature %u", _Slot, _MorphSheetId);
			return;
		}
	}
	if (sheet == CCharacterCL::_Sheet)
		return;

	// Check the replacement skeleton before releasing the current appearance.
	string skeletonName = sheet ? sheet->getSkelFilename() : _PlayerSheet->GenderInfos[_Gender].Skelfilename;
	USkeleton checkSkeleton = Scene->createSkeleton(skeletonName);
	if (checkSkeleton.empty())
	{
		nlwarning("PL:morph:%u: cannot load skeleton %s", _Slot, skeletonName.c_str());
		return;
	}
	Scene->deleteSkeleton(checkSkeleton);

	removeAllAttachedFX();
	for (uint i = 0; i < _Items.size(); ++i)
		_Items[i].release();
	for (uint i = 0; i < _Instances.size(); ++i)
		_Instances[i].createLoading(string());
	_Instances.clear();
	_Instances.resize(SLOTTYPE::NB_SLOT);
	_Face.createLoading(string());
	_Face = SInstanceCL();
	if (!_NameTransform.empty())
	{
		if (!_Skeleton.empty())
			_Skeleton.detachSkeletonSon(_NameTransform);
		Scene->deleteTransform(_NameTransform);
		_NameTransform = 0;
	}
	if (!_Light.empty() && !_Skeleton.empty())
		_Skeleton.detachSkeletonSon(_Light);
	_NameCLodDeltaZ = NameCLodDeltaZNotComputed;
	_HeadOffsetComputed = false;
	_HeadIdx = _FaceIdx = BadIndex;
	_AsyncTextureLoading = false;
	_PlayerCLAsyncTextureLoading = false;
	_BlendRemaining = 0;
	_CurrentState = NULL;
	if (!CCharacterCL::_Sheet)
	{
		_UnmorphedClipRadius = _ClipRadius;
		_UnmorphedClipDeltaZ = _ClipDeltaZ;
	}
	CCharacterCL::_Sheet = sheet;
	computeAutomaton();
	for (uint i = 0; i < animTypeCount; ++i)
	{
		_CurrentAnimSet[i] = NULL;
		animIndex((TAnimationType)i, CAnimation::UnknownAnim);
		animState((TAnimationType)i, CAnimationStateSheet::UnknownState);
		animOffset((TAnimationType)i, 0.0);
	}
	resetAllSoundAnimId();
	createPlayList();
	_CacheSkeletonShapeName.clear();
	_CustomScalePos = 1.f;

	if (sheet)
	{
		skeleton(skeletonName);
		_Skeleton.setScale(getScale(), getScale(), getScale());
		_CharacterScalePos = sheet->CharacterScalePos;
		_CustomScalePos = getScale();
		// clipping follows the creature size
		initClipSphere(sheet->ColRadius * getScale(), sheet->ColHeight * getScale(), sheet->ColLength, sheet->ColWidth, sheet->ClipRadius, sheet->ClipHeight);
		buildEquipment(sheet->Body, SLOTTYPE::CHEST_SLOT, -1, SLOTTYPE::CHEST_SLOT);
		buildEquipment(sheet->Arms, SLOTTYPE::ARMS_SLOT, -1, SLOTTYPE::ARMS_SLOT);
		buildEquipment(sheet->Hands, SLOTTYPE::HANDS_SLOT, -1, SLOTTYPE::HANDS_SLOT);
		buildEquipment(sheet->Legs, SLOTTYPE::LEGS_SLOT, -1, SLOTTYPE::LEGS_SLOT);
		buildEquipment(sheet->Feet, SLOTTYPE::FEET_SLOT, -1, SLOTTYPE::FEET_SLOT);
		_FaceIdx = buildEquipment(sheet->Face, SLOTTYPE::FACE_SLOT, -1, SLOTTYPE::FACE_SLOT);
		_HeadIdx = buildEquipment(sheet->Head, SLOTTYPE::HEAD_SLOT, -1, SLOTTYPE::HEAD_SLOT);
		computeSomeBoneId();
		computeAnimSet();
		setAnim(CAnimationStateSheet::Idle);
		sint clodId = getLodCharacterId(*Scene, sheet->getLodCharacterName());
		if (clodId >= 0)
		{
			_Skeleton.setLodCharacterShape(clodId);
			_Skeleton.setLodCharacterDistance(sheet->LodCharacterDistance);
		}
		if (!_Light.empty() && _NameBoneId != -1)
			_Skeleton.stickObject(_Light, _NameBoneId);
		_LookRdy = true;
	}
	else
	{
		_ClipRadius = _UnmorphedClipRadius;
		_ClipDeltaZ = _UnmorphedClipDeltaZ;
		// The database continues receiving equipment changes while morphed.
		CCDBNodeLeaf *vpa = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:Entities:E%u:P%u", _Slot, CLFECOMMON::PROPERTY_VPA));
		updateVisualPropertyVpa(gameCycle, vpa->getValue64());
	}
	CCDBNodeLeaf *vfx = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:Entities:E%u:P%u", _Slot, CLFECOMMON::PROPERTY_VISUAL_FX));
	updateVisualPropertyVisualFX(gameCycle, vfx->getValue64());
	buildInSceneInterface();
	if (UserEntity && UserEntity->selection() == _Slot)
		CAHManager::getInstance()->runActionHandler("set_target_name", NULL,
			toString("slot=%u|target=ui:interface:target:uc_title|title=ui:interface:target:header_opened:target_title:uc_hardtext", _Slot));
	setMorphCollision();
	if (isUser())
	{
		// the primitive that checks what the server accepts is a copy of the old size, checkPos builds it again
		UserEntity->removeCheckPrimitive();
		updateMorphPanel();
	}
	_MorphFXPending = true;
}

void CPlayerCL::setMorphCollision()
{
	// a morph collides with the size of its body, as on a mount (CUserEntity::setOnMount)
	if (!_Primitive)
		return;
	const CCharacterSheet *sheet = CCharacterCL::_Sheet;
	_Primitive->setRadius(std::min(sheet ? sheet->ColRadius * getScale() : 0.5f, (float)(RYZOM_ENTITY_SIZE_MAX/2)));
	_Primitive->setHeight(sheet ? sheet->ColHeight * getScale() : 2);
}

void CPlayerCL::applyBehaviour(const CBehaviourContext &behaviour)
{
	if (isMorphed() && behaviour.Behav.isCombat())
	{
		CBehaviourContext creatureBehaviour = behaviour;
		// the enum assignment clears the behaviour data, keep what the creature attack shares
		creatureBehaviour.Behav = MBEHAV::CREATURE_ATTACK_0;
		creatureBehaviour.Behav.DeltaHP = behaviour.Behav.DeltaHP;
		creatureBehaviour.Behav.CreatureAttack.Time = behaviour.Behav.Combat.Time;
		creatureBehaviour.Behav.CreatureAttack2.TargetMode = behaviour.Behav.Combat2.TargetMode;
		if (behaviour.Behav.Behaviour == MBEHAV::RANGE_ATTACK)
		{
			creatureBehaviour.Behav.CreatureAttack.ImpactIntensity = behaviour.Behav.Range.ImpactIntensity;
			creatureBehaviour.Behav.CreatureAttack.Localisation = behaviour.Behav.Range.Localisation;
			creatureBehaviour.Behav.CreatureAttack2.DamageType = DMGTYPE::UNDEFINED;
			creatureBehaviour.Behav.CreatureAttack2.HitType = HITTYPE::Undefined;
		}
		else
		{
			creatureBehaviour.Behav.CreatureAttack.ImpactIntensity = behaviour.Behav.Combat.ImpactIntensity;
			creatureBehaviour.Behav.CreatureAttack.ActionDuration = behaviour.Behav.Combat.ActionDuration;
			creatureBehaviour.Behav.CreatureAttack.Localisation = behaviour.Behav.Combat.Localisation;
			creatureBehaviour.Behav.CreatureAttack2.DamageType = behaviour.Behav.Combat2.DamageType;
			creatureBehaviour.Behav.CreatureAttack2.HitType = behaviour.Behav.Combat.HitType;
		}
		CCharacterCL::applyBehaviour(creatureBehaviour);
	}
	else
		CCharacterCL::applyBehaviour(behaviour);
}
