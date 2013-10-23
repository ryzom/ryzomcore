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
#include "game_share/action_factory.h"
#include "game_share/action_position.h"
#include "game_share/action_sint64.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/entity_types.h"
#include "distance_prioritizer.h"
#include "client_host.h"
#include "vision_provider.h"

#include <nel/misc/command.h>
#include "frontend_service.h"
#ifdef TEST_LOST_PACKET
#include <nel/misc/variable.h>
#endif

using namespace std;
using namespace CLFECOMMON;
using namespace NLMISC;
using namespace NLNET;


// Test feature to simulate a lost packet when sending a new sheet. Works with 1 client only.
#ifdef TEST_LOST_PACKET
CVariable<bool> TestPacketLost( "test", "TestPacketLost", "", true, 0, 1 );
TGameCycle TestPacketLostTimer = 0;
TCLEntityId TestPacketLostSlot = 0;
#endif

TClientId verbosePropertiesSent = INVALID_CLIENT;

// TODO-Minimal visual bandwith
/*sint32 NbMinimalVisualBits;

void cbNbMinimalVisualBytesChanged( IVariable& var );

CVariable<sint32> NbMinimalVisualBytes( "fe", "NbMinimalVisualBytes", "Min number of bytes for vision per msg", 50, 0, true, cbNbMinimalVisualBytesChanged, true );

void cbNbMinimalVisualBytesChanged( IVariable& var )
{
	NbMinimalVisualBits = NbMinimalVisualBytes.get() * 8;
}*/


/// Init
void		CDistancePrioritizer::init( CVisionArray *va, CVisionProvider *vp, CHistory *hy )
{
	_VisionArray = va;
	_VisionProvider = vp;
	_History = hy;
	SortSpreader.init();

	for ( sint i=0; i!=NB_VISUAL_PROPERTIES; ++i )
		_DistThresholdTable[i] = -1;

	_VisualPropertyTreeRoot = (TVPNodeServer*)NewNode();
	sint nbProp = _VisualPropertyTreeRoot->buildTree();

	// Check that the last property matches the number of properties-1
	if ( nbProp != NB_VISUAL_PROPERTIES )
	{
		nlwarning( "Found %hu properties in the sheets, the prioritizer knows %hu properties", NB_VISUAL_PROPERTIES, nbProp ); 
	}

	// Init distance thresholds
	CLFECOMMON::initThresholdTable( _DistThresholdTable );
}


/*
 * Calculate the priorities
 */
void		CDistancePrioritizer::calculatePriorities()
{
	// Sort
	if ( SortSpreader.mustProcessNow() )
	{
		THostMap::iterator icm;
		sint clientmapindex, outerBoundIndex;
		SortSpreader.getProcessingBounds( icm, clientmapindex, outerBoundIndex );

		while ( clientmapindex < outerBoundIndex )
		{
			CClientHost *clienthost = GETCLIENTA(icm);
			
			// Prioritize only at the opposite time of sending for a particular client
			if ( ! clienthost->whenToSend() )
			{
				// Update priorities
				updatePriorityOfEntitiesSeenByClient( clienthost->clientId() );

				// Sort entities by decreasing priority
				sortEntitiesOfClient( clienthost->clientId() );
			}

			++clientmapindex;
			++icm;
		}

		SortSpreader.endProcessing( icm );
	}
	SortSpreader.incCycle();
}

/*
#define arbitrateDiscreetProperty( name ) \
	get##name##node()->BranchHasPayload = entityIsWithinDistanceThreshold( PROPERTY_##name ) && discreetPropertyHasChanged( PROPERTY_##name, (TYPE_##name*)NULL )

#define arbitrateDiscreetPropertyWithoutThreshold( name ) \
	get##name##node()->BranchHasPayload = discreetPropertyHasChanged( PROPERTY_##name, (TYPE_##name*)NULL )

#define arbitrateTargetList( name ) \
	get##name##node()->BranchHasPayload = targetListHasChanged( PROPERTY_##name, (TYPE_##name*)NULL )
*/

#ifdef STORE_MIRROR_VP_IN_CLASS

