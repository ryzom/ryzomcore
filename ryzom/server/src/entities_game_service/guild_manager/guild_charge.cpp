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
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "nel/net/service.h"

#include "game_share/fame.h"

#include "guild_manager/fame_manager.h"
#include "egs_mirror.h"

#include "guild_manager/guild_charge.h"
#include "player_manager/character.h"
#include "mission_manager/mission_manager.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(CGuildCharge);

bool CGuildCharge::build( NLMISC::TStringId civ, const std::vector<std::string> & params )
{
	/// todo charge
	return false;
	/*
	if ( _Name.empty() )
	{
		nlwarning("<build> AI sent an empty charge name");
		return false;
	}
	_FileName = COutpostManager::getInstance().getChargeSavedFilePath() + NLMISC::strlwr( _Name ) + ".xml";
//	_FameId = civ;
	_Faction = CStaticFames::getInstance().getFactionIndex(CStringMapper::unmap(civ));

	_Mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( _Name );
	if ( !CMissionManager::getInstance()->getTemplate( _Mission ) )
	{
		nlwarning("<CHARGE> invalid mission %s", _Name.c_str() );
		return false;
	}
	
	// try to load the charge dynamic data
	try
	{
		CIFile f;
		if( f.open(_FileName,false) )
		{
			CIXml input;
			if( input.init( f ) )
			{
				xmlSerial( input );
				if ( _Owner != CGuild::InvalidGuildPtr )
					_Owner->setOwnedCharge( this );
				
				// applicants are not null : they just have been loaded
				for ( uint i = 0; i < _Applicants.size(); i++ )
				{
					_Applicants[i]->addAppliedCharge( this );
				}
				f.close();
				return true;
			}
			// Close the File.
			f.close();
			return false;
		}
		else
		{
			nlinfo("<CGuildCharge build>  :  file '%s' not found : should be the first time we use this charge",_FileName.c_str());
			return true;
		}
	}
	catch (const Exception & e)
	{
		nlwarning("<CGuildCharge build> file '%s' exception '%s'",_FileName.c_str(),e.what());
		return false;
	}
	*/
}// CGuildCharge::build

void CGuildCharge::xmlSerial( NLMISC::IStream & f )
{
	/// todo charge
	/*
	f.xmlPush( "charge_data" );		
		f.xmlPush("version");
			uint32 version;
			if ( !f.isReading() )
				version = getCurrentVersion();
			f.serial(version);
		f.xmlPop();
		
		f.xmlPush("owner");
			if ( !f.isReading()  )
			{
				uint32 owner = _Owner->getId();
				f.serial(owner);
			}
			else
			{
				uint32 owner;
				f.serial(owner);
				_Owner = CGuildManager::getInstance()->getGuildFromId( owner );
				if ( _Owner == NULL )
					_Owner = CGuild::InvalidGuildPtr;
			}
		f.xmlPop();

		f.xmlPush("applicants");
			f.xmlPush("size");
				uint32 size;
				if ( f.isReading() )
				{
					f.serial(size);
					_Applicants.reserve(size);
				}
				else
				{
					size = _Applicants.size();
					f.serial(size);
				}
			f.xmlPop();
			for ( uint i = 0; i < size; i++ )
			{
			f.xmlPush( NLMISC::toString("applicant%u",i + 1).c_str() );
				if ( !f.isReading()  )
				{
					uint32 owner = _Applicants[i]->getId();
					f.serial(owner);
				}
				else
				{
					uint32 owner;
					f.serial(owner);
					CGuild * guild = CGuildManager::getInstance()->getGuildFromId( owner );
					if ( guild != NULL )
						_Applicants.push_back( guild );
				}
			f.xmlPop();
			}
		f.xmlPop();
	f.xmlPop();
	*/

}// CGuildCharge::xmlSerial

void CGuildCharge::save()
{
	/// todo charge
	/*
	/// If AI has not sent charge info, we bail out
	if ( _FileName.empty() )
		return;
	if(UseBS)
	{
		try
		{
			CMemStream f;
			COXml output;
			if( output.init( &f , "1.0"))
			{
				{
					H_AUTO(SaveGuildChargeSerial);
					// save the guild
					xmlSerial( output );
				}
				{
					H_AUTO(SaveGuildChargeFlush);
					// flush the stream, write all the output file
					output.flush();
				}
				{
					H_AUTO(SaveGuildChargeSendMessage);
					CBackupMsgSaveFile msg( _Filename, CBackupMsgSaveFile::SaveFile, Bsi );
					msg.DataMsg.serialBuffer((uint8*)f.buffer(), f.length());
					Bsi.sendFile( msg );
				}
			}
		}
		catch (const Exception & e)
		{
			nlwarning("<CGuildCharge save> file %s exception '%s'",_FileName.c_str(),e.what());
		}
	}
	else
	{
		// TODO: paths
		COFile f;
		if (!f.open(_FileName))
		{
			nlwarning("(EGS)<CGuildCharge>  :  Can't open in write mode the file '%s'",_FileName.c_str());
			return;
		}
		try
		{
			COXml output;
			if( output.init( &f , "1.0"))
			{
				{
					H_AUTO(SaveGuildChargeSerial);
					// save the charge
					xmlSerial( output );
				}
				{
					H_AUTO(SaveGuildChargeFlush);
					// flush the stream, write all the output file
					output.flush();
				}
			}
		}
		catch(const Exception &)
		{
			//f.close();
			nlwarning("(EGS)<CPlayerManager::savePlayer>  :  Can't write file %s (disk full ?)",_FileName.c_str());
			return;
		}
	}
	*/
}// CGuildCharge::save

