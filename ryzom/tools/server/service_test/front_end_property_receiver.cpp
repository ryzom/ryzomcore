/** \file front_end_property_receiver.cpp
 * Container for manage property receivers for front end
 *
 * $Id: front_end_property_receiver.cpp,v 1.21 2004/03/01 19:22:19 lecroart Exp $
 */



#include "front_end_property_receiver.h"

#include "nel/misc/path.h"

using namespace std;
using namespace NLNET;
using namespace NLMISC;
using namespace NLGEORGES;

// Static members of CFrontEndPropertyReceiver
CFrontEndPropertyReceiver::TPropertiesIndex		CFrontEndPropertyReceiver::_FirstFreeEntity;
CFrontEndPropertyReceiver::TPropertiesIndex		CFrontEndPropertyReceiver::_FirstUpdatedEntityProperties;
CFrontEndPropertyReceiver::TPropertiesIndex		CFrontEndPropertyReceiver::_FirstUpdatedEntityVision;
vector< CFrontEndPropertyReceiver::SEntity >	CFrontEndPropertyReceiver::_VectorEntities;
CFrontEndPropertyReceiver::TMapIdToIndex		CFrontEndPropertyReceiver::_MapIdToIndex;
//CFrontEndPropertyReceiver::SIndexProperties		CFrontEndPropertyReceiver::PropertyIndexByType [ NB_TYPE_ENTITIES ] [ NB_PROPERTIES_PER_ENTITY ];
CFrontEndPropertyReceiver::SPropertyDesc		CFrontEndPropertyReceiver::PropertiesProperty [ NB_TYPE_ENTITIES ] [ NB_PROPERTIES_PER_ENTITY ];
CFrontEndPropertyReceiver::SOtherPropertyDesc	CFrontEndPropertyReceiver::OtherProperties [ NB_OTHER_PROPERTIES ];
uint32 CFrontEndPropertyReceiver::PropertiesContinuousMask [ NB_TYPE_ENTITIES ];


//---------------------------------------------------
// Init
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::initFrontEndPropertyReceiver( const string& filenameProperties, const string& filenameEntityIndex )
{
	// reserve place for vector....
	_VectorEntities.resize( FE_NBMAX_ENTITIES );			// not reserve! 
	
	// prepare FreeEntityProperties linking;
	unsigned int i = 0;
	for( std::vector< SEntity >::iterator it = _VectorEntities.begin();	it != _VectorEntities.end(); ++it )
	{
		it->code = 0;
		it->bitfield = 0;
		it->mask = 0;
		it->nextFreeEntity = ++i;
		it->nextUpdatedEntityProperties = -2;
		it->nextUpdatedEntityVision = -2;
	}
/*
	for( i = 0; i < NB_TYPE_ENTITIES; ++i )
	{
		for( int j = 0; j < NB_PROPERTIES_PER_ENTITY; ++j )
		{
			PropertyIndexByType [ i ] [ j ].PropertyName = "";
			PropertyIndexByType [ i ] [ j ].PropertyIndex = 127;
		}
	}
*/
	// prepare FreeEntityProperties linking;
	_FirstFreeEntity = 0;
	// prepare UpdatedEntityProperties linking;
	_FirstUpdatedEntityProperties = -1;
	// prepare UpdatedEntityProperties linking;
	_FirstUpdatedEntityVision = -1;

	// Load sheet for properties's property
	CLoader loader;
	UFormElm item;

	item.SetLoader( &loader );

	try
	{
		item.Load( CPath::lookup( filenameEntityIndex ) );
	}
	catch (Exception &e){ nlwarning("CFrontEndPropertyReceiver::initFrontEndPropertyReceiver : '%s' -> '%s'.", filenameEntityIndex.c_str(), e.what());}

	map< string, uint8 > EntityIndexMap;

	UFormElm *pRoot = item.GetElt(0);
	initEntityIndex( pRoot, EntityIndexMap );
	
	item.Clear();

	try
	{
		item.Load( CPath::lookup( filenameProperties ) );
	}
	catch (Exception &e){ nlwarning("CFrontEndPropertyReceiver::initFrontEndPropertyReceiver : '%s' -> '%s'.", filenameProperties.c_str(), e.what());}

	pRoot = item.GetElt(0);
	initPropertyDescriptor( pRoot, EntityIndexMap );

	// Precalculate the the continuous mask 
	for( int ii = 0; ii < NB_TYPE_ENTITIES; ++ii )
	{
		uint32 continuousMask = 0;
		
		for( uint32 j = 0; j < NB_PROPERTIES_PER_ENTITY; ++j )
		{
			for( int k = 0; k < NB_PROPERTIES_PER_ENTITY; ++k )
			{
				if( PropertiesProperty [ ii ] [ k ].Index == j )
				{
					uint32 idx = 1;
					idx <<= PropertiesProperty [ ii ] [ k ].Index;
					continuousMask |= idx  & ( ( PropertiesProperty [ ii ] [ k ].Continuous) ? 0 : 1 );
				}
			}
		}
		// Changed 4/04/2002: i to ii
		PropertiesContinuousMask [ ii ] = continuousMask;
	}

	// register callback function for process delta update messages of mirror and vision
	NLNET::TUnifiedCallbackItem _cbArray[3];

	_cbArray[0].Callback = cbDeltaUpdate;
	_cbArray[0].Key = "DELTA_UPDATE";
	_cbArray[1].Callback = cbDeltaUpdateRemove;
	_cbArray[1].Key = "DELTA_UPDATE_REMOVE";
	_cbArray[2].Callback = cbDeltaVision;
	_cbArray[2].Key = "VISIONS_DELTA";

	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, 3 );
}

