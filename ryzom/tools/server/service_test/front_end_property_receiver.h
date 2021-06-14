/** \file front_end_property_receiver.h
 * Container for manage property received by front end
 *
 * $Id: front_end_property_receiver.h,v 1.17 2004/03/01 19:22:19 lecroart Exp $
 */



#ifndef RY_FRONT_END_PROPERTY_RECEIVER_H
#define RY_FRONT_END_PROPERTY_RECEIVER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/path.h"
#include "nel/misc/entity_id.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
#include "nel/net/service.h"

#include <list>
#include <set>
#include <vector>
#include <map>
#include <string>

#define NB_TYPE_ENTITIES			3
#define NB_PROPERTIES_PER_ENTITY	16
#define FE_NBMAX_ENTITIES			50000
#define PROPERTY_CODE_ERASE			1<<31
#define PROPERTY_CODE_NEW			1<<30

#define NB_OTHER_PROPERTIES			2

/**
 * Manage delta received of property for front end
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2001
 */
class CFrontEndPropertyReceiver
{
public:
	typedef uint32 TPropertiesBitfield;
	typedef uint32 TPropertiesIndex;
	typedef uint32 TPropertiesCode;
	typedef uint32 TPropertiesContMask;
	typedef uint64 TPropertiesValue;
	typedef uint32 TPropertiesValueIndex;
	typedef uint16 TVisionSlot;
	typedef std::map< NLMISC::CEntityId, uint32 > TMapIdToIndex;

	struct SPropIndexCode
	{
		TPropertiesIndex	index;
		TPropertiesCode		code;
	};

	struct SEntity
	{
		NLMISC::CEntityId				id;						// Entity Id
		uint32							SheetId;					// Id fiche of entity
		TPropertiesCode					code;						// can be an "union" with bitfield and code => 3*32 bits -> 32 bits
		TPropertiesBitfield				bitfield;					// can be an "union" with bitfield and code => 3*32 bits -> 32 bits
		TPropertiesIndex				nextFreeEntity;				// can be an "union" with bitfield and code => 3*32 bits -> 32 bits
		TPropertiesIndex				nextUpdatedEntityProperties;
		TPropertiesIndex				nextUpdatedEntityVision;
		TPropertiesContMask				mask;						
		TPropertiesValue				properties[NB_PROPERTIES_PER_ENTITY];
		NLMISC::TGameCycle				TickPosition;
		std::map< TPropertiesIndex, TVisionSlot > VisionIn;
		std::set< TPropertiesIndex > VisionOut;

		/**
		 * Return GameCycle of property update
		 * \param PropertyIdx is property index
		 * \return Game cycle last update for property index
		 */
		NLMISC::TGameCycle getGameCycleForProperty( uint32 PropertyIdx ) { return TickPosition; }
	};

	struct SPropertyDesc
	{
		bool		Continuous;		// true if property is continuous type (FE not informed of changes)
		bool		Union;			// true if property is with another in 64 bits property
		std::string	PropertyName;	// name of property
		std::string	ServiceOwner;	// name of owner service of property
		uint8		UpdateFrequency;// Frequency of subcribtion updating (in number of ticks)
		uint8		Index;			// index of property (in array of property)
		uint8		NbBits;			// number of bits used by property
		uint8		UnionShift;		// bit shifting for union in 64 bits property destination
		uint64		UnionMask;		// mask for and/or operations for write property in 64 bits destination
		uint8		PropertyType;	// Type of property
	};

	struct SOtherPropertyDesc
	{
		std::string	PropertyName;	// name of property
		std::string	ServiceOwner;	// name of owner service of property
		uint8		NbBits;			// number of bits used by property
		uint8		UpdateFrequency;// Frequency of subcribtion updating (in number of ticks)
	};

	struct SIndexProperties
	{
		std::string		PropertyName;
		uint8			PropertyIndex;
	};

	enum EPropertyIndex { x = 0, y, z, theta, mode, behaviour, nameStringId };
	enum EntityTypeIndex { player = 0, unknown = 127 };

//	static SIndexProperties PropertyIndexByType [ NB_TYPE_ENTITIES ] [ NB_PROPERTIES_PER_ENTITY ];
	static SPropertyDesc PropertiesProperty [ NB_TYPE_ENTITIES ] [ NB_PROPERTIES_PER_ENTITY ];
	static SOtherPropertyDesc OtherProperties [ NB_OTHER_PROPERTIES ];
	static uint32 PropertiesContinuousMask [ NB_TYPE_ENTITIES ];

	/*
	 * Init of property receiver
	 *\param filename if georges sheet describe properties informations
	 */
	static void initFrontEndPropertyReceiver( const std::string& filenameProperties, const std::string& filenameEntityIndex );

	/*
	 * Init entity index map with sheet
	 *\param pElt is a pointer on current element item of sheet
	 *\param EntityIndexMap is map association name of element ans it's index
	 */
	static void initEntityIndex( NLGEORGES::UFormElm *pElt, std::map< std::string, uint8 >& EntityIndexMap );

