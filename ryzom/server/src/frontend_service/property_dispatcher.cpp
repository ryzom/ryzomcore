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


#include "property_dispatcher.h"
#include "vision_array.h"
#include "property_id_translator.h"
#include "frontend_service.h"
#include "fe_stat.h"
//#include <sstream>

using namespace std;
using namespace NLMISC;
using namespace CLFECOMMON;


/*
 * Constructor
 */
CPropertyDispatcher::CPropertyDispatcher() :
	_VisionArray( NULL ),
	_PropTranslator( NULL ),
	_ClientId( 0 )
	/*_HPThreshold( 0 ),
	_HPTDelta( 0 ),*/
	//_NbSentParcelsInCycle( 0 )
	/*NbSetPrio( 0 ),
	NbReenable( 0 )*/
{
	_NextParcel.Prio = DISABLE_PRIORITY;
	_NextParcel.Offset = INVALID_OFFSET;
}


/*
 * Initialization
 */
void				CPropertyDispatcher::init( TClientId clientid )
{
	_ClientId = clientid;
	_VisionArray = &(CFrontEndService::instance()->PrioSub.VisionArray);
	_PropTranslator = &(CFrontEndService::instance()->PrioSub.PropTranslator);

	// Initialize highest priority threshold from theorical maxratio
	//_HPThreshold = CFrontEndService::instance()->PrioSub.Prioritizer.MaxRatio / NB_PRIORITIES;
}


/*
 * Remove from the property dispatcher shelves, or does nothing if offset is INVALID_OFFSET
 */
void				CPropertyDispatcher::removeProp( const TPropParcelPtr& parcelptr )
{
	// Remove corresponding item in shelves (do nothing if there is no item yet)
	if ( parcelptr.Offset != INVALID_OFFSET )
	{
		//nlinfo( "FEPRIO: Removing prop from dispatcher: %u-%u", parcelptr.Prio, parcelptr.Offset );
		TPropertyShelf& shelf = getShelf( parcelptr.Prio );
		//TMPDEBUG
		if ( parcelptr.Offset < shelf.size() )
		{
			TPropParcel& parcel = getParcel( shelf, parcelptr.Offset );

			// Remove parcel
			TPropParcel& lastone = lastParcel( shelf );
			if ( &parcel != &lastone )
			{
				parcel = lastone;
				_VisionArray->updateLink( _ClientId, parcel, parcelptr.Offset );
			}
			shelf.pop_back();
		}
		else
		{
			nlwarning( "Cannot removeProp with invalid offset %hd", parcelptr.Offset );
		}
	}
}


/*
 * Remove all corresponding properties from the property dispatcher
 */
void				CPropertyDispatcher::removePropsFromShelves( TClientId clientid, TCLEntityId ceid )
{
	sint ppi = 0;
	TPropParcelPtr& parcelptr = _VisionArray->prioLoc( clientid, ceid, ppi );
	removeProp( parcelptr );
	parcelptr.Prio = DISABLE_PRIORITY;
	parcelptr.Offset = INVALID_OFFSET;

	// Skip special props

	for ( ppi=3; ppi!=MAX_PROPERTIES_PER_ENTITY; ++ppi )
	{
		TPropParcelPtr& parcelptr = _VisionArray->prioLoc( clientid, ceid, ppi );
		removeProp( parcelptr );
		parcelptr.Prio = DISABLE_PRIORITY;
		parcelptr.Offset = INVALID_OFFSET;
	}
}


/*
 * Make a new enabled parcel in the shelf corresponding to parcelptr.Prio and fill the parcelptr.Offset
 */
void				CPropertyDispatcher::addProp( TPropParcelPtr& parcelptr, TCLEntityId ceid, TPropIndex propindex )
{
	TPropParcel parcel;
	parcel.CeId = ceid;
	parcel.PropIndex = propindex;
	parcel.Enabled = true;

	// Add parcel and fill the offset field
	TPropertyShelf& shelf = getShelf( parcelptr.Prio );
	shelf.push_back( parcel );
	parcelptr.Offset = shelf.size() - 1;

	//printShelves( "after addProp", true );
}


/*
 * Initialize dispatch cycle
 */
