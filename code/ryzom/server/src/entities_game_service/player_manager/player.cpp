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

#include "nel/misc/eid_translator.h"
#include "nel/misc/path.h"
#include "nel/misc/algo.h"
#include "nel/misc/command.h"
#include "nel/net/service.h"

#include "game_share/msg_client_server.h"
#include "server_share/mail_forum_validator.h"

#include "player_manager/player.h"
#include "player_manager/player_manager.h"
#include "egs_mirror.h"
#include "egs_sheets/egs_sheets.h"
#include "zone_manager.h"
#include "building_manager/building_manager.h"
#include "guild_manager/fame_manager.h"

#include "player_manager/character.h"
#include "modules/shard_unifier_client.h"
#include "entities_game_service.h"


const uint MaxCharacterCountPerPlayer = 5;


using namespace std;
using namespace NLMISC;
using namespace NLNET;


NL_INSTANCE_COUNTER_IMPL(CAsyncPlayerLoad);
NL_INSTANCE_COUNTER_IMPL(CPlayer);

bool wipeAndRestore(const std::string &fileName);

FILE	*LastLoad = NULL;
const char	*LastLoadFileName = "last_loaded_char.bin";

extern CVariable<uint32>	MonkeyLoadEnable;
extern uint32 CharacterSaveCounter;
extern uint32 CharacterLoadCounter;
extern CPlayerService * PS;

//
//---------------------------------------------------
void	CPlayer::writeCrashMarker( uint32 userId, uint32 charId)
{
	if (LastLoad != NULL)
	{
		// data for load crash recovery
		uint32 TheCharId[2];
		TheCharId[0] = userId;
		TheCharId[1] = charId;
		fseek(LastLoad, 0, SEEK_SET);
		fwrite(TheCharId, 4*2, 1, LastLoad);
		fflush(LastLoad);
	}
}

std::string makeCharacterFileName(uint32 userId, uint32 charIndex)
{
	string charPath = PlayerManager.getCharacterPath(userId, false);

	string fileName;
	if( PlayerManager.getStallMode() == true )
		fileName = NLMISC::toString( "account_%u_%d.bin",userId, charIndex );
	else if ( XMLSave )
		fileName = charPath+NLMISC::toString( "account_%u_%d.xml", userId, charIndex );
	else
		fileName = charPath+NLMISC::toString( "account_%u_%d.bin", userId, charIndex );

	return fileName;
}

//
//---------------------------------------------------
void CPlayer::checkCrashMarker()
{
	// open and check the last load tag file
	if (LastLoad == NULL)
	{
		if (!CFile::isExists(LastLoadFileName))
		{
			// create the file if needed
			nlverify(LastLoad  = fopen(LastLoadFileName, "wb"));
			fclose(LastLoad);
		}
		
		nlverify(LastLoad = fopen(LastLoadFileName, "r+b"));
		
		// check 
		uint32	lastBad[2];
		uint32 nbRead = (uint32)fread(lastBad, 1, 4*2, LastLoad);
		if (nbRead == 8 && lastBad[0] != 0xffffffff)
		{
			// there is a char to to backup
			
			nlwarning("CPlayer::loadAllCharacters: last loaded character (%u:%u) is bad, tag it as bad and try to restore the last valid version",
				lastBad[0],
				lastBad[1]);

			uint32	userId = lastBad[0];
			uint32	charId = lastBad[1];
			
			wipeAndRestore(NLMISC::toString("%s/account_%u_%d.bin", PlayerManager.getCharacterPath(userId, false).c_str(), userId, charId));
			wipeAndRestore(NLMISC::toString("%s/account_%u_%d_pdr.xml", PlayerManager.getCharacterPath(userId, false).c_str(), userId, charId));
			wipeAndRestore(NLMISC::toString("%s/account_%u_%d_pdr.bin", PlayerManager.getCharacterPath(userId, false).c_str(), userId, charId));
			
			//			string fileName = makeCharacterFileName(lastBad[0], lastBad[1]);
			//			CFile::moveFile((fileName+".wiped").c_str(), fileName.c_str());
			//
			//			// try to restore a backup
			//			if (CFile::isExists(fileName+".last_good"))
			//			{
			//				nlwarning("  Restoring last good version...");
			//				CFile::copyFile(fileName, fileName+".last_good");
			//				nlwarning("  And copying the backup for comparison with bad file...");
			//				CFile::copyFile(fileName+".before_wipe", fileName+".last_good");
			//			}
			//			else
			//			{
			//				nlwarning("  Can't find a valid backup, CHARACTER IS LOST");
			//			}
		}
		
		writeCrashMarker(0xffffffff, 0xffffffff);
	}
}

//
//---------------------------------------------------
bool wipeAndRestore(const std::string &fileName)
{
	string newfn = fileName+".wiped";
	if (CFile::isExists(newfn))
	{
		uint	i;
		string	incFn;
		for (i=0; i<10 && CFile::isExists(incFn = newfn+"_"+toString(i)); ++i)
			;

		if (i==10)
		{
			// max wipe backup, just say the char is lost
			nlwarning("Error while loading character '%s', too many wiped version, no more wipe allowed", fileName.c_str());
			return false;
		}
		// move the last wiped file
		CFile::moveFile(incFn.c_str(), newfn.c_str());
	}
	CFile::moveFile(newfn.c_str(), fileName.c_str());

	//	// try to restore a backup
	//if (CFile::isExists(fileName+".last_good"))
	//{
	//	nlwarning("  Restoring last good version...");
	//	CFile::copyFile(fileName, fileName+".last_good");
	//	nlwarning("  And copying the backup for comparison with bad file...");
	//	CFile::copyFile(fileName+".before_wipe", fileName+".last_good");
	//
	//	// restore success
	//	return true;
	//}
	//else
	//{
		nlwarning("  Can't find a valid backup, CHARACTER IS LOST");

		// restore failure
		return false;
	//}
}



