// Ryzom - MMORPG Framework <https://wiki.ryzom.dev/>
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

// The world methods of the control endpoint: world.self, world.entities, world.target, world.go_to,
// world.stop, world.face, camera.get, camera.set, world.collision. They act as the player's input does:
// a target is a selection, a move is the client's own move to an entity or a turn and autowalk toward
// a point (in a straight line, with the collision's sliding: there is no path planner), the camera is
// the player's camera. Positions are world metres (x east, y north, z up); headings are degrees,
// counterclockwise from east, as atan2 gives them.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/misc/aabbox.h"
#include "nel/misc/common.h"
#include "nel/misc/line.h"
#include "nel/pacs/u_global_position.h"
#include "nel/pacs/u_global_retriever.h"
#include "nel/pacs/u_move_container.h"
#include "nel/pacs/u_move_primitive.h"

#include "client_cfg.h"
#include "continent_manager.h"
#include "entities.h"
#include "entity_cl.h"
#include "far_tp.h"
#include "game_share/mode_and_behaviour.h"
#include "motion/user_controls.h"
#include "pacs_client.h"
#include "user_entity.h"
#include "view.h"
#include "world_database_manager.h"

#include <cmath>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace ClientCtrl {

namespace {

const double s_Pi = 3.14159265358979323846;
// the bounds of the numbers the methods take: a value beyond them is a mistake of the caller, which the engine's
// floats and angles would carry on (a heading of infinity turns the character to NaN for good)
const double s_MaxCoordinate = 1e6; // metres: the world is some tens of kilometres across
const double s_MaxDegrees = 3600; // ten turns
// a jump of the character between two polls that no run makes: a teleport
const double s_TeleportJump = 100;

/// Reads a number of an object (the arguments, or an object among them) into value: absent or null leaves value as it
/// is; false with the call failed when it is not a number, not finite, or beyond limit either way
bool readNumber(CCtrlCall &call, const CJsonValue &object, const char *name, double limit, double &value)
{
	const CJsonValue &arg = object.get(name);
	if (arg.isNull())
		return true;
	if (!arg.isNumber() || !isValidDouble(arg.getNumber()) || fabs(arg.getNumber()) > limit)
	{
		call.fail(toString("%s is a number from %g to %g", name, -limit, limit));
		return false;
	}
	value = arg.getNumber();
	return true;
}

/// The point x, y of an object: false with the call failed unless both are there, as numbers within the world
bool readPoint(CCtrlCall &call, const CJsonValue &object, double &x, double &y)
{
	if (object.get("x").isNull() || object.get("y").isNull())
	{
		call.fail("a point needs both x and y");
		return false;
	}
	return readNumber(call, object, "x", s_MaxCoordinate, x) && readNumber(call, object, "y", s_MaxCoordinate, y);
}

double round2(double v)
{
	return floor(v * 100.0 + 0.5) / 100.0;
}

/// An angle in radians brought to -pi..pi, by fmod: a loop that adds or takes a turn never ends for a value far beyond
/// one, once a turn is below the precision of the double
double normaliseRadians(double a)
{
	a = fmod(a, 2 * s_Pi);
	if (a > s_Pi)
		a -= 2 * s_Pi;
	else if (a < -s_Pi)
		a += 2 * s_Pi;
	return a;
}

/// A heading in degrees from 0 to 360, to a tenth
double degrees(double radians)
{
	double d = fmod(radians * 180.0 / s_Pi, 360.0);
	if (d < 0)
		d += 360.0;
	d = floor(d * 10.0 + 0.5) / 10.0;
	return d >= 360.0 ? d - 360.0 : d;
}

CJsonValue posJson(const CVectorD &p)
{
	return CJsonValue::object().set("x", round2(p.x)).set("y", round2(p.y)).set("z", round2(p.z));
}

const char *typeName(const CEntityCL *e)
{
	switch (e->Type)
	{
	case CEntityCL::User: return "user";
	case CEntityCL::Player: return "player";
	case CEntityCL::NPC: return "npc";
	case CEntityCL::Fauna: return "fauna";
	case CEntityCL::ForageSource: return "forage_source";
	default: return "entity";
	}
}

CJsonValue entityJson(uint slot, CEntityCL *e)
{
	CJsonValue o = CJsonValue::object();
	o.set("slot", slot);
	std::string name = e->getDisplayName();
	o.set("name", name);
	// an entity without a name id has no name to wait for
	if (name.empty() && e->getNameId() != 0)
		o.set("name_arrived", false);
	o.set("type", typeName(e));
	o.set("sheet", e->sheetId().toString());
	o.set("dataset", (uint32)e->dataSetId());
	o.set("pos", posJson(e->pos()));
	o.set("heading", degrees(e->frontYaw()));
	o.set("mode", MBEHAV::modeToString(e->mode()));
	if (UserEntity != nullptr)
	{
		CVectorD d = e->pos() - UserEntity->pos();
		d.z = 0;
		o.set("distance", round2(d.norm()));
	}
	return o;
}

// The slots of the entity table: an entity can be in any of them (the frontend gives the slots), while
// CEntityManager::nbEntitiesAllocated is how many are in use, not where they end
uint slotCount()
{
	return (uint)EntitiesMngr.entities().size();
}

/// An entity by slot or name (as the target and move methods take it); nullptr with the call failed
CEntityCL *findEntity(CCtrlCall &call, uint &slot)
{
	const CJsonValue &args = call.getArgs();
	if (args.get("slot").isNumber())
	{
		slot = (uint)args.get("slot").getInt(0);
		CEntityCL *e = slot < slotCount() ? EntitiesMngr.entity(slot) : nullptr;
		if (e == nullptr)
			call.fail(toString("no entity in slot %u", slot));
		return e;
	}
	const std::string &name = args.get("name").getString();
	if (name.empty())
	{
		call.fail("slot or name is required");
		return nullptr;
	}
	// the nearest of that name
	CEntityCL *best = nullptr;
	double bestDistance = 1e30;
	for (uint i = 1; i < slotCount(); ++i)
	{
		CEntityCL *e = EntitiesMngr.entity(i);
		if (e == nullptr || toLowerAscii(e->getDisplayName()) != toLowerAscii(name))
			continue;
		double d = (e->pos() - UserEntity->pos()).norm();
		if (d < bestDistance)
		{
			bestDistance = d;
			best = e;
			slot = i;
		}
	}
	if (best == nullptr)
		call.fail(toString("no entity named '%s' in vision; world.entities lists them", name.c_str()));
	return best;
}

/// Whether the player could select the entity, as a click or /target does: false with the call failed and why
bool checkSelectable(CCtrlCall &call, uint slot, CEntityCL *e)
{
	if (!e->properties().selectable())
	{
		call.fail(toString("the entity in slot %u cannot be selected: a click or /target does not select it either", slot));
		return false;
	}
	if (slot == UserEntity->mount())
	{
		call.fail(toString("slot %u is the character's mount, which a click does not select", slot));
		return false;
	}
	return true;
}

void methodSelf(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	CJsonValue r = CJsonValue::object();
	r.set("name", UserEntity->getDisplayName());
	r.set("slot", (uint32)UserEntity->slot());
	r.set("dataset", (uint32)UserEntity->dataSetId());
	r.set("pos", posJson(UserEntity->pos()));
	r.set("heading", degrees(UserEntity->frontYaw()));
	r.set("mode", MBEHAV::modeToString(UserEntity->mode()));
	r.set("continent", ContinentMngr.getCurrentContinentSelectName());
	r.set("autowalk", UserControls.autowalkState());
	CLFECOMMON::TCLEntityId target = UserEntity->selection();
	CEntityCL *t = target != CLFECOMMON::INVALID_SLOT ? EntitiesMngr.entity(target) : nullptr;
	r.set("target", t != nullptr ? entityJson(target, t) : CJsonValue());
	call.reply(r);
}

void methodEntities(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	const CJsonValue &args = call.getArgs();
	double radius = 100.0;
	if (!readNumber(call, args, "radius", s_MaxCoordinate, radius))
		return;
	std::string type = args.get("type").getString();
	std::string contains = toLowerAscii(args.get("contains").getString());
	uint max = (uint)std::max((sint64)1, std::min((sint64)1000, args.get("max").getInt(100)));
	// distance and slot
	typedef std::pair<double, uint> TFound;
	std::vector<TFound> found;
	for (uint i = 1; i < slotCount(); ++i)
	{
		CEntityCL *e = EntitiesMngr.entity(i);
		if (e == nullptr)
			continue;
		CVectorD d = e->pos() - UserEntity->pos();
		d.z = 0;
		double distance = d.norm();
		if (distance > radius)
			continue;
		if (!type.empty() && type != typeName(e))
			continue;
		if (!contains.empty() && toLowerAscii(e->getDisplayName()).find(contains) == std::string::npos)
			continue;
		found.push_back(std::make_pair(distance, i));
	}
	std::sort(found.begin(), found.end());
	CJsonValue list = CJsonValue::array();
	for (uint i = 0; i < found.size() && i < max; ++i)
		list.push(entityJson(found[i].second, EntitiesMngr.entity(found[i].second)));
	CJsonValue r = CJsonValue::object();
	r.set("entities", list);
	r.set("count", (uint32)found.size());
	r.set("truncated", found.size() > max);
	r.set("self", posJson(UserEntity->pos()));
	call.reply(r);
}

void methodTarget(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	if (call.getArgs().get("none").getBool(false))
	{
		UserEntity->selection(CLFECOMMON::INVALID_SLOT);
		call.reply(CJsonValue::object().set("target", CJsonValue()));
		return;
	}
	uint slot;
	CEntityCL *e = findEntity(call, slot);
	if (e == nullptr || !checkSelectable(call, slot, e))
		return;
	// what a click on the entity does
	UserEntity->selection((CLFECOMMON::TCLEntityId)slot);
	call.reply(CJsonValue::object().set("target", entityJson(slot, e)));
}

class CMoveData;
/// The go_to under way: one at a time, as the character has one move
CMoveData *s_Move = nullptr;

/// A move under way: to an entity through the client's move, or to a point by turning and walking
class CMoveData : public ICtrlCallData
{
public:
	CCtrlCall *Call;
	bool ToEntity;
	uint Slot;
	CVectorD Point;
	double Distance;
	TTime Deadline;
	TTime LastProgress;
	double BestDistance;
	// where the character was at the last poll
	CVectorD LastPos;
	// the autowalk is this move's: a move that never started, or that ended, leaves the player's own alone
	bool Walking;

