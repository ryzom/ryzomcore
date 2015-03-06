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



#ifndef NL_ENTITY_TYPES_H
#define NL_ENTITY_TYPES_H

#include "nel/misc/types_nl.h"

//#ifdef USE_BACKEND
#include "ryzom_entity_id.h"
//#else // USE_BACKEND

#include "nel/misc/common.h"

namespace CLFECOMMON {



/*
 * HALF_FREQUENCY_SENDING_TO_CLIENT
 *
 * Define or undef this macro to control the rate of sending to the clients.
 * Not defined -> one send per cycle (usually 10 Hz).
 * Defined     -> one send every two cycles (usually 5 Hz).
 * Don't forget to adjust the variable ClientBandwidth in frontend_service.cfg.
 */
#define HALF_FREQUENCY_SENDING_TO_CLIENT


/*
 * INCLUDE_FE_STATS_IN_PACKETS
 *
 * Define or undef thus macro to add or remove the statistical data included
 * in the header of each datagram to the clients
 */
#undef INCLUDE_FE_STATS_IN_PACKETS


#ifdef INCLUDE_FE_STATS_IN_PACKETS
const uint32 STAT_HEADER_SIZE = 22;
#endif


// TEMP
typedef sint32	TPacketNumber;

//#endif // USE_BACKEND


/// Partial information on dataset row for the client
typedef uint32 TClientDataSetIndex;

/// Invalid value constant
const TClientDataSetIndex INVALID_CLIENT_DATASET_INDEX = 0xFFFFF; // 2^20-1


//#define FAKE_DISCREET_PROPERTIES
#ifdef FAKE_DISCREET_PROPERTIES
const uint32 NB_FAKE_DISC_ENTITIES_UPDATED_PER_CYCLE=3;
const uint32 NB_FAKE_DISC_PROPERTIES_UPDATED_PER_CYCLE=2;
#endif


//typedef	uint64	TBKEntityId;
//typedef uint16	TFTEntityId;	// Frontend entity id type, range about 1..5000 [NB: doesn't exist any longer]
typedef uint16	TProperty;
typedef uint8	TPropIndex;

// KAE: Commented because not used
//const uint32 NB_PROPERTIES = 256; //number of TProperty elements (if you change this, change the ids of special properties in the Front-end Service!)


/// Main type for positions/distances. Unit: millimeter
typedef sint32 TCoord;

// Unlimited threshold
const TCoord MAX_THRESHOLD = (1024*1024); // about 1 km


/*
 * These properties are common to all entities
 */

// Position: TProperty AND TPropIndex 0,1,2 (x,y,z)
const TPropIndex	PROPERTY_POSITION			= 0;
const TCoord		THRESHOLD_POSITION			= MAX_THRESHOLD;

const TPropIndex	PROPERTY_POSX				= 0;

const TPropIndex	PROPERTY_POSY				= 1;

const TPropIndex	PROPERTY_POSZ				= 2;

// Orientation angle: TProperty AND TPropIndex 3
const TPropIndex	PROPERTY_ORIENTATION		= 3; // Theta
const TCoord		THRESHOLD_ORIENTATION		= 50000;

const TPropIndex	LAST_CONTINUOUS_PROPERTY	= 3;
const TPropIndex	FIRST_DISCREET_PROPINDEX	= 4;


/*
 * These properties belong to players, npc
 *
 * Note: due to some hardcoding in client interface scripts, DO NOT change these values.
 */


// Sheet
const TPropIndex	PROPERTY_SHEET				= 4;
const TCoord		THRESHOLD_SHEET				= MAX_THRESHOLD;

// Behaviour
const TPropIndex	PROPERTY_BEHAVIOUR			= 5;
const TCoord		THRESHOLD_BEHAVIOUR			= 60000;

// Name
const TPropIndex	PROPERTY_NAME_STRING_ID		= 6;
const TCoord		THRESHOLD_NAME_STRING_ID	= 100000;

// Main target
const TPropIndex	PROPERTY_TARGET_ID			= 7;
const TCoord		THRESHOLD_TARGET_ID			= 60000; // threshold not used in combat modes
const double		THRESHOLD_TARGET_ID_CLIENT_M	= 55.0;

// Mode
const TPropIndex	PROPERTY_MODE				= 8;
const TCoord		THRESHOLD_MODE				= MAX_THRESHOLD;

// Visual property A
const TPropIndex	PROPERTY_VPA				= 9;
const TCoord		THRESHOLD_VPA				= MAX_THRESHOLD;

// Visual property B
const TPropIndex	PROPERTY_VPB				= 10;
const TCoord		THRESHOLD_VPB				= MAX_THRESHOLD;

// Visual property C
const TPropIndex	PROPERTY_VPC				= 11;
const TCoord		THRESHOLD_VPC				= 10000;

// Entity mounted
const TPropIndex	PROPERTY_ENTITY_MOUNTED_ID	= 12;
const TCoord		THRESHOLD_ENTITY_MOUNTED_ID	= MAX_THRESHOLD;

// Rider entity
const TPropIndex	PROPERTY_RIDER_ENTITY_ID	= 13;
const TCoord		THRESHOLD_RIDER_ENTITY_ID	= MAX_THRESHOLD;

// Contextual properties
const TPropIndex	PROPERTY_CONTEXTUAL			= 14;
const TCoord		THRESHOLD_CONTEXTUAL		= 100000;
const TCoord		THRESHOLD_CONTEXTUAL_NPC	= MAX_THRESHOLD; // necessary for mission icon switch on/off

// Client bars (HP...)
const TPropIndex	PROPERTY_BARS				= 15;
const TCoord		THRESHOLD_BARS				= 30000;
const double		THRESHOLD_BARS_CLIENT_M		= 28.0;

// Status
//const TPropIndex	PROPERTY_STATUS				= 16;
//const TCoord		THRESHOLD_STATUS			= MAX_THRESHOLD;

// Client bars (HP...)
const TPropIndex	PROPERTY_TARGET_LIST		= 16;
const TCoord		THRESHOLD_TARGET_LIST		= (TCoord) (100000 * sqrt(2.0));	// Projectiles reach is 100 meters, must apply a sqrt(2.0)
                                                                                // fector to get the max equivalent manhatan distance

const TPropIndex	PROPERTY_TARGET_LIST_0		= 16;
const TPropIndex	PROPERTY_TARGET_LIST_1		= 17;
const TPropIndex	PROPERTY_TARGET_LIST_2		= 18;
const TPropIndex	PROPERTY_TARGET_LIST_3		= 19;

// Guild Symbol
const TPropIndex	PROPERTY_GUILD_SYMBOL		= 20;
const TCoord		THRESHOLD_GUILD_SYMBOL		= MAX_THRESHOLD;

// Guild Name Id
const TPropIndex	PROPERTY_GUILD_NAME_ID		= 21;
const TCoord		THRESHOLD_GUILD_NAME_ID		= MAX_THRESHOLD;

// Visual FX present on entity
const TPropIndex	PROPERTY_VISUAL_FX			= 22;
const TCoord		THRESHOLD_VISUAL_FX			= 30000;

// Event Faction Id
const TPropIndex	PROPERTY_EVENT_FACTION_ID	= 23;
const TCoord		THRESHOLD_EVENT_FACTION_ID	= 60000;

// PvP Mode
const TPropIndex	PROPERTY_PVP_MODE			= 24;
const TCoord		THRESHOLD_PVP_MODE			= 60000;

// PvP Clan
const TPropIndex	PROPERTY_PVP_CLAN			= 25;
const TCoord		THRESHOLD_PVP_CLAN			= 60000;

// Mount people
const TPropIndex	PROPERTY_OWNER_PEOPLE		= 26;
const TCoord		THRESHOLD_OWNER_PEOPLE		= 60000;

// outpost infos
const TPropIndex	PROPERTY_OUTPOST_INFOS		= 27;
const TCoord		THRESHOLD_OUTPOST_INFOS		= 60000;

const uint			USER_DEFINED_PROPERTY_NB_BITS	= 32;

const TProperty		INVALID_PROPERTY = 0xFFFF;
const TPropIndex	INVALID_PROP_INDEX = 0xFF;

// Number of visual properties
const uint NB_VISUAL_PROPERTIES = 28;

const uint MAX_PROPERTIES_PER_ENTITY = NB_VISUAL_PROPERTIES;


// Special constant for unassociating
const TPropIndex PROPERTY_DISASSOCIATION = (TPropIndex)(~0)-1;


// Names (debug info)
inline const char *getPropText( TPropIndex p )
{
	static const char unknowProp[]= "UNKNOWN";
	switch( p )
	{
	case PROPERTY_POSITION: return "POS";
	case PROPERTY_POSY: return "Y";
	case PROPERTY_POSZ: return "Z";
	case PROPERTY_ORIENTATION: return "THETA";
	case PROPERTY_SHEET: return "SHEET";
	case PROPERTY_BEHAVIOUR: return "BEHAVIOUR";
	case PROPERTY_NAME_STRING_ID: return "NAME";
	case PROPERTY_TARGET_ID: return "TARGET_ID";
	case PROPERTY_MODE: return "MODE";
	case PROPERTY_VPA: return "VPA";
	case PROPERTY_VPB: return "VPB";
	case PROPERTY_VPC: return "VPC";
	case PROPERTY_ENTITY_MOUNTED_ID: return "ENTITY_MOUNTED";
	case PROPERTY_RIDER_ENTITY_ID: return "RIDER_ENTITY";
	case PROPERTY_CONTEXTUAL: return "CONTEXTUAL";
	case PROPERTY_BARS: return "BARS";
	case PROPERTY_TARGET_LIST: return "TARGET_LIST";
	case PROPERTY_VISUAL_FX: return "VISUAL_FX";
	case PROPERTY_GUILD_SYMBOL: return "TARGET_GUILD_SYMBOL";
	case PROPERTY_GUILD_NAME_ID: return "TARGET_GUILD_NAME_ID";
	case PROPERTY_EVENT_FACTION_ID: return "EVENT_FACTION_ID";
	case PROPERTY_PVP_MODE: return "PVP_MODE";
	case PROPERTY_PVP_CLAN: return "PVP_CLAN";
	case PROPERTY_OWNER_PEOPLE: return "OWNER_PEOPLE";
	case PROPERTY_OUTPOST_INFOS: return "OUTPOST_INFOS";
	//case PROPERTY_STATUS: return "STATUS";
	// yoyo: cannot return a toString()!! (local object)
	default: return unknowProp;
	}
}

// Short Names (debug info)
inline const char *getPropShortText( TPropIndex p )
{
	static const char unknowProp[]= "???";
	switch( p )
	{
	case PROPERTY_POSITION: return "POS";
	case PROPERTY_POSY: return "POY";
	case PROPERTY_POSZ: return "POZ";
	case PROPERTY_ORIENTATION: return "THE";
	case PROPERTY_SHEET: return "SHT";
	case PROPERTY_BEHAVIOUR: return "BHV";
	case PROPERTY_NAME_STRING_ID: return "NME";
	case PROPERTY_TARGET_ID: return "TGT";
	case PROPERTY_MODE: return "MOD";
	case PROPERTY_VPA: return "VPA";
	case PROPERTY_VPB: return "VPB";
	case PROPERTY_VPC: return "VPC";
	case PROPERTY_ENTITY_MOUNTED_ID: return "MNT";
	case PROPERTY_RIDER_ENTITY_ID: return "RDR";
	case PROPERTY_CONTEXTUAL: return "CTX";
	case PROPERTY_BARS: return "BRS";
	case PROPERTY_TARGET_LIST: return "TLS";
	case PROPERTY_VISUAL_FX: return "VFX";
	case PROPERTY_GUILD_SYMBOL: return "TGS";
	case PROPERTY_GUILD_NAME_ID: return "TGN";
	case PROPERTY_EVENT_FACTION_ID: return "EVF";
	case PROPERTY_PVP_MODE: return "PVPM";
	case PROPERTY_PVP_CLAN: return "PVPC";
	case PROPERTY_OWNER_PEOPLE: return "OWP";
	case PROPERTY_OUTPOST_INFOS: return "OTP";
	//case PROPERTY_STATUS: return "STA";
	// yoyo: cannot return a toString()!! (local object)
	default: return unknowProp;
	}
}

/// Set an entry in the threshold table
inline void	setThreshold( TCoord *table, TPropIndex propIndex, TCoord dist, uint nbPropIndices, bool check=false )
{
	static uint NbThresholdsSet = 0; // needs to be instanciated only in FS, where the function is used
	if ( check )
	{
		if ( NbThresholdsSet != NB_VISUAL_PROPERTIES )
			nlerror( "%u distance thresholds missing", NB_VISUAL_PROPERTIES-NbThresholdsSet );
	}
	else
	{
		NbThresholdsSet += nbPropIndices;
		if ( table[propIndex] == -1 )
		{
			table[propIndex] = dist;
		}
		else if ( table[propIndex] != dist )
		{
			nlwarning( "Found two different DistThreshold %d and %d for the same propIndex %hu", table[propIndex], dist, propIndex );
		}
	}
}


/// Check if there is no unset threshold
inline void	checkThresholds()
{
	setThreshold( NULL, 0, 0, 0, true );
}

/// Initialize threshold table
inline void	initThresholdTable( TCoord *table )
{
	setThreshold( table, PROPERTY_POSITION, THRESHOLD_POSITION, 3 );
	setThreshold( table, PROPERTY_ORIENTATION, THRESHOLD_ORIENTATION, 1 );
	setThreshold( table, PROPERTY_SHEET, THRESHOLD_SHEET, 1 );
	setThreshold( table, PROPERTY_BEHAVIOUR, THRESHOLD_BEHAVIOUR, 1 );
	setThreshold( table, PROPERTY_NAME_STRING_ID, THRESHOLD_NAME_STRING_ID, 1 );
	setThreshold( table, PROPERTY_TARGET_ID, THRESHOLD_TARGET_ID, 1 );
	setThreshold( table, PROPERTY_MODE, THRESHOLD_MODE, 1 );
	setThreshold( table, PROPERTY_VPA, THRESHOLD_VPA, 1 );
	setThreshold( table, PROPERTY_VPB, THRESHOLD_VPB, 1 );
	setThreshold( table, PROPERTY_VPC, THRESHOLD_VPC, 1 );
	setThreshold( table, PROPERTY_ENTITY_MOUNTED_ID, THRESHOLD_ENTITY_MOUNTED_ID, 1 );
	setThreshold( table, PROPERTY_RIDER_ENTITY_ID, THRESHOLD_RIDER_ENTITY_ID, 1 );
	setThreshold( table, PROPERTY_CONTEXTUAL, THRESHOLD_CONTEXTUAL, 1 );
	setThreshold( table, PROPERTY_BARS, THRESHOLD_BARS, 1 );
	setThreshold( table, PROPERTY_TARGET_LIST, THRESHOLD_TARGET_LIST, 4 );
	setThreshold( table, PROPERTY_GUILD_SYMBOL, THRESHOLD_GUILD_SYMBOL, 1 );
	setThreshold( table, PROPERTY_GUILD_NAME_ID, THRESHOLD_GUILD_NAME_ID, 1 );
	setThreshold( table, PROPERTY_VISUAL_FX, THRESHOLD_VISUAL_FX, 1 );
	setThreshold( table, PROPERTY_EVENT_FACTION_ID, THRESHOLD_EVENT_FACTION_ID, 1 );
	setThreshold( table, PROPERTY_PVP_MODE, THRESHOLD_PVP_MODE, 1 );
	setThreshold( table, PROPERTY_PVP_CLAN, THRESHOLD_PVP_CLAN, 1 );
	setThreshold( table, PROPERTY_OWNER_PEOPLE, THRESHOLD_OWNER_PEOPLE, 1 );
	setThreshold( table, PROPERTY_OUTPOST_INFOS, THRESHOLD_OUTPOST_INFOS, 1 );
	checkThresholds();
}


typedef uint8	TCLEntityId;	// Client entity id type, range in 1..255

const TCLEntityId INVALID_SLOT = 0xFF;

/// Sheet identifier
typedef uint32 TSheetId;
const TSheetId INVALID_SHEETID = 0xFFFFFFFF;

const uint8 TempFakeType = 1;

class TVPNodeBase;

TVPNodeBase	*NewNode();



#define setNodePropIndex( name ) \
	get##name##node()->PropIndex = CLFECOMMON::PROPERTY_##name

/*
 * Base class for nodes of the visual property tree
 */
class TVPNodeBase
{
public:
	TVPNodeBase						*VPParent;
	TVPNodeBase						*VPA;
	TVPNodeBase						*VPB;
	CLFECOMMON::TPropIndex			PropIndex;
	bool							BranchHasPayload;

