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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
#include "outpost_manager.h"

#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild_member_module.h"

#include "primitives_parser.h"
#include "egs_sheets/egs_sheets.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;


//-----------------------------------------------------------------------------
// vars
//-----------------------------------------------------------------------------

//COutpostManager* COutpostManager::_Instance = NULL;

extern NLMISC::CVariable<bool>    		UseBS;
CVariable<bool> LoadOutposts("egs", "LoadOutposts", "If false outposts won't be loaded", true, 0, true );

CFileDisplayer OutpostDisplayer("outposts.log");
CLog OutpostDbgLog(CLog::LOG_DEBUG), OutpostInfLog(CLog::LOG_INFO), OutpostWrnLog(CLog::LOG_WARNING), OutpostErrLog(CLog::LOG_ERROR);


//----------------------------------------------------------------------------
// AIS callbacks
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
static void cbOutpostSquadCreated( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	COutpostSquadCreatedMsg params;
	msgin.serial(params);
	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias(params.Outpost);
	if (outpost)
	{
		outpost->aieventSquadCreated(params.CreateOrder, params.GroupId);
	}
}

//----------------------------------------------------------------------------
static void cbOutpostSquadSpawned( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	COutpostSquadSpawnedMsg params;
	msgin.serial(params);
	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias(params.Outpost);
	if (outpost)
	{
		outpost->aieventSquadSpawned(params.GroupId);
	}
}

//----------------------------------------------------------------------------
static void cbOutpostSquadDespawned( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	COutpostSquadDespawnedMsg params;
	msgin.serial(params);
	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias(params.Outpost);
	if (outpost)
	{
		outpost->aieventSquadDespawned(params.GroupId);
	}
}

//----------------------------------------------------------------------------
static void cbOutpostSquadDied( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	COutpostSquadDiedMsg params;
	msgin.serial(params);
	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias(params.Outpost);
	if (outpost)
	{
		outpost->aieventSquadDied(params.GroupId);
	}
}

//----------------------------------------------------------------------------
static void cbOutpostSquadLeaderDied( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	COutpostSquadLeaderDiedMsg params;
	msgin.serial(params);
	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias(params.Outpost);
	if (outpost)
	{
		outpost->aieventSquadLeaderDied(params.GroupId);
	}
}


//----------------------------------------------------------------------------
// COutpostManager methods
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
COutpostManager::COutpostManager()
{
	_NextOutpostToSave = 0;
	_OutpostUpdateCursor = 0;
	_OutpostPrimitivesLoaded = false;
	_OutpostSaveFilesLoaded = false;

	OutpostDbgLog.addDisplayer( &OutpostDisplayer );
	OutpostInfLog.addDisplayer( &OutpostDisplayer );
	OutpostWrnLog.addDisplayer( &OutpostDisplayer );
	OutpostErrLog.addDisplayer( &OutpostDisplayer );
}

//----------------------------------------------------------------------------
inline std::string readProperty(
	const NLLIGO::IPrimitive* node, const char *propertyName,
	const char *parentClass, const std::string& parentName, bool allowEmpty=false, bool assertIt=false, bool allowNotFound=false )
{
	string value;
	bool res = node->getPropertyByName( propertyName, value );
	if ( (! allowNotFound) && (! (res && (allowEmpty || (! value.empty())))) )
	{
		OUTPOST_WRN( "Can't find '%s' in %s %s", propertyName, parentClass, parentName.c_str() );
		if ( assertIt )
			nlerror ( "Error in primitive, please read outposts.log" );
	}
	return value;
}