//---------------------------------------------------
// Init entity index map with sheet
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::initEntityIndex( UFormElm *pElt, map< string, uint8 >& EntityIndexMap )
{
	uint32 i;

	CItemEltAtom *pAtomElt = dynamic_cast<CItemEltAtom*>(pElt);
	if (pAtomElt != NULL)
	{
		EntityIndexMap.insert( make_pair( pAtomElt->GetName(), (uint8) atoi( pAtomElt->GetCurrentResult().c_str() ) ) );
		return;
	}

	// If the element is a structure
	CItemEltStruct *pStructElt = dynamic_cast<CItemEltStruct*>(pElt);
	if (pStructElt != NULL)
	{
		for (i = 0; i < pStructElt->GetNbStructElt(); ++i)
			initEntityIndex(pStructElt->GetStructElt(i), EntityIndexMap );
		return;
	}
}

//---------------------------------------------------
// Init properties descriptor
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::initPropertyDescriptor( UFormElm *pElt, map< string, uint8 >& EntityIndexMap, uint32 ident, uint32 itemNumber, bool OtherProperty )
{
	uint32 i;
	static uint32 entityIndex;

	CItemEltAtom *pAtomElt = dynamic_cast<CItemEltAtom*>(pElt);
	if (pAtomElt != NULL)
	{
		nldebug("FEPROPINIT:					Atom element %s, value %s", pAtomElt->GetName().c_str(), pAtomElt->GetCurrentResult().c_str() );
		
		if( ident == 2 )
		{
			entityIndex = atoi( pAtomElt->GetCurrentResult().c_str() );
		}
		else
		{
			if( !OtherProperty )
			{
				initPropertyDesc( pAtomElt, entityIndex, itemNumber );
			}
		}
		return;
	}

	// If the element is a structure
	CItemEltStruct *pStructElt = dynamic_cast<CItemEltStruct*>(pElt);
	if (pStructElt != NULL)
	{
		if( ident == 0 )
		{
			nldebug("FEPROPINIT: Struct element %s", pStructElt->GetName().c_str() );
		}
		else if( ident == 1 )
		{
			nldebug("FEPROPINIT:	Struct element %s", pStructElt->GetName().c_str() );
		}
		else if( ident == 2 )
		{
			nldebug("FEPROPINIT:		Struct element %s", pStructElt->GetName().c_str() );
		}
		else if( ident == 3 )
		{
			nldebug("FEPROPINIT:			Struct element %s", pStructElt->GetName().c_str() );
		}
		else if( ident == 4 )
		{
			nldebug("FEPROPINIT:				Struct element %s", pStructElt->GetName().c_str() );
		}
		if( ident == 3 )
		{
			UFormElm *pEltChild = pStructElt->GetElt("Index");
			if( pEltChild != 0 )
			{
				sint32 indexTest = atoi( pEltChild->GetCurrentResult().c_str() );
				if( atoi( pEltChild->GetCurrentResult().c_str() ) < 0)
				{
					OtherProperty = true;
					initOtherPropertyDesc( pStructElt );
				}
				else
				{
					OtherProperty = false;
				}
			}
		}
		for (i = 0; i < pStructElt->GetNbStructElt(); ++i)
		{
			initPropertyDescriptor( pStructElt->GetStructElt(i), EntityIndexMap/*, entityIndex*/, ident+1, itemNumber, OtherProperty );
			if( ident == 2 )
			{
				itemNumber++;
			};
		}
		if( ident == 3 )
		{
			if( PropertiesProperty [ entityIndex ] [ itemNumber ].Union == true )
			{
				uint64 unionMask = 1;

				unionMask<<=PropertiesProperty [ entityIndex ] [ itemNumber ].NbBits;		// setup bit NbBits to 1 (ie the one above desired bitmask) followed by all 0s
				unionMask-=1;																// setup the bitmask (bottm NbBits of unionMask are all 1 rest are 0)
				unionMask<<=PropertiesProperty [ entityIndex ] [ itemNumber ].UnionShift;	// shift up the bitmask to the correct location in the QuadWord

				PropertiesProperty [ entityIndex ] [ itemNumber ].UnionMask = unionMask;
			}
		}
		return;
	}
}

