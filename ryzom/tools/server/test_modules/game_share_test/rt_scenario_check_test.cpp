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

#include "nel/misc/types_nl.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/string_common.h"
#include "nel/misc/time_nl.h"
#include "game_share/object.h"
#include "game_share/rt_scenario_check.h"
#include "game_share/scenario_text_parser.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <limits>
#include <string>

using namespace R2;

namespace {

// A run time scenario with the shape the translator of the editor gives it (r2_translator.lua and
// the Rt classes of r2_components.lua): a base act and one act, two locations, a plot item, a text,
// and in each act a state that holds a group of npcs, an event with nested actions, and in the act
// a user trigger.
const char *s_Scenario = "scenario = {\n"
                         "	Class = [[RtScenario]],\n"
                         "	PlotItems = {\n"
                         "		{ Class = [[RtPlotItem]], Id = [[RtPlotItem_1]], SheetId = 4660, Name = [[Old key]], Description = [[Opens the gate]], Comment = [[]] },\n"
                         "	},\n"
                         "	Locations = {\n"
                         "		{ Class = [[RtLocation]], Id = [[RtLocation_1]], Island = [[uiR2_Lakes01]], EntryPoint = [[uiR2_Lakes01_1]], Season = 0 },\n"
                         "		{ Class = [[RtLocation]], Id = [[RtLocation_2]], Island = [[uiR2_Deserts03]], EntryPoint = [[uiR2_Deserts03_2]], Season = 4 },\n"
                         "	},\n"
                         "	Texts = { Class = [[RtTextManager]], Id = [[RtTextManager_1]], Texts = {\n"
                         "		{ Class = [[RtEntryText]], Id = [[RtEntryText_1]], Text = [[Welcome]] },\n"
                         "	} },\n"
                         "	Acts = {\n"
                         "		{\n"
                         "			Class = [[RtAct]], Id = [[RtAct_1]], Name = [[Permanent]], ActDescription = [[]], PreActDescription = [[]],\n"
                         "			LocationId = 0, WeatherValue = 0, IslandName = [[]], Season = 0,\n"
                         "			FaunaGrps = { },\n"
                         "			NpcGrps = {\n"
                         "				{ Class = [[RtNpcGrp]], Id = [[RtNpcGrp_1]], Name = [[RtNpcGrp_1]], Children = { [[RtNpc_1]], [[RtNpc_2]] }, AutoSpawn = 1,\n"
                         "					BotChat_parameters = [[]], BotEquipment = [[]], BotSheetClient = [[]], BotVerticalPos = [[auto]], Count = 0,\n"
                         "					GrpKeywords = [[]], GrpParameters = [[ring\n]], AiProfilParams = [[]] },\n"
                         "			},\n"
                         "			AiStates = {\n"
                         "				{ Class = [[RtAiState]], Id = [[RtAiState_1]], Name = [[Client1_10.init]], Children = { [[RtNpcGrp_1]] },\n"
                         "					AiActivity = [[normal]], AiMovement = [[stand_on_start_point]], AiProfileParams = 0, Keywords = [[]], VerticalPos = [[auto]],\n"
                         "					Pts = { { x = 21570.5, y = -1363.25, z = -20 }, { x = 21580, y = -1370, z = -21 } }, Reactions = { } },\n"
                         "			},\n"
                         "			Npcs = {\n"
                         "				{ Class = [[RtNpc]], Id = [[RtNpc_1]], Name = [[Guard]], Children = { }, ChatParameters = [[]], Equipment = [[]], IsStuck = 0,\n"
                         "					Keywords = [[]], Sheet = [[ring_guard_melee_tank_slash_f4.creature]], SheetClient = [[basic_fyros_male.creature]],\n"
                         "					BotVerticalPos = [[auto]], Angle = 1.5, DmProperty = 14, Pt = { x = 21571, y = -1364, z = -20 } },\n"
                         "				{ Class = [[RtNpc]], Id = [[RtNpc_2]], Name = [[]], Children = { }, ChatParameters = [[]], Equipment = [[]], IsStuck = 1,\n"
                         "					Keywords = [[]], Sheet = [[]], SheetClient = [[object_tent_fyros.creature]],\n"
                         "					BotVerticalPos = [[auto]], Angle = 0, DmProperty = 0, Pt = { x = 21572, y = -1365, z = -20 } },\n"
                         "			},\n"
                         "			Events = {\n"
                         "				{ Class = [[RtNpcEventHandler]], Id = [[RtNpcEventHandler_1]], Name = [[]], Event = [[start_of_state]],\n"
                         "					StatesByName = [[RtAiState_1]], GroupsByName = [[RtNpcGrp_1]], ActionsId = { [[RtNpcEventHandlerAction_1]] } },\n"
                         "			},\n"
                         "			Actions = {\n"
                         "				{ Class = [[RtNpcEventHandlerAction]], Id = [[RtNpcEventHandlerAction_1]], Action = [[multi_actions]], Name = [[]], Parameters = [[]], Weight = 1,\n"
                         "					Children = {\n"
                         "						{ Class = [[RtNpcEventHandlerAction]], Id = [[RtNpcEventHandlerAction_2]], Action = [[condition_if]], Name = [[]],\n"
                         "							Parameters = [[v0 == 1]], Weight = 1, Children = {\n"
                         "								{ Class = [[RtNpcEventHandlerAction]], Id = [[RtNpcEventHandlerAction_3]], Action = [[npc_say]], Name = [[]],\n"
                         "									Parameters = [[RtNpcGrp_1:Guard\nRtEntryText_1]], Weight = 1, Children = { } },\n"
                         "							} },\n"
                         "					} },\n"
                         "			},\n"
                         "			UserTriggers = { },\n"
                         "		},\n"
                         "		{\n"
                         "			Class = [[RtAct]], Id = [[RtAct_2]], Name = [[Act 1]], ActDescription = [[The gate]], PreActDescription = [[]],\n"
                         "			LocationId = 1, WeatherValue = 512, IslandName = [[]], Season = 0,\n"
                         "			FaunaGrps = { },\n"
                         "			NpcGrps = {\n"
                         "				{ Class = [[RtNpcGrp]], Id = [[RtNpcGrp_2]], Name = [[RtNpcGrp_2]], Children = { [[RtNpc_3]] }, AutoSpawn = 0,\n"
                         "					BotChat_parameters = [[]], BotEquipment = [[]], BotSheetClient = [[]], BotVerticalPos = [[auto]], Count = 0,\n"
                         "					GrpKeywords = [[]], GrpParameters = [[]], AiProfilParams = [[]] },\n"
                         "			},\n"
                         "			AiStates = {\n"
                         "				{ Class = [[RtAiState]], Id = [[RtAiState_2]], Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },\n"
                         "					AiActivity = [[normal]], AiMovement = [[follow_route]], AiProfileParams = 0, Keywords = [[]], VerticalPos = [[auto]],\n"
                         "					Pts = { }, Reactions = { }, IsTriggerZone = 1 },\n"
                         "			},\n"
                         "			Npcs = {\n"
                         "				{ Class = [[RtNpc]], Id = [[RtNpc_3]], Name = [[Yubo]], Children = { }, ChatParameters = [[]], Equipment = [[]], IsStuck = 0,\n"
                         "					Keywords = [[]], Sheet = [[]], SheetClient = [[chdfa1.creature]],\n"
                         "					BotVerticalPos = [[auto]], Angle = 0, DmProperty = 7, Pt = { x = 30000, y = -2000, z = 0 } },\n"
                         "			},\n"
                         "			Events = {\n"
                         "				{ Class = [[RtNpcEventHandler]], Id = [[RtNpcEventHandler_2]], Name = [[]], Event = [[user_event_1]],\n"
                         "					StatesByName = [[RtAiState_2]], GroupsByName = [[RtNpcGrp_2]], ActionsId = { [[RtNpcEventHandlerAction_4]] }, IsTriggerZone = 1 },\n"
                         "			},\n"
                         "			Actions = {\n"
                         "				{ Class = [[RtNpcEventHandlerAction]], Id = [[RtNpcEventHandlerAction_4]], Action = [[begin_state]], Name = [[]],\n"
                         "					Parameters = [[RtAiState_2]], Weight = 1, Children = { } },\n"
                         "			},\n"
                         "			UserTriggers = {\n"
                         "				{ Class = [[RtUserTrigger]], Id = [[RtUserTrigger_1]], Name = [[Open the gate]], Grp = [[RtNpcGrp_2]], TriggerId = 1 },\n"
                         "			},\n"
                         "		},\n"
                         "	},\n"
                         "}\n";

CObject::TSmartPtr parse(const std::string &text)
{
	std::string error;
	CObject::TSmartPtr result = CScenarioTextParser::parse(text, nullptr, error);
	EXPECT_TRUE(result != nullptr) << error;
	return result;
}

/// The scenario text with one part of it, which must be there exactly once, replaced
std::string with(const std::string &what, const std::string &replacement)
{
	std::string text = s_Scenario;
	std::string::size_type pos = text.find(what);
	EXPECT_NE(std::string::npos, pos) << "not in the scenario: " << what;
	if (pos == std::string::npos)
		return text;
	EXPECT_EQ(std::string::npos, text.find(what, pos + 1)) << "more than once in the scenario: " << what;
	text.replace(pos, what.size(), replacement);
	return text;
}

TRtScenarioVerdict check(const CObject *tree, std::string &error, const CRtScenarioLimits &limits = CRtScenarioLimits())
{
	return checkRtScenario(tree, limits, error);
}

void expectAccepted(const std::string &text, const CRtScenarioLimits &limits = CRtScenarioLimits())
{
	CObject::TSmartPtr tree = parse(text);
	ASSERT_TRUE(tree != nullptr);
	std::string error;
	EXPECT_EQ(RtScenarioOk, check(tree, error, limits)) << error;
	EXPECT_TRUE(error.empty()) << error;
}

/// The check refuses the text with the verdict, and the description names the place given
void expectRefused(const std::string &text, const std::string &place, TRtScenarioVerdict verdict = RtScenarioInvalid, const CRtScenarioLimits &limits = CRtScenarioLimits())
{
	CObject::TSmartPtr tree = parse(text);
	ASSERT_TRUE(tree != nullptr);
	std::string error;
	EXPECT_EQ(verdict, check(tree, error, limits)) << "accepted, or refused for another reason: " << error << "\nexpected at: " << place;
	EXPECT_NE(std::string::npos, error.find(place)) << error << "\nexpected at: " << place;
}

/// What the tree is after the copy the scenario service makes of it between its two modules
CObject::TSmartPtr throughSerialForm(CObject *tree)
{
	CObjectSerializerServer out(tree);
	NLMISC::CMemStream stream;
	out.serial(stream);
	stream.invert();
	CObjectSerializerServer in;
	in.serial(stream);
	return in.getData();
}

std::string repeat(const std::string &text, uint32 count)
{
	std::string out;
	out.reserve(text.size() * count);
	for (uint32 i = 0; i < count; ++i)
		out += text;
	return out;
}

// Trees for the tests of the time the check takes. They are built with the classes of the tree: the
// text of a tree of a million values would take longer to read than to check.

CObjectTable *addTable(CObject *parent, const std::string &key)
{
	CObjectTable *table = new CObjectTable();
	parent->add(key, CObject::TSmartPtr(table));
	return table;
}

/// An act with nothing in it, in the base location
CObjectTable *addAct(CObject *acts)
{
	CObjectTable *act = addTable(acts, "");
	act->add("Name", std::string("act"));
	act->add("ActDescription", std::string());
	act->add("PreActDescription", std::string());
	act->add("LocationId", (sint64)0);
	addTable(act, "Npcs");
	addTable(act, "AiStates");
	addTable(act, "Events");
	addTable(act, "UserTriggers");
	return act;
}

/// A scenario with one location and as many empty acts as the limits allow; acts receives their table
CObject::TSmartPtr newScenario(CObject *&acts, uint32 actCount = CRtScenarioLimits::DefaultMaxActs)
{
	CObjectTable *root = new CObjectTable();
	CObject::TSmartPtr keep(root);
	addTable(root, "PlotItems");
	CObjectTable *location = addTable(addTable(root, "Locations"), "");
	location->add("Season", (sint64)0);
	location->add("Island", std::string("uiR2_Lakes01"));
	location->add("EntryPoint", std::string("uiR2_Lakes01_1"));
	acts = addTable(root, "Acts");
	for (uint32 i = 0; i < actCount; ++i)
		addAct(acts);
	return keep;
}

CObject *getAct(CObject *acts, uint32 index)
{
	return acts->getValueAtPos(index);
}

/// A table with an id and, after count attributes nothing reads, what is given to add
CObjectTable *addWideTable(CObject *parent, const std::string &id, uint32 count)
{
	CObjectTable *table = addTable(parent, "");
	for (uint32 i = 0; i < count; ++i)
		table->add(NLMISC::toString("Unread%u", i), (sint64)i);
	table->add("Id", id);
	return table;
}

/// The time a check may take in these tests. The largest trees are checked in a fraction of a second
/// in a build without optimisation; a check that goes through a table once for each time it is named,
/// or that searches its ids one after the other, takes tens of seconds on the same trees. The bound
/// is between the two, far enough from the first that a slow or busy machine stays under it.
const double s_MaxCheckSeconds = 10.0;

/// Checks the tree, tells the time it took, and expects the verdict within the bound
void expectCheckedInTime(const char *what, const CObject *tree, TRtScenarioVerdict verdict, const CRtScenarioLimits &limits = CRtScenarioLimits())
{
	std::string error;
	NLMISC::TTicks start = NLMISC::CTime::getPerformanceTime();
	TRtScenarioVerdict result = checkRtScenario(tree, limits, error);
	double seconds = NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime() - start);
	printf("[   TIME   ] %s: %.0f ms\n", what, seconds * 1000.0);
	EXPECT_EQ(verdict, result) << what << ": " << error;
	EXPECT_LT(seconds, s_MaxCheckSeconds) << what;
}

/// An action that holds nothing, below a table of actions; with an id when one is given
CObjectTable *addAction(CObject *parent, const std::string &id = std::string())
{
	CObjectTable *action = addTable(parent, "");
	if (!id.empty())
		action->add("Id", id);
	action->add("Action", std::string("code"));
	addTable(action, "Children");
	return action;
}

/// A state that names no group
CObjectTable *addState(CObject *states)
{
	CObjectTable *state = addTable(states, "");
	state->add("AiMovement", std::string());
	addTable(state, "Children");
	return state;
}

/// An npc with what is read of it
CObjectTable *addNpc(CObject *npcs, const std::string &id)
{
	CObjectTable *npc = addTable(npcs, "");
	npc->add("Id", id);
	npc->add("Name", std::string("npc"));
	npc->add("ChatParameters", std::string());
	npc->add("Keywords", std::string());
	npc->add("SheetClient", std::string("chdfa1.creature"));
	npc->add("Sheet", std::string());
	return npc;
}

/// FNV-1a, the hash an earlier index of the ids was made with
uint32 fnv1a(const char *text, size_t size)
{
	uint32 hash = 2166136261u;
	for (size_t i = 0; i < size; ++i)
	{
		hash ^= (uint8)text[i];
		hash *= 16777619u;
	}
	return hash;
}

} // namespace

TEST(RtScenarioCheck, AcceptsTheShapeTheTranslatorMakes)
{
	expectAccepted(s_Scenario);

	CObject::TSmartPtr tree = parse(s_Scenario);
	ASSERT_TRUE(tree != nullptr);
	std::string error;
	EXPECT_EQ(RtScenarioOk, checkRtScenarioStartingAct(tree, 1, error)) << error;
}

TEST(RtScenarioCheck, AcceptsTheTreeAfterItsSerialForm)
{
	// the run time classes have their own serial form, which writes their ids as numbers
	CObject::TSmartPtr tree = parse(s_Scenario);
	ASSERT_TRUE(tree != nullptr);
	CObject::TSmartPtr copy = throughSerialForm(tree);
	ASSERT_TRUE(copy != nullptr);
	std::string error;
	EXPECT_EQ(RtScenarioOk, check(copy, error)) << error;
	EXPECT_EQ("RtNpcGrp_1", copy->getAttr("Acts")->getValueAtPos(0)->getAttr("NpcGrps")->getValueAtPos(0)->toString("Id"));
}

TEST(RtScenarioCheck, TheCopyOfATreeIsCheckedAsItIs)
{
	// A table that names a run time class and has an id that is not one the class gives is valid as
	// it is uploaded. The tree that is translated is a copy, made through the serial form: whether
	// the copy keeps such an id is for the serial form to say, and the copy passes when it does and
	// is refused, at the state that no longer finds its group, when it does not.
	std::string text = with("Id = [[RtNpcGrp_2]]", "Id = [[Guards]]");
	std::string::size_type pos = text.find("Children = { [[RtNpcGrp_2]] }");
	ASSERT_NE(std::string::npos, pos);
	text.replace(pos, strlen("Children = { [[RtNpcGrp_2]] }"), "Children = { [[Guards]] }");

	CObject::TSmartPtr tree = parse(text);
	ASSERT_TRUE(tree != nullptr);
	std::string error;
	EXPECT_EQ(RtScenarioOk, check(tree, error)) << error;

	CObject::TSmartPtr copy = throughSerialForm(tree);
	ASSERT_TRUE(copy != nullptr);
	std::string copyId = copy->getAttr("Acts")->getValueAtPos(1)->getAttr("NpcGrps")->getValueAtPos(0)->toString("Id");
	if (copyId == "Guards")
	{
		EXPECT_EQ(RtScenarioOk, check(copy, error)) << error;
	}
	else
	{
		EXPECT_EQ(RtScenarioInvalid, check(copy, error));
		EXPECT_NE(std::string::npos, error.find("'Guards'")) << error;
	}
}