	/// Constructor
	TVPNodeBase() : VPParent(NULL), VPA(NULL), VPB(NULL), PropIndex(~0), BranchHasPayload(false) {}
	virtual ~TVPNodeBase() {}
	/// Return true if the node is root of a tree
	bool		isRoot() const { return VPParent == NULL; }

	/// Return true if the node is leaf of a tree
	bool		isLeaf() const { return PropIndex != (CLFECOMMON::TPropIndex)~0; }

	/// Return the level of the node in a tree (root=1)
	uint		getLevel() const
	{
		const TVPNodeBase *node = this;
		uint level = 0;
		do
		{
			++level;
			node = node->VPParent;
		}
		while ( node );
		return level;
	}

	// From the main root
	TVPNodeBase	*getPOSITIONnode() { return VPA; }
	TVPNodeBase *getORIENTATIONnode() { return VPB->VPA; }

	// From the discrete root (mainroot->VPB->VPB)
	TVPNodeBase *getSHEETnode() { return VPA->VPA->VPA; }
	TVPNodeBase *getBEHAVIOURnode() { return VPA->VPA->VPB->VPA; }
	TVPNodeBase *getOWNER_PEOPLEnode() { return VPA->VPA->VPB->VPB; }
	TVPNodeBase *getNAME_STRING_IDnode() { return VPA->VPB->VPA->VPA; }
	TVPNodeBase *getCONTEXTUALnode() { return VPA->VPB->VPA->VPB; }
	TVPNodeBase *getTARGET_LISTnode() { return VPA->VPB->VPB->VPA; }
	TVPNodeBase *getTARGET_IDnode() { return VPA->VPB->VPB->VPB; }
	TVPNodeBase *getMODEnode() { return VPB->VPA->VPA->VPA; }
	TVPNodeBase *getVPAnode() { return VPB->VPA->VPA->VPB; }
	TVPNodeBase *getBARSnode() { return VPB->VPA->VPB->VPA; }
	TVPNodeBase *getVISUAL_FXnode() { return VPB->VPA->VPB->VPB; }
	TVPNodeBase *getVPBnode() { return VPB->VPB->VPA->VPA; }
	TVPNodeBase *getVPCnode() { return VPB->VPB->VPA->VPB->VPA; }
	TVPNodeBase *getEVENT_FACTION_IDnode() { return VPB->VPB->VPA->VPB->VPB->VPA; }
	TVPNodeBase *getPVP_MODEnode() { return VPB->VPB->VPA->VPB->VPB->VPB->VPA; }
	TVPNodeBase *getPVP_CLANnode() { return VPB->VPB->VPA->VPB->VPB->VPB->VPB; }
	TVPNodeBase *getENTITY_MOUNTED_IDnode() { return VPB->VPB->VPB->VPA->VPA; }
	TVPNodeBase *getRIDER_ENTITY_IDnode() { return VPB->VPB->VPB->VPA->VPB; }
	TVPNodeBase *getOUTPOST_INFOSnode() { return VPB->VPB->VPB->VPB->VPA; }
	TVPNodeBase *getGUILD_SYMBOLnode() { return VPB->VPB->VPB->VPB->VPB->VPA; }
	TVPNodeBase *getGUILD_NAME_IDnode() { return VPB->VPB->VPB->VPB->VPB->VPB; }