//----------------------------------------------------------------------------
void COutpostManager::loadOutpostPrimitives()
{
	if (!LoadOutposts.get())
		return;

	// only once
	if (_OutpostPrimitivesLoaded)
		return;

	const CPrimitivesParser::TPrimitivesList & primsList = CPrimitivesParser::getInstance().getPrimitives();
	string value;
	CPrimitivesParser::TPrimitivesList::const_iterator first, last;

	OUTPOST_DBG("Loading squad templates...");
	// *** Squad templates ***
	// get the primitive list and parse each primitive
	for ( first = primsList.begin(), last = primsList.end(); first != last; ++first )
	{
		// get the squad_template nodes
		for ( uint i = 0; i < first->Primitive.RootNode->getNumChildren(); ++i )	
		{
			const NLLIGO::IPrimitive* stNode = NULL;
			if ( first->Primitive.RootNode->getChild( stNode, i ) && stNode != NULL )
			{
				const char *parentClass = "squad_template";
				if (stNode->getPropertyByName("class",value) && !nlstricmp(value.c_str(),parentClass) )
				{
					string path = first->FileName;
					CSquadTemplate squadTemplate;
					squadTemplate.SquadName = readProperty( stNode, "name", parentClass, path, false, true );
					
					// get the squad_template variants
					for ( uint j = 0; j < stNode->getNumChildren(); ++j )	
					{
						const NLLIGO::IPrimitive* stvNode = NULL;
						if ( stNode->getChild( stvNode, j ) && stvNode != NULL )
						{
							parentClass = "squad_template_variant";
							if (stvNode->getPropertyByName("class",value) && !nlstricmp(value.c_str(),parentClass) )
							{
								path = first->FileName+"."+squadTemplate.SquadName;
								CSquadTemplate::CSquadTemplateVariant variant;

								// alias
								variant.Alias = 0;
								nlverify( CPrimitivesParser::getAlias(stvNode, variant.Alias) );

								// 'name' -> continent or tribe name
								variant.VariantName = readProperty( stvNode, "name", parentClass, path, false, true );
								path += "."+variant.VariantName;

								// 'is tribe'
								value = readProperty( stvNode, "is tribe", parentClass, path, true, false, true );
								variant.IsTribe = (value == "true");

								// 'squad sheet'
								value = readProperty( stvNode, "squad sheet", parentClass, path );
								if ( value.find( ".outpost_squad" ) == string::npos )
									value += ".outpost_squad";
								variant.SquadSheet = CSheetId(value);
								if ( variant.SquadSheet == CSheetId::Unknown )
									OUTPOST_WRN("PRIM_ERROR : invalid squad sheet '%s' (not in sheet_id.bin) in '%s'", value.c_str(), path.c_str() );
								//if ( ! CSheets::getOutpostSquadForm( variant.SquadSheet ) ) // already done when loading outpost
								//	OUTPOST_WRN("PRIM_ERROR : invalid squad sheet '%s' (error in georges form) in %s", value.c_str(), path.c_str() );

								// squad members are handled by the AIS
								if ( stvNode->getNumChildren() == 0 )
									OUTPOST_WRN("PRIM_ERROR : '%s' in '%s' has no members", parentClass, path.c_str() );

								squadTemplate.Variants.push_back( variant );
							}
						}
					}
					{
						std::string str;
						str += "Adding Squad template '" + squadTemplate.SquadName + "' (";
						std::vector<CSquadTemplate::CSquadTemplateVariant>::const_iterator it, itEnd;
						it=squadTemplate.Variants.begin(), itEnd=squadTemplate.Variants.end();
						if (it!=itEnd)
							str += it->VariantName;
						for (++it ; it!=itEnd; ++it)
							str += ", " + it->VariantName;
						str += ")";
						OUTPOST_DBG("%s", str.c_str());
					}
					addSquadTemplate( squadTemplate );
				}
			}
		}
	}
	OUTPOST_DBG("Squad templates loaded.");
	
	OUTPOST_DBG("Loading outposts...");
	// *** Outposts (must be done after squad templates) ***
	// get the primitive list and parse each primitive
	for ( first = primsList.begin(), last = primsList.end(); first != last; ++first )
	{
		// get the dynamic_system nodes
		for ( uint i = 0; i < first->Primitive.RootNode->getNumChildren(); ++i )	
		{
			const NLLIGO::IPrimitive* dynSystemNode = NULL;
			if ( first->Primitive.RootNode->getChild( dynSystemNode,i ) && dynSystemNode != NULL )
			{
				if (dynSystemNode->getPropertyByName("class",value) && !nlstricmp(value.c_str(),"dynamic_system") )
				{
					// get the continent, as outpost need a continent to be built. We get this value from the dynamic system node
					string dynSystemName;
					nlverify( dynSystemNode->getPropertyByName("name",dynSystemName) );
					nlverify( dynSystemNode->getPropertyByName("continent_name",value) );
					CONTINENT::TContinent continent = CONTINENT::toContinent( value );
					if ( continent == CONTINENT::UNKNOWN )
					{
						OUTPOST_WRN("Invalid continent '%s' in dynamic system '%s' of primitive file '%s'", 
						value.c_str(), dynSystemName.c_str(), first->FileName.c_str() );
						break;
					}
					// parse all the outpost within this dynamic system.
					for ( uint j = 0; j < dynSystemNode->getNumChildren(); ++j )	
					{
						const NLLIGO::IPrimitive* outpostNode = NULL;
						if ( dynSystemNode->getChild( outpostNode,j ) && outpostNode != NULL )
						{
							if (outpostNode->getPropertyByName("class",value) && !nlstricmp(value.c_str(),"outpost") )
							{
								CSmartPtr<COutpost> outpost = CSmartPtr<COutpost>(new COutpost());
								if ( outpost->build( outpostNode, first->FileName,dynSystemName, continent ) )
								{
									bool multiple = false;
									for (vector<NLMISC::CSmartPtr<COutpost> >::const_iterator it = _Outposts.begin(); it != _Outposts.end(); ++it)
									{
										if ( (*it)->getAlias() == outpost->getAlias() )
										{
											OUTPOST_WRN( "PRIM_ERROR : multiple outpost alias %s %s %s", CPrimitivesParser::aliasToString( outpost->getAlias() ).c_str(), (*it)->getName().c_str(), outpost->getName().c_str() );
											multiple = true;
										}
										if ( (*it)->getSheet() == outpost->getSheet() )
										{
											OUTPOST_WRN( "PRIM_ERROR : multiple outpost sheet %s %s %s", outpost->getSheet().toString().c_str(), (*it)->getName().c_str(), outpost->getName().c_str() );
											multiple = true;
										}
									}
									if( !multiple )
									{
										_Outposts.push_back(outpost);
										_OutpostsByAlias.insert( make_pair( outpost->getAlias(), outpost ) );
										_OutpostsBySheet.insert( make_pair( outpost->getSheet(), outpost ) );
										OUTPOST_INF("Outpost '%s' was successfully parsed", outpost->getName().c_str());
									
										// nb : short id starts at 1, 0 is used for invalid
										_OutpostAliasToShortId.insert( make_pair( outpost->getAlias(), (uint16)_Outposts.size()) );
									}
								}
							}
						}
					}
				}
			}
		}
	}
	OUTPOST_DBG("Outposts loaded.");
	
	OUTPOST_DBG("Clearing squad templates...");
	// No more need of squad templates
	_SquadTemplates.clear();
	OUTPOST_DBG("Squad templates cleared.");

	// outpost primitives now are loaded
	_OutpostPrimitivesLoaded = true;
}