//---------------------------------------------------
// CAsyncPlayerLoad :
// 
//---------------------------------------------------
CAsyncPlayerLoad::CAsyncPlayerLoad() : Player(NULL), UserId(0)
{
	clear();
}

void	CAsyncPlayerLoad::initChars()
{
	Chars.clear();
	Chars.resize(MaxCharacterCountPerPlayer);
}

// Ask BS to transfer character data
void	CAsyncPlayerLoad::startLoading()
{
	if (Player == NULL)
		return;

	CPlayer::checkCrashMarker();

	initChars();

	// try to load all characters
	TMapUserCharFileReplaced::iterator it = PlayerManager.UserCharIdReplaced.find(UserId);
	uint	i;
	for (i=0; i<MaxCharacterCountPerPlayer; ++i)
	{
		std::vector<CBackupFileClass>	classes;

		CBackupFileClass	classOk;
		if( it != PlayerManager.UserCharIdReplaced.end() && (*it).second.CharIndex == i )
		{
			classOk.Patterns.push_back((*it).second.Filename+".bin");
			classOk.Patterns.push_back((*it).second.Filename+"_pdr.xml");
			classOk.Patterns.push_back((*it).second.Filename+"_pdr.bin");
			PlayerManager.UserCharIdReplaced.erase(it);
			it = PlayerManager.UserCharIdReplaced.end();
		}
		else
		{
			classOk.Patterns.push_back(NLMISC::toString("account_%d_%d.bin", UserId, i));
			classOk.Patterns.push_back(NLMISC::toString("account_%d_%d_pdr.xml", UserId, i));
			classOk.Patterns.push_back(NLMISC::toString("account_%d_%d_pdr.bin", UserId, i));
		}
		classes.push_back(classOk);

//		nlinfo("BSIF: requesting file class...");
		BsiGlobal.requestFileClass(PlayerManager.getCharacterPath(UserId, true), classes, new CFileClassCallback(this, i));
	}
}

// Received characters file list
void	CAsyncPlayerLoad::receivedCharacterFileList(const CFileDescriptionContainer& fileList, uint charId)
{
	if (Player == NULL)
		return;

	if (Chars.size() <= charId || Chars[charId].Ready)
	{
		// already received char file list!!
		return;
	}

	if (fileList.empty())
	{
		// no char to load, set character as ready
		setCharReady(charId);
		return;
	}

	vector<CFileDescription>	files;

	uint	i;
	// store file list
	for (i=0; i<fileList.size(); ++i)
		Chars[charId].Files.push_back(fileList[i].FileName);

	startCharLoading(charId);
}

// Start Char loading
void	CAsyncPlayerLoad::startCharLoading(uint charId)
{
	if (Player == NULL || charId >= Chars.size())
		return;

	// no file to load, assumes no character can be loaded
	if (Chars[charId].Files.empty())
	{
		setCharReady(charId);
		return;
	}

	// pop next file to load and start loading with this file
//	nlinfo("BSIF: requesting file...");
	BsiGlobal.requestFile(Chars[charId].Files.front(), new CFileCallback(this, charId));
	Chars[charId].Files.erase(Chars[charId].Files.begin());
}


// Received character file
void	CAsyncPlayerLoad::receivedCharacterFile(const CFileDescription& fileDescription, NLMISC::IStream& dataStream, uint charId)
{
	if (Player == NULL)
		return;

	if (fileDescription.FileName.empty())
	{
		// no file
		setCharReady(charId);
		return;
	}

	if (charId >= Chars.size() || Chars[charId].Ready)
		return;

	if (fileDescription.FileName.empty())
	{
		setCharReady(charId);
		return;
	}

	bool	success = false;

	uint32	CharId = charId;
	uint32	UserId = Player->getUserId();

	NLMISC::CMemStream&	memStream = dynamic_cast<NLMISC::CMemStream&>(dataStream);
	if (&memStream != NULL)
	{
		std::string	reason;

		// data for load crash recovery
		CPlayer::writeCrashMarker(UserId, CharId);

		try
		{
			// create a new character record
			Player->_Characters[charId] = new CCharacter();
			Player->_Characters[charId]->setId( PlayerManager.createCharacterId( UserId, charId ) );

			if (testWildCard(CFile::getFilename(fileDescription.FileName).c_str(), "*_pdr.*"))
			{
				static CPersistentDataRecord	pdr;
				pdr.clear();
				
				if (pdr.fromBuffer(memStream))
				{
					// apply the loaded pdr record to the new character
					++CharacterLoadCounter;
					Player->_Characters[charId]->apply(pdr);
					Player->_Characters[charId]->setSaveDate(fileDescription.FileTimeStamp);
					success = true;
				}
				else
				{
					reason = "unable to build pdr from stream";
				}
			}
			else /*if (testWildCard(CFile::getFilename(fileDescription.FileName).c_str(), "*.*"))*/
			{
				memStream.xmlPush( NLMISC::toString("Character%d", charId).c_str() );
				memStream.serial( *(Player->_Characters[charId]) );
				memStream.xmlPop();
				success = true;
			}
		}
		catch (const Exception& e)
		{
			reason = e.what();
		}
		catch (...)
		{
			reason = "low level exception";
		}

		// and clean the tag file
		CPlayer::writeCrashMarker(0xffffffff, 0xffffffff);

		if (!success)
		{
			nlwarning("Failed to load user '%d' character '%d' from BS stream file '%s': %s", UserId, charId, fileDescription.FileName.c_str(), reason.c_str());

			// wipe?

			if (Player->_Characters[charId])
			{
				delete Player->_Characters[charId];
				Player->_Characters[charId] = NULL;
			}

			if (wipeAndRestore(fileDescription.FileName))
			{
				Chars[CharId].Files.insert(Chars[CharId].Files.begin(), fileDescription.FileName);
				startCharLoading(charId);
				return;
			}
		}
		else
		{
			egs_plinfo("LOADED User '%d' Character '%s' from BS stream file '%s'", UserId, Player->_Characters[charId]->getName().toUtf8().c_str(), fileDescription.FileName.c_str());

//			// create a valid backup (using backup service)
//			CBackupMsgSaveFile msg( fileDescription.FileName+".last_good", CBackupMsgSaveFile::SaveFile, BsiGlobal );
//
//			/*if (testWildCard(CFile::getFilename(filename).c_str(), "*.backup"))
//			{
//				//filename = CPath::standardizePath(CFile::getPath(filename))+CFile::getFilenameWithoutExtension(filename);
//			}*/
//
//			H_AUTO(SavePlayerMakeMsgBS);
//			memStream.seek(0, NLMISC::IStream::begin);
//			msg.DataMsg.serialBuffer((uint8*)memStream.buffer(), memStream.length());
//			BsiGlobal.sendFile( msg );
		}
	}
	else
	{
		// don't wipe here, internal issue
		nlwarning("Failed to load Character '%d' from BS pdr stream: stream is not a CMemStream", charId);
	}

	setCharReady(charId);
}