#define arbitrateDiscreetProperty( entry, name ) \
	GET_VP_NODE(name)->BranchHasPayload = \
				entityIsWithinDistanceThreshold( PROPERTY_##name ) \
			&&	discreetPropertyHasChanged( entry.Properties[PROPERTY_##name], sentity->VP_##name, PROPERTY_##name, (TYPE_##name*)NULL )

#define arbitrateDiscreetPropertyWithoutThreshold( entry, name ) \
	GET_VP_NODE(name)->BranchHasPayload = \
				discreetPropertyHasChanged( entry.Properties[PROPERTY_##name], sentity->VP_##name, PROPERTY_##name, (TYPE_##name*)NULL )

#else // STORE_MIRROR_VP_IN_CLASS

#define arbitrateDiscreetProperty( entry, name ) \
	GET_VP_NODE(name)->BranchHasPayload = \
				entityIsWithinDistanceThreshold( PROPERTY_##name ) \
			&&	discreetPropertyHasChanged( entry.Properties[PROPERTY_##name], PROPERTY_##name, (TYPE_##name*)NULL )

#define arbitrateDiscreetPropertyWithoutThreshold( entry, name ) \
	GET_VP_NODE(name)->BranchHasPayload = \
				discreetPropertyHasChanged( entry.Properties[PROPERTY_##name], PROPERTY_##name, (TYPE_##name*)NULL )

#define arbitrateTargetList( entry, name ) \

#endif // STORE_MIRROR_VP_IN_CLASS


#define arbitrateNeverSendProperty( name ) \
	GET_VP_NODE(name)->BranchHasPayload = false

#define arbitrateTargetList( entry, name ) \
	GET_VP_NODE(name)->BranchHasPayload = \
				targetListHasChanged( entry.Properties[PROPERTY_##name], PROPERTY_##name, (TYPE_##name*)NULL )


#define	DECLARE_AP(name)		CActionSint64 *ap = CActionFactory::getInstance()->getVolatilePropAction( TVPNodeServer::PrioContext.Slot, PROPERTY_##name )
#define	DECLARE_AP_INDEX(index)	CActionSint64 *ap = CActionFactory::getInstance()->getVolatilePropAction( TVPNodeServer::PrioContext.Slot, index )
#define	REMOVE_AP()

/*
#define	DECLARE_AP(name)		CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_##name )
#define	DECLARE_AP_INDEX(index)	CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, index )
#define	REMOVE_AP()				CActionFactory::getInstance()->remove( (CAction*&)ap )
*/


/*
 *
 */
void		CDistancePrioritizer::fillOutBox( CClientHost& client, TOutBox& outbox )
{
	H_AUTO(FillOutBox)

	TClientId clientId = client.clientId();
	initDispatchingCycle( clientId );
#ifdef NL_DEBUG
	client.MoveNumber = 0;
#endif

	if (!client.entityIndex().isValid())
		return;

	// initialize common context part
	TVPNodeServer::PrioContext.Prioritizer = this;
	TVPNodeServer::PrioContext.ClientHost = &client;
	TVPNodeServer::PrioContext.ClientId = clientId;
	CMirrorPropValueRO<TYPE_TARGET_ID> targetIndexOfClient( TheDataset, client.entityIndex(), DSPropertyTARGET_ID );

	// TODO-Minimal visual bandwith
	/*sint32 initialPosInBit = outbox.getPosInBit();*/
	sint32 currentPosInBit;
	while ( true )
	{
		currentPosInBit = outbox.getPosInBit();

		// Browse the seen entities, sorted by distance
		TCLEntityId slot = getNextEntityToStudy( clientId );
		if ( slot == INVALID_SLOT )
		{
			// Exit when all the pairs have been successfully filled
#ifdef TEST_LOST_PACKET
			if ( (TestPacketLostTimer!=0) && (CTickEventHandler::getGameCycle() >= TestPacketLostTimer) )
			{
				nldebug( "Negative ack for %hu %hu", client.clientId(), TestPacketLostSlot );
				_History->_PacketHistory.negativeAck( client.clientId(), TestPacketLostSlot, PROPERTY_SHEET, 0 );
				TestPacketLostTimer = 0;
			}
#endif
			return;
		}

		// TODO-Minimal visual bandwith
		// Always allow to write a minimal amount of visual properties
		/*if ( currentPosInBit - initialPosInBit >= NbMinimalVisualBits )*/
		{
			// Don't fill if the free space is lower than 32 bits (checked once per pair, not once per property)
			if ( currentPosInBit + 32 > client.getCurrentThrottle() )
			{
				// Exit when the size limit has been reached before all the pairs have been filled
#ifdef NL_DEBUG
				uint nbRemainingPairs = (uint)_PrioritizedEntitiesByClient[clientId].size() - _CurrentEntitiesToStudy[clientId];
				if ( nbRemainingPairs > 0 )
					LOG_WHAT_IS_SENT( "%u: C%hu S%hu: %u pairs remaining", CTickEventHandler::getGameCycle(), clientId, (uint16)slot, nbRemainingPairs );
				LOG_WHAT_IS_SENT( "C%hu: outbox full (%d bits)", clientId, currentPosInBit );
#endif
#ifdef TEST_LOST_PACKET
				if ( (TestPacketLostTimer!=0) && (CTickEventHandler::getGameCycle() >= TestPacketLostTimer) )
				{
					nldebug( "Negative ack for %hu %hu", client.clientId(), TestPacketLostSlot );
					_History->_PacketHistory.negativeAck( client.clientId(), TestPacketLostSlot, PROPERTY_SHEET, 0 );
					TestPacketLostTimer = 0;
				}
#endif
				return;
			}
		}

		// Get the entity corresponding to the client/slot pair
		TPairState&	pairState = _VisionArray->getPairState( clientId, slot );
		CEntity*	sentity = NULL;

		TEntityIndex	entityIndex = pairState.EntityIndex;
		TVPNodeServer::PrioContext.EntityIndex = entityIndex;

		if ( entityIndex.isValid() )
			sentity = TheEntityContainer->getEntity( entityIndex );

		if ( pairState.associationSuppressed() )
		{
			// Pure unassociation case: we must send an empty block
			serialSlotHeader( client, NULL, pairState, slot, outbox );
			uint32 bits = 0;
			outbox.serialAndLog2( bits, 2 ); // 2 bits for pos & other properties
			//nldebug( "Pure unassociation of C%hu S%hu", clientId, (uint16)slot );

			// The first time, sentity is non-null. If this is a resending (after a neg-ack), it's null.
			if ( sentity )
				_VisionProvider->postRemovePair( clientId, slot );
		}
		else
		{
			if ( sentity == NULL )
				continue;

			H_BEFORE(OneSlotArbitrate)

			H_BEFORE(OneSlotArbitrateInit)

			// Initialize the context
			TVPNodeServer::PrioContext.Slot = slot;
			TVPNodeServer::PrioContext.EntityIndex = entityIndex;
			TVPNodeServer::PrioContext.Sentity = sentity;
			//TVPNodeServer::PrioContext.PairState = &pairState;
			TVPNodeServer::PrioContext.DistanceCE = pairState.DistanceCE;
			TVPNodeServer::PrioContext.Timestamp = 0;
			TVPNodeServer::PrioContext.IsTarget = (targetIndexOfClient() == entityIndex);
			TVPNodeServer::PrioContext.PositionAlreadySent = (_History->getMileage( clientId, slot ) != 0);
			TVPNodeServer::PrioContext.ZCache = sentity->z(entityIndex);	// setup Z cache for all later mirror access to entity->z()

			const CPropertyHistory::CEntityEntry&	entry = _History->getEntityEntry(TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.Slot);

			// Debug display
			//nlinfo( "Preparing a block to client %hu (%s) for slot %hu", clientId, client.eId().toString().c_str(), (uint16)slot );

			// Continuous properties
			uint8 seenEntityType = TheDataset.getEntityId( entityIndex ).getType();

			H_AFTER(OneSlotArbitrateInit)

			bool	thetaIntMode = false;

			// Arbitrate all discreet properties
			switch (seenEntityType)
			{
			default:
				arbitrateAllDiscreetProperties(entry);
				break;

			case RYZOMID::npc:
				arbitrateNPCDiscreetProperties(entry);
				break;

			case RYZOMID::creature:
				arbitrateCreatureDiscreetProperties(entry);
				break;

			case RYZOMID::forageSource:
				arbitrateForageSourceDiscreetProperties(entry);

				thetaIntMode = true;

				break;
			}

			H_BEFORE(OneSlotArbitratePropagate)

			// Propagate back BranchHasPayload flags
			//_VisualPropertyTreeRoot->propagateBackBranchHasPayload();
			TVPNodeServer::fastPropagateBackBranchHasPayload();

			H_AFTER(OneSlotArbitratePropagate)

			H_AFTER(OneSlotArbitrate)

			// ******** Fill the buffer with the properties ********

			// Fill the header
			if ( _VisualPropertyTreeRoot->BranchHasPayload )
			{
				H_AUTO(OneSlotFill)

				//nldebug( "Filling for C%hu, pass %u, BEFORE HEADER: bitpos: %d", clientId, ++i, outbox.getPosInBit() );
				//nlinfo( "C%hu S%hu AB%hu", clientId, (uint16)slot, (uint32)pairState.AssociationChangeBits );
				serialSlotHeader( client, sentity, pairState, slot, outbox );

				//nldebug( "AFTER HEADER: pos: %d", outbox.getPosInBit() );

				// Fill the position if required
				//TVPNodeServer *currentNode = _VisualPropertyTreeRoot;

				outbox.serialBitAndLog( GET_VP_NODE(POSITION)->BranchHasPayload );
				if ( GET_VP_NODE(POSITION)->BranchHasPayload )
				{
					//CActionPosition *ap = (CActionPosition*)(CActionFactory::getInstance()->createByPropIndex( slot, PROPERTY_POSITION ));
					//ap->PropertyCode = PROPERTY_POSITION;
					CActionPosition*	ap = CActionFactory::getInstance()->getVolatilePositionAction(slot);

					// When the mode is transmitted or the entity is not in local mode (first bit of Z), transmit the *absolute* position
					if ( (! (TVPNodeServer::PrioContext.ZCache & 0x1)) ||
						 (GET_VP_NODE(POSITION)->BranchHasPayload && GET_VP_NODE(MODE)->BranchHasPayload) )
					{
						ap->Position[0] = sentity->X();
						ap->Position[1] = sentity->Y();
						//ap->Position[2] = sentity->z( entityIndex );
						ap->Position[2] = TVPNodeServer::PrioContext.ZCache;
						ap->IsRelative = false;
						LOG_WHAT_IS_SENT( "%u: C%hu S%hu: Filling ABSPOS: %d %d %d m", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, sentity->posXm( entityIndex ), sentity->posYm( entityIndex ), sentity->posZm( entityIndex ) );
					}
					else
					{
						CMirrorPropValueRO<sint32> propLX( TheDataset, entityIndex, DSPropertyLocalX );
						CMirrorPropValueRO<sint32> propLY( TheDataset, entityIndex, DSPropertyLocalY );
						CMirrorPropValueRO<sint32> propLZ( TheDataset, entityIndex, DSPropertyLocalZ );
						ap->Position[0] = propLX();
						ap->Position[1] = propLY();
						ap->Position[2] = propLZ();
						ap->IsRelative = true;
						LOG_WHAT_IS_SENT( "%u: C%hu S%hu: Filling RELPOS: %d %d %d mm", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, ap->Position[0], ap->Position[1], ap->Position[2] );
					}

					CActionFactory::getInstance()->packFast( ap, outbox );
					_History->storePosition( clientId, client.sendNumber(), ap, sentity->Mileage, TVPNodeServer::PrioContext.IsTarget, TVPNodeServer::PrioContext.Timestamp );
					//CActionFactory::getInstance()->remove( (CAction*&)ap );
					++client.NbActionsSentAtCycle;
				}

				// Fill the orientation if required
				//currentNode = currentNode->B;
				TVPNodeServer *currentNode = _VisualPropertyTreeRoot->B;

				outbox.serialBitAndLog( currentNode->BranchHasPayload );
				if ( currentNode->BranchHasPayload )
				{
					outbox.serialBitAndLog( currentNode->A->BranchHasPayload );
					if ( currentNode->A->BranchHasPayload )
					{
						//CActionSint64 *ap = (CActionSint64*)(CActionFactory::getInstance()->createByPropIndex( slot, PROPERTY_ORIENTATION ));
						//ap->PropertyCode = PROPERTY_ORIENTATION;	// useless: already set by createByPropIndex
						DECLARE_AP(ORIENTATION);

						CMirrorPropValueRO<float> prop( TheDataset, entityIndex, DSPropertyORIENTATION );
						LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu ORIENT (P%hu) : %.1f", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, prop() );
	
						uint32 value = ( thetaIntMode ? (uint32)(prop()) : *((uint32*)&(prop())) );

						ap->setAndPackValue( value, outbox );
						_History->store( clientId, client.sendNumber(), ap );

						// TODO: send less bits for forage source entity

						//CActionFactory::getInstance()->remove( (CAction*&)ap );
						REMOVE_AP();

						++client.NbActionsSentAtCycle;
					}

					// Fill the required discreet properties if required
					currentNode->B->fillDiscreetProperties( outbox );
					//nlinfo("end fill");
					//TVPNodeServer::fastFillDiscreetProperties( outbox );
					//nlinfo("end fastfill");
				}
			}

#ifdef NL_DEBUG
			if ( _VisualPropertyTreeRoot->A->BranchHasPayload )
			{
				//CClientEntityIdTranslator::CEntityInfo& info = client.IdTranslator.getInfo( slot );	// CHANGED BEN
				//if ( info.AssociationState == CClientEntityIdTranslator::CEntityInfo::AwaitingAssAck )	// CHANGED BEN
				if ( _VisionArray->getAssociationState(clientId, slot) == TPairState::AwaitingAssAck )
				{
					nlwarning( "C%hu S%hu: Sending position but sheet id not sent", clientId, (uint16)slot );
					client.displaySlotProperties( slot );
				}
			}

	//if ( (verbosePropertiesSent==9999) || (verbosePropertiesSent==TVPNodeServer::PrioContext.ClientId) )
	//{
	//	nldebug( "To C%hu", clientId );
	//	outbox.displayStream();
	//}
#endif

			// ******** End of iteration ********
			//_VisualPropertyTreeRoot->displayStatesOfTreeLeaves();
			//nldebug( "C%hu S%hu: end of pair block at bitpos %d", clientId, (uint16)slot, outbox.getPosInBit() );
		}

		// Reset the priority of the pair, even if nothing was filled
		if ( slot != 0 )
			pairState.resetPrio(); // note: nothing can remain to be filled, as we allow to exceed the size limit
	}
}


/*
 * Test the criterion for the position of the entity 'slot' seen by 'clientId'
 */
bool		CDistancePrioritizer::positionHasChangedEnough()
{
	// TEMP: do not send if local mode (because the local position is useless for the mektoub)
	if ( TVPNodeServer::PrioContext.ZCache & 0x1 )
		return false;

	uint32 lastSentMileage = _History->getMileage( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.Slot );
	if ( lastSentMileage != 0 )
	{
		// Calculate difference distance between current and lastsent mileage (unsigned to allow mileage overflow)
		if ( (TVPNodeServer::PrioContext.Sentity->Mileage - lastSentMileage) * _DistanceDeltaRatio > (uint32)(TVPNodeServer::PrioContext.DistanceCE) )
		{
#ifdef NL_DEBUG
			++(TVPNodeServer::PrioContext.ClientHost->MoveNumber);
#endif
			return true;
		}
		else
		{
			if (verbosePropertiesSent==0)
			{
				nldebug("Ignoring move: Mileage=%u, lastSentMileage=%u, _DistanceDeltaRatio=%u, DistanceCE=%u",
					(uint32) TVPNodeServer::PrioContext.Sentity->Mileage,
					(uint32) lastSentMileage,
					(uint32) _DistanceDeltaRatio,
					(uint32) TVPNodeServer::PrioContext.DistanceCE );
			}
			return false;
		}
	}
	else
	{
		// Not sent yet
		return true;
	}
}


/*
 * Test the criterion for thetaIntMode
 */
bool		CDistancePrioritizer::thetaIntModehasChanged(const CPropertyHistory::CPropertyEntry& entry)
{
	if ( ! entityIsWithinDistanceThreshold( PROPERTY_ORIENTATION ) )
		return false;

	if ( entry.HasValue )
	{
		CMirrorPropValueRO<float> currentTheta( TheDataset, TVPNodeServer::PrioContext.EntityIndex, DSPropertyORIENTATION );
		return ( (uint32)currentTheta != (uint32)entry.LastSent );
	}
	else
	{
	return TVPNodeServer::PrioContext.Sentity->propertyIsInitialized( PROPERTY_ORIENTATION, DSPropertyORIENTATION, TVPNodeServer::PrioContext.EntityIndex, (TYPE_ORIENTATION*)NULL );
	}
}


/*
 * Test the criterion for the orientation of the entity 'slot' seen by 'clientId' + initialized
 */
bool		CDistancePrioritizer::orientationHasChangedEnough(const CPropertyHistory::CPropertyEntry& entry, float angleRatio )
{
	if ( ! entityIsWithinDistanceThreshold( PROPERTY_ORIENTATION ) )
		return false;

	if ( entry.HasValue )
	{
		CMirrorPropValueRO<float> currentAngle( TheDataset, TVPNodeServer::PrioContext.EntityIndex, DSPropertyORIENTATION );

/*
		// Orientation is a float angle in radian
 		const float& oldangle = *((float*)&(entry.LastSent));
 		float deltaAngle = (float)fabs( (float)(currentAngle() - oldangle) );
 		deltaAngle = (float)fmod( deltaAngle+(2*Pi), (2*Pi) );

		//nldebug( "getDelta(theta) : dA=%g", deltaAngle );
		return ( deltaAngle > (float)Pi/angleRatio ); // the orientation is useful only when the pos does not change
*/

		float	oldangle;
		entry.getValue(oldangle);
		float	deltaAngle = (float)( Pi - fabs(fmod(currentAngle()-oldangle+4*Pi, 2*Pi)-Pi) );	// deltaAngle is in [0, 2*Pi]

		//nldebug( "getDelta(theta) : dA=%g", deltaAngle );
		return ( deltaAngle*angleRatio > (float)Pi ); // the orientation is useful only when the pos does not change

	}
	else
	{
		// Not sent yet => always sent theta, even if it's zero (anyway, it's unlikely to be exactly 0.0: remind that this initial float is determined, for bots, by leveldesigners with the mouse)
		return true;
	}
}


/*
 * Test the criterion for the specified property of the entity 'slot' seen by 'clientId'
 */
inline bool	CDistancePrioritizer::entityIsWithinDistanceThreshold( TPropIndex propIndex )
{
	// Compare distance with the threshold
	//nldebug( "C%hu - slot %hu - prop %hu: DISTANCE=%d THRESHOL=%d", TVPNodeServer::PrioContext.ClientHost->clientId(), (uint16)TVPNodeServer::PrioContext.Slot, propIndex, TVPNodeServer::PrioContext.DistanceCE, TheEntityTranslator->getDistThreshold( propertyid ) );	
	return ( TVPNodeServer::PrioContext.DistanceCE < getDistThreshold( propIndex ) );
}

/*
 * Special arbitrate case for BEHAVIOUR property
 * Threshold in this case should be the same than the target list threshold when
 * the behaviour is a range attack or projectile behaviour
 */
#ifndef NL_DEBUG
inline 
#endif
void CDistancePrioritizer::arbitrateDiscreetBehaviourProperty(const CPropertyHistory::CEntityEntry& entry, CEntity*	sentity)
{
#ifdef STORE_MIRROR_VP_IN_CLASS
	const CMirrorPropValueRO<TYPE_BEHAVIOUR>& propBehav = sentity->VP_BEHAVIOUR;
#else
	CMirrorPropValueRO<TYPE_BEHAVIOUR> propBehav( TheDataset, TVPNodeServer::PrioContext.EntityIndex, DSPropertyBEHAVIOUR );
#endif
	if (!discreetPropertyHasChanged( entry.Properties[PROPERTY_BEHAVIOUR], propBehav, PROPERTY_BEHAVIOUR, (TYPE_BEHAVIOUR*)NULL ))
	{
		GET_VP_NODE(BEHAVIOUR)->BranchHasPayload = false;
		return;
	}	
	const MBEHAV::CBehaviour &behav = propBehav();
	TPropIndex refDistanceProperty;
	switch(behav.Behaviour)
	{		
		case MBEHAV::CAST_OFF_SUCCESS:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_OFF_LINK:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_CUR_SUCCESS:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_CUR_LINK:		
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_MIX_SUCCESS:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_MIX_LINK:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::RANGE_ATTACK:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_ACID:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_BLIND:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_COLD:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_ELEC:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_FEAR:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_FIRE:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_HEALHP:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_MAD:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_POISON:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_ROOT:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_ROT:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_SHOCK:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_SLEEP:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_SLOW:
			refDistanceProperty = PROPERTY_TARGET_LIST;
		break;
		case MBEHAV::CAST_STUN:
			refDistanceProperty = PROPERTY_TARGET_LIST; // valid distance should be the same than the target list
			                                            // because target list and behaviour are sent together
		break;
		default:
			refDistanceProperty = PROPERTY_BEHAVIOUR;
		break;
	}	
	//
	GET_VP_NODE(BEHAVIOUR)->BranchHasPayload = entityIsWithinDistanceThreshold(refDistanceProperty);	
}


/*
 * Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
 */
void		CDistancePrioritizer::arbitrateAllDiscreetProperties(const CPropertyHistory::CEntityEntry& entry)
{
	H_AUTO(arbitrateAllDiscreetProperties);

	CEntity*	sentity = TVPNodeServer::PrioContext.Sentity;

	arbitrateDiscreetPropertyWithoutThreshold( entry, SHEET );
	arbitrateDiscreetBehaviourProperty( entry, sentity );

	// Don't limit to the distance threshold when triggering sending of the name of the target
	if ( TVPNodeServer::PrioContext.IsTarget )
		arbitrateDiscreetPropertyWithoutThreshold( entry, NAME_STRING_ID );
	else
		arbitrateDiscreetProperty( entry, NAME_STRING_ID );

	//arbitrateDiscreetProperty( entry, TARGET_ID ); // now done in fillOutBox() in "mode switch"
	arbitrateDiscreetProperty( entry, CONTEXTUAL );
	arbitrateDiscreetPropertyWithoutThreshold( entry, MODE );
	arbitrateDiscreetProperty( entry, VPA );
	arbitrateDiscreetProperty( entry, VPB );
	arbitrateDiscreetProperty( entry, VPC );
	arbitrateDiscreetPropertyWithoutThreshold( entry, ENTITY_MOUNTED_ID );
	arbitrateDiscreetPropertyWithoutThreshold( entry, RIDER_ENTITY_ID );

	arbitrateTargetList( entry, TARGET_LIST );
	arbitrateDiscreetProperty( entry, VISUAL_FX );

	arbitrateDiscreetProperty( entry, GUILD_SYMBOL );
	arbitrateDiscreetProperty( entry, GUILD_NAME_ID );
	arbitrateDiscreetProperty( entry, EVENT_FACTION_ID );
	arbitrateDiscreetProperty( entry, PVP_MODE );
	arbitrateDiscreetProperty( entry, PVP_CLAN );
	arbitrateNeverSendProperty( OWNER_PEOPLE );
	arbitrateDiscreetProperty( entry, OUTPOST_INFOS );

	if (TVPNodeServer::PrioContext.Slot != 0)
	{
		arbitrateCommonPosAndMode(entry);
		arbitrateDiscreetProperty( entry, TARGET_ID );
		arbitrateDiscreetProperty( entry, BARS );
	}
	else
	{
		arbitrateSlot0PosAndMode(entry);
		arbitrateDiscreetPropertyWithoutThreshold( entry, TARGET_ID ); // no need of threshold
		// never send BARS for User Player (no need since sent with the USER:BARS message)
		arbitrateNeverSendProperty( BARS );
	}
}

/*
 * Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
 */
void		CDistancePrioritizer::arbitrateNPCDiscreetProperties(const CPropertyHistory::CEntityEntry& entry)
{
	H_AUTO(arbitrateNPCDiscreetProperties);

	CEntity*	sentity = TVPNodeServer::PrioContext.Sentity;

	arbitrateDiscreetPropertyWithoutThreshold( entry, SHEET );
	arbitrateDiscreetBehaviourProperty(entry, sentity);

	// Don't limit to the distance threshold when triggering sending of the name of the target
	if ( TVPNodeServer::PrioContext.IsTarget )
		arbitrateDiscreetPropertyWithoutThreshold( entry, NAME_STRING_ID );
	else
		arbitrateDiscreetProperty( entry, NAME_STRING_ID );

	arbitrateDiscreetProperty( entry, TARGET_ID );	// NPC can never be in Slot 0

	// Specific distance for NPCs' contextual property
	GET_VP_NODE(CONTEXTUAL)->BranchHasPayload =
		TVPNodeServer::PrioContext.DistanceCE < THRESHOLD_CONTEXTUAL_NPC
		&& discreetPropertyHasChanged( entry.Properties[PROPERTY_CONTEXTUAL], sentity->VP_CONTEXTUAL, PROPERTY_CONTEXTUAL, (TYPE_CONTEXTUAL*)NULL );

	arbitrateDiscreetPropertyWithoutThreshold( entry, MODE );
	arbitrateDiscreetProperty( entry, BARS );
	arbitrateDiscreetProperty( entry, VPA );
	arbitrateDiscreetProperty( entry, VPB );
	arbitrateDiscreetProperty( entry, VPC );

	arbitrateDiscreetPropertyWithoutThreshold( entry, ENTITY_MOUNTED_ID );

	arbitrateNeverSendProperty( RIDER_ENTITY_ID );

	arbitrateTargetList( entry, TARGET_LIST );
	arbitrateDiscreetProperty( entry, VISUAL_FX );
	arbitrateDiscreetProperty( entry, GUILD_SYMBOL );
	arbitrateDiscreetProperty( entry, GUILD_NAME_ID );
	arbitrateNeverSendProperty( EVENT_FACTION_ID );
	arbitrateNeverSendProperty( PVP_MODE );
	arbitrateNeverSendProperty( PVP_CLAN );
	arbitrateNeverSendProperty( OWNER_PEOPLE );
	arbitrateDiscreetProperty( entry, OUTPOST_INFOS );

	arbitrateCommonPosAndMode(entry);
}

/*
 * Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
 */
void		CDistancePrioritizer::arbitrateCreatureDiscreetProperties(const CPropertyHistory::CEntityEntry& entry)
{
	H_AUTO(arbitrateCreatureDiscreetProperties);

	CEntity*	sentity = TVPNodeServer::PrioContext.Sentity;

	arbitrateDiscreetPropertyWithoutThreshold( entry, SHEET );
	arbitrateDiscreetBehaviourProperty(entry, sentity);

	// Don't limit to the distance threshold when triggering sending of the name of the target
	if ( TVPNodeServer::PrioContext.IsTarget )
		arbitrateDiscreetPropertyWithoutThreshold( entry, NAME_STRING_ID );
	else
		arbitrateDiscreetProperty( entry, NAME_STRING_ID );

	arbitrateDiscreetProperty( entry, TARGET_ID ); // Creature can never be in Slot 0
	arbitrateDiscreetProperty( entry, CONTEXTUAL );
	arbitrateDiscreetPropertyWithoutThreshold( entry, MODE );
	arbitrateDiscreetProperty( entry, BARS );

	arbitrateNeverSendProperty( VPA );
	arbitrateDiscreetProperty( entry, VPB );
	arbitrateNeverSendProperty( VPC );
	arbitrateNeverSendProperty( ENTITY_MOUNTED_ID );

	arbitrateDiscreetPropertyWithoutThreshold( entry, RIDER_ENTITY_ID );
	arbitrateTargetList( entry, TARGET_LIST );

	arbitrateNeverSendProperty( VISUAL_FX );
	arbitrateNeverSendProperty( GUILD_SYMBOL );
	arbitrateNeverSendProperty( GUILD_NAME_ID );
	arbitrateNeverSendProperty( EVENT_FACTION_ID );
	arbitrateNeverSendProperty( PVP_MODE );
	arbitrateNeverSendProperty( PVP_CLAN );
	arbitrateDiscreetProperty( entry, OWNER_PEOPLE );
	arbitrateNeverSendProperty( OUTPOST_INFOS );

	arbitrateCommonPosAndMode(entry);
}

/*
 * Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
 */
void		CDistancePrioritizer::arbitrateForageSourceDiscreetProperties(const CPropertyHistory::CEntityEntry& entry)
{
	H_AUTO(arbitrateForageSourceDiscreetProperties);

	CEntity*	sentity = TVPNodeServer::PrioContext.Sentity;

	arbitrateDiscreetPropertyWithoutThreshold( entry, SHEET );
	arbitrateNeverSendProperty( BEHAVIOUR );

	arbitrateDiscreetProperty( entry, NAME_STRING_ID );

	arbitrateDiscreetProperty( entry, TARGET_ID );	// ForageSource can never be in Slot 0
	arbitrateDiscreetProperty( entry, CONTEXTUAL );

	arbitrateNeverSendProperty( MODE );

	arbitrateDiscreetProperty( entry, BARS );

	arbitrateNeverSendProperty( VPA );
	arbitrateNeverSendProperty( VPB );
	arbitrateNeverSendProperty( VPC );
	arbitrateNeverSendProperty( ENTITY_MOUNTED_ID );
	arbitrateNeverSendProperty( RIDER_ENTITY_ID );

	arbitrateTargetList( entry, TARGET_LIST );
	arbitrateDiscreetProperty( entry, VISUAL_FX );

	arbitrateNeverSendProperty( GUILD_SYMBOL );
	arbitrateNeverSendProperty( GUILD_NAME_ID );
	arbitrateNeverSendProperty( EVENT_FACTION_ID );
	arbitrateNeverSendProperty( PVP_MODE );
	arbitrateNeverSendProperty( PVP_CLAN );
	arbitrateNeverSendProperty( OWNER_PEOPLE );
	arbitrateNeverSendProperty( OUTPOST_INFOS );
	
	GET_VP_NODE(POSITION)->BranchHasPayload = sentity->positionIsInitialized() && positionHasChangedEnough();
	GET_VP_NODE(ORIENTATION)->BranchHasPayload = thetaIntModehasChanged(entry.Properties[PROPERTY_ORIENTATION]);
}


/*
 * Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
 */
inline void		CDistancePrioritizer::arbitrateCommonPosAndMode(const CPropertyHistory::CEntityEntry& entry)
{
	// Position if changed enough (if not carried by mode)
	// Orientation if changed > 60 degrees (it's head angle only) or in first block (useful for static entities such as bot objects)
	bool	modeIsChanging = GET_VP_NODE(MODE)->BranchHasPayload;
	bool	sheetIsChanging = GET_VP_NODE(SHEET)->BranchHasPayload;
	bool	posIsReady = TVPNodeServer::PrioContext.Sentity->positionIsInitialized();

	GET_VP_NODE(POSITION)->BranchHasPayload = (!modeIsChanging) && posIsReady && positionHasChangedEnough();
	GET_VP_NODE(ORIENTATION)->BranchHasPayload = (sheetIsChanging && posIsReady) || orientationHasChangedEnough( entry.Properties[PROPERTY_ORIENTATION],  36.0f ); // 5 degrees
}

/*
 * Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
 */
inline void		CDistancePrioritizer::arbitrateSlot0PosAndMode(const CPropertyHistory::CEntityEntry& entry)
{
	arbitrateNeverSendProperty(POSITION);
	arbitrateNeverSendProperty(ORIENTATION);
}


/*
 *
 */
inline void		CDistancePrioritizer::serialSlotHeader( CClientHost& client, CEntity *sentity, TPairState& pairState, CLFECOMMON::TCLEntityId slot, TOutBox& outbox )
{
	// Slot (8 bits)
#ifdef NL_DEBUG
	sint beginbitpos = outbox.getPosInBit();
#endif
	outbox.serialAndLog1( slot );

	// Association change bits (2 bits)
	uint32 associationBits = (uint32)pairState.AssociationChangeBits;
	outbox.serialAndLog2( associationBits, 2 );
	if ( pairState.AssociationChangeBits != pairState.PrevAssociationBits )
	{
		//LOG_WHAT_IS_SENT( "slot %hu ab %u beginpos %d endpos %d beginbitpos %d endbitpos %d", (uint16)slot, associationBits, beginbitpos/8, outbox.getPosInBit()/8, beginbitpos, outbox.getPosInBit() );
		pairState.PrevAssociationBits = pairState.AssociationChangeBits; // & 0x3;
		_History->storeDisassociation( client.clientId(), slot, client.sendNumber(), pairState.AssociationChangeBits );
		//	pairState.AssociationChangeBits &= 0x7F;
	}


	// Timestamp (1 or 5 bits, depending on the type of entity) (TVPNodeServer::PrioContext.Timestamp initialized to 0)
	uint32 timestampDelta = 0;
	if ( sentity )
	{
		const CEntityId& seenEntityId = TheDataset.getEntityId( TVPNodeServer::PrioContext.EntityIndex );
		if ( seenEntityId.getType() == RYZOMID::player )
		{
			// For players, always set the timestamp delta, using TickPosition
			// Note: discreet property change times won't be accurate
			TVPNodeServer::PrioContext.Timestamp = sentity->TickPosition;
			timestampDelta = CTickEventHandler::getGameCycle() - sentity->TickPosition;
			if ( timestampDelta > 15 ) // clamp to 4bit
				timestampDelta = 15;
			timestampDelta |= 0x10; // 'timestampIsThere bit': first bit is bit 5 (high to low order)
		}
		else if ( seenEntityId.getType() >= RYZOMID::object )
		{
			// For non-players/non-bots types (e.g. bags), set the timestamp delta if entity is being spawned to the client
			//if ( _VisualPropertyTreeRoot->B->B->getSHEETnode()->BranchHasPayload ) // assumes this is done after arbitrateDiscreetProperties() // CHANGED BEN
			if ( GET_VP_NODE(SHEET)->BranchHasPayload ) // assumes this is done after arbitrateDiscreetProperties()
			{
				TVPNodeServer::PrioContext.Timestamp = TheDataset.getOnlineTimestamp( TVPNodeServer::PrioContext.EntityIndex );
				timestampDelta = CTickEventHandler::getGameCycle() - TVPNodeServer::PrioContext.Timestamp;
				if ( timestampDelta > 15 ) // clamp to 4bit
					timestampDelta = 15;
				timestampDelta |= 0x10; // 'timestampIsThere bit': first bit is bit 5 (high to low order)
			}
		}
		// For bots, the timestamp is not needed, the client will take _ServerGameCycle
	}

	outbox.serialAndLog2( timestampDelta, (timestampDelta!=0) ? 5 : 1 );
}

#ifdef STORE_MIRROR_VP_IN_CLASS

#define caseFillAction( name )	ap->setValue64( TVPNodeServer::PrioContext.Sentity->VP_##name() );

#else // STORE_MIRROR_VP_IN_CLASS

#define caseFillAction( name ) \
	CMirrorPropValueRO<TYPE_##name> prop( TheDataset, TVPNodeServer::PrioContext.EntityIndex, DSProperty##name ); \
	ap->setValue64( prop() );

#endif // STORE_MIRROR_VP_IN_CLASS






/*
 * SHEET
 */
void		fillSHEET( TOutBox& outbox, TPropIndex )
{
#ifdef NL_DEBUG
	// In non-debug cases, it is done in CVisionProvider::addPair()
	//CClientEntityIdTranslator::CEntityInfo& info = TVPNodeServer::PrioContext.ClientHost->IdTranslator.getInfo( TVPNodeServer::PrioContext.Slot );	// CHANGED BEN
	//info.AssociationState = CClientEntityIdTranslator::CEntityInfo::NormalAssociation;	// CHANGED BEN
	TVPNodeServer::PrioContext.Prioritizer->getVisionArray()->setAssociationState(	TVPNodeServer::PrioContext.ClientId,
																					TVPNodeServer::PrioContext.Slot,
																					TPairState::NormalAssociation);
#endif
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );

	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_SHEET ); // CHANGED BEN
	DECLARE_AP(SHEET);

	// Pack sheet and compressed row into the action
#ifdef STORE_MIRROR_VP_IN_CLASS
	CMirrorPropValueRO<TYPE_SHEET>& prop = TVPNodeServer::PrioContext.Sentity->VP_SHEET;
#else
	CMirrorPropValueRO<TYPE_SHEET> prop( TheDataset, TVPNodeServer::PrioContext.EntityIndex, DSPropertySHEET );
#endif

	uint32 sheetValue = prop();
	TDataSetIndex compressedRow = TVPNodeServer::PrioContext.EntityIndex.getCompressedIndex();
	uint64 value = (uint64)sheetValue | (((uint64)compressedRow) << 32);
	ap->setValue64( value );
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu SHEET at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_SHEET, outbox.getPosInBit(), ap->getValue() );

	// Add row into the action
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);

	// Include alias if non-null in mirror (only for mission giver NPCs)
	CMirrorPropValueRO<TYPE_ALIAS> aliasProp( TheDataset, TVPNodeServer::PrioContext.EntityIndex, DSPropertyNPC_ALIAS );
	if (aliasProp() != 0)
	{
		bool aliasBit = true;
		outbox.serialBitAndLog( aliasBit );
		outbox.serialAndLog1( const_cast<TYPE_ALIAS&>(aliasProp()) ); // no need to store in history, alias never changes for an entity
	}
	else
	{
		bool aliasBit = false;
		outbox.serialBitAndLog( aliasBit );
	}

#ifdef TEST_LOST_PACKET
	if ( TestPacketLost.get() )
	{
		nldebug( "This SHEET sending will be dropped..." );
		TestPacketLostTimer = CTickEventHandler::getGameCycle() + 10;
		TestPacketLost = false;
		TestPacketLostSlot = TVPNodeServer::PrioContext.Slot;
	}
#endif
}


/*
 * BEHAVIOUR
 */
void		fillBEHAVIOUR( TOutBox& outbox, TPropIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_BEHAVIOUR );
	DECLARE_AP(BEHAVIOUR);
	caseFillAction( BEHAVIOUR )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu BEHAVIOUR at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_BEHAVIOUR, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}


/*
 * NAME_STRING_ID
 */
void		fillNAME_STRING_ID( TOutBox& outbox, TPropIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_NAME_STRING_ID );
	DECLARE_AP(NAME_STRING_ID);
	caseFillAction( NAME_STRING_ID )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu NAME_STRING_ID at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_NAME_STRING_ID, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}

/*
 * TARGET_LIST
 */
static vector<TCLEntityId>	TargetSlotsList(256);

NLMISC_COMMAND(displayTargetList,"Display the target list of an entity","<entityId>")
{
	if ( args.size() > 1 )
		return false;

	CEntityId	entity;

	entity.fromString(args[0].c_str());

	CMirrorPropValueList<uint32>			targets(TheDataset, 
														entity,
														"TargetList");
	CMirrorPropValueList<uint32>::iterator	it;

	it = targets.begin();

	if (it != targets.end())
	{
		const uint32	*effectCycle = &((*it)());
		log.displayNL("TargetStamp: %d", *effectCycle);
		++it;
	}

	for (; it!=targets.end(); ++it)
	{
		uint32		index = (*it)();
		log.displayNL("Target: %d", TDataSetRow::createFromRawIndex(index).getIndex());
	}

	return true;
}


void		fillTARGET_LIST( TOutBox& outbox, TPropIndex )
{
	CClientHost	*client = TVPNodeServer::PrioContext.ClientHost;

	CMirrorPropValueList<uint32>			targets(TheDataset, 
														TVPNodeServer::PrioContext.EntityIndex,
														DSPropertyTARGET_LIST );
	CMirrorPropValueList<uint32>::iterator	it;

	TargetSlotsList.clear();

	it = targets.begin();

	for (; it!=targets.end(); )
	{
		TDataSetRow		index = TDataSetRow::createFromRawIndex((*it)());

		++it;
		// check list overflow
		if (it == targets.end())
			break;

		// distance to target (in 1/127 of 100m)
		uint32	dt = *(&((*it)()));

		// translate slot
		TCLEntityId		slot = client->IdTranslator.getCEId(index);
		if (slot != INVALID_SLOT)
		{
			TargetSlotsList.push_back(slot);
			TargetSlotsList.push_back((uint8)dt);
		}

		++it;
	}

	// serialises branch has payload
	bool	payLoad = true;
	outbox.serialBitAndLog(payLoad);

	// restricts to 256 entities
	uint	longListSize = (uint)TargetSlotsList.size();
	if (longListSize > 32)
		longListSize = 32;

	uint8	listSize = (uint8)longListSize;

	// serialises short size
	outbox.serialAndLog1(listSize);

	// serialises slot list
	if (listSize > 0)
		outbox.serialBuffer(&(TargetSlotsList[0]), listSize);

	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_TARGET_LIST );
	DECLARE_AP(TARGET_LIST);
	ap->setValue64( TheDataset.getChangeTimestamp( DSPropertyTARGET_LIST, TVPNodeServer::PrioContext.EntityIndex ) );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();

	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu TARGET_LIST at bitpos %d", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_TARGET_LIST, outbox.getPosInBit() );
}


/*
 * BARS
 */
void		fillBARS( TOutBox& outbox, TPropIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_BARS );
	DECLARE_AP(BARS);
	caseFillAction( BARS )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu BARS at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_BARS, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}


/*
 * VPA, VPB, VPC
 * Assumes:
 * - They have the same mirror size
 * - There mirror dataset property index is contiguous (VPC = VPB + 1 = VPA + 2)
 */
void		fillVisualPropertyABC( TOutBox& outbox, TPropIndex propIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, propIndex );
	DECLARE_AP_INDEX(propIndex);
	CMirrorPropValueRO<TYPE_VPA> prop( TheDataset, TVPNodeServer::PrioContext.EntityIndex, propIndex-PROPERTY_VPA+DSPropertyVPA ); \
	ap->setValue64( prop() );
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu %s at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, propIndex, CLFECOMMON::getPropText( propIndex ), outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}


/*
 * CONTEXTUAL
 */
void		fillCONTEXTUAL( TOutBox& outbox, TPropIndex propIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_CONTEXTUAL );
	DECLARE_AP(CONTEXTUAL);
	caseFillAction( CONTEXTUAL )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu CONTEXTUAL at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_CONTEXTUAL, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}

/*
 * VISUAL_FX
 */
void		fillVISUAL_FX( TOutBox& outbox, TPropIndex propIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_VISUAL_FX );
	DECLARE_AP(VISUAL_FX);
	caseFillAction( VISUAL_FX )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu VISUAL_FX at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_VISUAL_FX, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}

/*
 * MODE
 */
void		fillMODE( TOutBox& outbox, TPropIndex )
{
	// Fill for mode special case
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_MODE );
	DECLARE_AP(MODE);
	uint64 modeLong; // uses 8+4+16+16 = 44 bits

	// Mode value (on 8 bits)
	CMirrorPropValue<MBEHAV::TMode> prop( TheDataset, TVPNodeServer::PrioContext.EntityIndex, DSPropertyMODE );

	// Store in history
	ap->setValue64( prop().RawModeAndParam );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );

	// Game cycle when mode changed (4 bits -> 1.6 second max)
	uint32 tickModeDelta = CTickEventHandler::getGameCycle() - prop.getTimestamp();
	if ( tickModeDelta > 15 ) tickModeDelta = 15;

	// Pack all with position 2D when mode changed, or combat angle (16 bits * 2)
