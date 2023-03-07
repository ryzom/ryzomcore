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

#include "prioritizer.h"
#include "vision_array.h"
#include "property_dispatcher.h"
#include "property_id_translator.h"
#include "history.h"
#include "frontend_service.h"
#include "fe_stat.h"

using namespace CLFECOMMON;



/* Threshold for Distance/Delta.
 * Ex: RATIO_MAX_THRESHOLD = 2
 *
 *      Delta
 *   |--------|--------|
 *  D|L.......H......H/
 *  i|              /
 *  s|            /
 *  t|          /
 *  a|        /
 *  n|      /
 *  c|45  /
 *  e|  /
 *   |/
 *   o
 * Observer
 *
 * If Delta exceeds Distance/2, the priority shelf (bucket) is the highest one (H)
 * Else the shelves cover linearly the length between 0 and Distance/2.
 *
 * Note: the highest priority is 0, the lowest is NB_PRIORITIES-1.
 *
 */
const sint RATIO_MAX_THRESHOLD = 2;


// Deprecated
/*
const TCoord CPrioritizer::DeltaMinThreshold = 200;		// 20 cm (all smaller deltas will trigger min priority)
const TCoord CPrioritizer::DeltaMaxThreshold = 12000;	// 12 m (all higher deltas will trigger max priority)
const TCoord CPrioritizer::DistMinThreshold = 500;		// 50 cm (all nearer distances will trigger max priority if moving)
const TCoord CPrioritizer::DistMaxThreshold = 600000;	// 600 m (all further distances will trigger min priority)
*/

const TCoord CPrioritizer::MaxDelta = 2000000; // about 2000 m


/*
 * Constructor
 */
CPrioritizer::CPrioritizer() :
	_PrioStrategy( DistanceDelta ),
	_VisionArray( NULL ),
	_PropTranslator( NULL ),
	_History( NULL )
{
}


/*
 * Initialization
 */
void				CPrioritizer::init( CVisionArray *va,
										CPropertyIdTranslator* pt, CHistory *h )
{
	_VisionArray = va;
	_PropTranslator = pt;
	_History = h;

	PositionSpreader.init();
	OrientationSpreader.init();
	DiscreetSpreader.init();
}


/*
 * Set the priority strategy
 */
/*void				CPrioritizer::setPrioStrategy( TPrioStrategy ps )
{
	_PrioStrategy = ps;
	switch( _PrioStrategy )
	{
	case DistanceOnly:
		MaxRatio = (float)DistMaxThreshold;
		break;
	case DistanceDelta:
		MaxRatio = DistMaxThreshold / DeltaMinThreshold; // distance/delta, const max
		break;
	default:
		nlwarning( "Invalid priority strategy set, resetting to DistanceDelta" );
		setPrioStrategy( DistanceDelta );
	}
}*/


/*
 * Calculate the priorities for the current cycle
 */
void				CPrioritizer::calculatePriorities()
{
	/*switch( _PrioStrategy )
	{
	case DistanceOnly:
		calculatePrioDistance();
		break;
	case DistanceDelta:*/
	
	calculatePriorityOfPosition();
	calculatePriorityOfOrientation();
	calculatePriorityOfDiscreet();

	/*	break;
	default:
		nlstop;
	}*/
	
}


/*
 * Calculate using strategy DistanceOnly
 */