// Check All characters ready
bool	CAsyncPlayerLoad::allCharsReady()
{
	uint	i;
	for (i=0; i<Chars.size(); ++i)
		if (!Chars[i].Ready)
			return false;

	// set status connexion of player to connected
	Player->setPlayerConnection( true );

	// set player cookie, for later web activation
	Player->setLoginCookie( LoginCookie );

	NLNET::TServiceId frontEndId = PlayerManager.getPlayerFrontEndId( Player->getUserId() );
	// tell IOS which language the user chose
	CMessage msgLang("USER_LANGUAGE");
	msgLang.serial( UserId );
	msgLang.serial( frontEndId );
	msgLang.serial( LanguageId );
	CUnifiedNetwork::getInstance()->send("IOS",msgLang);

	// update the ring database (cleanup any desync)
	Player->updateCharactersInRingDB();

	// character summary is sent when SU tell that all name errors have been fixed
//	sendCharactersSummary( Player, AllAuthorized );

	if( MonkeyLoadEnable > 0)
		PS->egsAddMonkeyPlayerCallback(Player->getUserId());

	return true;
}




//---------------------------------------------------
// CPlayer :
// 
//---------------------------------------------------
CPlayer::CPlayer() 
	:_LanguageId("en"), _BetaTester(false), _PreOrder(false), _WindermeerCommunity(false)
{ 
	_ActiveCharIndex = -1; 
	_Verbose = false;
	_Characters.resize( MaxCharacterCountPerPlayer, 0 );
	_DisconnectionTime = 0;
} // CPlayer //


//---------------------------------------------------
// CPlayer : createCharacter
// 
//---------------------------------------------------
CEntityId CPlayer::createCharacter( const std::string& characterName, EGSPD::CPeople::TPeople people, GSGENDER::EGender gender ) 
{ 
	CCharacter * ch = new CCharacter();
	sint32 index = PlayerManager.getFirstFreeCharacterIndex( _UserId );
	if( index == - 1 )
		return CEntityId();

	CEntityId charId = PlayerManager.createCharacterId( _UserId, index );
	//Mirror.addEntity( charId ); // Untested here
	ch->setId( charId ); 

	CCreateCharMsg msg;
	msg.Name = characterName;
	msg.People = people;
	msg.Sex = gender;
	if(UseNewNewbieLandStartingPoint)
	{
		msg.StartPoint = RYZOM_STARTING_POINT::starting_city;
	}
	else
	{
		msg.StartPoint = RYZOM_STARTING_POINT::aegus;
	}
	msg.NbPointCaster = 1;
	msg.NbPointFighter = 2;
	msg.NbPointCrafter = 1;
	msg.NbPointHarvester = 1;
	
	CSheetId people_sheet;
	const CStaticRaceStats* staticRaceStats = 0;
	for( map< CSheetId, CStaticRaceStats >::const_iterator it = CSheets::getRaceStatsContainer().begin(); it != CSheets::getRaceStatsContainer().end(); ++it )
	{
		if( (*it).second.Race == msg.People )
		{
			people_sheet = (*it).first;
			break;
		}
	}
	nlassert( people_sheet != CSheetId::Unknown );
	staticRaceStats = CSheets::getRaceStats( people_sheet );
	nlassert( staticRaceStats );

	ch->setStartStatistics( msg );
	PlayerManager.addChar( _UserId, charId, ch, index );
	return charId;
}

//---------------------------------------------------
// getFirstFreeCharacterIndex
//---------------------------------------------------
sint32 CPlayer::getFirstFreeCharacterIndex()
{
	for( uint i = 0; i < _Characters.size(); ++i )
	{
		if( _Characters[ i ] == 0 )
		{
			return (sint32) i;
		}
	}
	return -1;
}

//---------------------------------------------------
// getCharacterIndex
//---------------------------------------------------
sint32 CPlayer::getCharacterIndex( uint32 number )
{
	uint32 NonNullNumber = ~0;
	for( uint i = 0; i < _Characters.size(); ++i )
	{
		if( _Characters[ i ] != 0 )
		{
			++NonNullNumber;
			if( NonNullNumber == number )
			{
				return i;
			}
		}
	}
	return -1;
}

//---------------------------------------------------
// getCharacter
//---------------------------------------------------
CCharacter *CPlayer::getCharacter( uint32 c )
{
	if( c < _Characters.size() )
	{
		return _Characters[c];
	}
	nlwarning("<CCharacter::getCharacter>  :  The character %d doesn't exist", c);
	return NULL;
}

//---------------------------------------------------
// getCharacter
//---------------------------------------------------
CCharacter * CPlayer::getCharacter( const NLMISC::CEntityId& id )
{
	for( uint i = 0; i < _Characters.size(); ++i )
	{
		if( _Characters[ i ] != 0 )
		{
			if( _Characters[ i ]->getId() == id )
			{
				return _Characters[i];
			}
		}
	}
	return NULL;
}

