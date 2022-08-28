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

//nel
#include "nel/misc/command.h"

// game share
#include "game_share/utils.h"

// local
#include "rfr_ryzom_file_retriever.h"
#include "remote_saves_interface.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace SAVES;


//-----------------------------------------------------------------------------
// some handy globals
//-----------------------------------------------------------------------------

static NLMISC::CSString WorkDirectory="./";


//-----------------------------------------------------------------------------
// some handy utility routines
//-----------------------------------------------------------------------------

static NLMISC::CSString buildRemoteSavesInterfaceSumary(CRemoteSavesInterface* saves)
{
	BOMB_IF(saves==NULL,"BUG: buildRemoteSavesInterfaceSumary() called with NULL pointer",return "");

	NLMISC::CSString txt="(";
	if (saves->isReady())
	{
		CFileDescriptionContainer fdc;
		saves->getFileList(fdc);
		txt+= NLMISC::toString("FILES: %u,",fdc.size());
	}
	else
	{
		txt+= "NOT READY,";
	}
	CRemoteSavesInterface::TCallbackSet cbs;
	saves->getCallbacks(cbs);
	txt+= NLMISC::toString("USES: %d)",cbs.size());

	return txt;
}


static void displayCharacterFileList(uint32 account, uint32 slot,NLMISC::CLog& log)
{
	NLMISC::CSString charIdString= NLMISC::toString("_%d_%d",account,slot);

	// get hold of a pointer to the file retriever singletgon
	CRyzomFileRetriever* rfr= CRyzomFileRetriever::getInstance();

	// setup a little set to make seraching easier later
	std::set<NLMISC::CSString> shardSet;

	// iterate over the used shard list
	CVectorSString usedShardList= rfr->getUsedShardList();
	for (uint32 i=0;i<usedShardList.size();++i)
	{
		CShardSavesInterface* shardSaves= rfr->getShardSavesInterface(usedShardList[i]);
		DROP_IF(shardSaves== NULL,"BUG: getShardSavesInterface() returned NULL for shard: "+usedShardList[i],return);

		// get the list of character save files
		CFileDescriptionContainer charFiles;
		shardSaves->getCharacterFileList(charFiles);
		for (uint32 j=0;j<charFiles.size();++j)
		{
			if (charFiles[j].FileName.contains(charIdString.c_str()))
			{
				log.displayNL("Shard %s: File: %s",usedShardList[i].c_str(),charFiles[j].toString().c_str());
			}
		}

		// get the list of files for items for sale in the sale store
		CFileDescriptionContainer saleStoreFiles;
		shardSaves->getSaleStoreFileList(saleStoreFiles);
		for (uint32 i=0;i<saleStoreFiles.size();++i)
		{
			if (saleStoreFiles[i].FileName.contains(charIdString.c_str()))
			{
				log.displayNL("Sale Store File: %s",saleStoreFiles[i].toString().c_str());
			}
		}

		// get the list of files for offline character commands
		CFileDescriptionContainer offlineCommandFiles;
		shardSaves->getOfflineCharacterCommandsFileList(offlineCommandFiles);
		for (uint32 i=0;i<offlineCommandFiles.size();++i)
		{
			if (offlineCommandFiles[i].FileName.contains(charIdString.c_str()))
			{
				log.displayNL("Offline Commands File: %s",offlineCommandFiles[i].toString().c_str());
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Ryzom File Retriever Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrListShards,"list the shard save module we're trying to attach to, attching to and ignoring","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	// get hold of a pointer to the file retriever singletgon
	CRyzomFileRetriever* rfr= CRyzomFileRetriever::getInstance();

	// setup a little set to make seraching easier later
	std::set<NLMISC::CSString> shardSet;

	// iterate over the used shard list
	CVectorSString usedShardList= rfr->getUsedShardList();
	for (uint32 i=0;i<usedShardList.size();++i)
	{
		// setup the description text for this shard
		CSString txt= usedShardList[i]+":";

		// add a clause to the description text foe the shard saves
		txt+=" shard"+buildRemoteSavesInterfaceSumary(rfr->getShardSavesInterface(usedShardList[i]));

		// add a clause to the description text foe the www saves
		txt+=" www"+buildRemoteSavesInterfaceSumary(rfr->getWwwSavesInterface(usedShardList[i]));

		// add a clause to the description text foe the incremental backups
		txt+=" bak"+buildRemoteSavesInterfaceSumary(rfr->getBakSavesInterface(usedShardList[i]));

		// display the text
		log.displayNL("Used remote shard: %s",txt.c_str());

		// add these shard save components to the set of 'used interfaces'
		shardSet.insert(usedShardList[i]);
	}

	// allocate a map to hold the list of modules found that don't correspond to used shards
	typedef std::map<NLMISC::CSString,NLMISC::CSString> TNewShards;
	TNewShards newShards;

	// iterate over the remote saves modules reported by the rfr singleton looking for modules that
	// aren't present in the used shards list
	CVectorSString detectedModules=	rfr->getSavesModules();
	for (uint32 i=0;i<detectedModules.size();++i)
	{
		NLMISC::CSString name= detectedModules[i].firstWord().strip();
		if (shardSet.find(name)==shardSet.end())
		{
			NLMISC::CSString type= detectedModules[i].word(1).strip();
			if (!newShards[name.strip()].empty()) newShards[name.strip()]+=' ';
			newShards[name.strip()]+=type;
		}
	}

	// display the list of new shards (regrouped by shard)
	for (TNewShards::iterator it= newShards.begin(); it!=newShards.end(); ++it)
	{
		log.displayNL("Unused remote shard: %s (%s)",(*it).first.c_str(),(*it).second.c_str());
	}

	return true;
}


NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrUseShard,"use shard save modules relating to a given shard name","<shard name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	CRyzomFileRetriever::getInstance()->useShard(args[0]);

	return true;
}


NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrStopUsingShard,"stop using shard save modules relating to a given shard name","<shard name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	CRyzomFileRetriever::getInstance()->stopUsingShard(args[0]);

	return true;
}


NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrListCharacterFiles,"display a list of the files belonging to a given character","<account_id> <slot>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	// convert args to numbers and check that they are valid
	uint32 account= NLMISC::CSString(args[0]).atoui();
	uint32 slot= NLMISC::CSString(args[1]).atoui();
	DROP_IF(account==0 && args[0]!="0","account Id is not a valid unsigned number: "+args[0],return false);
	DROP_IF(slot==0 && args[1]!="0","slot Id is not a valid unsigned number: "+args[1],return false);
	DROP_IF(slot>15,"slot Id is not valid (should be <=15): "+args[1],return false);

	// do the displaying
	displayCharacterFileList(account,slot,log);
	return true;
}


NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrListNamedCharacterFiles,"display a list of the files belonging to a given character","<shard name> <character_name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	uint32 account=0;
	uint32 slot=0;

	// look up character name and ensure that it exists
	bool found= CRyzomFileRetriever::getInstance()->getCharIdFromName(args[0],args[1],account,slot);
	DROP_IF(found==false,"named character not found: "+args[1]+" on shard: "+args[0],return false);

	// do the displaying
	displayCharacterFileList(account,slot,log);
	return true;
}


NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrListAccountFiles,"display a list of the files belonging to a given player","<account_id>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	// make sure the account number variable is valid
	uint32 account= NLMISC::CSString(args[0]).atoui();
	DROP_IF(account==0 && args[0]!="0","account Id is not a valid unsigned number: "+args[0],return false);

	// display the files for each possible slot
	for (uint32 i=0;i<16;++i)
	{
		displayCharacterFileList(account,i,log);
	}

	return true;
}


NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrListNamedAccountFiles,"display a list of the files belonging to a given player","<account_name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	// see whether we have an account name that matches
	uint32 account=0;
	bool found= CRyzomFileRetriever::getInstance()->getAccountIdFromName(args[0],account);
	DROP_IF(found==false,"named account not found: "+args[0],return false);

	// provoke execution of another NLMISC command to do the work
	NLMISC::ICommand::execute(NLMISC::toString("rfrListAccountFiles %u",account),log,quiet,human);

	return true;
}


NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrWorkDirectory,"get or set the directory that files are downloaded to or uploaded from","[<path>]")
{
	CNLSmartLogOverride logOverride(&log);

	switch (args.size())
	{
	case 0:
		log.displayNL("rfr work directory: %s",WorkDirectory.c_str());
		return true;

	case 1:
		{
			// setup the global to point to the new directory
			WorkDirectory= NLMISC::CPath::standardizePath(args[0]);

			// if the directory dosn't exist then create it
			if (!NLMISC::CFile::isDirectory(WorkDirectory))
			{
				NLMISC::CFile::createDirectoryTree(WorkDirectory);
			}
		}
		return true;
	}

	return false;
}


NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrGetCharacterFile,"download a character file","<shard> (<account_id> <slot> | <character_name>)")
{
	CNLSmartLogOverride logOverride(&log);

	uint32 account=0;
	uint32 slot=0;

	switch (args.size())
	{
	case 2:
		{
			// single argument after shard name must be a character name so look it up and ensure that it exists
			bool found= CRyzomFileRetriever::getInstance()->getCharIdFromName(args[0],args[1],account,slot);
			DROP_IF(found==false,"named character not found: "+args[1],return false);
		}
		break;

	case 3:
		// 2 args after shard name must be <account> <slot> so convert to numbers and check that they are valid
		account= NLMISC::CSString(args[1]).atoui();
		slot= NLMISC::CSString(args[2]).atoui();
		DROP_IF(account==0 && args[1]!="0","account Id is not a valid unsigned number: "+args[1],return false);
		DROP_IF(slot==0 && args[2]!="0","slot Id is not a valid unsigned number: "+args[2],return false);
		DROP_IF(slot>15,"slot Id is not valid (should be <=15): "+args[2],return false);
		break;

	default:
		return false;
	}

	// compose local and remote file names and queue up the upload
	NLMISC::CSString remoteFileName= CShardSavesInterface::getCharacterSaveFileName(account,slot);
	NLMISC::CSString localFileName= WorkDirectory+NLMISC::CFile::getFilename(remoteFileName);
	return CRyzomFileRetriever::getInstance()->downloadFile(args[0],remoteFileName,localFileName);
}

NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrGetOldCharacterFileSet,"download the complete set of backups for a given character","<shard> (<account_id> <slot> | <character_name>)")
{
	CNLSmartLogOverride logOverride(&log);

	uint32 account=0;
	uint32 slot=0;

	switch (args.size())
	{
	case 2:
		{
			// single argument after shard name must be a character name so look it up and ensure that it exists
			bool found= CRyzomFileRetriever::getInstance()->getCharIdFromName(args[0],args[1],account,slot);
			DROP_IF(found==false,"named character not found: "+args[1],return false);
		}
		break;

	case 3:
		// 2 args after shard name must be <account> <slot> so convert to numbers and check that they are valid
		account= NLMISC::CSString(args[1]).atoui();
		slot= NLMISC::CSString(args[2]).atoui();
		DROP_IF(account==0 && args[1]!="0","account Id is not a valid unsigned number: "+args[1],return false);
		DROP_IF(slot==0 && args[2]!="0","slot Id is not a valid unsigned number: "+args[2],return false);
		DROP_IF(slot>15,"slot Id is not valid (should be <=15): "+args[2],return false);
		break;

	default:
		return false;
	}

	// compose local and remote file names and queue up the upload
	NLMISC::CSString remoteFileName= CShardSavesInterface::getCharacterSaveFileName(account,slot);
	NLMISC::CSString localFileName= WorkDirectory+NLMISC::CFile::getFilename(remoteFileName);
	return CRyzomFileRetriever::getInstance()->downloadBackupFiles(args[0],remoteFileName,WorkDirectory);
}

NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrUploadFile,"upload a charcter save file (or other file)","<shard> <file name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	// extract the file name from the full path supplied
	NLMISC::CSString fileName= NLMISC::CFile::getFilename(args[1]);

	// see if we have a character save file
	if (fileName.left(8)=="account_" && fileName.right(8)=="_pdr.bin")
	{
		nlinfo("Uploading a character file: %s => %s",args[1].c_str(),(WorkDirectory+fileName).c_str());
		return true;
	}

	WARN("Failed to upload file due to unrecognised file type: "<<args[1].c_str());
	return false;
}

NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrListRootFiles,"list the core files for each shard","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	// get hold of a pointer to the file retriever singletgon
	CRyzomFileRetriever* rfr= CRyzomFileRetriever::getInstance();

	// iterate over the used shard list
	CVectorSString usedShardList= rfr->getUsedShardList();
	for (uint32 i=0;i<usedShardList.size();++i)
	{
		// display the shard name
		log.displayNL("Shard: %s",usedShardList[i].c_str());

		// if the shard's save file interface is ready for use then interrogate it...
		CShardSavesInterface* shardSaves= rfr->getShardSavesInterface(usedShardList[i]);
		DROP_IF(shardSaves== NULL,"BUG: getShardSavesInterface() returned NULL for shard: "+usedShardList[i],return false);
		if (shardSaves->isReady())
		{
			// get hold of the shard's file list
			CFileDescriptionContainer fdc;
			shardSaves->getFileList(fdc);

			// iterate over the shard's file list looking for the key files that interest us
			for (uint32 j=0;j<fdc.size();++j)
			{
				NLMISC::CSString fileName= fdc[j].FileName;
				if (fileName.left(2)=="./")
					fileName= fileName.leftCrop(2);

				if (fileName== shardSaves->getAccountNamesFileName()
				||  fileName== shardSaves->getCharacterNamesFileName()
				||  fileName== shardSaves->getGameCycleFileName()
				||  fileName== shardSaves->getGMPendingTPFileName())
				{
					log.displayNL("\t%s",fdc[j].toString().c_str());
				}
			}
		}
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrListGuildFiles,"list the guild files for a given shard","<shard name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	// get hold of a pointer to the file retriever singletgon
	CRyzomFileRetriever* rfr= CRyzomFileRetriever::getInstance();

	// get a pointer to the shard saves interface and make sure that it's ready for use
	CShardSavesInterface* shardSaves= rfr->getShardSavesInterface(args[0]);
	DROP_IF(shardSaves== NULL,"shard not found: "+args[0],return false);
	DROP_IF(!shardSaves->isReady(),"remote shard saves manager not ready: "+args[0],return true);

	// get the list of guild save files
	CFileDescriptionContainer fdc;
	shardSaves->getGuildFileList(fdc);

	// build a list of file descriptions to display
	CVectorSString files;
	for (uint32 i=0;i<fdc.size();++i)
	{
		NLMISC::CSString fileName= NLMISC::CFile::getFilename(fdc[i].FileName);
		if (fileName.left(6)=="guild_" && fileName.right(4)==".bin")
		{
			files.push_back(fdc[i].toString());
		}
	}

	// sort the file list
	std::sort(files.begin(),files.end());

	// display the sorted file list
	log.displayNL("Shard: %s",args[0].c_str());
	for (uint32 i=0;i<files.size();++i)
	{
		log.displayNL("\t%s",files[i].c_str());
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrListMailForumFiles,"list the mail / forum files for a given shard","<shard name> <guild or character name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	// get hold of a pointer to the file retriever singletgon
	CRyzomFileRetriever* rfr= CRyzomFileRetriever::getInstance();

	log.displayNL("Mail / Forum files from shard: %s for character / guild: %s",args[0].c_str(),args[0].c_str());

	// get a pointer to the shard's mail and forum interface and make sure it's ready for use
	CMailSavesInterface* mailSaves= rfr->getWwwSavesInterface(args[0]);
	DROP_IF(mailSaves==NULL,"shard not found: "+args[0],return false);
	DROP_IF(!mailSaves->isReady(),"remote shard saves manager not ready: "+args[0],return true);

	// get the list of mail and forum save files for the given entity
	CFileDescriptionContainer fdc;
	mailSaves->getEntityFileList(args[1],fdc);

	// display the files
	for (uint32 i=0;i<fdc.size();++i)
	{
		log.displayNL("\t%s",fdc[i].toString().c_str());
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(RyzomFileRetriever,rfrMoveMailForumFiles,"list the mail / forum files for a given character or guild on a given shard","<shard name> <old guild or character name> <new guild or character name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=3)
		return false;

	// get hold of a pointer to the file retriever singletgon
	CRyzomFileRetriever* rfr= CRyzomFileRetriever::getInstance();

	// get a pointer to the shard's mail and forum interface and make sure it's ready for use
	CMailSavesInterface* mailSaves= rfr->getWwwSavesInterface(args[0]);
	DROP_IF(mailSaves==NULL,"shard not found: "+args[0],return false);
	DROP_IF(!mailSaves->isReady(),"remote shard saves manager not ready: "+args[0],return true);

	// get the list of mail and forum save files for the source and destination entity names
	CFileDescriptionContainer fdcSrc, fdcDest;
	mailSaves->getEntityFileList(args[1],fdcSrc);
	mailSaves->getEntityFileList(args[2],fdcDest);

	// ensure that no destination files exist and that aat least some source files exist
	DROP_IF(!fdcDest.empty(),"FAILED to move mail / forum files because the destination directory is not empty: move files away first: "+args[2],return true);
	DROP_IF(fdcSrc.empty(),"FAILED to move mail / forum files because the source directory is empty: "+args[1],return true);

	// do the moving
	mailSaves->moveEntityFiles(args[1],args[2],false);

	return true;
}


//-----------------------------------------------------------------------------