/*
	// OBSOLETE: MBEHAV::COMBAT_FLOAT no longer used
	if ( prop().Mode == MBEHAV::COMBAT_FLOAT )
	{
		uint64 theta = (uint64)(*(uint32*)&(prop().Theta));
		modeLong = ((uint64)(prop().Mode)) | ((uint64)(tickModeDelta << 8)) | (theta << 12);
		LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu MODE at bitpos %d : %u [theta=%g dt=%u]", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_MODE, outbox.getPosInBit(), prop().Mode, prop().Theta, tickModeDelta );
	}
	else
*/
	{
		uint64 posModeX16, posModeY16;
		if ( TVPNodeServer::PrioContext.PositionAlreadySent )
		{
			// Take the pos from the mode change
			posModeX16 = prop().Pos.X16;
			posModeY16 = prop().Pos.Y16;
		}
		else
		{
			// Take the current pos
			posModeX16 = TVPNodeServer::PrioContext.Sentity->X() >> 4; // TODO: make a method for this formula
			posModeY16 = TVPNodeServer::PrioContext.Sentity->Y() >> 4;
			tickModeDelta = 1;
		}
		modeLong = ((uint64)((uint8)(prop().Mode))) | ((uint64)(tickModeDelta << 8)) | (posModeX16 << 12) | (posModeY16 << 28);
		LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu MODE at bitpos %d : %u [x16=%hu y16=%hu dt=%u] %s", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_MODE, outbox.getPosInBit(), prop().Mode, prop().Pos.X16, prop().Pos.Y16, tickModeDelta, TVPNodeServer::PrioContext.PositionAlreadySent?"":" WithFirstPos" );
		
		// The pos might be null (if mode set before 1st pos). The client has to handle the case.
/*#ifdef NL_DEBUG
		if ( posModeX16==0 || posModeY16==0 )
			nlwarning( "E%d: The pos16 in the mode is %hu %hu", TVPNodeServer::PrioContext.EntityIndex, (uint16)posModeX16, (uint16)posModeY16 );
#endif*/
	}

	// Fill
	ap->setAndPackValue( modeLong, outbox );
	//CActionFactory::getInstance()->remove( (CAction*&)act );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}

