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

#include "client_host.h"
#include "frontend_service.h"

#include "game_share/entity_types.h" // required for ifdef

#include "id_impulsions.h"
#include "uid_impulsions.h"

using namespace std;
using namespace NLMISC;
using namespace CLFECOMMON;


#ifdef HALF_FREQUENCY_SENDING_TO_CLIENT
//#pragma message ("HALF_FREQUENCY_SENDING_TO_CLIENT")
#else
//#pragma message ("FULL_FREQUENCY_SENDING_TO_CLIENT")
#endif


/**
 * Comparison functor
 * Helps sorting by distance the entities seen by a client, by priority
 */
struct TComparePairsByDistance
{
	TComparePairsByDistance( CVisionArray *va, TClientId clientId ) : _VisionArray(va), _ClientId(clientId) {}

	bool	operator() ( CLFECOMMON::TCLEntityId first, CLFECOMMON::TCLEntityId second )
	{
		return ( _VisionArray->getPairState( _ClientId, first ).DistanceCE < _VisionArray->getPairState( _ClientId, second ).DistanceCE );
	}

	TClientId		_ClientId;
	CVisionArray	*_VisionArray;
};

// get Pair state
inline TPairState&	CClientHost::getPairState(TCLEntityId e)
{
	return CFrontEndService::instance()->PrioSub.VisionArray.getPairState( _ClientId, e );
}
// get Pair state
inline const TPairState&	CClientHost::getPairState(TCLEntityId e) const
{
	return CFrontEndService::instance()->PrioSub.VisionArray.getPairState( _ClientId, e );
}


/*
 * Prepare a clean new outbox with current values
 *
 */
void CClientHost::setupOutBox( TOutBox& outbox )
{
	// only fill outbox headers if connected
	// in system mode, fill is done manually because packets are not sent at each update
	nlassert ( ConnectionState == Connected );
	// a. Add the send number belonging to the destination client
	uint32 sendnumber = getNextSendNumber();
	outbox.serialAndLog1( sendnumber );

	// b. System bit
	bool	systemMode = false;
	outbox.serialBitAndLog( systemMode );

	// b. Add the latest receive number belonging to the destination client
	outbox.serialAndLog1( _ReceiveNumber );

	// c. Add the "toggle" bit for important actions
	//OutBox.serialBit( _ToggleBit );
}

/*
 * Prepare a clean system header
 */
void	CClientHost::setupSystemHeader( TOutBox& outbox, uint8 code)
{
	// Only setup header for special states such as Synchronize, Probe...
	nlassert(ConnectionState != Connected);

	// checks the outbox is really cleared
	nlassert(outbox.length() == 0);

	// a. Add the send number belonging to the destination client
	uint32 sendnumber = getNextSendNumber();
	outbox.serialAndLog1( sendnumber );

	// b. System bit
	bool	systemMode = true;
	outbox.serialBitAndLog( systemMode );

	// c. System message code
	outbox.serialAndLog1( code );
}


/*
 * Set receive time now
 */
void CClientHost::setReceiveTimeNow()
{
	_ReceiveTime = CTime::getLocalTime();
}


/*
 * Initialize the client bandwidth
 */
void CClientHost::initClientBandwidth()
{
	setClientBandwidth( CFrontEndService::instance()->sendSub()->clientBandwidth() );
}


/*
 * CClientHost: Compute host stats
 */
void CClientHost::computeHostStats( const TReceivedMessage& msgin, uint32 currentcounter, bool updateAcknowledge )
{
	if ( _ReceiveNumber == 0xFFFFFFFF )
	{
		_FirstReceiveNumber = currentcounter;
		if (updateAcknowledge)
			_ReceiveNumber = currentcounter;
	}
	else if ( currentcounter <= _ReceiveNumber )
	{
		++_DatagramRepeated;
	}
	else if ( currentcounter > _ReceiveNumber+1 )
	{
		_DatagramLost += currentcounter-(_ReceiveNumber+1);
		if (updateAcknowledge)
			_ReceiveNumber = currentcounter;
	}
	else if (updateAcknowledge)
	{
		_ReceiveNumber = currentcounter;
	}
}


