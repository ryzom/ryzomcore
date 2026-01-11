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
#include "data_set_base.h"
#include <nel/georges/u_form_elm.h>

#include "ryzom_entity_id.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;


// The following variable controls the logging of nlinfo and nldebug messages produced by the MIRROR_INFO and MIRROR_DEBUG macros
CVariable<bool> VerboseMIRROR("ms","VerboseMIRROR","Enable verbose logging in Mirror operations",true,0,true);


#ifdef MIRROR_LIST_ROW_32BITS
const char *ListRowSizeString = "L32";
#else
const char *ListRowSizeString = "L16";
#endif


/*
 * Constructor
 */
CDataSetBase::CDataSetBase() : _MaxNbRows(INVALID_DATASET_INDEX) //, _SizeOfRow(Row32)
{
}



/*
 * Initialize
 */
void		CDataSetBase::init( const NLMISC::CSheetId& sheetId, const TDataSetSheet& properties )
{
	/*string dataSetName = "player";
	TPropertyIndex nbProperties = 4;
	_PropIndexToType.resize( nbProperties );
	_PropIndexToType[0] = TypeSint32;
	_PropIndexToType[1] = TypeSint32;
	_PropIndexToType[2] = TypeSint32;
	_PropIndexToType[3] = TypeSint32;
	_DataSetName = dataSetName;*/

	nlctassert( sizeof(TDataSetRow) == 4 );

	_SheetId = sheetId;
	_DataSetName = properties.DataSetName;
	_MaxNbRows = properties.getConfigDataSetSize();
	//_SizeOfRow = (_MaxNbRows>65534) ? Row32 : ((_MaxNbRows>254)?Row16:Row8);
	//nlinfo( "\t Dataset %s:      Rows will be transfered using %s-bit index", _DataSetName.c_str(), (_SizeOfRow==Row32)?"32":((_SizeOfRow==Row16)?"16":"8") );
	_PropIndexToType = properties.PropIndexToType;
	fillDataSizeByProp();
	_EntityTypesFilter = properties.EntityTypesFilter;
	_PropIsList = properties.PropIsList;
	_PropertyContainer.init( properties.NbProperties );
}


/*
 * Fill property info.
 * When the flag initValues is set:
 *   If the counts must be initialised from a remote MS, use the vector ptCountsToSet (it will be read
 *   then deleted) and timestamp, otherwise set NULL.
 * If propName is the special entityId property, the behaviour is different (propName need not exist)
 */