//---------------------------------------------------
// getCharacterCount
//---------------------------------------------------
uint8 CPlayer::getCharacterCount() const
{
	uint8 nbCharacter = 0;
	for( uint8 i = 0; i < _Characters.size(); ++i )
	{
		if( _Characters[ i ] != 0 )
		{
			++nbCharacter;
		}
	}
	return nbCharacter;
}

//---------------------------------------------------
// getCharactersSummary :
// 
//---------------------------------------------------
void CPlayer::getCharactersSummary( vector<CCharacterSummary>& chars )
{
	chars.clear();
	for( uint32 slot = 0; slot != _Characters.size(); ++slot )
	{
		CCharacterSummary ch;
		ch.CharacterSlot = (uint8)slot;

		if( _Characters[ slot ] != 0 )
		{
			// retrieve the home mainland Id of this character
			const CFarPositionStack& posStack = _Characters[ slot ]->PositionStack;
			if ( ! posStack.empty() )
				ch.Mainland = posStack[0].SessionId; // return the sessionId of the bottom of the stack
			else
			{
				nlwarning( "Position Stack empty when retrieving char summary for %s", _Characters[ slot ]->getId().toString().c_str() );
				ch.Mainland = TSessionId(0); // should not occur
			}

			// retrieve name and race
			ch.Name = _Characters[ slot ]->getName();
			ch.People = (EGSPD::CPeople::TPeople)_Characters[ slot ]->getRace();

			const CRegion * region = NULL;
			const CContinent * continent = NULL;

			if ( !posStack.empty() )
			{
				if ( CZoneManager::getInstance().getRegion( posStack[posStack.size()-1].PosState.X,posStack[posStack.size()-1].PosState.Y,&region,&continent ) && region && continent )
				{
					TVectorParamCheck params(2);
					params[0].Type = params[1].Type = STRING_MANAGER::place;
					params[0].Identifier = 	continent->getName();
					params[1].Identifier = 	region->getName();
					ch.Location = STRING_MANAGER::sendStringToUser( getUserId(),"START_LOCATION",params);
					ch.InNewbieland = continent->getId() == CONTINENT::NEWBIELAND;
				}
				else
				{
					/* When getRegion failed then region and continent may not be set */
					nlwarning("<CPlayer getCharactersSummary> Invalid location for user %u Pos=(%d,%d) Region=%s Continent=%s",
							  getUserId(),posStack[posStack.size()-1].PosState.X,posStack[posStack.size()-1].PosState.Y,
							  (region? region->getName().c_str(): "NULL"), (continent? continent->getName().c_str(): "NULL") );
				}
			}

			ch.VisualPropA = _Characters[ slot ]->getVisualPropertyA();
			ch.VisualPropB = _Characters[ slot ]->getVisualPropertyB();
			ch.VisualPropC = _Characters[ slot ]->getVisualPropertyC();
			ch.SheetId = _Characters[ slot ]->getType();
		}
		chars.push_back( ch );
	}
} // getCharactersSummary //

//---------------------------------------------------
// deleteCharacter : Delete a character
// 
//---------------------------------------------------
void CPlayer::deleteCharacter( uint32 index )
{
	if( index >= _Characters.size() )
	{
		nlwarning("<CPlayer::deleteCharacter>: Player %d, Slot %d are out of bound (0 - %d)", _UserId, index, _Characters.size() );
		return;
	}

	if( _Characters[ index ] == 0 )
	{
		nlwarning("<CPlayer::deleteCharacter> Player %d, Slot %d Attempt to delete a character on a emty slot", _UserId, index );
		return;
	}

	// Remove the directory for player mail
	string name = _Characters[ index ]->getName().toUtf8();
	CMailForumValidator::removeUser(_Characters[ index ]->getHomeMainlandSessionId(), name);

	_Characters[ index ]->destroyCharacter();
	CBuildingManager::getInstance()->removePlayerBuilding( _Characters[ index ]->getId() );

	CEntityId idDeleted = _Characters[ index ]->getId();
	CEntityIdTranslator::getInstance()->unregisterEntity( idDeleted );
	delete _Characters[ index ];
	_Characters[ index ] = 0;
	egs_plinfo("Player %u are delete his character slot %u", _UserId, index );
}

//---------------------------------------------------
// getState :
// 
//---------------------------------------------------
const CEntityState& CPlayer::getState( sint32 charIndex )
{
	nlassert( charIndex < (sint)_Characters.size() );

	if( charIndex == -1 )
	{
		charIndex = _ActiveCharIndex;
	}

	nlassert( charIndex > 0 && charIndex < (sint32)_Characters.size() );

	return _Characters[charIndex]->getState();

} // getState //


//---------------------------------------------------
// getType :
// 
//---------------------------------------------------
NLMISC::CSheetId CPlayer::getType( sint32 charIndex )
{
	nlassert( charIndex < (sint)_Characters.size() );

	if( charIndex == -1 )
	{
		charIndex = _ActiveCharIndex;
	}

	nlassert( charIndex > 0 && charIndex < (sint32)_Characters.size() );
	
	return _Characters[charIndex]->getType();

} // getState //