vector<string>	OutpostFiles;

struct TOupostFileClassCallback : public IBackupFileClassReceiveCallback
{
	virtual void callback(const CFileDescriptionContainer& fileList)
	{
		// store the list of oupost file
		for (uint i=0; i<fileList.size(); ++i)
			OutpostFiles.push_back(fileList[i].FileName);
	}
};

struct TOupostFileCallback : public IBackupFileReceiveCallback
{
	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		// forward to outpost manager
		COutpostManager::getInstance().outpostFileCallback(fileDescription, dataStream);
	}
};


vector<COutpost*> loadedOutposts;

void COutpostManager::outpostFileCallback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
{
	// get the corresponding outpost
	string aliasStr = CFile::getFilename(fileDescription.FileName);
	aliasStr.resize( aliasStr.length() - 4 );
	aliasStr = aliasStr.substr( 8);

	TAIAlias alias;
	NLMISC::fromString(aliasStr, alias);
	COutpost * outpost = getOutpostFromAlias( alias );
	if ( !outpost )
		OUTPOST_WRN("Invalid outpost file '%s' %s : not found in the primitive", fileDescription.FileName.c_str(), CPrimitivesParser::aliasToString(alias).c_str());
	else
	{
		// load dynamic data
		static CPersistentDataRecord	pdr;
		pdr.clear();
		pdr.fromBuffer(dataStream);
		outpost->apply( pdr );
		loadedOutposts.push_back( outpost );
	}
}