//---------------------------------------------------
// Init subscription for service
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::initFrontEndPropertySubscription( const string& serviceName )
{
	nlinfo("<CFrontEndPropertyReceiver::initFrontEndPropertySubscription> service name %s",serviceName.c_str() );

	// subscribe to delta update for properties
	pair< string, uint32 > Property;

	for( int ii = 0; ii < NB_TYPE_ENTITIES; ++ii )
	{
		for( uint32 j = 0; j < NB_PROPERTIES_PER_ENTITY; ++j )
		{
			if( PropertiesProperty [ii] [j].ServiceOwner == serviceName )
			{
				Property = make_pair( PropertiesProperty [ii] [j].PropertyName, 1 << PropertiesProperty [ii] [j].UpdateFrequency );
				CFrontEndPropertyReceiver::askPropertySubscribe( serviceName, Property );
			}
		}
	}

	// subscribe to delta update for other properties (like Sheet Id )
	for( int i = 0; i < NB_OTHER_PROPERTIES; ++i )
	{
		if( OtherProperties [i].ServiceOwner == serviceName )
		{
			Property = make_pair( OtherProperties [i].PropertyName, 1 << OtherProperties [i].UpdateFrequency );
			CFrontEndPropertyReceiver::askPropertySubscribe( serviceName, Property );
		}
	}
}

//---------------------------------------------------
// Remove entities in receiver when downing serviceID is owner
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::serviceDown( uint16 serviceId )
{
	for( TMapIdToIndex::iterator it = _MapIdToIndex.begin(); it != _MapIdToIndex.end(); ++it )
	{
		if( (*it).first.DynamicId == serviceId || (*it).first.CreatorId == serviceId )
		{
			deleteEntity( (*it).first );
		}
	}
}

//---------------------------------------------------
// release
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::freeFrontEndPropertyReceiver( void )
{
	endUpdatedProperties(); 
	endUpdatedVision();
}