//---------------------------------------------------
void CPlayer::loadAllCharacters()
{
	H_AUTO(LoadAllCharacters);

	bool characterFound = false;
	// load all character files
	for ( uint i = 0; i < MaxCharacterCountPerPlayer; ++i )
	{
		std::string fileNameBase= NLMISC::toString( "%saccount_%u_%d", PlayerManager.getCharacterPath(_UserId, false).c_str(), _UserId, i );
		uint32 newestDate= 0;
		enum { NONE, SERIAL_BIN, PDR_BIN, PDR_XML } mode = NONE;

		std::string serialBinFileName= fileNameBase+".bin";
		if (CFile::fileExists(serialBinFileName))
		{
			uint32 fileDate= CFile::getFileModificationDate(serialBinFileName);
			if (fileDate>newestDate)
			{
				newestDate= fileDate;
				mode= SERIAL_BIN;
			}
		}

		std::string pdrBinFileName= fileNameBase+"_pdr.bin";
		if (CFile::fileExists(pdrBinFileName))
		{
			uint32 fileDate= CFile::getFileModificationDate(pdrBinFileName);
			if (fileDate>newestDate)
			{
				newestDate= fileDate;
				mode= PDR_BIN;
			}
		}

		std::string pdrXmlFileName= fileNameBase+"_pdr.xml";
		if (CFile::fileExists(pdrXmlFileName))
		{
			uint32 fileDate= CFile::getFileModificationDate(pdrXmlFileName);
			if (fileDate>newestDate)
			{
				newestDate= fileDate;
				mode= PDR_XML;
			}
		}

		// if we didn't find a svaed file at all then give up
		if (mode==NONE)
			continue;

		// steup the new character
		CCharacter *pCh = new CCharacter();
		pCh->setId( PlayerManager.createCharacterId( _UserId, i ) );

		switch (mode)
		{
		case SERIAL_BIN:
			{
				H_AUTO(LoadAllCharactersSerialBin);

				CIFile f;
				f.open(serialBinFileName);
				try
				{
					f.xmlPush( ( string( "Character") + toString( i ) ).c_str() );
					pCh->serial( f );
					f.xmlPop();
					_Characters[ i ] = pCh;
					characterFound = true;
					egs_plinfo("LOADED Character '%s' from file: %s",pCh->getName().toUtf8().c_str(),serialBinFileName.c_str());
				}
				catch(const Exception &e)
				{
					nlwarning("Failed to load '%s': %s", serialBinFileName.c_str(), e.what());
					string newfn = serialBinFileName+".wiped";
					CFile::moveFile(newfn.c_str(), serialBinFileName.c_str());
				}
				catch(...)
				{
					nlwarning("Failed to load '%s': low level exception", serialBinFileName.c_str());
					string newfn = serialBinFileName+".wiped";
					CFile::moveFile(newfn.c_str(), serialBinFileName.c_str());
				}
			}
			break;

		case PDR_BIN:
			{
				H_AUTO(LoadAllCharactersPdrBin);

				static CPersistentDataRecord	pdr;
				pdr.clear();
				bool isOK;
				{
					H_AUTO(LoadAllCharactersPdrBinReadFile);
					isOK= pdr.readFromBinFile(pdrBinFileName.c_str());
				}
				if (!isOK)
					break;
				{
					H_AUTO(LoadAllCharactersPdrBinApply);
					pCh->apply(pdr);
				}
				_Characters[ i ] = pCh;
				characterFound = true;
				egs_plinfo("LOADED Character '%s' from file: %s",pCh->getName().toUtf8().c_str(),pdrBinFileName.c_str());
			}
			break;

		case PDR_XML:
			{
				H_AUTO(LoadAllCharactersPdrXml);

				static CPersistentDataRecord	pdr;
				pdr.clear();
				bool isOK;
				{
					H_AUTO(LoadAllCharactersPdrXmlReadFile);
					isOK= pdr.readFromTxtFile(pdrXmlFileName.c_str());
				}
				if (!isOK)
					break;
				{
					H_AUTO(LoadAllCharactersPdrXmlApply);
					pCh->apply(pdr);
				}
				_Characters[ i ] = pCh;
				characterFound = true;
				egs_plinfo("LOADED Character '%s' from file: %s",pCh->getName().toUtf8().c_str(),pdrXmlFileName.c_str());
			}
			break;
		}
	}
	// no character found : try to load old save format
	if ( !characterFound )
	{
		H_AUTO(LoadAllCharactersOldSaveFormat);

		string fileName = BsiGlobal.getLocalPath() + string("player_characters/account_") + toString(getUserId()) + string(".bin");
		CIFile f;
		bool open = f.open(fileName);
		bool xml = false;
		if( open == false  )
		{
			fileName = BsiGlobal.getLocalPath() + string("player_characters/account_") + toString(getUserId()) + string(".xml");
			open = f.open(fileName);
			xml = true;
		}
		/// END OF BINARY SAVE FILES PATCH
		if( open )
		{
			if ( xml )
			{
				CIXml input;
				//Init
				try
				{
					if( input.init( f ) )
					{
						// load player
						loadOldFormat( input );
						f.close();
						NLMISC::CFile::moveFile( ( BsiGlobal.getLocalPath() + string("player_characters/backup_account_") + toString(getUserId()) + string(".bin") ).c_str(), fileName.c_str() );
						for ( sint16 i = 0; i < (sint16)_Characters.size(); i++ )
						{
							if( _Characters[i] != 0 )
								PlayerManager.savePlayerChar( _UserId, i );
						}
					}
				}
				catch(const Exception &e)
				{
					nlwarning("Failed to load '%s': %s", fileName.c_str(), e.what());
					string newfn = fileName+".wiped";
					CFile::moveFile(newfn.c_str(), fileName.c_str());
				}
				catch(...)
				{
					nlwarning("Failed to load '%s': low level exception", fileName.c_str());
					string newfn = fileName+".wiped";
					CFile::moveFile(newfn.c_str(), fileName.c_str());
				}
			}
			else
			{
				try
				{
					loadOldFormat( f );
					// Close the File.
					f.close();
					NLMISC::CFile::moveFile( ( BsiGlobal.getLocalPath() + string("player_characters/backup_account_") + toString(getUserId()) + string(".bin") ).c_str(), fileName.c_str() );
					for ( sint16 i = 0; i < (sint16)_Characters.size(); i++ )
					{
						if( _Characters[i] != 0 )
							PlayerManager.savePlayerChar( _UserId, i );
					}
				}
				catch(const Exception &e)
				{
					nlwarning("Failed to load '%s': %s", fileName.c_str(), e.what());
					string newfn = fileName+".wiped";
					CFile::moveFile(newfn.c_str(), fileName.c_str());
				}
				catch(...)
				{
					nlwarning("Failed to load '%s': low level exception", fileName.c_str());
					string newfn = fileName+".wiped";
					CFile::moveFile(newfn.c_str(), fileName.c_str());
				}
			}
		}
	}
}

