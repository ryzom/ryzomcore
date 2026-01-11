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



#ifndef NL_ENTITY_CONTAINER_H
#define NL_ENTITY_CONTAINER_H

#include "nel/misc/types_nl.h"
#include "game_share/entity_types.h"
#include "game_share/mirror_prop_value.h"
#include "game_share/tick_event_handler.h"
#include "game_share/ryzom_mirror_properties.h"
#include "client_id_lookup.h"
#include "fe_types.h"
#include <map>
#include <set>

//	made to allow template method to support TDataSetRow without specialization.
template	<class T>
inline	uint32	asUInt32	(const	T&	obj)
{
	return	(uint32)obj;
}

template	<>
inline	uint32	asUInt32<TDataSetRow>	(const	TDataSetRow&	obj)
{
	return	obj.getIndex();
}

extern bool verboseVision;

extern NLMISC::CLog		  *TmpDebugLogger;


#ifdef NL_RELEASE
#define LOG_VISION ;
#else
//#define LOG_VISION if (!verboseVision) {} else nldebug
#define LOG_VISION TmpDebugLogger->displayNL
#endif

#define STORE_MIRROR_VP_IN_CLASS
//#undef STORE_MIRROR_VP_IN_CLASS


extern TPropertyIndex DSPropertyTickPos;
extern TPropertyIndex DSPropertyLocalX;
extern TPropertyIndex DSPropertyLocalY;
extern TPropertyIndex DSPropertyLocalZ;
//extern TPropertyIndex DSPropertyStunned;
extern TPropertyIndex DSFirstPropertyAvailableImpulseBitSize;


typedef uint64 TPropertiesValue;


extern const TDataSetIndex LAST_VISION_CHANGE;

/// Vision associations: entityindex, slot and 'replace' bool
typedef std::map< TEntityIndex, std::pair<CLFECOMMON::TCLEntityId,bool> > TMapOfVisionAssociations;

/// Information for vision disassociations: slots
typedef std::set< CLFECOMMON::TCLEntityId > TSetOfRemovedEntities;

/// Vision associations received before an entity is in mirror (TEMP workaround)
//typedef std::multimap< NLMISC::CEntityId, std::pair< TEntityIndex, std::pair<CLFECOMMON::TCLEntityId,bool> > > TMapOfEarlyVisionAssociations;



/**
 * CEntity
 */
struct CEntity
{
	/// Cache for property initialized
	bool										PropInitialized[CLFECOMMON::NB_VISUAL_PROPERTIES];

	/// Property X direct access + previous value
	CMirrorPropValueROCF<CLFECOMMON::TCoord>	X;

	/// Property Y direct access + previous value
	CMirrorPropValueROCF<CLFECOMMON::TCoord>	Y;

#ifdef STORE_MIRROR_VP_IN_CLASS
	CMirrorPropValueRO<TYPE_SHEET>				VP_SHEET;
	CMirrorPropValueRO<TYPE_BEHAVIOUR>			VP_BEHAVIOUR;
	CMirrorPropValueRO<TYPE_NAME_STRING_ID>		VP_NAME_STRING_ID;
	CMirrorPropValueRO<TYPE_TARGET_ID>			VP_TARGET_ID;
	CMirrorPropValueRO<TYPE_CONTEXTUAL>			VP_CONTEXTUAL;
	CMirrorPropValueRO<TYPE_MODE>				VP_MODE;
	CMirrorPropValueRO<TYPE_BARS>				VP_BARS;
	CMirrorPropValueRO<TYPE_VPA>				VP_VPA;
	CMirrorPropValueRO<TYPE_VPB>				VP_VPB;
	CMirrorPropValueRO<TYPE_VPC>				VP_VPC;
	CMirrorPropValueRO<TYPE_ENTITY_MOUNTED_ID>	VP_ENTITY_MOUNTED_ID;
	CMirrorPropValueRO<TYPE_RIDER_ENTITY_ID>	VP_RIDER_ENTITY_ID;
	CMirrorPropValueRO<TYPE_TARGET_LIST>		VP_TARGET_LIST;
	CMirrorPropValueRO<TYPE_VISUAL_FX>			VP_VISUAL_FX;
	CMirrorPropValueRO<TYPE_GUILD_SYMBOL>		VP_GUILD_SYMBOL;
	CMirrorPropValueRO<TYPE_GUILD_NAME_ID>		VP_GUILD_NAME_ID;
	CMirrorPropValueRO<TYPE_EVENT_FACTION_ID>	VP_EVENT_FACTION_ID;
	CMirrorPropValueRO<TYPE_PVP_MODE>			VP_PVP_MODE;
	CMirrorPropValueRO<TYPE_PVP_CLAN>			VP_PVP_CLAN;
	CMirrorPropValueRO<TYPE_OWNER_PEOPLE>		VP_OWNER_PEOPLE;
	CMirrorPropValueRO<TYPE_OUTPOST_INFOS>		VP_OUTPOST_INFOS;
#endif