	/*
	 * Init properties descriptor
	 *\Param pElt is a pointer on current element item of sheet
	 *\param EntityIndexMap is map association name of element ans it's index
	 *\param entityIndex is current index of entity
	 *\param ident is deep in tree structure
	 *\param itemNumber is idex/number of item in current deep tree structure
	 */
	static void initPropertyDescriptor( NLGEORGES::UFormElm *pElt, std::map< std::string, uint8 >& EntityIndexMap, uint32 ident = 0, uint32 itemNumber = 0, bool OtherProperty = false );

	/// Init subscription for service up
	static void initFrontEndPropertySubscription( const std::string& serviceName );

	/// Remove entities in receiver when downing serviceID is owner
	static void serviceDown( uint16 serviceId );

	/// release data
	static void freeFrontEndPropertyReceiver( void );

	/// Entity management : called for deleting entities
	static SEntity* getEntity( const TPropertiesIndex indexprop );

	/// Property management : Set a new property
	static bool setProperties( const NLMISC::CEntityId& id, uint32 indexvalue, TPropertiesValue& value );

	/// Property management : Get the first modified properties since the last EndUpdatedProperties
	static TPropertiesIndex getFirstUpdatedProperties();

	/// Property management : Get the next modified properties
	static TPropertiesIndex getNextUpdatedProperties( TPropertiesIndex indexprop );

	/// Property management : Call it after getting all the updated properties (eg, the.GetNextUpdatedProperties function return -1 )
	static void endUpdatedProperties();

	/// Vision management : Set a new delta of vision
	static bool setVision( const NLMISC::CEntityId& id, const std::map< NLMISC::CEntityId, TVisionSlot >& visionIn, const std::vector< NLMISC::CEntityId >& visionOut );

	/// Vision management : Get the first modified vision since the last EndUpdatedVision
	static TPropertiesIndex getFirstUpdatedVision();

	/// Vision management : Get the next modified vision
	static TPropertiesIndex getNextUpdatedVision( TPropertiesIndex indexprop );

	/// Vision management : Call it after getting all the updated vision (eg, the.GetNextUpdatedVision function return -1 )
	static void endUpdatedVision();

	/// UpdateProperties, unserial update message and update corresponding properties
	static void updateProperties( NLNET::CMessage& msgin );

	/// RemoveEntity, unserial remove message and update corresponding Entity / properties
	static void removeEntity( NLNET::CMessage& msgin );

	/// UpdateProperties, unserial update message and update corresponding properties
	static void updateVision( NLNET::CMessage& msgin );

	/// Ask subscribe for one property
	static void askPropertySubscribe( const std::string& ServiceSubscribe, const std::pair< std::string, uint32>& Property );

	/// Ask subscribe for list of properties
	static void askPropertiesSubscribe( const std::string& ServiceSubscribe, const std::list< std::pair< std::string, uint32 > >& Properties );

private:
	static TPropertiesIndex			_FirstFreeEntity;
	static TPropertiesIndex			_FirstUpdatedEntityProperties;
	static TPropertiesIndex			_FirstUpdatedEntityVision;
	static std::vector< SEntity >	_VectorEntities;
	static TMapIdToIndex			_MapIdToIndex;

	struct SPlayerVisionDelta
	{
		NLMISC::CEntityId							Id;
		std::vector< NLMISC::CEntityId >			EntityOut;
		std::map< NLMISC::CEntityId, TVisionSlot >	EntityIn;

		void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			f.serial( Id );
			f.serialCont( EntityOut );
			f.serialCont( EntityIn );
		}
	};

	// CEntityId / index management : Create a new index for the CEntityId
	static TPropertiesIndex assignIndexToId( const NLMISC::CEntityId& Id );

	// CEntityId / index management : Get index, return true if finded and the value is setting. If false, value is undefined
	static TPropertiesIndex findIndex( const NLMISC::CEntityId& id );

	// CEntityId / index management : Release index / id association
	static void releaseIndexToId( const NLMISC::CEntityId& id );

	// Entity management : delete entity
	static bool deleteEntity( const NLMISC::CEntityId& id );

	// Entity management : called for deleting entities
	static void setDeleted( TPropertiesIndex indexprop );

	// init property with george's sheet
	static void initPropertyDesc( NLGEORGES::UFormElm *pEltProperty, uint32 entity, uint32 property );

	// Init entity other properties with george's sheet
	static void initOtherPropertyDesc( NLGEORGES::UFormElm *pEltProperty );

};

// Callback for delta mirror update received
void cbDeltaUpdate( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId );

// Callback for delta vision update remove received
void cbDeltaUpdateRemove( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId );

// Callback for delta vision update received
void cbDeltaVision( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId );

#endif // RY_FRONT_END_PROPERTY_RECEIVER_H

/* End of front_end_property_receiver.h */