//---------------------------------------------------
void CPlayer::loadAllCharactersPdr()
{
	H_AUTO(loadAllCharactersPdr);

	// load all character files
	for ( uint i = 0; i < MaxCharacterCountPerPlayer; ++i )
	{
		string fileName= string("account_%u_%d_pdr")+ (XMLSave? ".xml": ".bin");

		// if the backup service isn't stalled then set the save path to the correct backup location
		if( !PlayerManager.getStallMode() )
			fileName = PlayerManager.getCharacterPath(_UserId, false) + fileName;

		// if the file doesn't exist then skip it
		if ( !NLMISC::CFile::fileExists(fileName) )
			continue;

		// setup a data record to hold the loaded player data
		static CPersistentDataRecord	pdr;
		pdr.clear();
		
		// try loading the save data from disk
		try
		{
			bool isOK= pdr.readFromFile(fileName.c_str());
			if (!isOK)
				continue;
		}
		catch(const Exception &e)
		{
			nlwarning("Failed to load '%s': %s", fileName.c_str(), e.what());
			string newfn = fileName+".wiped";
			CFile::moveFile(newfn.c_str(), fileName.c_str());
			continue;
		}
		catch(...)
		{
			nlwarning("Failed to load '%s': low level exception", fileName.c_str());
			string newfn = fileName+".wiped";
			CFile::moveFile(newfn.c_str(), fileName.c_str());
			continue;
		}

		// create a new character record
		_Characters[ i ] = new CCharacter();
		_Characters[ i ]->setId( PlayerManager.createCharacterId( _UserId, i ) );

		// apply the loaded pdr record to the new character
		_Characters[ i ]->apply(pdr);

		egs_plinfo("LOADED Character '%s' from file: %s",_Characters[ i ]->getName().toUtf8().c_str(),fileName.c_str());
	}
}

void CPlayer::saveCharacter(class NLMISC::IStream &f,sint32 index)
{
	H_AUTO(SaveCharacter);

	if ( (uint) index >= _Characters.size() )
	{
		nlwarning("<SAVE> Invalid chr index %d ( count : %u). User id : %u",index,_Characters.size(),_UserId);
		return;
	}

	if( _Characters[index] != 0 )
	{
		CFameManager::getInstance().savePlayerFame(_Characters[index]->getId(), _Characters[index]->getPlayerFamesContainer());
		_Characters[index]->serial( f );
	}
	else
	{
		nlwarning("<SAVE> Null chr index %d ( count : %u). User id : %u",index,_Characters.size(),_UserId);
		return;
	}
}

void CPlayer::storeCharacter(CPersistentDataRecord &pdr, sint32 index)
{
	H_AUTO(StoreCharacter);

	if ( (uint) index >= _Characters.size() )
	{
		nlwarning("<SAVE> Invalid chr index %d ( count : %u). User id : %u",index,_Characters.size(),_UserId);
		return;
	}

	if( _Characters[index] != 0 )
	{
//		TDataSetRow entityIndex = TheFameDataset.getDataSetRow(_Characters[index]->getId());
//		if (TheFameDataset.isAccessible(entityIndex))
		{
			++CharacterSaveCounter;
			_Characters[index]->store(pdr);
		}
	}
	else
	{
		nlwarning("<SAVE> Null chr index %d ( count : %u). User id : %u",index,_Characters.size(),_UserId);
		return;
	}
}

bool CPlayer::setActiveCharIndex( uint32 index, NLMISC::CEntityId charId )
{
	if (index >= _Characters.size() )
		return false;

	if( _Characters[index] != 0 )
	{
		_ActiveCharIndex = (sint32)index;
		_Characters[index]->updateConnexionStat();
	
		for ( uint32 i = 0; i < _Characters.size(); ++i )
		{
			if( i != index )
			{
				if( _Characters[i] != 0 )
				{
					delete _Characters[i];
					_Characters[i] = 0;
				}
			}
		}
	}
	return false;
}

void CPlayer::setValue( std::string var, std::string value )
{
	_Characters[_ActiveCharIndex]->setValue( var, value );
}

void CPlayer::getValue( std::string var, std::string& value )
{
	_Characters[_ActiveCharIndex]->getValue( var, value );
}

//---------------------------------------------------
// RemoveAllCharacters :
// 
//---------------------------------------------------
void CPlayer::removeAllCharacters()
{
	H_AUTO(RemoveAllCharacters);

	for( uint32 i = 0; i < _Characters.size(); ++i )
	{
		CCharacter * c = _Characters[ i ];
		if( c != 0 )
		{
			// first delete the character, as character destructor sometimes call some methods
			// which call CPlayerManager::getChar(), and so the pointer in _Characters may still be used at this point
			delete c;
			_Characters[ i ] = 0;
		}
	}
}

/// Send the characters info to the Shard unifier to resync the ring database
void CPlayer::updateCharactersInRingDB()
{
	if (IShardUnifierEvent::getInstance() == NULL)
	{
		// automatically validate the names and send the summary to client
		sendCharactersSummary( this);

		return;
	}

	vector<CHARSYNC::TCharInfo>	charInfos;

	charInfos.reserve(_Characters.size());
	for (uint i=0; i<_Characters.size(); ++i)
	{
		const CCharacter *ch = _Characters[i];
		if (ch != NULL)
		{
			CHARSYNC::TCharInfo ci;
			ch->fillCharInfo(ci);
//			ci.setCharEId(ch->getId());
//			ci.setCharName(ch->getName().toUtf8());
//			ci.setHomeSessionId(ch->getHomeMainlandSessionId());
//			ci.setBestCombatLevel(max(ch->getBestChildSkillValue(SKILLS::SF), ch->getBestChildSkillValue(SKILLS::SMO)));
//			ci.setGuildId(ch->getGuildId());
//			ci.setRespawnPoints(ch->getRespawnPoints().buildRingPoints());

			charInfos.push_back(ci);
		}
	}

	IShardUnifierEvent::getInstance()->onUpdateCharacters(_UserId, charInfos);
}


