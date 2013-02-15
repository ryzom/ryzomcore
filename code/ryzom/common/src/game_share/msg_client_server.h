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



#ifndef RY_MSG_CLIENT_SERVER_H
#define RY_MSG_CLIENT_SERVER_H


/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/ucstring.h"

#include "game_share/characteristics.h"
#include "game_share/character_summary.h"
#include "game_share/starting_point.h"
#include "game_share/base_types.h"
#include "game_share/chat_group.h"
#include "game_share/pvp_clan.h"
#include "game_share/far_position.h"

///////////
// CLASS //
///////////
/**
 * Message for the Target from a slot.
 * \author PUZIN Guillaume (GUIGUI)
 * \author Nevrax France
 * \date 2002
 */
class CTargetSlotMsg
{
public:
	uint32 Slot;

	void serial(NLMISC::CBitMemStream &f)
	{
		// Serialize 8bits.
		f.serial(Slot, 8);
	}
};

/**
 * Message for the Target from a slot.
 * \author PUZIN Guillaume (GUIGUI)
 * \author Nevrax France
 * \date 2002
 */
class CTargetPosMsg
{
public:
	sint32 X;
	sint32 Y;
	sint32 Z;

	void serial(NLMISC::CBitMemStream &f)
	{
		// Serialize the position.
		f.serial(X);
		f.serial(Y);
		f.serial(Z);
	}
};

/**
 * Message to ask server for a name
 * \author Matthieu 'TRAP' Besson
 * \author Nevrax France
 * \date 2003
 */
class CCheckNameMsg
{
public:
	ucstring	Name;
	TSessionId	HomeSessionId;

	void serialBitMemStream(NLMISC::CBitMemStream &f)
	{
		f.serial( Name );
		f.serial( HomeSessionId );
	}

	void serial(NLMISC::CMemStream &f)
	{
		f.serial( Name );
		f.serial( HomeSessionId );
	}
};


/**
 * Message to create a character.
 * \author PUZIN Guillaume (GUIGUI)
 * \author Nevrax France
 * \date 2002
 */
class CCreateCharMsg
{
public:
	uint8		Slot;

	NLMISC::CSheetId SheetId;

	TSessionId	Mainland; //mainland where char is
	ucstring	Name;	//character name choose by player
	uint8		People; //use people.h enum
	uint8		Sex;	//use gender.h enum

	//nb points allocated to role (0 not get, else between 1 to 3)
	uint8		NbPointFighter;
	uint8		NbPointCaster;
	uint8		NbPointCrafter;
	uint8		NbPointHarvester;

	RYZOM_STARTING_POINT::TStartPoint StartPoint; //enum of starting point choosing by player

	sint8		GabaritHeight;
	sint8		GabaritTorsoWidth ;
	sint8		GabaritArmsWidth;
	sint8		GabaritLegsWidth;
	sint8		GabaritBreastSize;

	sint8		MorphTarget1; // 0 - 7
	sint8		MorphTarget2;
	sint8		MorphTarget3;
	sint8		MorphTarget4;
	sint8		MorphTarget5;
	sint8		MorphTarget6;
	sint8		MorphTarget7;
	sint8		MorphTarget8;
	sint8		EyesColor;	  // 0 - 7
	sint8		Tattoo;		  // 0 = neutral, 1 - 15 Tattoo

	// hair
	sint8		HairType;	  // 0 - 3
	sint8		HairColor;	  // 0 - 5

	// color for equipement slots (Only for pre-equipped perso created with sheet)
	sint8		JacketColor;
	sint8		TrousersColor;
	sint8		HatColor;
	sint8		ArmsColor;
	sint8		HandsColor;
	sint8		FeetColor;

	// -----------------------------------------------------------------------------------------------------------
	// get reference to morph taget 0-7
	sint8	   &getMorphTarget(uint index)
	{
		switch(index)
		{
			case 0: return MorphTarget1;
			case 1: return MorphTarget2;
			case 2: return MorphTarget3;
			case 3: return MorphTarget4;
			case 4: return MorphTarget5;
			case 5: return MorphTarget6;
			case 6: return MorphTarget7;
			case 7: return MorphTarget8;
			default: nlassert(0); break;
		}
		return MorphTarget1;
	}