	/// Manhattan distance accumulated when moving
	uint32										Mileage;

	/// Multi target has changed
	bool										MultiTargetChanged;

	/// Game cycle of the latest change of pos on this entity
	CMirrorPropValueRO<NLMISC::TGameCycle>		TickPosition;

	/// Index of the next updated entity (for the vision) in the chain
	TEntityIndex								NextUpdatedEntityVision;

	/// New entities in the vision
	TMapOfVisionAssociations					VisionIn;

	/// Old entities in the vision
	TSetOfRemovedEntities						VisionOut;

	static CMirroredDataSet						*DataSet;

	// Not used because there is only one gamecycle stored per entity
	//NLMISC::TGameCycle getGameCycleForProperty( TPropIndex ) { return TickPosition; }

	/// Initialize struct
	void initFields( const TDataSetRow& entityIndex, NLMISC::CEntityId entityId )
	{
		if ( ! X.isReadable() )
			X.init( *DataSet, entityIndex, DSPropertyPOSX );
		if ( ! Y.isReadable() )
			Y.init( *DataSet, entityIndex, DSPropertyPOSY );
		TickPosition.init( *DataSet, entityIndex, DSPropertyTickPos );
		Mileage = 0;
		NextUpdatedEntityVision.initToLastChanged(); // = LAST_VISION_CHANGE;

#ifdef STORE_MIRROR_VP_IN_CLASS
		VP_SHEET.init( *DataSet, entityIndex, DSPropertySHEET );
		VP_BEHAVIOUR.init( *DataSet, entityIndex, DSPropertyBEHAVIOUR );
		VP_NAME_STRING_ID.init( *DataSet, entityIndex, DSPropertyNAME_STRING_ID );
		VP_TARGET_ID.init( *DataSet, entityIndex, DSPropertyTARGET_ID );
		VP_CONTEXTUAL.init( *DataSet, entityIndex, DSPropertyCONTEXTUAL );
		VP_MODE.init( *DataSet, entityIndex, DSPropertyMODE );
		VP_BARS.init( *DataSet, entityIndex, DSPropertyBARS );
		VP_VPA.init( *DataSet, entityIndex, DSPropertyVPA );
		VP_VPB.init( *DataSet, entityIndex, DSPropertyVPB );
		VP_VPC.init( *DataSet, entityIndex, DSPropertyVPC );
		VP_ENTITY_MOUNTED_ID.init( *DataSet, entityIndex, DSPropertyENTITY_MOUNTED_ID );
		VP_RIDER_ENTITY_ID.init( *DataSet, entityIndex, DSPropertyRIDER_ENTITY_ID );
		VP_TARGET_LIST.init( *DataSet, entityIndex, DSPropertyTARGET_LIST );
		VP_VISUAL_FX.init( *DataSet, entityIndex, DSPropertyVISUAL_FX );
		VP_GUILD_SYMBOL.init( *DataSet, entityIndex, DSPropertyGUILD_SYMBOL );
		VP_GUILD_NAME_ID.init( *DataSet, entityIndex, DSPropertyGUILD_NAME_ID );
		VP_EVENT_FACTION_ID.init( *DataSet, entityIndex, DSPropertyEVENT_FACTION_ID );
		VP_PVP_MODE.init( *DataSet, entityIndex, DSPropertyPVP_MODE );
		VP_PVP_CLAN.init( *DataSet, entityIndex, DSPropertyPVP_CLAN );
		VP_OWNER_PEOPLE.init( *DataSet, entityIndex, DSPropertyOWNER_PEOPLE );
		VP_OUTPOST_INFOS.init( *DataSet, entityIndex, DSPropertyOUTPOST_INFOS );
#endif

		invalidateProperties();
	}

	/// Reinitialize Initialized state of properties
	void invalidateProperties()
	{
		for ( sint p=0; p!=CLFECOMMON::MAX_PROPERTIES_PER_ENTITY; ++p )
		{
			PropInitialized[p] = false;
		}
	}

	/// Return the current cached state without checking the value
	bool propIsInitializedState( CLFECOMMON::TPropIndex propIndex ) const
	{
		return PropInitialized[propIndex];
	}