/*void				CPrioritizer::calculatePrioDistance()
{
	
	// Process the pairs selected by the Pair Selector

	uint32 uprio; // using uint32 because uint8 can be too short in some cases

	const TPairCE *pairCE;
	while ( (pairCE = _PairSelector->selectNextPair()) != NULL )
	{
		CVisionArray::TVAItem& item = _VisionArray->vaItem( pairCE->ClientId, pairCE->CeId );

		// Discard if client has left
		if ( item.DistanceCE != DISCARD_PAIR )
		{
			// For each property corresponding to the entity type
			TPropIndex p;
			for ( p=0; p<_PropTranslator->nbProperties( item.EntityIndex ); ++p )
			{
				// Calculate priority only if the update status is ToUpdate,
				// meaning that a change has not been sent to the client (especially
				// for a discreet property ; for a continuous property the status
				// is always ToUpdate)
				//
				if ( item.Properties[p].UpdateStatus == ToUpdate )
				{
					CClientHost *clienthost = CFrontEndService::instance()->sendSub()->clientIdCont()[pairCE->ClientId];
					//nlassertex( clienthost, ("c%hu", clientid) );
					float hpthreshold = clienthost->PropDispatcher.hpThreshold();

					// Clamp distance so that very near entities get the highest priority
					if ( (float)(item.DistanceCE) < hpthreshold )
					{
						uprio = HIGHEST_PRIORITY;
					}
					else
					{
						// Linear priority, excluding HIGHEST_PRIORITY
						nlassert( MaxRatio > hpthreshold );
						uprio = (uint32)((((float)item.DistanceCE-hpthreshold) * (float)(NB_PRIORITIES-2) / (MaxRatio-hpthreshold)) + 1.0f);
						nlassert( uprio > HIGHEST_PRIORITY );

						// Clamp priority
						if ( uprio >= NB_PRIORITIES )
						{
							uprio = NB_PRIORITIES - 1;
						}
					}

					// Set the priority of the pair
					clienthost->PropDispatcher.setPriority( pairCE->CeId, p, _VisionArray->prioLoc( pairCE->ClientId, pairCE->CeId, p ), (TPriority)uprio );
				}
			}
		}
	}
}*/






/*
 * Calculate position priorities using strategy DistanceDelta
 */
void				CPrioritizer::calculatePriorityOfPosition()
{
	if ( PositionSpreader.mustProcessNow() )
	{

#ifdef MEASURE_FRONTEND_TABLES
		DistCntFrame.setGameTick();
		DeltaCntFrame.setGameTick();
		PrioCntFrame.setGameTick();
		DistCntFrame.reset( -1 );
		DeltaCntFrame.reset( -1 );
		PrioCntFrame.reset( -1 );
#endif

		THostMap::iterator icm;
		sint clientmapindex, outerBoundIndex;
		PositionSpreader.getProcessingBounds( icm, clientmapindex, outerBoundIndex );
		sint bucket;

		while ( clientmapindex < outerBoundIndex )
		{
			CClientHost *clienthost = GETCLIENTA(icm);
		    TVAProp tvaPos = _VisionArray->tvaProp( clienthost->clientId(), PROPERTY_POSITION );

			for ( sint e=0; e!=MAX_SEEN_ENTITIES_PER_CLIENT; ++e )
			{
				// Calculate priority only if the update status is ToUpdate (even for a continuous prop)
				if ( tvaPos[e].PropState.UpdateStatus == ToUpdate )
				{
					TEntityIndex entityIndex = _VisionArray->getEntityIndex( clienthost->clientId(), (TCLEntityId)e );

					if ( entityIndex.isValid() )
					{

						// Get delta
						TCoord delta = getDeltaPos( entityIndex, clienthost->clientId(), e );

#ifdef MEASURE_FRONTEND_TABLES
						if ( clienthost->clientId() == 1 )
						{
							DistCntFrame.SeenEntities[e] = distanceCE;
							DeltaCntFrame.SeenEntities[e] = delta;
						}
#endif

						// Do not set priority if delta is zero (steady entities...)
						if ( delta != 0 )
						{
							TCoord distanceCE = _VisionArray->distanceCE( clienthost->clientId(), (TCLEntityId)e );
							float prioRatio = (float)distanceCE / (float)delta;

							// See explanation of formula on top of this file
							if ( prioRatio < RATIO_MAX_THRESHOLD )
							{
								bucket = HIGHEST_PRIORITY;
							}
							else
							{
								bucket = LOWEST_PRIORITY - (sint)((float)(LOWEST_PRIORITY*RATIO_MAX_THRESHOLD) / prioRatio);
							}

#ifdef MEASURE_FRONTEND_TABLES
							if ( clienthost->clientId() == 1 )
							{
								PrioCntFrame.SeenEntities[e] = bucket;
							}
#endif
						
							// Set the priority of the pair
							clienthost->PropDispatcher.setPriority( e, PROPERTY_POSITION, tvaPos[e].PropState, (TPriority)bucket );

							/*nlinfo( "FEPRIO: Set cep %hu-%hu-%hu to priority %u",
									(uint16)pairCE->ClientId, (uint16)pairCE->CeId, (uint16)p, uprio );*/
						}
					}
				}
			}

			++clientmapindex;
			++icm;
		}

		PositionSpreader.endProcessing( icm );

#ifdef MEASURE_FRONTEND_TABLES
		DistCntFrame.commit( DistCntClt1 );
		DeltaCntFrame.commit( DeltaCntClt1 );
		PrioCntFrame.commit( PrioCntClt1 );
#endif

	}

	PositionSpreader.incCycle();
}