/*
 * GUILD_NAME_ID
 */
void		fillGUILD_NAME_ID( TOutBox& outbox, TPropIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_GUILD_NAME_ID );
	DECLARE_AP(GUILD_NAME_ID);
	caseFillAction( GUILD_NAME_ID )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu GUILD_NAME_ID at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_GUILD_NAME_ID, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}

/*
 * GUILD_SYMBOL
 */
void		fillGUILD_SYMBOL( TOutBox& outbox, TPropIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_GUILD_SYMBOL );
	DECLARE_AP(GUILD_SYMBOL);
	caseFillAction( GUILD_SYMBOL )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu GUILD_SYMBOL at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_GUILD_SYMBOL, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}


/*
 * EVENT_FACTION_ID
 */
void		fillEVENT_FACTION_ID( TOutBox& outbox, TPropIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_EVENT_FACTION_ID );
	DECLARE_AP(EVENT_FACTION_ID);
	caseFillAction( EVENT_FACTION_ID )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu EVENT_FACTION_ID at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_EVENT_FACTION_ID, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}


/*
 * PVP_MODE
 */
void		fillPVP_MODE( TOutBox& outbox, TPropIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_PVP_MODE );
	DECLARE_AP(PVP_MODE);
	caseFillAction( PVP_MODE )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu PVP_MODE at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_PVP_MODE, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}