void				CPropertyDispatcher::initDispatcherCycle( /*bool rescalePriorities*/ )
{
	// Adjust threshold for highest priority selection
	/*if ( rescalePriorities )
	{
		adjustHPThreshold();
	}*/

	// Set the pointer ready to advance to the first parcel
	_NextParcel.Prio = INITIAL_PRIORITY;
	_NextParcel.Offset = -1;

	//_NbSentParcelsInCycle = 0;

	// Move pointer to first parcel
	incNextParcel();

	// Display debug info
	//printShelves( "after initDispatcherCycle", false );
	//printShelfSizes();
}


/*
 * Increment pointer to next parcel
 */
void				CPropertyDispatcher::incNextParcel()
{
	if ( _NextParcel.Offset < nbParcels(_NextParcel.Prio) - 1 )
	{
		// Advance to the next parcel in the same shelf
		++_NextParcel.Offset;

		// Check if the current parcel is enabled, otherwise advance to next one
		if ( ! getParcel( _NextParcel ).Enabled )
		{
			incNextParcel();
		}
	}
	else
	{
		if ( _NextParcel.Prio < LAST_PRIO_SENT )
		{
			// "Carriage return" to the next shelf
			++_NextParcel.Prio;
			_NextParcel.Offset = 0;

			// Skip any empty shelves
			skipEmptyShelves();
		}
		else
		{
			// Don't go past LAST_PRIO_SENT
			_NextParcel.Offset = INVALID_OFFSET;
		}
	}
}


/*
 * Skip any empty shelves
 */
inline void			CPropertyDispatcher::skipEmptyShelves()
{
	// Skip empty shelves
	while ( _PropShelves[_NextParcel.Prio].empty() && (_NextParcel.Prio < LAST_PRIO_SENT) )
	{
		++_NextParcel.Prio;
	}

	// Detect end of data
	if ( _PropShelves[_NextParcel.Prio].empty() )
	{
		_NextParcel.Offset = INVALID_OFFSET;
	}
	else
	{
		// Check if the current parcel is enabled, otherwise advance to next one
		if ( ! getParcel( _NextParcel ).Enabled )
		{
			incNextParcel();
		}
	}
}


/*
 * Return which property to send next, or NULL if there is no more.
 */
//const TPropParcel	*CPropertyDispatcher::getNextParcel()


/*
 * Set status: true if the latest parcel got by getNextParcel() can be sent in the current cycle, otherwise false
 * If true, the parcel gets disabled.
 * Must be called after getNextParcel() every time getNextParcel() is called
 */
void				CPropertyDispatcher::setParcelStatusTrue()
{
	// Status is set when the parcel was sent
	if ( _NextParcel.Offset != INVALID_OFFSET )
	{
		//++_NbSentParcelsInCycle;
		TPropParcel& parcel = getParcel( _NextParcel );

		// Continuous or discreet property ?
		//if ( _PropTranslator->isContinuous( _VisionArray->getEntityIndex( _ClientId, parcel.CeId ), parcel.PropIndex ) )
		if ( parcel.PropIndex < FIRST_DISCREET_PROPINDEX )
		{
			// Continuous: disable the parcel
			parcel.Enabled = false;

#ifdef MEASURE_FRONTEND_TABLES
			// Debug: this is ok because the position is a common prop and its propertyid equals the propindex
			if ( (_ClientId == 1) && (parcel.PropIndex == PROPERTY_POSITION) )
			{
				PosSentCntFrame.SeenEntities[parcel.CeId] = 1;
			}
#endif

		}
		else
		{
			// Discreet: ask parcel removal and set update status to Updating
			TPropState& propstate = _VisionArray->propState( _ClientId, parcel );
			addToRemoveList( &propstate );
			propstate.UpdateStatus = Updating;

			// DEBUG DISPLAY
			//nlinfo( "FESEND: Sent discrete property %hu to client %hu slot %hu", propertyid, _ClientId, (uint16)parcel.CeId );
		}
	}

	// Advance to next parcel
	incNextParcel();
}



/*
 * Remove the parcels that have been added to the remove list (call it after a cycle)
 */
void				CPropertyDispatcher::flush()
{
	// Remove all parcels referenced in the remove list
	TRemoveList::iterator irl;
	for ( irl=_RemoveList.begin(); irl!=_RemoveList.end(); ++irl )
	{
		// Remove parcel
		removeProp( *(*irl) );

		// Remove link
		(*irl)->Prio = DISABLE_PRIORITY;
		(*irl)->Offset = INVALID_OFFSET;
	}

	// Clear the remove list
	_RemoveList.clear();
}


