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



#ifndef NL_MIRROR_H
#define NL_MIRROR_H

#include <nel/misc/types_nl.h>
#include <nel/misc/sheet_id.h>
#include <nel/net/unified_network.h> // for service callbacks
#include "nel/net/service.h"

#include "mirror_misc_types.h"
#include "property_allocator_client.h"
#include "nel/net/transport_class.h"


class CMirror;

/**
 * Callback type for the functions called when the mirror system is ready.
 *
 * Mirror ready at level 1: the datasets can be initialized (declaring
 * entity types owning, properties, etc.) => callback passed to
 * CMirror::init().
 *
 * Mirror ready at level 2: the row management and properties can be used
 * => when mirrorIsReady() returns true (callbacks passed to
 * CMirror::addCallbackWhenMirrorReadyForUse()).
 *
 * Example of such a function:
 *
 * void cbMirrorIsReady( CMirror *mirror )
 * {
 *    // User code
 * }
 */
typedef void (*TMirrorReadyCallback) (CMirror*);

/**
 * Callback type for notification function
 * Example:
 * void cbProcessMirrorChanges()
 * {
 *    MainDataSet->beginAddedEntities();
 *    (...)
 *    MainDataSet->endAddedEntities();
 *
 *    MainDataSet->beginRemovedEntities();
 *    (...)
 *    MainDataSet->endRemovedEntities();
 *
 *    (...) // optional property changes notifications
 * }
 */
typedef void (*TNotificationCallback) (void);