	void setupFromCharacterSummary (const CCharacterSummary &cs)
	{
		Slot = 0;
		SheetId = NLMISC::CSheetId::Unknown;
		Mainland= cs.Mainland;
		Name	= cs.Name;
		People	= cs.People;
		Sex		= cs.VisualPropA.PropertySubData.Sex;

		HairType	= cs.VisualPropA.PropertySubData.HatModel;
		HairColor	= cs.VisualPropA.PropertySubData.HatColor;

		GabaritHeight		= cs.VisualPropC.PropertySubData.CharacterHeight;
		GabaritTorsoWidth	= cs.VisualPropC.PropertySubData.TorsoWidth;
		GabaritArmsWidth	= cs.VisualPropC.PropertySubData.ArmsWidth;
		GabaritLegsWidth	= cs.VisualPropC.PropertySubData.LegsWidth;
		GabaritBreastSize	= cs.VisualPropC.PropertySubData.BreastSize;
		// color for equipement slots
		JacketColor		= cs.VisualPropA.PropertySubData.JacketColor;
		TrousersColor	= cs.VisualPropA.PropertySubData.TrouserColor;
		HatColor		= cs.VisualPropA.PropertySubData.HatColor;
		ArmsColor		= cs.VisualPropA.PropertySubData.ArmColor;
		HandsColor		= cs.VisualPropB.PropertySubData.HandsColor;
		FeetColor		= cs.VisualPropB.PropertySubData.FeetColor;
		// blend shapes
		MorphTarget1 = cs.VisualPropC.PropertySubData.MorphTarget1;
		MorphTarget2 = cs.VisualPropC.PropertySubData.MorphTarget2;
		MorphTarget3 = cs.VisualPropC.PropertySubData.MorphTarget3;
		MorphTarget4 = cs.VisualPropC.PropertySubData.MorphTarget4;
		MorphTarget5 = cs.VisualPropC.PropertySubData.MorphTarget5;
		MorphTarget6 = cs.VisualPropC.PropertySubData.MorphTarget6;
		MorphTarget7 = cs.VisualPropC.PropertySubData.MorphTarget7;
		MorphTarget8 = cs.VisualPropC.PropertySubData.MorphTarget8;
		// eyes color
		EyesColor = cs.VisualPropC.PropertySubData.EyesColor;
		// tattoo number
		Tattoo = cs.VisualPropC.PropertySubData.Tattoo;
	}

	void serialBitMemStream(NLMISC::CBitMemStream &f)
	{
		f.serial(Slot);

		// Serialise SheetId, used for create character with sheet for tests
		f.serial( SheetId );

		// Serialize the user character.
		f.serial( Mainland );
		f.serial( Name );
		f.serial( People );
		f.serial( Sex );

		f.serial( NbPointFighter );
		f.serial( NbPointCaster );
		f.serial( NbPointCrafter );
		f.serial( NbPointHarvester );

		f.serialEnum( StartPoint );

		f.serial( HairType );
		f.serial( HairColor );

		f.serial( GabaritHeight		);	// 0 - 15
		f.serial( GabaritTorsoWidth );
		f.serial( GabaritArmsWidth	);
		f.serial( GabaritLegsWidth	);
		f.serial( GabaritBreastSize );

		f.serial( MorphTarget1 ); // 0 - 7
		f.serial( MorphTarget2 );
		f.serial( MorphTarget3 );
		f.serial( MorphTarget4 );
		f.serial( MorphTarget5 );
		f.serial( MorphTarget6 );
		f.serial( MorphTarget7 );
		f.serial( MorphTarget8 );
		f.serial( EyesColor );	  // 0 - 7
		f.serial( Tattoo );		  // 0 = neutral, 1 - 64 Tattoo

		// color for equipement slots (Only for pre-equipped perso created with sheet)
		f.serial( JacketColor );
		f.serial( TrousersColor );
		f.serial( HatColor );
		f.serial( ArmsColor );
		f.serial( HandsColor );
		f.serial( FeetColor );
	}