TEST(RtScenarioCheck, RefusesAnUploadWithoutATree)
{
	std::string error;
	EXPECT_EQ(RtScenarioInvalid, check(nullptr, error));
	EXPECT_FALSE(error.empty());

	CObject::TSmartPtr text(new CObjectString("scenario"));
	EXPECT_EQ(RtScenarioInvalid, check(text, error));
	CObject::TSmartPtr number(new CObjectNumber((sint64)1));
	EXPECT_EQ(RtScenarioInvalid, check(number, error));
	CObject::TSmartPtr empty(new CObjectTable());
	EXPECT_EQ(RtScenarioInvalid, check(empty, error));

	EXPECT_EQ(RtScenarioInvalid, checkRtScenarioStartingAct(nullptr, 1, error));
	EXPECT_EQ(RtScenarioInvalid, checkRtScenarioStartingAct(text, 1, error));
	EXPECT_EQ(RtScenarioInvalid, checkRtScenarioStartingAct(empty, 1, error));
}

TEST(RtScenarioCheck, LocationOfAnActIsAnIndexIntoTheLocations)
{
	// two locations: 0 and 1 exist
	expectAccepted(with("LocationId = 1,", "LocationId = 0,"));
	expectRefused(with("LocationId = 1,", "LocationId = 2,"), "Acts[1].LocationId");
	expectRefused(with("LocationId = 1,", "LocationId = 4294967296,"), "Acts[1].LocationId");
	expectRefused(with("LocationId = 1,", "LocationId = -1,"), "Acts[1].LocationId");
	expectRefused(with("LocationId = 1,", "LocationId = 0.5,"), "Acts[1].LocationId");
	expectRefused(with("LocationId = 1,", "LocationId = [[1]],"), "Acts[1].LocationId");
	expectRefused(with("LocationId = 1,", "LocationId = { 1 },"), "Acts[1].LocationId");
	expectRefused(with("LocationId = 1,", ""), "Acts[1].LocationId");

	// the base act keeps the index 0 the editor leaves it, which must exist as well
	expectRefused(with("LocationId = 0,", "LocationId = 2,"), "Acts[0].LocationId");
	std::string noLocation = with("LocationId = 1,", "LocationId = 0,");
	std::string::size_type first = noLocation.find("		{ Class = [[RtLocation]]");
	std::string::size_type end = noLocation.find("	Texts = ");
	ASSERT_NE(std::string::npos, first);
	ASSERT_NE(std::string::npos, end);
	noLocation.replace(first, end - first, "	},\n");
	expectRefused(noLocation, "Acts[0].LocationId");
}

TEST(RtScenarioCheck, StartingActIsOneOfTheActsAndNotTheBaseAct)
{
	CObject::TSmartPtr tree = parse(s_Scenario);
	ASSERT_TRUE(tree != nullptr);
	std::string error;
	EXPECT_EQ(RtScenarioInvalid, checkRtScenarioStartingAct(tree, 0, error));
	EXPECT_FALSE(error.empty());
	EXPECT_EQ(RtScenarioOk, checkRtScenarioStartingAct(tree, 1, error)) << error;
	EXPECT_TRUE(error.empty());
	EXPECT_EQ(RtScenarioInvalid, checkRtScenarioStartingAct(tree, 2, error));
	EXPECT_EQ(RtScenarioInvalid, checkRtScenarioStartingAct(tree, 0xFFFFFFFFu, error));

	// a scenario with only its base act has no act to start in
	CObject::TSmartPtr baseOnly = parse("scenario = { Acts = { { } } }");
	ASSERT_TRUE(baseOnly != nullptr);
	EXPECT_EQ(RtScenarioInvalid, checkRtScenarioStartingAct(baseOnly, 0, error));
	EXPECT_EQ(RtScenarioInvalid, checkRtScenarioStartingAct(baseOnly, 1, error));
}

TEST(RtScenarioCheck, RefusesMissingOrMistypedParts)
{
	// the scenario
	expectRefused("scenario = { }", "PlotItems");
	expectRefused(with("	PlotItems = {\n", "	Items = {\n"), "PlotItems");
	expectRefused(with("	Locations = {\n", "	Places = {\n"), "Locations");
	expectRefused(with("	Acts = {\n", "	Parts = {\n"), "Acts");
	expectRefused("scenario = { PlotItems = { }, Locations = { }, Acts = { } }", "Acts");
	expectRefused("scenario = { PlotItems = { }, Locations = { }, Acts = { 1 } }", "Acts[0]");

	// the plot items and the locations
	expectRefused(with("SheetId = 4660,", "SheetId = [[4660]],"), "PlotItems[0].SheetId");
	expectRefused(with("SheetId = 4660,", "SheetId = -1,"), "PlotItems[0].SheetId");
	expectRefused(with("SheetId = 4660,", "SheetId = 4294967296,"), "PlotItems[0].SheetId");
	expectRefused(with("Description = [[Opens the gate]],", ""), "PlotItems[0].Description");
	expectRefused(with("Comment = [[]] }", "Comment = 0 }"), "PlotItems[0].Comment");
	expectRefused(with("Island = [[uiR2_Deserts03]],", ""), "Locations[1].Island");
	expectRefused(with("EntryPoint = [[uiR2_Deserts03_2]],", "EntryPoint = 2,"), "Locations[1].EntryPoint");
	expectRefused(with("Season = 4 }", "Season = [[Winter]] }"), "Locations[1].Season");

	// the texts: none is fine, half of one is not
	expectAccepted(with("Texts = { Class = [[RtTextManager]], Id = [[RtTextManager_1]], Texts = {", "NoTexts = { A = {"));
	expectRefused(with("Texts = { Class = [[RtTextManager]], Id = [[RtTextManager_1]], Texts = {", "Texts = { Class = [[RtTextManager]], Lines = {"), "Texts.Texts");
	expectRefused(with("Text = [[Welcome]]", "Text = { }"), "Texts.Texts[0].Text");
	expectRefused(with("Id = [[RtEntryText_1]],", ""), "Texts.Texts[0].Id");

	// the acts
	expectRefused(with("Name = [[Act 1]],", ""), "Acts[1].Name");
	expectRefused(with("ActDescription = [[The gate]],", "ActDescription = 1,"), "Acts[1].ActDescription");
	expectRefused(with("ActDescription = [[The gate]], PreActDescription = [[]],", "ActDescription = [[The gate]],"), "Acts[1].PreActDescription");
	expectRefused(with("			UserTriggers = { },\n", ""), "Acts[0].UserTriggers");
	expectRefused(with("Grp = [[RtNpcGrp_2]],", ""), "Acts[1].UserTriggers[0].Grp");
	expectRefused(with("Name = [[Open the gate]],", "Name = 5,"), "Acts[1].UserTriggers[0].Name");

	// the states
	expectRefused(with("AiMovement = [[follow_route]],", ""), "Acts[1].AiStates[0].AiMovement");
	expectRefused(with("Pts = { }, Reactions", "Pts = 3, Reactions"), "Acts[1].AiStates[0].Pts");
	expectRefused(with("Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },", "Name = [[Client1_20.init]],"), "Acts[1].AiStates[0].Children");
	expectRefused(with("Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },", "Name = [[Client1_20.init]], Children = { 2 },"), "Acts[1].AiStates[0].Children[0]");
	expectRefused(with("Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },", "Name = [[Client1_20.init]], Children = { [[RtNpcGrp_9]] },"), "'RtNpcGrp_9'");
	// a state may be left without its points, and holds no group when it is only reached later
	expectAccepted(with("Pts = { }, Reactions = { }, IsTriggerZone = 1", "Reactions = { }"));
	expectAccepted(with("Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },", "Name = [[Client1_20.init]], Children = { },"));

	// the groups and their npcs
	expectRefused(with("Name = [[RtNpcGrp_2]], Children = { [[RtNpc_3]] },", "Name = [[RtNpcGrp_2]],"), "'RtNpcGrp_2'");
	expectRefused(with("Name = [[RtNpcGrp_2]], Children = { [[RtNpc_3]] },", "Name = [[RtNpcGrp_2]], Children = { { } },"), "'RtNpcGrp_2'.Children[0]");
	expectRefused(with("Name = [[RtNpcGrp_2]], Children = { [[RtNpc_3]] },", "Name = [[RtNpcGrp_2]], Children = { [[RtNpc_9]] },"), "'RtNpc_9'");
	expectRefused(with("SheetClient = [[chdfa1.creature]],", ""), "'RtNpc_3'");
	expectRefused(with("Name = [[Yubo]],", "Name = 3,"), "'RtNpc_3'");
	expectRefused(with("Sheet = [[]], SheetClient = [[chdfa1.creature]],", "SheetClient = [[chdfa1.creature]],"), "'RtNpc_3'");

	// the events and their actions
	expectRefused(with("			Events = {\n				{ Class = [[RtNpcEventHandler]], Id = [[RtNpcEventHandler_2]]", "			Events = { 1,\n				{ Class = [[RtNpcEventHandler]], Id = [[RtNpcEventHandler_2]]"), "Acts[1].Events[0]");
	expectRefused(with("ActionsId = { [[RtNpcEventHandlerAction_4]] }", "ActionsId = [[RtNpcEventHandlerAction_4]]"), "Acts[1].Events[0].ActionsId");
	expectRefused(with("ActionsId = { [[RtNpcEventHandlerAction_4]] }", "ActionsId = { 4 }"), "Acts[1].Events[0].ActionsId[0]");
	expectRefused(with("ActionsId = { [[RtNpcEventHandlerAction_4]] }", "ActionsId = { [[RtNpcEventHandlerAction_9]] }"), "'RtNpcEventHandlerAction_9'");
	expectRefused(with("Action = [[begin_state]],", ""), "'RtNpcEventHandlerAction_4'");
	expectRefused(with("Parameters = [[RtAiState_2]], Weight = 1, Children = { } },", "Parameters = [[RtAiState_2]], Weight = 1 },"), "'RtNpcEventHandlerAction_4'");
	expectRefused(with("Parameters = [[RtAiState_2]], Weight = 1,", "Parameters = [[RtAiState_2]], Weight = [[1]],"), "'RtNpcEventHandlerAction_4'");
	expectRefused(with("Parameters = [[RtAiState_2]], Weight = 1,", "Parameters = [[RtAiState_2]], Weight = -1,"), "'RtNpcEventHandlerAction_4'");
	// an action below another one, and the parameters of an action that reads them as they are
	expectRefused(with("Parameters = [[RtNpcGrp_1:Guard\nRtEntryText_1]], Weight = 1, Children = { } },", "Weight = 1 },"), "'RtNpcEventHandlerAction_3'");
	expectRefused(with("Parameters = [[v0 == 1]],", ""), "'RtNpcEventHandlerAction_2'");
	expectRefused(with("Parameters = [[v0 == 1]],", "Parameters = 1,"), "'RtNpcEventHandlerAction_2'");
	// the other actions leave parameters alone that are not a text, and an event can have no action
	expectAccepted(with("Parameters = [[RtAiState_2]],", ""));
	expectAccepted(with("ActionsId = { [[RtNpcEventHandlerAction_4]] },", ""));
	expectAccepted(with("			Events = {\n				{ Class = [[RtNpcEventHandler]], Id = [[RtNpcEventHandler_2]]", "			NoEvents = {\n				{ Class = [[RtNpcEventHandler]], Id = [[RtNpcEventHandler_2]]"));
}

TEST(RtScenarioCheck, RangesOfTheValuesThatAreUsedAsTheyAre)
{
	expectAccepted(with("WeatherValue = 512,", "WeatherValue = 1023,"));
	expectAccepted(with("WeatherValue = 512,", "WeatherValue = 511.5,"));
	expectAccepted(with("WeatherValue = 512,", ""));
	expectRefused(with("WeatherValue = 512,", "WeatherValue = 1024,"), "Acts[1].WeatherValue");
	expectRefused(with("WeatherValue = 512,", "WeatherValue = -1,"), "Acts[1].WeatherValue");
	expectRefused(with("WeatherValue = 512,", "WeatherValue = 65536,"), "Acts[1].WeatherValue");
	expectRefused(with("WeatherValue = 512,", "WeatherValue = [[512]],"), "Acts[1].WeatherValue");

	expectRefused(with("Season = 4 }", "Season = 5 }"), "Locations[1].Season");
	expectRefused(with("Season = 4 }", "Season = 256 }"), "Locations[1].Season");
	expectRefused(with("Season = 4 }", "Season = -1 }"), "Locations[1].Season");

	// a coordinate or an angle that is not a number is left alone by the translation; one that is
	// a number must be a finite one
	expectAccepted(with("Pt = { x = 30000, y = -2000, z = 0 }", "Pt = { x = [[east]], y = -2000 }"));
	CObject::TSmartPtr tree = parse(s_Scenario);
	ASSERT_TRUE(tree != nullptr);
	CObject *npc = tree->getAttr("Acts")->getValueAtPos(1)->getAttr("Npcs")->getValueAtPos(0);
	ASSERT_TRUE(npc != nullptr);
	std::string error;
	npc->getAttr("Pt")->set("x", std::numeric_limits<double>::infinity());
	EXPECT_EQ(RtScenarioInvalid, check(tree, error));
	EXPECT_NE(std::string::npos, error.find("'RtNpc_3'")) << error;
	npc->getAttr("Pt")->set("x", 30000.0);
	EXPECT_EQ(RtScenarioOk, check(tree, error)) << error;
	npc->set("Angle", std::numeric_limits<double>::quiet_NaN());
	EXPECT_EQ(RtScenarioInvalid, check(tree, error));
	npc->set("Angle", 0.0);
	CObject *point = tree->getAttr("Acts")->getValueAtPos(0)->getAttr("AiStates")->getValueAtPos(0)->getAttr("Pts")->getValueAtPos(1);
	ASSERT_TRUE(point != nullptr);
	point->set("y", -std::numeric_limits<double>::infinity());
	EXPECT_EQ(RtScenarioInvalid, check(tree, error));
	EXPECT_NE(std::string::npos, error.find("Acts[0].AiStates[0].Pts[1]")) << error;
}

TEST(RtScenarioCheck, QuotaOfBotsIsForTheBaseActAndOneActTogether)
{
	// the base act has two npcs and the act one
	CRtScenarioLimits limits;
	limits.MaxBotsPerAct = 3;
	expectAccepted(s_Scenario, limits);
	limits.MaxBotsPerAct = 2;
	expectRefused(s_Scenario, "Acts[1]", RtScenarioOverQuota, limits);
	limits.MaxBotsPerAct = 1;
	expectRefused(s_Scenario, "Acts[0]", RtScenarioOverQuota, limits);

	// the npcs that are listed count, as they always did
	limits.MaxBotsPerAct = 3;
	std::string listed = with("			Npcs = {\n				{ Class = [[RtNpc]], Id = [[RtNpc_3]]", "			Npcs = { { }, \n				{ Class = [[RtNpc]], Id = [[RtNpc_3]]");
	expectRefused(listed, "Acts[1]", RtScenarioOverQuota, limits);

	// and so do the bots that are made, when not every one of them is listed
	std::string unlisted = "			UserTriggers = { },\n"
	                       "			Unlisted = { { Id = [[RtNpc_8]], Name = [[]], ChatParameters = [[]], Keywords = [[]], Sheet = [[]], SheetClient = [[]] },\n"
	                       "				{ Id = [[RtNpc_9]], Name = [[]], ChatParameters = [[]], Keywords = [[]], Sheet = [[]], SheetClient = [[]] } },\n";
	std::string made = with("			UserTriggers = { },\n", unlisted);
	std::string::size_type pos = made.find("Children = { [[RtNpc_1]], [[RtNpc_2]] }");
	ASSERT_NE(std::string::npos, pos);
	made.replace(pos, strlen("Children = { [[RtNpc_1]], [[RtNpc_2]] }"), "Children = { [[RtNpc_1]], [[RtNpc_2]], [[RtNpc_8]], [[RtNpc_9]] }");
	limits.MaxBotsPerAct = 5;
	expectAccepted(made, limits);
	limits.MaxBotsPerAct = 4;
	expectRefused(made, "Acts[1]", RtScenarioOverQuota, limits);
	limits.MaxBotsPerAct = 3;
	expectRefused(made, "Acts[0]", RtScenarioOverQuota, limits);
}

