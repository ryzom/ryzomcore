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
#include "nel/misc/command.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/net/unified_network.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/tick_event_handler.h"
#include "egs_mirror.h"
#include "player_morph.h"
#include "player_morph_roles.h"
#include "egs_sheets/egs_sheets.h"
#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_manager.h"
#include "creature_manager/creature_manager.h"
#include "team_manager/team_manager.h"
#include "progression/progression_pve.h"

using namespace NLMISC;
using namespace std;

// the hp bar of a character is computed as 1023 * current hit points in a sint32
static const sint32 MorphMaxHp = INT_MAX / 1023;

const CStaticCreatures *getPlayerMorphForm(const TDataSetRow &rowId)
{
	if (!TheDataset.isAccessible(rowId) || TheDataset.getEntityId(rowId).getType() != RYZOMID::player)
		return NULL;
	CMirrorPropValue<TYPE_MORPH_SHEET> sheet(TheDataset, rowId, DSPropertyMORPH_SHEET);
	return sheet.getValue() ? CSheets::getCreaturesForm(CSheetId(sheet.getValue())) : NULL;
}

const CStaticCreatures *getActingCreatureForm(const TDataSetRow &rowId)
{
	const CStaticCreatures *form = getPlayerMorphForm(rowId);
	if (form)
		return form;
	CCreature *creature = CreatureManager.getCreature(rowId);
	return creature ? creature->getForm() : NULL;
}

bool isHominBody(const CEntityBase *entity)
{
	return entity->getId().getType() == RYZOMID::player && !getPlayerMorphForm(entity->getEntityRowId());
}

bool isMorphLockedItem(const CCharacter *c, INVENTORIES::TInventory inventory, uint32 slot)
{
	if (!getPlayerMorphForm(c->getEntityRowId()))
		return false;
	if (inventory == INVENTORIES::equipment || inventory == INVENTORIES::handling)
		return true;
	CInventoryPtr inv = c->getInventory(inventory);
	if (inv == NULL || slot >= inv->getSlotCount())
		return false;
	CGameItemPtr item = inv->getItem(slot);
	if (item == NULL || item->getRefInventory() == NULL)
		return false;
	INVENTORIES::TInventory ref = item->getRefInventory()->getInventoryId();
	return ref == INVENTORIES::equipment || ref == INVENTORIES::handling;
}

// role of the current morph, NULL for a plain creature morph or a role removed since
static const CPlayerMorphRole *getMorphRole(const CCharacter *c)
{
	return c->getMorphRole().empty() ? NULL : CPlayerMorphRoles::getInstance().getRole(c->getMorphRole());
}

// the moods a creature shows on its own, which a morph may take on
static bool isMorphMood(MBEHAV::EMode mode)
{
	return mode == MBEHAV::EAT || mode == MBEHAV::REST || mode == MBEHAV::ALERT || mode == MBEHAV::HUNGRY;
}

// a mood belongs to the body of a creature: another creature may not have it and the homin has none
static void endMorphMood(CCharacter *c)
{
	// afk rests too
	c->setAfkState(false);
	if (isMorphMood(c->getMode()))
		c->setMode(MBEHAV::NORMAL);
}

// a role may keep only part of what its creature does
static void getMorphActions(const CStaticCreatures &form, const CPlayerMorphRole *role, vector<CSheetId> &actions)
{
	const vector<CSheetId> &lists = form.getActionLists();
	for (uint i = 0; i < lists.size(); ++i)
	{
		const CStaticActionList *list = CSheets::getActionListForm(lists[i]);
		if (!list)
			continue;
		const vector<CSheetId> &listActions = list->getActions();
		for (uint j = 0; j < listActions.size(); ++j)
		{
			const CStaticAiAction *action = CSheets::getAiActionForm(listActions[j]);
			if (!action || action->getType() == AI_ACTION::UnknownType ||
				// a toxic cloud and a stench aura hurt every player around without checking the pvp rules
				action->getType() == AI_ACTION::ToxicCloud ||
				(action->getType() == AI_ACTION::EoTSpell && action->getData().OTEffectSpell.EffectFamily == AI_ACTION::Stench) ||
				// the hot action never builds from a HoTSpell sheet (magic_action_ai_hot.cpp), so it would do nothing
				action->getType() == AI_ACTION::HoTSpell)
				continue;
			if (role && !role->Actions.empty() && find(role->Actions.begin(), role->Actions.end(), listActions[j]) == role->Actions.end())
				continue;
			if (find(actions.begin(), actions.end(), listActions[j]) == actions.end())
				actions.push_back(listActions[j]);
		}
	}
}

