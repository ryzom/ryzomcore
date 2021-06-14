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


#ifndef INCLUDED_database_OUTPOST_H
#define INCLUDED_database_OUTPOST_H
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "nel/misc/string_common.h"
#include "cdb_group.h"
#include "player_manager/cdb.h"
#include "player_manager/cdb_synchronised.h"




#ifndef _SET_PROP_ACCESSOR_
#define _SET_PROP_ACCESSOR_
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, bool value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, uint8 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, uint16 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, uint32 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, uint64 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, sint8 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, sint16 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, sint32 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, sint64 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, const std::string &value, bool forceSending = false)
{
	db.x_setPropString(node, value, forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, const ucstring &value, bool forceSending = false)
{
	db.x_setPropString(node, value, forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, const NLMISC::CSheetId &value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value.asInt()), forceSending);
}


inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, bool &value)
{
	value = db.x_getProp(node) != 0;
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, uint8 &value)
{
	value = uint8(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, uint16 &value)
{
	value = uint16(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, uint32 &value)
{
	value = uint32(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, uint64 &value)
{
	value = db.x_getProp(node);
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, sint8 &value)
{
	value = uint8(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, sint16 &value)
{
	value = uint16(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, sint32 &value)
{
	value = uint32(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, sint64 &value)
{
	value = db.x_getProp(node);
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, std::string &value)
{
	value = db.x_getPropString(node);
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, ucstring &value)
{
	value = db.x_getPropUcstring(node);
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, NLMISC::CSheetId &value)
{
	value = uint32(db.x_getProp(node));
}
#endif // _SET_PROP_ACCESSOR_


	
	class CBankAccessor_OUTPOST : public CCDBGroup
	{
	public:
		static TCDBBank BankTag;

		
	class TOUTPOST_SELECTED
	{	
	public:
		
	class TGUILD
	{	
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_NAME;
		ICDBStructNode	*_ICON;
		ICDBStructNode	*_TRIBE;
		ICDBStructNode	*_NAME_ATT;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setNAME(CCDBGroup &dbGroup, ucstring value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _NAME, value, forceSending);
		}

		ucstring getNAME(const CCDBGroup &dbGroup)
		{
			ucstring value;
			_getProp(dbGroup.Database, _NAME, value);

			return value;
		}
		
		void setNAME(CCDBGroup &dbGroup, uint32 stringId, bool forceSending = false)
		{
			_setProp(dbGroup.Database, _NAME, stringId, forceSending);
		}
		uint32 getNAME_id(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _NAME, value);

			return value;
		}
		
		ICDBStructNode *getNAMECDBNode()
		{
			return _NAME;
		}
	
		void setICON(CCDBGroup &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _ICON, value, forceSending);
		}

		uint64 getICON(const CCDBGroup &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup.Database, _ICON, value);

			return value;
		}
		
		ICDBStructNode *getICONCDBNode()
		{
			return _ICON;
		}
	
		void setTRIBE(CCDBGroup &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _TRIBE, value, forceSending);
		}

		bool getTRIBE(const CCDBGroup &dbGroup)
		{
			bool value;
			_getProp(dbGroup.Database, _TRIBE, value);

			return value;
		}
		
		ICDBStructNode *getTRIBECDBNode()
		{
			return _TRIBE;
		}
	
		void setNAME_ATT(CCDBGroup &dbGroup, ucstring value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _NAME_ATT, value, forceSending);
		}

		ucstring getNAME_ATT(const CCDBGroup &dbGroup)
		{
			ucstring value;
			_getProp(dbGroup.Database, _NAME_ATT, value);

			return value;
		}
		
		void setNAME_ATT(CCDBGroup &dbGroup, uint32 stringId, bool forceSending = false)
		{
			_setProp(dbGroup.Database, _NAME_ATT, stringId, forceSending);
		}
		uint32 getNAME_ATT_id(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _NAME_ATT, value);

			return value;
		}
		
		ICDBStructNode *getNAME_ATTCDBNode()
		{
			return _NAME_ATT;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		ICDBStructNode	*_LEVEL;
		ICDBStructNode	*_STATUS;
		ICDBStructNode	*_STATE_END_DATE;
		ICDBStructNode	*_DISPLAY_CRASH;
		ICDBStructNode	*_WARCOST;
		ICDBStructNode	*_ROUND_LVL_THRESHOLD;
		ICDBStructNode	*_ROUND_LVL_MAX_ATT;
		ICDBStructNode	*_ROUND_LVL_MAX_DEF;
		ICDBStructNode	*_ROUND_LVL_CUR;
		ICDBStructNode	*_ROUND_ID_CUR;
		ICDBStructNode	*_ROUND_ID_MAX;
		ICDBStructNode	*_TIME_RANGE_DEF_WANTED;
		ICDBStructNode	*_TIME_RANGE_DEF;
		ICDBStructNode	*_TIME_RANGE_ATT;
		ICDBStructNode	*_TIME_RANGE_LENGTH;
		TGUILD	_GUILD;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSHEET(CCDBGroup &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _SHEET, value, forceSending);
		}

		NLMISC::CSheetId getSHEET(const CCDBGroup &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup.Database, _SHEET, value);

			return value;
		}
		
		ICDBStructNode *getSHEETCDBNode()
		{
			return _SHEET;
		}
	
		void setLEVEL(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _LEVEL, value, forceSending);
		}

		uint8 getLEVEL(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _LEVEL, value);

			return value;
		}
		
		ICDBStructNode *getLEVELCDBNode()
		{
			return _LEVEL;
		}
	
		void setSTATUS(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setSTATUS : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup.Database, _STATUS, value, forceSending);
		}

		uint8 getSTATUS(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _STATUS, value);

			return value;
		}
		
		ICDBStructNode *getSTATUSCDBNode()
		{
			return _STATUS;
		}
	
		void setSTATE_END_DATE(CCDBGroup &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _STATE_END_DATE, value, forceSending);
		}

		uint32 getSTATE_END_DATE(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _STATE_END_DATE, value);

			return value;
		}
		
		ICDBStructNode *getSTATE_END_DATECDBNode()
		{
			return _STATE_END_DATE;
		}
	
		void setDISPLAY_CRASH(CCDBGroup &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _DISPLAY_CRASH, value, forceSending);
		}

		bool getDISPLAY_CRASH(const CCDBGroup &dbGroup)
		{
			bool value;
			_getProp(dbGroup.Database, _DISPLAY_CRASH, value);

			return value;
		}
		
		ICDBStructNode *getDISPLAY_CRASHCDBNode()
		{
			return _DISPLAY_CRASH;
		}
	
		void setWARCOST(CCDBGroup &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _WARCOST, value, forceSending);
		}

		uint32 getWARCOST(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _WARCOST, value);

			return value;
		}
		
		ICDBStructNode *getWARCOSTCDBNode()
		{
			return _WARCOST;
		}
	
		void setROUND_LVL_THRESHOLD(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _ROUND_LVL_THRESHOLD, value, forceSending);
		}

		uint8 getROUND_LVL_THRESHOLD(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _ROUND_LVL_THRESHOLD, value);

			return value;
		}
		
		ICDBStructNode *getROUND_LVL_THRESHOLDCDBNode()
		{
			return _ROUND_LVL_THRESHOLD;
		}
	
		void setROUND_LVL_MAX_ATT(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _ROUND_LVL_MAX_ATT, value, forceSending);
		}

		uint8 getROUND_LVL_MAX_ATT(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _ROUND_LVL_MAX_ATT, value);

			return value;
		}
		
		ICDBStructNode *getROUND_LVL_MAX_ATTCDBNode()
		{
			return _ROUND_LVL_MAX_ATT;
		}
	
		void setROUND_LVL_MAX_DEF(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _ROUND_LVL_MAX_DEF, value, forceSending);
		}

		uint8 getROUND_LVL_MAX_DEF(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _ROUND_LVL_MAX_DEF, value);

			return value;
		}
		
		ICDBStructNode *getROUND_LVL_MAX_DEFCDBNode()
		{
			return _ROUND_LVL_MAX_DEF;
		}
	
		void setROUND_LVL_CUR(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _ROUND_LVL_CUR, value, forceSending);
		}

		uint8 getROUND_LVL_CUR(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _ROUND_LVL_CUR, value);

			return value;
		}
		
		ICDBStructNode *getROUND_LVL_CURCDBNode()
		{
			return _ROUND_LVL_CUR;
		}
	
		void setROUND_ID_CUR(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _ROUND_ID_CUR, value, forceSending);
		}

		uint8 getROUND_ID_CUR(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _ROUND_ID_CUR, value);

			return value;
		}
		
		ICDBStructNode *getROUND_ID_CURCDBNode()
		{
			return _ROUND_ID_CUR;
		}
	
		void setROUND_ID_MAX(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _ROUND_ID_MAX, value, forceSending);
		}

		uint8 getROUND_ID_MAX(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _ROUND_ID_MAX, value);

			return value;
		}
		
		ICDBStructNode *getROUND_ID_MAXCDBNode()
		{
			return _ROUND_ID_MAX;
		}
	
		void setTIME_RANGE_DEF_WANTED(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<5)-1, "setTIME_RANGE_DEF_WANTED : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 5 bits");
				

			_setProp(dbGroup.Database, _TIME_RANGE_DEF_WANTED, value, forceSending);
		}

		uint8 getTIME_RANGE_DEF_WANTED(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _TIME_RANGE_DEF_WANTED, value);

			return value;
		}
		
		ICDBStructNode *getTIME_RANGE_DEF_WANTEDCDBNode()
		{
			return _TIME_RANGE_DEF_WANTED;
		}
	
		void setTIME_RANGE_DEF(CCDBGroup &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _TIME_RANGE_DEF, value, forceSending);
		}

		uint32 getTIME_RANGE_DEF(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _TIME_RANGE_DEF, value);

			return value;
		}
		
		ICDBStructNode *getTIME_RANGE_DEFCDBNode()
		{
			return _TIME_RANGE_DEF;
		}
	
		void setTIME_RANGE_ATT(CCDBGroup &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _TIME_RANGE_ATT, value, forceSending);
		}

		uint32 getTIME_RANGE_ATT(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _TIME_RANGE_ATT, value);

			return value;
		}
		
		ICDBStructNode *getTIME_RANGE_ATTCDBNode()
		{
			return _TIME_RANGE_ATT;
		}
	
		void setTIME_RANGE_LENGTH(CCDBGroup &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _TIME_RANGE_LENGTH, value, forceSending);
		}

		uint16 getTIME_RANGE_LENGTH(const CCDBGroup &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup.Database, _TIME_RANGE_LENGTH, value);

			return value;
		}
		
		ICDBStructNode *getTIME_RANGE_LENGTHCDBNode()
		{
			return _TIME_RANGE_LENGTH;
		}
	TGUILD &getGUILD()
		{
			return _GUILD;
		}
		
	};
		
		static TOUTPOST_SELECTED	_OUTPOST_SELECTED;


	public:

		// Constructor
		CBankAccessor_OUTPOST()
		{
			// make sure the static tree is initialised (some kind of lazy initialisation)
			init();

			// init the base class
			CCDBGroup::init(BankTag);
		}

		
		static void init();

		static TOUTPOST_SELECTED &getOUTPOST_SELECTED()
		{
			return _OUTPOST_SELECTED;
		}
		

	};
	

#endif // INCLUDED_database_OUTPOST_H