TEST(RtScenarioCheck, AGroupBelongsToOneStateAndAnNpcToOneGroup)
{
	// in one state
	expectRefused(with("Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },", "Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]], [[RtNpcGrp_2]] },"), "'RtNpcGrp_2' is named a second time");
	// in a state of another act
	expectRefused(with("Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },", "Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]], [[RtNpcGrp_1]] },"), "'RtNpcGrp_1' is named a second time");
	// an npc twice in its group, and in two groups
	expectRefused(with("Name = [[RtNpcGrp_2]], Children = { [[RtNpc_3]] },", "Name = [[RtNpcGrp_2]], Children = { [[RtNpc_3]], [[RtNpc_3]] },"), "'RtNpc_3' is named a second time");
	expectRefused(with("Name = [[RtNpcGrp_2]], Children = { [[RtNpc_3]] },", "Name = [[RtNpcGrp_2]], Children = { [[RtNpc_3]], [[RtNpc_1]] },"), "'RtNpc_1' is named a second time");

	// The second naming is refused where it is, however often the id is named: the refusal does not
	// wait for a limit, and nothing is looked up again in a table that was checked.
	std::string many = with("Name = [[RtNpcGrp_2]], Children = { [[RtNpc_3]] },", "Name = [[RtNpcGrp_2]], Children = { " + repeat("[[RtNpc_3]], ", 5000) + "},");
	expectRefused(many, "'RtNpc_3' is named a second time");

	// an id that names a table of another kind: a state as its own group, an act as a group, a
	// group as one of its npcs
	expectRefused(with("Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },", "Name = [[Client1_20.init]], Children = { [[RtAiState_2]] },"), "'RtAiState_2'");
	expectRefused(with("Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },", "Name = [[Client1_20.init]], Children = { [[RtAct_2]] },"), "the group 'RtAct_2'.Children");
	expectRefused(with("Name = [[RtNpcGrp_2]], Children = { [[RtNpc_3]] },", "Name = [[RtNpcGrp_2]], Children = { [[RtNpcGrp_2]] },"), "the npc 'RtNpcGrp_2'");
	expectRefused(with("ActionsId = { [[RtNpcEventHandlerAction_4]] }", "ActionsId = { [[RtNpcEventHandler_2]] }"), "the action 'RtNpcEventHandler_2'.Action");
}

TEST(RtScenarioCheck, AnActionBelongsToOneEventOrToOneAction)
{
	// twice by one event, and by the events of two acts
	expectRefused(with("ActionsId = { [[RtNpcEventHandlerAction_4]] }", "ActionsId = { [[RtNpcEventHandlerAction_4]], [[RtNpcEventHandlerAction_4]] }"), "the action 'RtNpcEventHandlerAction_4' is named a second time");
	expectRefused(with("ActionsId = { [[RtNpcEventHandlerAction_4]] }", "ActionsId = { [[RtNpcEventHandlerAction_4]], [[RtNpcEventHandlerAction_1]] }"), "the action 'RtNpcEventHandlerAction_1' is named a second time");

	// An action that is below another one belongs to it. An event that names it as well is refused,
	// whichever of the two comes first.
	expectRefused(with("ActionsId = { [[RtNpcEventHandlerAction_4]] }", "ActionsId = { [[RtNpcEventHandlerAction_4]], [[RtNpcEventHandlerAction_3]] }"), "the action 'RtNpcEventHandlerAction_3' is named a second time");
	expectRefused(with("ActionsId = { [[RtNpcEventHandlerAction_1]] }", "ActionsId = { [[RtNpcEventHandlerAction_1]], [[RtNpcEventHandlerAction_2]] }"), "the action 'RtNpcEventHandlerAction_2' is named a second time");
	expectRefused(with("ActionsId = { [[RtNpcEventHandlerAction_1]] }", "ActionsId = { [[RtNpcEventHandlerAction_2]], [[RtNpcEventHandlerAction_1]] }"), "the action 'RtNpcEventHandlerAction_2' is named a second time");

	// an event can name several actions, each of them once
	std::string several = "				{ Id = [[RtNpcEventHandlerAction_5]], Action = [[code]], Children = { } },\n"
	                      "				{ Class = [[RtNpcEventHandlerAction]], Id = [[RtNpcEventHandlerAction_4]]";
	std::string text = with("				{ Class = [[RtNpcEventHandlerAction]], Id = [[RtNpcEventHandlerAction_4]]", several);
	std::string::size_type pos = text.find("ActionsId = { [[RtNpcEventHandlerAction_4]] }");
	ASSERT_NE(std::string::npos, pos);
	text.replace(pos, strlen("ActionsId = { [[RtNpcEventHandlerAction_4]] }"), "ActionsId = { [[RtNpcEventHandlerAction_4]], [[RtNpcEventHandlerAction_5]] }");
	expectAccepted(text);

	// actions below another one that have no id, or the id of a table that comes before them, cannot
	// be named: they are checked with the action they are below
	expectAccepted(with("{ Class = [[RtNpcEventHandlerAction]], Id = [[RtNpcEventHandlerAction_3]], Action = [[npc_say]],", "{ Class = [[RtNpcEventHandlerAction]], Action = [[npc_say]],"));
	expectAccepted(with("{ Class = [[RtNpcEventHandlerAction]], Id = [[RtNpcEventHandlerAction_3]], Action = [[npc_say]],", "{ Class = [[RtNpcEventHandlerAction]], Id = [[RtAct_1]], Action = [[npc_say]],"));
}

TEST(RtScenarioCheck, LimitsOfTheEditor)
{
	CRtScenarioLimits limits;
	EXPECT_EQ(11u, limits.MaxActs);
	EXPECT_EQ(32u, limits.MaxPlotItems);
	EXPECT_EQ(255u, limits.MaxPlotItemTextSize);
	EXPECT_EQ(205u, limits.MaxBotsPerAct);

	// the acts
	std::string emptyAct = "{ Name = [[]], ActDescription = [[]], PreActDescription = [[]], LocationId = 0, Npcs = { }, AiStates = { }, UserTriggers = { } }, ";
	std::string head = "scenario = { PlotItems = { }, Locations = { { Season = 0, Island = [[]], EntryPoint = [[]] } }, Acts = { ";
	expectAccepted(head + repeat(emptyAct, 11) + "} }");
	expectRefused(head + repeat(emptyAct, 12) + "} }", "Acts");

	// the plot items and their texts
	std::string item = "{ SheetId = 1, Name = [[n]], Description = [[d]], Comment = [[c]] }, ";
	std::string tail = "}, Locations = { { Season = 0, Island = [[]], EntryPoint = [[]] } }, Acts = { " + emptyAct + "} }";
	expectAccepted("scenario = { PlotItems = { " + repeat(item, 32) + tail);
	expectRefused("scenario = { PlotItems = { " + repeat(item, 33) + tail, "PlotItems");
	expectAccepted(with("Name = [[Old key]],", "Name = [[" + repeat("k", 255) + "]],"));
	expectRefused(with("Name = [[Old key]],", "Name = [[" + repeat("k", 256) + "]],"), "PlotItems[0].Name");
	expectRefused(with("Description = [[Opens the gate]],", "Description = [[" + repeat("d", 256) + "]],"), "PlotItems[0].Description");
	expectRefused(with("Comment = [[]] }", "Comment = [[" + repeat("c", 256) + "]] }"), "PlotItems[0].Comment");
}

TEST(RtScenarioCheck, SizeOfTheTreeAndOfWhatIsMadeOfIt)
{
	// the sample is 10 tables deep, at the children of its innermost action
	CRtScenarioLimits limits;
	limits.MaxDepth = 10;
	expectAccepted(s_Scenario, limits);
	limits.MaxDepth = 9;
	expectRefused(s_Scenario, "nested", RtScenarioInvalid, limits);

	// deeper than the default, in a place nothing reads
	CObjectTable *root = new CObjectTable();
	CObject::TSmartPtr keep(root);
	CObjectTable *level = root;
	for (uint32 i = 0; i < CScenarioTextParser::MaxDepth; ++i)
	{
		CObjectTable *next = new CObjectTable();
		level->add("Unread", CObject::TSmartPtr(next));
		level = next;
	}
	std::string error;
	EXPECT_EQ(RtScenarioInvalid, check(root, error));
	EXPECT_NE(std::string::npos, error.find("nested")) << error;

	// the number of values
	limits = CRtScenarioLimits();
	limits.MaxNodes = 1000;
	expectAccepted(s_Scenario, limits);
	limits.MaxNodes = 100;
	expectRefused(s_Scenario, "values", RtScenarioInvalid, limits);

	// the aliases of an act: its two managers, and one for each state, group, bot and event
	limits = CRtScenarioLimits();
	limits.MaxAliasesPerAct = 7;
	expectAccepted(s_Scenario, limits);
	limits.MaxAliasesPerAct = 6;
	expectRefused(s_Scenario, "Acts[0]", RtScenarioInvalid, limits);
}

TEST(RtScenarioCheck, ATableWhoseIdIsTakenIsLeftOutAsTheInstanceMapDoes)
{
	// two groups with one id: the first one is the group, whatever the second holds
	std::string second = "				{ Class = [[RtNpcGrp]], Id = [[RtNpcGrp_2]], Name = [[second]] },\n"
	                     "			},\n"
	                     "			AiStates = {\n"
	                     "				{ Class = [[RtAiState]], Id = [[RtAiState_2]]";
	expectAccepted(with("			},\n			AiStates = {\n				{ Class = [[RtAiState]], Id = [[RtAiState_2]]", second));

	// when the first one cannot be a group, the scenario is refused although the second could
	std::string first = "			NpcGrps = {\n"
	                    "				{ Id = [[RtNpcGrp_2]], Name = [[first]] },\n"
	                    "				{ Class = [[RtNpcGrp]], Id = [[RtNpcGrp_2]]";
	expectRefused(with("			NpcGrps = {\n				{ Class = [[RtNpcGrp]], Id = [[RtNpcGrp_2]]", first), "'RtNpcGrp_2'");

	// what such a table holds is not found by its id
	std::string inside = "			NpcGrps = {\n"
	                     "				{ Id = [[RtNpcGrp_2]], Name = [[first]], Children = { } },\n"
	                     "				{ Id = [[RtNpcGrp_2]], Inside = { { Id = [[Hidden_1]], Children = { } } } },\n"
	                     "				{ Class = [[RtNpcGrp]], Id = [[RtNpcGrp_3]]";
	std::string text = with("			NpcGrps = {\n				{ Class = [[RtNpcGrp]], Id = [[RtNpcGrp_2]]", inside);
	expectAccepted(text);
	std::string::size_type pos = text.find("Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },");
	ASSERT_NE(std::string::npos, pos);
	text.replace(pos, strlen("Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },"), "Name = [[Client1_20.init]], Children = { [[Hidden_1]] },");
	expectRefused(text, "'Hidden_1'");
}

TEST(RtScenarioCheck, DescriptionOfARefusalCanGoToTheLog)
{
	// text of the tree is shortened and loses its control characters
	std::string longId = "bad\nid\t" + repeat("x", 500);
	std::string text = with("Name = [[Client1_20.init]], Children = { [[RtNpcGrp_2]] },", "Name = [[Client1_20.init]], Children = { [[" + longId + "]] },");
	CObject::TSmartPtr tree = parse(text);
	ASSERT_TRUE(tree != nullptr);
	std::string error;
	EXPECT_EQ(RtScenarioInvalid, check(tree, error));
	EXPECT_LT(error.size(), (size_t)300) << error;
	EXPECT_NE(std::string::npos, error.find("'bad?id?xxx")) << error;
	for (size_t i = 0; i < error.size(); ++i)
		EXPECT_GE((uint32)(uint8)error[i], 0x20u) << "control character at " << i;
}

TEST(RtScenarioCheck, NumbersThatHaveNoMeaning)
{
	double nan = std::numeric_limits<double>::quiet_NaN();
	double inf = std::numeric_limits<double>::infinity();
	CObject::TSmartPtr tree = parse(s_Scenario);
	ASSERT_TRUE(tree != nullptr);
	CObject *act = tree->getAttr("Acts")->getValueAtPos(1);
	CObject *plotItem = tree->getAttr("PlotItems")->getValueAtPos(0);
	CObject *location = tree->getAttr("Locations")->getValueAtPos(1);
	CObject *action = act->getAttr("Actions")->getValueAtPos(0);
	std::string error;

	// not a number, where a number is needed
	act->set("LocationId", nan);
	EXPECT_EQ(RtScenarioInvalid, check(tree, error));
	EXPECT_NE(std::string::npos, error.find("Acts[1].LocationId")) << error;
	act->set("LocationId", 1.0);
	EXPECT_EQ(RtScenarioOk, check(tree, error)) << error;

	plotItem->set("SheetId", nan);
	EXPECT_EQ(RtScenarioInvalid, check(tree, error));
	EXPECT_NE(std::string::npos, error.find("PlotItems[0].SheetId")) << error;
	plotItem->set("SheetId", 4660.0);

	location->set("Season", nan);
	EXPECT_EQ(RtScenarioInvalid, check(tree, error));
	EXPECT_NE(std::string::npos, error.find("Locations[1].Season")) << error;
	location->set("Season", 4.0);

	action->set("Weight", nan);
	EXPECT_EQ(RtScenarioInvalid, check(tree, error));
	EXPECT_NE(std::string::npos, error.find("'RtNpcEventHandlerAction_4'.Weight")) << error;
	action->set("Weight", inf);
	EXPECT_EQ(RtScenarioInvalid, check(tree, error));
	action->set("Weight", 1.0);

	act->set("WeatherValue", nan);
	EXPECT_EQ(RtScenarioInvalid, check(tree, error));
	EXPECT_NE(std::string::npos, error.find("Acts[1].WeatherValue")) << error;
	act->set("WeatherValue", inf);
	EXPECT_EQ(RtScenarioInvalid, check(tree, error));
	act->set("WeatherValue", 512.0);
	EXPECT_EQ(RtScenarioOk, check(tree, error)) << error;

	// whole numbers no smaller type holds, kept as integers by the tree
	expectRefused(with("LocationId = 1,", "LocationId = 9007199254740993,"), "Acts[1].LocationId");
	expectRefused(with("LocationId = 1,", "LocationId = 9223372036854775807,"), "Acts[1].LocationId");
	expectRefused(with("LocationId = 1,", "LocationId = -9223372036854775808,"), "Acts[1].LocationId");
	expectRefused(with("LocationId = 1,", "LocationId = 18446744073709551617,"), "Acts[1].LocationId");
	expectRefused(with("SheetId = 4660,", "SheetId = 9223372036854775807,"), "PlotItems[0].SheetId");
	expectRefused(with("Season = 4 }", "Season = 9007199254740993 }"), "Locations[1].Season");
	expectRefused(with("Parameters = [[RtAiState_2]], Weight = 1,", "Parameters = [[RtAiState_2]], Weight = 9223372036854775807,"), "'RtNpcEventHandlerAction_4'.Weight");
	expectRefused(with("WeatherValue = 512,", "WeatherValue = 9007199254740993,"), "Acts[1].WeatherValue");
	expectRefused(with("Pt = { x = 30000, y = -2000, z = 0 }", "Pt = { x = 9223372036854775807, y = -2000, z = 0 }"), "'RtNpc_3'.Pt.x");
}

TEST(RtScenarioCheck, PositionsAndAnglesAreInARange)
{
	// by default, what the AI service can hold
	expectAccepted(with("Pt = { x = 30000, y = -2000, z = 0 }", "Pt = { x = 2147483, y = -2147483, z = 2147483 }"));
	expectRefused(with("Pt = { x = 30000, y = -2000, z = 0 }", "Pt = { x = 2147484, y = -2000, z = 0 }"), "'RtNpc_3'.Pt.x");
	expectRefused(with("Pt = { x = 30000, y = -2000, z = 0 }", "Pt = { x = 30000, y = -2147484, z = 0 }"), "'RtNpc_3'.Pt.y");
	expectRefused(with("Pt = { x = 30000, y = -2000, z = 0 }", "Pt = { x = 30000, y = -2000, z = 1e300 }"), "'RtNpc_3'.Pt.z");
	expectRefused(with("Pt = { x = 30000, y = -2000, z = 0 }", "Pt = { x = 1e300, y = -2000, z = 0 }"), "'RtNpc_3'.Pt.x");
	expectRefused(with("{ x = 21580, y = -1370, z = -21 }", "{ x = 21580, y = -1e30, z = -21 }"), "Acts[0].AiStates[0].Pts[1].y");
	expectRefused(with("{ x = 21580, y = -1370, z = -21 }", "{ x = 21580, y = -1370, z = -1e30 }"), "Acts[0].AiStates[0].Pts[1].z");

	// the rectangle the service gives
	CRtScenarioLimits limits;
	limits.MinX = -160;
	limits.MaxX = 30000;
	limits.MinY = -2000;
	limits.MaxY = 160;
	expectAccepted(s_Scenario, limits);
	expectAccepted(with("Pt = { x = 30000, y = -2000, z = 0 }", "Pt = { x = 0, y = 0, z = 0 }"), limits);
	limits.MaxX = 29999;
	expectRefused(s_Scenario, "'RtNpc_3'.Pt.x", RtScenarioInvalid, limits);
	limits.MaxX = 30000;
	limits.MinY = -1999;
	expectRefused(s_Scenario, "'RtNpc_3'.Pt.y", RtScenarioInvalid, limits);
	limits.MinY = -2000;
	limits.MinX = 21571;
	expectRefused(s_Scenario, "Acts[0].AiStates[0].Pts[0].x", RtScenarioInvalid, limits);

	// an angle is a few turns
	expectAccepted(with("Angle = 1.5,", "Angle = 65536,"));
	expectAccepted(with("Angle = 1.5,", "Angle = -65536,"));
	expectRefused(with("Angle = 1.5,", "Angle = 65537,"), "'RtNpc_1'.Angle");
	expectRefused(with("Angle = 1.5,", "Angle = -1e300,"), "'RtNpc_1'.Angle");
	expectAccepted(with("Angle = 1.5,", "Angle = [[north]],"));
}

