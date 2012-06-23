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



#include "range_mirror_manager.h"
#include <functional>

#include <nel/misc/o_xml.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/file.h>
#include <nel/net/unified_network.h>
#include <nel/georges/load_form.h>
#include <nel/misc/command.h>
#include "game_share/data_set_base.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


CRangeMirrorManager *RMMInstance = NULL;

// Pause state of the Tick Service (can be removed if the Range Manager is not hosted by the Tick Service)
extern bool Pause;


const char RANGE_MANAGER_BACKUP_FILE [30] = "mirror_row_ranges.xml";


void cbGetRange( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId mirrorServiceId )
{
	if ( Pause )
	{
		nlwarning( "A service is getting a range while the shard is paused" );
		beep( 880, 150 );
		beep( 660, 150 );
		beep( 880, 150 );
	}
	vector<string> datasetNames;
	NLNET::TServiceId declaratorServiceId;
	sint32 maxNbEntities;
	uint8 entityTypeId;
	msgin.serialCont( datasetNames );
	msgin.serial( declaratorServiceId );
	msgin.serial( maxNbEntities );
	msgin.serial( entityTypeId );

	RMMInstance->getRange( datasetNames, maxNbEntities, declaratorServiceId, mirrorServiceId, entityTypeId );
}


void cbReleaseRange( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId mirrorServiceId )
{
	vector<string> datasetNames;
	NLNET::TServiceId declaratorServiceId;
	msgin.serialCont( datasetNames );
	msgin.serial( declaratorServiceId );
	RMMInstance->releaseRanges( datasetNames, declaratorServiceId, mirrorServiceId );
}


void cbReacquireRanges( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId mirrorServiceId )
{
	RMMInstance->reacquireRanges( msgin, mirrorServiceId );
}


// Callback Array
TUnifiedCallbackItem RangeMirrorManagerCallbackArray[] =
{
	{ "GRG", cbGetRange },
	{ "RRG", cbReleaseRange },
	{ "RARG", cbReacquireRanges }
};


/*
 * Constructor
 */
CRangeMirrorManager::CRangeMirrorManager()
{
	nlassert( ! RMMInstance );
	RMMInstance = this;
}


class ServiceNameIs : public binary_function< CNamingClient::CServiceEntry, const char *, bool >
{
public:
	bool	operator() ( const CNamingClient::CServiceEntry& se, const char* name ) const
	{
		return (se.Name == string(name));
	}
};


/*
 * Callback called at MS going down
 *
 * Note: If this function conflicts with cbMSDown in mirror.cpp at linking,
 * it means that mirror.cpp has been selected for linking in the Tick Service,
 * which is bad (some CVariable may be incorrectly called then). In this case,
 * check that no global variables from mirror.cpp are used in the Tick Service.
 */
void cbMSDown( const string& serviceName, NLNET::TServiceId serviceId, void * )
{
	// Test if there is any MS remaining
	list<CNamingClient::CServiceEntry> serviceList = CNamingClient::getRegisteredServices();
	if ( find_if( serviceList.begin(), serviceList.end(), std::bind2nd(ServiceNameIs(), "MS") ) == serviceList.end() )
	{
		// Not found => reset the ranges
		RMMInstance->releaseAllRanges();
		nlinfo( "Detected that all MS are shutdown, clearing all ranges" );
	}
	else
	{
		// New: remove any service the MS of which is down (in case of MS crash)
		RMMInstance->releaseRangesByMS( serviceId );
	}
}


/*
 * Init
 */
