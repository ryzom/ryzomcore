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

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/misc/entity_id.h"
#include "stat_guild_container.h"
#include "stat_globals.h"


//-------------------------------------------------------------------------------------------------
// handy utilities
//-------------------------------------------------------------------------------------------------

NLMISC::CSString crunchUserId(const NLMISC::CSString& numericUserId)
{
	uint64 num=0;
	for (uint32 i=0;i<numericUserId.size();++i)
	{
		uint32 digit= (numericUserId[i]-'0');
		DROP_IF(digit>9,"invalid number for account id: "+numericUserId,return "-1");
		num=num*10+digit;
	}

	NLMISC::CEntityId eid(num);
	uint32 account=	(uint32)(eid.getShortId()/16);
	uint32 slot=	(uint32)(eid.getShortId()%16);
	return NLMISC::toString("%s %s (%s %d %d)",
		eid.toString().c_str(),
		STAT_GLOBALS::getCharacterName("default",account,slot).c_str(),
		STAT_GLOBALS::getAccountName(account).c_str(),
		account,slot);
}

//-------------------------------------------------------------------------------------------------
// methods CStatGuildContainer
//-------------------------------------------------------------------------------------------------

CStatGuildContainer::CStatGuildContainer()
{
	_InProgress= false;
}

CStatGuildContainer::~CStatGuildContainer()
{
}

void CStatGuildContainer::startScan()
{
	_InProgress= true;
	_GuildFiles.clear();
}

void CStatGuildContainer::addGuildFile(const NLMISC::CSString& fileName, CPersistentDataRecord& fileContent)
{
	CGuildFile& newFile= vectAppend(_GuildFiles);
	newFile.FileName=fileName;
	newFile.FileContent.readFromPdr(fileContent);
}

void CStatGuildContainer::endScan()
{
	_InProgress= false;
}

void CStatGuildContainer::display(NLMISC::CLog* log)
{
	nlassert(log!=NULL);

	// determine the longest file name length and id length;
	uint32 namelen=1;
	uint32 idlen=1;
	for (uint32 i=0;i<_GuildFiles.size();++i)
	{
		namelen=std::max(namelen,(uint32)_GuildFiles[i].FileName.size());
		idlen=std::max(idlen,(uint32)_GuildFiles[i].FileContent.getValue("Id").size());
	}

	// do the displaying...
	log->displayNL("Guild List:");
	for (uint32 i=0;i<_GuildFiles.size();++i)
	{
		log->displayNL("%-*s %*s  %s",namelen,_GuildFiles[i].FileName.c_str(),idlen,_GuildFiles[i].FileContent.getValue("Id").c_str(),_GuildFiles[i].FileContent.getValue("_Name").c_str());
	}
}

void CStatGuildContainer::writeMemberListFile(const NLMISC::CSString& path)
{
	// write a table (line per guild member)
	// columns: file name, guild id, guild name, rank, entry date, character id 
	//
	//	Guild member entries take the form:
	//
	//	Members:45638054051974.Members.Grade=Officer
	//	Members:45638054051974.Members.EnterTime=249276

	NLMISC::CSString result;
	result+="\"file name\",guild id,\"guild name\",rank,entry date,character id";
	for (uint32 i=0;i<_GuildFiles.size();++i)
	{
		NLMISC::CSString resultBase;
		resultBase+= '\n';
		resultBase+= _GuildFiles[i].FileName.quote();
		resultBase+= ',';
		resultBase+= _GuildFiles[i].FileContent.getValue("Id");
		resultBase+= ',';
		resultBase+= _GuildFiles[i].FileContent.getValue("_Name").quote();
		resultBase+= ',';

		CPersistentDataTreeNode* members=_GuildFiles[i].FileContent.getNode("Members");
		DROP_IF(members==NULL,"Skipping guild with no member data: "+_GuildFiles[i].FileName,continue);

		for (uint32 i=0;i<members->getChildren().size();++i)
		{
			NLMISC::CSString characterDescription;
			DROP_IF(members->getChildren()[i]==NULL,"Skipping null character in guild: "+_GuildFiles[i].FileName,continue);
			characterDescription+= members->getChildren()[i]->getValue("Members.Grade");
			characterDescription+= ',';
			characterDescription+= members->getChildren()[i]->getValue("Members.EnterTime");
			characterDescription+= ',';
			characterDescription+= crunchUserId(members->getChildren()[i]->getName().splitTo('#'));

			result+= resultBase;
			result+= characterDescription;
		}
	}

	nlinfo("Writing guild member list file: %s",path.c_str());
	result.writeToFile(path);
}