TEST(RtScenarioCheck, TablesThatAreNotTables)
{
	std::string texts = "	Texts = { Class = [[RtTextManager]], Id = [[RtTextManager_1]], Texts = {\n"
	                    "		{ Class = [[RtEntryText]], Id = [[RtEntryText_1]], Text = [[Welcome]] },\n"
	                    "	} },\n";
	expectRefused(with(texts, "	Texts = 5,\n"), "Texts is the number 5");
	expectRefused(with(texts, "	Texts = [[Welcome]],\n"), "Texts is the text 'Welcome'");
	expectRefused(with("		{ Class = [[RtEntryText]], Id = [[RtEntryText_1]], Text = [[Welcome]] },\n", "		[[Welcome]],\n"), "Texts.Texts[0] is the text 'Welcome'");
	expectRefused(with("		{ Class = [[RtEntryText]], Id = [[RtEntryText_1]], Text = [[Welcome]] },\n", "		7,\n"), "Texts.Texts[0] is the number 7");

	expectRefused(with("			UserTriggers = { },\n", "			UserTriggers = 1,\n"), "Acts[0].UserTriggers is the number 1");
	expectRefused(with("			UserTriggers = { },\n", "			UserTriggers = [[none]],\n"), "Acts[0].UserTriggers is the text 'none'");
	std::string userTrigger = "				{ Class = [[RtUserTrigger]], Id = [[RtUserTrigger_1]], Name = [[Open the gate]], Grp = [[RtNpcGrp_2]], TriggerId = 1 },\n";
	expectRefused(with(userTrigger, "				[[RtNpcGrp_2]],\n"), "Acts[1].UserTriggers[0] is the text 'RtNpcGrp_2'");
	expectRefused(with(userTrigger, "				2,\n"), "Acts[1].UserTriggers[0] is the number 2");

	// the act to start in, in a tree whose acts are not a table
	std::string error;
	CObject::TSmartPtr noActs = parse("scenario = { Acts = 5 }");
	ASSERT_TRUE(noActs != nullptr);
	EXPECT_EQ(RtScenarioInvalid, checkRtScenarioStartingAct(noActs, 1, error));
	EXPECT_FALSE(error.empty());
	CObject::TSmartPtr textActs = parse("scenario = { Acts = [[two]] }");
	ASSERT_TRUE(textActs != nullptr);
	EXPECT_EQ(RtScenarioInvalid, checkRtScenarioStartingAct(textActs, 1, error));
}

TEST(RtScenarioCheck, LimitsDerivedFromTheQuota)
{
	// what the translator makes for one entity that uses everything the editor shows for it
	EXPECT_EQ(210u, (uint32)CRtScenarioLimits::StatesPerEntity);
	EXPECT_EQ(612u, (uint32)CRtScenarioLimits::EventsPerEntity);
	EXPECT_EQ(813u, (uint32)CRtScenarioLimits::ActivityActionsPerEntity);
	EXPECT_EQ(506u, (uint32)CRtScenarioLimits::DialogActionsPerEntity);
	EXPECT_EQ(98u, (uint32)CRtScenarioLimits::TextsPerEntity);

	CRtScenarioLimits limits;
	EXPECT_EQ(205u, limits.MaxUserTriggersPerAct);
	EXPECT_EQ(86100u, limits.MaxStatesPerAct);
	EXPECT_EQ(820u, limits.MaxGroupsPerAct);
	EXPECT_EQ(250920u, limits.MaxEventsPerAct);
	EXPECT_EQ(540790u, limits.MaxActionsPerAct);
	EXPECT_EQ(441980u, limits.MaxTexts);
	limits.setBotQuota(10);
	EXPECT_EQ(10u, limits.MaxBotsPerAct);
	EXPECT_EQ(10u, limits.MaxUserTriggersPerAct);
	EXPECT_EQ(4200u, limits.MaxStatesPerAct);
	EXPECT_EQ(40u, limits.MaxGroupsPerAct);
	EXPECT_EQ(12240u, limits.MaxEventsPerAct);
	EXPECT_EQ(26380u, limits.MaxActionsPerAct);
	EXPECT_EQ(21560u, limits.MaxTexts);
	// a quota no product of which fits stays at the largest value
	limits.setBotQuota(0xFFFFFFFFu);
	EXPECT_EQ(0xFFFFFFFFu, limits.MaxStatesPerAct);
	EXPECT_EQ(0xFFFFFFFFu, limits.MaxGroupsPerAct);
	EXPECT_EQ(0xFFFFFFFFu, limits.MaxEventsPerAct);
	EXPECT_EQ(0xFFFFFFFFu, limits.MaxActionsPerAct);
	EXPECT_EQ(0xFFFFFFFFu, limits.MaxTexts);

	// The example of an act that is within the quota: 205 creatures with two sequences of twelve
	// Rest In Zone steps make 205 * (2 * (3 + 12 * 4) + 1) events.
	limits = CRtScenarioLimits();
	EXPECT_LT(205u * (2 * (3 + 12 * 4) + 1), limits.MaxEventsPerAct);

	// user triggers of an act
	std::string userTrigger = "				{ Class = [[RtUserTrigger]], Id = [[RtUserTrigger_1]], Name = [[Open the gate]], Grp = [[RtNpcGrp_2]], TriggerId = 1 },\n";
	std::string triggers = with(userTrigger, repeat("				{ Name = [[t]], Grp = [[RtNpcGrp_2]] },\n", 3));
	limits = CRtScenarioLimits();
	limits.MaxUserTriggersPerAct = 3;
	expectAccepted(triggers, limits);
	limits.MaxUserTriggersPerAct = 2;
	expectRefused(triggers, "Acts[1] has 3 user triggers", RtScenarioOverQuota, limits);

	// states of an act, and groups its states name: each act of the sample has one of each
	limits = CRtScenarioLimits();
	limits.MaxStatesPerAct = 1;
	limits.MaxGroupsPerAct = 1;
	expectAccepted(s_Scenario, limits);
	limits.MaxStatesPerAct = 0;
	expectRefused(s_Scenario, "Acts[0] has 1 states", RtScenarioOverQuota, limits);
	limits.MaxStatesPerAct = 1;
	limits.MaxGroupsPerAct = 0;
	expectRefused(s_Scenario, "Acts[0] has more than 0 groups", RtScenarioOverQuota, limits);
	std::string states = with("			AiStates = {\n				{ Class = [[RtAiState]], Id = [[RtAiState_2]]", "			AiStates = { { AiMovement = [[]], Children = { } },\n				{ Class = [[RtAiState]], Id = [[RtAiState_2]]");
	limits.MaxGroupsPerAct = 1;
	limits.MaxStatesPerAct = 2;
	expectAccepted(states, limits);
	limits.MaxStatesPerAct = 1;
	expectRefused(states, "Acts[1] has 2 states", RtScenarioOverQuota, limits);

	// events of an act
	std::string anchor = "			Events = {\n				{ Class = [[RtNpcEventHandler]], Id = [[RtNpcEventHandler_2]]";
	std::string events = with(anchor, "			Events = { { }, { },\n				{ Class = [[RtNpcEventHandler]], Id = [[RtNpcEventHandler_2]]");
	limits = CRtScenarioLimits();
	limits.MaxEventsPerAct = 3;
	expectAccepted(events, limits);
	limits.MaxEventsPerAct = 2;
	expectRefused(events, "Acts[1] has 3 events", RtScenarioOverQuota, limits);

	// Actions of an act: the base act has one event that starts three actions, one below the other,
	// and the act one event that starts one. The limit is for each act.
	limits = CRtScenarioLimits();
	limits.MaxActionsPerAct = 3;
	expectAccepted(s_Scenario, limits);
	limits.MaxActionsPerAct = 2;
	expectRefused(s_Scenario, "Acts[0] has more than 2 actions", RtScenarioOverQuota, limits);
	std::string action = "				{ Id = [[RtNpcEventHandlerAction_5]], Action = [[multi_actions]], Children = { { Action = [[code]], Children = { } }, { Action = [[code]], Children = { } } } },\n"
	                     "				{ Class = [[RtNpcEventHandlerAction]], Id = [[RtNpcEventHandlerAction_4]]";
	std::string more = with("				{ Class = [[RtNpcEventHandlerAction]], Id = [[RtNpcEventHandlerAction_4]]", action);
	std::string::size_type pos = more.find("ActionsId = { [[RtNpcEventHandlerAction_4]] }");
	ASSERT_NE(std::string::npos, pos);
	more.replace(pos, strlen("ActionsId = { [[RtNpcEventHandlerAction_4]] }"), "ActionsId = { [[RtNpcEventHandlerAction_4]], [[RtNpcEventHandlerAction_5]] }");
	limits.MaxActionsPerAct = 4;
	expectAccepted(more, limits);
	limits.MaxActionsPerAct = 3;
	expectRefused(more, "Acts[1] has more than 3 actions", RtScenarioOverQuota, limits);

	// texts of the scenario
	std::string entry = "		{ Class = [[RtEntryText]], Id = [[RtEntryText_1]], Text = [[Welcome]] },\n";
	std::string moreTexts = with(entry, repeat("		{ Id = [[t]], Text = [[Welcome]] },\n", 4));
	limits = CRtScenarioLimits();
	limits.MaxTexts = 4;
	expectAccepted(moreTexts, limits);
	limits.MaxTexts = 3;
	expectRefused(moreTexts, "Texts.Texts has 4 texts", RtScenarioOverQuota, limits);
}

TEST(RtScenarioCheck, WhatIsMadeIsCountedOverAllTheActs)
{
	// An event that holds nothing is one value of the tree and two nodes of what is made. With a
	// thousand of them the base act makes 2010 nodes; the act makes 7: a state, a group with its
	// parameters, a bot, an event with the action that holds its actions, and one action.
	const uint32 events = 1000;
	std::string anchor = "			Events = {\n				{ Class = [[RtNpcEventHandler]], Id = [[RtNpcEventHandler_1]]";
	std::string text = with(anchor, "			Events = { " + repeat("{ }, ", events) + "\n				{ Class = [[RtNpcEventHandler]], Id = [[RtNpcEventHandler_1]]");
	const uint32 baseAct = 10 + 2 * events;

	CRtScenarioLimits limits;
	limits.MaxNodes = baseAct + 7;
	expectAccepted(text, limits);
	// one short: the action of the act
	limits.MaxNodes = baseAct + 6;
	expectRefused(text, "Acts[1].Events[0] makes the translated scenario larger", RtScenarioInvalid, limits);
	// the bot of the group of the act
	limits.MaxNodes = baseAct + 3;
	expectRefused(text, "Acts[1].AiStates[0]: the group 'RtNpcGrp_2' makes the translated scenario larger", RtScenarioInvalid, limits);
	// the group
	limits.MaxNodes = baseAct + 2;
	expectRefused(text, "Acts[1].AiStates[0]: the group 'RtNpcGrp_2' makes the translated scenario larger", RtScenarioInvalid, limits);
	// the state
	limits.MaxNodes = baseAct;
	expectRefused(text, "Acts[1].AiStates[0] makes the translated scenario larger", RtScenarioInvalid, limits);

	// the aliases of an act with more groups and bots: the base act of the sample needs 7, and
	// one more for each group and each bot
	std::string groups = "				{ Id = [[RtNpcGrp_8]], Children = { [[RtNpc_8]] } }, { Id = [[RtNpcGrp_9]], Children = { } },\n"
	                     "				{ Id = [[RtNpc_8]], Name = [[]], ChatParameters = [[]], Keywords = [[]], Sheet = [[]], SheetClient = [[]] },\n"
	                     "			},\n"
	                     "			AiStates = {\n"
	                     "				{ Class = [[RtAiState]], Id = [[RtAiState_1]]";
	std::string more = with("			},\n			AiStates = {\n				{ Class = [[RtAiState]], Id = [[RtAiState_1]]", groups);
	std::string::size_type pos = more.find("Name = [[Client1_10.init]], Children = { [[RtNpcGrp_1]] },");
	ASSERT_NE(std::string::npos, pos);
	more.replace(pos, strlen("Name = [[Client1_10.init]], Children = { [[RtNpcGrp_1]] },"), "Name = [[Client1_10.init]], Children = { [[RtNpcGrp_1]], [[RtNpcGrp_8]], [[RtNpcGrp_9]] },");
	limits = CRtScenarioLimits();
	limits.MaxAliasesPerAct = 10;
	expectAccepted(more, limits);
	limits.MaxAliasesPerAct = 9;
	expectRefused(more, "Acts[0] needs 10 aliases", RtScenarioInvalid, limits);
}

TEST(RtScenarioCheck, AnActOfEntitiesThatUseAllTheEditorShows)
{
	// One act with as many entities as the quota allows, each with what the translator makes for an
	// entity whose activities and dialogs are full: 210 states, 612 events and 1319 actions, and
	// 98 texts. The limits of an act are twice that, so the act passes. The tables are the smallest
	// that pass, and still more than a million values: a real tree of this act is larger, and is
	// refused for its size before any of these limits is reached, so the size is not limited here.
	CRtScenarioLimits limits;
	limits.MaxNodes = 0xFFFFFFFFu;
	const uint32 entities = limits.MaxBotsPerAct;
	const uint32 actionsPerEntity = CRtScenarioLimits::ActivityActionsPerEntity + CRtScenarioLimits::DialogActionsPerEntity;

	CObject *acts = nullptr;
	CObject::TSmartPtr tree = newScenario(acts, 2);
	CObject *act = getAct(acts, 1);
	CObject *states = act->getAttr("AiStates");
	CObject *events = act->getAttr("Events");
	CObjectTable *groups = addTable(tree, "Groups");
	CObjectTable *actions = addTable(tree, "Actions");
	CObjectTable *texts = addTable(addTable(tree, "Texts"), "Texts");
	uint32 actionId = 0;
	for (uint32 e = 0; e < entities; ++e)
	{
		// its group, with the entity in it, in the first of its states
		std::string groupId = NLMISC::toString("RtNpcGrp_%u", e);
		std::string npcId = NLMISC::toString("RtNpc_%u", e);
		addNpc(act->getAttr("Npcs"), npcId);
		CObjectTable *group = addTable(groups, "");
		group->add("Id", groupId);
		addTable(group, "Children")->add("", npcId);
		for (uint32 i = 0; i < CRtScenarioLimits::StatesPerEntity; ++i)
		{
			CObjectTable *state = addState(states);
			if (i == 0)
				state->getAttr("Children")->add("", groupId);
		}

		// its events: each starts one action, and the last one the rest of the actions, below it
		for (uint32 i = 0; i < CRtScenarioLimits::EventsPerEntity; ++i, ++actionId)
		{
			std::string id = NLMISC::toString("RtNpcEventHandlerAction_%u", actionId);
			CObjectTable *action = addAction(actions, id);
			addTable(addTable(events, ""), "ActionsId")->add("", id);
			if (i + 1 == CRtScenarioLimits::EventsPerEntity)
			{
				CObject *below = action->getAttr("Children");
				for (uint32 a = CRtScenarioLimits::EventsPerEntity; a < actionsPerEntity; ++a)
					addAction(below);
			}
		}

		for (uint32 i = 0; i < CRtScenarioLimits::TextsPerEntity; ++i)
		{
			CObjectTable *entry = addTable(texts, "");
			entry->add("Id", std::string("RtEntryText_1"));
			entry->add("Text", std::string("Welcome"));
		}
	}
	expectCheckedInTime("an act of 205 entities with 210 states, 612 events and 1319 actions each", tree, RtScenarioOk, limits);

	// with the size of a tree as it is, the tree is refused for its size
	expectCheckedInTime("the same act with the limit of the size of a tree", tree, RtScenarioInvalid);
}

