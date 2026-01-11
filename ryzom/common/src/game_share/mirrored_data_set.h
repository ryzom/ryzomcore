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



#ifndef NL_MIRRORED_DATA_SET_H
#define NL_MIRRORED_DATA_SET_H

#include <nel/misc/types_nl.h>
#include <nel/net/message.h>

#include "mirrored_data_set_types.h"


/// Define/undef to show/hide entity creations, additions & removals (nldebug)
#undef DISPLAY_ROW_CHANGES


/**
 * Type of a user callback for smart-displaying the value of a property
 * The pointer passed in argument will be the pointer on the value.
 *
 * Example:
 * string cbBehaviourToString( void *ptValue )
 * {
 *    return CBehaviour( *((uint32*)ptValue) ).toString();
 * }
 */
typedef std::string (*TValueToStringFunc) (void *);


/**
 * A dataset handles several properties (ex: X Y Z).
 *
 * One property belongs to only one dataset.
 * The dataset contains the values of these properties for the entities that have these (ex:
 * a player has a property X therefore its entity will be added into the dataset that handles
 * property X).
 *
 * One entity can be added in several datasets (ex: if another dataset handles
 * property Behaviour, player entities will be added into it as well).
 *
 * To access/change a value of a property, the best way is to instanciante an
 * object of the class CMirrorPropValue<T> where T is the type of the value.
 *
 * Most of the following method comments give an indication on the time-complexity.
 * Fast means it uses indexed-array addressing, while slow means that some map or
 * hash map look-ups are involved.
 *
 * \seealso CMirror
 * \seealso CMirrorPropValue
 * \seealso CMirrorPropValueCF
 * \seealso CMirrorPropValueBaseCF
 * \seealso CMirrorPropValueList
 * \seealso CMirrorPropValueAlice
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CMirroredDataSet : public CDataSetBase
{
public:

	//--- PROPERTY INIT --------------------------------------------------------------------------

	/**
	 * Declare that the current service is interested in the specified property
	 * (which must belong to the dataset).
	 *
	 * It means you want the values of the property to be updated each time
	 * they are changed by another service (formerly called 'subscribing').
	 *
	 * You may also change a value so that it will be reflected to all other
	 * services interested in the property (formerly called 'emitting').
	 *
	 * Call this method in the callback provided to CMirror::init(), not before.
	 *
	 * Arguments:
	 * - If the current service will never modify any value of the property,
	 * set PSOReadOnly in options for optimisation (incompatible with
	 * PSOWriteOnly and PSOReadWrite).
	 * - If the current service will only write some values and is not
	 * interested in the other values possibly changes by another service,
	 * set PSOWriteOnly in options for optimisation. Use with caution, only
	 * if the services that can write the property do not handle the same
	 * rows (incompatible with PSOReadOnly and PSOReadWrite)
	 * - If the current service will modify and read changes of the property,
	 * set PSOReadWrite in options.
	 * - If the current service needs to be notified when a value has been
	 * changed by another service, using the notification methods,
	 * set PSONotifyChanges in options (incompatible with PSOWriteOnly).
	 * - If you want the value to be displayed every time a property value
	 * is assigned in the mirror (using CMirrorPropValue::operator=() or
	 * CMirroredDataSet::setValue() or CMirrorPropValueList operations by
	 * the current service), set PSOMonitorAssignment in options. This debug
	 * feature is available only in mode NL_DEBUG (otherwise it has no
	 * effect).
	 * - If POSNotifyChanges is set and notifyGroupByPropName is empty
	 * (or not specified in the arguments), each change will be notified
	 * by its property, using the methods getNextChangedValueForProp() (or
	 * getNextChangedValue()) specifying (or reporting) a property index.
	 * Instead, you can set notifyGroupByPropName to a previously declared
	 * property name (or the same one). Then all changes concerning the
	 * properties declared with the same notifyGroupByPropName argument
	 * will be notified at the same time. For example, declaring X, Y, Z,
	 * Theta with X as notifyGroupByPropName will notify when one of these
	 * properties has changed for an entity. If several of these have
	 * changed since the last query, only one change will be notified. To
	 * query changes for the whole group, use getNextChangedValueForProp()
	 * (or getNextChangedValue()) with the property index of the property
	 * passed in notifyGroupByPropName (in the example, X). Note: the
	 * group property must belong to the same dataset as the declared
	 * property.
	 *
	 * Example:
	 * declareProperty( "CurrentColor", PSOReadOnly );
	 * declareProperty( "X", PSOReadWrite | PSONotifyChanges, "X" );
	 * declareProperty( "Y", PSOReadWrite | PSONotifyChanges, "X" );
	 *
	 * The property will not be ready (i.e. readable or modifiable) until
	 * CMirror::mirrorIsReady() returns true.
	 *
	 * Note: please make sure that two distinct services do not change the
	 * same value at the same time.
	 */
	void				declareProperty( const std::string& propName,
										 TPropSubscribingOptions options,
										 const std::string notifyGroupByPropName="" );

	//--- ENTITY ADDITION ----------------------------------------------------------------------------------

	/**
	 * Declare an entity previously added by CMirror::createEntity() in the dataset
	 * (if the entity type is matching the dataset) to other services.
	 *
	 * To obtain the entity index from the entityId, use getDataSetRow().
	 *
	 * IMPORTANT: You are allowed to call this method only in code synchronized with the mirror.
	 * Refer to CMirror::isRunningSynchronizedCode() to know the conditions.
	 *
	 * \seealso CMirror::declareEntity() for alternate version.
	 */
	void				declareEntity( const TDataSetRow& entityIndex );


	///--- ACCESSORS ---------------------------------------------------------------------------------------
	//
	// To access/reference a property value and keep the reference,
	// please instanciate a CMirrorPropValue.
	//
	// You can get the "change timestamp" calling getChangeTimestamp() in the base class.
	//

	/**
	 * Return true if the row is currently accessible, meaning the corresponding entity is online now
	 * and can be accessed in the mirror, If row.isNull(), return false.
	 * If another service B on the same machine despawns the entity in the same game cycle, after
	 * isAccessible() returned true in A, the row can still be accessed (read/written) by A.
	 *
	 * WARNING: don't assert isAccessible().
	 * The following code is WRONG:
	 *   if ( TheDataset.isAccessible( r ) )
	 *		doRead( r );
	 *   (...)
	 *   void doRead( const TDataSetRow& r )
	 *   {
	 *      nlassert( TheDataset.isAccessible( r ) );
	 *      CMirrorPropValue<uint32> value( TheDataset, r, DSMyProperty );
	 *      cout << value();
	 *   }
	 * Why? Because AT ANY MOMENT the status of isAccessible() can change to false.
	 * It happens when a service B on the same machine despawns the entity.
	 * However, reading the value will still work even if the entity despawned between
	 * the call to isAccessible() and the call to doRead().
	 * The doRead() function will be CORRECT without the nlassert or by replacing
	 * it with an harmless 'if(TheDataset.isAccessible(r))'.
	 */
	inline bool					isAccessible( const TDataSetRow& row ) const;

	/// Return a string showing the values tested by isAccessible()
	std::string					explainIsAccessible( const TDataSetRow& row ) const;

	/**
	 * Same as isAccessible(), but does not check if the row is online.
	 * Thus it will return true for a row that is created locally but yet unpublished
	 * (which is not called "online").
	 * Caution: it will also return true if the row is offline, provided the current
	 * counter matches the counter in 'row'.
	 */
	inline bool					isAccessible2( const TDataSetRow& row ) const;

	/**
	 * Get the entity id corresponding to entityIndex in the dataset (linear access).
	 *
	 * Validity: same as getDataSetRow() (see below).
	 * Warning: now this method checks if the datasetrow is still valid.
	 * If the row has been reassigned, it will return Unknown
	 */
	const NLMISC::CEntityId&	getEntityId( const TDataSetRow& entityIndex ) const;

	/**
	 * Get the row corresponding to the specified entity id in the dataset (result isNull() if not found) (map-style access).
	 *
	 * getDataSetRow() will return the entityId:
	 * - Between createEntity() and removeEntity() if the entity is managed by the same service
	 * - Between the adding and the removal notifications if the entity is managed by another service
	 */
	TDataSetRow					getDataSetRow( const NLMISC::CEntityId& entityId ) const;

	/**
	 * Get a valid datasetrow usable with the current entity at entity index, if the entity is online.
	 * If the entity is offline, the method returns an invalid datasetrow (offline means the row is not used;
	 * warning: after despawning an entity by a service A, and before notifying the entity removal in a
	 * service B, the entity will be reported as offline).
	 */
	TDataSetRow					getCurrentDataSetRow( TDataSetIndex entityIndex ) const;

	/**
	 * Get a valid datasetrow usable with the current entity at entity index, be the entity online or not.
	 * warning: after despawning an entity by a service A, and before notifying the entity removal in a
	 * service B, the entity will be reported as offline).
	 */
	TDataSetRow					forceCurrentDataSetRow( TDataSetIndex entityIndex ) const;

	/** Access the map of entities (begin iteration). Use GET_ENTITY_INDEX(it) to access the TDataSetRow index.
	 * You can iterate it to get the online entities (use GET_ENTITY_INDEX to obtain a dataset index from
	 * the iterator, then call getCurrentDataSetRow() to have a dataset row, and check isValid() in the case
	 * when the entity would have been despawned and the service not notified yet.
	 */
	inline TEntityIdToEntityIndexMap::const_iterator	entityBegin() const;

	/// Access the map of entities (test end of iteration).
	inline TEntityIdToEntityIndexMap::const_iterator	entityEnd() const;

	/// Get the property index corresponding to the specified property name in the dataset (slow)
	TPropertyIndex		getPropertyIndex( const std::string& propName ) const;

	/// Set a property value, by fast index
	template <class T, class CPropLocationClass>
	void				setValue( const TDataSetRow& entityIndex, TPropertyIndex propertyIndex, T value );

	/// Set a property value, by slow ids
	template <class T, class CPropLocationClass>
	void				setValue( const NLMISC::CEntityId& entityId, const std::string& propName, T value );

	/// Get a property value, by fast index
	template <class T>
	void				getValue( const TDataSetRow& entityIndex, TPropertyIndex propIndex, T& returnedValue ) const
						{ T *ptvalue; getPropPointer( &ptvalue, propIndex, entityIndex ); returnedValue = *ptvalue; }

	/// Get a property value, by slow ids
	template <class T>
	void				getValue( const NLMISC::CEntityId& entityId, const std::string& propName, T& returnedValue ) const;


	//--- ENTITY ADDITION/REMOVAL NOTIFICATION (ROW MANAGEMENT) --------------------------------------------
	//
	// You must call, in the notification callback provided to
	// CMirror::setNotificationCallback(), processAddedEntities()
	// (if you don't use addition notification) or beginAddedEntities()+
	// getNextAddedEntity()+endAddedEntities();
	// and you must call processRemovedEntities() (if you don't use removal notification)
	// or beginRemovedEntities()+getNextRemovedEntity()+endRemovedEntities().
	//
	// // Sample code (with removal and addition notification):
	// TDataSetRow entityIndex;
	//
	// TheDataset.beginAddedEntities();
	// while ( (entityIndex = TheDataset.getNextAddedEntity()) != LAST_CHANGED )
	//    (...) // your own processing here
	// TheDataset.endAddedEntities();
	//
	// CEntityId *pEntityId;
	// TheDataset.beginRemovedEntities();
	// while ( (entityIndex = TheDataset.getNextRemovedEntity( &pEntityId )) != LAST_CHANGED )
	//    (...) // your own processing here
	// TheDataset.endRemovedEntities();
    //
	// Important note: it is NOT garanteed that an addition notification for an entity occurs
	// before a change notification for this entity. If you use rows (TDataSetRow) to manage
	// your objects, this should not be a problem.
	//

	/**
	 * Begin a browsing session (see getNextAddedEntity(), endAddedEntities()), only in
	 * the notification callback provided to CMirror::setNotificationCallback(). The
	 * browsing of added entities must be done BEFORE the browsing of the removed entities.
	 */
	void				beginAddedEntities() {}

	/**
	 * Continue browsing the added entities, until the method returns LAST_CHANGED.
	 * They can have been added either by the current (caller) service or by another
	 * service that owns some entities concerned by the dataset.
	 */
	TDataSetRow			getNextAddedEntity();

	/// End browsing the added entities.
	void				endAddedEntities() {}

	/**
	 * If you need not browsing the added entities, you must call this method *instead* of
	 * beginAddedEntities() + {getNextAddedEntity()} + endAddedEntities().
	 * Must be done BEFORE the browsing of removed entities.
	 * Call only when mirrorIsReady() is true.
	 */
	void				processAddedEntities();

	/**
	 * Begin a browsing session (see getNextRemovedEntity(), endRemovedEntities()), only in
	 * the notification callback provided to CMirror::setNotificationCallback(). The
	 * browsing of removed entities must be done AFTER the browsing of the removed entities.
	 */
	void				beginRemovedEntities() {}

	/**
	 * Continue browsing the removed entities, until the method returns LAST_CHANGED.
	 * They can have been removed either by the current (caller) service or by another
	 * service that owns some entities concerned by the dataset.
	 * You must not use the entity index anymore. That is why the method also outputs the
	 * entity id of the removed entity (note: it makes the pointer you provide pointing to
	 * the entity id. The pointer will not remain valid after calling endRemovedEntities()).
	 * Note: The returned datasetrow has the same counter as it had in getNextAddedEntity().
	 * It means you can use the whole TDataSetRow as a key of a map/set, inserting the result
	 * of getNextAddedEntity() and erasing the result of getNextRemovedEntity().
	 */
	TDataSetRow			getNextRemovedEntity( NLMISC::CEntityId **entityId );

	/// End browsing the removed entities. Be sure to call this method every cycle.
	void				endRemovedEntities() {}

	/**
	 * If you need not browsing the removed entities, you must call this method *instead* of
	 * beginRemovedEntities() + {getNextRemovedEntity()} + endRemovedEntities().
	 * Must be done AFTER the browsing of removed entities.
	 * Call only when mirrorIsReady() is true.
	 */
	void				processRemovedEntities();


	//--- PROPERTY CHANGE NOTIFICATION ---------------------------------------------------------------------
	//
	// You don't need to call this methods if you don't use property notification.
	//

	/**
	 * Browse the values of properties that have changed in the dataset since the previous
	 * cycle. Only the properties for which you have subscribed with notifyValueChanges set
	 * to true will be reported. You may use a different notification scheme instead,
	 * specifying for which property you query the changes (see ...forProp() methods below).
	 * Don't use both schemes in the same service (they are not compatible).
	 */
	void				beginChangedValues();

	/**
	 * Continue browsing the changed values, until the method returns LAST_CHANGED in entityIndex.
	 * See also setFastCallbackForAdditionHasBeenNotified().
	 */
	void				getNextChangedValue( TDataSetRow& entityIndex, TPropertyIndex& propIndex );

	/// End browsing the changed values. Be sure to call this method every cycle.
	void				endChangedValues();


	/**
	 * Browse the values for a particular property (for which you have subscribed with
	 * notifyValueChanges set to true). Use *either* beginChangedValues()/getNextChangedValue()/
	 * endChangedValues() or beginChangedValueForProp()/getNextChangedValueForProp()/
	 * endChangedValuesForProp(), not both notifications schemes in the same service
	 * (they are not compatible).
	 */
	void				beginChangedValuesForProp( TPropertyIndex propIndex );

	/**
	 * Continue browsing the changed values for the property passed to beginChangedValuesForProp(),
	 * until the method returns LAST_CHANGED. See also setFastCallbackForAdditionHasBeenNotified().
	 */
	TDataSetRow			getNextChangedValueForProp();

	/**
	 * End browsing the changed values for the property passed to beginChangedValuesForProp().
	 * Be sure to call this method every cycle.
	 */
	void				endChangedValuesForProp();


	// --- INFORMATION -------------------------------------------------------------------------------------

	/// Return the number of our entities currently existing (only the ones created by the current service)
	uint32				getNbOwnedEntities()	{ return _NumberOfCurrentCreatedEntities; }

	/// Return the list of declared ranges by entity type, by all services
	const TDeclaredEntityRangeOfType& getDeclaredEntityRanges() const { return _DeclaredEntityRanges; }

	/// Return the name of the dataset
	const std::string&	name() const { return _DataSetName; }

	/// Return true if the specified property represents a list and not a single value
	bool				propIsList( TPropertyIndex propIndex ) const;

	/// Display all or part of the entities in the dataset (0xFF=no filter). Return the number of entities matching the filters
	uint				displayEntities( NLMISC::CLog& log=*NLMISC::InfoLog, uint8 onlyEntityType=0xFF, uint8 onlyCreatorId=0xFF, uint8 onlyDynamicId=0xFF, bool hideUndeclaredbool=false, bool displayLocalCreationTimestamp=true, bool sortByEntityId=false, bool sortByDatasetRow=false ) const;

	/// Look for a property value (if anyValue if true, ignores searchedValue and displays all values for the property) (see displayEntities() for common params)
	uint				lookForValue( NLMISC::CLog& log=*NLMISC::InfoLog, const std::string& propName="", bool anyValue=false, const std::string& searchedValueStr="0", uint8 onlyEntityType=0xFF, uint8 onlyCreatorId=0xFF, uint8 onlyDynamicId=0xFF, bool hideUndeclaredbool=false, bool displayLocalCreationTimestamp=true, bool sortByEntityId=false, bool sortByDatasetRow=false ) const;

	/// Return the number of declared entities
	uint				getNbOnlineEntities() const { return (uint)_EntityIdToEntityIndex.size() - _NumberOfUndeclaredEntities; }

	/// Return the number of undeclared entities
	uint				getNbOfflineEntities() const { return _NumberOfUndeclaredEntities; }

	/// Display the values of one property for all entities
	template <class T, class CPropLocationClass>
	void				displayPropValues( TPropertyIndex propIndex, T* pt, NLMISC::CLog& log=*NLMISC::InfoLog ) const;

	/// Display a value
	void				displayPropValue( const std::string& propName, const TDataSetRow& entityIndex, TPropertyIndex propIndex, const char *flagsStr, NLMISC::CLog& log=*NLMISC::InfoLog ) const;

	/// Set a display callback for the specified property. It will be used by CMirror::displayRows() (command "displayMirrorRow")
	void				setDisplayCallback( TPropertyIndex propIndex, TValueToStringFunc cb );

	/// Change a value from a string
	void				setValueFromString( const TDataSetRow& entityIndex, TPropertyIndex propIndex, const std::string& valueStr );

	/// Return a property value as string
	void				getValueToString( const TDataSetRow& entityIndex, TPropertyIndex propIndex, std::string& result ) const;

	/// Display the trackers
	void				displayTrackers( NLMISC::CLog& log=*NLMISC::InfoLog ) const;

	/// Scan the rows for entities that we wouldn't know, and display them
	void				displayUnknownEntities( TDataSetIndex first, TDataSetIndex end, NLMISC::CLog& log );

	/// Report a number of list cells from uncleared previous session (call before clearing them)
	static void			reportOrphanSharedListCells( sint32 nb );

	/// Return the number of known shared list cells
	static sint32		getNbKnownSharedListCells();