	// WHEN ADDING A PROPERTY, CHECK IF ENUM FOR CNetworkConnection::AddNewEntity
	// BASE INDEX IS STILL VALID!
	// and don't forget to update NB_VISUAL_PROPERTIES

	// Return the number of visual properties
	uint		buildTree()
	{
		makeChildren();
		setNodePropIndex( POSITION );
		VPB->makeChildren();
		setNodePropIndex( ORIENTATION );

		TVPNodeBase *discreetRoot = VPB->VPB;
		discreetRoot->makeDescendants( 3 ); // 8 leaves + those created by additional makeChildren()
		discreetRoot->setNodePropIndex( SHEET );
		discreetRoot->VPA->VPA->VPB->makeChildren();
		discreetRoot->setNodePropIndex( BEHAVIOUR );
		discreetRoot->setNodePropIndex( OWNER_PEOPLE );
		discreetRoot->VPA->VPB->VPA->makeChildren();
		discreetRoot->setNodePropIndex( NAME_STRING_ID );
		discreetRoot->setNodePropIndex( CONTEXTUAL );
		discreetRoot->VPA->VPB->VPB->makeChildren();
		discreetRoot->setNodePropIndex( TARGET_LIST );
		discreetRoot->setNodePropIndex( TARGET_ID );
		discreetRoot->VPB->VPA->VPA->makeChildren();
		discreetRoot->setNodePropIndex( MODE );
		discreetRoot->setNodePropIndex( VPA );
		discreetRoot->VPB->VPA->VPB->makeChildren();
		discreetRoot->setNodePropIndex( BARS );
		discreetRoot->setNodePropIndex( VISUAL_FX );
		discreetRoot->VPB->VPB->VPA->makeChildren();
		discreetRoot->setNodePropIndex( VPB );
		discreetRoot->VPB->VPB->VPA->VPB->makeChildren();
		discreetRoot->setNodePropIndex( VPC );
		discreetRoot->VPB->VPB->VPA->VPB->VPB->makeChildren();
		discreetRoot->setNodePropIndex( EVENT_FACTION_ID );
		discreetRoot->VPB->VPB->VPA->VPB->VPB->VPB->makeChildren();
		discreetRoot->setNodePropIndex( PVP_MODE );
		discreetRoot->setNodePropIndex( PVP_CLAN );
		discreetRoot->VPB->VPB->VPB->makeChildren();
		discreetRoot->VPB->VPB->VPB->VPA->makeChildren();
		discreetRoot->setNodePropIndex( ENTITY_MOUNTED_ID );
		discreetRoot->setNodePropIndex( RIDER_ENTITY_ID );
		discreetRoot->VPB->VPB->VPB->VPB->makeChildren();
		discreetRoot->setNodePropIndex( OUTPOST_INFOS );
		discreetRoot->VPB->VPB->VPB->VPB->VPB->makeChildren();
		discreetRoot->setNodePropIndex( GUILD_SYMBOL );
		discreetRoot->setNodePropIndex( GUILD_NAME_ID );

		return NB_VISUAL_PROPERTIES;
	}