	/** Return true if the property has been set (at least once) to non-zero in the mirror.
	 * For positions, use positionIsInitialized() instead.
	 */
	template <class T>
	bool propertyIsInitialized( CLFECOMMON::TPropIndex propIndex, TPropertyIndex dsPropertyIndex, const TEntityIndex& entityIndex, T* pt )
	{
		return PropInitialized[propIndex] || checkPropertyInitialized( propIndex, dsPropertyIndex, entityIndex, pt );
	}

	/// Return if the position of the current processed entity is initialized
	bool positionIsInitialized( const TEntityIndex& entityIndex )
	{
		return PropInitialized[CLFECOMMON::PROPERTY_POSITION] || checkPositionInitialized( entityIndex );
	}
	
	/// Return if the position of the current processed entity is initialized
	bool positionIsInitialized()
	{
		return PropInitialized[CLFECOMMON::PROPERTY_POSITION] || checkPositionInitialized();
	}
	
	/// Helper for propertyIsInitialized()
	template <class T>
	bool checkPropertyInitialized( CLFECOMMON::TPropIndex propIndex, TPropertyIndex dsPropertyIndex, const TEntityIndex& entityIndex, T* )
	{
		CMirrorPropValueRO<T> value( *DataSet, entityIndex, dsPropertyIndex );
		return PropInitialized[propIndex] = ( value() != 0 );
/*
		if ( value() != 0 )
		{
			PropInitialized[propIndex] = true;
			return true;
		}
		else
			return false;
*/
	}

	/// Helper for propertyIsInitialized(): overload for row properties (specialization syntax is too much different among compilers)
	bool checkPropertyInitialized( CLFECOMMON::TPropIndex propIndex, TPropertyIndex dsPropertyIndex, const TEntityIndex& entityIndex, TDataSetRow* )
	{
		CMirrorPropValueRO<TDataSetRow> value( *DataSet, entityIndex, dsPropertyIndex );
		return PropInitialized[propIndex] = ( asUInt32<TDataSetRow>(value()) != 0 );
/*
		if ( asUInt32<TDataSetRow>(value()) != 0 ) // test as integer
		{
			PropInitialized[propIndex] = true;
			return true;
		}
		else
			return false;
*/
	}

	/// Helper for positionIsInitialized()
	bool checkPositionInitialized( const TEntityIndex& entityIndex )
	{
		return PropInitialized[CLFECOMMON::PROPERTY_POSITION] = (X() != 0 || Y() != 0);
/*
		if ( ! ((X() == 0) && (Y() == 0)) ) // not local pos because can be 0 0
		{
			PropInitialized[PROPERTY_POSITION] = true;
#ifdef NL_DEBUG
			nldebug( "%u: E%u: Position initialized to %d %d", CTickEventHandler::getGameCycle(), entityIndex.getIndex(), X(), Y() );
#endif
			return true;
		}
		else
			return false;
*/
	}

	/// Helper for positionIsInitialized()
	bool checkPositionInitialized()
	{
		return PropInitialized[CLFECOMMON::PROPERTY_POSITION] = (X() != 0 || Y() != 0);
	}


	/// Insert entityId, slot into VisionIn 
	//void insertIntoVisionIn( TEntityIndex iviewer, const NLMISC::CEntityId& entityId, CLFECOMMON::TCLEntityId slot, bool replace );
	void insertIntoVisionIn( const TEntityIndex& iviewer, const TDataSetRow& entityIndex, CLFECOMMON::TCLEntityId slot, bool replace );

	CLFECOMMON::TCoord	posXm( const TEntityIndex& ) const
	{
		return X()/1000;
	}
	
	CLFECOMMON::TCoord	posYm( const TEntityIndex& ) const
	{
		return Y()/1000;
	}

	CLFECOMMON::TCoord	posZm( const TEntityIndex& entityIndex ) const
	{
		return z( entityIndex )/1000;
	}

	CLFECOMMON::TCoord	z( const TEntityIndex& entityIndex ) const
	{
		CMirrorPropValueRO<sint32> zValue( *DataSet, entityIndex, DSPropertyPOSZ );
		return zValue();
	}

	CLFECOMMON::TCoord	posLocalXm( const TEntityIndex& entityIndex ) const
	{
		CMirrorPropValueRO<sint32> prop( *DataSet, entityIndex, DSPropertyLocalX );
		return prop()/1000; 
	}

	CLFECOMMON::TCoord	posLocalYm( const TEntityIndex& entityIndex ) const
	{
		CMirrorPropValueRO<sint32> prop( *DataSet, entityIndex, DSPropertyLocalY );
		return prop()/1000;
	}

	CLFECOMMON::TCoord	posLocalZm( const TEntityIndex& entityIndex ) const
	{
		CMirrorPropValueRO<sint32> prop( *DataSet, entityIndex, DSPropertyLocalZ );
		return prop()/1000;
	}