/*
 * PVP_CLAN
 */
void		fillPVP_CLAN( TOutBox& outbox, TPropIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, PROPERTY_PVP_CLAN );
	DECLARE_AP(PVP_CLAN);
	caseFillAction( PVP_CLAN )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu PVP_CLAN at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_PVP_CLAN, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	//CActionFactory::getInstance()->remove( (CAction*&)ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}


/*
 * OWNER_PEOPLE
 */
void		fillOWNER_PEOPLE( TOutBox& outbox, TPropIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	DECLARE_AP(OWNER_PEOPLE);
	caseFillAction( OWNER_PEOPLE )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu OWNER_PEOPLE at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_OWNER_PEOPLE, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}


/*
 * OUTPOST_INFOS
 */
void		fillOUTPOST_INFOS( TOutBox& outbox, TPropIndex )
{
	bool payloadBit = true;
	outbox.serialBitAndLog( payloadBit );
	DECLARE_AP(OUTPOST_INFOS);
	caseFillAction( OUTPOST_INFOS )
	LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu OUTPOST_INFOS at bitpos %d - value %"NL_I64"u", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, PROPERTY_OUTPOST_INFOS, outbox.getPosInBit(), ap->getValue() );
	ap->packFast( outbox );
	CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );
	REMOVE_AP();
	++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
}