TEST(RtScenarioCheck, AnActAtItsLimitsAndOneOver)
{
	// an act with exactly as many states, groups, user triggers and events as an act can have
	CRtScenarioLimits limits;
	CObject *acts = nullptr;
	CObject::TSmartPtr tree = newScenario(acts, 2);
	CObject *act = getAct(acts, 1);
	CObject *states = act->getAttr("AiStates");
	CObject *events = act->getAttr("Events");
	CObject *userTriggers = act->getAttr("UserTriggers");
	CObjectTable *groups = addTable(tree, "Groups");
	for (uint32 i = 0; i < limits.MaxStatesPerAct; ++i)
	{
		CObjectTable *state = addState(states);
		if (i < limits.MaxGroupsPerAct)
		{
			std::string groupId = NLMISC::toString("RtNpcGrp_%u", i);
			CObjectTable *group = addTable(groups, "");
			group->add("Id", groupId);
			addTable(group, "Children");
			state->getAttr("Children")->add("", groupId);
		}
	}
	for (uint32 i = 0; i < limits.MaxUserTriggersPerAct; ++i)
	{
		CObjectTable *userTrigger = addTable(userTriggers, "");
		userTrigger->add("Name", std::string("trigger"));
		userTrigger->add("Grp", std::string("RtNpcGrp_0"));
	}
	for (uint32 i = 0; i < limits.MaxEventsPerAct; ++i)
		addTable(events, "");
	expectCheckedInTime("an act of 86100 states, 820 groups, 205 user triggers and 250920 events", tree, RtScenarioOk);

	// one more of each, one at a time
	std::string error;
	CObjectTable *state = addState(states);
	EXPECT_EQ(RtScenarioOverQuota, check(tree, error));
	EXPECT_NE(std::string::npos, error.find("Acts[1] has 86101 states")) << error;
	states->take((sint32)states->getSize() - 1);
	EXPECT_EQ(RtScenarioOk, check(tree, error)) << error;

	CObjectTable *group = addTable(groups, "");
	group->add("Id", std::string("RtNpcGrp_one_more"));
	addTable(group, "Children");
	state = static_cast<CObjectTable *>(states->getValueAtPos(limits.MaxGroupsPerAct).getPtr());
	state->getAttr("Children")->add("", std::string("RtNpcGrp_one_more"));
	EXPECT_EQ(RtScenarioOverQuota, check(tree, error));
	EXPECT_NE(std::string::npos, error.find("Acts[1] has more than 820 groups")) << error;
	state->getAttr("Children")->take(0);
	EXPECT_EQ(RtScenarioOk, check(tree, error)) << error;

	addTable(userTriggers, "");
	EXPECT_EQ(RtScenarioOverQuota, check(tree, error));
	EXPECT_NE(std::string::npos, error.find("Acts[1] has 206 user triggers")) << error;
	userTriggers->take((sint32)userTriggers->getSize() - 1);
	EXPECT_EQ(RtScenarioOk, check(tree, error)) << error;

	addTable(events, "");
	EXPECT_EQ(RtScenarioOverQuota, check(tree, error));
	EXPECT_NE(std::string::npos, error.find("Acts[1] has 250921 events")) << error;

	// The actions of an act and the texts of a scenario, with the limits of a quota of 20, so that a
	// tree that has as many is a small one: 52760 actions and 43120 texts.
	limits.setBotQuota(20);
	CObject *acts2 = nullptr;
	CObject::TSmartPtr tree2 = newScenario(acts2, 2);
	CObjectTable *actions = addTable(tree2, "Actions");
	CObjectTable *action = addAction(actions, "RtNpcEventHandlerAction_1");
	CObject *below = action->getAttr("Children");
	for (uint32 i = 1; i < limits.MaxActionsPerAct; ++i)
		addAction(below);
	addTable(addTable(getAct(acts2, 1)->getAttr("Events"), ""), "ActionsId")->add("", std::string("RtNpcEventHandlerAction_1"));
	CObjectTable *texts = addTable(addTable(tree2, "Texts"), "Texts");
	for (uint32 i = 0; i < limits.MaxTexts; ++i)
	{
		CObjectTable *entry = addTable(texts, "");
		entry->add("Id", std::string("RtEntryText_1"));
		entry->add("Text", std::string("Welcome"));
	}
	expectCheckedInTime("an act of 52760 actions and a scenario of 43120 texts", tree2, RtScenarioOk, limits);

	addAction(below);
	EXPECT_EQ(RtScenarioOverQuota, check(tree2, error, limits));
	EXPECT_NE(std::string::npos, error.find("Acts[1] has more than 52760 actions")) << error;
	below->take((sint32)below->getSize() - 1);
	EXPECT_EQ(RtScenarioOk, check(tree2, error, limits)) << error;

	CObjectTable *entry = addTable(texts, "");
	entry->add("Id", std::string("RtEntryText_1"));
	entry->add("Text", std::string("Welcome"));
	EXPECT_EQ(RtScenarioOverQuota, check(tree2, error, limits));
	EXPECT_NE(std::string::npos, error.find("Texts.Texts has 43121 texts")) << error;
}

// The largest trees the limits allow, one for each way a tree can be large, and trees that name one
// thing many times. The time a check takes grows with the size of the tree, and each of these is
// checked in a fraction of a second; the bound that is tested is s_MaxCheckSeconds.

TEST(RtScenarioCheckTime, AsManyValuesAsATreeCanHave)
{
	CRtScenarioLimits limits;
	CObject *acts = nullptr;
	CObject::TSmartPtr tree = newScenario(acts);
	CObjectTable *unread = addTable(tree, "Unread");
	for (uint32 i = 0; i < limits.MaxNodes - 200; ++i)
		unread->add("", (sint64)i);
	expectCheckedInTime("a million values", tree, RtScenarioOk);

	// one value over the limit is refused when it is reached
	for (uint32 i = 0; i < 200; ++i)
		unread->add("", (sint64)i);
	expectCheckedInTime("a million values and more", tree, RtScenarioInvalid);
}

TEST(RtScenarioCheckTime, AsManyTablesWithAnIdAsATreeCanHave)
{
	CRtScenarioLimits limits;
	CObject *acts = nullptr;
	CObject::TSmartPtr tree = newScenario(acts);
	CObjectTable *unread = addTable(tree, "Unread");
	for (uint32 i = 0; i < (limits.MaxNodes - 200) / 2; ++i)
		addTable(unread, "")->add("Id", NLMISC::toString("Unread_%u", i));
	expectCheckedInTime("half a million tables with an id", tree, RtScenarioOk);
}

TEST(RtScenarioCheckTime, IdsThatAreChosen)
{
	// The time it takes to find an id does not depend on the ids. An earlier index of the ids was
	// searched from the place a hash gave (FNV-1a, in a table of a power of two of places): ids of
	// the ordinary form whose hash has the bits 12 to 19 clear all came to the same places, and a
	// tree of them took as long as the square of their number. 100000 of them are actions here, each
	// named by an event, with 200000 other ids around them.
	CObject *acts = nullptr;
	CObject::TSmartPtr tree = newScenario(acts, 2);
	CObjectTable *actions = addTable(tree, "Actions");
	CObjectTable *actionIds = addTable(addTable(getAct(acts, 1)->getAttr("Events"), ""), "ActionsId");
	const uint32 chosen = 100000;
	char id[32] = "RtNpc_0";
	const size_t prefix = 6;
	size_t digits = 1;
	uint32 found = 0;
	for (uint32 n = 0; found < chosen && n < 0x7FFFFFFFu; ++n)
	{
		if ((fnv1a(id, prefix + digits) & 0x000FF000u) == 0)
		{
			std::string text(id, prefix + digits);
			addAction(actions, text);
			actionIds->add("", text);
			++found;
		}

		// the next number, in place
		size_t at = prefix + digits;
		while (at > prefix && id[at - 1] == '9')
		{
			id[at - 1] = '0';
			--at;
		}
		if (at > prefix)
			++id[at - 1];
		else
		{
			memmove(id + prefix + 1, id + prefix, digits);
			id[prefix] = '1';
			++digits;
		}
	}
	ASSERT_EQ(chosen, found);

	CObjectTable *unread = addTable(tree, "Unread");
	for (uint32 i = 0; i < 200000; ++i)
		addTable(unread, "")->add("Id", NLMISC::toString("Unread_%u", i));
	expectCheckedInTime("100000 chosen ids that are named, among 300000", tree, RtScenarioOk);
}

TEST(RtScenarioCheckTime, AsManyStatesAndGroupsAsATreeCanHave)
{
	// every act with as many groups as it can have, each in a state, and as many more states as
	// the tree has room for: a state that names no group is three values
	CRtScenarioLimits limits;
	CObject *acts = nullptr;
	CObject::TSmartPtr tree = newScenario(acts);
	CObjectTable *groups = addTable(tree, "Groups");
	uint32 perAct = (limits.MaxNodes - 400) / limits.MaxActs / 3 - limits.MaxGroupsPerAct * 4 / 3 - 1;
	uint32 id = 0;
	uint32 count = 0;
	for (uint32 a = 0; a < limits.MaxActs; ++a)
	{
		CObject *states = getAct(acts, a)->getAttr("AiStates");
		for (uint32 i = 0; i < perAct; ++i, ++count)
		{
			CObjectTable *state = addState(states);
			if (i < limits.MaxGroupsPerAct)
			{
				std::string groupId = NLMISC::toString("RtNpcGrp_%u", id++);
				CObjectTable *group = addTable(groups, "");
				group->add("Id", groupId);
				addTable(group, "Children");
				state->getAttr("Children")->add("", groupId);
			}
		}
	}
	expectCheckedInTime(NLMISC::toString("%u states and %u groups in 11 acts", count, id).c_str(), tree, RtScenarioOk);
}

TEST(RtScenarioCheckTime, AsManyBotsUserTriggersAndEventsAsTheActsCanHave)
{
	// Every act with the bots of the quota, shared with the base act, its user triggers, and 45000
	// events: an event that holds nothing is two nodes of what is made, which has room for no more
	// in 11 acts.
	CRtScenarioLimits limits;
	CObject *acts = nullptr;
	CObject::TSmartPtr tree = newScenario(acts);
	uint32 id = 0;
	for (uint32 a = 0; a < limits.MaxActs; ++a)
	{
		CObject *act = getAct(acts, a);
		uint32 bots = a == 0 ? 5 : limits.MaxBotsPerAct - 5;
		CObjectTable *group = addTable(act, "Group");
		group->add("Id", NLMISC::toString("RtNpcGrp_%u", a));
		CObjectTable *children = addTable(group, "Children");
		CObject *npcs = act->getAttr("Npcs");
		for (uint32 i = 0; i < bots; ++i, ++id)
		{
			std::string npcId = NLMISC::toString("RtNpc_%u", id);
			CObjectTable *npc = addNpc(npcs, npcId);
			npc->add("Angle", 1.5);
			CObjectTable *point = addTable(npc, "Pt");
			point->add("x", 21570.5);
			point->add("y", -1363.25);
			point->add("z", -20.0);
			children->add("", npcId);
		}
		addState(act->getAttr("AiStates"))->getAttr("Children")->add("", NLMISC::toString("RtNpcGrp_%u", a));

		CObject *userTriggers = act->getAttr("UserTriggers");
		for (uint32 i = 0; i < limits.MaxUserTriggersPerAct; ++i)
		{
			CObjectTable *userTrigger = addTable(userTriggers, "");
			userTrigger->add("Name", std::string("trigger"));
			userTrigger->add("Grp", NLMISC::toString("RtNpcGrp_%u", a));
		}

		CObject *events = act->getAttr("Events");
		for (uint32 i = 0; i < 45000; ++i)
			addTable(events, "");
	}
	expectCheckedInTime("11 acts with 205 bots, 205 user triggers and 45000 events each", tree, RtScenarioOk);
}

TEST(RtScenarioCheckTime, AsManyActionsAsATreeCanHave)
{
	// every act with an event that starts 31001 actions: an action below another one is three
	// values, and the tree has room for no more
	CRtScenarioLimits limits;
	CObject *acts = nullptr;
	CObject::TSmartPtr tree = newScenario(acts);
	CObjectTable *actions = addTable(tree, "Actions");
	for (uint32 a = 0; a < limits.MaxActs; ++a)
	{
		std::string id = NLMISC::toString("RtNpcEventHandlerAction_%u", a);
		CObject *below = addAction(actions, id)->getAttr("Children");
		for (uint32 i = 0; i < 31000; ++i)
			addAction(below);
		addTable(addTable(getAct(acts, a)->getAttr("Events"), ""), "ActionsId")->add("", id);
	}
	expectCheckedInTime("11 acts that start 31001 actions each", tree, RtScenarioOk);
}

TEST(RtScenarioCheckTime, AsManyTextsAsATreeCanHave)
{
	// a text is three values
	CRtScenarioLimits limits;
	CObject *acts = nullptr;
	CObject::TSmartPtr tree = newScenario(acts);
	CObjectTable *texts = addTable(addTable(tree, "Texts"), "Texts");
	for (uint32 i = 0; i < (limits.MaxNodes - 400) / 3; ++i)
	{
		CObjectTable *entry = addTable(texts, "");
		entry->add("Id", std::string("RtEntryText_1"));
		entry->add("Text", std::string("Welcome"));
	}
	expectCheckedInTime("349392 texts", tree, RtScenarioOk);
}

TEST(RtScenarioCheckTime, TablesWithManyAttributes)
{
	// a group, an npc and an action with 300000 attributes each in front of the ones that are read:
	// each is gone through a few times
	CObject *acts = nullptr;
	CObject::TSmartPtr tree = newScenario(acts, 2);
	CObject *act = getAct(acts, 1);
	const uint32 width = 300000;

	CObjectTable *npc = addWideTable(act->getAttr("Npcs"), "RtNpc_1", width);
	npc->add("Name", std::string("npc"));
	npc->add("ChatParameters", std::string());
	npc->add("Keywords", std::string());
	npc->add("SheetClient", std::string("chdfa1.creature"));
	npc->add("Sheet", std::string());

	CObjectTable *group = addWideTable(tree, "RtNpcGrp_1", width);
	addTable(group, "Children")->add("", std::string("RtNpc_1"));
	addState(act->getAttr("AiStates"))->getAttr("Children")->add("", std::string("RtNpcGrp_1"));

	CObjectTable *action = addWideTable(tree, "RtNpcEventHandlerAction_1", width);
	action->add("Action", std::string("code"));
	addTable(action, "Children");
	addTable(addTable(act->getAttr("Events"), ""), "ActionsId")->add("", std::string("RtNpcEventHandlerAction_1"));
	expectCheckedInTime("three tables of 300000 attributes", tree, RtScenarioOk);
}

TEST(RtScenarioCheckTime, AnIdThatIsNamedManyTimes)
{
	// a group with 20000 attributes that a state names 20000 times, the same for an npc and for an
	// action: the second naming is refused, so the table is gone through once
	const uint32 width = 20000;
	const uint32 times = 20000;
	{
		CObject *acts = nullptr;
		CObject::TSmartPtr tree = newScenario(acts, 2);
		CObjectTable *group = addWideTable(tree, "RtNpcGrp_1", width);
		addTable(group, "Children");
		CObject *children = addState(getAct(acts, 1)->getAttr("AiStates"))->getAttr("Children");
		for (uint32 i = 0; i < times; ++i)
			children->add("", std::string("RtNpcGrp_1"));
		expectCheckedInTime("a group of 20000 attributes named 20000 times", tree, RtScenarioInvalid);
	}
	{
		CObject *acts = nullptr;
		CObject::TSmartPtr tree = newScenario(acts, 2);
		CObjectTable *npc = addWideTable(tree, "RtNpc_1", width);
		npc->add("Name", std::string("npc"));
		npc->add("ChatParameters", std::string());
		npc->add("Keywords", std::string());
		npc->add("SheetClient", std::string("chdfa1.creature"));
		npc->add("Sheet", std::string());
		CObjectTable *group = addTable(tree, "Group");
		group->add("Id", std::string("RtNpcGrp_1"));
		CObjectTable *children = addTable(group, "Children");
		for (uint32 i = 0; i < times; ++i)
			children->add("", std::string("RtNpc_1"));
		addState(getAct(acts, 1)->getAttr("AiStates"))->getAttr("Children")->add("", std::string("RtNpcGrp_1"));
		expectCheckedInTime("an npc of 20000 attributes named 20000 times", tree, RtScenarioInvalid);
	}
	{
		// by one event, and by as many events
		CObject *acts = nullptr;
		CObject::TSmartPtr tree = newScenario(acts, 2);
		CObjectTable *action = addWideTable(tree, "RtNpcEventHandlerAction_1", width);
		action->add("Action", std::string("code"));
		addTable(action, "Children");
		CObject *events = getAct(acts, 1)->getAttr("Events");
		CObjectTable *actionIds = addTable(addTable(events, ""), "ActionsId");
		for (uint32 i = 0; i < times; ++i)
			actionIds->add("", std::string("RtNpcEventHandlerAction_1"));
		expectCheckedInTime("an action of 20000 attributes named 20000 times by an event", tree, RtScenarioInvalid);

		events->take(0);
		for (uint32 i = 0; i < times; ++i)
			addTable(addTable(events, ""), "ActionsId")->add("", std::string("RtNpcEventHandlerAction_1"));
		expectCheckedInTime("an action of 20000 attributes named by 20000 events", tree, RtScenarioInvalid);
	}
}

TEST(RtScenarioCheckTime, NestedActions)
{
	// 600 events that each start a chain of 120 actions, one below the other
	CObject *acts = nullptr;
	CObject::TSmartPtr tree = newScenario(acts, 2);
	CObject *events = getAct(acts, 1)->getAttr("Events");
	CObjectTable *actions = addTable(tree, "Actions");
	const uint32 chains = 600;
	const uint32 depth = 120;
	uint32 id = 0;
	for (uint32 c = 0; c < chains; ++c)
	{
		CObject *parent = actions;
		for (uint32 d = 0; d < depth; ++d, ++id)
		{
			std::string actionId = NLMISC::toString("RtNpcEventHandlerAction_%u", id);
			parent = addAction(parent, actionId)->getAttr("Children");
			if (d == 0)
				addTable(addTable(events, ""), "ActionsId")->add("", actionId);
		}
	}
	expectCheckedInTime("600 chains of 120 nested actions", tree, RtScenarioOk);

	// an event that names an action of the middle of a chain as well is refused there
	addTable(addTable(events, ""), "ActionsId")->add("", std::string("RtNpcEventHandlerAction_60"));
	expectCheckedInTime("an action of a chain that an event names too", tree, RtScenarioInvalid);
}