//----------------------------------------------------------------------------
void COutpostManager::loadOutpostSaveFiles()
{
	if (!LoadOutposts.get())
		return;

	// only once
	if (_OutpostSaveFilesLoaded)
		return;

	// check preconditions
	nlassert(_OutpostPrimitivesLoaded);
	nlassert(CGuildManager::getInstance()->loadedGuilds());

	OUTPOST_DBG("Applying outpost save files...");
	// all outposts where parsed. Let's try to load their saved files to init the dynamic data.
	string path = /*Bsi.getLocalPath() +*/ string("outposts");
	// create the saved file directory, if needed
//	CFile::createDirectory(path);
	
	// get all the outpost files
	TOupostFileClassCallback *ccb = new TOupostFileClassCallback;
	OutpostFiles.clear();
	vector<CBackupFileClass> outpostClasses(1);
	outpostClasses[0].Patterns.push_back("outpost*.bin");
	Bsi.syncLoadFileClass("outposts", outpostClasses, ccb);

	// load all the outpost file
	loadedOutposts.clear();
	TOupostFileCallback *cb = new TOupostFileCallback;
	Bsi.syncLoadFiles(OutpostFiles, cb);

//	CPath::getPathContent (path, false, false, true,files ,NULL);

//	vector<COutpost*> loadedOutposts;
//	for (uint i = 0; i < files.size(); i++)
//	{
//		string file = files[i].substr( path.size() + 1);
//		// check if we have an outpost file
//		if ( file.find( "outpost_" ) == 0 &&
//			file.find( ".bin" ) == file.length() - 4 )
//		{
//			// get the corresponding outpost
//			string aliasStr = file;
//			aliasStr.resize( file.length() - 4 );
//			aliasStr = aliasStr.substr( 8, file.length() - 8 );
//
//			TAIAlias alias;
//			NLMISC::fromString(aliasStr, alias);
//			COutpost * outpost = getOutpostFromAlias( alias );
//			if ( !outpost )
//				OUTPOST_WRN("Invalid outpost file '%s' ('%u') : not found in the primitive", file.c_str(), alias);
//			else
//			{
//				// load dynamic data
//				static CPersistentDataRecord	pdr;
//				pdr.clear();
//				pdr.readFromBinFile(files[i].c_str());
//				outpost->apply( pdr );
//				loadedOutposts.push_back( outpost );
//			}
//		}
//	}
	OUTPOST_DBG("Outpost save files applied.");
	
	OUTPOST_DBG("Initializing unsaved outposts...");
	// Init new outpost that have never been saved
	uint nbNewOutposts = 0;
	for (vector<NLMISC::CSmartPtr<COutpost> >::const_iterator it = _Outposts.begin(); it != _Outposts.end(); ++it)
	{
		if ( std::find( loadedOutposts.begin(), loadedOutposts.end(), *it ) == loadedOutposts.end() )
		{
			COutpost *newOutpost = (*it);
			newOutpost->initNewOutpost();
			++nbNewOutposts;
		}
	}
	if ( nbNewOutposts != 0 )
	{
		OUTPOST_INF( "Created default squads for %u new outposts", nbNewOutposts );
	}
	OUTPOST_DBG("Unsaved outposts initialized.");

	OUTPOST_DBG("Registering outposts in guilds...");
	// register outposts in guilds
	for (uint i = 0; i < _Outposts.size(); i++)
	{
		_Outposts[i]->registerOutpostInGuilds();
	}
	OUTPOST_DBG("Outposts registered in guilds.");

	// array of callback items
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "OUTPOST_SQUAD_CREATED",		cbOutpostSquadCreated		},
		{ "OUTPOST_SQUAD_SPAWNED",		cbOutpostSquadSpawned		},
		{ "OUTPOST_SQUAD_DESPAWNED",	cbOutpostSquadDespawned		},
		{ "OUTPOST_SQUAD_DIED",			cbOutpostSquadDied			},
		{ "OUTPOST_SQUAD_LEADER_DIED",	cbOutpostSquadLeaderDied	},
	};
	// register call back for outpost manager
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );

	// outpost save files now are loaded
	_OutpostSaveFilesLoaded = true;

	// trigger pending ready AI instance events
	for (	std::set<uint32>::const_iterator it = _PendingReadyAIInstances.begin();
			it != _PendingReadyAIInstances.end();
			++it)
	{
		onAIInstanceReadyOrDown(*it, true);
	}
	_PendingReadyAIInstances.clear();
}