CPlayer::~CPlayer()
{
	H_AUTO(PlyerDtor);

	PlayerManager.cancelAsyncLoadPlayer( _UserId );

	egs_plinfo( "Player with userId = %u removed", _UserId );	
	removeAllCharacters();
}

// Return true if the specified priv string contains the priviledge string of the player
bool CPlayer::havePriv (const std::string &privs) const
{
	// priv vs privs concept added by Sadge to allow all players to access commands that have 
	// privs defined as "::"
	std::string priv;
	if (privs!="::")
		priv=privs;

	if ( _UserPriv.empty() && !priv.empty() )
	{
		return false;
	}
	if ( priv.empty() || priv.find(_UserPriv) != string::npos ) // _UserPriv looks like ":GM", priv looks like ":GM:SG:G:"
	{
		return true;
	}
	//nlwarning( "ADMIN: UserId %u UserName '%s' has priv '%s' that is not enough, he need '%s'", _UserId, _UserName.c_str(), _UserPriv.c_str(), priv.c_str() );
	return false;
}

void CPlayer::isBetaTester(bool betaTester)
{
	_BetaTester = betaTester;

	// update the client
	for (uint i = 0; i < _Characters.size(); i++)
	{
		CCharacter * c = _Characters[i];
		if (c != NULL && c->getEnterFlag() )
		{
			c->sendBetaTesterStatus();
		}
	}
}

void CPlayer::isWindermeerCommunity(bool windermeerCommunityPlayer)
{
	_WindermeerCommunity = windermeerCommunityPlayer;
	
	// update the client
	for (uint i = 0; i < _Characters.size(); i++)
	{
		CCharacter * c = _Characters[i];
		if (c != NULL && c->getEnterFlag() )
		{
			c->sendWindermeerStatus();
		}
	}
}

void CPlayer::loadOldFormat(class NLMISC::IStream &f)
{
	H_AUTO(PlyerLoadOldFormat);

	if(f.isReading())
	{
		uint32 sz;
		f.xmlPushBegin("Player");
		f.xmlSetAttrib ("NBCharacters");
		f.serial(sz);
		f.xmlPushEnd();

		uint32 version;
		f.xmlPush("PlayerVersion");
		f.serial( version );
		f.xmlPop();

		nlassert( sz <= _Characters.size() );

		uint i;
		for( i = 0; i < sz; i++ )
		{
			CCharacter *pCh = new CCharacter();
			pCh->setId( PlayerManager.createCharacterId( _UserId, i ) );

			f.xmlPush( ( string( "Character") + toString( i ) ).c_str() );
			pCh->serial( f );
			f.xmlPop();
			_Characters[ i ] = pCh;
		}
		f.xmlPop();
	}
	else
	{
		f.xmlPushBegin("Player");
		f.xmlSetAttrib ("NBCharacters");
		uint32 nbChar = getCharacterCount();
		f.serial(nbChar);
		f.xmlPushEnd();

		f.xmlPush("PlayerVersion");
		uint32 version = getCurrentVersion();
		f.serial( version );
		f.xmlPop();


		vector<CCharacter *>::const_iterator itCh;
		int i = 0;
		for( itCh = _Characters.begin(); itCh != _Characters.end(); ++itCh )
		{
			if( *itCh != 0 )
			{
				CCharacter &ch = *(*itCh);
				f.xmlPush( ( string( "Character" ) + toString(i) ).c_str() );

				
				if (ch.getEnterFlag())
				{
					// backup the fame values
					CFameManager &fm = CFameManager::getInstance();
					fm.savePlayerFame(ch.getId(), ch.getPlayerFamesContainer());
				}
				ch.serial( f );
				/*if (ch.getEnterFlag())
				{
					// free the temporary container.
					ch.deletePlayerFamesContainer();
				}
				*/
				f.xmlPop();
				i++;
			}
		}
		f.xmlPop();
	}
}