	CMoveData(CCtrlCall &call)
	    : Call(&call)
	    , ToEntity(false)
	    , Slot(0)
	    , Distance(0)
	    , Deadline(0)
	    , LastProgress(0)
	    , BestDistance(0)
	    , Walking(false)
	{
	}

	~CMoveData()
	{
		if (s_Move == this)
			s_Move = nullptr;
		if (Walking && UserEntity != nullptr)
			UserControls.autowalkState(false);
	}
};

CEntityCL *moveEntity(const CMoveData &data)
{
	return data.Slot < slotCount() ? EntitiesMngr.entity(data.Slot) : nullptr;
}

double moveDistance(const CMoveData &data)
{
	CVectorD target = data.Point;
	if (data.ToEntity)
	{
		CEntityCL *e = moveEntity(data);
		if (e == nullptr)
			return -1;
		target = e->pos();
	}
	CVectorD d = target - UserEntity->pos();
	d.z = 0;
	return d.norm();
}

/// The distance from the character to where the client's move to an entity stops (CUserEntity::applyMotion): the place
/// a melee attacker takes, the size of the entity and the reach of the character away from it
double distanceToStop(CEntityCL *e)
{
	CVectorD toEntity = e->pos() - UserEntity->pos();
	toEntity.z = 0;
	if (toEntity == CVectorD::Null)
		toEntity = UserEntity->front();
	CVectorD stop = e->getAttackerPos(atan2(toEntity.y, toEntity.x), UserEntity->attackRadius() + ClientCfg.AttackDist);
	CVectorD d = stop - UserEntity->pos();
	d.z = 0;
	return d.norm();
}

void turnToward(const CVectorD &point)
{
	CVectorD d = point - UserEntity->pos();
	if (d.x == 0 && d.y == 0)
		return;
	UserEntity->rotate((float)normaliseRadians(atan2(d.y, d.x) - UserEntity->frontYaw()));
}

/// Whether the client's move to an entity is still the one of this go_to: the move goes to the target, so a move the
/// player started to another target is not
bool ownEntityMove(const CMoveData &data)
{
	return data.ToEntity && UserEntity->moveTo() && UserEntity->targetSlot() == data.Slot;
}

/// Answers a go_to and ends its move: only its own, as one the player started since (another target, the autowalk
/// turned off) is theirs
void finishMove(CCtrlCall &call, CMoveData &data, const char *result)
{
	if (ownEntityMove(data))
		UserEntity->resetAnyMoveTo();
	else if (data.Walking && UserControls.autowalkState())
		UserControls.autowalkState(false);
	data.Walking = false;
	if (s_Move == &data)
		s_Move = nullptr;
	CJsonValue r = CJsonValue::object();
	r.set("result", result);
	r.set("pos", posJson(UserEntity->pos()));
	double distance = moveDistance(data);
	if (distance >= 0)
		r.set("distance", round2(distance));
	CEntityCL *e = data.ToEntity ? moveEntity(data) : nullptr;
	if (e != nullptr)
		r.set("distance_to_stop", round2(distanceToStop(e)));
	r.set("elapsed_ms", (sint64)call.getElapsed());
	call.reply(r);
}

void methodGoTo(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	const CJsonValue &args = call.getArgs();
	double distance = 2.0, timeout = 60.0;
	if (!readNumber(call, args, "distance", 1000, distance) || !readNumber(call, args, "timeout_seconds", 1e6, timeout))
		return;
	bool toPoint = !args.get("x").isNull() || !args.get("y").isNull();
	double x = 0, y = 0;
	uint slot = 0;
	if (toPoint)
	{
		if (!readPoint(call, args, x, y))
			return;
	}
	else
	{
		CEntityCL *e = findEntity(call, slot);
		if (e == nullptr)
			return;
		if (e == UserEntity)
		{
			call.fail("that is the character itself");
			return;
		}
		if (!checkSelectable(call, slot, e))
			return;
	}
	// one move at a time, as the character has one: the open one ends here
	if (s_Move != nullptr)
		finishMove(*s_Move->Call, *s_Move, "replaced by another world.go_to");
	CMoveData *data = new CMoveData(call);
	TTime now = CTime::getLocalTime();
	data->Distance = std::max(0.5, distance);
	data->Deadline = now + (TTime)(std::max(1.0, std::min(600.0, timeout)) * 1000.0);
	data->LastProgress = now;
	data->LastPos = UserEntity->pos();
	if (toPoint)
	{
		data->Point = CVectorD(x, y, UserEntity->pos().z);
		turnToward(data->Point);
		UserControls.autowalkState(true);
		data->Walking = true;
	}
	else
	{
		data->ToEntity = true;
		data->Slot = slot;
		// what a click and a double click on the entity do: it becomes the target, and the client's own move goes to
		// the target
		if (UserEntity->selection() != slot)
			UserEntity->selection((CLFECOMMON::TCLEntityId)slot);
		UserEntity->moveTo((CLFECOMMON::TCLEntityId)slot, data->Distance, CUserEntity::None);
	}
	data->BestDistance = moveDistance(*data);
	call.setData(data);
	s_Move = data;
}

void pollGoTo(CCtrlCall &call)
{
	CMoveData &data = *static_cast<CMoveData *>(call.getData());
	if (UserEntity == nullptr || !FarTP.isIngame())
	{
		call.fail("the character left the game during the move");
		return;
	}
	// a loading screen, or a jump between two polls that no run makes, is a teleport: the move asked for has no
	// meaning at the new place
	CVectorD moved = UserEntity->pos() - data.LastPos;
	moved.z = 0;
	if (!isInterfaceAvailable() || moved.norm() > s_TeleportJump)
	{
		finishMove(call, data, "teleported");
		return;
	}
	data.LastPos = UserEntity->pos();
	double distance = moveDistance(data);
	if (distance < 0)
	{
		finishMove(call, data, "the entity left the vision");
		return;
	}
	TTime now = CTime::getLocalTime();
	if (data.ToEntity)
	{
		// the client's move ends by itself where it stops for that entity (distanceToStop), or held up within distance
		// of it; a movement key or another action ends it too
		if (!UserEntity->moveTo())
		{
			finishMove(call, data, distanceToStop(moveEntity(data)) <= data.Distance + 0.3 ? "arrived" : "interrupted");
			return;
		}
		if (UserEntity->targetSlot() != data.Slot)
		{
			finishMove(call, data, "target changed");
			return;
		}
	}
	else
	{
		if (!UserControls.autowalkState())
		{
			finishMove(call, data, "interrupted");
			return;
		}
		if (distance <= data.Distance + 0.3)
		{
			finishMove(call, data, "arrived");
			return;
		}
	}
	// a move that gains nothing for three seconds is stuck against something
	if (distance < data.BestDistance - 0.2)
	{
		data.BestDistance = distance;
		data.LastProgress = now;
	}
	else if (now - data.LastProgress > 3000)
	{
		finishMove(call, data, "blocked");
		return;
	}
	if (now >= data.Deadline)
	{
		finishMove(call, data, "timed out");
		return;
	}
	if (!data.ToEntity)
		turnToward(data.Point);
}

void cancelGoTo(CCtrlCall &call)
{
	// the autowalk of a move to a point stops with the data
	CMoveData *data = static_cast<CMoveData *>(call.getData());
	if (data != nullptr && UserEntity != nullptr && ownEntityMove(*data))
		UserEntity->resetAnyMoveTo();
}

void methodStop(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	bool ended = s_Move != nullptr;
	if (ended)
		finishMove(*s_Move->Call, *s_Move, "stopped by world.stop");
	UserEntity->resetAnyMoveTo();
	UserControls.autowalkState(false);
	call.reply(CJsonValue::object().set("pos", posJson(UserEntity->pos())).set("ended_go_to", ended));
}

void methodFace(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	const CJsonValue &args = call.getArgs();
	if (!args.get("heading").isNull())
	{
		double heading = 0;
		if (!readNumber(call, args, "heading", s_MaxDegrees, heading))
			return;
		double wanted = heading * s_Pi / 180.0;
		CVectorD p = UserEntity->pos();
		turnToward(CVectorD(p.x + cos(wanted) * 10.0, p.y + sin(wanted) * 10.0, p.z));
	}
	else if (!args.get("x").isNull() || !args.get("y").isNull())
	{
		double x = 0, y = 0;
		if (!readPoint(call, args, x, y))
			return;
		turnToward(CVectorD(x, y, 0));
	}
	else
	{
		uint slot;
		CEntityCL *e = findEntity(call, slot);
		if (e == nullptr)
			return;
		turnToward(e->pos());
	}
	// the camera looks where the character does
	if (!args.get("keep_camera").getBool(false))
		UserControls.resetCameraDeltaYaw();
	call.reply(CJsonValue::object().set("heading", degrees(UserEntity->frontYaw())));
}

CJsonValue cameraJson()
{
	CJsonValue r = CJsonValue::object();
	r.set("heading", degrees(UserEntity->frontYaw() + UserControls.getCameraDeltaYaw()));
	r.set("camera_offset", round2(normaliseRadians(UserControls.getCameraDeltaYaw()) * 180.0 / s_Pi));
	r.set("pitch", round2(UserEntity->getHeadPitch() * 180.0 / s_Pi));
	r.set("distance", round2(ClientCfg.CameraDistance));
	r.set("first_person", UserEntity->viewMode() == CUserEntity::FirstPV);
	r.set("position", posJson(CVectorD(View.currentViewPos())));
	return r;
}

void methodCameraGet(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	call.reply(cameraJson());
}

void methodCameraSet(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	const CJsonValue &args = call.getArgs();
	// everything is checked before anything changes
	bool hasPitch = !args.get("pitch").isNull(), hasDistance = !args.get("distance").isNull(), hasOffset = !args.get("offset").isNull(),
	     hasLookAt = !args.get("look_at").isNull();
	double pitch = 0, distance = 0, offset = 0, lookX = 0, lookY = 0;
	if (!readNumber(call, args, "pitch", 90, pitch) || !readNumber(call, args, "distance", 1000, distance) || !readNumber(call, args, "offset", s_MaxDegrees, offset))
		return;
	if (hasDistance && distance < 0)
	{
		call.fail("distance is from 0 to 1000 metres");
		return;
	}
	// as the mouse wheel and /camera: inside a building that keeps the view from the eyes the distance does not change
	if (hasDistance && UserEntity->forceIndoorFPV())
	{
		call.fail("the character is inside a building that keeps the view from the eyes: the distance does not change there, as with the mouse wheel; leave distance out");
		return;
	}
	if (hasLookAt)
	{
		if (!args.get("look_at").isObject())
		{
			call.fail("look_at is an object with x and y");
			return;
		}
		if (!readPoint(call, args.get("look_at"), lookX, lookY))
			return;
	}
	if (args.get("reset").getBool(false))
	{
		UserControls.resetCameraDeltaYaw();
		UserEntity->setHeadPitch(0);
	}
	if (hasPitch)
		UserEntity->setHeadPitch(pitch * s_Pi / 180.0);
	// as /camera: never below the shortest view from behind, so that the same arguments give the same view; the
	// longest is the one of the place (the view clamps to it)
	if (hasDistance)
		View.cameraDistance((float)std::max(distance, (double)ClientCfg.CameraDistMin));
	// the offset is brought to a half turn either way first: the yaw the camera adds is kept as it is given
	if (hasOffset)
	{
		UserControls.resetCameraDeltaYaw();
		UserControls.appendCameraDeltaYaw((float)normaliseRadians(offset * s_Pi / 180.0));
	}
	if (hasLookAt)
	{
		UserControls.resetCameraDeltaYaw();
		turnToward(CVectorD(lookX, lookY, 0));
	}
	call.reply(cameraJson());
}

// *** the collision view ***

const char *const s_EdgeNames[] = { "wall", "surface_boundary", "cell_border", "water_edge", "exterior_edge", "door" };
const char *const s_EdgeColors[] = { "#d22", "#3a3", "#888", "#28c", "#c80", "#e0e" };

void methodCollision(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	if (GR == nullptr)
	{
		call.fail("no collision data: no continent is loaded");
		return;
	}
	const CJsonValue &args = call.getArgs();
	double radius = 30.0;
	if (!readNumber(call, args, "radius", s_MaxCoordinate, radius))
		return;
	radius = std::max(2.0, std::min(200.0, radius));
	CVectorD centre = UserEntity->pos();
	if (!args.get("x").isNull() || !args.get("y").isNull())
	{
		double x = 0, y = 0;
		if (!readPoint(call, args, x, y))
			return;
		centre = CVectorD(x, y, centre.z);
	}
	std::string format = args.get("format").getString("svg");
	if (format != "svg" && format != "json")
	{
		call.fail("format is svg or json");
		return;
	}
	bool asSvg = format == "svg";
	// each of the edges, the primitives and the entities: at most this many
	uint max = (uint)std::max((sint64)1, std::min((sint64)50000, args.get("max").getInt(5000)));
	bool wantBorders = true, wantPrimitives = true, wantEntities = true;
	const CJsonValue &layers = args.get("layers");
	if (!layers.isNull() && !layers.isArray())
	{
		call.fail("layers is a list: borders, primitives, entities");
		return;
	}
	if (layers.size() > 0)
	{
		wantBorders = wantPrimitives = wantEntities = false;
		for (uint i = 0; i < layers.size(); ++i)
		{
			const std::string &l = layers.at(i).getString();
			if (l != "borders" && l != "primitives" && l != "entities")
			{
				call.fail(toString("layers are borders, primitives and entities: '%s' is none of them", l.c_str()));
				return;
			}
			wantBorders = wantBorders || l == "borders";
			wantPrimitives = wantPrimitives || l == "primitives";
			wantEntities = wantEntities || l == "entities";
		}
	}
	bool typeWanted[6] = { true, true, true, true, true, true };
	const CJsonValue &types = args.get("types");
	if (!types.isNull() && !types.isArray())
	{
		call.fail("types is a list of edge types: numbers 0 to 5 or their names");
		return;
	}
	if (types.size() > 0)
	{
		for (uint t = 0; t < 6; ++t)
			typeWanted[t] = false;
		for (uint i = 0; i < types.size(); ++i)
		{
			const CJsonValue &type = types.at(i);
			sint64 t = -1;
			if (type.isNumber() && floor(type.getNumber()) == type.getNumber())
				t = type.getInt(-1);
			for (uint k = 0; k < 6 && t < 0; ++k)
				if (type.getString() == s_EdgeNames[k])
					t = k;
			if (t < 0 || t >= 6)
			{
				call.fail(toString("type %s is none of the edge types: 0 to 5, or wall, surface_boundary, cell_border, water_edge, exterior_edge, door", type.toString().c_str()));
				return;
			}
			typeWanted[t] = true;
		}
	}
	uint omittedEdges = 0, omittedPrimitives = 0, omittedEntities = 0;

	// the borders of the retrievers that are loaded around the centre
	typedef std::pair<CLine, uint8> TEdge;
	std::vector<TEdge> edges;
	if (wantBorders)
	{
		CAABBox box;
		box.setCenter(CVector((float)centre.x, (float)centre.y, (float)centre.z));
		box.setHalfSize(CVector((float)radius, (float)radius, 50.f));
		GR->getBorders(box, edges);
	}
	uint counts[6] = { 0, 0, 0, 0, 0, 0 };
	uint shown = 0;
	std::string paths[6];
	CJsonValue lines = CJsonValue::array();
	uint heightsKnown = 0, heightsUnknown = 0;
	for (uint i = 0; i < edges.size(); ++i)
	{
		uint8 t = edges[i].second;
		if (t >= 6 || !typeWanted[t])
			continue;
		const CLine &l = edges[i].first;
		// both ends outside the square: not shown. The exterior mesh edges and the doors come for the whole retriever,
		// so most of them go here.
		if ((fabs(l.V0.x - centre.x) > radius && fabs(l.V1.x - centre.x) > radius && (l.V0.x - centre.x) * (l.V1.x - centre.x) > 0)
		    || (fabs(l.V0.y - centre.y) > radius && fabs(l.V1.y - centre.y) > radius && (l.V0.y - centre.y) * (l.V1.y - centre.y) > 0))
			continue;
		if (shown >= max)
		{
			++omittedEdges;
			continue;
		}
		++shown;
		++counts[t];
		// SVG has y down: north is up in the picture
		if (asSvg)
		{
			paths[t] += toString("M%.2f %.2fL%.2f %.2f", l.V0.x, -l.V0.y, l.V1.x, -l.V1.y);
			paths[t] += ' ';
		}
		// where the retriever has no ordered chains, both heights of a chain edge are the centre of the box; the exterior
		// mesh edges and the doors always have their own
		bool known = t >= 4 || !(fabs(l.V0.z - centre.z) < 0.001 && fabs(l.V1.z - centre.z) < 0.001);
		if (known)
			++heightsKnown;
		else
			++heightsUnknown;
		if (!asSvg)
		{
			CJsonValue o = CJsonValue::object();
			o.set("type", s_EdgeNames[t]);
			CJsonValue from = CJsonValue::array();
			from.push(CJsonValue(round2(l.V0.x)));
			from.push(CJsonValue(round2(l.V0.y)));
			o.set("from", from);
			CJsonValue to = CJsonValue::array();
			to.push(CJsonValue(round2(l.V1.x)));
			to.push(CJsonValue(round2(l.V1.y)));
			o.set("to", to);
			if (known)
			{
				CJsonValue z = CJsonValue::array();
				z.push(CJsonValue(round2(l.V0.z)));
				z.push(CJsonValue(round2(l.V1.z)));
				o.set("z", z);
			}
			lines.push(o);
		}
	}

	std::string shapes;
	CJsonValue primitives = CJsonValue::array();
	if (wantPrimitives && PACS != nullptr)
	{
		std::vector<const NLPACS::UMovePrimitive *> prims;
		PACS->getPrimitives(prims);
		for (uint i = 0; i < prims.size(); ++i)
		{
			const NLPACS::UMovePrimitive *prim = prims[i];
			uint8 wi = prim->getFirstWorldImageV();
			CVectorD p = prim->getFinalPosition(wi);
			if (fabs(p.x - centre.x) > radius || fabs(p.y - centre.y) > radius)
				continue;
			if (primitives.size() >= max)
			{
				++omittedPrimitives;
				continue;
			}
			const char *kind = !prim->isCollisionable() ? "trigger" : (prim->getReactionType() == NLPACS::UMovePrimitive::DoNothing ? "static" : "moving");
			const char *color = !prim->isCollisionable() ? "#bbb" : (prim->getReactionType() == NLPACS::UMovePrimitive::DoNothing ? "#d22" : "#cc0");
			CJsonValue o = CJsonValue::object();
			o.set("kind", kind);
			o.set("pos", posJson(p));
			if (prim->getPrimitiveType() == NLPACS::UMovePrimitive::_2DOrientedBox)
			{
				float width, depth;
				prim->getSize(width, depth);
				double angle = prim->getOrientation(wi);
				o.set("box", CJsonValue::object().set("width", round2(width)).set("depth", round2(depth)).set("angle", degrees(angle)));
				shapes += toString("<rect x='%.2f' y='%.2f' width='%.2f' height='%.2f' transform='rotate(%.1f %.2f %.2f)' fill='none' stroke='%s' stroke-width='0.1'/>", p.x - width / 2,
				    -p.y - depth / 2, width, depth, -angle * 180.0 / s_Pi, p.x, -p.y, color);
			}
			else
			{
				o.set("radius", round2(prim->getRadius()));
				shapes += toString("<circle cx='%.2f' cy='%.2f' r='%.2f' fill='none' stroke='%s' stroke-width='0.1'/>", p.x, -p.y, prim->getRadius(), color);
			}
			primitives.push(o);
		}
	}

	CJsonValue entities = CJsonValue::array();
	if (wantEntities)
	{
		for (uint i = 1; i < slotCount(); ++i)
		{
			CEntityCL *e = EntitiesMngr.entity(i);
			if (e == nullptr || fabs(e->pos().x - centre.x) > radius || fabs(e->pos().y - centre.y) > radius)
				continue;
			if (entities.size() >= max)
			{
				++omittedEntities;
				continue;
			}
			std::string name = e->getDisplayName();
			// a name is text of the picture: its markup characters are escaped
			std::string label = name.empty() ? toString("#%u", i) : name;
			std::string escaped;
			for (uint k = 0; k < label.size(); ++k)
			{
				char c = label[k];
				escaped += c == '<' ? std::string("&lt;") : c == '>' ? std::string("&gt;")
				    : c == '&'                                       ? std::string("&amp;")
				    : c == '\''                                      ? std::string("&apos;")
				                                                     : std::string(1, c);
			}
			shapes += toString("<circle cx='%.2f' cy='%.2f' r='0.4' fill='#06c'/><text x='%.2f' y='%.2f' font-size='%.2f' fill='#036'>", e->pos().x, -e->pos().y, e->pos().x + 0.6, -e->pos().y,
			    radius / 40.0);
			shapes += escaped + "</text>";
			entities.push(CJsonValue::object().set("slot", i).set("name", name).set("type", typeName(e)).set("pos", posJson(e->pos())));
		}
	}

	// the character as a triangle along its heading
	CVectorD self = UserEntity->pos();
	double yaw = UserEntity->frontYaw();
	shapes += toString("<path d='M%.2f %.2fL%.2f %.2fL%.2f %.2fZ' fill='#000'/>", self.x + cos(yaw) * 1.2, -(self.y + sin(yaw) * 1.2), self.x + cos(yaw + 2.5) * 0.7,
	    -(self.y + sin(yaw + 2.5) * 0.7), self.x + cos(yaw - 2.5) * 0.7, -(self.y + sin(yaw - 2.5) * 0.7));

	CJsonValue r = CJsonValue::object();
	r.set("centre", posJson(centre));
	r.set("radius", radius);
	CJsonValue countJson = CJsonValue::object();
	for (uint t = 0; t < 6; ++t)
		countJson.set(s_EdgeNames[t], counts[t]);
	r.set("edges", countJson);
	r.set("heights", toString("%u lines with real heights, %u without (no ordered chains loaded there; exterior mesh edges and doors always have theirs)", heightsKnown, heightsUnknown));
	r.set("covers", "the retrievers loaded around the centre: the streamed zones, not the whole radius far from the character");
	r.set("primitives", primitives);
	r.set("entities", entities);
	bool truncated = omittedEdges > 0 || omittedPrimitives > 0 || omittedEntities > 0;
	r.set("truncated", truncated);
	if (truncated)
		r.set("omitted", CJsonValue::object().set("edges", omittedEdges).set("primitives", omittedPrimitives).set("entities", omittedEntities).set("max", max));
	if (!asSvg)
	{
		r.set("lines", lines);
		call.reply(r);
		return;
	}
	std::string svg = toString("<svg xmlns='http://www.w3.org/2000/svg' viewBox='%.2f %.2f %.2f %.2f'>", centre.x - radius, -centre.y - radius, radius * 2, radius * 2);
	svg += "<!-- world metres, x east, north up (y is negated); ";
	for (uint t = 0; t < 6; ++t)
		svg += toString("%s %s%s", s_EdgeNames[t], s_EdgeColors[t], t < 5 ? ", " : "");
	svg += " -->";
	for (uint t = 0; t < 6; ++t)
		if (!paths[t].empty())
		{
			// the path data is long: it does not go through a format, which caps what it writes
			svg += toString("<path class='%s' stroke='%s' stroke-width='%.2f' fill='none' d='", s_EdgeNames[t], s_EdgeColors[t], radius / 300.0);
			svg += paths[t];
			svg += "'/>";
		}
	svg += shapes;
	svg += "</svg>";
	r.set("svg", call.attach("image/svg+xml", svg));
	call.reply(r);
}

} // anonymous namespace