	/// Compute the manhattan mileage with X
	void				updateMileageX( const TEntityIndex& entityIndex )
	{
		//H_BEFORE(TestX)
		if ( ! X.isReadable() )
			X.init( *DataSet, entityIndex, DSPropertyPOSX );
		//H_AFTER(TestX)

#ifdef NL_DEBUG
		if ( X() == 0 )
			nldebug( "X is zero" );
#endif
		//H_BEFORE(XMileage)
		Mileage += abs( X() - X.previousValue() );
		X.resetChangeFlag();
		//H_AFTER(XMileage)
	}

	/// Compute the manhattan mileage with Y
	void				updateMileageY( const TEntityIndex& entityIndex )
	{
		//H_BEFORE(TestY)
		if ( ! Y.isReadable() )
			Y.init( *DataSet, entityIndex, DSPropertyPOSY );
		//H_AFTER(TestY)

#ifdef NL_DEBUG
		if ( Y() == 0 )
			nldebug( "Y is zero" );
#endif
		//H_BEFORE(YMileage)
		Mileage += abs( Y() - Y.previousValue() );
		Y.resetChangeFlag();
		//H_AFTER(YMileage)
	}

	void				displayProperties( const TEntityIndex& entityIndex, NLMISC::CLog *log=NLMISC::InfoLog, TClientId optClientId=INVALID_CLIENT, CLFECOMMON::TCLEntityId optSlot=CLFECOMMON::INVALID_SLOT ) const;

	static void			fillVisualPropertiesFromMirror( uint64 properties[], const TEntityIndex& entityIndex );
};


/**
 * <Class description>
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CEntityContainer
{
public:

	/// Instance
	static CEntityContainer*	I;

	/// Constructor
	CEntityContainer();

	/// Initialisation
	void					init( CClientIdLookup *cl, void (*cbUpdate)(), void (*cbSync)() = NULL );

	/// Update (called every cycle by the mirror system)
	void					updateMirror();

	/** Get the entity structure corresponding to an entity index
	 * Preconditions:
	 * entityIndex != INVALID_ENTITY_INDEX
	 * entityIndex < _Entities.size()
	 */
	CEntity							*getEntity( const TEntityIndex& entityIndex )
	{
#ifdef NL_DEBUG
		nlassert( entityIndex.isValid() );
		nlassert( entityIndex.getIndex() < (uint32)_Entities.size() );
#endif
		return &_Entities[entityIndex.getIndex()];
	}

	/// Return the entity index corresponding to an entity id (INVALID_ENTITY_INDEX if failed)
	TEntityIndex					entityIdToIndex( const NLMISC::CEntityId& entityId ) const
	{
		return _DataSet->getDataSetRow( entityId );
	}

	/// Convert a visual property index into a dataset property index (access to a static array)
	static TPropertyIndex			propertyIndexInDataSetToVisualPropIndex( CLFECOMMON::TPropIndex vPropIndex )
	{
		return _VisualPropIndexToDataSet[vPropIndex];
	}
	
	/// Initialisation of the properties (after mirror system is ready)
	void							initMirror();

	/// Return true when the mirror is ready
	bool							mirrorIsReady() const { return _Mirror.mirrorIsReady(); }

	/// Return the dataset
	CMirroredDataSet&				dataset() { return *_DataSet; }

	/// Return the mirror
	const CMirror&					mirrorInstance() { return _Mirror; }

	CClientIdLookup					*EntityToClient;

	/// Temporary set of entities entering vision, received before their entityIndex is known (workaround)
	// removed, there should not be any more early visions
	//TMapOfEarlyVisionAssociations	EarlyVisionIns;

protected:

	/// Set the mapping and return the dataset property index
	TPropertyIndex					mapVisualPropIndex( const std::string& propName, CLFECOMMON::TPropIndex vPropIndex );

	/// Return the property index at startup
	TPropertyIndex					getPropertyIndex( const std::string& propName ) const;

private:

	/// Vector of entity structures
	std::vector< CEntity >			_Entities;

	/// Mirror
	CMirror							_Mirror;

	/// The dataset used by the front-end service
	CMirroredDataSet				*_DataSet;

	/// Vector to convert dataset property indices into visual property indices
	//std::vector< TPropertyIndex >	_VisualPropIndexToDataSet; // CHANGED BEN
	static TPropertyIndex			_VisualPropIndexToDataSet[CLFECOMMON::NB_VISUAL_PROPERTIES];
};


#define TheEntityContainer (CEntityContainer::I)
#define TheDataset (CEntityContainer::I->dataset())


#endif // NL_ENTITY_CONTAINER_H

/* End of entity_container.h */