/*
 *
 */
const char *associationStateToString( uint8 as )
{
	switch( as )
	{
	case TPairState::UnusedAssociation:		return "Unused"; break;
	case TPairState::AwaitingAssAck:		return "Assocn"; break;
	case TPairState::NormalAssociation:		return "Normal"; break;
	case TPairState::AwaitingDisAck:		return "Disass"; break;

/*
	// CHANGED BEN
	case CClientEntityIdTranslator::CEntityInfo::UnusedAssociation : return "Unused"; break;
	case CClientEntityIdTranslator::CEntityInfo::AwaitingAssAck : return "Assocn"; break;
	case CClientEntityIdTranslator::CEntityInfo::NormalAssociation : return "Normal"; break;
	case CClientEntityIdTranslator::CEntityInfo::AwaitingDisAck : return "Disass"; break;
*/
	/*case CClientEntityIdTranslator::CEntityInfo::SubstitutionBeforeDisAck : return "Substitution before dis ack"; break;
	case CClientEntityIdTranslator::CEntityInfo::SubstitutionAfterDisAck : return "Substitution after dis ack"; break;
	case CClientEntityIdTranslator::CEntityInfo::CancelledSubstitution : return "Cancelled substitution"; break;*/
	default: return "INVALID ASSOCIATION CODE";
	}
}


inline std::string		getUserName( const TEntityIndex& entityIndex )
{
	string name;
	if ( entityIndex.isValid() )
	{
		TClientId clientId = CFrontEndService::instance()->receiveSub()->EntityToClient.getClientId( entityIndex );
		if ( clientId != INVALID_CLIENT )
		{
			CClientHost *client = CFrontEndService::instance()->receiveSub()->clientIdCont()[clientId];
			if ( client )
			{
				name = string(" ") + client->UserName;
			}
		}
	}
	return name;
}


/*
 * display nlinfo
 */