protected:

	friend class CMirror;
	friend class TDataSets;

	// Can't declare template classes as friend!
	//friend class CMirrorPropValue;
	//friend class CPropLocationPacked;
	//friend class CPropLocationUnpacked;

	/// Init
	void				init( const NLMISC::CSheetId& sheetId, const TDataSetSheet& properties, CPropertyAllocatorClient *pac );

	/// Add a range for an owned entity type
	const TEntityRange&	addEntityTypeRange( uint8 entityTypeId, TDataSetIndex first, TDataSetIndex last );

	/// Add an entity (of owned entity type)
	uint				addEntityToDataSet( bool fillEntityId, NLMISC::CEntityId& entityId, bool declare );

	/// Remove an entity (of owned entity type)
	uint				removeEntityFromDataSet( const NLMISC::CEntityId& entityId );

	/// Return the local mirror service id
	NLNET::TServiceId	localMSId() const;

	/// Add tracker or add self trackers for the specified property
	void				accessNewPropTracker( bool self, TPropertyIndex propIndex, sint32 smid, sint32 mutid );

	/// Add trackers or set the self trackers for adding/removing entities
	void				accessEntityTrackers( bool self, sint32 smidAdd, sint32 smidRemove, sint32 mutidAdd, sint32 mutidRemove );

	/// Serial owned entity ranges (without entity types) and return the number of ranges
	uint				serialOutOwnedRanges( NLMISC::IStream& s ) const;

	/// Scan the rows for entities that we wouldn't know, and return the number of entities added
	uint				scanAndResyncExistingEntities( TDataSetIndex first, TDataSetIndex end, bool deleteGhosts, const std::vector<NLNET::TServiceId8>& spawnerIdsToIgnore );

	///
	void				serialOutMirrorInfo( NLNET::CMessage& msgout );

	/// Scan the rows for 'already set' values for the property, set by the specified service.
	uint16				scanAndResyncProp( TPropertyIndex propIndex, CChangeTrackerClientProp *tracker, NLNET::TServiceId serviceId );

	/// Return the specified self prop tracker, or NULL
	CChangeTrackerClientProp *getSelfPropTracker( TPropertyIndex propIndex );

	/// Empty a list without knowing the type of elements
	void				killList( TDataSetRow entityIndex, TPropertyIndex propIndex );

	///
	void				addDeclaredEntityRange( uint8 entityType, const TEntityRange& src, NLNET::TServiceId declaratorService );

	///
	void				addDeclaredEntityRanges( const TEntityRangeOfType& ranges, NLNET::TServiceId declaratorService );

	///
	void				enableEntityNotification() { _IsEntityNotificationEnabled = true; }

	///
	bool				entityMatchesFilter( const NLMISC::CEntityId& entityId, TDataSetIndex entityIndex, uint8 onlyEntityType, uint8 onlyCreatorId, uint8 onlyDynamicId, bool hideUndeclared ) const;