/*
 * Return the number of enabled parcels in a shelf
 */
TPropParcelOffset	CPropertyDispatcher::nbEnabledParcels( TPriority prio )
{
	nlassert( prio <= NB_PRIORITIES );
	sint32 nb = 0;
	TPropertyShelf::iterator ips;
	for ( ips=_PropShelves[prio].begin(); ips!=_PropShelves[prio].end(); ++ips )
	{
		if ( (*ips).Enabled )
			++nb;
	}
	return (TPropParcelOffset)nb;
}


/*
 * Adjust HPThreshold (dichotomic)
 */
/*void				CPropertyDispatcher::adjustHPThreshold()
{
	uint32 nbhpparcels = (uint32)nbEnabledParcels( HIGHEST_PRIORITY );
	float hpoldvalue = _HPThreshold;
	//nlinfo( "NbHP=%u NbSentParcelsInCycle=%u", (uint32)nbParcels( HIGHEST_PRIORITY ), _NbSentParcelsInCycle );
	if ( nbhpparcels < _NbSentParcelsInCycle / 2 )
	{
		// Increase threshold when HIGHEST_PRIORITY can be filled more
		if ( _HPTDelta < 0 )
		{
			_HPThreshold = _HPThreshold - _HPTDelta/2.0f;
			//nlinfo( "FEPRIO: Stabilizing HPThreshold (+) to %.2f", _HPThreshold );
	    }
		else
		{
			_HPThreshold *= 2.0f;
			//nlinfo( "FEPRIO: Rising HPThreshold to %.2f, %u estimated actions", _HPThreshold, CFrontEndService::instance()->SentActionsLastCycle );
			//printShelfSizes();
		}
	}
	else
	{
		// Decrease threshold when HIGHEST_PRIORITY is overloaded
		if ( nbhpparcels > _NbSentParcelsInCycle )
		{
			if ( _HPTDelta > 0 )
			{
				_HPThreshold = _HPThreshold - _HPTDelta/2.0f;
				//nlinfo( "FEPRIO: Stabilizing HPThreshold (-) to %.2f", _HPThreshold );
			}
			else
			{
				_HPThreshold /= 2.0f; // decrease threshold if HIGHEST_PRIORITY is crowded
				//nlinfo( "FEPRIO: Lowering HPThreshold to %.2f, %u estimated actions", _HPThreshold, CFrontEndService::instance()->SentActionsLastCycle );
				//printShelfSizes();	
			}
		}
	}

	float maxratio = CFrontEndService::instance()->PrioSub.Prioritizer.MaxRatio;
	if ( _HPThreshold >= maxratio )
	{
		// Fix threshold if it is too big after being calculated in adjustHPThreshold()
		_HPThreshold = maxratio* 0.8f;
		_HPTDelta = 0.0f;
		//nlinfo( "FEPRIO: Fixing HPThreshold to %.2f", _HPThreshold );
	}	//nlinfo( "FEPRIO: HPThreshold is %.2f", _HPThreshold );
	else
	{
		_HPTDelta = _HPThreshold - hpoldvalue;
	}
}*/

	
/*
 * Return the amount of priorities used corresponding to the specified number of actions (e.g. 1.2 means priority 0 is full and priority 1 is 20% filled)
 */
float				CPropertyDispatcher::getPrioRatio()
{
	// FEATURE DISABLED (_NbSentParcelsInCycle not calculated)
	uint32 nbactions = 0;//_NbSentParcelsInCycle;
	//nlinfo( "NbSent: %u size0: %u", _NbSentParcelsInCycle, _PropShelves[0].size() );
	float ratio = 0.0f;
	uint32 amount = 0;
	TPriority p;
	for ( p=0; p!=NB_PRIORITIES; ++p )
	{
		uint32 nbenabledinshelf = nbEnabledParcels(p);
		if ( nbactions - amount > nbenabledinshelf )
		{
			// Accumulate amount of full shelf
			amount += _PropShelves[p].size();
			ratio += 1.0f;
		}
		else
		{
			// Accumulate amount of last shelf (decimals)
			if ( ! _PropShelves[p].empty() )
			{				
				ratio += (float)(nbactions - amount) / (float)nbenabledinshelf;
			}
			//comment Ben nlinfo( "FEPRIO: %u HP of %u total => ratio %.1f last=%u", _PropShelves[0].size(), nbactions, ratio, p );
			return ratio;
		}
	}
	return ratio;
}