void CClientHost::displayClientProperties( bool full, bool allProps, bool sortByDistance, NLMISC::CLog *log ) const
{
	// General properties
	bool invision = false;
	TSheetId sheetId = INVALID_SHEETID;
	string sheetIdS = "_";
	CEntity *sentity = NULL;
	if ( (_EntityIndex.isValid()) && (_EntityIndex.getIndex() < (uint32)TheDataset.maxNbRows()) )
	{
		sentity = TheEntityContainer->getEntity( _EntityIndex );
		if ( sentity )
		{
			invision = true;
			CMirrorPropValueRO<uint32> propSheet( TheDataset, _EntityIndex, DSPropertySHEET );
			sheetId = propSheet();
			if ( sentity->propertyIsInitialized( PROPERTY_SHEET, DSPropertySHEET, _EntityIndex, (TYPE_SHEET*)NULL ) )
			{
				sheetIdS = CSheetId(sheetId).toString();
			}
		}
	}

	const char *notset = "(not set)";
	log->displayNL( "C%hu E%s %s %s sheet %s Uid %u, %s, %s", _ClientId, _EntityIndex.isValid() ? toString("%u", _EntityIndex.getIndex()).c_str() : notset, _Id.isUnknownId() ? notset : _Id.toString().c_str(), getEntityName( _EntityIndex ).c_str(), (sheetId!=~0) ? toString( "%s (%u)", sheetIdS.c_str(), sheetId).c_str() : notset, Uid, UserName.c_str(), invision?"in vision":"not in vision" );
	if ( sentity )
	{
		log->displayNL( "Position (m): %d %d %d - Local: %d %d %d - Mode: %s", sentity->posXm( _EntityIndex ), sentity->posYm( _EntityIndex ), sentity->posZm( _EntityIndex ), sentity->posLocalXm( _EntityIndex ), sentity->posLocalYm( _EntityIndex ), sentity->posLocalZm( _EntityIndex ), (sentity->z( _EntityIndex )&0x1)?"Relative":"Absolute" );
	}
	log->displayNL( "Host %s, %s, %s", _Address.asString().c_str(), _Disconnected?"disconnected":"connected", _Synchronized?"synchronized":"not synchronized" );
	log->displayNL( "Latest activity %u ms ago, state %s", (uint32)(CTime::getLocalTime()-_ReceiveTime),
		ConnectionState==Synchronize?"SYNC":ConnectionState==Connected?"CONN":ConnectionState==Probe?"PROBE":ConnectionState==ForceSynchronize?"FORCE_SYNC":"DISC" );
#ifdef HALF_FREQUENCY_SENDING_TO_CLIENT
	sint freq = 5;
#else
	sint freq = 10;
#endif
	if ( _EntityIndex.isValid())
	{
		CMirrorPropValueRO<uint16> availableImpulseBitsize( TheDataset, _EntityIndex, DSFirstPropertyAvailableImpulseBitSize );
		log->displayNL( "Bit bandwidth usage feeding client at %d Hz: %d bps (max 13312), current throttle %d; including impulsion %d, database throttle %d",
			freq, _BitBandwidthUsageAvg*freq, getCurrentThrottle()*freq, _BitImpulsionUsageAvg*freq, availableImpulseBitsize*freq );

		// Impulsion stats
		log->displayNL( "Nb messages in impulse queues: %u %u %u", ImpulseEncoder.queueSize(0), ImpulseEncoder.queueSize(1), ImpulseEncoder.queueSize(2) );
	}

	if ( full )
	{
		log->displayNL( "Vision slots (%hu free):", NbFreeEntityItems );

		// Client sees *
		vector<sint> slots;

		sint e;
		for ( e=0; e!=MAX_SEEN_ENTITIES_PER_CLIENT; ++e )
		{
			//uint8 assState;
			//if ( (assState = CFrontEndService::instance()->PrioSub.VisionArray.getAssociationState( this, (TCLEntityId)e )) != CClientEntityIdTranslator::CEntityInfo::UnusedAssociation )	// CHANGED BEN
			if ( getPairState( (TCLEntityId)e ).AssociationState != TPairState::UnusedAssociation )
			{
				slots.push_back( e );
			}
		}

		if ( sortByDistance )
		{
			sort( slots.begin(), slots.end(), TComparePairsByDistance( &(CFrontEndService::instance()->PrioSub.VisionArray), _ClientId ) );
		}

		vector<sint>::iterator islot;
		for ( islot=slots.begin(); islot!=slots.end(); ++islot )
		{
			displaySlotProperties( *islot, allProps, log );
		}

		// Client is seen by * on this front-end
		/*if ( (_EntityIndex.isValid()) && (_EntityIndex < TheDataSet.maxNbRows()) )
		{
			log->displayNL( "Visibility of this entity by other clients on this front-end:" );
			TObserverList::const_iterator iol;
			for ( iol= CFrontEndService::instance()->PrioSub.VisionProvider.observerList( _EntityIndex ).begin();
				  iol!=CFrontEndService::instance()->PrioSub.VisionProvider.observerList( _EntityIndex ).end();
				  ++iol )
			{
				log->displayNL( "* Seen by client %hu using slot %hu", (*iol).ClientId, (uint16)(*iol).Slot );
			}
		}
		else
		{
			log->displayNL( "Cannot access the observer list" );
		}*/

		TheEntityContainer->mirrorInstance().displayRows( _Id, *log );
	}
}


/*
 * display nlinfo for one slot
 */
