// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef R2_RT_SCENARIO_CHECK_H
#define R2_RT_SCENARIO_CHECK_H

#include "nel/misc/types_nl.h"
#include "scenario_text_parser.h"

#include <string>

namespace R2 {

class CObject;

/// Weather of an act: 0 for the automatic weather, else 1 to this value (the editor sends 1 + its
/// slider of 0 to 1022; the weather value of a player has 10 bits)
const uint32 RtScenarioMaxWeatherValue = 1023;
/// Season of a location: 0 automatic, 1 spring, 2 summer, 3 autumn, 4 winter (r2_translator.lua)
const uint32 RtScenarioMaxSeason = 4;
/// Largest coordinate in metres, either way. The AI service holds a coordinate as a sint32 in
/// millimetres (CAICoord) and its primitive parser multiplies what it is given by 1000.
const sint32 RtScenarioMaxCoordinate = 2147483;
/// Largest angle in radians, either way. The editor keeps the angle of an entity as it was turned,
/// without bringing it back into one turn, and the AI service converts an angle to 1/32768 of a
/// half turn in a sint32 (CAngle), which holds 205887 radians: this is about 10000 turns.
const sint32 RtScenarioMaxAngle = 65536;
/// Margin in metres around the islands when positions are bounded by them: one zone of the landscape
const sint32 RtScenarioIslandMargin = 160;

/**
 * Limits a run time scenario is checked against. The defaults are the limits that exist elsewhere
 * and do not depend on the configuration of the service; the service fills in the others.
 */
struct CRtScenarioLimits
{
	/// Largest nesting of tables. The scenario file the editor loads has the same limit, and the
	/// functions that walk the tree afterwards (the instance map, the translation of nested actions)
	/// recurse once per level.
	static const uint32 DefaultMaxDepth = CScenarioTextParser::MaxDepth;
	/// Largest number of values in the tree, and of nodes the primitives made from it may have. A
	/// run time scenario is the translation of a scenario file, which has this limit; each state,
	/// group, bot, event and action of a tree the editor makes becomes one node.
	static const uint32 DefaultMaxNodes = CScenarioTextParser::MaxNodes;
	/// The base act and the acts the editor lets its user add (r2ed_max_num_additionnal_acts in
	/// r2ed_scenario.xml of the client interface)
	static const uint32 DefaultMaxActs = 1 + 10;
	/// Plot items the editor lets its user define (r2ed_max_num_plot_items in r2ed_scenario.xml; the
	/// client database has that many slots for them)
	static const uint32 DefaultMaxPlotItems = 32;
	/// Bytes in the name, description and comment of a plot item (the translator of the editor
	/// requires string.len < 256 for each, r2_translator.lua)
	static const uint32 DefaultMaxPlotItemTextSize = 255;
	/// Bots of the base act and of one other act together: the quota of the editor, where every
	/// entity and every feature a user places takes one unit. This default is the default of the
	/// service: MaxNpcs (100) + MaxStaticObjects (100) + the margin of 5 it has always allowed.
	static const uint32 DefaultMaxBotsPerAct = 100 + 100 + 5;
	/// Aliases the primitive file of one act can give out. This default is the 20 dynamic alias bits
	/// of the shipped world_editor_classes.xml; the service reads the size from its ligo configuration.
	static const uint32 DefaultMaxAliasesPerAct = (1 << 20) - 1;

	// The editor has a quota for the entities of an act and none for their states, events, actions and
	// texts. The limits of those are derived from the quota, with what the translator of the editor
	// makes for one entity that uses everything the editor shows for it (r2_logic.lua,
	// r2_translator.lua). The tree as a whole stays within MaxNodes whatever these limits allow.