// the client builds the abilities panel from this
static void sendMorphAbilities(CCharacter *c, const CStaticCreatures &form, uint32 formId)
{
	vector<CSheetId> actions;
	getMorphActions(form, getMorphRole(c), actions);
	vector<string> labels;
	for (uint i = 0; i < actions.size(); ++i)
	{
		const CStaticAiAction *action = CSheets::getAiActionForm(actions[i]);
		AI_ACTION::TAiEffectType effect = AI_ACTION::UnknownEffect;
		if (action->getType() == AI_ACTION::Melee || action->getType() == AI_ACTION::Range)
			effect = action->getData().Combat.EffectFamily;
		else if (action->getType() == AI_ACTION::EffectSpell)
			effect = action->getData().EffectSpell.EffectFamily;
		else if (action->getType() == AI_ACTION::EoTSpell)
			effect = action->getData().OTEffectSpell.EffectFamily;
		labels.push_back("uiMorph" + (effect == AI_ACTION::UnknownEffect ?
			AI_ACTION::toString(action->getType()) : AI_ACTION::toString(effect)));
	}
	bool attackable = c->getContextualProperty().directAccessForStructMembers().attackable();

	NLNET::CMessage msgout("IMPULSION_ID");
	CEntityId id = c->getId();
	msgout.serial(id);
	CBitMemStream bms;
	if (!GenericMsgManager.pushNameToStream("USER:MORPH_ABILITIES", bms))
	{
		nlwarning("<sendMorphAbilities> Msg name USER:MORPH_ABILITIES not found");
		return;
	}
	bms.serial(formId);
	bms.serialCont(actions);
	bms.serialCont(labels);
	bms.serial(attackable);
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	NLNET::CUnifiedNetwork::getInstance()->send(NLNET::TServiceId(id.getDynamicId()), msgout);
}

// a creature sheet given by name, with or without ".creature", or by id; unknown for anything else
static CSheetId getCreatureSheetArg(const string &arg)
{
	CSheetId sheetId;
	string name = toLowerAscii(arg);
	if (!name.empty() && name.find_first_not_of("0123456789") == string::npos)
	{
		uint32 id;
		if (fromString(name, id) && toString(id) == name)
			sheetId = CSheetId(id);
	}
	else
	{
		if (name.find('.') == string::npos)
			name += ".creature";
		sheetId.buildSheetId(name);
	}
	const CStaticCreatures *sheet = CSheets::getCreaturesForm(sheetId);
	if (!sheet || sheet->getRace() < EGSPD::CPeople::Creature || sheet->getRace() >= EGSPD::CPeople::EndCreature)
		return CSheetId::Unknown;
	return sheetId;
}

// the morph is applied once the client of the player confirms it can display the creature (MORPH_READY)
static void requestMorph(CCharacter *c, const CSheetId &sheetId, bool attackable, const string &role, TGameCycle endCycle)
{
	c->setPendingMorph(sheetId.asInt(), attackable, role, endCycle);
	PlayerManager.sendImpulseToClient(c->getId(), "USER:MORPH_PREPARE", sheetId.asInt());
}

void endPlayerMorph(CCharacter *c)
{
	c->setPendingMorph(0, false);
	c->setMorphRole(string());
	c->setMorphEndCycle(0);
	CPhraseManager::getInstance().cancelAllPhrases(c->getEntityRowId());
	endMorphMood(c);
	CMirrorPropValue<TYPE_MORPH_SHEET> morphSheet(TheDataset, c->getEntityRowId(), DSPropertyMORPH_SHEET);
	morphSheet = 0;
	c->setMorphAttackable(false);
	c->setMorphBody(0);
}

void checkMorphTransformation(CCharacter *c)
{
	const CPlayerMorphRole *role = getMorphRole(c);
	// a transformation already asked for waits for its client
	if (!role || role->NextRole.empty() || c->getPendingMorphSheet() != 0)
		return;
	if (sint64(c->currentHp()) * 100 > sint64(c->maxHp()) * role->NextAtPercent)
		return;
	const CPlayerMorphRole *next = CPlayerMorphRoles::getInstance().getRole(role->NextRole);
	if (next)
		requestMorph(c, next->Creature, next->Attackable, role->NextRole, c->getMorphEndCycle());
}