void				CRangeMirrorManager::init()
{
	// Init callbacks
	CUnifiedNetwork::getInstance()->addCallbackArray( RangeMirrorManagerCallbackArray, sizeof(RangeMirrorManagerCallbackArray)/sizeof(TUnifiedCallbackItem) );
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "MS", cbMSDown, 0 );

	// Load datasets into temporary map to get the names
	TSDataSetSheets sDataSetSheets;
	loadForm( "dataset", "data_shard/datasets.packed_sheets", sDataSetSheets );
	TSDataSetSheets::iterator ism;
	for ( ism=sDataSetSheets.begin(); ism!=sDataSetSheets.end(); ++ism )
	{
		nlinfo( "\tDataset: %s", (*ism).second.DataSetName.c_str() );
		sint32 actualDataSetSize = (sint32)((*ism).second.getConfigDataSetSize());
		_RangeListByDataSet.insert( make_pair( (*ism).second.DataSetName, CRangeList( actualDataSetSize + 1 ) ) );
	}

	/* // Replaced by the redeclaration of ranges by MS
	// Load ranges from xml file (backup in case of shutdown or crash)
	loadRanges();

	// Release the ranges that do not correspond to a 'live' service
	bool b = false;
	TRangeListByDataSet::iterator ids;
	for ( ids=_RangeListByDataSet.begin(); ids!=_RangeListByDataSet.end(); ++ids )
	{
		b |= GET_RANGE_LIST(ids).releaseRangesForServicesDown();
	}
	if ( b )
		saveRanges();*/
}


/*
 * Get a range
 */
void				CRangeMirrorManager::getRange( const std::vector<string>& datasetNames, sint32 nbRows, NLNET::TServiceId declaratorServiceId, NLNET::TServiceId mirrorServiceId, uint8 entityTypeId )
{
	CMessage msgout( "RG" );
	msgout.serial( declaratorServiceId );
	msgout.serial( entityTypeId );
	//nlassert( datasetNames.size() < 256 );
	uint8 nbDatasets = (uint8)datasetNames.size();
	uint nbErrors = 0;
	string errorStr;
	msgout.serial( nbDatasets );
	for ( sint i=0; i!=nbDatasets; ++i )
	{
		TDataSetIndex first, last;

		TRangeListByDataSet::iterator ids = _RangeListByDataSet.find( datasetNames[i] );
		if ( ids != _RangeListByDataSet.end() )
		{
			// Acquire range
			if ( ! GET_RANGE_LIST(ids).acquireRange( declaratorServiceId, mirrorServiceId, nbRows, &first, &last ) )
			{
				errorStr = NLMISC::toString( "Cannot acquire requested range of %d rows in %s, index limit reached (declarator=%hu entityTypeId=%hu) (check 'max number of rows' in dataset file)", nbRows, ((*ids).first).c_str(), declaratorServiceId.get(), (uint16)entityTypeId, nbRows );
				nlwarning( errorStr.c_str() );
				displayRanges();
//#ifdef NL_OS_WINDOWS
				//nlstop;
//#endif
				first	=	INVALID_DATASET_INDEX;
				last	=	INVALID_DATASET_INDEX;
				++nbErrors;
			}
		}
		else
		{
			nlwarning( "RangeMirrorManager: dataset %s not found", datasetNames[i].c_str() );
			first	=	INVALID_DATASET_INDEX;
			last	=	INVALID_DATASET_INDEX;
			++nbErrors;
#ifdef NL_OS_WINDOWS
			nlstop;
#endif
		}

		// Answer mirror service
		msgout.serial( const_cast<string&>(datasetNames[i]) );
		msgout.serial( first, last );
		if ( ! errorStr.empty() )
		{
			msgout.serial( errorStr );
			errorStr.clear();
		}

		// Save ranges into xml file
		//saveRanges();
	}

	// Send answer
	CUnifiedNetwork::getInstance()->send( mirrorServiceId, msgout );
	nlinfo( "RangeMirrorManager: got %u ranges for owner %hu (via MS %hu) (%u errors)", ((uint)nbDatasets)-nbErrors, declaratorServiceId.get(), mirrorServiceId.get(), nbErrors );
}


/*
 * Release a range
 */