/*
 * Display the contents of the shelves
 */
void				CPropertyDispatcher::printShelves( const char *title, bool checkIntegrity, bool proptext, bool hidedisabled ) const
{
	bool integrity = true;
	//stringstream ss;
	string str;
	string check;
	if ( checkIntegrity )
	{
		check = " checking integrity";
	}
	//ss << "Client: " << _ClientId << /*" HPThreshold: " << _HPThreshold <<*/ " Next parcel: ";
	str += "Client: " + NLMISC::toString(_ClientId) + /*" HPThreshold: " << NLMISC::toString(_HPThreshold) <<*/ " Next parcel: ";
	if ( _NextParcel.Offset == -1 )
	{
		//ss << "Ready to start" << endl;
		str += "Ready to start\n";
	}
	else if ( _NextParcel.Offset == INVALID_OFFSET )
	{
		//ss << "INVALID_OFFSET (prio=" << _NextParcel.Prio << ")" << endl;
		str += "INVALID_OFFSET (prio=" + NLMISC::toString(_NextParcel.Prio) + ")\n";
	}
	else
	{
		//ss << "prio " << _NextParcel.Prio << " offset " << _NextParcel.Offset << endl;
		str += "prio " + NLMISC::toString(_NextParcel.Prio) + " offset " + NLMISC::toString(_NextParcel.Offset) + "\n";
	}
	//ss << "Property shelves " << title << check << ":" << endl;
	str += "Property shelves " + NLMISC::toString(title) + NLMISC::toString(check) + ":\n";
	TPriority p;

	// Scan all shelves
	for ( p=0; p!=NB_PRIORITIES; ++p )
	{
		if ( ! _PropShelves[p].empty() )
		{
			//ss << p << ':';
			str += NLMISC::toString(p) + ':';
			TPropertyShelf::const_iterator ipr;

			// Scan all parcels
			for ( ipr=_PropShelves[p].begin(); ipr!=_PropShelves[p].end(); ++ipr )
			{
				const TPropParcel& parcel = (*ipr);
				if ( (!hidedisabled) || parcel.Enabled )
				{
					//ss << ' ' << parcel.CeId << ' ';
					str += " " + NLMISC::toString(parcel.CeId) + " ";
					if ( proptext )
						//ss << getPropText( parcel.PropIndex );
						str += NLMISC::toString(getPropText( parcel.PropIndex ));
					else
						//ss << parcel.PropIndex;
						str += NLMISC::toString(parcel.PropIndex);
					if ( !hidedisabled )
						//ss << ' ' << parcel.Enabled;
						str += " " + NLMISC::toString(parcel.Enabled);
					//ss << " -";
					str += " -";
				}

				// Check integrity (debugging feature)
				if ( checkIntegrity )
				{
					TPropParcelPtr parcelptr = _VisionArray->propState( _ClientId, parcel );
					if ( ! (parcelptr.Prio == p) &&
						   (parcelptr.Offset == TPropParcelOffset(ipr-_PropShelves[p].begin())) )
					{
						integrity = false;
					}
				}
			}
			//ss << endl;
			str += "\n";
		}
	}
	// ss << ends;
	nlwarning( "FEPRIO: %s", str.c_str() );
	if ( ! integrity )
	{
		nlerror( "Property Dispatcher: integrity check failed" );
	}
}


/*
 * Display the sizes of the shelves
 */
void CPropertyDispatcher::printShelfSizes() const
{
	sint prio;
	//InfoLog->display( "FEPRIO: Threshold: %.2f Parcels by prio:", _HPThreshold );
	InfoLog->display( "FEPRIO: Parcels by prio:" );
	for ( prio=HIGHEST_PRIORITY; prio!=NB_PRIORITIES; ++prio )
	{
		InfoLog->displayRaw( " %u", _PropShelves[prio].size() );
	}
	InfoLog->displayRawNL( "" );
}


/*
 * Count the number of clients or properties of a certain type in the property shelves
 */