void checkMorphEnd(CCharacter *c)
{
	const TGameCycle now = CTickEventHandler::getGameCycle();
	// a request whose time is over before its client answered is only dropped
	if (c->getPendingMorphSheet() != 0 && c->getPendingMorphEndCycle() != 0 && now >= c->getPendingMorphEndCycle())
		c->setPendingMorph(0, false);
	// a dead morph ends when it respawns or is resurrected
	if (c->getMorphEndCycle() == 0 || now < c->getMorphEndCycle() || c->isDead())
		return;
	// a newer request still turns the homin into its creature when its client answers
	const uint32 sheet = c->getPendingMorphSheet();
	const bool attackable = c->getPendingMorphAttackable();
	const string role = c->getPendingMorphRole();
	const TGameCycle endCycle = c->getPendingMorphEndCycle();
	endPlayerMorph(c);
	c->setPendingMorph(sheet, attackable, role, endCycle);
}

bool morphPlayer(CCharacter *c, const vector<string> &args, CLog &log)
{
	if (!c || !c->getEnterFlag() || !TheDataset.isAccessible(c->getEntityRowId()))
	{
		log.displayNL("Morph needs an online player.");
		return false;
	}
	bool attackable = false;
	uint32 duration = 0;
	// the end of the morph has to fit in a game cycle
	if ((args.size() > 1 && !fromString(args[1], attackable)) || (args.size() > 2 && !fromString(args[2], duration)) ||
		duration / CTickEventHandler::getGameTimeStep() >= double(std::numeric_limits<TGameCycle>::max() - CTickEventHandler::getGameCycle()))
	{
		log.displayNL("Bad attackable flag or duration.");
		return false;
	}
	if (c->isDead() || c->getMode() == MBEHAV::MOUNT_NORMAL || c->getMode() == MBEHAV::MOUNT_SWIM)
	{
		log.displayNL("Morph requires a living, dismounted character.");
		return false;
	}

	// a role name comes before a creature of the same name
	const CPlayerMorphRole *role = CPlayerMorphRoles::getInstance().getRole(args[0]);
	CSheetId sheetId = role ? role->Creature : getCreatureSheetArg(args[0]);
	if (role && args.size() == 1)
		attackable = role->Attackable;
	if (!CSheets::getCreaturesForm(sheetId))
	{
		log.displayNL("Unknown role or non-creature sheet: %s. Appearance unchanged.", args[0].c_str());
		return false;
	}

	const TGameCycle endCycle = duration ? TGameCycle(duration / CTickEventHandler::getGameTimeStep() + CTickEventHandler::getGameCycle()) : 0;
	requestMorph(c, sheetId, attackable, role ? args[0] : string(), endCycle);
	log.displayNL("Morph requested: %s. Use unmorph to restore the appearance.", sheetId.toString().c_str());
	return true;
}

void unmorphPlayer(CCharacter *c)
{
	// the creature died, not the homin: it stands up with the values it had before the morph
	if (c->isDead() && getPlayerMorphForm(c->getEntityRowId()))
		c->resurrected();
	else
		endPlayerMorph(c);
}

void dropMorphLoot(CCharacter *c, const TDataSetRow &killerRowId)
{
	const CPlayerMorphRole *role = getMorphRole(c);
	const CStaticCreatures *form = getPlayerMorphForm(c->getEntityRowId());
	CCharacter *finalBlower = PlayerManager.getChar(killerRowId);
	if (!role || role->LootTable == CSheetId::Unknown || !form || !finalBlower)
		return;
	const CStaticLootTable *loot = CSheets::getLootTableForm(role->LootTable);
	const CStaticItem *bagForm = CSheets::getForm(CSheetId("bag.sitem"));
	if (!loot || !bagForm)
	{
		nlwarning("<dropMorphLoot> loot table %s of role %s or the bag form not found", role->LootTable.toString().c_str(), c->getMorphRole().c_str());
		return;
	}
	CSheetId lootSetId = loot->selectRandomLootSet();
	const CStaticLootSet *lootSet = lootSetId == CSheetId::Unknown ? NULL : CSheets::getLootSetForm(lootSetId);
	if (!lootSet)
		return;
	vector<CGameItemPtr> items;
	bool haveLoot = false;
	CCreature::createLootSetItems(*lootSet, bagForm, form->getXPLevel(), items, haveLoot);

	// the final blower takes the loot, what its bag refuses goes to the first team member whose bag takes it
	vector<CCharacter *> takers(1, finalBlower);
	CTeam *team = TeamManager.getTeam(finalBlower->getTeamId());
	if (team)
	{
		const list<CEntityId> &ids = team->getTeamMembers();
		for (list<CEntityId>::const_iterator it = ids.begin(); it != ids.end(); ++it)
		{
			CCharacter *member = PlayerManager.getChar(*it);
			if (member && member != finalBlower && member->getEnterFlag())
				takers.push_back(member);
		}
	}
	for (uint i = 0; i < items.size(); ++i)
	{
		SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
		params[0].Int = max((sint32)items[i]->getStackSize(), 1);
		params[1].SheetId = items[i]->getSheetId();
		params[2].Int = (sint32)items[i]->quality();
		uint j = 0;
		// NEVER USE THE ITEM AFTER A addItemToInventory because of autostack
		while (j < takers.size() && !takers[j]->addItemToInventory(INVENTORIES::bag, items[i]))
			++j;
		if (j < takers.size())
			CCharacter::sendDynamicSystemMessage(takers[j]->getEntityRowId(), "LOOT_SUCCESS", params);
		else
			// lost like the loot left on a creature corpse
			items[i].deleteItem();
	}
}

