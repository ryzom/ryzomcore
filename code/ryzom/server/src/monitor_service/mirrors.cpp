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

#include "mirrors.h"
#include "game_share/tick_event_handler.h"
#include "game_share/synchronised_message.h"
#include "nel/net/service.h"
#include "game_share/misc_const.h"
#include "client.h"

extern std::map<TYPE_NAME_STRING_ID, std::string>	StringMap;
extern std::set<TYPE_NAME_STRING_ID>				StringAsked;

// ***************************************************************************

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// ***************************************************************************

CMirror				CMirrors::Mirror;
CMirroredDataSet	*CMirrors::DataSet = NULL;
std::vector<CGlobalEntityEntry>	GlobalEntites;

// ***************************************************************************

/*
 * Initialisation 2
 */
void	cbMirrorIsReady( CMirror *mirror )
{
	//	nlstop("Not Implemented");
	/* no type owner
	Mirror.declareEntityTypeOwner( RYZOMID::npc,		AI::maxBotsNpc()	);
	Mirror.declareEntityTypeOwner( RYZOMID::creature,	AI::maxBotsCreature()	+	AI::maxBotsPet() ); */

	// ** todo hulud remove this, set it with a xml file serialised in a class

	// This mirror properties are checked a each ticks
	CMirrors::DataSet = &(CMirrors::Mirror.getDataSet("fe_temp"));
	CMirrors::DataSet->declareProperty( "X", PSOReadOnly | PSONotifyChanges, "X" );
	CMirrors::DataSet->declareProperty( "Y", PSOReadOnly | PSONotifyChanges, "X" );
	CMirrors::DataSet->declareProperty( "Z", PSOReadOnly | PSONotifyChanges, "X" );
	CMirrors::DataSet->declareProperty( "Theta", PSOReadOnly | PSONotifyChanges, "X" );

	// This mirror properties are checked once, whena primitive is added
	CMirrors::DataSet->declareProperty( "NameIndex", PSOReadOnly );

	// This mirror properties are checked when they are modified
	CMirrors::DataSet->declareProperty( "Sheet", PSOReadOnly );
	CMirrors::DataSet->declareProperty( "SheetServer", PSOReadOnly );
	CMirrors::DataSet->declareProperty( "Mode", PSOReadOnly | PSONotifyChanges );
	CMirrors::DataSet->declareProperty( "Behaviour", PSOReadOnly | PSONotifyChanges );
	CMirrors::DataSet->declareProperty( "Target", PSOReadOnly );
	CMirrors::DataSet->declareProperty( "CurrentHitPoints", PSOReadOnly | PSONotifyChanges );
	CMirrors::DataSet->declareProperty( "MaxHitPoints", PSOReadOnly | PSONotifyChanges );
	CMirrors::DataSet->declareProperty( "BestRoleLevel", PSOReadOnly );
	CMirrors::DataSet->declareProperty( "CombatState", PSOReadOnly );
	CMirrors::DataSet->declareProperty( "TeamId", PSOReadOnly );
	CMirrors::DataSet->declareProperty( "VisualPropertyA", PSOReadOnly );
	CMirrors::DataSet->declareProperty( "VisualPropertyB", PSOReadOnly );
	CMirrors::DataSet->declareProperty( "VisualPropertyC", PSOReadOnly );
	CMirrors::DataSet->declareProperty( "ActionFlags", PSOReadOnly );
	
	initRyzomVisualPropertyIndices( *CMirrors::DataSet );

	CMirrors::Mirror.setNotificationCallback( CMirrors::processMirrorUpdates );
}

// ***************************************************************************

#define initIsolatedPropTable( name, thetype, defaultvalue ) \
	NL_ALLOC_CONTEXT(AIIPT); \
	name = new thetype [MAX_NB_ENTITIES_ISOLATED]; \
	for ( i=0; i!=MAX_NB_ENTITIES_ISOLATED; ++i ) \
		name[i] = defaultvalue

// ***************************************************************************

/*
 * Initialisation 1
 */
void CMirrors::init( void (*cbUpdate)(), void (*cbSync)() )
{
	// Init the mirror system
	vector<string> datasetNames;
	datasetNames.push_back( "fe_temp" );
	Mirror.init( datasetNames, cbMirrorIsReady, cbUpdate, cbSync );
}

// ***************************************************************************

