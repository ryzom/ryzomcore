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

double round2(double v)
{
	return floor(v * 100.0 + 0.5) / 100.0;
}

double degrees(double radians)
{
	double d = radians * 180.0 / s_Pi;
	while (d < 0)
		d += 360.0;
	while (d >= 360.0)
		d -= 360.0;
	return floor(d * 10.0 + 0.5) / 10.0;
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
	if (name.empty())
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

/// An entity by slot or name (as the target and move methods take it); nullptr with the call failed
CEntityCL *findEntity(CCtrlCall &call, uint &slot)
{
	const CJsonValue &args = call.getArgs();
	if (args.get("slot").isNumber())
	{
		slot = (uint)args.get("slot").getInt(0);
		CEntityCL *e = slot < EntitiesMngr.nbEntitiesAllocated() ? EntitiesMngr.entity(slot) : nullptr;
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
	for (uint i = 1; i < EntitiesMngr.nbEntitiesAllocated(); ++i)
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
	double radius = args.get("radius").getNumber(100.0);
	std::string type = args.get("type").getString();
	std::string contains = toLowerAscii(args.get("contains").getString());
	uint max = (uint)std::max((sint64)1, std::min((sint64)1000, args.get("max").getInt(100)));
	std::vector<std::pair<double, uint>> found;
	for (uint i = 1; i < EntitiesMngr.nbEntitiesAllocated(); ++i)
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
	if (e == nullptr)
		return;
	// what a click on the entity does
	UserEntity->selection((CLFECOMMON::TCLEntityId)slot);
	call.reply(CJsonValue::object().set("target", entityJson(slot, e)));
}

/// A move under way: to an entity through the client's move, or to a point by turning and walking
class CMoveData : public ICtrlCallData
{
public:
	bool ToEntity;
	uint Slot;
	CVectorD Point;
	double Distance;
	TTime Deadline;
	TTime LastProgress;
	double BestDistance;
	// the autowalk is this move's: a move that never started leaves the player's own alone
	bool Walking;

	CMoveData()
	    : ToEntity(false)
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
		if (Walking && UserEntity != nullptr)
			UserControls.autowalkState(false);
	}
};

double moveDistance(const CMoveData &data)
{
	CVectorD target = data.Point;
	if (data.ToEntity)
	{
		CEntityCL *e = data.Slot < EntitiesMngr.nbEntitiesAllocated() ? EntitiesMngr.entity(data.Slot) : nullptr;
		if (e == nullptr)
			return -1;
		target = e->pos();
	}
	CVectorD d = target - UserEntity->pos();
	d.z = 0;
	return d.norm();
}

void turnToward(const CVectorD &point)
{
	CVectorD d = point - UserEntity->pos();
	if (d.x == 0 && d.y == 0)
		return;
	double wanted = atan2(d.y, d.x);
	double delta = wanted - UserEntity->frontYaw();
	while (delta > s_Pi)
		delta -= 2 * s_Pi;
	while (delta < -s_Pi)
		delta += 2 * s_Pi;
	UserEntity->rotate((float)delta);
}

void methodGoTo(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	const CJsonValue &args = call.getArgs();
	CMoveData *data = new CMoveData();
	data->Distance = std::max(0.5, args.get("distance").getNumber(2.0));
	data->Deadline = CTime::getLocalTime() + (TTime)(std::max(1.0, std::min(600.0, args.get("timeout_seconds").getNumber(60.0))) * 1000.0);
	data->LastProgress = CTime::getLocalTime();
	if (args.get("x").isNumber() && args.get("y").isNumber())
	{
		data->ToEntity = false;
		data->Slot = 0;
		data->Point = CVectorD(args.get("x").getNumber(), args.get("y").getNumber(), UserEntity->pos().z);
		turnToward(data->Point);
		UserControls.autowalkState(true);
		data->Walking = true;
	}
	else
	{
		uint slot;
		if (findEntity(call, slot) == nullptr)
		{
			delete data;
			return;
		}
		data->ToEntity = true;
		data->Slot = slot;
		// the client's own move: what a double click on the entity does
		UserEntity->moveTo((CLFECOMMON::TCLEntityId)slot, data->Distance, CUserEntity::None);
	}
	data->BestDistance = moveDistance(*data);
	call.setData(data);
}

void finishMove(CCtrlCall &call, CMoveData &data, const char *result)
{
	if (data.ToEntity)
		UserEntity->resetAnyMoveTo();
	else
		UserControls.autowalkState(false);
	CJsonValue r = CJsonValue::object();
	r.set("result", result);
	r.set("pos", posJson(UserEntity->pos()));
	r.set("distance", round2(std::max(0.0, moveDistance(data))));
	r.set("elapsed_ms", (sint64)call.getElapsed());
	call.reply(r);
}

void pollGoTo(CCtrlCall &call)
{
	CMoveData &data = *static_cast<CMoveData *>(call.getData());
	if (UserEntity == nullptr || !FarTP.isIngame())
	{
		call.fail("the character left the game during the move");
		return;
	}
	double distance = moveDistance(data);
	if (distance < 0)
	{
		finishMove(call, data, "the entity left the vision");
		return;
	}
	TTime now = CTime::getLocalTime();
	if (distance <= data.Distance + 0.3)
	{
		finishMove(call, data, "arrived");
		return;
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
	CMoveData *data = static_cast<CMoveData *>(call.getData());
	if (data != nullptr && data->ToEntity && UserEntity != nullptr)
		UserEntity->resetAnyMoveTo();
}

void methodStop(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	UserEntity->resetAnyMoveTo();
	UserControls.autowalkState(false);
	call.reply(CJsonValue::object().set("pos", posJson(UserEntity->pos())));
}

void methodFace(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	const CJsonValue &args = call.getArgs();
	if (args.get("heading").isNumber())
	{
		double wanted = args.get("heading").getNumber() * s_Pi / 180.0;
		CVectorD p = UserEntity->pos();
		turnToward(CVectorD(p.x + cos(wanted) * 10.0, p.y + sin(wanted) * 10.0, p.z));
	}
	else if (args.get("x").isNumber() && args.get("y").isNumber())
	{
		turnToward(CVectorD(args.get("x").getNumber(), args.get("y").getNumber(), 0));
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
	r.set("camera_offset", round2(UserControls.getCameraDeltaYaw() * 180.0 / s_Pi));
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
	if (args.get("reset").getBool(false))
	{
		UserControls.resetCameraDeltaYaw();
		UserEntity->setHeadPitch(0);
	}
	if (args.get("pitch").isNumber())
		UserEntity->setHeadPitch(args.get("pitch").getNumber() * s_Pi / 180.0);
	if (args.get("distance").isNumber())
		View.cameraDistance((float)args.get("distance").getNumber());
	if (args.get("offset").isNumber())
	{
		UserControls.resetCameraDeltaYaw();
		UserControls.appendCameraDeltaYaw((float)(args.get("offset").getNumber() * s_Pi / 180.0));
	}
	if (args.get("look_at").isObject())
	{
		const CJsonValue &at = args.get("look_at");
		UserControls.resetCameraDeltaYaw();
		turnToward(CVectorD(at.get("x").getNumber(), at.get("y").getNumber(), 0));
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
	double radius = std::max(2.0, std::min(200.0, args.get("radius").getNumber(30.0)));
	CVectorD centre = UserEntity->pos();
	if (args.get("x").isNumber() && args.get("y").isNumber())
		centre = CVectorD(args.get("x").getNumber(), args.get("y").getNumber(), centre.z);
	std::string format = args.get("format").getString("svg");
	bool wantBorders = true, wantPrimitives = true, wantEntities = true;
	const CJsonValue &layers = args.get("layers");
	if (layers.size() > 0)
	{
		wantBorders = wantPrimitives = wantEntities = false;
		for (uint i = 0; i < layers.size(); ++i)
		{
			const std::string &l = layers.at(i).getString();
			wantBorders = wantBorders || l == "borders";
			wantPrimitives = wantPrimitives || l == "primitives";
			wantEntities = wantEntities || l == "entities";
		}
	}
	bool typeWanted[6] = { true, true, true, true, true, true };
	const CJsonValue &types = args.get("types");
	if (types.size() > 0)
	{
		for (uint t = 0; t < 6; ++t)
			typeWanted[t] = false;
		for (uint i = 0; i < types.size(); ++i)
		{
			sint64 t = types.at(i).isNumber() ? types.at(i).getInt(-1) : -1;
			for (uint k = 0; k < 6 && t < 0; ++k)
				if (types.at(i).getString() == s_EdgeNames[k])
					t = k;
			if (t >= 0 && t < 6)
				typeWanted[t] = true;
		}
	}

	// the borders of the retrievers that are loaded around the centre
	std::vector<std::pair<CLine, uint8>> edges;
	if (wantBorders)
	{
		CAABBox box;
		box.setCenter(CVector((float)centre.x, (float)centre.y, (float)centre.z));
		box.setHalfSize(CVector((float)radius, (float)radius, 50.f));
		GR->getBorders(box, edges);
	}
	uint counts[6] = { 0, 0, 0, 0, 0, 0 };
	std::string paths[6];
	CJsonValue lines = CJsonValue::array();
	uint heightsKnown = 0, heightsUnknown = 0;
	for (uint i = 0; i < edges.size(); ++i)
	{
		uint8 t = edges[i].second;
		if (t >= 6 || !typeWanted[t])
			continue;
		const CLine &l = edges[i].first;
		// both ends outside the square: not shown
		if ((fabs(l.V0.x - centre.x) > radius && fabs(l.V1.x - centre.x) > radius && (l.V0.x - centre.x) * (l.V1.x - centre.x) > 0)
		    || (fabs(l.V0.y - centre.y) > radius && fabs(l.V1.y - centre.y) > radius && (l.V0.y - centre.y) * (l.V1.y - centre.y) > 0))
			continue;
		++counts[t];
		// SVG has y down: north is up in the picture
		paths[t] += toString("M%.2f %.2fL%.2f %.2f", l.V0.x, -l.V0.y, l.V1.x, -l.V1.y);
		paths[t] += ' ';
		// where the retriever has no ordered chains, both heights are the centre of the box
		bool known = !(fabs(l.V0.z - centre.z) < 0.001 && fabs(l.V1.z - centre.z) < 0.001);
		if (known)
			++heightsKnown;
		else
			++heightsUnknown;
		if (format == "json")
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
		for (uint i = 1; i < EntitiesMngr.nbEntitiesAllocated(); ++i)
		{
			CEntityCL *e = EntitiesMngr.entity(i);
			if (e == nullptr || fabs(e->pos().x - centre.x) > radius || fabs(e->pos().y - centre.y) > radius)
				continue;
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
	r.set("heights", toString("%u lines with real heights, %u without (no ordered chains loaded there)", heightsKnown, heightsUnknown));
	r.set("covers", "the retrievers loaded around the centre: the streamed zones, not the whole radius far from the character");
	r.set("primitives", primitives);
	r.set("entities", entities);
	if (format == "json")
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
	endpoint.addMethod(new CCtrlFunctionMethod("world.target", "Targets an entity by slot or name (the nearest of that name), as a click on it does; none clears the target.",
	    CCtrlSchema().integer("slot", "The slot.").str("name", "Or the name.").boolean("none", "Clear the target."), false, methodTarget));
	endpoint.addMethod(new CCtrlFunctionMethod("world.go_to",
	    "Moves the character to an entity (slot or name) with the client's own move, or to a point (x, y) by turning toward it and walking: in a straight line, sliding along what it meets, with no path around "
	    "obstacles. Answers when it arrived within distance, is blocked (no progress for 3 s), or timed out; with the final position.",
	    CCtrlSchema()
	        .integer("slot", "The entity's slot.")
	        .str("name", "Or its name.")
	        .number("x", "Or a point: x in metres.")
	        .number("y", "And y.")
	        .number("distance", "How close to stop (2 m by default).")
	        .number("timeout_seconds", "How long at most (60 by default)."),
	    false, methodGoTo, pollGoTo, cancelGoTo));
	endpoint.addMethod(new CCtrlFunctionMethod("world.stop", "Stops a move and the autowalk.", CCtrlSchema(), false, methodStop));
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
	    "(behind the character, level: for a camera that started pitched up).",
	    CCtrlSchema()
	        .number("pitch", "Degrees.")
	        .number("distance", "Metres.")
	        .number("offset", "Degrees from the character's heading.")
	        .object("look_at", "x, y: a point to look at.")
	        .boolean("reset", "Behind the character, level."),
	    false, methodCameraSet));
	endpoint.addMethod(new CCtrlFunctionMethod("world.collision",
	    "A top-down picture of the collision around the character, what Ctrl+Alt+O draws, as SVG in world metres (north up): walls (red), boundaries between walkable surfaces (green), retriever cell borders (gray), "
	    "water edges (blue), exterior mesh edges (orange), doors (magenta); move primitives (static red, moving yellow, triggers gray); entities with names; the character as a triangle along its heading. "
	    "layers and types filter it; format json gives the lines with their heights where known. Only loaded retrievers are covered.",
	    CCtrlSchema()
	        .number("radius", "Metres around the centre (30 by default, 200 at most).")
	        .number("x", "The centre, x (the character by default).")
	        .number("y", "Its y.")
	        .strings("layers", "borders, primitives, entities (all by default).")
	        .any("types", "Edge types to keep: numbers 0-5 or names (wall, surface_boundary, cell_border, water_edge, exterior_edge, door).")
	        .choice("format", "svg|json", "svg (the default) or json."),
	    true, methodCollision));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_world.cpp */