//----------------------------------------------------------------------------
void COutpostManager::validateOutpostOwners()
{
	TOutposts::iterator first(_Outposts.begin()), last(_Outposts.end());
	for (; first != last; ++first)
	{
		COutpost *outpost = *first;

		if (outpost->getOwnerGuild() != 0 && CGuildManager::getInstance()->getGuildFromId(outpost->getOwnerGuild()) == NULL)
		{
			nlwarning("validateOutpostOwners : the outpost '%s' owner guild %u no found, removing owner and attacker",
				outpost->getName().c_str(),
				outpost->getOwnerGuild());

			outpost->setOwnerGuild(0);
			outpost->setAttackerGuild(0);
		}

		if (outpost->getAttackerGuild() != 0 && CGuildManager::getInstance()->getGuildFromId(outpost->getAttackerGuild()) == NULL)
		{
			nlwarning("validateOutpostOwners : the outpost '%s' attacker guild %u no found, removing attacker",
				outpost->getName().c_str(),
				outpost->getAttackerGuild());

			outpost->setAttackerGuild(0);
		}
	}
}


//----------------------------------------------------------------------------
bool COutpostManager::fillSquadDescriptor( const std::string& squadName, bool isTribe, const std::string& variantContext, COutpostSquadDescriptor& squadDesc )
{
	// Allow bypassing variantContext if the variant is specified in squadName
	string squadMainName, squadVariant;
	string::size_type p = squadName.find( ':' );
	if ( p != string::npos )
	{
		squadMainName = squadName.substr( 0, p );
		squadVariant = squadName.substr( p+1 );
	}
	else
	{
		squadMainName = squadName;
		squadVariant = variantContext;
	}
	// Find and fill
	for( vector<CSquadTemplate>::const_iterator ist=_SquadTemplates.begin(); ist!=_SquadTemplates.end(); ++ist )
	{
		const CSquadTemplate& st = (*ist);
		if ( st.SquadName != squadMainName )
			continue;
		for ( vector<CSquadTemplate::CSquadTemplateVariant>::const_iterator itv=st.Variants.begin(); itv!=st.Variants.end(); ++itv )
		{
			const CSquadTemplate::CSquadTemplateVariant& tv = (*itv);
			if ( tv.IsTribe != isTribe )
				continue;
			if ( NLMISC::nlstricmp( tv.VariantName.c_str(), squadVariant.c_str() ) != 0 )
				continue;
			if (tv.SquadSheet==NLMISC::CSheetId::Unknown || CSheets::getOutpostSquadForm(tv.SquadSheet)==NULL)
				continue;
			squadDesc.init( tv.SquadSheet, tv.Alias, squadName );
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------
void COutpostManager::askOutpostGuildDBUpdate(TAIAlias outpostAlias, COutpostGuildDBUpdater::TDBPropSet dbPropSet)
{
	for (uint i = 0; i < _OutpostsWaitingGuildDBUpdate.size(); i++)
	{
		if (outpostAlias == _OutpostsWaitingGuildDBUpdate[i].first)
		{
			_OutpostsWaitingGuildDBUpdate[i].second |= dbPropSet;
			return;
		}
	}

	_OutpostsWaitingGuildDBUpdate.push_back( make_pair(outpostAlias, dbPropSet) );
}

//----------------------------------------------------------------------------
void COutpostManager::doOutpostGuildDBUpdates()
{
	for (uint i = 0; i < _OutpostsWaitingGuildDBUpdate.size(); i++)
	{
		TAIAlias outpostAlias = _OutpostsWaitingGuildDBUpdate[i].first;
		COutpostGuildDBUpdater::TDBPropSet dbPropSet = _OutpostsWaitingGuildDBUpdate[i].second;

		CHashMap<uint, NLMISC::CSmartPtr<COutpost> >::iterator it = _OutpostsByAlias.find(outpostAlias);
		if (it == _OutpostsByAlias.end())
			continue;

		NLMISC::CSmartPtr<COutpost> & outpost = (*it).second;

		// check data consistency
		if (outpost->isBelongingToAGuild() && outpost->getOwnerGuild() == outpost->getAttackerGuild())
		{
			nlwarning("Owner and attacker are the same guild (%u) for the outpost '%s'",
				outpost->getOwnerGuild(),
				outpost->getName().c_str()
				);
			DEBUG_STOP;
		}

		CGuild * ownerGuild = CGuildManager::getInstance()->getGuildFromId(outpost->getOwnerGuild());
		if (ownerGuild != NULL)
		{
			COutpostGuildDBUpdaterPtr dbUpdater = ownerGuild->getOutpostGuildDBUpdater(outpost);
			if (dbUpdater != NULL)
				dbUpdater->updateOutpostGuildDB(dbPropSet);
		}

		CGuild * attackerGuild = CGuildManager::getInstance()->getGuildFromId(outpost->getAttackerGuild());
		if (attackerGuild != NULL)
		{
			COutpostGuildDBUpdaterPtr dbUpdater = attackerGuild->getOutpostGuildDBUpdater(outpost);
			if (dbUpdater != NULL)
				dbUpdater->updateOutpostGuildDB(dbPropSet);
		}
	}

	_OutpostsWaitingGuildDBUpdate.clear();
}

//----------------------------------------------------------------------------
void COutpostManager::onAIInstanceReadyOrDown( uint32 instanceNumber, bool startOrStop )
{
	if (!_OutpostSaveFilesLoaded)
	{
		if (startOrStop)
		{
			_PendingReadyAIInstances.insert(instanceNumber);
			OUTPOST_DBG( "onAIInstanceReadyOrDown : added pending ready AI instance %u", instanceNumber);
		}
		else
		{
			_PendingReadyAIInstances.erase(instanceNumber);
			OUTPOST_DBG( "onAIInstanceReadyOrDown : removed pending ready AI instance %u", instanceNumber);
		}
		return;
	}

	OUTPOST_DBG( "onAIInstanceReadyOrDown %u : %s", instanceNumber, startOrStop ? "STARTED" : "STOPED" );

	for (uint i = 0; i < _Outposts.size(); i++)
	{
		NLMISC::CSmartPtr<COutpost> & outpost = _Outposts[i];
		if ( outpost->getAIInstanceNumber() == instanceNumber )
		{
			if ( startOrStop )
				outpost->aisUp();
			else
				outpost->aisDown();
		}
	}
}

//----------------------------------------------------------------------------
void COutpostManager::tickUpdate()
{
	H_AUTO(COutpostManager_tickUpdate);
	
	// do nothing until outpost save files are loaded
	if (!_OutpostSaveFilesLoaded)
		return;

	// do nothing if no outpost is loaded
	if (_Outposts.empty())
		return;

	// update outposts
	{
		H_AUTO(COutpostManagerUPDATE);
		
		if (OutpostUpdatePeriod.get() == 0)
			OutpostUpdatePeriod.set(1);
		
		if (OutpostUpdatePeriod.get() == 1)
		{
			// update every outposts each tick
			uint32 currentTime = CTime::getSecondsSince1970();
			for (uint32 i = 0; i < _Outposts.size(); i++)
			{
				_Outposts[i]->updateOutpost(currentTime);
			}
			
			if (_OutpostUpdateCursor != 0)
				_OutpostUpdateCursor = 0;
		}
		else
		{
			uint32 nbOutpost = (uint32)_Outposts.size();
			uint32 nbOutpostPerTick = (uint32)floor(double(nbOutpost) / double(OutpostUpdatePeriod.get()));
			nbOutpostPerTick = std::max(uint32(1), nbOutpostPerTick); // The strict minimum is a single outpost update per tick
			
			uint32 beginIndex = _OutpostUpdateCursor;
			nlassert(beginIndex < _Outposts.size());
			
			// move the cursor and roll it in valid outpost indexes range
			_OutpostUpdateCursor += nbOutpostPerTick;
			uint32 endIndex = _OutpostUpdateCursor;
			if (_OutpostUpdateCursor >= nbOutpost)
				_OutpostUpdateCursor -= nbOutpost;
			
			if (beginIndex != endIndex)
			{
				// update a range of outposts each tick
				uint32 currentTime = CTime::getSecondsSince1970();
				if (endIndex < _Outposts.size())
				{
					for (uint32 i = beginIndex; i < endIndex; i++)
						_Outposts[i]->updateOutpost(currentTime);
				}
				else
				{
					endIndex -= (uint32)_Outposts.size();
					for (uint32 i = beginIndex; i < _Outposts.size(); i++)
						_Outposts[i]->updateOutpost(currentTime);
					for (uint32 i = 0; i < endIndex; i++)
						_Outposts[i]->updateOutpost(currentTime);
				}
			}
		}
	}
	
	// do outpost guild database updates
	doOutpostGuildDBUpdates();
	
	// save 1 outpost
	if ( ( CTickEventHandler::getGameCycle() % OutpostSavingPeriod ) == 0 )
	{
		nlassert(_NextOutpostToSave < _Outposts.size());
		NLMISC::CSmartPtr<COutpost> & outpost = _Outposts[_NextOutpostToSave];
		nlassert(outpost != NULL);
		saveOutpost(outpost);

		_NextOutpostToSave++;
		if (_NextOutpostToSave >= _Outposts.size())
			_NextOutpostToSave = 0;
	}
}

//----------------------------------------------------------------------------
void COutpostManager::saveAll()
{
	H_AUTO(COutpostManager_saveAll);

	if (!_OutpostSaveFilesLoaded)
	{
		nldebug("No need to save outposts, they are not loaded yet!");
		return;
	}

	nldebug("Saving all outposts...");
	TTime startTime = NLMISC::CTime::getLocalTime();

	std::vector<NLMISC::CSmartPtr<COutpost> >::const_iterator it;
	for (it = _Outposts.begin(); it != _Outposts.end(); ++it)
	{
		const NLMISC::CSmartPtr<COutpost> & outpost = *it;
		nlassert(outpost != NULL);
		saveOutpost(outpost);
	}

	TTime endTime = NLMISC::CTime::getLocalTime();
	nldebug("Saved all outposts in %"NL_I64"d ms.", (endTime-startTime));
}

//----------------------------------------------------------------------------
void COutpostManager::saveOutpost(NLMISC::CSmartPtr<COutpost> outpost)
{
	nlassert(_OutpostSaveFilesLoaded);
	if (outpost == NULL)
		return;

	H_AUTO(COutpostManager_saveOutpost);

	static CPersistentDataRecordRyzomStore	pdr;
	pdr.clear();
	{
		H_AUTO(COutpostManagerSTORE)
		outpost->store(pdr);
	}
	string fileName = NLMISC::toString("outposts/outpost_%u.%s", outpost->getAlias(), (XMLSave)? "xml" : "bin" );
	if( UseBS )
	{
		try
		{
			CBackupMsgSaveFile msg( fileName, CBackupMsgSaveFile::SaveFile, Bsi );
			if (XMLSave)
			{
				H_AUTO(COutpostSerialXML);
				std::string s;
				pdr.toString(s);
				msg.DataMsg.serialBuffer((uint8*)&s[0], (uint)s.size());
			}
			else
			{
				H_AUTO(COutpostSerialBin);					
				uint32 bufSize= pdr.totalDataSize();
				vector<char> buffer;
				buffer.resize(bufSize);
				pdr.toBuffer(&buffer[0],bufSize);
				msg.DataMsg.serialBuffer((uint8*)&buffer[0], bufSize);
			}
			Bsi.sendFile( msg );
		}
		catch(const Exception &)
		{
			OUTPOST_WRN("Can't serial file %s",fileName.c_str());
			return;
		}
	}
	else
	{
		H_AUTO( COutpostManagerSTORE_NO_BS )
		pdr.writeToBinFile((Bsi.getLocalPath() + fileName).c_str());
	}
}

//----------------------------------------------------------------------------
//COutpostManager & COutpostManager::getInstance()
//{
//	if ( !_Instance )
//		_Instance = new COutpostManager;
//	return *_Instance;
//}

//----------------------------------------------------------------------------
NLMISC::CSmartPtr<COutpost> COutpostManager::getOutpostFromAlias( TAIAlias alias )
{
	CHashMap<uint, NLMISC::CSmartPtr<COutpost> >::iterator it = _OutpostsByAlias.find( alias );
	if ( it == _OutpostsByAlias.end() )
		return NULL;
	return (*it).second;
}

//----------------------------------------------------------------------------
NLMISC::CSmartPtr<COutpost> COutpostManager::getOutpostFromSheet( NLMISC::CSheetId sheet )
{
	std::map<NLMISC::CSheetId, NLMISC::CSmartPtr<COutpost> >::iterator it = _OutpostsBySheet.find( sheet );
	if ( it == _OutpostsBySheet.end() )
		return NULL;
	return (*it).second;
}

//----------------------------------------------------------------------------
uint16 COutpostManager::getOutpostShortId( TAIAlias alias )
{
	uint16 shortId = 0;

	map<TAIAlias,uint16>::iterator it = _OutpostAliasToShortId.find( alias );
	if( it != _OutpostAliasToShortId.end() )
	{
		shortId = (*it).second;
	}

	return shortId;
}

//----------------------------------------------------------------------------
TAIAlias COutpostManager::getOutpostAliasFromShortId( uint16 shortId )
{
	TAIAlias alias = 0;
	
	map<uint16,TAIAlias>::iterator it = _OutpostShortIdToAlias.find( shortId );
	if( it != _OutpostShortIdToAlias.end() )
	{
		alias = (*it).second;
	}
	
	return alias;
}

//----------------------------------------------------------------------------
void COutpostManager::onRemoveGuild( CGuild *guild )
{
	for (uint i = 0; i < _Outposts.size(); i++)
	{
		NLMISC::CSmartPtr<COutpost> & outpost = _Outposts[i];
		if ( outpost->getOwnerGuild() == guild->getId() )
		{
			outpost->ownerGuildVanished();
		}
		else if ( outpost->getAttackerGuild() == guild->getId() )
		{
			outpost->attackerGuildVanished();
		}
	}
}

//----------------------------------------------------------------------------
void COutpostManager::onBuildingSpawned(CCreature *pBuilding)
{
	for (uint i = 0; i < _Outposts.size(); i++)
	{
		CSmartPtr<COutpost> & outpost = _Outposts[i];
		if (outpost->onBuildingSpawned(pBuilding))
			break;
	}
}

//----------------------------------------------------------------------------
void COutpostManager::dumpOutpostList(NLMISC::CLog & log) const
{
	for (uint i = 0; i < _Outposts.size(); i++)
	{
		const CSmartPtr<COutpost> & outpost = _Outposts[i];
		if (outpost != NULL)
			log.displayNL("%u: %s", i, outpost->toString().c_str());
	}
}

//----------------------------------------------------------------------------
TAIAlias COutpostManager::getOutpostFromUserPosition( CCharacter *user ) const
{
	H_AUTO(COutpostManager_getOutpostFromUserPosition);

	CVector vect( float(user->getState().X)/1000.f, float(user->getState().Y)/1000.f, 0.f );

	for (uint i = 0; i < _Outposts.size(); i++)
	{
		COutpost * outpost = _Outposts[i];
		if( outpost->contains(vect, false) )
		{
			// Maybe we should check here that user is owner of the outpost
		//	if( outpost->isCharacterInConflict(user) )
		//	{
				return outpost->getAlias();
		//	}
		//	else
		//	{
		//		OUTPOST_DBG("<CPVPManager::getPVPZoneFromUserPosition> user not in conflict");
		//	}
		}
	}

	return CAIAliasTranslator::Invalid;
}

//----------------------------------------------------------------------------
void COutpostManager::enterOutpostZone(CCharacter* user)
{
	H_AUTO(COutpostManager_enterPVPZone);
	
	nlassert(user);
	
	COutpost* outpost = getOutpostFromAlias(user->getCurrentOutpostZone());
	if (outpost && outpost->getOwnerGuild()==user->getGuildId())
		PlayerManager.sendImpulseToClient(user->getId(), "GUILD:OPEN_INVENTORY");
}

//----------------------------------------------------------------------------
void COutpostManager::leaveOutpostZone(CCharacter* user)
{
	H_AUTO(COutpostManager_leavePVPZone);
	
	nlassert(user);
	
//	COutpost* outpost = getOutpostFromAlias(user->getCurrentOutpostZone());
//	if (outpost && outpost->getOwnerGuild()==user->getGuildId())
		PlayerManager.sendImpulseToClient(user->getId(), "GUILD:CLOSE_INVENTORY");
}

//----------------------------------------------------------------------------
void COutpostManager::setConstructionTime(uint32 nNbSeconds)
{
	H_AUTO(COutpostManager_setConstructionTime);

	uint32 currentTime = CTime::getSecondsSince1970();

	for (uint i = 0; i < _Outposts.size(); i++)
	{
		COutpost * outpost = _Outposts[i];
		outpost->setConstructionTime(nNbSeconds, currentTime);
	}
}