void CPropertyDispatcher::displayCounts( TPropIndex propindex )
{
	sint prio = 0;
	sint ofst = 0;
	sint matching = 0;
	sint enabled_matching = 0;
	sint discrete_matching = 0;
	sint self_matching = 0; // slot 0

	while ( prio < NB_PRIORITIES )
	{
		for ( ofst = 0; ofst != (sint)_PropShelves[prio].size(); ++ofst )
		{
			bool propok = ( (propindex == 0xFF) || (propindex == _PropShelves[prio][ofst].PropIndex) );
			if ( propok )
			{
				++matching;
				
				if ( _PropShelves[prio][ofst].Enabled)
				{
					++enabled_matching;
				}
				if ( _PropShelves[prio][ofst].PropIndex > LAST_CONTINUOUS_PROPERTY )
				{
					++discrete_matching;
				}
				if ( _PropShelves[prio][ofst].CeId == 0 )
				{
					++self_matching;
				}
			}
		}
		++prio;
	}
	if ( propindex == 0xFF )
		nlinfo( "FEPRIO: Searching for all property indexes" );
	else
		 // warning: mixing property/propindex (TEMP)
		nlinfo( "FEPRIO: Searching for propindex %hu (%s)", (uint16)propindex, getPropText(propindex) );

	nlinfo( "FEPRIO: Found %d matching properties, including %d enabled, %d discrete, %d self-vision", matching, enabled_matching, discrete_matching, self_matching );
}



NLMISC_COMMAND( displayPropShelves, "Display the shelves of the property dispatcher", "[<clientid>|0] [<hideDisabled>0/1] [<propText>1/0] [<checkIntegrity>0/1]" )
{
	// Parse arguments
	TClientId clientid = 0;
	bool hidedisabled = false;
	bool check = false;
	bool proptext = true;
	if ( args.size() > 0 )
	{
		clientid = atoi(args[0].c_str());
		if ( args.size() > 1 )
		{
			hidedisabled = (atoi(args[1].c_str()) != 0);
			if ( args.size() > 2 )
			{
				check = (atoi(args[2].c_str()) != 0);
				if ( args.size() > 3 )
				{
					proptext = (atoi(args[3].c_str()) != 0);
				}
			}
		}
	}

	// Display
	THostMap& clientmap = CFrontEndService::instance()->instance()->receiveSub()->clientMap();
	THostMap::iterator icm;
	for ( icm=clientmap.begin(); icm!=clientmap.end(); ++icm )
	{
		if ( (clientid == 0) || (clientid == GETCLIENTA(icm)->clientId()) )
		{
			GETCLIENTA(icm)->PropDispatcher.printShelves( "command", check, proptext, hidedisabled, &log );
		}
	}
	return true;
}


NLMISC_COMMAND( displayPropShelfSizes, "Display the sizes of the shelves of the property dispatcher", "[<clientid>|0]" )
{
	// Parse arguments
	TClientId clientid = 0;
	if ( args.size() > 0 )
	{
		clientid = atoi(args[0].c_str());
	}

	// Display
	THostMap& clientmap = CFrontEndService::instance()->instance()->receiveSub()->clientMap();
	THostMap::iterator icm;
	for ( icm=clientmap.begin(); icm!=clientmap.end(); ++icm )
	{
		if ( (clientid == 0) || (clientid == GETCLIENTA(icm)->clientId()) )
		{
			GETCLIENTA(icm)->PropDispatcher.printShelfSizes(&log);
		}
	}
	return true;
}


NLMISC_COMMAND( countClientsOrPropertiesInShelves, "Count the number of clients or properties of a certain type in the property shelves", "[<clientid|0> [<propindex>]]" )
{
	// Parse arguments
	TClientId clientid = 0;
	TPropIndex propindex = 0xFF;
	if ( args.size() > 0 )
	{
		clientid = atoi(args[0].c_str());
		if ( args.size() > 1 )
		{
			propindex = atoi(args[1].c_str());
		}
	}

	if ( clientid == 0 )
		log.displayNL( "FEPRIO: Searching for all clients" );
	else
		log.displayNL( "FEPRIO: Searching for client %hu", clientid );

	// Count
	THostMap& clientmap = CFrontEndService::instance()->instance()->receiveSub()->clientMap();
	THostMap::iterator icm;
	for ( icm=clientmap.begin(); icm!=clientmap.end(); ++icm )
	{
		if ( (clientid == 0) || (clientid == GETCLIENTA(icm)->clientId()) )
		{
			GETCLIENTA(icm)->PropDispatcher.displayCounts( propindex, &log );
		}
	}
	return true;
}