void CClientHost::displaySlotProperties( CLFECOMMON::TCLEntityId e, bool full, NLMISC::CLog *log ) const
{
	//const CClientEntityIdTranslator::CEntityInfo& info = IdTranslator.getInfo( (TCLEntityId)e );
	TEntityIndex seenEntityIndex = getPairState( e ).EntityIndex;
	if ( ! seenEntityIndex.isValid() )
		return;
	CEntity *seenEntity = TheEntityContainer->getEntity( seenEntityIndex );
	CAction::TValue vlsx, vlsy, vlsz;
	CFrontEndService::instance()->history()->getPosition( _ClientId, e, vlsx, vlsy, vlsz ); // always the absolute pos
	TCoord lsx = ((sint32)vlsx)/1000, lsy = ((sint32)vlsy)/1000, lsz = ((sint32)vlsz)/1000;
	TCoord sentMileage = CFrontEndService::instance()->history()->getMileage( _ClientId, e );
	TCoord mileageDelta = seenEntity->Mileage - sentMileage;
	const TPairState& pairState = getPairState( e );
	string sheetId, sheetIdS;
	if ( seenEntity->propertyIsInitialized( PROPERTY_SHEET, DSPropertySHEET, seenEntityIndex, (TYPE_SHEET*)NULL ) )
	{
		CMirrorPropValueRO<uint32> propSheet( TheDataset, seenEntityIndex, DSPropertySHEET );
		sheetId = toString("%u", propSheet() );
		sheetIdS = CSheetId(propSheet()).toString();
	}
	else
	{
		sheetId = "_";
		sheetIdS = "_";
	}
	const CEntityId& seenEntityId = TheDataset.getEntityId( seenEntityIndex );
	log->displayNL( "* Slot %d E%u %s %s st%s %s sheet %s (%s) dist(m) %0.1f   %s pos %d %d %d, sent %d %d %d %sdelta %d prio %.1f ab %hu",
		e,
		seenEntityIndex.getIndex(),
		(seenEntityId.getType()==RYZOMID::player)?(string("PLAYER")+getUserName(seenEntityIndex)).c_str():RYZOMID::toString( (RYZOMID::TTypeId)seenEntityId.getType() ).c_str(),
		seenEntityId.toString().c_str(), 
		associationStateToString( getPairState( e ).AssociationState),
		getEntityName(seenEntityIndex).c_str(),
		sheetId.c_str(),
		sheetIdS.c_str(),
		(float)pairState.DistanceCE/1000.0f,
		getDirection( seenEntity, seenEntityIndex ),
		seenEntity->posXm( seenEntityIndex ),
		seenEntity->posYm( seenEntityIndex ),
		seenEntity->posZm( seenEntityIndex ),
		lsx, lsy, lsz,
		(sentMileage==0)?"(not sent yet) ":"", mileageDelta,
		pairState.getPrio(), (uint16)(pairState.AssociationChangeBits & 0x3) );

	if ( full )
	{
		log->displayNL( "| Visual properties of E%u: ", seenEntityIndex.getIndex() );
		seenEntity->displayProperties( seenEntityIndex, log, _ClientId, e );
	}
}


/*
 * Return the cardinal direction from the player to the seen entity
 */
const char * CClientHost::getDirection( CEntity *seenEntity, const TEntityIndex& seenEntityIndex ) const
{
	if ( !_EntityIndex.isValid() )
		return "-";
	CEntity *entity = TheEntityContainer->getEntity( _EntityIndex );
	if ( entity == NULL )
		return "-";

	float dx = (float)(seenEntity->X() - entity->X());
	float dy = (float)(seenEntity->Y() - entity->Y());
	float angle = (float)fmod( atan2(dy, dx) + (2*Pi), 2*Pi );
	uint direction = (uint) ( 8.0f*angle/((float)Pi) );
	nlassert ( direction<16 );
	const char * txts[] =
	{
		"E",
		"ENE",
		"NE",
		"NNE",
		"N",
		"NNW",
		"NW",
		"WNW",
		"W",
		"WSW",
		"SW",
		"SSW",
		"S",
		"SSE",
		"SE",
		"ESE",
	};

	return txts[direction];
}


/*
 * display nlinfo (1 line only)
 */
void CClientHost::displayShortProps(NLMISC::CLog *log) const
{
	// General properties
	bool invision = false;
	if ( (_EntityIndex.isValid()) && (_EntityIndex.getIndex() < (uint32)TheDataset.maxNbRows()) )
	{
		CEntity *sentity = TheEntityContainer->getEntity( _EntityIndex );
		if ( sentity != NULL )
			invision = true;
	}
	log->displayNL( "Client %hu: E%u %s '%s' uid %u '%s' %s, %s", _ClientId, _EntityIndex.getIndex(), _Id.toString().c_str(), getEntityName(_EntityIndex).c_str(), Uid, UserName.c_str(), invision?"in vision":"not in vision", _Address.asString().c_str() );
}