/*
 * TARGET_ID, ENTITY_MOUNTED, RIDER_ENTITY
 */
void		fillRowProperty( TOutBox& outbox, TPropIndex propIndex )
{
	bool payloadBit = true;
	TCLEntityId slot;
	CMirrorPropValueRO<TDataSetRow> prop( TheDataset, TVPNodeServer::PrioContext.EntityIndex, CEntityContainer::propertyIndexInDataSetToVisualPropIndex( propIndex ) );
	TEntityIndex	targetindex(prop());
	if ( !targetindex.isValid() )
	{
		// No target
		slot = INVALID_SLOT;
	}
	else
	{
		TEntityIndex seenEntityIndex = TVPNodeServer::PrioContext.EntityIndex;
		const char *propName = getPropText( propIndex );
		LOG_WHAT_IS_SENT( "%u: About to send client %hu (%s) the %s '%u --> %u'", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientHost->clientId(), TVPNodeServer::PrioContext.ClientHost->eId().toString().c_str(), propName, seenEntityIndex.getIndex(), targetindex.getIndex() );
		if ( targetindex == TVPNodeServer::PrioContext.ClientHost->entityIndex() )
		{
			// The entity targets the client
			slot = 0;
		}
		else if ( targetindex == seenEntityIndex )
		{
			// The entity targets itself
			slot = INVALID_SLOT;
		}
		else
		{
			// Translate CEntityId to slot: get entityid, then slot
			TCLEntityId	result = TVPNodeServer::PrioContext.ClientHost->IdTranslator.getCEId( targetindex );
			if ( result != INVALID_SLOT )
			{
				slot = result;
			}
			else
			{
				LOG_WHAT_IS_SENT( "%s slot not found: E%u", propName, targetindex.getIndex() );
				payloadBit = false;
			}
		}
	}

	// Fill for property target/mount special case
	outbox.serialBitAndLog( payloadBit );
	if ( payloadBit )
	{
		LOG_WHAT_IS_SENT( "%u: Filling buffer for C%hu S%hu P%hu %s at bitpos %d - slot %hu", CTickEventHandler::getGameCycle(), TVPNodeServer::PrioContext.ClientId, (uint16)TVPNodeServer::PrioContext.Slot, propIndex, CLFECOMMON::getPropText( propIndex ), outbox.getPosInBit(), (uint16)slot );
		//CActionSint64 *ap = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( TVPNodeServer::PrioContext.Slot, propIndex );
		DECLARE_AP_INDEX(propIndex);
		ap->setValue64( prop().getIndex() );

		// Store the entity index into the history
		CFrontEndService::instance()->history()->store( TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.ClientHost->sendNumber(), ap );

		// The value that will be sent is the slot, not the entity index
		ap->setAndPackValue( slot, outbox );

		//CActionFactory::getInstance()->remove( (CAction*&)ap );
		REMOVE_AP();
		++(TVPNodeServer::PrioContext.ClientHost->NbActionsSentAtCycle);
	}
}