void				CRangeMirrorManager::releaseRanges( const std::vector<string>& datasetNames, NLNET::TServiceId declaratorServiceId, NLNET::TServiceId mirrorServiceId )
{
	for ( sint i=0; i!=(sint)datasetNames.size(); ++i )
	{
		TRangeListByDataSet::iterator ids = _RangeListByDataSet.find( datasetNames[i] );
		if ( ids != _RangeListByDataSet.end() )
		{
			uint nbReleased = 0;

			// Release ranges
			while ( GET_RANGE_LIST(ids).releaseOneRange( declaratorServiceId ) )
				++nbReleased;
		}
	}

	// Save ranges into xml file
	//saveRanges();
	
	nlinfo( "RangeMirrorManager: released %u ranges of owner %hu (via MS %hu)", datasetNames.size(), declaratorServiceId.get(), mirrorServiceId.get() );
}


/*
 * Release all ranges owned by a service
 */
void				CRangeMirrorManager::releaseRangesByService( NLNET::TServiceId declaratorServiceId )
{
	uint nbReleased = 0;
	TRangeListByDataSet::iterator ids;
	for ( ids=_RangeListByDataSet.begin(); ids!=_RangeListByDataSet.end(); ++ids )
	{
		while ( GET_RANGE_LIST(ids).releaseOneRange( declaratorServiceId ) )
			++nbReleased;
	}
	nlinfo( "RangeMirrorManager: released %u ranges of owner %hu", nbReleased, declaratorServiceId.get() );
}


/*
 * Release all acquired ranges
 */
void				CRangeMirrorManager::releaseAllRanges()
{
	TRangeListByDataSet::iterator ids;
	for ( ids=_RangeListByDataSet.begin(); ids!=_RangeListByDataSet.end(); ++ids )
	{
		GET_RANGE_LIST(ids).releaseAllRanges();
	}

	// Save ranges into xml file
	//saveRanges();
}


/*
 * Release ranges by MS
 */
void				CRangeMirrorManager::releaseRangesByMS( NLNET::TServiceId msId )
{
	CMessage msgout( "REIR" ); // Remove Entities In Ranges
	msgout.serial( msId );
	uint16 nbDatasets = (uint16)_RangeListByDataSet.size();
	msgout.serial( nbDatasets );

	bool hasErasedRanges = false;
	TRangeListByDataSet::iterator ids;
	for ( ids=_RangeListByDataSet.begin(); ids!=_RangeListByDataSet.end(); ++ids )
	{
		msgout.serial( const_cast<string&>((*ids).first) );
		vector<TRowRange> erasedRanges;
		GET_RANGE_LIST(ids).releaseRangesByMS( msId, erasedRanges );
		msgout.serialCont( erasedRanges );
		hasErasedRanges |= (!erasedRanges.empty());
	}

	if ( hasErasedRanges )
	{
		// Tell all other MS that they have to remove all entities in these ranges!
		CUnifiedNetwork::getInstance()->send( "MS", msgout );
	}
}


/*
 * Reacquire ranges after a service failure or shutdown
 *
 * Warning: this will fail if some services acquire some ranges before any reacquisition!
 */
void				CRangeMirrorManager::reacquireRanges( NLNET::CMessage& msgin, NLNET::TServiceId mirrorServiceId )
{
	uint nbReacquired = 0;
	uint16 nbServices;
	msgin.serial( nbServices );
	for ( uint i=0; i!=(uint)nbServices; ++i )
	{
		NLNET::TServiceId serviceId;
		uint16 nbRanges;
		msgin.serial( serviceId );
		msgin.serial( nbRanges );
		for ( uint r=0; r!=(uint)nbRanges; ++r )
		{
			string datasetname;
			TDataSetIndex first, last;
			msgin.serial( datasetname );
			msgin.serial( first, last );
			TRangeListByDataSet::iterator ids = _RangeListByDataSet.find( datasetname );
			if ( ids != _RangeListByDataSet.end() )
			{
				if ( ! CUnifiedNetwork::getInstance()->getServiceName( serviceId ).empty() )
				{
					GET_RANGE_LIST(ids).reacquireRange( serviceId, mirrorServiceId, first, last );
					++nbReacquired;
				}
				else
				{
					nlinfo( "RangeMirrorManager: skipping offline service %hu when reacquiring", serviceId.get() );
				}
			}
			else
			{
				nlwarning( "RangeMirrorManager: dataset %s not found", datasetname.c_str() );
			}
		}
	}
	nlinfo( "RangeMirrorManager: reacquired %u ranges for MS-%hu", nbReacquired, mirrorServiceId.get() );
}


