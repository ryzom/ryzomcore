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



#ifndef NL_PROPERTY_DISPATCHER_H
#define NL_PROPERTY_DISPATCHER_H

#include "nel/misc/types_nl.h"
#include "game_share/entity_types.h"
#include "fe_types.h"
#include "vision_array.h"
#include <vector>


/*
 * Forward declarations
 */
class CPropertyIdTranslator;


/*
 * Property shelves: one container (bucket) for parcels for each priority level, per client.
 */
typedef std::vector<TPropParcel> TPropertyShelf;
typedef TPropertyShelf TPropertyShelves [NB_PRIORITIES];


/**
 * Property Dispatcher.
 * It tells which properties to send, ordered by decreasing priority.
 * The properties are referenced in 'shelves' (buckets).
 * There are as many shelves as priority levels.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CPropertyDispatcher
{
public:

	/// Constructor
	CPropertyDispatcher();

	/// Initialization
	void				init( TClientId clientid );

						// __________________________________________________________
						// For Vision Array (called by Vision Provider & Prioritizer)

	/// Change a priority
	void				setPriority( CLFECOMMON::TCLEntityId ceid, CLFECOMMON::TPropIndex propindex, TPropParcelPtr& parcelptr, TPriority newprio )
	{
						//++NbSetPrio; // stats

						if ( parcelptr.Prio != newprio )
						{
							// Remove existing parcel (will be silent if parcelptr.Offset is INVALID_OFFSET)
							removeProp( parcelptr );

							// Add a new enabled parcel and update the pointer to the parcel
							parcelptr.Prio = newprio;
							addProp( parcelptr, ceid, propindex );
						}
						else
						{
							// Only enable the parcel
							if ( parcelptr.Offset < nbParcels( parcelptr.Prio ) ) //TEMP
								enableProp( parcelptr );
							else
								nlwarning( "Invalid offset %hd for enabling parcel in propdispatcher! (client %hu slot %hu prio %hu", parcelptr.Offset, _ClientId, (uint16)ceid, (uint16)parcelptr.Prio );
							//++NbReenable;
						}
	}

	/// Remove from the property dispatcher shelves, or does nothing if offset is INVALID_OFFSET
	void				removeProp( const TPropParcelPtr& parcelptr );

	/// Remove all corresponding properties from the property dispatcher
	void				removePropsFromShelves( TClientId clientid, CLFECOMMON::TCLEntityId ceid );

						// _______________
						// For Prioritizer

	/// Return the number of parcels of a shelf
	TPropParcelOffset	nbParcels( TPriority prio ) { nlassert( prio <= NB_PRIORITIES ); return _PropShelves[prio].size(); }

	/// Return the number of enabled parcels in a shelf
	TPropParcelOffset	nbEnabledParcels( TPriority prio );

						// ___________________________
						// For priority subsystem root

	/// Remove the parcels that have been added to the remove list (call it after a cycle)
	void				flush();

	/// Initialize dispatch cycle
	void				initDispatcherCycle( /*bool rescalePriorities*/ );

	/// Display the contents of the shelves
	void				printShelves( const char *title, bool checkIntegrity, bool proptext=false, bool hidedisabled=false ) const;

	/// Display the number of parcels per shelf
	void				printShelfSizes() const;

	// Threshold value
	//float				hpThreshold() const { return _HPThreshold; }

	// Reset threshold
	//void				resetThreshold() { _HPThreshold = 0.0f; _HPTDelta = 0.0f; }

	// Adjust HPThreshold (dichotomic)
	//void				adjustHPThreshold();

						// _____________________
						// For sending subsystem

	/// Return which property to send next, or NULL if there is no more.
	const TPropParcel	*getNextParcel()
	{
		if ( _NextParcel.Offset == INVALID_OFFSET )
			return NULL;

		// Return current parcel
		const TPropParcel *parcel = &getParcel( _NextParcel );
		//nlassert( parcel->PropIndex < MAX_PROPERTIES_PER_ENTITY );
		return parcel;

		// incNextParcel() will be called by setStatus()
	}

	/** Set status: true if the latest parcel got by getNextParcel() can be sent in the current cycle, otherwise false
	 * If true, the parcel gets disabled.
	 * Must be called after getNextParcel() every time getNextParcel() is called
	 */
	void				setParcelStatusTrue();

	void				setParcelStatusFalse()
	{
		// Advance to next parcel
		incNextParcel();
	}

	/// Return the amount of priorities used corresponding to the number of actions sent last cycle (e.g. 1.2 means priority 0 is full and priority 1 is 20% filled)
	float				getPrioRatio();

						// _____
						// Stats

	// Count the number of properties of a certain type in the property shelves
	void				displayCounts( CLFECOMMON::TPropIndex propindex );

	/// Number of calls to setPriority
	//uint32				NbSetPrio;

	/// Number of calls to setPriority where the prio is unchanged
	//uint32				NbReenable;