/**
 * This class allows a service to share, access and modify the values
 * of any property of any entity in a shard.
 *
 * A property belongs to one dataset (see CMirroredDataSet), and is stored
 * and addressed using a TPropertyIndex.
 * An entity added in a dataset is stored and addressed using a TDataSetRow.
 *
 * The mirror system handles the sharing of entities and the sharing of
 * properties between services, using shared memory for services on the
 * same physical machine, and network communication for remote services.
 * It handles the notification (if required) of the service when a value
 * has changed or an entity has been added or removed.
 *
 * Several services are allowed to create distinct entities that have
 * the same type (i.e. that will have the same dataset(s)). In a dataset,
 * each service will have a specific range of rows to avoid conflicts.
 * See declareEntityTypeOwner(), addEntity(), removeEntity().
 *
 * The datasets are loaded at init(). They can be browsed using dsBegin()
 * and dsEnd(), and a specific dataset can be retrieved by name
 * (getDataSet()).
 *
 * The recommended way to use this class is to declare a CMirror object
 * as a member of an instanciated singleton class. However, if you use it
 * by declaring a static CMirror object, you must not forget to call
 * the release() method in your release code.
 *
 * Mirror initialization and readiness:
 * 1. In your service's init(), call CMirror::init() passing the datasets
 * to load and the callback for mirror initialization at level 1.
 * 2. The mirror gets ready at level 1 and calls the callback. In this
 * callback, declare entity types owning, datasets, and so on.
 * 3. The mirror gets ready at level 2. It calls the callbacks passed
 * to addCallbackWhenMirrorReadyForUse() (if any), then executes the
 * commands in StartCommandsWhenMirrorReady in the config file (if any),
 * and mirrorIsReady() returns true.
 * 4. Then you can add/remove entities, check notifications, read/write
 * property values, and so on.
 *
 * Note:
 * The mirror uses IService::setClosureClearanceCallback().
 * Currently you can't use it along with the mirror.
 *
 * \seealso CMirrorredDataSet
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
class CMirror
{
public:

	/// Constructor
	CMirror();

	/**
	 * Initialize. Call it in the init() method of your service.
	 *
	 * In dataSetsToLoad, give the names of the datasets the current service is interested in.
	 *
	 * You should provide a callback that will be called when the mirror system is ready
	 * for dataset initialization (level 1).
	 * Then, in the body of your callback, you can start using the mirror and dataset methods.
	 * Do not use the mirror before the callback has been called!
	 *
	 * The three other callbacks, if provided, will be called respectively when a tick update
	 * (incrementing the game cycle) is received, when a tick sync (value of the first game cycle)
	 * is received, and when the service is requested to exit AND it's ok to remove entities
	 * (mirror-synchronized time). Note: calling CMirror::release() in the tickReleaseFunc is
	 * allowed. Note: the service may still receive some messages, especially tick update, *after*
	 * the tickReleaseFunc has been called.
	 *
	 * Set the tag to a number >0 if the current service is not interested in receiving the entities
	 * (and their properties) of any other services having the same tag. Typically, you can duplicate
	 * a service DS (Dummy Service) in your shard: if every instance of DS has the same tag, it will
	 * not receive the entities created by the other DSes, thus reducing the network traffic between
	 * the machines where the DSes run. Practically, it will work well in the following conditions:
	 * - Service instances sharing a tag are run on a machine where there is no other mirrored service
	 * (including the case with a single service instance per machine).
	 * - One or more service instance(s) sharing the tag are run on *one* machine with other mirrored
	 * services: only one machine can have this case (otherwise the traffic won't be reduced, and entities
	 * added/changed will be communicated between services (run of different machines) sharing the tag).
	 * - If a tagged service modifies a property also modified by another service (with a different tag or
	 * AllTag), the property changed will not always be notified to a service having the same tag. Ex:
	 * A(1) B(AllTag) Then C(1) connects and wants to be notified of a property written by B. If B modifies
	 * the property then A modifies it (on the same entity), C won't be notified of it at connection.
	 * Forbidden value: ExcludedTag.
	 */
	void				init( std::vector<std::string>& dataSetsToLoad,
							  TMirrorReadyCallback cbLevel1,
							  TNotificationCallback tickUpdateFunc,
							  TNotificationCallback tickSyncFunc = NULL,
							  TNotificationCallback tickReleaseFunc = NULL,
							  TMTRTag tag=AllTag );

	/**
	 * Deinitialize. You must call this method in your release code if your CMirror
	 * object is a static object!
	 */
	void				release();

	/**
	 * Set a callback to call when a particular service has its mirror system ready.
	 * The callback behaviour is identical to the one of NLNET::CUnifiedNetwork, except that
	 * your callback will not be called as soon as the service is up, but when it is able
	 * to use the mirror system functionalities (for example, adding entities).
	 *
	 * Call this method in the init() method of your service, otherwise you could miss if
	 * a service gets ready before your callback setting.
	 * Warning: if a service does not use the mirror system at all, no callback will ever
	 * be called for it.
	 *
	 * You can set more than one callback, each one will be called one after one.
	 * If the serviceName is "*", the callback will be call for any services. If you set
	 * the same callback for a specific service S and for "*", the callback might be called twice.
	 * You should always set back to true (it puts the specified callback at the end of the array).
	 *
	 * \param synchronizedCallback Set true if you want the callback to be called in a synchronized
	 * moment to allow you to spawn or despawn entities (see isRunningSynchronizedCode()), instead
	 * of calling it as soon as received.
	 */
	void				setServiceMirrorUpCallback( const std::string &serviceName, NLNET::TUnifiedNetCallback cb, void *arg=0, bool back=true, bool synchronizedCallback=true );

	/**
	 * This is the counterpart of setServiceMirrorUpCallback().
	 * If a service with the specified name is stopped, after its mirror system was ready,
	 * the provided callback(s) will be called.
	 *
	 * If the synchronizedCallback and back flags have the same value in both
	 * setServiceMirrorDownCallback() and setServiceDownCallback(), the callback(s) provided to
	 * setServiceMirrorDownCallback() will be called before the callback(s) provided to
	 * setServiceDownCallback(). In any case, these will still be called.
	 * For callbacks provided to CUnifiedNetwork::setServiceDownCallback(),
	 * the behaviour with a false synchronizedCallback flag are applied.
	 */
	void				setServiceMirrorDownCallback( const std::string &serviceName, NLNET::TUnifiedNetCallback cb, void *arg=0, bool back=true, bool synchronizedCallback=true );

	/**
	 * This is similar to CUnifiedNetwork::setServiceUpCallback() but it allows you to spawn or despawn
	 * entities in the callback if you set synchronizedCallback to true (see isRunningSynchronizedCode())
	 * \seealso setServiceMirrorUpCallback()
	 */
	void				setServiceUpCallback( const std::string &serviceName, NLNET::TUnifiedNetCallback cb, void *arg=0, bool back=true, bool synchronizedCallback=true );

	/**
	 * This is similar to CUnifiedNetwork::setServiceUpCallback() but it allows you to spawn or despawn
	 * entities in the callback if you set synchronizedCallback to true (see isRunningSynchronizedCode())
	 * \seealso setServiceMirrorDownCallback()
	 */
	void				setServiceDownCallback( const std::string &serviceName, NLNET::TUnifiedNetCallback cb, void *arg=0, bool back=true, bool synchronizedCallback=true );

	/**
	 * Call this method every cycle (in your update routine) if you use mirror watches
	 */
	void				updateWatches();

	//--- ENTITY ADDITION/REMOVAL --------------------------------------------------------------------------

	/*
	 * Declare the current service as a user of the entities of the specified entity type.
	 * It means the current service wants to be receive the corresponding entities. It will
	 * be notified about their additions and removals.
	 * If more than one service is an owner of a particular entity type, declaring or not
	 * as a user the same entity type will specify if the service wants to receive or not
	 * the entities created by other owners; of course the service is always aware of the
	 * entities it creates, although the mirror system does not notify them to itself.
	 */
	//void				declareEntityTypeUser( uint8 entityTypeId );

	/**
	 * Declare the current service as an owner of the entities of the specified entity type.
	 * It means the current service will create (at most maxNbEntities entities) and remove
	 * this kind of entities (it can remove only the ones it has created) in all the datasets
	 * corresponding to the entity type.
	 * Another service can be a co-owner of the same entity type, but it will create and
	 * remove other entity ids (i.e. in a different range of rows).
	 * Call this method after all declareEntityTypeUser() calls.
	 *
	 * *Important*: after calling this method, wait until mirrorIsReady() (mirror ready
	 * level 2) returns true before adding any entity, otherwise the adding would fail.
	 */
	void				declareEntityTypeOwner( uint8 entityTypeId, sint32 maxNbEntities );

	/**
	 * Provide a callback that will be called the first time mirrorIsReady() returns true.
	 * You can provide several callbacks (ny calling several times this method).
	 *
	 * Note: immediately after having called all the callbacks passed to this method, the
	 * commands founds in StartCommandsWhenMirrorReady (in the config file) will be executed.
	 */
	void				addCallbackWhenMirrorReadyForUse( TMirrorReadyCallback cbLevel2 );

	/**
	 * Add an entity into the mirror. The caller must be an owner of the corresponding
	 * entity type and it must not create more entities than maxNbEntities (see
	 * declareEntityTypeOwner()). The entity will be added into all the datasets
	 * corresponding to the entity type (possibly with a different row in each one).
	 *
	 * The entity will not be declared to all services until you call declareEntity(),
	 * either in CMirror (global) or in CMirroredDataSet (per dataset).
	 *
	 * If fillEntityId is set to true, then the rowindex value for the first dataset is
	 * copied into the first 32 bits of the entityId id part. This is used for automatic
	 * id generation.
	 *
	 * Returns false if part or all of the process failed (for example, if the entity
	 * already exists in a dataset).
	 */
	bool				createEntity( NLMISC::CEntityId& entityId, bool fillEntityId=false );

	/**
	 * Declare the entity (previously added by createEntity()) in all datasets
	 * concerned by its entity type. This method is separated from createEntity()
	 * so that it allows for example to:
	 * - create an entity, set a few properties values, *then* declare it to other services;
	 * - create an entity and prevent from declaring it to other services!
	 *
	 * IMPORTANT: You are allowed to call this method only in code synchronized with the mirror.
	 * Refer to isRunningSynchronizedCode() to know the conditions.
	 *
	 * Returns false in case of failure.
	 *
	 * \seealso CMirroredDataSet::declareEntity() for alternate version.
	 */
	bool				declareEntity( const NLMISC::CEntityId& entityId );

	/**
	 * Create + declare an entity. Previously known as addEntity().
	 *
	 * IMPORTANT: You are allowed to call this method only in code synchronized with the mirror.
	 * Refer to isRunningSynchronizedCode() to know the conditions.
	 *
	 * Returns false if part or all of the process failed (for example, if the entity
	 * already exists in a dataset).
	 */
	bool				createAndDeclareEntity( const NLMISC::CEntityId& entityId ) { return addEntity( false, const_cast<NLMISC::CEntityId&>(entityId), true ); }

	/**
	 * Public for backward compatibility, usage is deprecated. See createAndDeclareEntity().
	 *
	 * IMPORTANT: You are allowed to call this method only in code synchronized with the mirror.
	 * Refer to isRunningSynchronizedCode() to know the conditions.
	 */
	bool				addEntity( bool fillEntityId, NLMISC::CEntityId& entityId, bool declare=true );

	/**
	 * Remove an entity from the mirror. The caller must be an owner of the corresponding
	 * entity type (see declareEntityTypeOwner()) and it must have added the specified
	 * entity before (see createEntity()).
	 *
	 * The entity will be automatically undeclared if it has been declared (see declareEntity())
	 *
	 * IMPORTANT: You are allowed to call this method only in code synchronized with the mirror.
	 * Refer to isRunningSynchronizedCode() to know the conditions.
	 *
	 * Returns false if part or all of the process failed.
	 */
	bool				removeEntity( const NLMISC::CEntityId& entityId );

	/**
	 * Provide a callback in which you will process the notifications of entities and property
	 * changes (see notification methods in CMirroredDataSet). You should not process the
	 * notifications outside this callback! The callback will be called only when mirrorIsReady().
	 *
	 * IMPORTANT: The notifications of entity additions must be browsed before the notifications
	 * of entity removals.
	 */
	void				setNotificationCallback( TNotificationCallback cb ) { _NotificationCallback = cb; }


	//--- MIRROR STATUS ------------------------------------------------------------------------------------

	/**
	 * Return true if the mirror system and all requested properties are ready
	 * for use (level 2) (the level 1 means "ready for dataset initialization").
	 */
	bool				mirrorIsReady() const { return _MirrorAllReady; }

	/// Return true if the specified service is known as "mirror ready"
	bool				serviceHasMirrorReady( NLNET::TServiceId serviceId ) const;

	/// Return the serviceId of the local Mirror Service
	NLNET::TServiceId	localMSId() const { return _PropAllocator.mirrorServiceId(); }

	/// Info: display the properties allocated on the mirror
	void				displayProperties( NLMISC::CLog& log=*NLMISC::InfoLog ) const;

	/// Info: display the number of active entities, created by the current mirror
	void				displayEntityTypesOwned( NLMISC::CLog& log=*NLMISC::InfoLog ) const;

	/// Info: display the contents of the rows corresponding to the entity id
	void				displayRows( const NLMISC::CEntityId& entityId, NLMISC::CLog& log=*NLMISC::InfoLog ) const;

	/// Debug: change a value from a string
	void				changeValue( const NLMISC::CEntityId& entityId, const std::string& propName, const std::string& valueStr );

	/// Scan the entities to find if some are unknown by this service, and add them if requested (debug feature)
	void				rescanExistingEntities( CMirroredDataSet& dataset, NLMISC::CLog& log=*NLMISC::InfoLog, bool addUnknown=false );

	/**
	 * Return true when we are running code synchronized with the mirror service.
	 * It will be always true when called:
	 * - in a callback provided to addCallbackWhenMirrorReadyForUse().
	 * - in a callback provided to setServiceMirrorUpCallback(), setServiceUpCallback() or setServiceDownCallback()
	 *   if their parameter synchronizedCallback is set to true (default).
	 * - in a command when it is launched by "StartCommandsWhenMirrorReady".
	 * - in the tick update callback, if mirrorIsReady().
	 * - in the mirror notification callback.
	 * - in a callback of a message sent using sendMessageViaMirror().
	 * - in a callback of a CMirrorTransportClass object.
	 * - in the callback tickReleaseFunc provided to init(), called before quitting.
	 * It will be false in any other case.
	 */
	bool				isRunningSynchronizedCode() const { return _IsExecutingSynchronizedCode; }

	/// Return the Mirror Traffic Reduction tag
	TMTRTag				tag() const { return _MTRTag; }

	//--- DATASETS ACCESSORS -------------------------------------------------------------------------------

	/// Access the map of datasets (begin iteration). Use GET_NDATASET(it) to access the CMirroredDataSet object
	TNDataSets::iterator dsBegin() { return _NDataSets.begin(); }

	/// Access the map of datasets (test end of iteration)
	TNDataSets::iterator dsEnd() { return _NDataSets.end(); }

	/// Get a dataset by name. If not found, throws EMirror() (does not create a new dataset).
	CMirroredDataSet&	getDataSet( const std::string& dataSetName ) { return _NDataSets[dataSetName]; }

	/// Check if a dataset exist
	bool				isDataSetExist(const std::string &dataSetName) { return _NDataSets.find(dataSetName) != _NDataSets.end(); }

	/// Return the number of dataSet availables
	uint32				getDataSetCount() const	{ return (uint32)_NDataSets.size(); }

	/// Destructor
	~CMirror();

	typedef uint8 TServiceEventType;