namespace {

/// The text with every occurrence of a part of it replaced
std::string replaceAll(std::string text, const std::string &what, const std::string &replacement)
{
	std::string::size_type pos = 0;
	uint32 count = 0;
	while ((pos = text.find(what, pos)) != std::string::npos)
	{
		text.replace(pos, what.size(), replacement);
		pos += replacement.size();
		++count;
	}
	EXPECT_NE(0u, count) << "not in the scenario: " << what;
	return text;
}

/// The scenario with the action of its act, a begin_state, made a script
std::string withScript(const std::string &action, const std::string &script)
{
	return replaceAll(with("Action = [[begin_state]]", "Action = [[" + action + "]]"), "Parameters = [[RtAiState_2]]", "Parameters = [[" + script + "]]");
}

void expectScriptAccepted(const std::string &script, bool isCondition = false)
{
	std::string function;
	EXPECT_TRUE(checkRtScenarioScript(script, isCondition, function)) << "refused for '" << function << "': " << script;
}

/// The script is refused, and the refusal names this (a function, a keyword) between quotes
void expectScriptRefused(const std::string &script, const std::string &name, bool isCondition = false)
{
	std::string refusal;
	EXPECT_FALSE(checkRtScenarioScript(script, isCondition, refusal)) << "accepted: " << script;
	EXPECT_NE(std::string::npos, refusal.find("'" + name + "'")) << refusal << "\nfor: " << script;
}

/// The script is refused, and the refusal has this text
void expectScriptRefusedFor(const std::string &script, const std::string &text, bool isCondition = false)
{
	std::string refusal;
	EXPECT_FALSE(checkRtScenarioScript(script, isCondition, refusal)) << "accepted, a script of " << script.size() << " bytes";
	EXPECT_NE(std::string::npos, refusal.find(text)) << refusal;
}

/// The beginning of a statement, this many tokens of it: "v0 = 1 + 1 + 1 ..."
std::string tokensOf(uint32 tokens)
{
	std::string statement = "v0 =";
	for (uint32 i = 2; i < tokens; ++i)
		statement += (i % 2 == 0) ? " 1" : " +";
	return statement;
}

} // namespace

TEST(RtScenarioCheck, IdsAreNamesTheServiceMakesNamesOf)
{
	const char *rule = "an id has letters, digits and '_'";

	// every reference to the id is changed with it: the id is what is refused
	expectAccepted(replaceAll(s_Scenario, "RtNpcGrp_1", "Group_1_of_the_Base_Act"));
	expectAccepted(replaceAll(s_Scenario, "RtNpcGrp_1", std::string(RtScenarioMaxIdSize, 'g')));
	expectRefused(replaceAll(s_Scenario, "RtNpcGrp_1", std::string(RtScenarioMaxIdSize + 1, 'g')), rule);

	const char *const groupIds[] = { "RtNpcGrp_1;x", "RtNpcGrp 1", "RtNpcGrp_1.x", "RtNpcGrp_1:x", "RtNpcGrp_*", "RtNpcGrp_?", "RtNpcGrp\\\"1", "1002_RtNpcGrp-1", "RtNpcGrp_1\xc3\xa9" };
	for (uint32 i = 0; i < sizeof(groupIds) / sizeof(groupIds[0]); ++i)
	{
		expectRefused(replaceAll(s_Scenario, "RtNpcGrp_1", groupIds[i]), "Acts[0].AiStates[0].Children[0] is the id");
		expectRefused(replaceAll(s_Scenario, "RtNpcGrp_1", groupIds[i]), rule);
	}

	// an npc, a state and an event
	expectRefused(replaceAll(s_Scenario, "RtNpc_3", "RtNpc_3 x"), "Acts[1].AiStates[0]: the group 'RtNpcGrp_2'.Children[0] is the id");
	expectRefused(with("Id = [[RtAiState_1]], Name", "Id = [[RtAiState_1;x]], Name"), "Acts[0].AiStates[0].Id is 'RtAiState_1;x'");
	expectRefused(with("Id = [[RtNpcEventHandler_2]]", "Id = [[r2_0002_x.y]]"), "Acts[1].Events[0].Id is 'r2_0002_x.y'");
	expectRefused(with("Id = [[RtNpcEventHandler_2]]", "Id = [[]]"), "Acts[1].Events[0].Id is ''");
}

TEST(RtScenarioCheck, AnActionIsOfATypeOfAScenario)
{
	// the parameter of the action of the scenario, the name of a state, is one these types can have
	const char *const types[] = {
		"begin_state", "code", "dynamic_if", "emot", "facing",
		"multi_actions", "null_action", "punctual_state", "punctual_state_end", "random_select",
		"sit_down", "stand_up", "switch_actions", "trigger_event_0", "trigger_event_9"
	};
	for (uint32 i = 0; i < sizeof(types) / sizeof(types[0]); ++i)
	{
		EXPECT_TRUE(isRtScenarioActionType(types[i])) << types[i];
		expectAccepted(with("Action = [[begin_state]]", std::string("Action = [[") + types[i] + "]]"));
	}
	const char *const withParameters[][2] = {
		{ "condition_if", "v0 == 1" }, { "condition_if_else", "RtNpcGrp_2:v1 != 0" }, { "modify_variable", "v0 + 1" },
		{ "set_timer_t0", "100" }, { "set_timer_t3", "2.5" }, { "npc_say", "RtNpcGrp_2:Yubo\nRtEntryText_1\n" }
	};
	for (uint32 i = 0; i < sizeof(withParameters) / sizeof(withParameters[0]); ++i)
	{
		EXPECT_TRUE(isRtScenarioActionType(withParameters[i][0])) << withParameters[i][0];
		expectAccepted(withScript(withParameters[i][0], withParameters[i][1]));
	}

	// a say action is written "say: <phrase>": the word before the colon is the channel the npc talks in
	EXPECT_TRUE(isRtScenarioActionType("say"));
	expectAccepted(withScript("say", "say: a_phrase"));
	expectRefused(withScript("say", "shout: a_phrase"), "Acts[1].Events[0]: the action 'RtNpcEventHandlerAction_4'.Parameters is 'shout: a_phrase', a say action begins with 'say:'");
	expectRefused(withScript("say", "a_phrase"), "a say action begins with 'say:'");
	expectRefused(withScript("say", ""), "a say action begins with 'say:'");

	// types of action the AI service has for the primitives of a shard, and names it would read as one
	const char *const others[] = {
		"send_message", "spawn_dyn_group", "set_flags_on_dyn_zones", "outpost_send_squad_status", "outpost_report_squad_death",
		"set_fauna_activity", "random_select_state", "set_state_timeout", "spawn", "despawn", "CODE", "Code", "code ", "", "trigger_event_10", "set_timer_t4"
	};
	for (uint32 i = 0; i < sizeof(others) / sizeof(others[0]); ++i)
	{
		EXPECT_FALSE(isRtScenarioActionType(others[i])) << others[i];
		expectRefused(with("Action = [[begin_state]]", std::string("Action = [[") + others[i] + "]]"), "which is not an action of a scenario");
	}
	expectRefused(with("Action = [[begin_state]]", "Action = [[send_message]]"), "Acts[1].Events[0]: the action 'RtNpcEventHandlerAction_4'.Action is 'send_message'");

	// an action below another one
	expectRefused(with("Action = [[npc_say]]", "Action = [[send_message]]"), "Acts[0].Events[0]: the action 'RtNpcEventHandlerAction_3'.Action is 'send_message'");
}

TEST(RtScenarioCheck, TheFunctionsOfAScenarioAreAListInOrder)
{
	uint32 count = 0;
	const char *const *functions = getRtScenarioScriptFunctions(count);
	ASSERT_GT(count, 0u);
	for (uint32 i = 0; i < count; ++i)
	{
		if (i != 0)
			EXPECT_LT(strcmp(functions[i - 1], functions[i]), 0) << functions[i] << " is not after " << functions[i - 1] << ": it is not found";
		EXPECT_TRUE(isRtScenarioScriptFunction(functions[i])) << functions[i];
	}

	// the same name with other parameters is another function
	EXPECT_TRUE(isRtScenarioScriptFunction("emote_css_"));
	EXPECT_FALSE(isRtScenarioScriptFunction("emote_ss_"));
	EXPECT_TRUE(isRtScenarioScriptFunction("npcSay_css_"));
	EXPECT_FALSE(isRtScenarioScriptFunction("npcSay_ss_"));
	EXPECT_FALSE(isRtScenarioScriptFunction("setEvent"));
	EXPECT_FALSE(isRtScenarioScriptFunction("setEvent_f_ "));
	EXPECT_FALSE(isRtScenarioScriptFunction(""));

	// functions for the scripts of a shard
	const char *const others[] = {
		"setGlobalNelVar_ss_", "setGlobalNelVar_sf_", "loadFile_s_", "import_s_", "teleportPlayer_sffff_", "summonPlayer_fs_",
		"setPlayerController_ss_", "clearPlayerController_s_", "addBotChat_s_", "setSimplePhrase_ss_", "dataSetVar_ss_", "dataGetVar_s_s",
		"setZoneState_sf_", "queryEgs_sscfs_", "getPlayerStat_ss_f", "setDynEnergy_sff_", "setNamedEntityPropCb_sss_", "spawnManager_s_",
		"despawnBotByAlias_s_", "setBotHPScaleByAlias_fs_", "rename_s_", "followPlayer_sf_", "setUrl_ss_"
	};
	for (uint32 i = 0; i < sizeof(others) / sizeof(others[0]); ++i)
		EXPECT_FALSE(isRtScenarioScriptFunction(others[i])) << others[i];
}

TEST(RtScenarioCheck, AScriptCallsTheFunctionsOfAScenario)
{
	// what the translator of the editor writes
	expectScriptAccepted("()setEvent(1);");
	expectScriptAccepted("()r2_0001_RtNpcGrp_1.setEvent(1);\n");
	expectScriptAccepted("v2 = 3;\n()setTimer(31, 0);\n");
	expectScriptAccepted("(r2_0001_RtNpcGrp_1.is_enable)r2_0001_RtNpcGrp_1.timerIsEnabled(0);\n");
	expectScriptAccepted("(@group)r2_0001_RtNpcGrp_2.context();\n \n()r2_0001_RtNpcGrp_1.npcSay(@group,\"Bob\", \"DSS_1 RtEntryText_1\");\n \n");
	expectScriptAccepted("(@group1)r2_0001_RtNpcGrp_1.context();\n(@group2)r2_0001_RtNpcGrp_2.context();\n()r2_0001_RtNpcGrp_1.facing(@group1,\"Bob\", @group2, \"Ann\");\n");
	expectScriptAccepted("//next_road\nif (  3 == 0  || r2_0001_RtNpcGrp_1.RoadCountLimit < 3 - 1) {\n\tif ( 3 != 0) { r2_0001_RtNpcGrp_1.RoadCountLimit = r2_0001_RtNpcGrp_1.RoadCountLimit + 1; }\n\t()r2_0001_RtNpcGrp_1.postNextState(\"r2_0001_RtAiState_4\");\n} else {\n\t()setEvent(9);\n}\n");
	expectScriptAccepted("if (nextState == 1) {\n\t()stopMoving();\n\t()setMode(\"Rest\");\n} else {\n\tlog(\"unknown state=\", nextState, \", zone=z\");\n\t()break();\n}\n");
	expectScriptAccepted("print(\"<dialog_starts>\");\n()setTimer(10, 1);\n");
	expectScriptAccepted("()setScenarioPoints(1, r2_0001_RtNpcGrp_1.ScenarioPoints);\n()r2_0001_RtNpcGrp_1.setTimer(300,1);\n");
	expectScriptAccepted("(r)rndm(0, 3);\nswitch(v2)\n{\n\tcase 1 :\n\t\t()setEvent(5);\n}\n");
	expectScriptAccepted("r2_0001_RtNpcGrp_1.Active == 1 && r2_0001_RtNpcGrp_1.NbPlayer >= 2", true);
	expectScriptAccepted("v0 == 1", true);
	expectScriptAccepted("", true);
	expectScriptAccepted("");

	// a function the AI service does not have
	expectScriptRefused("()r2_0001_RtNpcGrp_1.timerEnable(1);\n", "timerEnable");

	// A script of a scenario defines and calls no function of its own, and has no loop, no block
	// one statement of which is picked at random and no block for the groups below a group
	expectScriptRefusedFor("step()\n{\n\t()setEvent(1);\n}\n", "defines or calls the script function 'step'");
	expectScriptRefusedFor("()setEvent(1);\nstep();\n", "defines or calls the script function 'step'");
	expectScriptRefusedFor("r2_0001_RtNpcGrp_1.step();\n", "defines or calls the script function 'step'");
	expectScriptRefusedFor("a() = r2_0001_RtNpcGrp_1.b();\n", "defines or calls the script function");
	expectScriptRefused("while (v0 == 0) { v1 = v1 + 1; }", "while");
	expectScriptRefused("rand { ()setEvent(1); ()setEvent(2); }", "rand");
	expectScriptRefused("onchildren() { ()setEvent(1); }", "onchildren");
	expectScriptRefused("if (v0 == 1) { v1 = 1; } while (v1 == 1) ()setEvent(1);", "while");
	// a word that only begins like one of these, and the same words in a text
	expectScriptAccepted("whilev = 1;\nrandom = 2;\n()setEvent(1);\nlog(\"while rand onchildren\");\n");

	// the size of a script, and how deep its parentheses and braces are
	expectScriptAccepted(std::string(RtScenarioMaxScriptSize - 16, ' ') + "()setEvent(1);");
	expectScriptRefusedFor(std::string(RtScenarioMaxScriptSize, ' ') + "()setEvent(1);", "a script has 65536 at most");
	expectScriptAccepted("v0 = " + std::string(RtScenarioMaxScriptDepth, '(') + "1" + std::string(RtScenarioMaxScriptDepth, ')') + ";");
	expectScriptRefusedFor("v0 = " + std::string(RtScenarioMaxScriptDepth + 1, '(') + "1" + std::string(RtScenarioMaxScriptDepth + 1, ')') + ";", "more than 32 deep");
	expectScriptAccepted(repeat("if (v0 == 1) {", RtScenarioMaxScriptDepth) + repeat("}", RtScenarioMaxScriptDepth));
	expectScriptRefusedFor(repeat("if (v0 == 1) {", RtScenarioMaxScriptDepth + 1) + repeat("}", RtScenarioMaxScriptDepth + 1), "more than 32 deep");

	// functions for the scripts of a shard
	expectScriptRefused("()setGlobalNelVar(\"a\", \"b\");", "setGlobalNelVar");
	expectScriptRefused("()setEvent(1);\n()r2_0001_RtNpcGrp_1.teleportPlayer(\"(0x0000000001:00:00:00)\", 1, 2, 3, 4);", "teleportPlayer");
	expectScriptRefused("(@child)newNpcChildGroup(\"a\", \"b\", \"c\");", "newNpcChildGroup");
	expectScriptRefused("(v0, $name, @ctx)@other.queryEgs(\"a\", \"b\", @ctx, 1, \"c\");", "queryEgs");
	expectScriptRefused("( v0 , r2_0001_RtNpcGrp_1.v1 )\n\tgetPlayerStat ( \"a\" , \"b\" ) ;", "getPlayerStat");
	expectScriptRefused("if (v0 == 1) { ()loadFile(\"x\"); }", "loadFile");
	expectScriptRefused("()setevent(1);", "setevent");
	expectScriptRefused("()set(1);", "set");
	expectScriptRefused("()timer(1);", "timer");
	// the condition of a dynamic_if is a script too
	expectScriptRefused("()getPlayerStat(\"a\", \"b\") == 1", "getPlayerStat", true);
}

TEST(RtScenarioCheck, AScriptIsReadAsTheAiServiceReadsIt)
{
	// What is a text, a comment or script is told here with a row of parentheses deeper than a script
	// can have: it is refused where it is read as script, and nowhere else
	const std::string deep(RtScenarioMaxScriptDepth + 1, '(');
	const char *tooDeep = "more than 32 deep";
	expectScriptRefusedFor("v0 = " + deep + "1;", tooDeep);

	// what is in a text or in a comment is not read
	expectScriptAccepted("()npcSay(@group, \"Bob\", \"RAW " + deep + "\");");
	expectScriptAccepted("// " + deep + "\n()setEvent(1);");
	expectScriptAccepted("()setEvent(1); // " + deep);
	expectScriptAccepted("# " + deep + "\n()setEvent(1);");

	// a text ends at its quote: what follows it on the line is read
	expectScriptRefusedFor("$name = \"Bob\" " + deep + ";", tooDeep);
	// a quote after a backslash is inside its text
	expectScriptAccepted("$name = \"Bob \\\" " + deep + "\";");
	// a quote without its end is no text: what follows it is read
	expectScriptRefusedFor("()setEvent(1); \" " + deep, tooDeep);
	// a comment line is one that begins the script: the others begin with the space the compiler joins them with
	expectScriptRefusedFor("()setEvent(1);\n# " + deep, tooDeep);
	// what follows a comment is on the next line
	expectScriptRefusedFor("// a\nv0 = " + deep + "1;", tooDeep);

	// The compiler joins the lines it is given, the empty ones too. The animation module gives it none
	// that is empty: a line that begins with '#' after empty lines is a comment when it comes from
	// there, and script when the empty lines are compiled with it, which is how it is read.
	expectScriptAccepted("\n\n()setEvent(1);\n\n\n()setEvent(2);\n\n");
	expectScriptRefusedFor("\n\n# " + deep + "\n()setEvent(1);", tooDeep);
	expectScriptRefusedFor("\n# " + deep, tooDeep);

	// The compiler cuts a line at the "//" that is outside a text, and looks for the quotes in its own
	// way: it does not look at the character after a '/' that is alone
	expectScriptAccepted("print(\"a/\", \"b//c\"); ()setEvent(1);");
	expectScriptRefusedFor("print(\"a\\\"/\", \"b//c\"); v0 = " + deep + "1;", tooDeep);
	expectScriptAccepted("()setEvent(1); v0 = 4 /");
	expectScriptAccepted("v0 = \"//\"; v1 = 1 /");

	// The condition of a dynamic_if is put between the parentheses of an if by the AI service: it is
	// one level deep, and a condition of one line does not begin a line
	expectScriptAccepted("v0 == " + std::string(RtScenarioMaxScriptDepth - 1, '(') + "1" + std::string(RtScenarioMaxScriptDepth - 1, ')'), true);
	expectScriptRefusedFor("v0 == " + std::string(RtScenarioMaxScriptDepth, '(') + "1" + std::string(RtScenarioMaxScriptDepth, ')'), tooDeep, true);
	expectScriptRefusedFor("# " + deep, tooDeep, true);
	expectScriptAccepted("# " + deep + "\nv0 == 1", true);
	// its lines are the ones that are not empty, as the animation module makes them
	expectScriptAccepted("\n\n# " + deep + "\n\nv0 == 1\n\n", true);
	// and they get the prefix of the session when the condition has a colon: no line begins with a
	// '#' then
	expectScriptRefusedFor("# : " + deep + "\nv0 == 1", tooDeep, true);
}

