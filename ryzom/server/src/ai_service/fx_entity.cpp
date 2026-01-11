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

#include "fx_entity.h"
#include "ai_instance.h"
#include "game_share/ryzom_entity_id.h"
#include "ai_grp_npc.h"
#include "server_share/r2_variables.h"
#include "server_share/r2_vision.h"

using namespace MULTI_LINE_FORMATER;

CFxEntity::CFxEntity(CAIPos const& pos, NLMISC::CSheetId const& sheet)
: _EntityId(NLMISC::CEntityId::getNewEntityId(RYZOMID::fx_entity))
, _DataSetRow()
, _Pos(pos)
, _Sheet(sheet)
{
}

CFxEntity::~CFxEntity()
{
}

bool CFxEntity::spawn()
{
	// Add into mirror
	if (!CMirrors::createEntity(_EntityId).isValid())
		return false;
	_DataSetRow = TheDataset.getDataSetRow(_EntityId);
	
	// Set the sheet id
	CMirrorPropValue<TYPE_SHEET> sheetMirror( TheDataset, _DataSetRow, DSPropertySHEET );
	sheetMirror = _Sheet.asInt();
	
	// Set the initial position
	CMirrorPropValue<TYPE_POSX> posX( TheDataset, _DataSetRow, DSPropertyPOSX );
	CMirrorPropValue<TYPE_POSY> posY( TheDataset, _DataSetRow, DSPropertyPOSY );
	posX = (TYPE_POSX)(_Pos.x().asInt());
	posY = (TYPE_POSY)(_Pos.y().asInt());
	
	// Set the mode
	MBEHAV::TMode md;
	md.setModeAndPos( MBEHAV::NORMAL, _DataSetRow );
	CMirrorPropValue<MBEHAV::TMode> mode( TheDataset, _DataSetRow, DSPropertyMODE );
	mode = md;
	
	// Set the WhoSeesMe bitfield (every bit set to 1)
	const uint64 bitfield = IsRingShard? R2_VISION::buildWhoSeesMe(R2_VISION::VISIBLE, true): UINT64_CONSTANT(0xffffffffffffffff);
	CMirrorPropValue<TYPE_WHO_SEES_ME> whoSeesMe(TheDataset, _DataSetRow, DSPropertyWHO_SEES_ME );
	whoSeesMe = bitfield;
	
	// Contextual properties init
	CMirrorPropValue<TYPE_CONTEXTUAL> contextualProperties(TheDataset, _DataSetRow, DSPropertyCONTEXTUAL );
	contextualProperties = 0;
	
	CMirrors::declareEntity( _DataSetRow );

	CFxEntityManager::getInstance()->registerEntity(CFxEntityPtr(this));

	return true;
}

void CFxEntity::despawn()
{
	CFxEntityManager::getInstance()->unregisterEntity(CFxEntityPtr(this));
	
	if (_DataSetRow.isValid())
		CMirrors::removeEntity(_EntityId);
}

NLMISC::CEntityId const& CFxEntity::id() const
{
	return _EntityId;
}

std::string CFxEntity::get(std::string const& prop)
{
	if (prop=="sheet")
		return _Sheet.toString();
	else if (prop=="position")
		return _Pos.toString();
	else
		nlwarning("Trying to get a bad property ('%s') on fx entity '%s'", prop.c_str(), _EntityId.toString().c_str());
	return std::string();
}

std::string CFxEntity::getOneLineInfoString() const
{
	return "FxEntity eid=" + _EntityId.toString() + " sheet=" + _Sheet.toString() + " x=" + NLMISC::toString(_Pos.x()) + " y=" + NLMISC::toString(_Pos.x());
}

std::vector<std::string> CFxEntity::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	
	pushTitle(container, "CFxEntity");
	pushEntry(container, "eid=" + _EntityId.toString());
	container.back() += " sheet=" + _Sheet.toString();
	pushFooter(container);
	
	
	return container;
}