/*
 * Display the ranges
 */
void				CRangeMirrorManager::displayRanges( NLMISC::CLog *log )
{
	TRangeListByDataSet::iterator ids;
	for ( ids=_RangeListByDataSet.begin(); ids!=_RangeListByDataSet.end(); ++ids )
	{
		log->displayNL( "Range list for dataset %s (%d rows max):", ((*ids).first).c_str(), GET_RANGE_LIST(ids).totalMaxRows() );
		GET_RANGE_LIST(ids).displayRanges( log );
	}
}


/*
 * Save the ranges into a data file
 */
void				CRangeMirrorManager::saveRanges()
{
	try
	{
		COFile file;
		if ( file.open( RANGE_MANAGER_BACKUP_FILE ) )
		{
			COXml output;
			if ( output.init( &file, "1.0" ) )
			{
				output.serialCont( _RangeListByDataSet );
				output.flush();
			}
			file.close();
		}
		else
			throw EFileNotOpened( RANGE_MANAGER_BACKUP_FILE );
	}
	catch (const Exception &e)
	{
		nlwarning( "Can't save ranges: %s", e.what() );
	}
}


/*
 * Load the ranges from a data file
 */
void				CRangeMirrorManager::loadRanges()
{
	try
	{
		CIFile file;
		if ( file.open( RANGE_MANAGER_BACKUP_FILE ) )
		{
			CIXml input;
			if ( input.init( file ) )
			{
				input.serialCont( _RangeListByDataSet );
			}
			file.close();
		}
		else
			throw EFileNotOpened( RANGE_MANAGER_BACKUP_FILE );
		nlinfo( "RangeMirrorManager: successfully loaded ranges" );
	}
	catch (const Exception &e)
	{
		nlinfo( "Can't load ranges: %s", e.what() ); // Info because not a problem
	}
}


/*
 * Serial CRangeList
 */
void				CRangeList::serial( NLMISC::IStream& s )
{
	s.serial( _TotalMaxRows );
	s.serialCont( _RangeList );
}


/*
 * Acquire the row 0
 */
void				CRangeList::acquireFirstRow()
{
	TDataSetIndex first, last;
	acquireRange( NLNET::TServiceId(0), NLNET::TServiceId(0), 1, &first, &last );
}


/*
 * Acquire a range and return the bounds, if it is possible with the limit of totalMaxRows in the dataset.
 */
bool				CRangeList::acquireRange( NLNET::TServiceId ownerServiceId, NLNET::TServiceId mirrorServiceId, sint32 nbRows, TDataSetIndex *first, TDataSetIndex *last )
{
	TDataSetIndex prevlast(~0);
	TRangeList::iterator irl = _RangeList.begin();

	// Find a compatible range
	while ( ! rangeInserted( ownerServiceId, mirrorServiceId, nbRows, prevlast, irl ) )
	{
		prevlast = (*irl).Last;
		++irl;
	}

	if ( (*irl).Last < (uint32)_TotalMaxRows )
	{
		// OK
		*first = (*irl).First;
		*last = (*irl).Last;
		return true;
	}
	else
	{
		// Exceeding size: cancel
		nldebug( "Exceeding %u", _TotalMaxRows );
		_RangeList.erase( irl );
		return false;
	}
}


