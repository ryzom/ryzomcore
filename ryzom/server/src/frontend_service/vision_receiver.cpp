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
#include "vision_receiver.h"
#include <vector>

using namespace NLMISC;
using namespace NLNET;
using namespace std;


CVisionReceiver *VisionReceiverInstance = NULL;

/*
 * Constructor
 */
CVisionReceiver::CVisionReceiver() : _HasPendingDelta(false)
{
	nlassert( ! VisionReceiverInstance );
	VisionReceiverInstance = this;
}



/*
 * Initialisation
 */
void CVisionReceiver::init()
{
#ifdef NL_DEBUG
	nlwarning("To change, assign -1 is a bad idea.");
#endif
	_FirstUpdatedEntityVision=TEntityIndex();

	// Register callback function for receiving vision deltas
	NLNET::TUnifiedCallbackItem _cbArray[1];
	_cbArray[0].Callback = cbDeltaNewVision;
	_cbArray[0].Key = "VISIONS_DELTA_2";
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, 1 );
}


//---------------------------------------------------
// Vision management : Set a new delta of vision
// 
//---------------------------------------------------
bool CVisionReceiver::setVision( const CPlayerVisionDelta &visionDelta )
{
	// Find the index
	//TEntityIndex iviewer = TheEntityContainer->entityIdToIndex( visionDelta.PlayerId );
	const TEntityIndex& iviewer = visionDelta.PlayerIndex;

	if ( !iviewer.isValid() ) // the vision is too old, the player has gone
	{
		//nldebug( "Discarding vision for player %s because not found", visionDelta.PlayerId.toString().c_str() );
		nldebug( "Discarding vision for player E%u because not found", visionDelta.PlayerIndex.getIndex() );
		return false;
	}

	CEntity *p = TheEntityContainer->getEntity( iviewer );
	// If entity has no unprocessed vision
	if( p->NextUpdatedEntityVision == LAST_VISION_CHANGE )
	{
		// Insert the entity in the vision list
		p->NextUpdatedEntityVision = _FirstUpdatedEntityVision;
		_FirstUpdatedEntityVision = iviewer;

		for( vector< CPlayerVisionDelta::CIdSlot >::const_iterator itIn = visionDelta.EntitiesIn.begin(); itIn != visionDelta.EntitiesIn.end(); ++itIn )
		{
			// Add
			//p->insertIntoVisionIn( iviewer, (*itIn).Id, (*itIn).Slot, false );
			p->insertIntoVisionIn( iviewer, (*itIn).Index, (*itIn).Slot, false );
		}

		for( vector< CPlayerVisionDelta::CIdSlot >::const_iterator itOut = visionDelta.EntitiesOut.begin(); itOut != visionDelta.EntitiesOut.end(); ++itOut )
		{
			p->VisionOut.insert( (*itOut).Slot );
		}

		/*for( vector< CPlayerVisionDelta::CIdSlot >::const_iterator itRep = visionDelta.EntitiesReplace.begin(); itRep != visionDelta.EntitiesReplace.end(); ++itRep )
		{
			// Replace (e.g. character -> bag)
			//p->insertIntoVisionIn( iviewer, (*itRep).Id, (*itRep).Slot, true );
			p->insertIntoVisionIn( iviewer, (*itRep).Index, (*itRep).Slot, true );
		}*/
	}
	else
	{
		// See 'Note' comments below
		nlwarning( "Deprecated code executed (merging vision), please warn the author" );

		// Merging old and new delta vision update (only if front-end has not processed the previous vision update yet)
		for( vector< CPlayerVisionDelta::CIdSlot >::const_iterator itIn = visionDelta.EntitiesIn.begin(); itIn != visionDelta.EntitiesIn.end(); ++itIn )
		{
			// Add
			//p->insertIntoVisionIn( iviewer, (*itIn).Id, (*itIn).Slot, false );
			p->insertIntoVisionIn( iviewer, (*itIn).Index, (*itIn).Slot, false );

			// Remove from VisionOut
			set< CLFECOMMON::TCLEntityId >::iterator it = p->VisionOut.find( (*itIn).Slot );
			if( it != p->VisionOut.end() )
			{
				p->VisionOut.erase( it );
			}

			// Note: if the new entityId that comes in the slot is the same as the previous,
			// we should delete both entries from VisionIn and VisionOut.
			// If they are different, we should first removePair, then addPair (which is the
			// order done in CVisionProvider::processVision().
			//
			// Instead of that, we always addPair and never removePair. Why this?
			// A long time ago, the CPlayerVisionDelta contained entityIds instead of slots
			// (these were allocated by the front-end). But what?
			//
			// Maybe the queue system (with _NextDeltas) made this case obsolete, meaning
			// we never get in this probably-bugged code again!
		}

		for( vector< CPlayerVisionDelta::CIdSlot >::const_iterator itOut = visionDelta.EntitiesOut.begin(); itOut != visionDelta.EntitiesOut.end(); ++itOut )
		{
			// Find the index of the id
			//TEntityIndex iviewed = TheEntityContainer->entityIdToIndex( (*itOut).Id );
			TEntityIndex iviewed = (*itOut).Index;
			if ( iviewed.isValid() )
			{
				// Remove the index of the VisionIn list (for proper merging)
				//nlinfo( "** Add E%u to VisionOut", iviewed );
				TMapOfVisionAssociations::iterator it = p->VisionIn.find( iviewed );
				if( it != p->VisionIn.end() )
				{
					p->VisionIn.erase( it );
				}
			}
			p->VisionOut.insert( (*itOut).Slot );

			// Note: see above
		}
		
		/*for( vector< CPlayerVisionDelta::CIdSlot >::const_iterator itRep = visionDelta.EntitiesReplace.begin(); itRep != visionDelta.EntitiesReplace.end(); ++itRep )
		{
			// Replace (e.g. character -> sack)
			//p->insertIntoVisionIn( iviewer, (*itRep).Id, (*itRep).Slot, true );
			p->insertIntoVisionIn( iviewer, (*itRep).Index, (*itRep).Slot, true );

			// Note: ?? What next?
		}*/

	}
	return true;
}