/*
 * Set the CEntityId
 */
void CClientHost::setEId( const NLMISC::CEntityId& assigned_id )
{
	_Id = assigned_id;
}


/*
 * Destructor
 */
CClientHost::~CClientHost()
{
	//REMOVE_PROPERTY_FROM_EMITER( _Id, uint16, "AvailImpulseBitsize" );
	//_Id = CEntityId::Unknown;
}


/*
 * Set the entity index
 */
void				CClientHost::setEntityIndex( const TEntityIndex& ei )
{
	_EntityIndex = ei;
	CFrontEndService::instance()->PrioSub.VisionArray.setEntityIndex( _ClientId, 0, ei );
}


void				CClientHost::CGenericMultiPartTemp::set (CActionGenericMultiPart *agmp, CClientHost *client)
{
	if (NbBlock == 0xFFFFFFFF)
	{
		// new GenericMultiPart
		NbBlock = agmp->NbBlock;
		NbCurrentBlock = 0;
		TempSize = 0;
		Temp.clear();
		Temp.resize(NbBlock);
		BlockReceived.resize(NbBlock);
		for (uint i = 0; i < NbBlock; i++)
			BlockReceived[i] = false;
	}
	
	nlassert (NbBlock == agmp->NbBlock);
	nlassert (NbBlock > agmp->Part);

	// check if the block was already received
	if (BlockReceived[agmp->Part])
	{
		return;
	}

	Temp[agmp->Part] = agmp->PartCont;
	BlockReceived[agmp->Part] = true;

	NbCurrentBlock++;
	TempSize += (uint32)agmp->PartCont.size();

	if (NbCurrentBlock == NbBlock)
	{
		// reform the total action
		CBitMemStream bms(true);

		uint8 *ptr = bms.bufferToFill (TempSize);

		for (uint i = 0; i < Temp.size (); i++)
		{
			memcpy (ptr, &(Temp[i][0]), Temp[i].size());
			ptr += Temp[i].size();
		}

		NbBlock = 0xFFFFFFFF;

		if ( client->eId().isUnknownId() )
		{
			routeImpulsionUidFromClient(bms, client->Uid, client->LastReceivedGameCycle);
		}
		else
		{
			routeImpulsionIdFromClient(bms, client->eId(), client->LastReceivedGameCycle);
		}
	}
}

void				CClientHost::resetClientVision()
{
	// Get all entities seen by this client
	sint e;
	for ( e=0; e!=MAX_SEEN_ENTITIES_PER_CLIENT; ++e )
	{
		TEntityIndex entityindex = getPairState( (TCLEntityId)e ).EntityIndex;

		//if ( CFrontEndService::instance()->PrioSub.VisionArray.getAssociationState( this, (TCLEntityId)e ) != CClientEntityIdTranslator::CEntityInfo::UnusedAssociation )	// CHANGED BEN
		if (getPairState( (TCLEntityId)e ).AssociationState != TPairState::UnusedAssociation )
		{
			// Remove from observer list: the clients who see the entity (and its ceid for them)
			/*if ( entityindex != INVALID_ENTITY_INDEX )
			{
				CFrontEndService::instance()->PrioSub.VisionProvider.removeFromObserverList( entityindex, clienthost->clientId(), (TCLEntityId)e );
			}
			else
			{
				nlwarning( "Invalid entity index while trying to remove slot %hu from observer list of leaving client %hu", (uint16)e, clienthost->clientId() );
			}*/

			// Remove pair from history
			CFrontEndService::instance()->history()->removeEntityOfClient( e, clientId() );

			// No need to remove Id because they are stored in the client object, which will be deleted
		}
	}
	CFrontEndService::instance()->PrioSub.Prioritizer.removeAllEntitiesSeenByClient( clientId() );

	// Reset items
	for ( e=0; e!=MAX_SEEN_ENTITIES_PER_CLIENT; ++e )
	{
		// Reset EntityIndex
		getPairState(e).resetItem();
		getPairState(e).resetAssociation();
	}
}