protected:

	/// Easy access to a shelf
	TPropertyShelf&		getShelf( TPriority prio ) { /*nlassert( prio <= NB_PRIORITIES );*/ return _PropShelves[prio]; }

	/// Easy access to a parcel
	TPropParcel&		getParcel( const TPropParcelPtr& parcelptr ) { nlassertex( parcelptr.Prio <= NB_PRIORITIES && parcelptr.Offset < nbParcels(parcelptr.Prio), ("prio=%hu offset=%hd", (uint16)parcelptr.Prio, parcelptr.Offset) ); return _PropShelves[parcelptr.Prio][parcelptr.Offset]; }

	/// Easy access to a parcel of a shelf
	TPropParcel&		getParcel( TPropertyShelf& shelf, TPropParcelOffset offset ) { /*nlassert( offset < (TPropParcelOffset)shelf.size() );*/ return shelf[offset]; }

	/// Easy access to the last parcel of a shelf
	TPropParcel&		lastParcel( TPropertyShelf& shelf ) { /*nlassert( ! shelf.empty() );*/ return *(shelf.end()-1); }

	/// Increment pointer to next parcel
	void				incNextParcel();

	/// Skip any empty shelves
	void				skipEmptyShelves();

	/// Add a parcelptr to the remove list. The parcel will be removed at the beginning of the next cycle
	void				addToRemoveList( TPropParcelPtr *pparcelptr )
	{
		_RemoveList.push_back( pparcelptr );
	}

	/// Make a new enabled parcel in the shelf corresponding to parcelptr.Prio and fill parcelptr.Offset
	void				addProp( TPropParcelPtr& parcelptr, CLFECOMMON::TCLEntityId ceid, CLFECOMMON::TPropIndex propindex );

	/// Enable an existing parcel
	void				enableProp( const TPropParcelPtr& parcelptr )
	{
		getParcel( parcelptr ).Enabled = true;
	}

//private:
protected:

	/** Type of the remove list.
	 * Each item points to the member PrioLoc of a TPropState stored in _VisionArray.
	 * It is a pointer because of the following reasons:
	 * - these PrioLoc are modified by removeProp()
	 * - they do not move within memory as the vision array is static
	 */
	typedef std::vector<TPropParcelPtr*> TRemoveList;

	/// Property shelves
	TPropertyShelves			_PropShelves;

	/// Pointer to the next parcel to dispatch
	TPropParcelPtr				_NextParcel;

	/// The remove list
	TRemoveList					_RemoveList;

	/// Vision array
	CVisionArray				*_VisionArray;

	/// Property id translator
	CPropertyIdTranslator		*_PropTranslator;

	/// Destination client
	TClientId					_ClientId;

	// Threshold used to control that the number of elements in priority 0 (Highest Prio) is stable
	//float						_HPThreshold;

	// Difference between previous and current _HPThreshold
	//float						_HPTDelta;

	/// Number of parcels sent in the current cycle
	//uint32						_NbSentParcelsInCycle;
};


#endif // NL_PROPERTY_DISPATCHER_H

/* End of property_dispatcher.h */