//---------------------------------------------------
// Init property with george's sheet
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::initPropertyDesc( UFormElm *pElt, uint32 entity, uint32 property )
{
	if( pElt->GetName() == string("Index") )
	{
		PropertiesProperty [ entity ] [ property ].Index = (uint8) atoi( pElt->GetCurrentResult().c_str() );
	}
	else if( pElt->GetName() == string("NbBits") )
	{
		PropertiesProperty [ entity ] [ property ].NbBits = (uint8) atoi( pElt->GetCurrentResult().c_str() );
	}
	else if(pElt->GetName() == string("Continuous") )
	{
		PropertiesProperty [ entity ] [ property ].Continuous = (pElt->GetCurrentResult() == "true") ? true : false;
	}
	else if( pElt->GetName() == string("Union") )
	{
		PropertiesProperty [ entity ] [ property ].Union = (pElt->GetCurrentResult() == "true") ? true : false;
	}
	else if( pElt->GetName() == string("Shift") )
	{
		PropertiesProperty [ entity ] [ property ].UnionShift = (uint8) atoi( pElt->GetCurrentResult().c_str() );
	}
	else if( pElt->GetName() == string("PropertyName") )
	{
		PropertiesProperty [ entity ] [ property ].PropertyName = pElt->GetCurrentResult();
	}
	else if( pElt->GetName() == string("ServiceOwner") )
	{
		PropertiesProperty [ entity ] [ property ].ServiceOwner = pElt->GetCurrentResult();
	}
	else if( pElt->GetName() == string("UpdateFrequency") )
	{
		PropertiesProperty [ entity ] [ property ].UpdateFrequency = (uint8) atoi( pElt->GetCurrentResult().c_str() );
	}
	else if( pElt->GetName() == string("PropertyType") )
	{
		PropertiesProperty [ entity ] [ property ].PropertyType = (uint8) atoi( pElt->GetCurrentResult().c_str() );
	}
	else
	{
		nlwarning("CFrontEndPropertyReceiver::initPropertyDesc property %s is unknown for entity %d property %d", pElt->GetName().c_str(), entity, property );
	}
}

//---------------------------------------------------
// Init entity other properties with george's sheet
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::initOtherPropertyDesc( UFormElm *pEltProperty )
{
	uint32 Index = 0;
	
	UFormElm *pElt = pEltProperty->GetElt("Index");
	if( pElt != 0 )
	{
		Index = 255 - (uint8) atoi( pElt->GetCurrentResult().c_str() );
	}
	else
	{
		nlwarning("CFrontEndPropertyReceiver::initOtherPropertyDesc property Index not in properties's property sheet" );
	}

	pElt = pEltProperty->GetElt("NbBits");
	if( pElt != 0 )
	{
		OtherProperties[ Index ].NbBits = (uint8) atoi( pElt->GetCurrentResult().c_str() );
	}
	else
	{
		nlwarning("CFrontEndPropertyReceiver::initOtherPropertyDesc property NbBits not in properties's property sheet" );
	}

	pElt = pEltProperty->GetElt("PropertyName");
	if( pElt != 0 )
	{
		OtherProperties[ Index ].PropertyName = pElt->GetCurrentResult();
	}
	else
	{
		nlwarning("CFrontEndPropertyReceiver::initOtherPropertyDesc property PropertyName not in properties's property sheet" );
	}

	pElt = pEltProperty->GetElt("ServiceOwner");
	if( pElt != 0 )
	{
		OtherProperties[ Index ].ServiceOwner = pElt->GetCurrentResult();
	}
	else
	{
		nlwarning("CFrontEndPropertyReceiver::initOtherPropertyDesc property ServiceOwner not in properties's property sheet" );
	}
}

//---------------------------------------------------
// Entity management : delete entity
// 
//---------------------------------------------------
bool CFrontEndPropertyReceiver::deleteEntity( const CEntityId& id )
{
	TPropertiesIndex i = findIndex( id );
	// if failed, do nothing
	if( i == -1 )
		return( false );
	// must delete it!
	_VectorEntities[i].code |= PROPERTY_CODE_ERASE;

	// If it's not in the list of updated entity, push it in!
	if( _VectorEntities[i].nextUpdatedEntityProperties == -2 )
	{
		_VectorEntities[i].nextUpdatedEntityProperties = _FirstUpdatedEntityProperties;
		_FirstUpdatedEntityProperties = i;
	}
	return( true );
}

//---------------------------------------------------
// Entity management : called for deleting entities
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::setDeleted( TPropertiesIndex indexprop )
{
	// buid the link of free entries.
	_VectorEntities[indexprop].nextFreeEntity = _FirstFreeEntity;
	_FirstFreeEntity = indexprop;
	releaseIndexToId( _VectorEntities[indexprop].id );
}

//---------------------------------------------------
// Entity management : give pointer on SEntity
// 
//---------------------------------------------------
CFrontEndPropertyReceiver::SEntity* CFrontEndPropertyReceiver::getEntity( const TPropertiesIndex indexprop ) 
{
	return( &(_VectorEntities[indexprop]) );
}