TEST(RtScenarioCheck, AScriptHasNoControlCharacterButTheTabAndTheEndsOfItsLines)
{
	expectScriptAccepted("\t()setEvent(1);\r\n\t()setEvent(2);\n");
	// the bytes of a text of the author in another script than latin
	expectScriptAccepted("()npcSay(@group, \"Zo\xc3\xa9 \xe9\xbe\x8d\", \"DSS_1 RtEntryText_1\");");
	expectScriptAccepted("()npcSay(@group, \"\x7f\", \"DSS_1 RtEntryText_1\");");

	// the lexer of the AI service reads a script up to its first zero byte
	std::string script = "()setEvent(1);";
	script += '\0';
	script += "()setEvent(2);";
	expectScriptRefusedFor(script, "has the control character 0x00 at byte 14, a script has the tab and the ends of its lines only");

	// the others, where they are script, in a text and in a comment, and in a condition
	const char others[] = { '\x00', '\x01', '\x08', '\x0b', '\x0c', '\x0e', '\x1b', '\x1f' };
	for (uint32 i = 0; i < sizeof(others) / sizeof(others[0]); ++i)
	{
		const std::string character(1, others[i]);
		const std::string rule = NLMISC::toString("has the control character 0x%02x", (uint32)(uint8)others[i]);
		expectScriptRefusedFor("()setEvent(1); " + character, rule);
		expectScriptRefusedFor(character + "()setEvent(1);", rule + " at byte 0");
		expectScriptRefusedFor("$a = \"a" + character + "b\";", rule);
		expectScriptRefusedFor("// a" + character + "\n()setEvent(1);", rule);
		expectScriptRefusedFor("v0 == 1" + character, rule, true);
	}
}

TEST(RtScenarioCheck, AStatementOfAScriptHasALargestNumberOfTokens)
{
	const uint32 most = RtScenarioMaxScriptStatementTokens;
	const char *rule = "has a statement of more than 256 tokens";

	// the ';' that ends a statement is not one of its tokens
	expectScriptAccepted(tokensOf(most - 1) + ";");
	expectScriptAccepted(tokensOf(most) + ";");
	expectScriptRefusedFor(tokensOf(most + 1) + ";", rule);

	// each statement is counted alone: a ';', a '{' and a '}' end it
	expectScriptAccepted(repeat(tokensOf(most - 1) + ";\n", 20));
	expectScriptAccepted("if (v0 == 1) { " + tokensOf(most - 1) + "; } " + tokensOf(most - 1) + ";");
	expectScriptRefusedFor("if (v0 == 1) { " + tokensOf(most - 1) + "; } " + tokensOf(most + 1) + ";", rule);

	// the parameters and the results of a call, and the parts of a print
	expectScriptAccepted("()setTimer(" + repeat("1 + ", 100) + "1, 0);");
	expectScriptRefusedFor("()setTimer(" + repeat("1, ", most / 2) + "1);", rule);
	expectScriptRefusedFor("(" + repeat("v0, ", most / 2) + "v0)rndm(0, 3);", rule);
	expectScriptRefusedFor("print(" + repeat("\"a\", ", most / 2) + "\"a\");", rule);
	expectScriptRefusedFor("v0 = " + repeat("1 * ", most / 2) + "1;", rule);

	// ifs without braces, one inside the other, are one statement
	expectScriptAccepted(repeat("if (v0 == 1) ", 10) + "()setEvent(1);");
	expectScriptRefusedFor(repeat("if (v0 == 1) ", most / 7 + 1) + "()setEvent(1);", rule);

	// a text is one token whatever it holds, and a comment none
	expectScriptAccepted("$a = \"" + repeat("1 + ", most) + "\";");
	expectScriptAccepted("v0 = 1 // " + repeat("+ 1 ", most) + "\n + 1;");

	// the condition of a dynamic_if is a statement with the "if (" and the ")" the AI service puts
	// around it
	expectScriptAccepted(tokensOf(most - 3), true);
	expectScriptRefusedFor(tokensOf(most - 2), rule, true);
}

TEST(RtScenarioCheck, ElseAfterElseHasALargestNumber)
{
	const uint32 most = RtScenarioMaxScriptElseChain;
	const char *rule = "has more than 32 'else' that follow one another";

	expectScriptAccepted("if (v0 == 1) { ()setEvent(1); } else if (v0 == 2) { ()setEvent(2); } else { ()setEvent(3); }");

	// with braces and without
	expectScriptAccepted(repeat("if (v0 == 1) { ()setEvent(1); } else ", most) + "{ ()setEvent(2); }");
	expectScriptRefusedFor(repeat("if (v0 == 1) { ()setEvent(1); } else ", most + 1) + "{ ()setEvent(2); }", rule);
	expectScriptAccepted(repeat("if (v0 == 1) v1 = 1; else ", most) + "v1 = 2;");
	expectScriptRefusedFor(repeat("if (v0 == 1) v1 = 1; else ", most + 1) + "v1 = 2;", rule);

	// a statement that does not follow an else ends the chain
	expectScriptAccepted(repeat("if (v0 == 1) { v1 = 1; } else { v1 = 2; }\n", 200));
	expectScriptAccepted(repeat(repeat("if (v0 == 1) v1 = 1; else ", most) + "v1 = 2;\n", 4));
	expectScriptAccepted(repeat(repeat("if (v0 == 1) { v1 = 1; } else ", most) + "{ v1 = 2; }\n{ v1 = 3; }\n", 4));

	// the chain between the braces of a statement of a chain is another chain
	expectScriptAccepted(repeat("if (v0 == 1) { " + repeat("if (v0 == 1) v1 = 1; else ", most) + "v1 = 2; } else ", most) + "v1 = 3;");
	expectScriptRefusedFor("if (v0 == 1) { " + repeat("if (v0 == 1) v1 = 1; else ", most + 1) + "v1 = 2; } else v1 = 3;", rule);

	// the else of ifs without braces one inside the other
	expectScriptRefusedFor(repeat("if (v0 == 1) ", most + 1) + "v1 = 1;" + repeat(" else v1 = 2;", most + 1), rule);

	// the else the AI service puts after the condition of a dynamic_if is the only one of its chain
	expectScriptAccepted("v0 == 1", true);
}

TEST(RtScenarioCheck, ATextOfAScriptHasALargestSize)
{
	const uint32 most = RtScenarioMaxScriptTextSize;
	const char *rule = "a text of a script has 4096 at most";

	// the bytes between the quotes
	expectScriptAccepted("()npcSay(@group, \"" + std::string(most, 'a') + "\", \"t\");");
	expectScriptRefusedFor("()npcSay(@group, \"" + std::string(most + 1, 'a') + "\", \"t\");", "has a text of 4097 bytes between quotes, a text of a script has 4096 at most");
	expectScriptRefusedFor("print(\"" + std::string(most + 1, 'a') + "\");", rule);
	expectScriptRefusedFor("$a = \"" + std::string(RtScenarioMaxScriptSize - 16, 'a') + "\";", rule);

	// each text is measured alone
	expectScriptAccepted(repeat("$a = \"" + std::string(most, 'a') + "\";\n", 8));

	// a quote after a backslash is inside its text, which goes on to the quote that ends it
	expectScriptAccepted("$a = \"" + std::string(most - 2, 'a') + "\\\"\";");
	expectScriptRefusedFor("$a = \"" + std::string(most / 2, 'a') + "\\\"" + std::string(most / 2, 'a') + "\";", "has a text of 4098 bytes between quotes");

	// what is in a comment is no text
	expectScriptAccepted("// \"" + std::string(most + 1, 'a') + "\"\n()setEvent(1);");

	// in the condition of a dynamic_if
	expectScriptAccepted("$a == \"" + std::string(most, 'a') + "\"", true);
	expectScriptRefusedFor("$a == \"" + std::string(most + 1, 'a') + "\"", rule, true);
}

TEST(RtScenarioCheck, TheScriptsOfAScenarioAreChecked)
{
	expectAccepted(withScript("code", "()setEvent(1);\n()r2_0001_RtNpcGrp_2.setTimer(30, 0);\n"));
	expectAccepted(withScript("dynamic_if", "r2_0001_RtNpcGrp_2.Active == 1"));

	expectRefused(withScript("code", "()setEvent(1);\n()setGlobalNelVar(\"a\", \"b\");\n"),
	    "Acts[1].Events[0]: the action 'RtNpcEventHandlerAction_4'.Parameters calls the function 'setGlobalNelVar', which is not a function of a scenario");
	expectRefused(withScript("dynamic_if", "()getPlayerStat(\"a\", \"b\") == 1"), "calls the function 'getPlayerStat'");

	// the parameters of the other actions are no script: what a script cannot have is a text there
	const std::string deep(RtScenarioMaxScriptDepth + 1, '(');
	expectRefused(withScript("code", "v0 = " + deep + "1;"), "Parameters has parentheses and braces more than 32 deep");
	expectAccepted(withScript("npc_say", "RtNpcGrp_2:Yubo\n" + deep));

	// what a script of a scenario does not have
	expectRefused(withScript("code", "while (v0 == 0) { v0 = 0; }"), "Parameters uses 'while', which a script of a scenario does not have");
	expectRefused(withScript("code", "step() { ()setEvent(1); }"), "Parameters defines or calls the script function 'step'");
	expectRefused(withScript("code", std::string(RtScenarioMaxScriptSize + 1, ' ')), "a script has 65536 at most");
	expectRefused(withScript("code", "()setEvent(1);\x01"), "Parameters has the control character 0x01 at byte 14, a script has the tab and the ends of its lines only");
	expectRefused(withScript("dynamic_if", "v0 == 1\x1f"), "Parameters has the control character 0x1f at byte 7");
	expectRefused(withScript("code", tokensOf(RtScenarioMaxScriptStatementTokens + 1) + ";"), "Parameters has a statement of more than 256 tokens");
	expectRefused(withScript("code", repeat("if (v0 == 1) v1 = 1; else ", RtScenarioMaxScriptElseChain + 1) + "v1 = 2;"), "Parameters has more than 32 'else' that follow one another");
	expectRefused(withScript("code", "$a = \"" + std::string(RtScenarioMaxScriptTextSize + 1, 'a') + "\";"), "Parameters has a text of 4097 bytes between quotes, a text of a script has 4096 at most");

	// an action below another one
	expectRefused(replaceAll(with("Action = [[npc_say]]", "Action = [[code]]"), "Parameters = [[RtNpcGrp_1:Guard\nRtEntryText_1]]", "Parameters = [[()summonPlayer(0, \"x\");]]"),
	    "Acts[0].Events[0]: the action 'RtNpcEventHandlerAction_3'.Parameters calls the function 'summonPlayer'");
}

TEST(RtScenarioCheck, TheParametersOfAnActionHaveTheShapeOfItsType)
{
	// names of states and of groups
	expectAccepted(withScript("begin_state", "RtAiState_2\nClient1_20.init\nRtAiState_2|wander"));
	expectRefused(withScript("begin_state", "RtAiState_2;x"), "Parameters has the line 'RtAiState_2;x'");
	expectRefused(withScript("punctual_state", "a state"), "Parameters has the line 'a state'");
	expectRefused(withScript("trigger_event_3", "RtNpcGrp_2\nRtNpcGrp:2"), "Parameters has the line 'RtNpcGrp:2'");

	// The npc that talks, of a group that has an id, on a line and the id of the text on the next. The
	// lines are the ones the animation module makes of the parameters: the empty ones too, and without
	// the '\r'. (The text of a scenario leaves out a line end that follows its "[[".)
	expectAccepted(withScript("npc_say", "RtNpcGrp_2:Yubo the \"brave\"\nRtEntryText_1"));
	expectAccepted(withScript("npc_say", "RtNpcGrp_2:Yubo\nRtEntryText_1\n"));
	expectAccepted(withScript("npc_say", "RtNpcGrp_2:Yubo\r\nRtEntryText_1\r\n"));
	expectAccepted(withScript("npc_say", "RtNpcGrp_2\r:Yubo\nRtEntryText_1"));
	expectAccepted(withScript("npc_say", "RtNpcGrp_2:Yubo\nRtEntryText_1\nmore\n\nlines"));
	// the translator leaves the second line empty for a text the user of the editor did not write
	expectAccepted(withScript("npc_say", "RtNpcGrp_2:Yubo\n\n"));
	// without a group the leader of the group of the action talks
	expectAccepted(withScript("npc_say", "Yubo\nRtEntryText_1"));
	expectAccepted(withScript("npc_say", "\n\nRtEntryText_1"));
	expectRefused(withScript("npc_say", "RtNpcGrp 2:Yubo\nRtEntryText_1"), "Parameters names the group 'RtNpcGrp 2'");
	expectRefused(withScript("npc_say", ":Yubo\nRtEntryText_1"), "Parameters names the group ''");
	expectRefused(withScript("npc_say", "RtNpcGrp_2;x:Yubo\n\n"), "Parameters names the group 'RtNpcGrp_2;x'");
	// With less than two lines the animation module gives the AI service an action that has nothing to
	// say, whatever the line is
	const char *sayRule = "an npc_say action has the npc on a line and the text on the next";
	const char *const oneLine[] = { "", "RtEntryText_1", "Welcome: to the gate", "RtNpcGrp_2:Yubo", "RtNpcGrp_2:Yubo\n", "RtNpcGrp_2:Yubo\r\r", "RtNpcGrp_2:Yubo\r\n\r", "RtNpcGrp 2:Yubo", " \n", "\r\r" };
	for (uint32 i = 0; i < sizeof(oneLine) / sizeof(oneLine[0]); ++i)
		expectRefused(withScript("npc_say", oneLine[i]), sayRule);
	expectRefused(withScript("npc_say", "RtNpcGrp_2:Yubo\n"), "Acts[1].Events[0]: the action 'RtNpcEventHandlerAction_4'.Parameters is 'RtNpcGrp_2:Yubo?', an npc_say action has the npc on a line and the text on the next");
	expectRefused(replaceAll(with("Action = [[begin_state]]", "Action = [[npc_say]]"), "Parameters = [[RtAiState_2]]", "Parameters = 12"), "Parameters is the number 12, an npc_say action has the npc on a line and the text on the next");
	expectRefused(replaceAll(with("Action = [[begin_state]]", "Action = [[npc_say]]"), "Parameters = [[RtAiState_2]], ", ""), "Parameters is missing, an npc_say action has the npc on a line and the text on the next");
	expectAccepted(withScript("facing", "RtNpcGrp_2:Yubo\nRtNpcGrp_2:Yubo"));
	expectRefused(withScript("facing", "RtNpcGrp_2:Yubo\nRtNpcGrp.2:Yubo"), "Parameters names the group 'RtNpcGrp.2'");
	expectAccepted(withScript("emot", "Bow\nRtNpcGrp_2:Yubo"));
	expectRefused(withScript("emot", ""), "an emot action begins with the name of the emote");
	expectRefused(withScript("emot", "Bow\nRtNpcGrp;2:Yubo"), "Parameters names the group 'RtNpcGrp;2'");

	// a variable, an operator and a value
	const char *const conditions[] = { "condition_if", "condition_if_else", "modify_variable" };
	for (uint32 i = 0; i < sizeof(conditions) / sizeof(conditions[0]); ++i)
	{
		expectAccepted(withScript(conditions[i], "v0 == 1"));
		expectAccepted(withScript(conditions[i], "  v0   ==   1  "));
		expectRefused(withScript(conditions[i], ""), "a variable, an operator and a value are needed");
		expectRefused(withScript(conditions[i], "v0"), "a variable, an operator and a value are needed");
		expectRefused(withScript(conditions[i], "v0 =="), "a variable, an operator and a value are needed");
		expectRefused(withScript(conditions[i], "v0 == 1 2"), "a variable, an operator and a value are needed");
		expectRefused(withScript(conditions[i], "v0 == 1\nv1 == 2"), "a variable, an operator and a value are needed");
	}

	// a time
	expectAccepted(withScript("set_timer_t1", "0"));
	expectAccepted(withScript("set_timer_t2", "36000"));
	expectRefused(withScript("set_timer_t0", ""), "a number is needed");
	expectRefused(withScript("set_timer_t0", " "), "a number is needed");
	expectRefused(withScript("set_timer_t0", "ten"), "a number is needed");
	expectRefused(withScript("set_timer_t0", "10 20"), "a number is needed");
	expectRefused(withScript("set_timer_t0", "1.2.3"), "a number is needed");
	expectRefused(withScript("set_timer_t0", "-1"), "a number is needed");
	expectRefused(withScript("set_timer_t0", "10\n20"), "a number is needed");

	// the variable of a switch
	expectAccepted(withScript("switch_actions", "v2"));
	expectAccepted(withScript("switch_actions", "oldChatStepVar"));
	expectRefused(withScript("switch_actions", ""), "the name of a variable is needed");
	expectRefused(withScript("switch_actions", "v2 v3"), "the name of a variable is needed");
	expectRefused(withScript("switch_actions", "2v"), "the name of a variable is needed");
	expectRefused(withScript("switch_actions", "v2\n1\n2"), "the name of a variable is needed");
}