	/// Sequences of activity of an entity and steps of a sequence the editor is made for (maxSequences
	/// and maxElements in r2_activities.lua; it holds back a fifteenth step, not an eighth sequence)
	static const uint32 ActivitySequencesPerEntity = 7;
	static const uint32 ActivityStepsPerSequence = 14;
	/// States of an entity: a start and a stop state for each sequence, and a state for each step,
	/// two for a Patrol step (createActivityStates, createStepStates): 7 * (2 + 14 * 2) = 210
	static const uint32 StatesPerEntity = ActivitySequencesPerEntity * (2 + ActivityStepsPerSequence * 2);
	/// Events of an entity. A sequence has 3 (start of its state, group spawned, its timer). A step
	/// has up to 6: Repeat Road, Follow Route and Go To Zone make 1, Patrol and Guard Zone 2, Rest In
	/// Zone and Feed In Zone 4, Hunt In Zone 5, and a time limit, in seconds or by a dialog, 1 more.
	/// The entity has 3: its initialisation and the two of its dialogs. 7 * (3 + 14 * 6) + 3 = 612
	static const uint32 EventsPerEntity = ActivitySequencesPerEntity * (3 + ActivityStepsPerSequence * 6) + 3;
	/// Actions of the activities of an entity. A sequence has 4 (for its start, for the spawn, the
	/// switch of its steps and its end). A step has up to 8: one for each of the 5 events of Hunt In
	/// Zone, 2 for a time limit by a dialog, 1 to go to the step. The entity has 1.
	/// 7 * (4 + 14 * 8) + 1 = 813
	static const uint32 ActivityActionsPerEntity = ActivitySequencesPerEntity * (4 + ActivityStepsPerSequence * 8) + 1;
	/// Actions of the dialogs of an entity. The entity has 2, a dialog 2 (the switch of its steps and
	/// the first wait), a step 5 (the editor gives a step one line: the step, say, emote, facing and
	/// the wait). The editor has no maximum for dialogs: the ones of the activities are taken.
	/// 2 + 7 * (2 + 14 * 5) = 506
	static const uint32 DialogActionsPerEntity = 2 + ActivitySequencesPerEntity * (2 + ActivityStepsPerSequence * 5);
	/// Texts of an entity: one for each step of its dialogs, counted as above: 7 * 14 = 98
	static const uint32 TextsPerEntity = ActivitySequencesPerEntity * ActivityStepsPerSequence;
	/// Groups for one unit of the quota. An entity or a feature has one group, and a feature that is
	/// made of other components leaves up to two of them out of its cost (the fauna feature), which
	/// makes 3: 4 are allowed.
	static const uint32 GroupsPerQuotaUnit = 4;
	/// What the editor does not limit is allowed as much again: more sequences than 7, the reactions
	/// a user gives an entity, longer dialogs.
	static const uint32 Allowance = 2;

	uint32 MaxDepth;
	uint32 MaxNodes;
	uint32 MaxActs;
	uint32 MaxPlotItems;
	uint32 MaxPlotItemTextSize;
	/// Set with setBotQuota(), which derives the limits below from it
	uint32 MaxBotsPerAct;
	uint32 MaxAliasesPerAct;
	/// User triggers of one act. A user trigger is a feature that takes one unit of the quota: an
	/// act has at most as many as the quota allows (MaxBotsPerAct).
	uint32 MaxUserTriggersPerAct;
	/// States of one act (MaxBotsPerAct * StatesPerEntity * Allowance, 86100 by default)
	uint32 MaxStatesPerAct;
	/// Groups the states of one act name (MaxBotsPerAct * GroupsPerQuotaUnit, 820 by default)
	uint32 MaxGroupsPerAct;
	/// Events of one act (MaxBotsPerAct * EventsPerEntity * Allowance, 250920 by default)
	uint32 MaxEventsPerAct;
	/// Actions the events of one act start, the actions below other actions included
	/// (MaxBotsPerAct * (ActivityActionsPerEntity + DialogActionsPerEntity) * Allowance, 540790 by
	/// default)
	uint32 MaxActionsPerAct;
	/// Texts of the scenario (MaxActs * MaxBotsPerAct * TextsPerEntity * Allowance, 441980 by default)
	uint32 MaxTexts;
	/// Where a position can be, in metres. The service gives the rectangle that holds every island
	/// it knows and the origin, which is where the editor puts what has no position, with a margin
	/// (RtScenarioIslandMargin). The default is every position the AI service can hold.
	sint32 MinX;
	sint32 MaxX;
	sint32 MinY;
	sint32 MaxY;