void registerWorldMethods(CCtrlEndpoint &endpoint)
{
	endpoint.addMethod(new CCtrlFunctionMethod("world.self", "The character: name, slot, dataset id, position, heading, mode, continent, autowalk, and its target.", CCtrlSchema(), true, methodSelf));
	endpoint.addMethod(new CCtrlFunctionMethod("world.entities",
	    "The entities in vision, nearest first: slot, name (name_arrived false while its string is on its way), type (player, npc, fauna, forage_source), sheet, position, distance, heading, mode.",
	    CCtrlSchema()
	        .number("radius", "Metres around the character (100 by default).")
	        .choice("type", "player|npc|fauna|forage_source|entity", "Only this type.")
	        .str("contains", "Only names containing this.")
	        .integer("max", "At most this many (100 by default)."),
	    true, methodEntities));
	endpoint.addMethod(new CCtrlFunctionMethod("world.target",
	    "Targets an entity by slot or name (the nearest of that name), as a click on it does: only one a player can select, not the character's mount. none clears the target.",
	    CCtrlSchema().integer("slot", "The slot.").str("name", "Or the name.").boolean("none", "Clear the target."), false, methodTarget));
	endpoint.addMethod(new CCtrlFunctionMethod("world.go_to",
	    "Moves the character to an entity (slot or name) with the client's own move, or to a point (x, y) by turning toward it and walking: in a straight line, sliding along what it meets, with no path around "
	    "obstacles. The entity becomes the target first, as a click makes it, and the client's move stops where it puts the character for that entity: its size and the character's reach away "
	    "(distance_to_stop in the answer), not nearer. One move at a time: a second go_to or world.stop ends the first. Answers with the final position and result: arrived, blocked (no progress for "
	    "3 s), timed out, interrupted (a key or another action ended the move), target changed, teleported, the entity left the vision, replaced by another world.go_to, stopped by world.stop.",
	    CCtrlSchema()
	        .integer("slot", "The entity's slot.")
	        .str("name", "Or its name.")
	        .number("x", "Or a point: x in metres.")
	        .number("y", "And y.")
	        .number("distance", "To a point: how close to stop (2 m by default). To an entity: how near its stop place a move that something holds up counts as there.")
	        .number("timeout_seconds", "How long at most (60 by default, 600 at most)."),
	    false, methodGoTo, pollGoTo, cancelGoTo));
	endpoint.addMethod(new CCtrlFunctionMethod("world.stop", "Stops a move and the autowalk, and ends a world.go_to under way.", CCtrlSchema(), false, methodStop));
	endpoint.addMethod(new CCtrlFunctionMethod("world.face",
	    "Turns the character toward an entity (slot or name), a point (x, y) or a heading in degrees, and the camera with it unless keep_camera.",
	    CCtrlSchema()
	        .integer("slot", "The entity's slot.")
	        .str("name", "Or its name.")
	        .number("x", "Or a point.")
	        .number("y", "Its y.")
	        .number("heading", "Or a heading, degrees counterclockwise from east.")
	        .boolean("keep_camera", "Leave the camera where it looks."),
	    false, methodFace));
	endpoint.addMethod(new CCtrlFunctionMethod("camera.get", "The camera: heading, its offset from the character's heading, pitch, distance, first person, position.", CCtrlSchema(), true,
	    methodCameraGet));
	endpoint.addMethod(new CCtrlFunctionMethod("camera.set",
	    "Sets the camera as the player moves it: pitch (degrees, up positive), distance (metres), offset from the character's heading (degrees), look_at {x, y} (turns the character), or reset "
	    "(behind the character, level: for a camera that started pitched up). The distance follows the mouse wheel's rules: the view from behind, from the shortest the configuration allows "
	    "to the longest the place allows, and refused inside a building that keeps the view from the eyes.",
	    CCtrlSchema()
	        .number("pitch", "Degrees, -90 to 90 (the client stops just short of either).")
	        .number("distance", "Metres.")
	        .number("offset", "Degrees from the character's heading.")
	        .object("look_at", "x, y: a point to look at.")
	        .boolean("reset", "Behind the character, level."),
	    false, methodCameraSet));
	endpoint.addMethod(new CCtrlFunctionMethod("world.collision",
	    "A top-down picture of the collision around the character, what Ctrl+Alt+O draws, as SVG in world metres (north up): walls (red), boundaries between walkable surfaces (green), retriever cell borders (gray), "
	    "water edges (blue), exterior mesh edges (orange), doors (magenta); move primitives (static red, moving yellow, triggers gray); entities with names; the character as a triangle along its heading. "
	    "layers and types filter it; format json gives the lines with their heights where known. Only loaded retrievers are covered; exterior mesh edges and doors come for a whole retriever "
	    "and are cut to the square here. max bounds each of edges, primitives and entities, and truncated with omitted says what was left out.",
	    CCtrlSchema()
	        .number("radius", "Metres around the centre (30 by default, 200 at most).")
	        .number("x", "The centre, x (the character by default).")
	        .number("y", "Its y.")
	        .strings("layers", "borders, primitives, entities (all by default).")
	        .any("types", "Edge types to keep: numbers 0-5 or names (wall, surface_boundary, cell_border, water_edge, exterior_edge, door).")
	        .choice("format", "svg|json", "svg (the default) or json.")
	        .integer("max", "At most this many edges, and as many primitives and entities (5000 by default, 50000 at most)."),
	    true, methodCollision));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_world.cpp */