/*
 * Calculate orientation priorities using strategy DistanceDelta
 */
void				CPrioritizer::calculatePriorityOfOrientation()
{
	if ( OrientationSpreader.mustProcessNow() )
	{
		THostMap::iterator icm;
		sint clientmapindex, outerBoundIndex;
		OrientationSpreader.getProcessingBounds( icm, clientmapindex, outerBoundIndex );
		sint bucket;

		while ( clientmapindex < outerBoundIndex )
		{
			CClientHost *clienthost = GETCLIENTA(icm);
			TVAProp tvaOrt = _VisionArray->tvaProp( clienthost->clientId(), PROPERTY_ORIENTATION );

			for ( sint e=0; e!=MAX_SEEN_ENTITIES_PER_CLIENT; ++e )
			{
				// Calculate priority only if the update status is ToUpdate (even for a continuous prop)
				if ( tvaOrt[e].PropState.UpdateStatus == ToUpdate )
				{
					TEntityIndex entityIndex = _VisionArray->getEntityIndex( clienthost->clientId(), (TCLEntityId)e );

					if ( entityIndex.isValid() )
					{

						// Get delta
						TCoord delta = getDeltaOrientation( entityIndex, clienthost->clientId(), e );

						// Do not set priority if delta is zero (steady entities...)
						if ( delta != 0 )
						{
							TCoord distanceCE = _VisionArray->distanceCE( clienthost->clientId(), (TCLEntityId)e );
							float prioRatio = (float)distanceCE / (float)delta;

							// See explanation of formula on top of this file
							if ( prioRatio < RATIO_MAX_THRESHOLD )
							{
								bucket = HIGHEST_PRIORITY;
							}
							else
							{
								bucket = LOWEST_PRIORITY - (sint)((float)(LOWEST_PRIORITY*RATIO_MAX_THRESHOLD) / prioRatio);
							}

							// Set the priority of the pair
							clienthost->PropDispatcher.setPriority( e, PROPERTY_ORIENTATION, tvaOrt[e].PropState, (TPriority)bucket );

							/*nlinfo( "FEPRIO: Set cep %hu-%hu-%hu to priority %u",
									(uint16)pairCE->ClientId, (uint16)pairCE->CeId, (uint16)p, uprio );*/
						}
					}
				}
			}

			++clientmapindex;
			++icm;
		}

		OrientationSpreader.endProcessing( icm );
	}

	OrientationSpreader.incCycle();
}


/*
 * Calculate discreet props priorities using distance threshold
 */
void				CPrioritizer::calculatePriorityOfDiscreet()
{
	if ( DiscreetSpreader.mustProcessNow() )
	{
		THostMap::iterator icm;
		sint clientmapindex, outerBoundIndex;
		DiscreetSpreader.getProcessingBounds( icm, clientmapindex, outerBoundIndex );
		sint bucket;

		while ( clientmapindex < outerBoundIndex )
		{
			CClientHost *clienthost = GETCLIENTA(icm);

			for	( sint e=0; e!=MAX_SEEN_ENTITIES_PER_CLIENT; ++e )
			{
				TEntityIndex entityIndex = _VisionArray->getEntityIndex( clienthost->clientId(), (TCLEntityId)e );
				if ( entityIndex.isValid() )
				{
					TCoord distanceCE = _VisionArray->distanceCE( clienthost->clientId(), (TCLEntityId)e );

					// For each discreet property corresponding to the entity type
					for ( sint p=FIRST_DISCREET_PROPINDEX; p<_PropTranslator->nbProperties( entityIndex ); ++p )
					{
						// Calculate priority only if the update status is ToUpdate,
						// meaning that a change has not been sent to the client
						//
						if ( _VisionArray->tvaProp( clienthost->clientId(), p )[e].PropState.UpdateStatus == ToUpdate )
						{
							// Get TProperty from TPropIndex
							TProperty propertyid = _PropTranslator->getPropertyId( entityIndex, p );

							if ( distanceCE < _PropTranslator->getDistThreshold( propertyid ) )
							{
								bucket = HIGHEST_PRIORITY;
							}
							else
							{
								bucket = LOWEST_PRIORITY;
							}

							// Set the priority of the pair
							clienthost->PropDispatcher.setPriority( e, p, _VisionArray->prioLoc( clienthost->clientId(), e, p ), (TPriority)bucket );

							//nlinfo( "FEPRIO: Set cep %hu-%hu-%hu to priority %u",
							//	(uint16)pairCE->ClientId, (uint16)pairCE->CeId, (uint16)p, uprio );
						}
					}
				}
			}

			++clientmapindex;
			++icm;
		}

		DiscreetSpreader.endProcessing( icm );
	}

	DiscreetSpreader.incCycle();
}