//---------------------------------------------------
// Property management : Set a new property
// 
//---------------------------------------------------
bool CFrontEndPropertyReceiver::setProperties( const CEntityId& id, uint32 index, TPropertiesValue& value )
{
	// Find the index
	TPropertiesIndex i = findIndex( id );
	if( i == -1 )
	{
		// it's a new index to build
		i = assignIndexToId( id );
		// if failed
		if( i == -1 )
			return( false );
		// if not, it's a new entity
		_VectorEntities[i].id = id;
		_VectorEntities[i].code |= PROPERTY_CODE_NEW;
		// Set the ContinueValuesMask;
		TPropertiesContMask mask = 0;
		// set mask of Continuous properties (bits for property continuous is set to 0, 1 if none continuous)
		_VectorEntities[i].mask = PropertiesContinuousMask [ id.Type ];
	}
	else
	{
		if( _VectorEntities[i].code & PROPERTY_CODE_ERASE )
		{
			_VectorEntities[i].code &= ~PROPERTY_CODE_ERASE;
		}
	}
	SEntity* p = &(_VectorEntities[i]);

	// Set the bitfield and the value
	uint32 indexProperty = PropertiesProperty [ id.Type ] [ index ].Index;
	if( (1 << indexProperty) & (~ p->mask) )
	{
		p->bitfield |= 1 << indexProperty;
	}
	if( PropertiesProperty [ id.Type ] [ index ].Union )
	{
		p->properties[ indexProperty ] = ( p->properties[ indexProperty ] & ~ PropertiesProperty [ id.Type ] [ index ].UnionMask ) | ( ( value << PropertiesProperty [ id.Type ] [ index ].UnionShift ) & PropertiesProperty [ id.Type ] [ index ].UnionMask );
	}
	else
	{
		p->properties[ indexProperty ] = value;
	}
	// If it's not in the list of updated entity, push it in!
	if( p->nextUpdatedEntityProperties == -2 )
	{
		p->nextUpdatedEntityProperties = _FirstUpdatedEntityProperties;
		_FirstUpdatedEntityProperties = i;
	}
	return( true );
}


//---------------------------------------------------
// Property management : Get the first modified properties since the last EndUpdatedProperties
// 
//---------------------------------------------------
CFrontEndPropertyReceiver::TPropertiesIndex CFrontEndPropertyReceiver::getFirstUpdatedProperties()
{
	return( _FirstUpdatedEntityProperties );
}

//---------------------------------------------------
// Property management : Get the next modified properties
// 
//---------------------------------------------------
CFrontEndPropertyReceiver::TPropertiesIndex CFrontEndPropertyReceiver::getNextUpdatedProperties( TPropertiesIndex indexprop )
{
	// if it's the end or continuous values have changed and there is not new ou erase entity in the code, then go to the next elt
	SEntity* p = &(_VectorEntities[indexprop]);
	TPropertiesIndex i = p->nextUpdatedEntityProperties;
	if( ( i == -1 )||( p->code )||( p->bitfield & p->mask ) ) 
		return( i );
	else
		return(	getNextUpdatedProperties( i ) ); 
}

//---------------------------------------------------
// Property management : Call it after getting all the updated properties (eg, the.GetNextUpdatedProperties function return -1 )
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::endUpdatedProperties()
{
	// for all the entities in the list of updated
	while( _FirstUpdatedEntityProperties != -1 )
	{
		// Get the next entity
		SEntity* p = &(_VectorEntities[_FirstUpdatedEntityProperties]);
		// erase entity if needed
		if( ( p->code & PROPERTY_CODE_ERASE ) != 0 )
		{
			setDeleted( _FirstUpdatedEntityProperties );
		}
		// RAZ code
		p->code &= ~( PROPERTY_CODE_NEW | PROPERTY_CODE_ERASE );
		// RAZ bitfield
		p->bitfield = 0;
		// Remove it from list of updated entities
		_FirstUpdatedEntityProperties = p->nextUpdatedEntityProperties;
		p->nextUpdatedEntityProperties = -2;
	}
}