void CStatGuildContainer::writeInventoryFile(const NLMISC::CSString& path)
{
	// write a table (line per item in guild inventory)
	// columns: file name, guild id, guild name, slot, sheet id, quality, quantity, creator 
	//
	//	Guild inventory entries take the form:
	//
	//	_Inventory.Child#45._SheetId=7582766
	//	_Inventory.Child#45._Recommended=99
	//	_Inventory.Child#45.StackSize=11
	//	_Inventory.Child#45._CreatorId=16711680

	NLMISC::CSString result;
	result+="\"file name\",guild id,\"guild name\",item,sheet,quality,quantity,creator";
	for (uint32 i=0;i<_GuildFiles.size();++i)
	{
		NLMISC::CSString resultBase;
		resultBase+= '\n';
		resultBase+= _GuildFiles[i].FileName.quote();
		resultBase+= ',';
		resultBase+= _GuildFiles[i].FileContent.getValue("Id");
		resultBase+= ',';
		resultBase+= _GuildFiles[i].FileContent.getValue("_Name").quote();
		resultBase+= ',';

		CPersistentDataTreeNode* inventory=_GuildFiles[i].FileContent.getNode("_Inventory");
		DROP_IF(inventory==NULL,"Skipping guild with no inventory data: "+_GuildFiles[i].FileName,continue);

		uint32 childCount=0;
		for (uint32 i=0;i<inventory->getChildren().size();++i)
		{
			// skip parameters of the inventory object that don't interest us
			if (inventory->getChildren()[i]->getName().left(5)!="Child")
			{
				continue;
			}

			++childCount;
			NLMISC::CSString itemDescription;
			itemDescription+= NLMISC::toString("%3d",childCount);
			itemDescription+= ',';
			NLMISC::CSString sheet= inventory->getChildren()[i]->getValue("_SheetId");
			itemDescription+= NLMISC::CSString(NLMISC::toString("%8s: ",sheet.c_str())+NLMISC::CSheetId(sheet.atoui()).toString()).quote();
			itemDescription+= ',';
			itemDescription+= inventory->getChildren()[i]->getValue("_Recommended");
			itemDescription+= ',';
			CPersistentDataTreeNode* StackSize= inventory->getChildren()[i]->getChild("StackSize");
			itemDescription+= (StackSize==NULL)? 1: StackSize->getValue();
			itemDescription+= ',';
			itemDescription+= crunchUserId(inventory->getChildren()[i]->getValue("_CreatorId"));

			result+= resultBase;
			result+= itemDescription;
		}
	}

	nlinfo("Writing guild inventory file: %s",path.c_str());
	result.writeToFile(path);
}

void CStatGuildContainer::writeMiscDataFile(const NLMISC::CSString& path)
{
	// write a table (line per guild)
	// columns: file name, id, name, race, num members, num graded members, created, money, inv slots, item count, village, building, description
	//
	//	Guild basic properties take the form:
	//
	//	Id=81
	//	_Name=="Les Epees du Vents"
	//	Race=Tryker
	//	CreationDate=0
	//	Money=1416085
	//	Village=373
	//	Building=182452283
	//	_Description=="Bonne humeur"
	//
	//	Members:45638054051974.Members.EnterTime=249276
	//
	//	_Inventory.Child#45._SheetId=7582766


	NLMISC::CSString result;
	result+="\"file name\",guild id,\"guild name\",race,members,graded members,created,money,unique items,total items,village,building,leader,\"description\"";
	for (uint32 i=0;i<_GuildFiles.size();++i)
	{
		// count the number of items int he guild inventory
		CPersistentDataTreeNode* inventory=_GuildFiles[i].FileContent.getNode("_Inventory");
		DROP_IF(inventory==NULL,"Skipping guild with no inventory data: "+_GuildFiles[i].FileName,continue);
		uint32 itemCount=0;
		uint32 invSlotCount=0;
		for (uint32 j=0;j<inventory->getChildren().size();++j)
		{
			// skip parameters of the inventory object that don't interest us
			if (inventory->getChildren()[j]->getName().left(5)=="Child")
			{
				uint32 stackSize= inventory->getChildren()[j]->getValue("StackSize").atoui();
				if (stackSize==0)
					++itemCount;
				else
					itemCount+=stackSize;
				++invSlotCount;
			}
		}

		// count the members and extract the leader
		uint32 numGuildMembers=0;
		uint32 numGuildOfficers=0;
		NLMISC::CSString guildLeader;
		CPersistentDataTreeNode* members=_GuildFiles[i].FileContent.getNode("Members");
		DROP_IF(members==NULL,"Skipping guild with no member data: "+_GuildFiles[i].FileName,continue);
		for (uint32 j=0;j<members->getChildren().size();++j)
		{
			DROP_IF(members->getChildren()[j]==NULL,"Skipping null character in guild: "+_GuildFiles[i].FileName,continue);
			if (members->getChildren()[j]->getValue("Members.Grade")=="Leader")
				guildLeader= crunchUserId(members->getChildren()[j]->getName().splitTo('#'));
			if (members->getChildren()[j]->getValue("Members.Grade")!="Member")
				++numGuildOfficers;
			++numGuildMembers;
		}

		NLMISC::CSString guildDescription;
		guildDescription+= '\n';
		guildDescription+= _GuildFiles[i].FileName.quote();
		guildDescription+= ',';
		guildDescription+= _GuildFiles[i].FileContent.getValue("Id");
		guildDescription+= ',';
		guildDescription+= _GuildFiles[i].FileContent.getValue("_Name").quote();
		guildDescription+= ',';
		guildDescription+= _GuildFiles[i].FileContent.getValue("Race");
		guildDescription+= ',';
		guildDescription+= NLMISC::toString(numGuildMembers);
		guildDescription+= ',';
		guildDescription+= NLMISC::toString(numGuildOfficers);
		guildDescription+= ',';
		guildDescription+= _GuildFiles[i].FileContent.getValue("CreationDate");
		guildDescription+= ',';
		guildDescription+= _GuildFiles[i].FileContent.getValue("Money");
		guildDescription+= ',';
		guildDescription+= NLMISC::toString(invSlotCount);
		guildDescription+= ',';
		guildDescription+= NLMISC::toString(itemCount);
		guildDescription+= ',';
		guildDescription+= _GuildFiles[i].FileContent.getValue("Village");
		guildDescription+= ',';
		guildDescription+= _GuildFiles[i].FileContent.getValue("Building");
		guildDescription+= ',';
		guildDescription+= guildLeader;
		guildDescription+= ',';
		guildDescription+= _GuildFiles[i].FileContent.getValue("_Description").quote();
		guildDescription+= ',';

		result+= guildDescription;
	}

	nlinfo("Writing guild info file: %s",path.c_str());
	result.writeToFile(path);
}


//-------------------------------------------------------------------------------------------------