bool TraceDelta = true;


/*
 * Return the delta corresponding to a property
 */
inline TCoord		CPrioritizer::getDeltaPos( TEntityIndex entityindex, TClientId clientid, TCLEntityId ceid )
{
	// Position special case
	uint32 lastSentMileage = _History->getMileage( clientid, ceid );

	if ( lastSentMileage != 0 )
	{
		CFrontEndPropertyReceiver::SEntity *entity = CFrontEndPropertyReceiver::getEntity( entityindex );
		
		// Calculate difference distance between current and lastsent mileage (unsigned to allow mileage overflow)
		uint32 d = (entity->Mileage - lastSentMileage);//(uint32)frand(100000.0);

		// Ignore not significant deltas (maybe not needed anymore with integer calculation)
		if ( d < 5 )
		{ 
			//nlinfo( "Low position delta = %u", d );
			return 0;
		}
		else
		{
			/*if ( TraceDelta )
				nlinfo( "%d", d );*/
			return (TCoord)d;
		}
	}
	else
	{
		// Not sent yet, set max delta
		//nlinfo( "FEPRIO: getDelta(position) : first time" );
		return MaxDelta;
	}
	//nlinfo( "Delta: %.2f (current: %s last: %s)", delta, current_entitypos.asString().c_str(), lastsent_entitypos.asString().c_str() );
}



inline TCoord		CPrioritizer::getDeltaOrientation( TEntityIndex entityindex, TClientId clientid, TCLEntityId ceid )
{
	bool histohasvalue;
	const CFrontEndPropertyReceiver::TPropertiesValue current_value = CFrontEndPropertyReceiver::getEntity( entityindex )->properties[PROPERTY_ORIENTATION];
	const CFrontEndPropertyReceiver::TPropertiesValue& lastsent_value = _History->getPropertyEntry( clientid, ceid, PROPERTY_ORIENTATION, histohasvalue ).LastSent;
	if ( histohasvalue )
	{
		// Orientation is a float angle in radian
		const float& newangle = *((float*)&current_value);//frand(6.28);
		const float& oldangle = *((float*)&lastsent_value);
		float deltaAngle = (float)fabs( (float)(newangle - oldangle) );
		deltaAngle = (float)fmod( deltaAngle+(2*Pi), (2*Pi) );
			
		// Delta=1 m corresponds to Pi
		//nlinfo( "getDelta(theta) : dA=%g", deltaAngle );
		if ( deltaAngle > Pi )
			return (TCoord)(2000.0f - (deltaAngle * 1000.0f / Pi));
		else
			return (TCoord)(deltaAngle * 1000.0f / Pi);
	}
	else
	{
		// Not sent yet, set max delta
		return MaxDelta;
	}
}


