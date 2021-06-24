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



#ifndef RY_EVS_H
#define RY_EVS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/vectord.h"
#include "nel/misc/line.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/geom_ext.h"

#include <string>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <deque>

#include "nel/net/service.h"

#include "game_share/ryzom_entity_id.h"
#include "game_share/entity_types.h"
#include "game_share/mirror_prop_value.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/mirror.h"

#include <nel/3d/u_instance.h>

// raw pacs include -- not in user interface :/
#include "pacs/chain.h"

#include "move_grid.h"
#include "../combat_move_service/sheets.h"

class CEntity;

typedef CMoveGrid<CEntity*, 256, 32.0f>						TEntityGrid;
typedef CMoveGrid<NLPACS::COrderedChain3f*, 256, 32.0f>		TChainGrid;

class CEntity
{
public:
	//
	CEntity() : Position(0.0, 0.0, 0.0), Orientation(0.0f), Check(false), Target(NLMISC::CEntityId::Unknown), SheetId(0), Radius(0.5f), Height(2.0f), EntityIndex() {}

	//
	NLMISC::CEntityId							Id;
	TDataSetRow									EntityIndex;
	NLMISC::CVectorD							Position;
	NLMISC::CVectorD							NonMirroredPos;
	float										Orientation;
	bool										Check;
	TEntityGrid::CIterator						Iterator;

	//
	NLMISC::CEntityId							Target;

	//
	NLMISC::CSheetId							SheetId;
	std::string									SheetName;
	float										Radius;
	float										Height;

	//
	CMirrorPropValueAlice1DS<TYPE_COMBAT_STATE>	CombatState;

	//
	CMirrorPropValueAlice1DS<TYPE_POSX>			X;
	CMirrorPropValueAlice1DS<TYPE_POSY>			Y;
	CMirrorPropValueAlice1DS<TYPE_POSZ>			Z;
	CMirrorPropValueAlice1DS<TYPE_ORIENTATION>	Theta;
	CMirrorPropValueAlice1DS<TYPE_SHEET>		Sheet;

	//
	std::deque<NLMISC::CVectorD>				Path;

	//
	void	setup(const TDataSetRow& entityIndex);

	//
	void	setSheet(const NLMISC::CSheetId &sheetId);
};

/**
 * CEntityViewService
 *
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CEntityViewService : public NLNET::IService
{
private:
	typedef std::map<NLMISC::CEntityId, CEntity>	TEntityMap;

public:

	/** 
	 * init the service
	 */
	void init(void);

	/**
	 * main loop
	 */
	bool update(void);

	/**
	 * release
	 */
	void release(void);

	void displayEntities()
	{
		TEntityMap::iterator	ite;
		uint					n = 0;
		for (ite=_Entities.begin(); ite!=_Entities.end(); ++ite)
		{
			CEntity	&entity=(*ite).second;
			nlinfo("  %s: (%.3f,%.3f,%.3f)", entity.Id.toString().c_str(), entity.Position.x, entity.Position.y, entity.Position.z);
			++n;
		}
		nlinfo("%d entities in GPMS", n);
	}

	/// Set target
	void	setTarget(const NLMISC::CEntityId &entity, const NLMISC::CEntityId &target)
	{
		TEntityMap::iterator	ite, itt;

		if ((ite = _Entities.find(entity)) == _Entities.end())
			return;

		(*ite).second.Target = target;
	}

	/// Set target
	void	setPos(const NLMISC::CEntityId &entity, sint x, sint y, sint z)
	{
		TEntityMap::iterator	ite, itt;

		if ((ite = _Entities.find(entity)) == _Entities.end())
			return;

		(*ite).second.NonMirroredPos = NLMISC::CVectorD(x*0.001, y*0.001, z*0.001);
	}

	void	initMirror();

	CMirror				Mirror;
	CMirroredDataSet	*DataSet;




	//
	void	updateEntities();

	//
	TEntityMap::iterator	createEntity(const NLMISC::CEntityId &id)
	{
		nlinfo("Create entity %s", id.toString().c_str());

		std::pair<TEntityMap::iterator,bool>	res = _Entities.insert(std::make_pair<NLMISC::CEntityId,CEntity>(id, CEntity()));
		TEntityMap::iterator	ite = res.first;
		nlassert(ite != _Entities.end() && res.second);

		(*ite).second.Iterator = _EntityGrid.insert(&((*ite).second), (*ite).second.Position);
		(*ite).second.Id = id;

		return ite;
	}

	//
	void					removeEntity(const NLMISC::CEntityId &id)
	{
		nlinfo("Remove entity %s", id.toString().c_str());

		TEntityMap::iterator	ite = _Entities.find(id);
		if (ite == _Entities.end())
			return;

		_EntityGrid.remove((*ite).second.Iterator);
		_Entities.erase(ite);

		return;
	}

	//
	CEntity					*getEntity(const NLMISC::CEntityId &id)
	{
		TEntityMap::iterator	it = _Entities.find(id);

		if (it == _Entities.end())
			return NULL;

		return &((*it).second);
	}


private:
	//
	TEntityMap										_Entities;

	//
	std::vector<NL3D::UInstance*>					_OBoxes;
	std::vector<NL3D::UInstance*>					_TBoxes;
	std::vector<NL3D::UInstance*>					_OCylinders;
	std::vector<NL3D::UInstance*>					_TCylinders;

	std::vector<NLMISC::CLine>						_Directions;
	std::vector<NLMISC::CLineColor>					_Targets;

	//
	TEntityGrid										_EntityGrid;
	TChainGrid										_ChainGrid;

	std::vector<NLPACS::COrderedChain3f>			_Chains;
};

extern CEntityViewService		*pEVS;

#define TheMirror (pEVS->Mirror)
#define TheDataset (*(pEVS->DataSet))


#endif //RY_GPMS_H