	void serial(NLMISC::CMemStream &f)
	{
		f.serial(Slot);

		// Serialise SheetId, used for create character with sheet for tests
		f.serial( SheetId );

		// Serialize the user character.
		f.serial( Mainland );
		f.serial( Name );
		f.serial( People );
		f.serial( Sex );

		f.serial( NbPointFighter );
		f.serial( NbPointCaster );
		f.serial( NbPointCrafter );
		f.serial( NbPointHarvester );

		f.serialEnum( StartPoint );

		f.serial( HairType );
		f.serial( HairColor );

		f.serial( GabaritHeight		);	// 0 - 14
		f.serial( GabaritTorsoWidth );
		f.serial( GabaritArmsWidth	);
		f.serial( GabaritLegsWidth	);
		f.serial( GabaritBreastSize );

		f.serial( MorphTarget1 ); // 0 - 7
		f.serial( MorphTarget2 );
		f.serial( MorphTarget3 );
		f.serial( MorphTarget4 );
		f.serial( MorphTarget5 );
		f.serial( MorphTarget6 );
		f.serial( MorphTarget7 );
		f.serial( MorphTarget8 );
		f.serial( EyesColor );	  // 0 - 7
		f.serial( Tattoo );		  // 0 = neutral, 1 - 15 Tattoo

		// color for equipement slots (Only for pre-equipped perso created with sheet)
		f.serial( JacketColor );
		f.serial( TrousersColor );
		f.serial( HatColor );
		f.serial( ArmsColor );
		f.serial( HandsColor );
		f.serial( FeetColor );
	}

	void dump()
	{
		nlinfo("Slot    = %d", Slot);
		nlinfo("SheetId = %d", SheetId.asInt());
		nlinfo("Mainland= %d", Mainland.asInt());
		nlinfo("Name    = %s", Name.toString().c_str());
		nlinfo("People  = %d", People);
		nlinfo("Sex     = %d", Sex);
		nlinfo("StartPoint = %d", StartPoint);

		nlinfo("Nb Point allocated to Fighter Role: %d", NbPointFighter );
		nlinfo("Nb Point allocated to Caster Role: %d", NbPointCaster );
		nlinfo("Nb Point allocated to Crafter Role: %d", NbPointCrafter );
		nlinfo("Nb Point allocated to Harvester Role: %d", NbPointHarvester );

		nlinfo("GabaritHeight     = %d", GabaritHeight);
		nlinfo("GabaritTorsoWidth = %d", GabaritTorsoWidth);
		nlinfo("GabaritArmsWidth  = %d", GabaritArmsWidth);
		nlinfo("GabaritLegsWidth  = %d", GabaritLegsWidth);
		nlinfo("GabaritBreastSize = %d", GabaritBreastSize);

		nlinfo("MorphTarget1 = %d", MorphTarget1);
		nlinfo("MorphTarget2 = %d", MorphTarget2);
		nlinfo("MorphTarget3 = %d", MorphTarget3);
		nlinfo("MorphTarget4 = %d", MorphTarget4);
		nlinfo("MorphTarget5 = %d", MorphTarget5);
		nlinfo("MorphTarget6 = %d", MorphTarget6);
		nlinfo("MorphTarget7 = %d", MorphTarget7);
		nlinfo("MorphTarget8 = %d", MorphTarget8);

		nlinfo("EyesColor = %d", EyesColor);
		nlinfo("Tattoo = %d", Tattoo);

		nlinfo("HairType = %d", HairType);
		nlinfo("HairColor     = %d", HairColor);

		nlinfo("JacketColor   = %d", JacketColor);
		nlinfo("TrousersColor = %d", TrousersColor);
		nlinfo("HatColor      = %d", HatColor);
		nlinfo("ArmsColor     = %d", ArmsColor);
		nlinfo("HandsColor    = %d", HandsColor);
		nlinfo("FeetColor     = %d", FeetColor);
	}
};