protected:

	/// Set the local MS id if the service reported by cbServiceDown("MS") or cbServiceUp("MS") is the local Mirror Service
	bool				detectLocalMS( NLNET::TServiceId serviceId, bool upOrDown );

	/// Return true if the mirror service is up and the entity ranges are ready
	bool				mirrorIsUp() const { return mirrorServiceIsUp() && (_PendingEntityTypesRanges == 0); }

	/// Return true if the specified property is ready
	bool				propIsAllocated( const std::string& propName ) const { return _PropAllocator.getPropertySegment( propName ) != NULL; }

	/// Receiving green light for level1
	void				initAfterMirrorUp();

	/// Init mirror, level1
	void				doInitMirrorLevel1();

	/// Set _MirrorAllReady (level2) to true if the conditions are met
	void				testMirrorReadyStatus();

	/// Set the segment pointer in the property container
	void				setPropertyInfo( std::string& propName, void *segmentPt, uint32 dataTypeSize );

	/// Access segments for non-subscribed properties allocated on the local MS (useful for cleaning a whole row when creating an entity)
	void				applyListOfOtherProperties( NLNET::CMessage& msgin, uint nbProps, const char *msgName, bool isInitial );

	/// Return the dataset corresponding to a property (or NULL if not found)
	CMirroredDataSet	*getDataSetByPropName( std::string& propName, TPropertyIndex& propIndex ) { return _PropAllocator._PropertiesInMirror.getDataSetByPropName( propName, propIndex ); }

	/// Return true if the local MS was found
	bool				mirrorServiceIsUp() const { return (localMSId() != NLNET::TServiceId(~0)); }

	/// Return the inited state (ready at level 1)
	bool				mirrorInited() const { return _MirrorGotReadyLevel1; }

	/// Receive the broadcasting of a service that has its mirror system ready
	void				receiveServiceHasMirrorReady( const std::string& serviceName, NLNET::TServiceId serviceId, NLNET::CMessage& msgin );

	/// Return a property value as string
	void				getPropValueStr ( const NLMISC::CEntityId& entityId, const std::string& propName, std::string& result ) const;

	/// Send to a reconnecting MS the state of our mirror info
	void				resyncMirrorInfo();

	/// Main mirror update
	void				updateMirrorAndReceiveMessages( NLNET::CMessage& msgin );

	/// Return true if the dataset is found in the entity types owned
	bool				datasetMatchesEntityTypesOwned( const CMirroredDataSet& dataset ) const;

	/// Execute user callback when receiving first game cycle
	void				userSyncCallback();

	/// Rescan
	void				scanAndResyncEntitiesExceptIgnored( const std::vector<NLNET::TServiceId8>& creatorIdsToIgnore );

	/// Execute start callbacks & commands
	void				executeMirrorReady2CallbacksAndStartCommands();

	/// Execute commands when mirror is released
	void				executeMirrorReleaseCommands();

	/// Tick update
	void				onTick();

	/// Closure clearance callback
	bool				requestClosure();

	/// Set service mirror/up/down callback
	void				setServiceMUDCallback( const std::string &serviceName, NLNET::TUnifiedNetCallback cb, void *arg, bool back, bool synchronizedCallback, TServiceEventType et );

	/// React to a service mirror/up/down event
	void				processServiceEvent( const std::string &serviceName, NLNET::TServiceId serviceId, TServiceEventType et );

	void				pushEntityRanges( NLNET::CMessage& msgout );
	void				receiveSMIdToAccessPropertySegment( NLNET::CMessage& msgin );
	void				receiveRangesForEntityType( NLNET::CMessage& msgin );
	void				receiveTracker( bool entitiesOrProp, NLNET::CMessage& msgin );
	void				releaseTrackers( NLNET::CMessage& msgin );
	void				receiveAcknowledgeAddEntityTracker( NLNET::CMessage& msgin, NLNET::TServiceId serviceId );
	void				receiveAcknowledgeAddPropTracker( NLNET::CMessage& msgin, NLNET::TServiceId serviceId );
	void				deleteTracker( CChangeTrackerClient& tracker, std::vector<CChangeTrackerClient>& vect );
	friend void			cbMSUpDn( const std::string& serviceName, NLNET::TServiceId serviceId, void *upOrDn );
	friend void			cbAnyServiceUpDn( const std::string& serviceName, NLNET::TServiceId serviceId, void *vEventType );
	friend void			cbRecvSMIdToAccessPropertySegment( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void			cbRecvRangesForEntityType( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void			cbRecvAddPropTracker( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void			cbRecvAddEntityTracker( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void			cbRecvAcknowledgeAddEntityTracker( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void			cbRecvAcknowledgeAddPropTracker( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void			cbReleaseTrackers( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void			cbAllMirrorsOnline( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void			cbGrantStartService( NLNET::CMessage &msgin, const std::string&, NLNET::TServiceId );
	friend void			cbRecvServiceHasMirrorReadyBroadcast( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void			cbRecvServiceHasMirrorReadyReply( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void			cbUpdateMirrorAndReceiveMessages( NLNET::CMessage &msgin, const std::string&, NLNET::TServiceId );
	friend void			cbSyncGameCycle();
	friend void			cbServiceMirrorUpForSMIRUB( const std::string& serviceName, NLNET::TServiceId serviceId, void * );
	friend void			cbscanAndResyncEntitiesExceptIgnored( NLNET::CMessage &msgin, const std::string&, NLNET::TServiceId );
	friend void			cbApplyListOfOtherProperties( NLNET::CMessage &msgin, const std::string&, NLNET::TServiceId );
	friend void			cbAddListOfOtherProperties( NLNET::CMessage &msgin, const std::string&, NLNET::TServiceId );
	friend void			cbTickUpdateFunc();
	friend bool			cbRequestClosure();


private:

	/// Type of callback and its user data
	struct TCallbackArgItemM
	{
		NLNET::TUnifiedNetCallback	Cb;
		void*						Arg;
		bool						Synchronized;
		TServiceEventType			EventType;
	};

	/// Instanciated callback item
	struct TCallbackArgItemMExt : public TCallbackArgItemM
	{
		NLNET::TServiceId			ServiceId;
		std::string					ServiceName;

		TCallbackArgItemMExt( const TCallbackArgItemM& src, const std::string& servName, NLNET::TServiceId servId )
		{
			Cb = src.Cb;
			Arg = src.Arg;
			Synchronized = src.Synchronized;
			EventType = src.EventType;
			ServiceId = servId;
			ServiceName = servName;
		}
	};

	/// Type of map of service up callbacks with their user data.
	typedef std::map< std::string, std::list<TCallbackArgItemM> >	TNameMappedCallbackM;

	/// Type of set of known 'service mirror up' (service ids)
	typedef std::set< NLNET::TServiceId >	TNotifiedServices;

	/// Shared memory allocator
	CPropertyAllocatorClient	_PropAllocator;

	/// Map of entity types and ranges owned
	TEntityTypesOwned			_EntityTypesOwned;

	/// Number of entity types declared to the MS with no answer yet received
	sint						_PendingEntityTypesRanges;

	/// The datasets with keys as strings (names)
	TNDataSets					_NDataSets;

	/// Map of datasets with keys as CSheetId
	TSDataSets					_SDataSets;

	/// Map of datasets sheets
	TSDataSetSheets				_SDataSetSheets;

	/// Names of datasets to load
	std::vector<std::string>	_DataSetsToLoad;

	/// "Mirror ready at level 1" callback
	TMirrorReadyCallback		_ReadyL1Callback;

	/// "Mirror ready at level 2" callbacks
	std::vector<TMirrorReadyCallback> _ReadyL2Callbacks;

	/// Map of 'service mirror up' callbacks (by service name)
	TNameMappedCallbackM		_ServiceUpDnCallbacks;

	/// 'Service mirror up' callbacks for *
	std::vector<TCallbackArgItemM>		_ServiceUpDnUniCallbacks;

	// All defered callbacks for synchronized mode
	std::vector<TCallbackArgItemMExt>	_SynchronizedServiceUpDnCallbackQueue;

	/// Set of services that are known 'mirror ready'
	TNotifiedServices			_ServicesMirrorUpNotified;

	/// Tick update callback
	TNotificationCallback		_TickUpdateFunc;

	/// Notification of mirror changes callback
	TNotificationCallback		_NotificationCallback;

	/// User sync callback (called after the mirror one)
	TNotificationCallback		_UserSyncCallback;

	/// Tick release callback
	TNotificationCallback		_TickReleaseFunc;

	/// General status
	bool						_MirrorAllReady;

	/// Status of mirror service detection and range mirror manager online state
	bool						_MirrorGotReadyLevel1;

	/// Status of mirror start commands (to call them only once when mirror gets ready at level 2)
	bool						_MirrorGotReadyLevel2;

	/// Status of expectation of the list of non-subscribed properties allocated on the local MS
	bool						_ListOfOtherPropertiesReceived;

	/// Flag set between ReadyLevel2 and MirrorAllReady, to detect if the local MS is back (for resyncing)
	bool						_AwaitingAllMirrorsOnline;

	/// True if we are in code synchronized with the mirror service
	bool						_IsExecutingSynchronizedCode;

	/// True if the service wants to quit
	bool						_ClosureRequested;

	/// Mirror Traffic Reduction Tag (corresponding to the tags of the 'self' trackers)
	TMTRTag						_MTRTag;

public:

	/// A particular entity to use in mirror commands when no argument provided
	NLMISC::CEntityId			MonitoredEntity;
};


const CMirror::TServiceEventType ETMirrorUp=0, ETServiceUp=1, ETServiceDn=2, ETMirrorDn=3;


/*
 * Tick update
 */
inline void CMirror::onTick()
{
	_IsExecutingSynchronizedCode = true;
	_TickUpdateFunc();
	_IsExecutingSynchronizedCode = false;
}

#endif // NL_MIRROR_H

/* End of mirror.h */