void		CDataSetBase::setPropertyPointer( std::string& propName, TPropertyIndex propIndex, void *segmentPt, bool initValues, uint32 dataTypeSize, bool isReadOnly, bool mustMonitorAssignment, std::vector<uint8> *ptCountsToSet, NLMISC::TGameCycle /* timestamp */, uint32 segmentSize )
{
	uint8 *endPt = (uint8*)segmentPt;

	// Handle special case of _CEntityId_
	if ( propName.substr(0,LENGTH_OF_ENTITYID_PREFIX) == ENTITYID_PREFIX )
	{
		_PropertyContainer.EntityIdArray.EntityIds = (CEntityId*)segmentPt;
		_PropertyContainer.EntityIdArray.OnlineBitfieldPt = (uint32*)((uint8*)segmentPt + (sizeof(CEntityId)*maxNbRows()));
		_PropertyContainer.EntityIdArray.OnlineTimestamps = (NLMISC::TGameCycle*)(((uint8*)_PropertyContainer.EntityIdArray.OnlineBitfieldPt) + maxNbRows()/8 + 4);
		_PropertyContainer.EntityIdArray.Counts = ((uint8*)_PropertyContainer.EntityIdArray.OnlineTimestamps) + maxNbRows()*sizeof(NLMISC::TGameCycle);
		_PropertyContainer.EntityIdArray.SpawnerServiceIds = ((TServiceId8*)_PropertyContainer.EntityIdArray.Counts) + maxNbRows()*sizeof(TServiceId8);
		endPt = ((uint8*)_PropertyContainer.EntityIdArray.SpawnerServiceIds) + maxNbRows()*sizeof(TServiceId8);

		// In the Mirror Service, initialize the value
		if ( initValues )
		{
			// Set the entityid array to the default value
			for ( TDataSetIndex i=0; i!=(uint32)maxNbRows(); ++i )
			{
				_PropertyContainer.EntityIdArray.EntityIds[i] = CEntityId::Unknown;
				_PropertyContainer.EntityIdArray.setOnline( i, false );
				if ( ptCountsToSet != NULL )
					_PropertyContainer.EntityIdArray.Counts[i] = (*ptCountsToSet)[i];
				else
					_PropertyContainer.EntityIdArray.Counts[i] = 1; // begins at 1, 0 is reserved for "no counter"

				// Always init the online timestamps to 0 (otherwise the assert in declareEntity() will fail)
				_PropertyContainer.EntityIdArray.OnlineTimestamps[i] = 0;

				_PropertyContainer.EntityIdArray.SpawnerServiceIds[i] = TServiceId8(0);
			}
			if ( ptCountsToSet != NULL )
			{
				delete ptCountsToSet;
			}
		}
	}
	else
	{
		if ( (uint)propIndex < (uint)nbProperties() )
		{
			_PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps = (TGameCycle*)segmentPt;
#ifdef STORE_CHANGE_SERVICEIDS
			_PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds = (TServiceId8*)((uint8*)segmentPt + (sizeof(TGameCycle)*maxNbRows()));
			_PropertyContainer.PropertyValueArrays[propIndex].Values = (void*)((uint8*)_PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds + (sizeof(TServiceId8)*maxNbRows()));
#else
			_PropertyContainer.PropertyValueArrays[propIndex].Values = (void*)((uint8*)segmentPt + (sizeof(TGameCycle)*maxNbRows()));
#endif
			_PropertyContainer.PropertyValueArrays[propIndex].DataTypeSize = dataTypeSize;
#ifdef NL_DEBUG
			_PropertyContainer.PropertyValueArrays[propIndex].IsReadOnly = isReadOnly;
			_PropertyContainer.PropertyValueArrays[propIndex].IsMonitored = mustMonitorAssignment;
#endif

			// Cell container for list properties
			if ( _PropIsList[ propIndex ] )
			{
				uint8 *ptFreeCellsFront = ((uint8*)(_PropertyContainer.PropertyValueArrays[propIndex].Values)) + sizeof(TSharedListRow)*maxNbRows();
				_PropertyContainer.PropertyValueArrays[propIndex].ListCellContainer = ptFreeCellsFront + sizeof(TSharedListRow);

				// In the Mirror Service, set up the list headers (1 per datasetrow) and container
				if ( initValues )
				{
					// Init the headers
					TSharedListRow *listHeaders = (TSharedListRow*)(_PropertyContainer.PropertyValueArrays[propIndex].Values);
					for ( TDataSetIndex i=0; i!=(uint32)maxNbRows(); ++i )
					{
						listHeaders[i] = INVALID_SHAREDLIST_ROW;
					}
					uint32 datasize = getDataSizeOfProp( propIndex );

					//nldebug( "%p + %u bytes + %u", _PropertyContainer.PropertyValueArrays[propIndex].Values, sizeof(TSharedListRow)*maxNbRows(), sizeof(TSharedListRow) );
					//nldebug( "%p", _PropertyContainer.PropertyValueArrays[propIndex].ListCellContainer );
					//nldebug( "Segment size: %u", segmentSize );
					nldebug( "Shared List container uses %u bytes (at %u bytes per cell)", segmentSize - ( (uint8*)(_PropertyContainer.PropertyValueArrays[propIndex].ListCellContainer) - (uint8*)segmentPt ), sizeof(TSharedListRow)+datasize );
					//nldebug( "End at %p", (uint8*)segmentPt + segmentSize );

					// Init the list of free cells
					TSharedListRow *freeCellsFront = (TSharedListRow*)ptFreeCellsFront;
					freeCellsFront = 0;

					uint8 *cellPt = (uint8*)(_PropertyContainer.PropertyValueArrays[propIndex].ListCellContainer);
					for ( TSharedListRow j=0; j!=NB_SHAREDLIST_CELLS-1 /*TEMP*/; ++j )
					{
						TSharedListRow *nextField = (TSharedListRow*)cellPt;
						*nextField = j+1;
						cellPt += sizeof(TSharedListRow) + datasize; // assumes #pragma pack!
					}
					TSharedListRow *nextField = (TSharedListRow*)cellPt;
					*nextField = INVALID_SHAREDLIST_ROW;

					endPt = ((uint8*)_PropertyContainer.PropertyValueArrays[propIndex].ListCellContainer) + (NB_SHAREDLIST_CELLS*(sizeof(TSharedListRow) + datasize)); // assumes #pragma pack!
				}
			}
			else
			{
				_PropertyContainer.PropertyValueArrays[propIndex].ListCellContainer = NULL;

				if ( initValues )
				{
					endPt = ((uint8*)_PropertyContainer.PropertyValueArrays[propIndex].Values) + maxNbRows()*dataTypeSize;
				}
			}
		}
	}

	if ( initValues )
	{
		// Check that the total size of segment is sufficient
		nlassert( ((uint8*)segmentPt) + segmentSize >= endPt );
	}
}