/**
 * Error Message to create a character.
 * All field not true is an error
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CCreateCharErrorMsg
{
public:
	bool		Slot;
	bool		Mainland;
	bool		Name;
	bool		People;
	bool		Role;
	bool		Sex;
	bool		StartPoint;

	bool		Characteristics[ CHARACTERISTICS::NUM_CHARACTERISTICS ];

	bool		GabaritHeight;
	bool		GabaritTorsoWidth ;
	bool		GabaritLegsWidth;
	bool		GabaritArmsWidth;

	// color for equipement slots
	bool		JacketColor;
	bool		TrousersColor;
	bool		HatColor;
	bool		ArmsColor;
	bool		HandsColor;
	bool		FeetColor;

	// hair
	bool		HairType;
	bool		HairColor;

	// Facial detail
	bool		FacialMorphDetail;	// 24 lower bits is used

	void serialBitMemStream(NLMISC::CBitMemStream &f)
	{
		// Serialize the user character.
		f.serial( Slot );
		f.serial( Mainland );
		f.serial( Name );
		f.serial( People );
		f.serial( Role );
		f.serial( Sex );
		f.serial( StartPoint );

		for( int i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
		{
			f.serial( Characteristics[ i ] );
		}

		f.serial( HairType );
		f.serial( HairColor );
		f.serial( GabaritHeight );
		f.serial( GabaritTorsoWidth );
		f.serial( GabaritLegsWidth );
		f.serial( GabaritArmsWidth );
		f.serial( JacketColor );
		f.serial( TrousersColor );
		f.serial( HatColor );
		f.serial( ArmsColor );
		f.serial( HandsColor );
		f.serial( FeetColor );
		f.serial( FacialMorphDetail );
	}

	void serial(NLMISC::CMemStream &f)
	{
		// Serialize the user character.
		f.serial( Slot );
		f.serial( Mainland );
		f.serial( Name );
		f.serial( People );
		f.serial( Role );
		f.serial( Sex );
		f.serial( StartPoint );

		for( int i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
		{
			f.serial( Characteristics[ i ] );
		}

		f.serial( HairType );
		f.serial( HairColor );
		f.serial( GabaritHeight );
		f.serial( GabaritTorsoWidth );
		f.serial( GabaritLegsWidth );
		f.serial( GabaritArmsWidth );
		f.serial( JacketColor );
		f.serial( TrousersColor );
		f.serial( HatColor );
		f.serial( ArmsColor );
		f.serial( HandsColor );
		f.serial( FeetColor );
		f.serial( FacialMorphDetail );
	}

	CCreateCharErrorMsg()
	{
		Slot = true;
		Mainland = true;
		Name = true;
		People = true;
		Role = true;
		Sex = true;
		StartPoint = true;

		for( int i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
		{
			Characteristics[ i ] = true;
		}

		HairType = true;
		HairColor = true;
		GabaritHeight = true;
		GabaritTorsoWidth = true;
		GabaritLegsWidth = true;
		GabaritArmsWidth = true;
		JacketColor = true;
		TrousersColor = true;
		HatColor = true;
		ArmsColor = true;
		HandsColor = true;
		FeetColor = true;
		FacialMorphDetail = true;
	}
};

/**
 * Message for select a existing character.
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CSelectCharMsg
{
public:
	uint8	c;

	void serial(NLMISC::CBitMemStream &f)
	{
		// Serialize the selected character.
		f.serial(c);
	}
};

/**
 * Message for the character information at the beginning.
 * \author PUZIN Guillaume (GUIGUI)
 * \author Nevrax France
 * \date 2002
 */
class CUserCharMsg
{
public:

	static void write(NLMISC::CBitMemStream &s, const COfflineEntityState& state, uint8 season, uint32 userRole, bool isInRingSession, TSessionId highestMainlandSessionId, uint32 firstConnectedTime, uint32 playedTime)
	{
		nlWrite( s, serial, state );
		uint32 v = (uint32)season; // can be 0 for "auto"
		s.serial( v, 3 );
		v = (uint32)userRole & 0x3; // bits 0-1
		v |= (((uint32)isInRingSession) << 2); // bit 2
		s.serial( v, 3 );
		s.serial( highestMainlandSessionId );
		s.serial( firstConnectedTime );
		s.serial( playedTime );
	}

	static void read(NLMISC::CBitMemStream &s, COfflineEntityState& state, uint8& season, uint32& userRole, bool& isInRingSession, TSessionId& highestMainlandSessionId, uint32& firstConnectedTime, uint32& playedTime)
	{
		nlRead( s, serial, state );
		uint32 v = 0;
		s.serial( v, 3 );
		season = (uint8)v;
		v = 0;
		s.serial( v, 3 );
		userRole = (v & 0x3); // bits 0-1
		isInRingSession = ((v & 0x4) != 0); // bit 2
		s.serial( highestMainlandSessionId );
		s.serial( firstConnectedTime );
		s.serial( playedTime );
	}
};

/**
 * Message for the position.
 * \author PUZIN Guillaume (GUIGUI)
 * \author Nevrax France
 * \date 2002
 */
class CPositionMsg
{
public:
	sint32	X;
	sint32	Y;
	sint32	Z;
	float	Heading;

	void serial(NLMISC::CBitMemStream &f)
	{
		// Serialize the user character.
		f.serial(X);
		f.serial(Y);
		f.serial(Z);
		f.serial(Heading);
	}
};

/**
 * Message to select a memorised phrase.
 * \author PUZIN Guillaume (GUIGUI)
 * \author Nevrax France
 * \date 2002
 */