/*
 * Reacquire ranges
 */
void				CRangeList::reacquireRange( NLNET::TServiceId ownerServiceId, NLNET::TServiceId mirrorServiceId, TDataSetIndex first, TDataSetIndex last )
{
	TDataSetIndex prevlast(first-1);
	TRangeList::iterator irl = _RangeList.begin();

	while ( ! rangeInserted( ownerServiceId, mirrorServiceId, last-first+1, prevlast, irl ) )
	{
		prevlast = (*irl).Last;
		++irl;
	}

	if ( ! ((*irl).Last < (uint32)_TotalMaxRows) )
	{
		nlwarning( "Failed to reacquire range [%d,%d] for %hu", first, last, ownerServiceId.get() );
		_RangeList.erase( irl );
	}
}


/*
 * Release a range previously acquired
 */
bool				CRangeList::releaseOneRange( NLNET::TServiceId ownerServiceId )
{
	TRangeList::iterator irl;
	for ( irl=_RangeList.begin(); irl!=_RangeList.end(); ++irl )
	{
		if ( (*irl).OwnerServiceId == ownerServiceId )
		{
			_RangeList.erase( irl );
			return true;
		}
	}
	return false;
}


/*
 * Release all acquired ranges
 */
void				CRangeList::releaseAllRanges()
{
	_RangeList.clear();
	acquireFirstRow();
}


/*
 * Release ranges by MS
 */
void				CRangeList::releaseRangesByMS( NLNET::TServiceId msId, std::vector<TRowRange>& erasedRanges )
{
	TRangeList::iterator irl;
	for ( irl=_RangeList.begin(); irl!=_RangeList.end(); )
	{
		if ( (*irl).MirrorServiceId == msId )
		{
			nlinfo( "RangeMirrorManager: releasing range for %hu (MS-%hu down)", (*irl).OwnerServiceId.get(), msId.get() );
			erasedRanges.push_back( *irl );
			irl = _RangeList.erase( irl );
		}
		else
			++irl;
	}
}


/*
 * Release the ranges for services that are not alive (return true if any)
 *
 * This does not work if a service has been shutdown and restarted (or another
 * service got its service id). That's why this method is not used!
 */
/*bool				CRangeList::releaseRangesForServicesDown()
{
	bool b = false;
	list<TRowRange>::iterator irr;
	for ( irr=_RangeList.begin(); irr!=_RangeList.end(); )
	{
		if ( CUnifiedNetwork::getInstance()->getServiceName( (*irr).OwnerServiceId ).empty() ) // TEMP
		{
			nldebug( "RangeMirrorManager: releasing range for %hu because service down", (*irr).OwnerServiceId );
			irr = _RangeList.erase( irr );
			b = true;
		}
		else
		{
			++irr;
		}
	}
	return b;
}*/


/*
 * Display the ranges
 */
void				CRangeList::displayRanges( NLMISC::CLog *log )
{
	TRangeList::iterator irl;
	for ( irl=_RangeList.begin(); irl!=_RangeList.end(); ++irl )
	{
		log->displayNL( "\tFrom %d to %d: owned by %hu", (*irl).First, (*irl).Last, (*irl).OwnerServiceId.get() );
	}
}


NLMISC_COMMAND( resetRanges, "RMM: Release all the acquired ranges", "" )
{
	RMMInstance->releaseAllRanges();
	return true;
}


NLMISC_COMMAND( releaseRangesForService, "RMM: Release range owned by a specified service", "<serviceId>" )
{
	if ( args.size() < 1 )
		return false;
	uint16 serviceId;
	NLMISC::fromString(args[0], serviceId);

	RMMInstance->releaseRangesByService( NLNET::TServiceId(serviceId) );
	return true;
}

NLMISC_COMMAND( displayRanges, "RMM: Display the acquired rangesd", "" )
{
	RMMInstance->displayRanges( &log );
	return true;
}