TVPNodeServer::TPrioContext								TVPNodeServer::PrioContext;

// Flattened Node Tree
TVPNodeServer*											TVPNodeServer::FlatVPTree[CLFECOMMON::NB_VISUAL_PROPERTIES];

// Reordered Node tree (used by fastPropagateBackBranchHasPayload())
std::vector<TVPNodeServer::CSortedFlatVPTreeItem>		TVPNodeServer::SortedFlatVPTree;
const bool												TVPNodeServer::FalseBoolPayLoad = false;

// Reordered Node tree (used by fastFillDiscreetProperties)
std::vector<TVPNodeServer::CSortedFlatVPTreeFillItem>	TVPNodeServer::SortedFlatVPTreeFill;

void	TVPNodeServer::initSortedFlatVPTree()
{
	if (isLeaf())
		return;

	CSortedFlatVPTreeItem	item;

	item.Node = this;

	if (a())
	{
		item.APayLoad = &(a()->BranchHasPayload);
		a()->initSortedFlatVPTree();
	}
	if (b())
	{
		item.BPayLoad = &(b()->BranchHasPayload);
		b()->initSortedFlatVPTree();
	}

	SortedFlatVPTree.push_back(item);
}

void	TVPNodeServer::initSortedFlatVPTreeFill()
{
	uint	thisItem = (uint)SortedFlatVPTreeFill.size();
	SortedFlatVPTreeFill.push_back(CSortedFlatVPTreeFillItem());

	SortedFlatVPTreeFill[thisItem].Node = this;

	if (a())	a()->initSortedFlatVPTreeFill();
	if (b())	b()->initSortedFlatVPTreeFill();

	SortedFlatVPTreeFill[thisItem].NextIfNoPayload = (uint)SortedFlatVPTreeFill.size();
}