class CMemPhraseMsg
{
public:
	uint8 NumPhrase;

	void serial(NLMISC::CBitMemStream &f)
	{
		// Serialize the number of the phrase to select.
		f.serial(NumPhrase);
	}
};


/**
 * CNewDynamicStringMsg
 * Message to add a dynamic string in string database
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CNewDynamicStringMsg
{
public:
	uint32 DynamicStringIndex;
	std::string DynamicString;

	void serial(NLMISC::CBitMemStream &f)
	{
		f.serial( DynamicStringIndex );
		f.serial( DynamicString );
	}
};



/**
 * CChatMsg
 * Message to chat
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CChatMsg
{
public:
	TDataSetIndex	CompressedIndex;
	uint32			SenderNameId;
	uint8			ChatMode;
//	uint32			DynChatChanID;
	NLMISC::CEntityId	DynChatChanID;
	ucstring		Content;

	CChatMsg()
	{
		CompressedIndex = INVALID_DATASET_INDEX;
		SenderNameId = 0;
		ChatMode = 0;
		DynChatChanID = NLMISC::CEntityId::Unknown;
	}

	void serial(NLMISC::CBitMemStream &f)
	{
		f.serial( CompressedIndex );
		f.serial( SenderNameId );
		f.serial( ChatMode );
		if(ChatMode==CChatGroup::dyn_chat)
			f.serial(DynChatChanID);
		f.serial( Content );
	}
};

/**
 * CChatMsg2
 * Message to chat
 *
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2002
 */
class CChatMsg2
{
public:
	TDataSetIndex	CompressedIndex;
	uint32			SenderNameId;
	uint8			ChatMode;
	uint32			PhraseId;
	ucstring		CustomTxt;

	CChatMsg2()
	{
		CompressedIndex = INVALID_DATASET_INDEX;
		SenderNameId = 0;
		ChatMode = 0;
		PhraseId = 0;
		CustomTxt = "";
	}

	void serial(NLMISC::CBitMemStream &f)
	{
		f.serial( CompressedIndex );
		f.serial( SenderNameId );
		f.serial( ChatMode );
		f.serial( PhraseId );
		f.serial( CustomTxt );
	}
};

/**
 * CFarTell
 * Message for far tell
 *
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2006
 */
class CFarTellMsg
{
public:
	ucstring		SenderName;
	ucstring		Text;

	void serial(NLMISC::CBitMemStream &f)
	{
		f.serial( SenderName);
		f.serial( Text );
	}
};


/**
 * CRespawnPointsMsg
 * Message from server to client indicating all valid respawn points.
 * The local vector of points is reseted if the flag need reset is set.
 * The respawn points are added to the local vector of points.
 *
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date December 2003
 */
class CRespawnPointsMsg
{
public:
	struct SRespawnPoint
	{
		sint32 x, y;

		SRespawnPoint() { x = y = 0; }
		SRespawnPoint(sint32 nX, sint32 nY) { x = nX; y = nY;}

		void serial(NLMISC::IStream &f)
		{
			f.serial(x);
			f.serial(y);
		}
	};

	bool						NeedToReset;
	std::vector<SRespawnPoint>	RespawnPoints;

	// ----------------------------------------

	CRespawnPointsMsg()
	{
		NeedToReset = false;
	}

	void serial(NLMISC::CBitMemStream &f)
	{
		f.serial(NeedToReset);
		f.serialCont(RespawnPoints);
	}
};



/**
 * CFactionWarsMsg
 * Message from server to client indicating all faction wars.
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date December 2005
 */
class CFactionWarsMsg
{
public:

	/// faction wars
	std::vector<PVP_CLAN::CFactionWar>	FactionWarOccurs;

	void serial(NLMISC::CBitMemStream &f)
	{
		f.serialCont(FactionWarOccurs);
	}
};


namespace NPC_ICON
{
	/**
	 * State of a NPC Mission Giver regarding icons
	 */
	enum TNPCMissionGiverState
	{
		AwaitingFirstData,
		NotAMissionGiver,
		ListHasOutOfReachMissions,
		ListHasAlreadyTakenMissions,
		ListHasAvailableMission,
		AutoHasUnavailableMissions,
		AutoHasAvailableMission,

		NbMissionGiverStates
	};

	const NLMISC::TGameCycle DefaultClientNPCIconRefreshTimerDelayGC = 60*10; // 1 min
}


#endif // RY_MSG_CLIENT_SERVER_H

/* End of msg_client_server.h */