NLMISC_COMMAND(morph, "Temporarily give a player a creature appearance", "<eid> <role, or creature sheet name or id> [<players can attack it 0|1> [<duration in seconds, 0 for none>]]")
{
	if (args.size() < 2 || args.size() > 4)
		return false;
	morphPlayer(PlayerManager.getChar(CEntityId(args[0])), vector<string>(args.begin() + 1, args.end()), log);
	return true;
}

NLMISC_COMMAND(unmorph, "Restore the player appearance", "<eid>")
{
	if (args.size() != 1)
		return false;
	CCharacter *c = PlayerManager.getChar(CEntityId(args[0]));
	if (!c || !c->getEnterFlag() || !TheDataset.isAccessible(c->getEntityRowId()))
	{
		log.displayNL("Unmorph needs an online player.");
		return true;
	}
	unmorphPlayer(c);
	log.displayNL("Player appearance restored.");
	return true;
}

NLMISC_COMMAND(morphRole, "Create or replace a creature role for morphs", "<eid> <role> <creature sheet name or id> [<attackable 0|1> [<hit points, 0 for the creature ones> [<loot table, - for none> [<next role> <at hit points %>]]]]")
{
	if (args.size() < 3 || args.size() > 8 || args.size() == 7)
		return false;
	CPlayerMorphRole role;
	role.Creature = getCreatureSheetArg(args[2]);
	if (role.Creature == CSheetId::Unknown)
	{
		log.displayNL("Unknown or non-creature sheet: %s.", args[2].c_str());
		return true;
	}
	if ((args.size() > 3 && !fromString(args[3], role.Attackable)) ||
		(args.size() > 4 && (!fromString(args[4], role.Hp) || role.Hp < 0 || role.Hp > MorphMaxHp)) ||
		(args.size() > 7 && (!fromString(args[7], role.NextAtPercent) || role.NextAtPercent == 0 || role.NextAtPercent >= 100)))
		return false;
	if (args.size() > 5 && args[5] != "-")
	{
		string lootTable = toLowerAscii(args[5]);
		if (lootTable.find('.') == string::npos)
			lootTable += ".loot_table";
		role.LootTable.buildSheetId(lootTable);
		if (!CSheets::getLootTableForm(role.LootTable))
		{
			log.displayNL("Unknown loot table: %s.", args[5].c_str());
			return true;
		}
	}
	if (args.size() > 6)
		role.NextRole = args[6];
	// a replaced role keeps the abilities chosen for it while they still fit its creature
	const CPlayerMorphRole *oldRole = CPlayerMorphRoles::getInstance().getRole(args[1]);
	if (oldRole && oldRole->Creature == role.Creature)
		role.Actions = oldRole->Actions;
	CPlayerMorphRoles::getInstance().setRole(args[1], role);
	log.displayNL("Morph role %s saved.", args[1].c_str());
	return true;
}