namespace CLFECOMMON
{
	// Factory for TVPNodeBase::buildTree()
	TVPNodeBase	*NewNode()
	{
		return (TVPNodeBase*) new TVPNodeServer();
	}
};


#ifdef NL_DEBUG

NLMISC_DYNVARIABLE( uint32, MoveNumber, "MoveNumber of entities seen by monitored client" )
{
	if ( get )
	{
		CFrontEndService *fe = CFrontEndService::instance();
		//nlassert( fe->MonitoredClient <= MAX_NB_CLIENTS );
		CClientHost *client = fe->receiveSub()->clientIdCont()[fe->MonitoredClient];
		if ( client )
		{
			*pointer = client->MoveNumber;
		}
		else
		{
			*pointer = 9999;
		}
	}
}

#endif


NLMISC_COMMAND(verbosePropertiesSent,"Turn on or off or check the state of verbose logging of what is sent","<clientId> | all | off")
{
	if ( args.size() > 1 )
		return false;

	if ( args.size() == 1 )
	{
		if ( args[0] == string("all") )
			verbosePropertiesSent = 0;
		else if ( args[0] == string("off") )
			verbosePropertiesSent = INVALID_CLIENT;
		else
			NLMISC::fromString(args[0], verbosePropertiesSent);
	}

	log.displayNL( "verbosePropertiesSent is %s", (verbosePropertiesSent==INVALID_CLIENT)?"off":((verbosePropertiesSent==0)?"all":toString("C%hu", verbosePropertiesSent).c_str()) );
	return true;
}