/*
 * Fill the datasize array using the prop to type array
 */
void		CDataSetBase::fillDataSizeByProp()
{
	_DataSizeByProp.resize( _PropIndexToType.size() );
	vector<uint32>::iterator itd;
	vector<TTypeOfProp>::const_iterator itp;
	for ( itp=_PropIndexToType.begin(), itd=_DataSizeByProp.begin(); itp!=_PropIndexToType.end(); ++itp, ++itd )
	{
		switch ( *itp )
		{
		case TypeUint8: *itd = 1; break;
		case TypeSint8: *itd = 1; break;
		case TypeUint16: *itd = 2; break;
		case TypeSint16: *itd = 2; break;
		case TypeUint32: *itd = 4; break;
		case TypeSint32: *itd = 4; break;
		case TypeUint64: *itd = 8; break;
		case TypeSint64: *itd = 8; break;
		case TypeFloat: *itd = 4; break;
		case TypeDouble: *itd = 8; break;
		case TypeCEntityId: *itd = 8; break;
		case TypeBool: *itd = 1; break;
		default: *itd = 0; break;
		}
	}
}


/*
 * Check that the template argument T passed to CMirrorPropValue<T> has the right size
 */
void		CDataSetBase::checkTemplateSize( uint32 passedSize, TPropertyIndex propIndex, const TDataSetRow& entityIndex) const
{
	nlassertex( passedSize == _PropertyContainer.PropertyValueArrays[propIndex].DataTypeSize,
		("Wrong CMirrorPropValue template argument: size=%u (expected: %u) (%s/E%d/P%hd)",
		passedSize, _PropertyContainer.PropertyValueArrays[propIndex].DataTypeSize, name().c_str(), entityIndex.getIndex(), propIndex ) );
}


/*
 * Read the sheet
 */