	CRtScenarioLimits()
	    : MaxDepth(DefaultMaxDepth)
	    , MaxNodes(DefaultMaxNodes)
	    , MaxActs(DefaultMaxActs)
	    , MaxPlotItems(DefaultMaxPlotItems)
	    , MaxPlotItemTextSize(DefaultMaxPlotItemTextSize)
	    , MaxAliasesPerAct(DefaultMaxAliasesPerAct)
	    , MinX(-RtScenarioMaxCoordinate)
	    , MaxX(RtScenarioMaxCoordinate)
	    , MinY(-RtScenarioMaxCoordinate)
	    , MaxY(RtScenarioMaxCoordinate)
	{
		setBotQuota(DefaultMaxBotsPerAct);
	}

	/// Sets the quota of an act, and the limits that are derived from it
	void setBotQuota(uint32 maxBotsPerAct)
	{
		MaxBotsPerAct = maxBotsPerAct;
		MaxUserTriggersPerAct = maxBotsPerAct;
		MaxStatesPerAct = multiply(maxBotsPerAct, StatesPerEntity * Allowance);
		MaxGroupsPerAct = multiply(maxBotsPerAct, GroupsPerQuotaUnit);
		MaxEventsPerAct = multiply(maxBotsPerAct, EventsPerEntity * Allowance);
		MaxActionsPerAct = multiply(maxBotsPerAct, (ActivityActionsPerEntity + DialogActionsPerEntity) * Allowance);
		MaxTexts = multiply(multiply(MaxActs, maxBotsPerAct), TextsPerEntity * Allowance);
	}

	/// A product that stays at the largest value instead of wrapping around
	static uint32 multiply(uint32 a, uint32 b)
	{
		uint64 product = (uint64)a * (uint64)b;
		return product > 0xFFFFFFFFu ? 0xFFFFFFFFu : (uint32)product;
	}
};

enum TRtScenarioVerdict
{
	RtScenarioOk = 0,
	/// The tree is not a run time scenario the service can translate: something is missing, has
	/// another type than the one that is read, names an element that does not exist or that is
	/// named already, or is out of range
	RtScenarioInvalid,
	/// The scenario is well formed and has more of something than the service allows: bots, states,
	/// groups, user triggers, events or actions in an act, or texts
	RtScenarioOverQuota
};

/**
 * Checks the run time scenario a client uploaded: everything the animation module reads from the
 * tree when it translates it into primitives for the AI service, items and texts, and afterwards
 * when it runs the session. A tree that passes can be translated without a missing value, a value
 * of an unexpected type or an index out of range. Attributes that are not read are not looked at.
 *
 * A group belongs to one state, an npc to one group, and an action to one event or to one action
 * above it, as in every tree the editor makes: a group, an npc or an action that is named a second
 * time is refused. Every table is then checked once, and translated once afterwards: the time
 * both take grows with the size of the tree, which MaxNodes bounds.
 *
 * rtScenario can be NULL (an empty upload, or one whose compressed data could not be read), which
 * is refused like any other tree that is not a scenario.
 *
 * Returns RtScenarioOk, or the reason for the refusal with a description in error (one line, that
 * names the place in the tree and the value; text taken from the tree is shortened and has its
 * control characters replaced, so it can go to the log).
 */
TRtScenarioVerdict checkRtScenario(const CObject *rtScenario, const CRtScenarioLimits &limits, std::string &error);

/**
 * Checks the act a client asks the scenario to start in: it is one of the acts of the tree and not
 * the base act (index 0), which is always loaded and never started. A scenario with only a base
 * act cannot be started.
 */
TRtScenarioVerdict checkRtScenarioStartingAct(const CObject *rtScenario, uint32 startingAct, std::string &error);

} // namespace R2

#endif // R2_RT_SCENARIO_CHECK_H