TEST(RtScenarioCheck, AnEventIsOfATypeOfAScenarioAndNamesStatesAndGroups)
{
	const char *const types[] = {
		"start_of_state", "end_of_state", "group_spawned", "group_despawned", "group_eliminated", "group_under_attack", "bot_killed",
		"bot_target_killed", "destination_reached", "destination_reached_all", "player_target_npc", "player_arrived_trigger_zone",
		"player_left_trigger_zone", "timer_t0_triggered", "timer_t3_triggered", "user_event_0", "user_event_9", "variable_changed",
		"variable_v0_changed", "variable_v3_changed"
	};
	for (uint32 i = 0; i < sizeof(types) / sizeof(types[0]); ++i)
		expectAccepted(with("Event = [[user_event_1]]", std::string("Event = [[") + types[i] + "]]"));

	const char *const others[] = { "state_timeout", "punctual_state_timeout", "user_event_10", "timer_t4_triggered", "Start_Of_State", "", "outpost_peace_state_begin" };
	for (uint32 i = 0; i < sizeof(others) / sizeof(others[0]); ++i)
		expectRefused(with("Event = [[user_event_1]]", std::string("Event = [[") + others[i] + "]]"), "which is not an event of a scenario");
	expectRefused(with("Event = [[user_event_1]]", "Event = [[state_timeout]]"), "Acts[1].Events[0].Event is 'state_timeout'");

	// the states and the groups of an event: a name on each line, or none
	expectAccepted(with("StatesByName = [[RtAiState_2]]", "StatesByName = [[]]"));
	expectAccepted(with("StatesByName = [[RtAiState_2]]", "StatesByName = [[RtAiState_2\nall_sequence_states\nClient1_20.init]]"));
	expectRefused(with("StatesByName = [[RtAiState_2]]", "StatesByName = [[RtAiState_2\nRtAiState 3]]"), "Acts[1].Events[0].StatesByName has the line 'RtAiState 3'");
	expectAccepted(with("GroupsByName = [[RtNpcGrp_2]]", "GroupsByName = [[RtNpcGrp_2\nRtNpcGrp_1]]"));
	expectRefused(with("GroupsByName = [[RtNpcGrp_2]]", "GroupsByName = [[RtNpcGrp_2.x]]"), "Acts[1].Events[0].GroupsByName has the line 'RtNpcGrp_2.x'");
	expectRefused(with("GroupsByName = [[RtNpcGrp_2]]", "GroupsByName = [[r2_0002_*]]"), "GroupsByName has the line 'r2_0002_*'");
}

TEST(RtScenarioCheck, TheTextsOfAGroupAnNpcAndAStateAreWhatTheEditorWrites)
{
	// a group
	expectAccepted(with("GrpParameters = [[ring\n]]", "GrpParameters = [[ring\nbot_attackable\nplayer_attackable\naggro range: 30\nrespawn time:-1\ndenied_astar_flags:WaterAndNoGo\n]]"));
	expectAccepted(with("GrpParameters = [[ring\n]]", "GrpParameters = [[ring\naggro range: 0\n\naggro range: 120\naggro range: 12.5\n]]"));
	const char *const groupLines[] = { "Ring", "ring ", "aggro range: -1", "aggro range:30", "aggro range: 30 ", "aggro range: 1e9", "respawn time:10", "denied_astar_flags:Water", "faction:tribe", "nb_bots:4" };
	for (uint32 i = 0; i < sizeof(groupLines) / sizeof(groupLines[0]); ++i)
		expectRefused(with("GrpParameters = [[ring\n]]", std::string("GrpParameters = [[ring\n") + groupLines[i] + "\n]]"), "which is not a parameter of a group of a scenario");
	expectRefused(with("GrpParameters = [[ring\n]]", "GrpParameters = [[ring\nnb_bots:4\n]]"), "Acts[0].AiStates[0]: the group 'RtNpcGrp_1'.GrpParameters has the line 'nb_bots:4'");
	expectAccepted(with("Children = { [[RtNpc_3]] }, AutoSpawn = 0,\n					BotChat_parameters = [[]], BotEquipment = [[]], BotSheetClient = [[]], BotVerticalPos = [[auto]], Count = 0,\n					GrpKeywords = [[]], GrpParameters = [[]], AiProfilParams = [[]]",
	    "Children = { [[RtNpc_3]] }, AutoSpawn = 0,\n					BotChat_parameters = [[]], BotEquipment = [[]], BotSheetClient = [[]], BotVerticalPos = [[auto]], Count = 0,\n					GrpKeywords = [[]], GrpParameters = [[]], AiProfilParams = [[running\n]]"));
	expectRefused(with("Children = { [[RtNpc_3]] }, AutoSpawn = 0,\n					BotChat_parameters = [[]], BotEquipment = [[]], BotSheetClient = [[]], BotVerticalPos = [[auto]], Count = 0,\n					GrpKeywords = [[]], GrpParameters = [[]], AiProfilParams = [[]]",
	    "Children = { [[RtNpc_3]] }, AutoSpawn = 0,\n					BotChat_parameters = [[]], BotEquipment = [[]], BotSheetClient = [[]], BotVerticalPos = [[auto]], Count = 0,\n					GrpKeywords = [[]], GrpParameters = [[]], AiProfilParams = [[faction:tribe]]"),
	    "AiProfilParams has the line 'faction:tribe'");

	// an npc: no chat, no keywords, the equipment of its look
	const char *npc = "Name = [[Yubo]], Children = { }, ChatParameters = [[]], Equipment = [[]], IsStuck = 0,\n					Keywords = [[]]";
	expectAccepted(with(npc, "Name = [[Yubo]], Children = { }, ChatParameters = [[\n]], Equipment = [[VPA:1234567890\nVPB:00ff00FF\nVPC:42\nFAUNA_BOT_USE_BOTNAME\n]], IsStuck = 0,\n					Keywords = [[]]"));
	expectRefused(with(npc, "Name = [[Yubo]], Children = { }, ChatParameters = [[shop:items]], Equipment = [[]], IsStuck = 0,\n					Keywords = [[]]"),
	    "Acts[1].AiStates[0]: the group 'RtNpcGrp_2': the npc 'RtNpc_3'.ChatParameters is 'shop:items', an npc of a scenario has none");
	expectRefused(with(npc, "Name = [[Yubo]], Children = { }, ChatParameters = [[]], Equipment = [[]], IsStuck = 0,\n					Keywords = [[boss]]"),
	    "Keywords is 'boss', an npc of a scenario has none");
	const char *const equipment[] = { "VPD:12", "VPA:", "VPA:1 2", "CHANDS: 1", "ri:icfalb.sitem", "a b" };
	for (uint32 i = 0; i < sizeof(equipment) / sizeof(equipment[0]); ++i)
		expectRefused(with(npc, std::string("Name = [[Yubo]], Children = { }, ChatParameters = [[]], Equipment = [[") + equipment[i] + "]], IsStuck = 0,\n					Keywords = [[]]"),
		    "which is not the equipment of an npc of a scenario");

	// a state: no keywords and no parameters of a profile (its movement and its activity have a test
	// of their own)
	const char *state = "AiActivity = [[normal]], AiMovement = [[follow_route]], AiProfileParams = 0, Keywords = [[]]";
	expectRefused(with(state, "AiActivity = [[normal]], AiMovement = [[follow_route]], AiProfileParams = 0, Keywords = [[patrol]]"),
	    "Acts[1].AiStates[0].Keywords is 'patrol', a state of a scenario has none");
	expectAccepted(with(state, "AiActivity = [[normal]], AiMovement = [[follow_route]], AiProfileParams = 0, Keywords = [[\n\n]]"));
	expectAccepted(with(state, "AiActivity = [[normal]], AiMovement = [[follow_route]], AiProfileParams = 0, Keywords = [[]], AiProfilesParams = [[]]"));
	expectAccepted(with(state, "AiActivity = [[normal]], AiMovement = [[follow_route]], AiProfileParams = 0, Keywords = [[]], AiProfilesParams = [[\n\n\n]]"));
	// what is not a text is not given to the AI service
	expectAccepted(with(state, "AiActivity = [[normal]], AiMovement = [[follow_route]], AiProfileParams = 0, Keywords = [[]], AiProfilesParams = 0"));
	const char *const profileParameters[] = { "running", "faction:tribe", "\nevent_bot_killed:1\n", " ", "\n\n\t" };
	for (uint32 i = 0; i < sizeof(profileParameters) / sizeof(profileParameters[0]); ++i)
		expectRefused(with(state, std::string("AiActivity = [[normal]], AiMovement = [[follow_route]], AiProfileParams = 0, Keywords = [[]], AiProfilesParams = [[\n") + profileParameters[i] + "]]"),
		    "a state of a scenario has none");
	expectRefused(with(state, "AiActivity = [[normal]], AiMovement = [[follow_route]], AiProfileParams = 0, Keywords = [[]], AiProfilesParams = [[running]]"),
	    "Acts[1].AiStates[0].AiProfilesParams is 'running', a state of a scenario has none");
}

TEST(RtScenarioCheck, TheMovementAndTheActivityOfAStateAreWordsTheEditorWrites)
{
	const char *state = "AiActivity = [[normal]], AiMovement = [[follow_route]]";

	// what the translator of the editor writes
	const char *const movements[] = { "", "idle", "follow_route", "stand_on_vertices", "wander", "stand_on_start_point" };
	for (uint32 i = 0; i < sizeof(movements) / sizeof(movements[0]); ++i)
		expectAccepted(with(state, std::string("AiActivity = [[normal]], AiMovement = [[") + movements[i] + "]]"));
	const char *const activities[] = { "", "no_change", "normal" };
	for (uint32 i = 0; i < sizeof(activities) / sizeof(activities[0]); ++i)
		expectAccepted(with(state, std::string("AiActivity = [[") + activities[i] + "]], AiMovement = [[follow_route]]"));
	// a state without an activity is given none
	expectAccepted(with(state, "AiMovement = [[follow_route]]"));
	expectAccepted(with(state, "AiActivity = 0, AiMovement = [[follow_route]]"));

	// The other profiles the AI service finds by a word: of a bot, of the groups of a shard, and the
	// words of one list in the other. A word of the right shape is not enough.
	const char *const otherMovements[] = {
		"bot_flee", "bot_fight", "squad", "faction", "tribu", "escorted", "guard_escorted", "guard", "bandit", "normal", "no_change",
		"patrol_route", "repeat_road", "follow route", "Wander", "wander ", " wander", "wander\n", "wander\nidle", "idle_", "x"
	};
	for (uint32 i = 0; i < sizeof(otherMovements) / sizeof(otherMovements[0]); ++i)
		expectRefused(with(state, std::string("AiActivity = [[normal]], AiMovement = [[") + otherMovements[i] + "]]"), "which is not a movement of a state of a scenario");
	const char *const otherActivities[] = {
		"bot_flee", "bot_fight", "squad", "faction", "tribu", "escorted", "guard_escorted", "guard", "bandit", "idle", "wander", "follow_route",
		"stand_on_start_point", "Normal", "normal:1", "normal ", "no change", "normal\n", "civil", "x"
	};
	for (uint32 i = 0; i < sizeof(otherActivities) / sizeof(otherActivities[0]); ++i)
		expectRefused(with(state, std::string("AiActivity = [[") + otherActivities[i] + "]], AiMovement = [[follow_route]]"), "which is not an activity of a state of a scenario");

	expectRefused(with(state, "AiActivity = [[normal]], AiMovement = [[bot_fight]]"), "Acts[1].AiStates[0].AiMovement is 'bot_fight', which is not a movement of a state of a scenario");
	expectRefused(with(state, "AiActivity = [[bot_flee]], AiMovement = [[follow_route]]"), "Acts[1].AiStates[0].AiActivity is 'bot_flee', which is not an activity of a state of a scenario");
	// the movement is read whatever it is: it must be a text
	expectRefused(with(state, "AiActivity = [[normal]], AiMovement = 0"), "Acts[1].AiStates[0].AiMovement is the number 0, a text is needed");
}

TEST(RtScenarioCheck, NamesAndWordsHaveALargestSize)
{
	const std::string id(RtScenarioMaxIdSize, 'n');
	const std::string longId(RtScenarioMaxIdSize + 1, 'n');
	const char *idRule = "an id has letters, digits and '_', 64 at most";
	const char *stateRule = "the name of a state or a group has letters, digits, '_', '.' and '|', 128 at most";

	// the id of a state, an npc and an event (IdsAreNamesTheServiceMakesNamesOf has the one of a group)
	expectAccepted(with("Id = [[RtAiState_1]], Name", "Id = [[" + id + "]], Name"));
	expectRefused(with("Id = [[RtAiState_1]], Name", "Id = [[" + longId + "]], Name"), "Acts[0].AiStates[0].Id is");
	expectRefused(with("Id = [[RtAiState_1]], Name", "Id = [[" + longId + "]], Name"), idRule);
	expectAccepted(replaceAll(s_Scenario, "RtNpc_3", id));
	expectRefused(replaceAll(s_Scenario, "RtNpc_3", longId), idRule);
	expectAccepted(with("Id = [[RtNpcEventHandler_2]]", "Id = [[" + id + "]]"));
	expectRefused(with("Id = [[RtNpcEventHandler_2]]", "Id = [[" + longId + "]]"), idRule);

	// the name of a state where an event or an action names one: two ids and what joins them
	const std::string stateName(RtScenarioMaxIdSize * 2, 's');
	expectAccepted(with("StatesByName = [[RtAiState_2]]", "StatesByName = [[" + stateName + "]]"));
	expectRefused(with("StatesByName = [[RtAiState_2]]", "StatesByName = [[" + stateName + "s]]"), stateRule);
	expectAccepted(withScript("begin_state", stateName));
	expectRefused(withScript("begin_state", stateName + "s"), stateRule);
	expectAccepted(withScript("trigger_event_0", "RtNpcGrp_2\n" + stateName));
	expectRefused(withScript("trigger_event_0", "RtNpcGrp_2\n" + stateName + "s"), stateRule);
	// the name of a group where an event names one
	expectAccepted(with("GroupsByName = [[RtNpcGrp_2]]", "GroupsByName = [[" + id + "]]"));
	expectRefused(with("GroupsByName = [[RtNpcGrp_2]]", "GroupsByName = [[" + longId + "]]"), idRule);

	// the group of the npc of an action
	expectAccepted(withScript("npc_say", id + ":Yubo\nRtEntryText_1"));
	expectRefused(withScript("npc_say", longId + ":Yubo\nRtEntryText_1"), idRule);
	expectAccepted(withScript("facing", id + ":Yubo\n" + id + ":Yubo"));
	expectRefused(withScript("facing", id + ":Yubo\n" + longId + ":Yubo"), idRule);
	expectAccepted(withScript("emot", "Bow\n" + id + ":Yubo"));
	expectRefused(withScript("emot", "Bow\n" + longId + ":Yubo"), idRule);

	// the name of an emote and of the variable of a switch
	expectAccepted(withScript("emot", std::string(RtScenarioMaxIdSize, 'e') + "\nRtNpcGrp_2:Yubo"));
	expectRefused(withScript("emot", std::string(RtScenarioMaxIdSize + 1, 'e') + "\nRtNpcGrp_2:Yubo"), "an emot action begins with the name of the emote");
	expectAccepted(withScript("switch_actions", std::string(RtScenarioMaxIdSize, 'v')));
	expectRefused(withScript("switch_actions", std::string(RtScenarioMaxIdSize + 1, 'v')), "the name of a variable is needed");

	// a number: 16 characters
	expectAccepted(withScript("set_timer_t0", "1234567890123456"));
	expectRefused(withScript("set_timer_t0", "12345678901234567"), "a number is needed");
	expectAccepted(withScript("set_timer_t0", "12345678.1234567"));
	expectRefused(withScript("set_timer_t0", "12345678.12345678"), "a number is needed");
	expectAccepted(with("GrpParameters = [[ring\n]]", "GrpParameters = [[ring\naggro range: 1234567890123456\n]]"));
	expectRefused(with("GrpParameters = [[ring\n]]", "GrpParameters = [[ring\naggro range: 12345678901234567\n]]"), "which is not a parameter of a group of a scenario");

	// the equipment of an npc: a visual property of 32 characters, a word of 64
	const char *npc = "Name = [[Yubo]], Children = { }, ChatParameters = [[]], Equipment = [[]], IsStuck = 0,\n					Keywords = [[]]";
	const char *equipmentRule = "which is not the equipment of an npc of a scenario";
	expectAccepted(with(npc, "Name = [[Yubo]], Children = { }, ChatParameters = [[]], Equipment = [[VPA:" + std::string(32, '7') + "\n" + std::string(RtScenarioMaxIdSize, 'W') + "]], IsStuck = 0,\n					Keywords = [[]]"));
	expectRefused(with(npc, "Name = [[Yubo]], Children = { }, ChatParameters = [[]], Equipment = [[VPA:" + std::string(33, '7') + "]], IsStuck = 0,\n					Keywords = [[]]"), equipmentRule);
	expectRefused(with(npc, "Name = [[Yubo]], Children = { }, ChatParameters = [[]], Equipment = [[" + std::string(RtScenarioMaxIdSize + 1, 'W') + "]], IsStuck = 0,\n					Keywords = [[]]"), equipmentRule);
}