//---------------------------------------------------
// Vision management : Set a new delta of vision
// 
//---------------------------------------------------
bool CFrontEndPropertyReceiver::setVision( const CEntityId& id, const map< NLMISC::CEntityId, TVisionSlot >& visionIn, const vector< NLMISC::CEntityId >& visionOut )
{
	// Find the index
	TPropertiesIndex iviewer = findIndex( id );
	if( iviewer == -1 )
	{
		nlwarning( "Trouble when synchronising vision and properties: viewer not created" );
		return false;
	}
	
	SEntity* p = &(_VectorEntities[iviewer]);
	// If entity has no unprocessed vision
	if( p->nextUpdatedEntityVision == -2 )
	{
		// Insert the entity in the vision list
		p->nextUpdatedEntityVision = _FirstUpdatedEntityVision;
		_FirstUpdatedEntityVision = iviewer;

		for( map< CEntityId, TVisionSlot >::const_iterator itIn = visionIn.begin(); itIn != visionIn.end(); ++itIn )
		{
			// Find the index of the id
			TPropertiesIndex iviewed = findIndex( (*itIn).first );
			if( iviewed == -1 )
			{
				// no synchronisation
				nlwarning( "CFrontEndPropertyReceiver::setVision: Trouble when synchronising vision in and properties: viewed not inserted" );	
			}
			else
			{
				p->VisionIn.insert( make_pair( iviewed, (*itIn).second ) );
			}
		}

		for( vector< NLMISC::CEntityId >::const_iterator itOut = visionOut.begin(); itOut != visionOut.end(); ++itOut )
		{
			// Find the index of the id
			TPropertiesIndex iviewed = findIndex( *itOut );
			if( iviewed == -1 )
			{
				// no synchronisation
				nlwarning( "CFrontEndPropertyReceiver::setVision: Trouble when synchronising vision out and properties: viewed not inserted" );	
			}
			else
			{
				p->VisionOut.insert( iviewed );
			}
		}
	}
	else
	{
		// merging old and new delta vision update (only if front-end have not process previous vision update)
		for( map< NLMISC::CEntityId, TVisionSlot >::const_iterator itIn = visionIn.begin(); itIn != visionIn.end(); ++itIn )
		{
			// Find the index of the id
			TPropertiesIndex iviewed = findIndex( (*itIn).first );
			if( iviewed == -1 )
			{
				// no synchronisation
				nlwarning( "CFrontEndPropertyReceiver::setVision: Trouble when synchronising vision in and properties: viewed not inserted" );	
			}
			else
			{
				p->VisionIn.insert( make_pair( iviewed, (*itIn).second ) );
				set< TPropertiesIndex >::iterator it = p->VisionOut.find( iviewed );
				if( it != p->VisionOut.end() )
				{
					p->VisionOut.erase( it );
				}
			}
		}

		for( vector< NLMISC::CEntityId >::const_iterator itOut = visionOut.begin(); itOut != visionOut.end(); ++itOut )
		{
			// Find the index of the id
			TPropertiesIndex iviewed = findIndex( *itOut );
			if( iviewed == -1 )
			{
				// no synchronisation
				nlwarning( "CFrontEndPropertyReceiver::setVision: Trouble when synchronising vision out and properties: viewed not inserted" );	
			}
			else
			{
				p->VisionOut.insert( iviewed );
				map< TPropertiesIndex, TVisionSlot >::iterator it = p->VisionIn.find( iviewed );
				if( it != p->VisionIn.end() )
				{
					p->VisionIn.erase( it );
				}
			}
		}
	}
	return true;
}

//---------------------------------------------------
// Vision management : Get the first modified vision since the last EndUpdatedVision
// 
//---------------------------------------------------
CFrontEndPropertyReceiver::TPropertiesIndex CFrontEndPropertyReceiver::getFirstUpdatedVision()
{
	return( _FirstUpdatedEntityVision );
}

//---------------------------------------------------
// Vision management : Get the next modified vision
// 
//---------------------------------------------------
CFrontEndPropertyReceiver::TPropertiesIndex CFrontEndPropertyReceiver::getNextUpdatedVision( TPropertiesIndex indexprop )
{
	return( _VectorEntities[indexprop].nextUpdatedEntityVision );
}