NLMISC_COMMAND( forcePriority, "Force a priority", "<clientid> <slot> <propindex> <priority>" )
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 4) return false;
	
	// get the values
	TClientId clientid = atoi(args[0].c_str());
	sint slot = atoi(args[1].c_str());
	TPropIndex propindex = atoi(args[2].c_str());
	TPriority priority = atoi(args[3].c_str());

	if ( (clientid <= MaxNbClients) && (slot < MAX_SEEN_ENTITIES_PER_CLIENT) && (propindex < NB_VISUAL_PROPERTIES/*16*/) && ( priority < NB_PRIORITIES ) )
	{
		CClientHost *clienthost = CFrontEndService::instance()->sendSub()->clientIdCont()[clientid];
		//nlassertex( clienthost, ("c%hu", clientid) );
		clienthost->PropDispatcher.setPriority( slot, propindex, CFrontEndService::instance()->PrioSub.VisionArray.prioLoc( clientid, slot, propindex ), priority );
	}
	else
	{
		log.displayNL( "Invalid argument value(s)" );
	}
	return true;
}


/*NLMISC_COMMAND( setPriorityStrategy, "Change the priority strategy, distance-delta or distance only", "<strategy>1/0" )
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 1) return false;
	
	// get the values
	sint strat = atoi(args[0].c_str());

	switch( strat )
	{
	case 0: CFrontEndService::instance()->PrioSub.Prioritizer.setPrioStrategy( CPrioritizer::DistanceOnly ); break;
	case 1: CFrontEndService::instance()->PrioSub.Prioritizer.setPrioStrategy( CPrioritizer::DistanceDelta ); break;
	default: return false;
	}

	THostMap::iterator ihm;
	for ( ihm=CFrontEndService::instance()->receiveSub()->clientMap().begin(); ihm!=CFrontEndService::instance()->receiveSub()->clientMap().end(); ++ihm )
	{
		GETCLIENTA(ihm)->PropDispatcher.resetThreshold();
	}
	return true;
}


NLMISC_COMMAND( getPriorityStrategy, "Get the priority strategy", "" )
{
	switch( CFrontEndService::instance()->PrioSub.Prioritizer.prioStrategy() )
	{
	case CPrioritizer::DistanceOnly: log.displayNL( "Distance only" ); break;
	case CPrioritizer::DistanceDelta: log.displayNL( "Distance-delta" ); break;
	}
	return true;
}


NLMISC_COMMAND( getPriorityThresholds, "Get the threshold constants", "" )
{
	log.displayNL( "MinDelta=%d MaxDelta=%d MinDist=%d MaxDist=%d",
					CPrioritizer::DeltaMinThreshold, CPrioritizer::DeltaMaxThreshold,
					CPrioritizer::DistMinThreshold, CPrioritizer::DistMaxThreshold );
	return true;
}*/


NLMISC_COMMAND( getDelta, "Get the delta for a priority", "<client> <slot> <propindex>" )
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 3) return false;
	
	// get the values
	TClientId clientid = atoi(args[0].c_str());
	sint slot = atoi(args[1].c_str());
	TPropIndex propindex = atoi(args[2].c_str());

	if ( (clientid <= MaxNbClients) && (CFrontEndService::instance()->sendSub()->clientIdCont()[clientid] != NULL) )
	{
		if ( slot < MAX_SEEN_ENTITIES_PER_CLIENT )
		{
			TEntityIndex entityindex = CFrontEndService::instance()->PrioSub.VisionArray.getEntityIndex( clientid, (TCLEntityId)slot );
			if ( propindex > 15 )
				return false;
			
			TProperty propertyid = CFrontEndService::instance()->PrioSub.PropTranslator.getPropertyId( entityindex, propindex );

			if ( propertyid == PROPERTY_POSITION )
				log.displayNL( "%d", CFrontEndService::instance()->PrioSub.Prioritizer.getDeltaPos( entityindex, clientid, (TCLEntityId)slot ) );
			else if ( propertyid == PROPERTY_ORIENTATION )
				log.displayNL( "%d", CFrontEndService::instance()->PrioSub.Prioritizer.getDeltaOrientation( entityindex, clientid, (TCLEntityId)slot ) );
			else
				log.displayNL( "Invalid property id" );
		}
		else
		{
			log.displayNL( "Invalid slot" );
		}
	}
	else
	{
		log.displayNL( "There is no such a client id" );
	}
	return true;
}


/*NLMISC_COMMAND( delta, "Trace delta", "0/1" )
{
	if ( args.size() == 0 )
		return false;

	TraceDelta = ( atoi(args[0].c_str()) == 1 );
	return true;
}*/
