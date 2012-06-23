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


#ifndef INCLUDED_database_GUILD_H
#define INCLUDED_database_GUILD_H
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "nel/misc/string_common.h"
#include "cdb_group.h"
#include "player_manager/cdb.h"
#include "player_manager/cdb_synchronised.h"


#include "game_share/far_position.h"
	
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, TCharConnectionState value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, TCharConnectionState &value)
{
	value = (TCharConnectionState)db.x_getProp(node);
}
			


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


	
	class CBankAccessor_GUILD : public CCDBGroup
	{
	public:
		static TCDBBank BankTag;

		
	class TGUILD
	{	
	public:
		
	class TFAME
	{	
	public:
		
	class TArray
	{	
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_VALUE;
		ICDBStructNode	*_THRESHOLD;
		ICDBStructNode	*_TREND;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setVALUE(CCDBGroup &dbGroup, sint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _VALUE, value, forceSending);
		}

		sint8 getVALUE(const CCDBGroup &dbGroup)
		{
			sint8 value;
			_getProp(dbGroup.Database, _VALUE, value);

			return value;
		}
		
		ICDBStructNode *getVALUECDBNode()
		{
			return _VALUE;
		}
	
		void setTHRESHOLD(CCDBGroup &dbGroup, sint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _THRESHOLD, value, forceSending);
		}

		sint8 getTHRESHOLD(const CCDBGroup &dbGroup)
		{
			sint8 value;
			_getProp(dbGroup.Database, _THRESHOLD, value);

			return value;
		}
		
		ICDBStructNode *getTHRESHOLDCDBNode()
		{
			return _THRESHOLD;
		}
	
		void setTREND(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _TREND, value, forceSending);
		}

		uint8 getTREND(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _TREND, value);

			return value;
		}
		
		ICDBStructNode *getTRENDCDBNode()
		{
			return _TREND;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_CULT_ALLEGIANCE;
		ICDBStructNode	*_CIV_ALLEGIANCE;
		TArray _Array[6];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setCULT_ALLEGIANCE(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setCULT_ALLEGIANCE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup.Database, _CULT_ALLEGIANCE, value, forceSending);
		}

		uint8 getCULT_ALLEGIANCE(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _CULT_ALLEGIANCE, value);

			return value;
		}
		
		ICDBStructNode *getCULT_ALLEGIANCECDBNode()
		{
			return _CULT_ALLEGIANCE;
		}
	
		void setCIV_ALLEGIANCE(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setCIV_ALLEGIANCE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup.Database, _CIV_ALLEGIANCE, value, forceSending);
		}

		uint8 getCIV_ALLEGIANCE(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _CIV_ALLEGIANCE, value);

			return value;
		}
		
		ICDBStructNode *getCIV_ALLEGIANCECDBNode()
		{
			return _CIV_ALLEGIANCE;
		}
	TArray &getArray(uint32 index)
		{
			nlassert(index < 6);
			return _Array[index];
		}
		
	};
		
	class TMEMBERS
	{	
	public:
		
	class TArray
	{	
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_NAME;
		ICDBStructNode	*_GRADE;
		ICDBStructNode	*_ONLINE;
		ICDBStructNode	*_ENTER_DATE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

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
	
		void setGRADE(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setGRADE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup.Database, _GRADE, value, forceSending);
		}

		uint8 getGRADE(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _GRADE, value);

			return value;
		}
		
		ICDBStructNode *getGRADECDBNode()
		{
			return _GRADE;
		}
	
		void setONLINE(CCDBGroup &dbGroup, TCharConnectionState value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _ONLINE, value, forceSending);
		}

		TCharConnectionState getONLINE(const CCDBGroup &dbGroup)
		{
			TCharConnectionState value;
			_getProp(dbGroup.Database, _ONLINE, value);

			return value;
		}
		
		ICDBStructNode *getONLINECDBNode()
		{
			return _ONLINE;
		}
	
		void setENTER_DATE(CCDBGroup &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _ENTER_DATE, value, forceSending);
		}

		uint32 getENTER_DATE(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _ENTER_DATE, value);

			return value;
		}
		
		ICDBStructNode *getENTER_DATECDBNode()
		{
			return _ENTER_DATE;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[256];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 256);
			return _Array[index];
		}
		
	};
		
	class TINVENTORY
	{	
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SESSION;
		ICDBStructNode	*_BULK_MAX;
		ICDBStructNode	*_MONEY;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSESSION(CCDBGroup &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _SESSION, value, forceSending);
		}

		uint16 getSESSION(const CCDBGroup &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup.Database, _SESSION, value);

			return value;
		}
		
		ICDBStructNode *getSESSIONCDBNode()
		{
			return _SESSION;
		}
	
		void setBULK_MAX(CCDBGroup &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _BULK_MAX, value, forceSending);
		}

		uint32 getBULK_MAX(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _BULK_MAX, value);

			return value;
		}
		
		ICDBStructNode *getBULK_MAXCDBNode()
		{
			return _BULK_MAX;
		}
	
		void setMONEY(CCDBGroup &dbGroup, uint64 value, bool forceSending = false)
		{
			_setProp(dbGroup.Database, _MONEY, value, forceSending);
		}

		uint64 getMONEY(const CCDBGroup &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup.Database, _MONEY, value);

			return value;
		}
		
		ICDBStructNode *getMONEYCDBNode()
		{
			return _MONEY;
		}
	
	};
		
	class TOUTPOST
	{	
	public:
		
	class TO
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
		
	class TSQUAD_SPAWN_ZONE
	{	
	public:
		
	class TArray
	{	
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_X;
		ICDBStructNode	*_Y;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setX(CCDBGroup &dbGroup, sint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _X, value, forceSending);
		}

		sint32 getX(const CCDBGroup &dbGroup)
		{
			sint32 value;
			_getProp(dbGroup.Database, _X, value);

			return value;
		}
		
		ICDBStructNode *getXCDBNode()
		{
			return _X;
		}
	
		void setY(CCDBGroup &dbGroup, sint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _Y, value, forceSending);
		}

		sint32 getY(const CCDBGroup &dbGroup)
		{
			sint32 value;
			_getProp(dbGroup.Database, _Y, value);

			return value;
		}
		
		ICDBStructNode *getYCDBNode()
		{
			return _Y;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[16];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 16);
			return _Array[index];
		}
		
	};
		
	class TSQUAD_SHOP
	{	
	public:
		
	class TArray
	{	
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		

	public:
		void init(ICDBStructNode *parent, uint index);

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
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[16];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 16);
			return _Array[index];
		}
		
	};
		
	class TSQUADS
	{	
	public:
		
	class TSP
	{	
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		

	public:
		void init(ICDBStructNode *parent, uint index);

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
	
	};
		
	class TT
	{	
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		ICDBStructNode	*_SPAWN;
		

	public:
		void init(ICDBStructNode *parent, uint index);

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
	
		void setSPAWN(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setSPAWN : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup.Database, _SPAWN, value, forceSending);
		}

		uint8 getSPAWN(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _SPAWN, value);

			return value;
		}
		
		ICDBStructNode *getSPAWNCDBNode()
		{
			return _SPAWN;
		}
	
	};

	private:
		ICDBStructNode	*_BranchNode;

		TSP __SP[24];
		TT _T[24];

	public:
		void init(ICDBStructNode *parent);

		// accessors to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TSP &getSP(uint32 index)
		{
			nlassert(index < 24);
			return __SP[index];
		}
		TT &getT(uint32 index)
		{
			nlassert(index < 24);
			return _T[index];
		}
		
	};
		
	class TBUILDINGS
	{	
	public:
		
	class TArray
	{	
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		

	public:
		void init(ICDBStructNode *parent, uint index);

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
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[4];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 4);
			return _Array[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_OWNED;
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
		ICDBStructNode	*_SQUAD_CAPITAL;
		TGUILD	_GUILD;
		TSQUAD_SPAWN_ZONE	_SQUAD_SPAWN_ZONE;
		TSQUAD_SHOP	_SQUAD_SHOP;
		TSQUADS	_SQUADS;
		TBUILDINGS	_BUILDINGS;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setOWNED(CCDBGroup &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _OWNED, value, forceSending);
		}

		bool getOWNED(const CCDBGroup &dbGroup)
		{
			bool value;
			_getProp(dbGroup.Database, _OWNED, value);

			return value;
		}
		
		ICDBStructNode *getOWNEDCDBNode()
		{
			return _OWNED;
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
	
		void setSQUAD_CAPITAL(CCDBGroup &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _SQUAD_CAPITAL, value, forceSending);
		}

		uint32 getSQUAD_CAPITAL(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _SQUAD_CAPITAL, value);

			return value;
		}
		
		ICDBStructNode *getSQUAD_CAPITALCDBNode()
		{
			return _SQUAD_CAPITAL;
		}
	TGUILD &getGUILD()
		{
			return _GUILD;
		}
		TSQUAD_SPAWN_ZONE &getSQUAD_SPAWN_ZONE()
		{
			return _SQUAD_SPAWN_ZONE;
		}
		TSQUAD_SHOP &getSQUAD_SHOP()
		{
			return _SQUAD_SHOP;
		}
		TSQUADS &getSQUADS()
		{
			return _SQUADS;
		}
		TBUILDINGS &getBUILDINGS()
		{
			return _BUILDINGS;
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_CANDEL;
		TO _O[16];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setCANDEL(CCDBGroup &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _CANDEL, value, forceSending);
		}

		bool getCANDEL(const CCDBGroup &dbGroup)
		{
			bool value;
			_getProp(dbGroup.Database, _CANDEL, value);

			return value;
		}
		
		ICDBStructNode *getCANDELCDBNode()
		{
			return _CANDEL;
		}
	TO &getO(uint32 index)
		{
			nlassert(index < 16);
			return _O[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_COUNTER;
		ICDBStructNode	*_PROXY;
		ICDBStructNode	*_NAME;
		ICDBStructNode	*_DESCRIPTION;
		ICDBStructNode	*_ICON;
		ICDBStructNode	*_XP;
		ICDBStructNode	*_CHARGE_POINTS;
		ICDBStructNode	*_VILLAGE;
		ICDBStructNode	*_PEOPLE;
		ICDBStructNode	*_CREATION_DATE;
		TFAME	_FAME;
		TMEMBERS	_MEMBERS;
		TINVENTORY	_INVENTORY;
		TOUTPOST	_OUTPOST;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setCOUNTER(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _COUNTER, value, forceSending);
		}

		uint8 getCOUNTER(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _COUNTER, value);

			return value;
		}
		
		ICDBStructNode *getCOUNTERCDBNode()
		{
			return _COUNTER;
		}
	
		void setPROXY(CCDBGroup &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _PROXY, value, forceSending);
		}

		bool getPROXY(const CCDBGroup &dbGroup)
		{
			bool value;
			_getProp(dbGroup.Database, _PROXY, value);

			return value;
		}
		
		ICDBStructNode *getPROXYCDBNode()
		{
			return _PROXY;
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
	
		void setDESCRIPTION(CCDBGroup &dbGroup, ucstring value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _DESCRIPTION, value, forceSending);
		}

		ucstring getDESCRIPTION(const CCDBGroup &dbGroup)
		{
			ucstring value;
			_getProp(dbGroup.Database, _DESCRIPTION, value);

			return value;
		}
		
		void setDESCRIPTION(CCDBGroup &dbGroup, uint32 stringId, bool forceSending = false)
		{
			_setProp(dbGroup.Database, _DESCRIPTION, stringId, forceSending);
		}
		uint32 getDESCRIPTION_id(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _DESCRIPTION, value);

			return value;
		}
		
		ICDBStructNode *getDESCRIPTIONCDBNode()
		{
			return _DESCRIPTION;
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
	
		void setXP(CCDBGroup &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _XP, value, forceSending);
		}

		uint32 getXP(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _XP, value);

			return value;
		}
		
		ICDBStructNode *getXPCDBNode()
		{
			return _XP;
		}
	
		void setCHARGE_POINTS(CCDBGroup &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _CHARGE_POINTS, value, forceSending);
		}

		uint32 getCHARGE_POINTS(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _CHARGE_POINTS, value);

			return value;
		}
		
		ICDBStructNode *getCHARGE_POINTSCDBNode()
		{
			return _CHARGE_POINTS;
		}
	
		void setVILLAGE(CCDBGroup &dbGroup, ucstring value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _VILLAGE, value, forceSending);
		}

		ucstring getVILLAGE(const CCDBGroup &dbGroup)
		{
			ucstring value;
			_getProp(dbGroup.Database, _VILLAGE, value);

			return value;
		}
		
		void setVILLAGE(CCDBGroup &dbGroup, uint32 stringId, bool forceSending = false)
		{
			_setProp(dbGroup.Database, _VILLAGE, stringId, forceSending);
		}
		uint32 getVILLAGE_id(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _VILLAGE, value);

			return value;
		}
		
		ICDBStructNode *getVILLAGECDBNode()
		{
			return _VILLAGE;
		}
	
		void setPEOPLE(CCDBGroup &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _PEOPLE, value, forceSending);
		}

		uint8 getPEOPLE(const CCDBGroup &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup.Database, _PEOPLE, value);

			return value;
		}
		
		ICDBStructNode *getPEOPLECDBNode()
		{
			return _PEOPLE;
		}
	
		void setCREATION_DATE(CCDBGroup &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup.Database, _CREATION_DATE, value, forceSending);
		}

		uint32 getCREATION_DATE(const CCDBGroup &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup.Database, _CREATION_DATE, value);

			return value;
		}
		
		ICDBStructNode *getCREATION_DATECDBNode()
		{
			return _CREATION_DATE;
		}
	TFAME &getFAME()
		{
			return _FAME;
		}
		TMEMBERS &getMEMBERS()
		{
			return _MEMBERS;
		}
		TINVENTORY &getINVENTORY()
		{
			return _INVENTORY;
		}
		TOUTPOST &getOUTPOST()
		{
			return _OUTPOST;
		}
		
	};
		
		static TGUILD	_GUILD;


	public:

		// Constructor
		CBankAccessor_GUILD()
		{
			// make sure the static tree is initialised (some kind of lazy initialisation)
			init();

			// init the base class
			CCDBGroup::init(BankTag);
		}

		
		static void init();

		static TGUILD &getGUILD()
		{
			return _GUILD;
		}
		

	};
	

#endif // INCLUDED_database_GUILD_H