void	CMirrors::processMirrorUpdates()
{
	TDataSetRow	entityIndex;
	CEntityId	*pEntityId;
	
	// Process added entities
	DataSet->beginAddedEntities();
	while	((entityIndex = DataSet->getNextAddedEntity())	!=	LAST_CHANGED)
	{
		// Should not be present
		uint32 index = entityIndex.getIndex();
		if (index >= GlobalEntites.size())
			GlobalEntites.resize (index+1);
		nlassert ((GlobalEntites[index].Flags & CGlobalEntityEntry::Present) == 0);
		GlobalEntites[index].Flags = CGlobalEntityEntry::Present;

		uint	iclient;
		for (iclient=0; iclient<Clients.size(); ++iclient)
		{
			CMonitorClient	&client = *Clients[iclient];
			const CVector	&topleft = client.getTopLeft();
			const CVector	&bottomright = client.getBottomRight();

			CMirrorPropValueRO<TYPE_POSX> valueX( *DataSet, entityIndex, DSPropertyPOSX );
			CMirrorPropValueRO<TYPE_POSY> valueY( *DataSet, entityIndex, DSPropertyPOSY );
			CVector		pos((float)valueX / 1000.f, (float)valueY / 1000.f, 0.0f);

			// is entity in client window
			//nlassert(entityIndex.getIndex() < client.Entites.size());
			if (pos.x > topleft.x && pos.x < bottomright.x && pos.y > topleft.y && pos.y < bottomright.y)
			{
				// Add
				client.add( entityIndex );
			}
		}

		// Does the string exist ?
		CMirrorPropValueRO<TYPE_NAME_STRING_ID> stringId( TheDataset, entityIndex, DSPropertyNAME_STRING_ID);
		TYPE_NAME_STRING_ID strId = stringId;

		// Valid string ID ?
		if (strId)
		{
			std::map<TYPE_NAME_STRING_ID, std::string>::iterator ite = StringMap.find (strId);
			if (ite == StringMap.end())
			{
				// Not asked yet ?
				if (StringAsked.find (strId) == StringAsked.end())
				{
					// No, ask IOS for the string
					StringAsked.insert (strId);
					CMessage msgout( "GET_STRING" );
					msgout.serial( strId );
					sendMessageViaMirror( "IOS", msgout );
				}
			}
		}
	}
	DataSet->endAddedEntities();

	// Process removed entities
	DataSet->beginRemovedEntities();
	while( (entityIndex = DataSet->getNextRemovedEntity(&pEntityId)) != LAST_CHANGED )
	{
		// Should not be present
		uint32 index = entityIndex.getIndex();
		nlassert (index < GlobalEntites.size());
		nlassert (GlobalEntites[index].Flags & CGlobalEntityEntry::Present);
		GlobalEntites[index].Flags &= ~CGlobalEntityEntry::Present;

		uint	iclient;
		for (iclient=0; iclient<Clients.size(); ++iclient)
		{
			if (index < Clients[iclient]->Entites.size()) 
			{
				if (Clients[iclient]->Entites[index].Flags & CMonitorClient::CEntityEntry::Present)
				{
					Clients[iclient]->remove(entityIndex);
				}
			}
		}
	}
	DataSet->endRemovedEntities();

	// Process properties changed and notified in the mirror
	TPropertyIndex propIndex;
	DataSet->beginChangedValues();
	DataSet->getNextChangedValue( entityIndex, propIndex );
	while ( entityIndex != LAST_CHANGED )
	{
		CMirrorPropValueRO<TYPE_POSX> valueX( *DataSet, entityIndex, DSPropertyPOSX );
		CMirrorPropValueRO<TYPE_POSY> valueY( *DataSet, entityIndex, DSPropertyPOSY );

		CVector		pos((float)valueX / 1000.f, (float)valueY / 1000.f, 0.0f);

		uint	iclient;
		for (iclient=0; iclient<Clients.size(); ++iclient)
		{
			CMonitorClient	&client = *Clients[iclient];
			const CVector	&topleft = client.getTopLeft();
			const CVector	&bottomright = client.getBottomRight();

			// is entity in client window
			//nlassert(entityIndex.getIndex() < client.Entites.size());
			bool	wasPresent = (entityIndex.getIndex() < client.Entites.size() && (client.Entites[entityIndex.getIndex()].Flags & CMonitorClient::CEntityEntry::Present) != 0);
			// See if position changed
			if (propIndex == DSPropertyPOSX || propIndex == DSPropertyPOSX)
			{
				if (pos.x > topleft.x && pos.x < bottomright.x && pos.y > topleft.y && pos.y < bottomright.y)
				{
					if (!wasPresent)
					{
						// Add
						client.add( entityIndex );
					}

					client.Entites[entityIndex.getIndex()].Flags |= CMonitorClient::CEntityEntry::PosDirty;
				}
				else if (wasPresent)
				{
					// Rmv
					client.remove( entityIndex );
				}
			}
			// deals with others less frequently updated properties
			if (wasPresent)
			{
				if (propIndex == DSPropertyCURRENT_HIT_POINTS || 
					propIndex == DSPropertyMAX_HIT_POINTS ||
					propIndex == DSPropertyMODE ||
					propIndex == DSPropertyBEHAVIOUR
				   )
				{
					client.Entites[entityIndex.getIndex()].Flags |= CMonitorClient::CEntityEntry::MiscPropDirty;
				}
			}
		}

		//nldebug( "Pos changed from mirror E%d", entityIndex  );
		DataSet->getNextChangedValue( entityIndex, propIndex );
	}
	DataSet->endChangedValues();

}

// ***************************************************************************

void CMirrors::release()
{
	Mirror.release();
}

// ***************************************************************************