	void		makeChildren()
	{
		VPA = NewNode();
		VPA->VPParent = this;
		VPB = NewNode();
		VPB->VPParent = this;
	}

	void		makeDescendants( uint nbLevels )
	{
		makeChildren();
		if ( nbLevels > 1 )
		{
			VPA->makeDescendants( nbLevels-1 );
			VPB->makeDescendants( nbLevels-1 );
		}
	}

};



} // CLFECOMMON



/*
 * This coordinate may be relative (e.g. in a ship) or absolute
 */
inline CLFECOMMON::TCoord getRelativeCoordinateFrom64( uint64 posvalue )
{
	// In Most Significant DWord
	return (CLFECOMMON::TCoord)(posvalue >> 32);
}


/*
 * This coordinate is always absolute
 */
inline CLFECOMMON::TCoord getAbsoluteCoordinateFrom64( uint64 posvalue )
{
	// In Least Significant DWord
	return (CLFECOMMON::TCoord)posvalue;
}


/*
 * This coordinates are always absolute
 */
/*inline void getAbsoluteCoordinates2D( const uint64 *posvalue,
								      CLFECOMMON::TCoord& posx, CLFECOMMON::TCoord& posy )
{
	// In Least Significant DWord
	posx = (CLFECOMMON::TCoord)(posvalue[CLFECOMMON::PROPERTY_POSX]);
	posy = (CLFECOMMON::TCoord)(posvalue[CLFECOMMON::PROPERTY_POSY]);
}*/


inline std::string toHexaString(const std::vector<uint8> &v)
{
	std::string res;
	for (uint i = 0; i < v.size(); i++)
	{
		res += NLMISC::toString("%x",v[i]);
	}
	return res;
}


#endif // NL_ENTITY_TYPES_H

/* End of entity_types.h */