void		TDataSetSheet::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
	if( (UForm*)form )
	{
		// Get the name of the dataset
		DataSetName = sheetId.toString();
		DataSetName.erase( DataSetName.find_last_of( '.' ) ); // erase .dataset
		MIRROR_INFO( "MIRROR: Reading sheet for dataset '%s'", DataSetName.c_str() );

		UFormElm& root = form->getRootNode();

		// Get the max number of rows (will be used only if not overriden in the .cfg)
		if ( root.getValueByName( MaxNbRows, "max number of rows or entities" ) )
		{
			MIRROR_INFO( "MIRROR: \tMaxNbRows = %d in sheet", MaxNbRows );
		}
		else
		{
			MaxNbRows = 0;
			nlinfo( "MaxNbRows of %s not specified in sheet, expecting value in .cfg", DataSetName.c_str() );
		}

		// Get property types and names
		UFormElm *arrayProperties;
		if( root.getNodeByName( &arrayProperties, "properties" ) && arrayProperties )
		{
			uint size;
			nlverify( arrayProperties->getArraySize(size) );
			nlassert( size < 0x10000 );
			NbProperties = size;
			PropertyNames.resize( NbProperties );
			PropIndexToType.resize( NbProperties );
			//Weights.resize( NbProperties );
			PropIsList.resize( NbProperties );

			for ( sint p=0; p!=NbProperties; ++p )
			{
				const UFormElm *property = NULL;
				if ( arrayProperties->getArrayNode( &property, p ) && property )
				{
					// Name of the property
					property->getValueByName( PropertyNames[p], "name" );

					// Type of the property
					string typeStr;
					property->getValueByName( typeStr, "type" );
					TTypeOfProp theType;
					if ( typeStr == "uint8" ) theType = TypeUint8;
					else if ( typeStr == "sint8" ) theType = TypeSint8;
					else if ( typeStr == "uint16" ) theType = TypeUint16;
					else if ( typeStr == "sint16" ) theType = TypeSint16;
					else if ( typeStr == "uint32" ) theType = TypeUint32;
					else if ( typeStr == "sint32" ) theType = TypeSint32;
					else if ( typeStr == "uint64" ) theType = TypeUint64;
					else if ( typeStr == "sint64" ) theType = TypeSint64;
					else if ( typeStr == "float" ) theType = TypeFloat;
					else if ( typeStr == "double" ) theType = TypeDouble;
					else if ( typeStr == "entityid" ) theType = TypeCEntityId;
					else if ( typeStr == "boolean" ) theType = TypeBool;
					else
					{
						nlerror( "MIRROR: CDataSetBase::readGeorges: unknown data type '%s'", typeStr.c_str() ); // not forcing to TypeUint64 anymore
						theType = TypeUint64;
					}
					PropIndexToType[p] = theType;

					// Weight
					/*sint32 weight;
					if ( ! property->getValueByName( weight, "weight" ) )
					{
						weight = 1;
					}
					if ( weight > 255 )
					{
						nlwarning( "Weights must be lower than 256" );
						weight = 255;
					}
					Weights[p] = (uint8)weight;
					*/

					// List or single value?
					bool b;
					if ( ! property->getValueByName( b, "is a list" ) )
					{
						b = false;
					}
					PropIsList[p] = b;

					MIRROR_INFO( "MIRROR: \tProperty '%s' : Type %s%s"/*", Weight %d"*/, PropertyNames[p].c_str(), PropIsList[p]?"list of ":"", typeStr.c_str()/*, Weights[p]*/ );

					// persistant
					//property->getValueByName(propElt.Persistant, "persistant");
					// mirror
					//property->getValueByName(propElt.Mirror, "mirror");
				}
			}
		}
		else
		{
			nlwarning( "MIRROR: CDataSetBase::readGeorges: 'properties' not found" );
		}

		// Get entity type filter
		UFormElm *arrayEntityTypes;
		if ( root.getNodeByName( &arrayEntityTypes, "entity types" ) && arrayEntityTypes )
		{
			uint size;
			nlverify( arrayEntityTypes->getArraySize(size) );
			nlassert( size < 0x10000 );
			EntityTypesFilter.resize( size );
			for ( uint et=0; et!=size; ++et )
			{
				string typeName;
				arrayEntityTypes->getArrayValue( typeName, et );
				EntityTypesFilter[et] = RYZOMID::fromString(typeName);
				MIRROR_INFO( "MIRROR: \tLinking to entity type %hu", (uint16)EntityTypesFilter[et] );
			}
		}
		else
		{
			nlwarning( "MIRROR: CDataSetBase::readGeorges: 'entity types' not found" );
		}
	}
}


/*
 * Serial (for fast binary sheet loading)
 */
void		TDataSetSheet::serial( NLMISC::IStream& s )
{
	s.serial( DataSetName );
	s.serial( MaxNbRows );
	s.serial( NbProperties );
	if ( s.isReading() )
	{
		PropIndexToType.resize( NbProperties );
		PropertyNames.resize( NbProperties );
		//Weights.resize( NbProperties );
		PropIsList.resize( NbProperties );
	}
	for ( TPropertyIndex p=0; p!=NbProperties; ++p )
	{
		s.serial( PropertyNames[p] );
		s.serialEnum( PropIndexToType[p] );
		MIRROR_DEBUG( "MIRROR: Serializing %s, type %u", PropertyNames[p].c_str(), PropIndexToType[p] );
		//s.serial( Weights[p] );
		bool b = PropIsList[p];
		s.serial( b );
		PropIsList[p] = b;
	}
	s.serialCont( EntityTypesFilter );
}


/*
 * Return the dataset size to use.
 * If found in the .cfg, use the value of the property DataSetSize + DataSetName.
 * If not found, use the value in the sheet.
 */
TDataSetIndex	TDataSetSheet::getConfigDataSetSize() const
{
	string cfgVarName = "DatasetSize" + DataSetName;
	CConfigFile::CVar *cfgVar = NLNET::IService::getInstance()->ConfigFile.getVarPtr( cfgVarName );
	TDataSetIndex result;
	if ( cfgVar )
	{
		result = cfgVar->asInt();
		nlinfo( "\t%s: %u (from cfg)", cfgVarName.c_str(), result );
	}
	else
	{
		result = MaxNbRows;
		nlinfo( "\t%s: %u (from packed sheet)", cfgVarName.c_str(), result );
	}
	return result;
}