//---------------------------------------------------
// Vision management : Call it after getting all the updated vision (eg, the.GetNextUpdatedVision function return -1 )
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::endUpdatedVision()
{
	// for all the entities in the list of updated
	while( _FirstUpdatedEntityVision != -1 )
	{
		// Get the next entity
		TPropertiesIndex i = _FirstUpdatedEntityVision;
		SEntity* p = &(_VectorEntities[_FirstUpdatedEntityVision]);
		// Clean the lists
		p->VisionIn.clear(); 
		p->VisionOut.clear(); 
		// Remove it from list of updated entities
		_FirstUpdatedEntityVision = p->nextUpdatedEntityVision;
		p->nextUpdatedEntityVision = -2;
	}
}

//---------------------------------------------------
// CEntityId / index management : Create a new index for the CEntityId
// 
//---------------------------------------------------
CFrontEndPropertyReceiver::TPropertiesIndex CFrontEndPropertyReceiver::assignIndexToId( const CEntityId& id )
{
	// insert a new elt
	std::pair< TMapIdToIndex::iterator, bool> result = _MapIdToIndex.insert( TMapIdToIndex::value_type( id, _FirstFreeEntity ) );
	// if failed
	if( !result.second )
		return( -1 );
	// if not, actualize _FirstFreeEntityProperties
	TPropertiesIndex i = _FirstFreeEntity;
	_FirstFreeEntity = _VectorEntities[i].nextFreeEntity;
	return( i );
}

//---------------------------------------------------
// CEntityId / index management : Release index / id association
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::releaseIndexToId( const NLMISC::CEntityId& id )
{
	// find the index from the id
	TMapIdToIndex::iterator it = _MapIdToIndex.find( id );
	if( it != _MapIdToIndex.end() )
	{
		_MapIdToIndex.erase( it );
	}
}

//---------------------------------------------------
// CEntityId / index management : Get index, return true if finded and the value is setting. If false, value is undefined
// 
//---------------------------------------------------
CFrontEndPropertyReceiver::TPropertiesIndex CFrontEndPropertyReceiver::findIndex( const CEntityId& id )
{
	// find the index from the id
	TMapIdToIndex::iterator it = _MapIdToIndex.find( id );
	// if failed
	if( it == _MapIdToIndex.end() )
		return( -1 );
	// if not
	return( it->second );
}

//---------------------------------------------------
// UpdateProperties, unserial update mirrors message and process it
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::updateProperties( CMessage& msgin )
{
	//nlinfo("Received a properties update");

	TPropertiesValue Value;
	
	while( (uint32)msgin.getPos () != msgin.length() )
	{
		string PropertyName;
		msgin.serial( PropertyName );

		if( PropertyName == "Mode" )
		{
			nlwarning("Mode updated");
		}

		CEntityId id;
		msgin.serial( id );
		while( id != CEntityId::Unknown )
		{
			Value = 0;
			
			uint32 indexProperty = 0;
			
			while( indexProperty < NB_PROPERTIES_PER_ENTITY )
			{
				if( PropertiesProperty[ id.Type & 0x7f ] [ indexProperty ].PropertyName == PropertyName )
				{
					switch( PropertiesProperty[ id.Type & 0x7f ] [ indexProperty ].PropertyType )
					{
						case 0: // uint8
							{
								uint8 v;
								msgin.serial( v );
								Value = v;
							}
							break;
						case 1: // sint8
							{
								sint8 v;
								msgin.serial( v );
								Value = v;
							}
							break;
						case 2:	// uint16
							{
								uint16 v;
								msgin.serial( v );
								Value = v;
							}
							break;
						case 3:	// sint16
							{
								sint16 v;
								msgin.serial( v );
								Value = v;
							}
							break;
						case 4:	// uint32
							{
								uint32 v;
								msgin.serial( v );
								Value = v;
							}
							break;
						case 5:	// sint32
							{
								sint32 v;
								msgin.serial( v );
								Value = v;
							}
							break;
						case 6:	// uint64
							{
								uint64 v;
								msgin.serial( v );
								Value = v;
							}
							break;
						case 7:	// sint64
							{
								sint64 v;
								msgin.serial( v );
								Value = v;
							}
							break;
						case 8:	// float
							{
								float v;
								msgin.serial( v );
								*((float *)(&Value)) = v;
							}
							break;
						case 9:	// double
							{
								double v;
								msgin.serial( v );
								*((double *)(&Value)) = v;
							}
							break;

						default:
							nlwarning("Unknown type %d", PropertiesProperty[ id.Type & 0x7f ] [ indexProperty ].PropertyType );
							nlstop;
					}
					setProperties( id, indexProperty, Value );
					break;
				}
				
				++indexProperty;

				// if property is special property type, it's still code dependant at this time....
				if( indexProperty == NB_PROPERTIES_PER_ENTITY )
				{
					if( PropertyName == string("TickPos") )
					{
						TPropertiesIndex Index = findIndex( id );
						if( Index == 255 )
						{
							Value = 0;
							setProperties( id, x, Value );	// Create entity entry
							Index = findIndex( id );
						}
						msgin.serial( _VectorEntities[ Index ].TickPosition );
					}
					else if( PropertyName == string("Sheet") )
					{
						TPropertiesIndex Index = findIndex( id );
						if( Index == 255 )
						{
							Value = 0;
							setProperties( id, x, Value );	// Create entity entry
							Index = findIndex( id );
						}
						msgin.serial( _VectorEntities[ Index ].SheetId );
					}
				}
			}
			msgin.serial( id );
		}
	}
}

	/// 