NLMISC_CATEGORISED_COMMAND(egs, convertToPdr, "Load all possible characters from xml/bin save format and save to pds format","[-recurse <1|0> (default true)] [-xml <1|0> (default false)] [-overwrite <1|0> (default false)] [-wcbin <binary filewildcard>]* [-wcxml <xml filewildcard>]* [-wcexcl <exclude filewildcard>]* <source directory> [destination directory]")
{
	bool						recurse = true;
	bool						xml = false;
	bool						overwrite = false;

	std::vector<std::string>	binwildcards;
	std::vector<std::string>	xmlwildcards;
	std::vector<std::string>	excludewildcards;

	binwildcards.push_back("account_*_*.bin");
	excludewildcards.push_back("account_*_*_pdr.*");

	uint	op = 0;
	while (op < args.size() && args[op][0] == '-')
	{
		std::string	opt = args[op++];
		if (op == args.size())
			return false;
		std::string	param = args[op++];

		bool val = false;
		NLMISC::fromString(param, val);

		if (opt == "-recurse")
		{
			recurse = (param == "true" || val);
		}
		else if (opt == "-xml")
		{
			xml = (param == "true" || val);
		}
		else if (opt == "-overwrite")
		{
			overwrite = (param == "true" || val);
		}
		else if (opt == "-wcbin")
		{
			binwildcards.push_back(param);
		}
		else if (opt == "-wcxml")
		{
			xmlwildcards.push_back(param);
		}
		else if (opt == "-wcexcl")
		{
			excludewildcards.push_back(param);
		}
		else
		{
			return false;
		}
	}

	if (op == args.size())
		return false;

	std::string		srcPath = CPath::standardizePath(args[op++]);
	std::string		dstPath = srcPath;

	if (op < args.size())
		dstPath = CPath::standardizePath(args[op++]);

	if (op != args.size())
		return false;

	uint	i;
	log.displayNL("Will exclude from conversion files with following wildcards:");
	for (i=0; i<excludewildcards.size(); ++i)
		log.displayNL("- '%s'", excludewildcards[i].c_str());
	log.displayNL("Will try to convert binary files with following wildcards:");
	for (i=0; i<binwildcards.size(); ++i)
		log.displayNL("- '%s'", binwildcards[i].c_str());
	log.displayNL("Will try to convert xml files with following wildcards:");
	for (i=0; i<xmlwildcards.size(); ++i)
		log.displayNL("- '%s'", xmlwildcards[i].c_str());

	std::vector<std::string>	files;
	CPath::getPathContent(srcPath, recurse, false, true, files);

	std::vector<std::string>	faultyFiles;
	uint						convert = 0;

	for (i=0; i<files.size(); ++i)
	{
		// in the form srcPath/subPath/filename
		// file will be rewritten like dstPath/subPath/filename
		std::string	file = files[i];
		std::string	subfile = file.substr(srcPath.size());	// subfile is subPath/filename

		std::string	checkpath = file.substr(0, srcPath.size());
		if (checkpath != srcPath)
		{
			log.displayNL("Issue while loading file '%s', path is not bound to source path '%s'", file.c_str(), srcPath.c_str());
			faultyFiles.push_back(file);
			continue;
		}

		std::string	filename = CFile::getFilename(file);

		CIXml				xmlstream;
		CIFile				filestream;
		NLMISC::IStream*	stream = NULL;;

		bool	useXml = false;

		uint	j;

		// check if file is excluded
		for (j=0; j<excludewildcards.size(); ++j)
			if (testWildCard(filename, excludewildcards[j]))
				break;
		if (j != excludewildcards.size())
			continue;

		// check if file is matching bin wildcards
		for (j=0; j<binwildcards.size(); ++j)
			if (testWildCard(filename, binwildcards[j]))
				break;
		if (j != binwildcards.size())
		{
			if (!filestream.open(file))
			{
				log.displayNL("convertToPdr: FAILED to open file '%s' !", file.c_str());
				faultyFiles.push_back(file);
				continue;
			}
			stream = &filestream;
		}

		// check if file is matchin xml wildcards
		for (j=0; j<xmlwildcards.size(); ++j)
			if (testWildCard(filename, xmlwildcards[j]))
				break;

		if (stream == NULL && j != xmlwildcards.size())
		{
			if (!filestream.open(file) || !xmlstream.init(filestream))
			{
				log.displayNL("convertToPdr: FAILED to open and init xml file '%s' !", file.c_str());
				faultyFiles.push_back(file);
				continue;
			}
			stream = &xmlstream;
		}

		// did it match any wildcard?
		if (stream == NULL)
			continue;

		// check filename contains enough info to allow correct loading
		uint32	UserId, CharId;
		if (sscanf(filename.c_str(), "account_%d_%u", &UserId, &CharId) != 2)
		{
			log.displayNL("convertToPdr: FAILED to parse file '%s' name as a 'account_xxxx_x' pattern, please check wildcards for invalid syntax", file.c_str());
			faultyFiles.push_back(file);
			continue;
		}

		bool		success = false;
		std::string	reason;

		CCharacter*	character = new CCharacter();

		try
		{
			// create a new character record
			character->setId( PlayerManager.createCharacterId( UserId, CharId ) );

			stream->xmlPush( NLMISC::toString("Character%d", CharId).c_str() );
			stream->serial( *character );
			stream->xmlPop();

			static CPersistentDataRecordRyzomStore	pdr;
			pdr.clear();
			
			character->store(pdr);

			// temporary get path to write file into
			std::string	dstFile = CFile::getPath(dstPath + subfile);
			// create directory tree if path doesn't exist yet
			CFile::createDirectoryTree(dstFile);

			dstFile = CPath::standardizePath(dstFile) + toString("account_%d_%u_pdr.%s", UserId, CharId, (xml ? "xml" : "bin"));

			if (CFile::fileExists(dstFile) && !overwrite)
			{
				reason = "output file '"+dstFile+"' already exists, cannot overwrite!";
			}
			else
			{
				bool	writeSuccess = false;
				if (xml)
				{
					writeSuccess = pdr.writeToTxtFile(dstFile.c_str());
				}
				else
				{
					writeSuccess = pdr.writeToBinFile(dstFile.c_str());
				}

				// check file can be read back
				if (writeSuccess)
				{
					static CPersistentDataRecord	pdrTest;
					pdrTest.clear();
					
					if (pdrTest.readFromFile(dstFile.c_str()))
					{
						CCharacter*	characterTest = new CCharacter();
						characterTest->setId( PlayerManager.createCharacterId( UserId, CharId ) );
						characterTest->apply(pdrTest);
						delete characterTest;

						success = true;
						++convert;
					}
				}
			}
		}
		catch (const Exception& e)
		{
			reason = e.what();
		}
		catch (...)
		{
			reason = "low level exception";
		}

		delete character;

		if (!success)
		{
			log.displayNL("convertToPdr: GENERAL FAILURE when converting file '%s' to pdr format, please check for errors manually (reason: %s)", file.c_str(), reason.c_str());
			faultyFiles.push_back(file);
		}
	}

	log.displayNL("Converted %d files, %d files have generated errors:", convert, faultyFiles.size());
	for (i=0; i<faultyFiles.size(); ++i)
		log.displayNL("- %s", faultyFiles[i].c_str());
	log.displayNL("End of faulty files list");

	return true;
}