void CGuildCharge::cycleUpdate()
{
	/// todo charge
	/*
	H_AUTO(CGuildChargeCycleUpdate);
	// remove the current owner
	TVectorParamCheck empty;
	if ( _Owner != CGuild::InvalidGuildPtr )
	{		
		if ( _Outpost )
			_Outpost->resetOwner();

		_Owner->endCharge();
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::dyn_string_id);
		for ( uint16 i = 0; i < _Owner->getMemberCount(); i++ )
		{
			CEntityId id;
			if ( _Owner->getMemberEid( i,id ) )
			{
				params[0].StringId = STRING_MANAGER::sendStringToClient( TheDataset.getDataSetRow(id), getTitleText(),empty );
				CCharacter::sendDynamicSystemMessage( id,"CHARGE_END", params );
			}
		}
		_Owner->updateAllUserChargeDb();
		_Owner = CGuild::InvalidGuildPtr;
	}
	// get the best applicant and set it as owner
	CGuild * guild = CGuild::InvalidGuildPtr;
	sint32 fame = (sint32)0x80000000;
	bool alreadyDone = true;
	for ( uint i = 0; i < _Applicants.size(); i++ )
	{
		if ( _Applicants[i] == CGuild::InvalidGuildPtr )
		{
			nlwarning("<CGuildDuty cycleUpdate> Invalid applicant %u, guild id = %u",i,_Applicants[i]->getId());
		}
		else
		{
			const NLMISC::CEntityId & id = _Applicants[i]->getEntityId();
			sint32 fameBuf = CFameInterface::getInstance().getFameIndexed( id, _Faction );

			bool selected = false;
			if ( alreadyDone )
			{
				if ( !_Applicants[i]->hasCompletedCharge( _Name ) )
				{
					selected = true;
					alreadyDone = false;
				}
				else if ( fameBuf > fame && _Applicants[i]->getOwnedCharge() == NULL )
					selected = true;
			}
			else
			{
				if ( !_Applicants[i]->hasCompletedCharge( _Name ) &&
					 ( fameBuf > fame && _Applicants[i]->getOwnedCharge() == NULL ) )
				{
					selected = true;
				}

			}

			if ( selected )
			{
				fame = fameBuf;
				guild = _Applicants[i];
			}
			_Applicants[i]->removeAppliedCharge(this);
		}
	}

	_Applicants.clear();
	_Owner = guild;
	if ( guild != CGuild::InvalidGuildPtr )
	{
		_Owner->beginCharge( this,_Giver );
		if ( _Outpost )
		  _Outpost->setOwner( guild );
	}
	*/
}// CGuildCharge::cycleUpdate

void CGuildCharge::sendTexts( const TDataSetRow &userId, uint32 & title, uint32& details )
{
	/// todo charge
	/*
//	TVectorParamCheck vect;
	SM_STATIC_PARAMS_2(vect, STRING_MANAGER::bot, STRING_MANAGER::place);
	vect[0].EId = CAIAliasTranslator::getInstance()->getEntityId(_Giver);
	if (_Outpost != NULL)
		vect[1].Identifier = _Outpost->getName();
	else
		vect[1].Identifier = "";
	/// send title
	title = STRING_MANAGER::sendStringToClient( userId,_TextTitle, vect );
	
	// send details ( 2 cases : there are applicants or not )
	uint guildNameId = 0;
	sint32 fame = 0x7FFF;
	for ( uint i = 0; i < _Applicants.size(); i++ )
	{
		if (_Applicants[i] == CGuild::InvalidGuildPtr )
		{
			nlwarning("<CGuildDuty sendTexts> Invalid applicant %u, guild id = %u",i,_Applicants[i]->getId());
		}
		else
		{
			const NLMISC::CEntityId & id = _Applicants[i]->getEntityId();
			sint32 fameBuf = CFameInterface::getInstance().getFameIndexed( id, _Faction );
			if ( fameBuf > fame )
			{
				fame = fameBuf;
				guildNameId = _Applicants[i]->getNameId();
			}
		}
	}

//	SM_STATIC_PARAMS_2(vect2, STRING_MANAGER::string_id, STRING_MANAGER::integer);
//	vect2[0].StringId = guildNameId;
//	vect2[1].Int = fame;

	details = STRING_MANAGER::sendStringToClient( userId,_TextDetails, vect );
	*/
}// CGuildCharge::sendTexts