NLMISC_COMMAND(morphRoleAbilities, "Choose which creature actions a morph role may use", "<eid> <role> [<ai action sheet>...]")
{
	if (args.size() < 2)
		return false;
	const CPlayerMorphRole *oldRole = CPlayerMorphRoles::getInstance().getRole(args[1]);
	if (!oldRole)
	{
		log.displayNL("Unknown morph role: %s.", args[1].c_str());
		return true;
	}
	CPlayerMorphRole role = *oldRole;
	// the sheets may have been rebuilt without the creature since the role was saved
	const CStaticCreatures *form = CSheets::getCreaturesForm(role.Creature);
	if (!form)
	{
		log.displayNL("Creature %s of morph role %s no longer exists.", role.Creature.toString().c_str(), args[1].c_str());
		return true;
	}
	vector<CSheetId> creatureActions;
	getMorphActions(*form, NULL, creatureActions);
	role.Actions.clear();
	for (uint i = 2; i < args.size(); ++i)
	{
		string name = toLowerAscii(args[i]);
		if (name.find('.') == string::npos)
			name += ".aiaction";
		CSheetId action;
		if (!action.buildSheetId(name) || find(creatureActions.begin(), creatureActions.end(), action) == creatureActions.end())
		{
			log.displayNL("%s is no action of creature %s.", args[i].c_str(), role.Creature.toString().c_str());
			return true;
		}
		role.Actions.push_back(action);
	}
	CPlayerMorphRoles::getInstance().setRole(args[1], role);
	log.displayNL("Morph role %s uses %s.", args[1].c_str(), role.Actions.empty() ? "all actions of its creature" : toString("%u actions", role.Actions.size()).c_str());
	return true;
}

NLMISC_COMMAND(removeMorphRole, "Remove a creature role for morphs", "<eid> <role>")
{
	if (args.size() != 2)
		return false;
	if (!CPlayerMorphRoles::getInstance().getRole(args[1]))
	{
		log.displayNL("Unknown morph role: %s.", args[1].c_str());
		return true;
	}
	CPlayerMorphRoles::getInstance().removeRole(args[1]);
	log.displayNL("Morph role %s removed.", args[1].c_str());
	return true;
}

NLMISC_COMMAND(listMorphRoles, "List the creature roles for morphs", "<eid>")
{
	if (args.size() != 1)
		return false;
	const CPlayerMorphRoles::TRoleMap &roles = CPlayerMorphRoles::getInstance().getRoles();
	for (CPlayerMorphRoles::TRoleMap::const_iterator it = roles.begin(); it != roles.end(); ++it)
	{
		const CPlayerMorphRole &role = (*it).second;
		log.displayNL("%s: %s attackable %u hp %d actions %u loot %s next %s at %u%%", (*it).first.c_str(),
			role.Creature.toString().c_str(), role.Attackable, role.Hp, (uint)role.Actions.size(),
			role.LootTable == CSheetId::Unknown ? "-" : role.LootTable.toString().c_str(),
			role.NextRole.empty() ? "-" : role.NextRole.c_str(), role.NextAtPercent);
	}
	if (roles.empty())
		log.displayNL("No morph roles.");
	return true;
}

void cbClientMorphReady(NLNET::CMessage &msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientMorphReady);

	CEntityId id;
	uint32 formId;
	msgin.serial(id);
	msgin.serial(formId);
	CCharacter *c = PlayerManager.getChar(id);
	// an answer to an older request leaves the newer one pending
	if (!c || !c->getEnterFlag() || !TheDataset.isAccessible(c->getEntityRowId()) ||
		formId == 0 || formId != c->getPendingMorphSheet())
		return;
	const CStaticCreatures *form = CSheets::getCreaturesForm(CSheetId(formId));
	// an answer after the end of the morph time comes too late
	const TGameCycle endCycle = c->getPendingMorphEndCycle();
	if (!form || c->isDead() || c->getMode() == MBEHAV::MOUNT_NORMAL || c->getMode() == MBEHAV::MOUNT_SWIM ||
		(endCycle != 0 && CTickEventHandler::getGameCycle() >= endCycle))
	{
		c->setPendingMorph(0, false);
		return;
	}

	CPhraseManager::getInstance().cancelAllPhrases(c->getEntityRowId());
	// the creature must not finish what the homin started and earn the homin xp for it (CCharacter::entersWater)
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->transferDamageOnFictitiousCreature(c->getEntityRowId(), c->getTeamId());
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->clearAllXpForPlayer(c->getEntityRowId(), c->getTeamId(), true);
	endMorphMood(c);
	c->setMorphAttackable(c->getPendingMorphAttackable());
	c->setMorphRole(c->getPendingMorphRole());
	c->setMorphEndCycle(endCycle);
	c->setPendingMorph(0, false);
	CMirrorPropValue<TYPE_MORPH_SHEET> morphSheet(TheDataset, c->getEntityRowId(), DSPropertyMORPH_SHEET);
	morphSheet = formId;
	const CPlayerMorphRole *role = getMorphRole(c);
	c->setMorphBody(role && role->Hp != 0 ? role->Hp : min(form->getScores(SCORES::hit_points), MorphMaxHp));
	// the client moves the morph at these speeds, as it moves a mount (CCharacter::applyCurrentSpeed)
	CBankAccessor_PLR::getUSER().setMOUNT_WALK_SPEED(c->_PropertyDatabase, checkedCast<uint16>(form->getWalkSpeed() * 1000.0f));
	CBankAccessor_PLR::getUSER().setMOUNT_RUN_SPEED(c->_PropertyDatabase, checkedCast<uint16>(form->getRunSpeed() * 1000.0f));
	sendMorphAbilities(c, *form, formId);
}