private:

	typedef std::vector< TValueToStringFunc > TValueToStringFuncVec;

	CPropertyAllocatorClient	*_PropAllocator;

	/// Range management for this service
	TEntityRangeOfType			_EntityTypesRanges;

	/// Information about declarations of all services
	TDeclaredEntityRangeOfType	_DeclaredEntityRanges;

	/**
	 * Map CEntityId --> TDataSetRow. Contains the online entities.
	 * Self addition/removal: mapped in addEntityToDataSet(), unmapped in removeEntityFromDataSet().
	 * Remote addition/removal: mapped in getNextAddedEntity(), unmapped in endRemovedEntities().
	 */
	TEntityIdToEntityIndexMap	_EntityIdToEntityIndex;

	/// Index of current tracker for notification
	uint32						_CurrentChangedPropertyTracker;

	/// List of trackers (not indexed by property), always allocated because they are added and allocated at the same time
	TSelfPropTrackers			_SelfPropTrackers;

	/// List of trackers (indexed by TPropertyIndex), always allocated because they are added and allocated at the same time
	TTrackersLists				_PropTrackers;

	// List of entity tracker filters for other services (same indices as _EntityTrackers[*])
	//TEntityTrackerFilterList	_EntityTrackerFilters;

	/// Lists of entity trackers for other services, always allocated because they are added and allocated at the same time
	TTrackerListForEnt			_EntityTrackers [2];

	/// Self entity trackers, may be not allocated before the smids are received
	CChangeTrackerClient		_SelfEntityTrackers [2];

	/// Callbacks
	TValueToStringFuncVec		_ValueToStringCb;

	/// Number of entities created but not declared
	uint32						_NumberOfUndeclaredEntities;

	/// Number of entities currently existing, created by us
	uint32						_NumberOfCurrentCreatedEntities;

	/// If false, can't notify row management yet
	bool						_IsEntityNotificationEnabled;

public:

	/// Constructor (public because invocated by template <class T> loadForm())
	CMirroredDataSet() : _PropAllocator(NULL), _NumberOfUndeclaredEntities(0), _NumberOfCurrentCreatedEntities(0), _IsEntityNotificationEnabled(false) {}

	/// Tell a property has changed (public because of friend template limitation)
	void				setChanged( const TDataSetRow& datasetRow, TPropertyIndex propIndex );

	/// Return a valueToString callback
	TValueToStringFunc	valueToStringCb( TPropertyIndex propIndex ) const { return _ValueToStringCb[propIndex]; }
};


// Inline and template definitions
#include "mirrored_data_set_inline.h"


#endif // NL_MIRRORED_DATA_SET_H

/* End of mirrored_data_set.h */