//---------------------------------------------------
// Vision management : Call it after getting all the updated vision (eg, the.GetNextUpdatedVision function return -1 )
// 
//---------------------------------------------------
void CVisionReceiver::endUpdatedVision()
{
	// for all the entities in the list of updated
	
	while(_FirstUpdatedEntityVision.isValid())
	{
		// Get the next entity
		TEntityIndex i = _FirstUpdatedEntityVision;
		CEntity *p = TheEntityContainer->getEntity( _FirstUpdatedEntityVision );
		// Clean the lists
		p->VisionIn.clear(); 
		p->VisionOut.clear(); 
		// Remove it from list of updated entities
		_FirstUpdatedEntityVision = p->NextUpdatedEntityVision;
		p->NextUpdatedEntityVision.initToLastChanged();
	}

	if ( _NextDeltas.empty() )
	{
		// The only delta has been processed by the front-end
		//nldebug( "End of vision scanning" );
		_HasPendingDelta = false;
	}
	else
	{
		// Somes more deltas need to be processed
		//nldebug( "End of vision scanning - Applying one more delta" );
		nlassert( _HasPendingDelta );
		list< CPlayerVisionDelta >& deltaVision = _NextDeltas.front();

		// Apply the next delta to the vision
		for( list< CPlayerVisionDelta >::iterator it = deltaVision.begin(); it != deltaVision.end(); ++it )
		{
//			setVision( (*it).Id, (*it).EntityIn, (*it).EntityOut );
			setVision( *it );
		}

		_NextDeltas.pop();
	}
}


//---------------------------------------------------
// UpdateVision, unserial update vision message and process it
// 
//---------------------------------------------------
inline void CVisionReceiver::updateNewVision( CMessage& msgin )
{
	//LOG_VISION("FEVIS:%u: Receiving a vision update", CTickEventHandler::getGameCycle() );

	list< CPlayerVisionDelta > deltaVision;
	CPlayerVisionDelta::decodeVisionDelta(msgin, deltaVision);

	//list< SPlayerVisionDelta > deltaVision;
	//msgin.serialCont( deltaVision );

	if ( ! _HasPendingDelta )
	{
		// Apply the delta to the vision
		//nldebug( "Applying delta" );
		//for( list< SPlayerVisionDelta >::iterator it = deltaVision.begin(); it != deltaVision.end(); ++it )
		for( list< CPlayerVisionDelta >::iterator it = deltaVision.begin(); it != deltaVision.end(); ++it )
		{
			setVision( *it );
		}
		_HasPendingDelta = true;
	}
	else
	{
		// Store the delta
		//nldebug( "Storing delta" );
		_NextDeltas.push( deltaVision );
	}
}




//---------------------------------------------------
// Callback for delta vision update received
// 
//---------------------------------------------------
void cbDeltaNewVision( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	//H_BEFORE(RcvUpdateDeltaNewVision);
	//CFrontEndService::instance()->BackEndRecvWatch3.start();
	VisionReceiverInstance->updateNewVision( msgin );
	//CFrontEndService::instance()->BackEndRecvWatch3.stop();
	//H_AFTER(RcvUpdateDeltaNewVision);
}