void cbClientMorphAction(NLNET::CMessage &msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientMorphAction);

	CEntityId id;
	uint32 actionId;
	msgin.serial(id);
	msgin.serial(actionId);
	CCharacter *c = PlayerManager.getChar(id);
	if (!c || !c->getEnterFlag() || !TheDataset.isAccessible(c->getEntityRowId()))
		return;
	const CStaticCreatures *form = getPlayerMorphForm(c->getEntityRowId());
	if (!form)
		return;
	vector<CSheetId> actions;
	getMorphActions(*form, getMorphRole(c), actions);
	CSheetId actionSheet(actionId);
	if (find(actions.begin(), actions.end(), actionSheet) == actions.end())
	{
		nlwarning("<cbClientMorphAction> %s used action %s which its creature form does not have", id.toString().c_str(), actionSheet.toString().c_str());
		return;
	}
	if (c->isDead() || c->isStunned() || c->getMode() == MBEHAV::MOUNT_NORMAL || c->getMode() == MBEHAV::MOUNT_SWIM)
		return;

	const CStaticAiAction *action = CSheets::getAiActionForm(actionSheet);
	// plain melee and range attacks repeat like the player's cyclic attack; using it again stops it
	bool cyclic = (action->getType() == AI_ACTION::Melee || action->getType() == AI_ACTION::Range) &&
		action->getData().Combat.EffectFamily == AI_ACTION::UnknownEffect;
	const CEntityPhrases *phrases = CPhraseManager::getInstance().getEntityPhrases(c->getEntityRowId());
	if (cyclic && phrases && phrases->getCyclicAiAction() == actionSheet)
	{
		CPhraseManager::getInstance().cancelAllPhrases(c->getEntityRowId());
		return;
	}
	CEntityBase *target = action->isSelfAction() ? c : CEntityBaseManager::getEntityBasePtr(c->getTarget());
	if (!target || !TheDataset.isAccessible(target->getEntityRowId()))
		return;
	const TGameCycle now = CTickEventHandler::getGameCycle();
	if (c->dateOfNextAllowedAction() > now)
		return;
	// a creature does one action at a time: a cyclic attack gives way, a spell or special attack runs to the end
	// of its latency, so an instant spell can neither end its state nor skip its latency
	if (phrases && phrases->getCurrentActionConst() != NULL)
	{
		if (!phrases->getCurrentActionConst()->cyclic())
			return;
		CPhraseManager::getInstance().cancelAllPhrases(c->getEntityRowId());
	}
	endMorphMood(c);
	// an instant spell runs at once and leaves no latency, so the creature attack speed paces it like the AIS paces creatures
	if (CPhraseManager::getInstance().executeAiAction(c->getEntityRowId(), target->getEntityRowId(), actionSheet, 1.0f, 1.0f, cyclic) == NULL &&
		action->getType() != AI_ACTION::Melee && action->getType() != AI_ACTION::Range)
		c->dateOfNextAllowedAction(now + form->getAttackLatency());
}

void cbClientMorphMode(NLNET::CMessage &msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientMorphMode);

	CEntityId id;
	uint8 mode;
	msgin.serial(id);
	msgin.serial(mode);
	CCharacter *c = PlayerManager.getChar(id);
	if (!c || !c->getEnterFlag() || !TheDataset.isAccessible(c->getEntityRowId()) ||
		!getPlayerMorphForm(c->getEntityRowId()) || c->isDead() || c->getMode() == MBEHAV::MOUNT_NORMAL || c->getMode() == MBEHAV::MOUNT_SWIM)
		return;
	// a mood is taken on and left from standing or from another mood, never out of a fight
	if ((mode != MBEHAV::NORMAL && !isMorphMood(MBEHAV::EMode(mode))) ||
		(c->getMode() != MBEHAV::NORMAL && !isMorphMood(c->getMode())))
		return;
	c->setAfkState(false);
	c->setMode(MBEHAV::EMode(mode));
}