//---------------------------------------------------
// RemoveEntity, unserial remove message and remove corresponding Entity
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::removeEntity( CMessage& msgin )
{
	list< CEntityId > entityToRemove;

	msgin.serialCont( entityToRemove );

	for( list< CEntityId >::iterator it = entityToRemove.begin(); it != entityToRemove.end(); ++it )
	{
		deleteEntity( *it );
	}
}

//---------------------------------------------------
// UpdateVision, unserial update vision message and process it
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::updateVision( CMessage& msgin )
{
	nlinfo("Received a vision update");
	list< SPlayerVisionDelta > deltaVision;

	msgin.serialCont( deltaVision );

	for( list< SPlayerVisionDelta >::iterator it = deltaVision.begin(); it != deltaVision.end(); ++it )
	{
		setVision( (*it).Id, (*it).EntityIn, (*it).EntityOut );
	}
}

//---------------------------------------------------
// Ask subscribe for one property
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::askPropertySubscribe( const string& ServiceSubscribe, const pair< string, uint32 >& Property )
{
	CMessage msgOut( "PROPERTY_SUBSCRIBE" );
	string serviceSubscribe = ServiceSubscribe;

	string PropertyName = Property.first;
	uint32 UpdateDelay = Property.second;

	msgOut.serial( PropertyName );
	msgOut.serial( UpdateDelay );
	CUnifiedNetwork::getInstance()->send( serviceSubscribe, msgOut );

	nlinfo("<CFrontEndPropertyReceiver::askPropertySubscribe> ask for property %s to service %s", PropertyName.c_str() , serviceSubscribe.c_str() );
}

//---------------------------------------------------
// Ask subscribe for list of properties
// 
//---------------------------------------------------
void CFrontEndPropertyReceiver::askPropertiesSubscribe( const string& ServiceSubscribe, const list< pair< string, uint32 > >& Properties )
{
	CMessage msgOut( "PROPERTIES_SUBSCRIBE" );
	uint16 numberProperty = Properties.size();
	msgOut.serial( numberProperty );

	string PropertyName;
	uint32 UpdateDelay;
	for( list< pair< string, uint32 > >::const_iterator it = Properties.begin(); it != Properties.end(); ++it )
	{
		PropertyName = (*it).first;
		UpdateDelay = (*it).second;
		msgOut.serial( PropertyName );
		msgOut.serial( UpdateDelay );
		nlinfo("<CFrontEndPropertyReceiver::askPropertySubscribe> ask for property %s to service %s", PropertyName.c_str() , ServiceSubscribe.c_str() );
	}
	CUnifiedNetwork::getInstance()->send( ServiceSubscribe, msgOut );
}

//---------------------------------------------------
// Callback for delta mirror update received
// 
//---------------------------------------------------
void cbDeltaUpdate( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CFrontEndPropertyReceiver::updateProperties( msgin );
}

// Callback for delta vision update remove received
void cbDeltaUpdateRemove( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CFrontEndPropertyReceiver::removeEntity( msgin );
}

//---------------------------------------------------
// Callback for delta vision update received
// 
//---------------------------------------------------
void cbDeltaVision( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CFrontEndPropertyReceiver::updateVision( msgin );
}

