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


#ifndef INCLUDED_DATABASE_PLR_H
#define INCLUDED_DATABASE_PLR_H
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


	
	class CBankAccessor_PLR : public CCDBSynchronised
	{
	public:
		static TCDBBank BankTag;

		
	class TGameTime
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_Hours;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setHours(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _Hours, value, forceSending);
		}

		uint16 getHours(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _Hours, value);

			return value;
		}
		
		ICDBStructNode *getHoursCDBNode()
		{
			return _Hours;
		}
	
	};
		
	class TINTERFACES
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_FLAGS;
		ICDBStructNode	*_NB_BONUS_LANDMARKS;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setFLAGS(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _FLAGS, value, forceSending);
		}

		uint64 getFLAGS(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _FLAGS, value);

			return value;
		}
		
		ICDBStructNode *getFLAGSCDBNode()
		{
			return _FLAGS;
		}
	
		void setNB_BONUS_LANDMARKS(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NB_BONUS_LANDMARKS, value, forceSending);
		}

		uint16 getNB_BONUS_LANDMARKS(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _NB_BONUS_LANDMARKS, value);

			return value;
		}
		
		ICDBStructNode *getNB_BONUS_LANDMARKSCDBNode()
		{
			return _NB_BONUS_LANDMARKS;
		}
	
	};
		
	class TUSER
	{
	public:
		
	class TSKILL_POINTS_
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_VALUE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setVALUE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VALUE, value, forceSending);
		}

		uint32 getVALUE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _VALUE, value);

			return value;
		}
		
		ICDBStructNode *getVALUECDBNode()
		{
			return _VALUE;
		}
	
	};
		
	class TFACTION_POINTS_
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_VALUE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setVALUE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VALUE, value, forceSending);
		}

		uint32 getVALUE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _VALUE, value);

			return value;
		}
		
		ICDBStructNode *getVALUECDBNode()
		{
			return _VALUE;
		}
	
	};
		
	class TRRPS_LEVELS
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_VALUE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setVALUE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VALUE, value, forceSending);
		}

		uint32 getVALUE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _VALUE, value);

			return value;
		}
		
		ICDBStructNode *getVALUECDBNode()
		{
			return _VALUE;
		}
	
	};
		
	class TNPC_CONTROL
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		ICDBStructNode	*_RUN;
		ICDBStructNode	*_WALK;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSHEET(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SHEET, value, forceSending);
		}

		NLMISC::CSheetId getSHEET(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _SHEET, value);

			return value;
		}
		
		ICDBStructNode *getSHEETCDBNode()
		{
			return _SHEET;
		}
	
		void setRUN(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _RUN, value, forceSending);
		}

		uint32 getRUN(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _RUN, value);

			return value;
		}
		
		ICDBStructNode *getRUNCDBNode()
		{
			return _RUN;
		}
	
		void setWALK(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _WALK, value, forceSending);
		}

		uint32 getWALK(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _WALK, value);

			return value;
		}
		
		ICDBStructNode *getWALKCDBNode()
		{
			return _WALK;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_HAIR_TYPE;
		ICDBStructNode	*_HAIR_COLOR;
		ICDBStructNode	*_DEATH_XP_MALUS;
		ICDBStructNode	*_IN_DUEL;
		ICDBStructNode	*_IN_PVP_CHALLENGE;
		ICDBStructNode	*_MOUNT_WALK_SPEED;
		ICDBStructNode	*_MOUNT_RUN_SPEED;
		ICDBStructNode	*_TEAM_MEMBER;
		ICDBStructNode	*_TEAM_LEADER;
		ICDBStructNode	*_OUTPOST_ADMIN;
		ICDBStructNode	*_BERSERK;
		ICDBStructNode	*_ACT_TSTART;
		ICDBStructNode	*_ACT_TEND;
		ICDBStructNode	*_ACT_TYPE;
		ICDBStructNode	*_ACT_NUMBER;
		ICDBStructNode	*_ACT_REFUSED_NUM;
		ICDBStructNode	*_ACT_CANCELED_NUM;
		ICDBStructNode	*_SPEED_FACTOR;
		ICDBStructNode	*_SKILL_POINTS;
		ICDBStructNode	*_IS_NEWBIE;
		ICDBStructNode	*_IS_TRIAL;
		ICDBStructNode	*_DEFAULT_WEIGHT_HANDS;
		ICDBStructNode	*_IS_INVISIBLE;
		ICDBStructNode	*_COUNTER;
		TSKILL_POINTS_ _SKILL_POINTS_[4];
		TFACTION_POINTS_ _FACTION_POINTS_[6];
		TRRPS_LEVELS _RRPS_LEVELS[6];
		TNPC_CONTROL	_NPC_CONTROL;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setHAIR_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _HAIR_TYPE, value, forceSending);
		}

		uint8 getHAIR_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _HAIR_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getHAIR_TYPECDBNode()
		{
			return _HAIR_TYPE;
		}
	
		void setHAIR_COLOR(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setHAIR_COLOR : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _HAIR_COLOR, value, forceSending);
		}

		uint8 getHAIR_COLOR(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _HAIR_COLOR, value);

			return value;
		}
		
		ICDBStructNode *getHAIR_COLORCDBNode()
		{
			return _HAIR_COLOR;
		}
	
		void setDEATH_XP_MALUS(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DEATH_XP_MALUS, value, forceSending);
		}

		uint8 getDEATH_XP_MALUS(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _DEATH_XP_MALUS, value);

			return value;
		}
		
		ICDBStructNode *getDEATH_XP_MALUSCDBNode()
		{
			return _DEATH_XP_MALUS;
		}
	
		void setIN_DUEL(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _IN_DUEL, value, forceSending);
		}

		bool getIN_DUEL(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _IN_DUEL, value);

			return value;
		}
		
		ICDBStructNode *getIN_DUELCDBNode()
		{
			return _IN_DUEL;
		}
	
		void setIN_PVP_CHALLENGE(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _IN_PVP_CHALLENGE, value, forceSending);
		}

		bool getIN_PVP_CHALLENGE(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _IN_PVP_CHALLENGE, value);

			return value;
		}
		
		ICDBStructNode *getIN_PVP_CHALLENGECDBNode()
		{
			return _IN_PVP_CHALLENGE;
		}
	
		void setMOUNT_WALK_SPEED(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _MOUNT_WALK_SPEED, value, forceSending);
		}

		uint16 getMOUNT_WALK_SPEED(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _MOUNT_WALK_SPEED, value);

			return value;
		}
		
		ICDBStructNode *getMOUNT_WALK_SPEEDCDBNode()
		{
			return _MOUNT_WALK_SPEED;
		}
	
		void setMOUNT_RUN_SPEED(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _MOUNT_RUN_SPEED, value, forceSending);
		}

		uint16 getMOUNT_RUN_SPEED(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _MOUNT_RUN_SPEED, value);

			return value;
		}
		
		ICDBStructNode *getMOUNT_RUN_SPEEDCDBNode()
		{
			return _MOUNT_RUN_SPEED;
		}
	
		void setTEAM_MEMBER(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TEAM_MEMBER, value, forceSending);
		}

		bool getTEAM_MEMBER(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _TEAM_MEMBER, value);

			return value;
		}
		
		ICDBStructNode *getTEAM_MEMBERCDBNode()
		{
			return _TEAM_MEMBER;
		}
	
		void setTEAM_LEADER(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TEAM_LEADER, value, forceSending);
		}

		bool getTEAM_LEADER(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _TEAM_LEADER, value);

			return value;
		}
		
		ICDBStructNode *getTEAM_LEADERCDBNode()
		{
			return _TEAM_LEADER;
		}
	
		void setOUTPOST_ADMIN(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _OUTPOST_ADMIN, value, forceSending);
		}

		bool getOUTPOST_ADMIN(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _OUTPOST_ADMIN, value);

			return value;
		}
		
		ICDBStructNode *getOUTPOST_ADMINCDBNode()
		{
			return _OUTPOST_ADMIN;
		}
	
		void setBERSERK(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BERSERK, value, forceSending);
		}

		bool getBERSERK(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _BERSERK, value);

			return value;
		}
		
		ICDBStructNode *getBERSERKCDBNode()
		{
			return _BERSERK;
		}
	
		void setACT_TSTART(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ACT_TSTART, value, forceSending);
		}

		uint32 getACT_TSTART(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _ACT_TSTART, value);

			return value;
		}
		
		ICDBStructNode *getACT_TSTARTCDBNode()
		{
			return _ACT_TSTART;
		}
	
		void setACT_TEND(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ACT_TEND, value, forceSending);
		}

		uint32 getACT_TEND(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _ACT_TEND, value);

			return value;
		}
		
		ICDBStructNode *getACT_TENDCDBNode()
		{
			return _ACT_TEND;
		}
	
		void setACT_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setACT_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _ACT_TYPE, value, forceSending);
		}

		uint8 getACT_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _ACT_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getACT_TYPECDBNode()
		{
			return _ACT_TYPE;
		}
	
		void setACT_NUMBER(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ACT_NUMBER, value, forceSending);
		}

		uint8 getACT_NUMBER(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _ACT_NUMBER, value);

			return value;
		}
		
		ICDBStructNode *getACT_NUMBERCDBNode()
		{
			return _ACT_NUMBER;
		}
	
		void setACT_REFUSED_NUM(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ACT_REFUSED_NUM, value, forceSending);
		}

		uint8 getACT_REFUSED_NUM(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _ACT_REFUSED_NUM, value);

			return value;
		}
		
		ICDBStructNode *getACT_REFUSED_NUMCDBNode()
		{
			return _ACT_REFUSED_NUM;
		}
	
		void setACT_CANCELED_NUM(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ACT_CANCELED_NUM, value, forceSending);
		}

		uint8 getACT_CANCELED_NUM(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _ACT_CANCELED_NUM, value);

			return value;
		}
		
		ICDBStructNode *getACT_CANCELED_NUMCDBNode()
		{
			return _ACT_CANCELED_NUM;
		}
	
		void setSPEED_FACTOR(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SPEED_FACTOR, value, forceSending);
		}

		uint8 getSPEED_FACTOR(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _SPEED_FACTOR, value);

			return value;
		}
		
		ICDBStructNode *getSPEED_FACTORCDBNode()
		{
			return _SPEED_FACTOR;
		}
	
		void setSKILL_POINTS(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SKILL_POINTS, value, forceSending);
		}

		uint32 getSKILL_POINTS(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _SKILL_POINTS, value);

			return value;
		}
		
		ICDBStructNode *getSKILL_POINTSCDBNode()
		{
			return _SKILL_POINTS;
		}
	
		void setIS_NEWBIE(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _IS_NEWBIE, value, forceSending);
		}

		bool getIS_NEWBIE(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _IS_NEWBIE, value);

			return value;
		}
		
		ICDBStructNode *getIS_NEWBIECDBNode()
		{
			return _IS_NEWBIE;
		}
	
		void setIS_TRIAL(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _IS_TRIAL, value, forceSending);
		}

		bool getIS_TRIAL(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _IS_TRIAL, value);

			return value;
		}
		
		ICDBStructNode *getIS_TRIALCDBNode()
		{
			return _IS_TRIAL;
		}
	
		void setDEFAULT_WEIGHT_HANDS(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DEFAULT_WEIGHT_HANDS, value, forceSending);
		}

		uint32 getDEFAULT_WEIGHT_HANDS(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _DEFAULT_WEIGHT_HANDS, value);

			return value;
		}
		
		ICDBStructNode *getDEFAULT_WEIGHT_HANDSCDBNode()
		{
			return _DEFAULT_WEIGHT_HANDS;
		}
	
		void setIS_INVISIBLE(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			_setProp(dbGroup, _IS_INVISIBLE, value, forceSending);
		}

		bool getIS_INVISIBLE(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _IS_INVISIBLE, value);

			return value;
		}
		
		ICDBStructNode *getIS_INVISIBLECDBNode()
		{
			return _IS_INVISIBLE;
		}

		void setCOUNTER(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setCOUNTER : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _COUNTER, value, forceSending);
		}

		uint8 getCOUNTER(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _COUNTER, value);

			return value;
		}
		
		ICDBStructNode *getCOUNTERCDBNode()
		{
			return _COUNTER;
		}
	TSKILL_POINTS_ &getSKILL_POINTS_(uint32 index)
		{
			nlassert(index < 4);
			return _SKILL_POINTS_[index];
		}
		TFACTION_POINTS_ &getFACTION_POINTS_(uint32 index)
		{
			nlassert(index < 6);
			return _FACTION_POINTS_[index];
		}
		TRRPS_LEVELS &getRRPS_LEVELS(uint32 index)
		{
			nlassert(index < 6);
			return _RRPS_LEVELS[index];
		}
		TNPC_CONTROL &getNPC_CONTROL()
		{
			return _NPC_CONTROL;
		}
		
	};
		
	class TDEFENSE
	{
	public:
		
	class TSLOTS
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_MODIFIER;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setMODIFIER(CCDBSynchronised &dbGroup, sint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _MODIFIER, value, forceSending);
		}

		sint8 getMODIFIER(const CCDBSynchronised &dbGroup)
		{
			sint8 value;
			_getProp(dbGroup, _MODIFIER, value);

			return value;
		}
		
		ICDBStructNode *getMODIFIERCDBNode()
		{
			return _MODIFIER;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[6];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 6);
			return _Array[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_DEFENSE_MODE;
		ICDBStructNode	*_PROTECTED_SLOT;
		TSLOTS	_SLOTS;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setDEFENSE_MODE(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DEFENSE_MODE, value, forceSending);
		}

		bool getDEFENSE_MODE(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _DEFENSE_MODE, value);

			return value;
		}
		
		ICDBStructNode *getDEFENSE_MODECDBNode()
		{
			return _DEFENSE_MODE;
		}
	
		void setPROTECTED_SLOT(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setPROTECTED_SLOT : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _PROTECTED_SLOT, value, forceSending);
		}

		uint8 getPROTECTED_SLOT(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _PROTECTED_SLOT, value);

			return value;
		}
		
		ICDBStructNode *getPROTECTED_SLOTCDBNode()
		{
			return _PROTECTED_SLOT;
		}
	TSLOTS &getSLOTS()
		{
			return _SLOTS;
		}
		
	};
		
	class TFLAGS
	{
	public:
		
	class TBRICK_TICK_RANGE
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TICK_RANGE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTICK_RANGE(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TICK_RANGE, value, forceSending);
		}

		uint64 getTICK_RANGE(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _TICK_RANGE, value);

			return value;
		}
		
		ICDBStructNode *getTICK_RANGECDBNode()
		{
			return _TICK_RANGE;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[64];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 64);
			return _Array[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_CRITICAL;
		ICDBStructNode	*_PARRY;
		ICDBStructNode	*_DODGE;
		TBRICK_TICK_RANGE	_BRICK_TICK_RANGE;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setCRITICAL(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _CRITICAL, value, forceSending);
		}

		uint8 getCRITICAL(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _CRITICAL, value);

			return value;
		}
		
		ICDBStructNode *getCRITICALCDBNode()
		{
			return _CRITICAL;
		}
	
		void setPARRY(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PARRY, value, forceSending);
		}

		uint8 getPARRY(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _PARRY, value);

			return value;
		}
		
		ICDBStructNode *getPARRYCDBNode()
		{
			return _PARRY;
		}
	
		void setDODGE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DODGE, value, forceSending);
		}

		uint8 getDODGE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _DODGE, value);

			return value;
		}
		
		ICDBStructNode *getDODGECDBNode()
		{
			return _DODGE;
		}

		TBRICK_TICK_RANGE &getBRICK_TICK_RANGE()
		{
			return _BRICK_TICK_RANGE;
		}

	};

	class TTARGET
	{
	public:
		
	class TBARS
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_UID;
		ICDBStructNode	*_HP;
		ICDBStructNode	*_SAP;
		ICDBStructNode	*_STA;
		ICDBStructNode	*_FOCUS;
		ICDBStructNode	*_PLAYER_LEVEL;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setUID(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<20)-1, "setUID : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 20 bits");
				

			_setProp(dbGroup, _UID, value, forceSending);
		}

		uint32 getUID(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _UID, value);

			return value;
		}
		
		ICDBStructNode *getUIDCDBNode()
		{
			return _UID;
		}
	
		void setHP(CCDBSynchronised &dbGroup, sint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _HP, value, forceSending);
		}

		sint8 getHP(const CCDBSynchronised &dbGroup)
		{
			sint8 value;
			_getProp(dbGroup, _HP, value);

			return value;
		}
		
		ICDBStructNode *getHPCDBNode()
		{
			return _HP;
		}
	
		void setSAP(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setSAP : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _SAP, value, forceSending);
		}

		uint8 getSAP(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _SAP, value);

			return value;
		}
		
		ICDBStructNode *getSAPCDBNode()
		{
			return _SAP;
		}
	
		void setSTA(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setSTA : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _STA, value, forceSending);
		}

		uint8 getSTA(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _STA, value);

			return value;
		}
		
		ICDBStructNode *getSTACDBNode()
		{
			return _STA;
		}
	
		void setFOCUS(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setFOCUS : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _FOCUS, value, forceSending);
		}

		uint8 getFOCUS(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _FOCUS, value);

			return value;
		}
		
		ICDBStructNode *getFOCUSCDBNode()
		{
			return _FOCUS;
		}
	
		void setPLAYER_LEVEL(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PLAYER_LEVEL, value, forceSending);
		}

		uint8 getPLAYER_LEVEL(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _PLAYER_LEVEL, value);

			return value;
		}
		
		ICDBStructNode *getPLAYER_LEVELCDBNode()
		{
			return _PLAYER_LEVEL;
		}
	
	};
		
	class TCONTEXT_MENU
	{
	public:
		
	class TMISSIONS_OPTIONS
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TITLE;
		ICDBStructNode	*_PLAYER_GIFT_NEEDED;
		ICDBStructNode	*_PRIORITY;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTITLE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TITLE, value, forceSending);
		}

		uint32 getTITLE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TITLE, value);

			return value;
		}
		
		ICDBStructNode *getTITLECDBNode()
		{
			return _TITLE;
		}
	
		void setPLAYER_GIFT_NEEDED(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PLAYER_GIFT_NEEDED, value, forceSending);
		}

		bool getPLAYER_GIFT_NEEDED(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _PLAYER_GIFT_NEEDED, value);

			return value;
		}
		
		ICDBStructNode *getPLAYER_GIFT_NEEDEDCDBNode()
		{
			return _PLAYER_GIFT_NEEDED;
		}
	
		void setPRIORITY(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<2)-1, "setPRIORITY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 2 bits");
				

			_setProp(dbGroup, _PRIORITY, value, forceSending);
		}

		uint8 getPRIORITY(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _PRIORITY, value);

			return value;
		}
		
		ICDBStructNode *getPRIORITYCDBNode()
		{
			return _PRIORITY;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[8];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 8);
			return _Array[index];
		}
		
	};
		
	class TMISSION_RING
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TITLE;
		ICDBStructNode	*_ID;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTITLE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TITLE, value, forceSending);
		}

		uint32 getTITLE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TITLE, value);

			return value;
		}
		
		ICDBStructNode *getTITLECDBNode()
		{
			return _TITLE;
		}
	
		void setID(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ID, value, forceSending);
		}

		uint32 getID(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _ID, value);

			return value;
		}
		
		ICDBStructNode *getIDCDBNode()
		{
			return _ID;
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

		ICDBStructNode	*_PROGRAMMES;
		ICDBStructNode	*_WEB_PAGE_TITLE;
		ICDBStructNode	*_WEB_PAGE_URL;
		ICDBStructNode	*_OUTPOST;
		ICDBStructNode	*_COUNTER;
		TMISSIONS_OPTIONS	_MISSIONS_OPTIONS;
		TMISSION_RING	_MISSION_RING;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setPROGRAMMES(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PROGRAMMES, value, forceSending);
		}

		uint32 getPROGRAMMES(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _PROGRAMMES, value);

			return value;
		}
		
		ICDBStructNode *getPROGRAMMESCDBNode()
		{
			return _PROGRAMMES;
		}
	
		void setWEB_PAGE_TITLE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _WEB_PAGE_TITLE, value, forceSending);
		}

		uint32 getWEB_PAGE_TITLE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _WEB_PAGE_TITLE, value);

			return value;
		}
		
		ICDBStructNode *getWEB_PAGE_TITLECDBNode()
		{
			return _WEB_PAGE_TITLE;
		}
	
		void setWEB_PAGE_URL(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _WEB_PAGE_URL, value, forceSending);
		}

		uint32 getWEB_PAGE_URL(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _WEB_PAGE_URL, value);

			return value;
		}
		
		ICDBStructNode *getWEB_PAGE_URLCDBNode()
		{
			return _WEB_PAGE_URL;
		}
	
		void setOUTPOST(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _OUTPOST, value, forceSending);
		}

		NLMISC::CSheetId getOUTPOST(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _OUTPOST, value);

			return value;
		}
		
		ICDBStructNode *getOUTPOSTCDBNode()
		{
			return _OUTPOST;
		}
	
		void setCOUNTER(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setCOUNTER : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _COUNTER, value, forceSending);
		}

		uint8 getCOUNTER(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _COUNTER, value);

			return value;
		}
		
		ICDBStructNode *getCOUNTERCDBNode()
		{
			return _COUNTER;
		}
	TMISSIONS_OPTIONS &getMISSIONS_OPTIONS()
		{
			return _MISSIONS_OPTIONS;
		}
		TMISSION_RING &getMISSION_RING()
		{
			return _MISSION_RING;
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_CONTEXT_VAL;
		ICDBStructNode	*_AGGRESSIVE;
		ICDBStructNode	*_FORCE_RATIO;
		TBARS	_BARS;
		TCONTEXT_MENU	_CONTEXT_MENU;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setCONTEXT_VAL(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _CONTEXT_VAL, value, forceSending);
		}

		uint16 getCONTEXT_VAL(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _CONTEXT_VAL, value);

			return value;
		}
		
		ICDBStructNode *getCONTEXT_VALCDBNode()
		{
			return _CONTEXT_VAL;
		}
	
		void setAGGRESSIVE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setAGGRESSIVE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _AGGRESSIVE, value, forceSending);
		}

		uint8 getAGGRESSIVE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _AGGRESSIVE, value);

			return value;
		}
		
		ICDBStructNode *getAGGRESSIVECDBNode()
		{
			return _AGGRESSIVE;
		}
	
		void setFORCE_RATIO(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setFORCE_RATIO : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _FORCE_RATIO, value, forceSending);
		}

		uint8 getFORCE_RATIO(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _FORCE_RATIO, value);

			return value;
		}
		
		ICDBStructNode *getFORCE_RATIOCDBNode()
		{
			return _FORCE_RATIO;
		}
	TBARS &getBARS()
		{
			return _BARS;
		}
		TCONTEXT_MENU &getCONTEXT_MENU()
		{
			return _CONTEXT_MENU;
		}
		
	};
		
	class TGROUP
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_PRESENT;
		ICDBStructNode	*_UID;
		ICDBStructNode	*_NAME;
		ICDBStructNode	*_HP;
		ICDBStructNode	*_SAP;
		ICDBStructNode	*_STA;
		ICDBStructNode	*_POS;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setPRESENT(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PRESENT, value, forceSending);
		}

		bool getPRESENT(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _PRESENT, value);

			return value;
		}
		
		ICDBStructNode *getPRESENTCDBNode()
		{
			return _PRESENT;
		}
	
		void setUID(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<20)-1, "setUID : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 20 bits");
				

			_setProp(dbGroup, _UID, value, forceSending);
		}

		uint32 getUID(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _UID, value);

			return value;
		}
		
		ICDBStructNode *getUIDCDBNode()
		{
			return _UID;
		}
	
		void setNAME(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NAME, value, forceSending);
		}

		uint32 getNAME(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NAME, value);

			return value;
		}
		
		ICDBStructNode *getNAMECDBNode()
		{
			return _NAME;
		}
	
		void setHP(CCDBSynchronised &dbGroup, sint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _HP, value, forceSending);
		}

		sint8 getHP(const CCDBSynchronised &dbGroup)
		{
			sint8 value;
			_getProp(dbGroup, _HP, value);

			return value;
		}
		
		ICDBStructNode *getHPCDBNode()
		{
			return _HP;
		}
	
		void setSAP(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setSAP : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _SAP, value, forceSending);
		}

		uint8 getSAP(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _SAP, value);

			return value;
		}
		
		ICDBStructNode *getSAPCDBNode()
		{
			return _SAP;
		}
	
		void setSTA(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setSTA : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _STA, value, forceSending);
		}

		uint8 getSTA(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _STA, value);

			return value;
		}
		
		ICDBStructNode *getSTACDBNode()
		{
			return _STA;
		}
	
		void setPOS(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _POS, value, forceSending);
		}

		uint64 getPOS(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _POS, value);

			return value;
		}
		
		ICDBStructNode *getPOSCDBNode()
		{
			return _POS;
		}
	
	};
		
	class TMISSIONS
	{
	public:
		
	class TArray
	{
	public:
		
	class TGOALS
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TEXT;
		ICDBStructNode	*_NPC_ALIAS;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTEXT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TEXT, value, forceSending);
		}

		uint32 getTEXT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TEXT, value);

			return value;
		}
		
		ICDBStructNode *getTEXTCDBNode()
		{
			return _TEXT;
		}
	
		void setNPC_ALIAS(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NPC_ALIAS, value, forceSending);
		}

		uint32 getNPC_ALIAS(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NPC_ALIAS, value);

			return value;
		}
		
		ICDBStructNode *getNPC_ALIASCDBNode()
		{
			return _NPC_ALIAS;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[20];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 20);
			return _Array[index];
		}
		
	};
		
	class TTARGET
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TITLE;
		ICDBStructNode	*_X;
		ICDBStructNode	*_Y;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTITLE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TITLE, value, forceSending);
		}

		uint32 getTITLE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TITLE, value);

			return value;
		}
		
		ICDBStructNode *getTITLECDBNode()
		{
			return _TITLE;
		}
	
		void setX(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _X, value, forceSending);
		}

		uint32 getX(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _X, value);

			return value;
		}
		
		ICDBStructNode *getXCDBNode()
		{
			return _X;
		}
	
		void setY(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _Y, value, forceSending);
		}

		uint32 getY(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _Y, value);

			return value;
		}
		
		ICDBStructNode *getYCDBNode()
		{
			return _Y;
		}
	
	};
		
	class THISTO
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TEXT;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTEXT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TEXT, value, forceSending);
		}

		uint32 getTEXT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TEXT, value);

			return value;
		}
		
		ICDBStructNode *getTEXTCDBNode()
		{
			return _TEXT;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[30];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 30);
			return _Array[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TYPE;
		ICDBStructNode	*_ICON;
		ICDBStructNode	*_TITLE;
		ICDBStructNode	*_DETAIL_TEXT;
		ICDBStructNode	*_BEGIN_DATE;
		ICDBStructNode	*_END_DATE;
		ICDBStructNode	*_OR_STEPS;
		ICDBStructNode	*_FINISHED;
		ICDBStructNode	*_ABANDONNABLE;
		ICDBStructNode	*_SLEEP;
		TGOALS	_GOALS;
		TTARGET _TARGET[8];
		THISTO	_HISTO;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<2)-1, "setTYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 2 bits");
				

			_setProp(dbGroup, _TYPE, value, forceSending);
		}

		uint8 getTYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _TYPE, value);

			return value;
		}
		
		ICDBStructNode *getTYPECDBNode()
		{
			return _TYPE;
		}
	
		void setICON(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ICON, value, forceSending);
		}

		NLMISC::CSheetId getICON(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _ICON, value);

			return value;
		}
		
		ICDBStructNode *getICONCDBNode()
		{
			return _ICON;
		}
	
		void setTITLE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TITLE, value, forceSending);
		}

		uint32 getTITLE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TITLE, value);

			return value;
		}
		
		ICDBStructNode *getTITLECDBNode()
		{
			return _TITLE;
		}
	
		void setDETAIL_TEXT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DETAIL_TEXT, value, forceSending);
		}

		uint32 getDETAIL_TEXT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _DETAIL_TEXT, value);

			return value;
		}
		
		ICDBStructNode *getDETAIL_TEXTCDBNode()
		{
			return _DETAIL_TEXT;
		}
	
		void setBEGIN_DATE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BEGIN_DATE, value, forceSending);
		}

		uint32 getBEGIN_DATE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _BEGIN_DATE, value);

			return value;
		}
		
		ICDBStructNode *getBEGIN_DATECDBNode()
		{
			return _BEGIN_DATE;
		}
	
		void setEND_DATE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _END_DATE, value, forceSending);
		}

		uint32 getEND_DATE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _END_DATE, value);

			return value;
		}
		
		ICDBStructNode *getEND_DATECDBNode()
		{
			return _END_DATE;
		}
	
		void setOR_STEPS(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _OR_STEPS, value, forceSending);
		}

		bool getOR_STEPS(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _OR_STEPS, value);

			return value;
		}
		
		ICDBStructNode *getOR_STEPSCDBNode()
		{
			return _OR_STEPS;
		}
	
		void setFINISHED(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<2)-1, "setFINISHED : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 2 bits");
				

			_setProp(dbGroup, _FINISHED, value, forceSending);
		}

		uint8 getFINISHED(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _FINISHED, value);

			return value;
		}
		
		ICDBStructNode *getFINISHEDCDBNode()
		{
			return _FINISHED;
		}
	
		void setABANDONNABLE(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ABANDONNABLE, value, forceSending);
		}

		bool getABANDONNABLE(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _ABANDONNABLE, value);

			return value;
		}
		
		ICDBStructNode *getABANDONNABLECDBNode()
		{
			return _ABANDONNABLE;
		}
	
		void setSLEEP(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SLEEP, value, forceSending);
		}

		bool getSLEEP(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _SLEEP, value);

			return value;
		}
		
		ICDBStructNode *getSLEEPCDBNode()
		{
			return _SLEEP;
		}
	TGOALS &getGOALS()
		{
			return _GOALS;
		}
		TTARGET &getTARGET(uint32 index)
		{
			nlassert(index < 8);
			return _TARGET[index];
		}
		THISTO &getHISTO()
		{
			return _HISTO;
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[15];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 15);
			return _Array[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_LEADER_INDEX;
		ICDBStructNode	*_SUCCESSOR_INDEX;
		TArray _Array[8];
		TMISSIONS	_MISSIONS;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setLEADER_INDEX(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setLEADER_INDEX : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _LEADER_INDEX, value, forceSending);
		}

		uint8 getLEADER_INDEX(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _LEADER_INDEX, value);

			return value;
		}
		
		ICDBStructNode *getLEADER_INDEXCDBNode()
		{
			return _LEADER_INDEX;
		}
	
		void setSUCCESSOR_INDEX(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setSUCCESSOR_INDEX : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _SUCCESSOR_INDEX, value, forceSending);
		}

		uint8 getSUCCESSOR_INDEX(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _SUCCESSOR_INDEX, value);

			return value;
		}
		
		ICDBStructNode *getSUCCESSOR_INDEXCDBNode()
		{
			return _SUCCESSOR_INDEX;
		}
	TArray &getArray(uint32 index)
		{
			nlassert(index < 8);
			return _Array[index];
		}
		TMISSIONS &getMISSIONS()
		{
			return _MISSIONS;
		}
		
	};
		
	class TDM_GIFT
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TEXT;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTEXT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TEXT, value, forceSending);
		}

		uint32 getTEXT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TEXT, value);

			return value;
		}
		
		ICDBStructNode *getTEXTCDBNode()
		{
			return _TEXT;
		}
	
	};
		
	class TEXCHANGE
	{
	public:
		
	class TGIVE
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		ICDBStructNode	*_QUALITY;
		ICDBStructNode	*_QUANTITY;
		ICDBStructNode	*_USER_COLOR;
		ICDBStructNode	*_WEIGHT;
		ICDBStructNode	*_NAMEID;
		ICDBStructNode	*_INFO_VERSION;
		ICDBStructNode	*_ENCHANT;
		ICDBStructNode	*_RM_CLASS_TYPE;
		ICDBStructNode	*_RM_FABER_STAT_TYPE;
		ICDBStructNode	*_PREREQUISIT_VALID;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSHEET(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SHEET, value, forceSending);
		}

		NLMISC::CSheetId getSHEET(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _SHEET, value);

			return value;
		}
		
		ICDBStructNode *getSHEETCDBNode()
		{
			return _SHEET;
		}
	
		void setQUALITY(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setQUALITY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _QUALITY, value, forceSending);
		}

		uint16 getQUALITY(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _QUALITY, value);

			return value;
		}
		
		ICDBStructNode *getQUALITYCDBNode()
		{
			return _QUALITY;
		}
	
		void setQUANTITY(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setQUANTITY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _QUANTITY, value, forceSending);
		}

		uint16 getQUANTITY(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _QUANTITY, value);

			return value;
		}
		
		ICDBStructNode *getQUANTITYCDBNode()
		{
			return _QUANTITY;
		}
	
		void setUSER_COLOR(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setUSER_COLOR : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _USER_COLOR, value, forceSending);
		}

		uint8 getUSER_COLOR(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _USER_COLOR, value);

			return value;
		}
		
		ICDBStructNode *getUSER_COLORCDBNode()
		{
			return _USER_COLOR;
		}
	
		void setWEIGHT(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _WEIGHT, value, forceSending);
		}

		uint16 getWEIGHT(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _WEIGHT, value);

			return value;
		}
		
		ICDBStructNode *getWEIGHTCDBNode()
		{
			return _WEIGHT;
		}
	
		void setNAMEID(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NAMEID, value, forceSending);
		}

		uint32 getNAMEID(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NAMEID, value);

			return value;
		}
		
		ICDBStructNode *getNAMEIDCDBNode()
		{
			return _NAMEID;
		}
	
		void setINFO_VERSION(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _INFO_VERSION, value, forceSending);
		}

		uint8 getINFO_VERSION(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _INFO_VERSION, value);

			return value;
		}
		
		ICDBStructNode *getINFO_VERSIONCDBNode()
		{
			return _INFO_VERSION;
		}
	
		void setENCHANT(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setENCHANT : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _ENCHANT, value, forceSending);
		}

		uint16 getENCHANT(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _ENCHANT, value);

			return value;
		}
		
		ICDBStructNode *getENCHANTCDBNode()
		{
			return _ENCHANT;
		}
	
		void setRM_CLASS_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setRM_CLASS_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _RM_CLASS_TYPE, value, forceSending);
		}

		uint8 getRM_CLASS_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _RM_CLASS_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getRM_CLASS_TYPECDBNode()
		{
			return _RM_CLASS_TYPE;
		}
	
		void setRM_FABER_STAT_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<6)-1, "setRM_FABER_STAT_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 6 bits");
				

			_setProp(dbGroup, _RM_FABER_STAT_TYPE, value, forceSending);
		}

		uint8 getRM_FABER_STAT_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _RM_FABER_STAT_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getRM_FABER_STAT_TYPECDBNode()
		{
			return _RM_FABER_STAT_TYPE;
		}
	
		void setPREREQUISIT_VALID(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PREREQUISIT_VALID, value, forceSending);
		}

		bool getPREREQUISIT_VALID(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _PREREQUISIT_VALID, value);

			return value;
		}
		
		ICDBStructNode *getPREREQUISIT_VALIDCDBNode()
		{
			return _PREREQUISIT_VALID;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[8];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 8);
			return _Array[index];
		}
		
	};
		
	class TRECEIVE
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		ICDBStructNode	*_QUALITY;
		ICDBStructNode	*_QUANTITY;
		ICDBStructNode	*_USER_COLOR;
		ICDBStructNode	*_WEIGHT;
		ICDBStructNode	*_NAMEID;
		ICDBStructNode	*_INFO_VERSION;
		ICDBStructNode	*_ENCHANT;
		ICDBStructNode	*_RM_CLASS_TYPE;
		ICDBStructNode	*_RM_FABER_STAT_TYPE;
		ICDBStructNode	*_PREREQUISIT_VALID;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSHEET(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SHEET, value, forceSending);
		}

		NLMISC::CSheetId getSHEET(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _SHEET, value);

			return value;
		}
		
		ICDBStructNode *getSHEETCDBNode()
		{
			return _SHEET;
		}
	
		void setQUALITY(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setQUALITY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _QUALITY, value, forceSending);
		}

		uint16 getQUALITY(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _QUALITY, value);

			return value;
		}
		
		ICDBStructNode *getQUALITYCDBNode()
		{
			return _QUALITY;
		}
	
		void setQUANTITY(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setQUANTITY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _QUANTITY, value, forceSending);
		}

		uint16 getQUANTITY(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _QUANTITY, value);

			return value;
		}
		
		ICDBStructNode *getQUANTITYCDBNode()
		{
			return _QUANTITY;
		}
	
		void setUSER_COLOR(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setUSER_COLOR : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _USER_COLOR, value, forceSending);
		}

		uint8 getUSER_COLOR(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _USER_COLOR, value);

			return value;
		}
		
		ICDBStructNode *getUSER_COLORCDBNode()
		{
			return _USER_COLOR;
		}
	
		void setWEIGHT(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _WEIGHT, value, forceSending);
		}

		uint16 getWEIGHT(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _WEIGHT, value);

			return value;
		}
		
		ICDBStructNode *getWEIGHTCDBNode()
		{
			return _WEIGHT;
		}
	
		void setNAMEID(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NAMEID, value, forceSending);
		}

		uint32 getNAMEID(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NAMEID, value);

			return value;
		}
		
		ICDBStructNode *getNAMEIDCDBNode()
		{
			return _NAMEID;
		}
	
		void setINFO_VERSION(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _INFO_VERSION, value, forceSending);
		}

		uint8 getINFO_VERSION(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _INFO_VERSION, value);

			return value;
		}
		
		ICDBStructNode *getINFO_VERSIONCDBNode()
		{
			return _INFO_VERSION;
		}
	
		void setENCHANT(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setENCHANT : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _ENCHANT, value, forceSending);
		}

		uint16 getENCHANT(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _ENCHANT, value);

			return value;
		}
		
		ICDBStructNode *getENCHANTCDBNode()
		{
			return _ENCHANT;
		}
	
		void setRM_CLASS_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setRM_CLASS_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _RM_CLASS_TYPE, value, forceSending);
		}

		uint8 getRM_CLASS_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _RM_CLASS_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getRM_CLASS_TYPECDBNode()
		{
			return _RM_CLASS_TYPE;
		}
	
		void setRM_FABER_STAT_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<6)-1, "setRM_FABER_STAT_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 6 bits");
				

			_setProp(dbGroup, _RM_FABER_STAT_TYPE, value, forceSending);
		}

		uint8 getRM_FABER_STAT_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _RM_FABER_STAT_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getRM_FABER_STAT_TYPECDBNode()
		{
			return _RM_FABER_STAT_TYPE;
		}
	
		void setPREREQUISIT_VALID(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PREREQUISIT_VALID, value, forceSending);
		}

		bool getPREREQUISIT_VALID(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _PREREQUISIT_VALID, value);

			return value;
		}
		
		ICDBStructNode *getPREREQUISIT_VALIDCDBNode()
		{
			return _PREREQUISIT_VALID;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[8];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 8);
			return _Array[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TEXT;
		ICDBStructNode	*_ID;
		ICDBStructNode	*_BEGUN;
		ICDBStructNode	*_ACCEPTED;
		ICDBStructNode	*_MONEY;
		ICDBStructNode	*_FORCE_REFUSE;
		ICDBStructNode	*_COUNTER;
		TGIVE	_GIVE;
		TRECEIVE	_RECEIVE;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTEXT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TEXT, value, forceSending);
		}

		uint32 getTEXT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TEXT, value);

			return value;
		}
		
		ICDBStructNode *getTEXTCDBNode()
		{
			return _TEXT;
		}
	
		void setID(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ID, value, forceSending);
		}

		uint8 getID(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _ID, value);

			return value;
		}
		
		ICDBStructNode *getIDCDBNode()
		{
			return _ID;
		}
	
		void setBEGUN(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BEGUN, value, forceSending);
		}

		bool getBEGUN(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _BEGUN, value);

			return value;
		}
		
		ICDBStructNode *getBEGUNCDBNode()
		{
			return _BEGUN;
		}
	
		void setACCEPTED(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ACCEPTED, value, forceSending);
		}

		bool getACCEPTED(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _ACCEPTED, value);

			return value;
		}
		
		ICDBStructNode *getACCEPTEDCDBNode()
		{
			return _ACCEPTED;
		}
	
		void setMONEY(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			_setProp(dbGroup, _MONEY, value, forceSending);
		}

		uint64 getMONEY(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _MONEY, value);

			return value;
		}
		
		ICDBStructNode *getMONEYCDBNode()
		{
			return _MONEY;
		}
	
		void setFORCE_REFUSE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setFORCE_REFUSE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _FORCE_REFUSE, value, forceSending);
		}

		uint8 getFORCE_REFUSE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _FORCE_REFUSE, value);

			return value;
		}
		
		ICDBStructNode *getFORCE_REFUSECDBNode()
		{
			return _FORCE_REFUSE;
		}
	
		void setCOUNTER(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setCOUNTER : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _COUNTER, value, forceSending);
		}

		uint8 getCOUNTER(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _COUNTER, value);

			return value;
		}
		
		ICDBStructNode *getCOUNTERCDBNode()
		{
			return _COUNTER;
		}
	TGIVE &getGIVE()
		{
			return _GIVE;
		}
		TRECEIVE &getRECEIVE()
		{
			return _RECEIVE;
		}
		
	};
		
	class TINVENTORY
	{
	public:
		
	class THAND
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_INDEX_IN_BAG;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setINDEX_IN_BAG(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<9)-1, "setINDEX_IN_BAG : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 9 bits");
				

			_setProp(dbGroup, _INDEX_IN_BAG, value, forceSending);
		}

		uint16 getINDEX_IN_BAG(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _INDEX_IN_BAG, value);

			return value;
		}
		
		ICDBStructNode *getINDEX_IN_BAGCDBNode()
		{
			return _INDEX_IN_BAG;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[2];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 2);
			return _Array[index];
		}
		
	};
		
	class TEQUIP
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_INDEX_IN_BAG;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setINDEX_IN_BAG(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<9)-1, "setINDEX_IN_BAG : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 9 bits");
				

			_setProp(dbGroup, _INDEX_IN_BAG, value, forceSending);
		}

		uint16 getINDEX_IN_BAG(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _INDEX_IN_BAG, value);

			return value;
		}
		
		ICDBStructNode *getINDEX_IN_BAGCDBNode()
		{
			return _INDEX_IN_BAG;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[19];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 19);
			return _Array[index];
		}
		
	};
		
	class TTEMP
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		ICDBStructNode	*_QUALITY;
		ICDBStructNode	*_QUANTITY;
		ICDBStructNode	*_USER_COLOR;
		ICDBStructNode	*_WEIGHT;
		ICDBStructNode	*_NAMEID;
		ICDBStructNode	*_INFO_VERSION;
		ICDBStructNode	*_ENCHANT;
		ICDBStructNode	*_RM_CLASS_TYPE;
		ICDBStructNode	*_RM_FABER_STAT_TYPE;
		ICDBStructNode	*_PREREQUISIT_VALID;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSHEET(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SHEET, value, forceSending);
		}

		NLMISC::CSheetId getSHEET(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _SHEET, value);

			return value;
		}
		
		ICDBStructNode *getSHEETCDBNode()
		{
			return _SHEET;
		}
	
		void setQUALITY(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<12)-1, "setQUALITY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 12 bits");
				

			_setProp(dbGroup, _QUALITY, value, forceSending);
		}

		uint16 getQUALITY(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _QUALITY, value);

			return value;
		}
		
		ICDBStructNode *getQUALITYCDBNode()
		{
			return _QUALITY;
		}
	
		void setQUANTITY(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<12)-1, "setQUANTITY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 12 bits");
				

			_setProp(dbGroup, _QUANTITY, value, forceSending);
		}

		uint16 getQUANTITY(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _QUANTITY, value);

			return value;
		}
		
		ICDBStructNode *getQUANTITYCDBNode()
		{
			return _QUANTITY;
		}
	
		void setUSER_COLOR(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setUSER_COLOR : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _USER_COLOR, value, forceSending);
		}

		uint8 getUSER_COLOR(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _USER_COLOR, value);

			return value;
		}
		
		ICDBStructNode *getUSER_COLORCDBNode()
		{
			return _USER_COLOR;
		}
	
		void setWEIGHT(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _WEIGHT, value, forceSending);
		}

		uint16 getWEIGHT(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _WEIGHT, value);

			return value;
		}
		
		ICDBStructNode *getWEIGHTCDBNode()
		{
			return _WEIGHT;
		}
	
		void setNAMEID(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NAMEID, value, forceSending);
		}

		uint32 getNAMEID(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NAMEID, value);

			return value;
		}
		
		ICDBStructNode *getNAMEIDCDBNode()
		{
			return _NAMEID;
		}
	
		void setINFO_VERSION(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _INFO_VERSION, value, forceSending);
		}

		uint8 getINFO_VERSION(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _INFO_VERSION, value);

			return value;
		}
		
		ICDBStructNode *getINFO_VERSIONCDBNode()
		{
			return _INFO_VERSION;
		}
	
		void setENCHANT(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setENCHANT : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _ENCHANT, value, forceSending);
		}

		uint16 getENCHANT(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _ENCHANT, value);

			return value;
		}
		
		ICDBStructNode *getENCHANTCDBNode()
		{
			return _ENCHANT;
		}
	
		void setRM_CLASS_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setRM_CLASS_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _RM_CLASS_TYPE, value, forceSending);
		}

		uint8 getRM_CLASS_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _RM_CLASS_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getRM_CLASS_TYPECDBNode()
		{
			return _RM_CLASS_TYPE;
		}
	
		void setRM_FABER_STAT_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<6)-1, "setRM_FABER_STAT_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 6 bits");
				

			_setProp(dbGroup, _RM_FABER_STAT_TYPE, value, forceSending);
		}

		uint8 getRM_FABER_STAT_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _RM_FABER_STAT_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getRM_FABER_STAT_TYPECDBNode()
		{
			return _RM_FABER_STAT_TYPE;
		}
	
		void setPREREQUISIT_VALID(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PREREQUISIT_VALID, value, forceSending);
		}

		bool getPREREQUISIT_VALID(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _PREREQUISIT_VALID, value);

			return value;
		}
		
		ICDBStructNode *getPREREQUISIT_VALIDCDBNode()
		{
			return _PREREQUISIT_VALID;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TYPE;
		ICDBStructNode	*_ENABLE_TAKE;
		TArray _Array[16];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TYPE, value, forceSending);
		}

		uint8 getTYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _TYPE, value);

			return value;
		}
		
		ICDBStructNode *getTYPECDBNode()
		{
			return _TYPE;
		}
	
		void setENABLE_TAKE(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ENABLE_TAKE, value, forceSending);
		}

		bool getENABLE_TAKE(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _ENABLE_TAKE, value);

			return value;
		}
		
		ICDBStructNode *getENABLE_TAKECDBNode()
		{
			return _ENABLE_TAKE;
		}
	TArray &getArray(uint32 index)
		{
			nlassert(index < 16);
			return _Array[index];
		}
		
	};
		
	class TSHARE
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		ICDBStructNode	*_QUALITY;
		ICDBStructNode	*_QUANTITY;
		ICDBStructNode	*_USER_COLOR;
		ICDBStructNode	*_WEIGHT;
		ICDBStructNode	*_NAMEID;
		ICDBStructNode	*_INFO_VERSION;
		ICDBStructNode	*_ENCHANT;
		ICDBStructNode	*_RM_CLASS_TYPE;
		ICDBStructNode	*_RM_FABER_STAT_TYPE;
		ICDBStructNode	*_PREREQUISIT_VALID;
		ICDBStructNode	*_NB_MEMBER;
		ICDBStructNode	*_WANTED;
		ICDBStructNode	*_CHANCE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSHEET(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SHEET, value, forceSending);
		}

		NLMISC::CSheetId getSHEET(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _SHEET, value);

			return value;
		}
		
		ICDBStructNode *getSHEETCDBNode()
		{
			return _SHEET;
		}
	
		void setQUALITY(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setQUALITY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _QUALITY, value, forceSending);
		}

		uint16 getQUALITY(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _QUALITY, value);

			return value;
		}
		
		ICDBStructNode *getQUALITYCDBNode()
		{
			return _QUALITY;
		}
	
		void setQUANTITY(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setQUANTITY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _QUANTITY, value, forceSending);
		}

		uint16 getQUANTITY(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _QUANTITY, value);

			return value;
		}
		
		ICDBStructNode *getQUANTITYCDBNode()
		{
			return _QUANTITY;
		}
	
		void setUSER_COLOR(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setUSER_COLOR : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _USER_COLOR, value, forceSending);
		}

		uint8 getUSER_COLOR(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _USER_COLOR, value);

			return value;
		}
		
		ICDBStructNode *getUSER_COLORCDBNode()
		{
			return _USER_COLOR;
		}
	
		void setWEIGHT(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _WEIGHT, value, forceSending);
		}

		uint16 getWEIGHT(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _WEIGHT, value);

			return value;
		}
		
		ICDBStructNode *getWEIGHTCDBNode()
		{
			return _WEIGHT;
		}
	
		void setNAMEID(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NAMEID, value, forceSending);
		}

		uint32 getNAMEID(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NAMEID, value);

			return value;
		}
		
		ICDBStructNode *getNAMEIDCDBNode()
		{
			return _NAMEID;
		}
	
		void setINFO_VERSION(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _INFO_VERSION, value, forceSending);
		}

		uint8 getINFO_VERSION(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _INFO_VERSION, value);

			return value;
		}
		
		ICDBStructNode *getINFO_VERSIONCDBNode()
		{
			return _INFO_VERSION;
		}
	
		void setENCHANT(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setENCHANT : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _ENCHANT, value, forceSending);
		}

		uint16 getENCHANT(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _ENCHANT, value);

			return value;
		}
		
		ICDBStructNode *getENCHANTCDBNode()
		{
			return _ENCHANT;
		}
	
		void setRM_CLASS_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setRM_CLASS_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _RM_CLASS_TYPE, value, forceSending);
		}

		uint8 getRM_CLASS_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _RM_CLASS_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getRM_CLASS_TYPECDBNode()
		{
			return _RM_CLASS_TYPE;
		}
	
		void setRM_FABER_STAT_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<6)-1, "setRM_FABER_STAT_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 6 bits");
				

			_setProp(dbGroup, _RM_FABER_STAT_TYPE, value, forceSending);
		}

		uint8 getRM_FABER_STAT_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _RM_FABER_STAT_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getRM_FABER_STAT_TYPECDBNode()
		{
			return _RM_FABER_STAT_TYPE;
		}
	
		void setPREREQUISIT_VALID(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PREREQUISIT_VALID, value, forceSending);
		}

		bool getPREREQUISIT_VALID(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _PREREQUISIT_VALID, value);

			return value;
		}
		
		ICDBStructNode *getPREREQUISIT_VALIDCDBNode()
		{
			return _PREREQUISIT_VALID;
		}
	
		void setNB_MEMBER(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setNB_MEMBER : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _NB_MEMBER, value, forceSending);
		}

		uint8 getNB_MEMBER(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _NB_MEMBER, value);

			return value;
		}
		
		ICDBStructNode *getNB_MEMBERCDBNode()
		{
			return _NB_MEMBER;
		}
	
		void setWANTED(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _WANTED, value, forceSending);
		}

		bool getWANTED(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _WANTED, value);

			return value;
		}
		
		ICDBStructNode *getWANTEDCDBNode()
		{
			return _WANTED;
		}
	
		void setCHANCE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setCHANCE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _CHANCE, value, forceSending);
		}

		uint8 getCHANCE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _CHANCE, value);

			return value;
		}
		
		ICDBStructNode *getCHANCECDBNode()
		{
			return _CHANCE;
		}
	
	};
		
	class TTM_
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_NAME;
		ICDBStructNode	*_VALID;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setNAME(CCDBSynchronised &dbGroup, ucstring value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NAME, value, forceSending);
		}

		ucstring getNAME(const CCDBSynchronised &dbGroup)
		{
			ucstring value;
			_getProp(dbGroup, _NAME, value);

			return value;
		}
		
		void setNAME(CCDBSynchronised &dbGroup, uint32 stringId, bool forceSending = false)
		{
			_setProp(dbGroup, _NAME, stringId, forceSending);
		}
		uint32 getNAME_id(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NAME, value);

			return value;
		}
		
		ICDBStructNode *getNAMECDBNode()
		{
			return _NAME;
		}
	
		void setVALID(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VALID, value, forceSending);
		}

		bool getVALID(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _VALID, value);

			return value;
		}
		
		ICDBStructNode *getVALIDCDBNode()
		{
			return _VALID;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SESSION;
		TArray _Array[16];
		TTM_ _TM_[8];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSESSION(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SESSION, value, forceSending);
		}

		uint8 getSESSION(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _SESSION, value);

			return value;
		}
		
		ICDBStructNode *getSESSIONCDBNode()
		{
			return _SESSION;
		}
	TArray &getArray(uint32 index)
		{
			nlassert(index < 16);
			return _Array[index];
		}
		TTM_ &getTM_(uint32 index)
		{
			nlassert(index < 8);
			return _TM_[index];
		}
		
	};
		
	class TROOM
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

		
		void setSESSION(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SESSION, value, forceSending);
		}

		uint16 getSESSION(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _SESSION, value);

			return value;
		}
		
		ICDBStructNode *getSESSIONCDBNode()
		{
			return _SESSION;
		}
	
		void setBULK_MAX(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BULK_MAX, value, forceSending);
		}

		uint32 getBULK_MAX(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _BULK_MAX, value);

			return value;
		}
		
		ICDBStructNode *getBULK_MAXCDBNode()
		{
			return _BULK_MAX;
		}
	
		void setMONEY(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			_setProp(dbGroup, _MONEY, value, forceSending);
		}

		uint64 getMONEY(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _MONEY, value);

			return value;
		}

		ICDBStructNode *getMONEYCDBNode()
		{
			return _MONEY;
		}
	};

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_MONEY;
		ICDBStructNode	*_COUNTER;
		THAND	_HAND;
		TEQUIP	_EQUIP;
		TTEMP	_TEMP;
		TSHARE	_SHARE;
		TROOM	_ROOM;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		void setMONEY(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			_setProp(dbGroup, _MONEY, value, forceSending);
		}

		uint64 getMONEY(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _MONEY, value);

			return value;
		}
		
		ICDBStructNode *getMONEYCDBNode()
		{
			return _MONEY;
		}
	
		void setCOUNTER(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setCOUNTER : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _COUNTER, value, forceSending);
		}

		uint8 getCOUNTER(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _COUNTER, value);

			return value;
		}
		
		ICDBStructNode *getCOUNTERCDBNode()
		{
			return _COUNTER;
		}
	THAND &getHAND()
		{
			return _HAND;
		}
		TEQUIP &getEQUIP()
		{
			return _EQUIP;
		}
		TTEMP &getTEMP()
		{
			return _TEMP;
		}
		TSHARE &getSHARE()
		{
			return _SHARE;
		}
		TROOM &getROOM()
		{
			return _ROOM;
		}
		
	};
		
	class TMODIFIERS
	{
	public:
		
	class TBONUS
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		ICDBStructNode	*_DISABLED;
		ICDBStructNode	*_DISABLED_TIME;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSHEET(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SHEET, value, forceSending);
		}

		NLMISC::CSheetId getSHEET(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _SHEET, value);

			return value;
		}
		
		ICDBStructNode *getSHEETCDBNode()
		{
			return _SHEET;
		}
	
		void setDISABLED(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DISABLED, value, forceSending);
		}

		bool getDISABLED(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _DISABLED, value);

			return value;
		}
		
		ICDBStructNode *getDISABLEDCDBNode()
		{
			return _DISABLED;
		}
	
		void setDISABLED_TIME(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DISABLED_TIME, value, forceSending);
		}

		uint32 getDISABLED_TIME(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _DISABLED_TIME, value);

			return value;
		}
		
		ICDBStructNode *getDISABLED_TIMECDBNode()
		{
			return _DISABLED_TIME;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[12];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 12);
			return _Array[index];
		}
		
	};
		
	class TMALUS
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		ICDBStructNode	*_DISABLED;
		ICDBStructNode	*_DISABLED_TIME;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSHEET(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SHEET, value, forceSending);
		}

		NLMISC::CSheetId getSHEET(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _SHEET, value);

			return value;
		}
		
		ICDBStructNode *getSHEETCDBNode()
		{
			return _SHEET;
		}
	
		void setDISABLED(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DISABLED, value, forceSending);
		}

		bool getDISABLED(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _DISABLED, value);

			return value;
		}
		
		ICDBStructNode *getDISABLEDCDBNode()
		{
			return _DISABLED;
		}
	
		void setDISABLED_TIME(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DISABLED_TIME, value, forceSending);
		}

		uint32 getDISABLED_TIME(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _DISABLED_TIME, value);

			return value;
		}
		
		ICDBStructNode *getDISABLED_TIMECDBNode()
		{
			return _DISABLED_TIME;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[12];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 12);
			return _Array[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TOTAL_MALUS_EQUIP;
		TBONUS	_BONUS;
		TMALUS	_MALUS;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTOTAL_MALUS_EQUIP(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TOTAL_MALUS_EQUIP, value, forceSending);
		}

		uint8 getTOTAL_MALUS_EQUIP(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _TOTAL_MALUS_EQUIP, value);

			return value;
		}
		
		ICDBStructNode *getTOTAL_MALUS_EQUIPCDBNode()
		{
			return _TOTAL_MALUS_EQUIP;
		}
	TBONUS &getBONUS()
		{
			return _BONUS;
		}
		TMALUS &getMALUS()
		{
			return _MALUS;
		}
		
	};
		
	class TDISABLE_CONSUMABLE
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_FAMILY;
		ICDBStructNode	*_DISABLE_TIME;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setFAMILY(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _FAMILY, value, forceSending);
		}

		uint16 getFAMILY(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _FAMILY, value);

			return value;
		}
		
		ICDBStructNode *getFAMILYCDBNode()
		{
			return _FAMILY;
		}
	
		void setDISABLE_TIME(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DISABLE_TIME, value, forceSending);
		}

		uint32 getDISABLE_TIME(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _DISABLE_TIME, value);

			return value;
		}
		
		ICDBStructNode *getDISABLE_TIMECDBNode()
		{
			return _DISABLE_TIME;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[12];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 12);
			return _Array[index];
		}
		
	};
		
	class TBOTCHAT
	{
	public:
		
	class TDM_CHOICE
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TEXT;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTEXT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TEXT, value, forceSending);
		}

		uint32 getTEXT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TEXT, value);

			return value;
		}
		
		ICDBStructNode *getTEXTCDBNode()
		{
			return _TEXT;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TITLE;
		TArray _Array[8];
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTITLE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TITLE, value, forceSending);
		}

		uint32 getTITLE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TITLE, value);

			return value;
		}
		
		ICDBStructNode *getTITLECDBNode()
		{
			return _TITLE;
		}
	TArray &getArray(uint32 index)
		{
			nlassert(index < 8);
			return _Array[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_PLAYER_GIFT;
		ICDBStructNode	*_CREATE_GUILD;
		ICDBStructNode	*_TRADE;
		ICDBStructNode	*_CHOOSE_MISSION;
		ICDBStructNode	*_DM_TITLE;
		ICDBStructNode	*_DM_DESCRIPTION;
		ICDBStructNode	*_ROLEMASTER_TYPE;
		TDM_CHOICE _DM_CHOICE[3];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setPLAYER_GIFT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PLAYER_GIFT, value, forceSending);
		}

		uint32 getPLAYER_GIFT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _PLAYER_GIFT, value);

			return value;
		}
		
		ICDBStructNode *getPLAYER_GIFTCDBNode()
		{
			return _PLAYER_GIFT;
		}
	
		void setCREATE_GUILD(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _CREATE_GUILD, value, forceSending);
		}

		uint32 getCREATE_GUILD(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _CREATE_GUILD, value);

			return value;
		}
		
		ICDBStructNode *getCREATE_GUILDCDBNode()
		{
			return _CREATE_GUILD;
		}
	
		void setTRADE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TRADE, value, forceSending);
		}

		uint32 getTRADE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TRADE, value);

			return value;
		}
		
		ICDBStructNode *getTRADECDBNode()
		{
			return _TRADE;
		}
	
		void setCHOOSE_MISSION(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _CHOOSE_MISSION, value, forceSending);
		}

		uint32 getCHOOSE_MISSION(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _CHOOSE_MISSION, value);

			return value;
		}
		
		ICDBStructNode *getCHOOSE_MISSIONCDBNode()
		{
			return _CHOOSE_MISSION;
		}
	
		void setDM_TITLE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DM_TITLE, value, forceSending);
		}

		uint32 getDM_TITLE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _DM_TITLE, value);

			return value;
		}
		
		ICDBStructNode *getDM_TITLECDBNode()
		{
			return _DM_TITLE;
		}
	
		void setDM_DESCRIPTION(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DM_DESCRIPTION, value, forceSending);
		}

		uint32 getDM_DESCRIPTION(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _DM_DESCRIPTION, value);

			return value;
		}
		
		ICDBStructNode *getDM_DESCRIPTIONCDBNode()
		{
			return _DM_DESCRIPTION;
		}
	
		void setROLEMASTER_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<2)-1, "setROLEMASTER_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 2 bits");
				

			_setProp(dbGroup, _ROLEMASTER_TYPE, value, forceSending);
		}

		uint8 getROLEMASTER_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _ROLEMASTER_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getROLEMASTER_TYPECDBNode()
		{
			return _ROLEMASTER_TYPE;
		}
	TDM_CHOICE &getDM_CHOICE(uint32 index)
		{
			nlassert(index < 3);
			return _DM_CHOICE[index];
		}
		
	};
		
	class TASCENSOR
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_ICON;
		ICDBStructNode	*_NAME;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setICON(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ICON, value, forceSending);
		}

		uint64 getICON(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _ICON, value);

			return value;
		}
		
		ICDBStructNode *getICONCDBNode()
		{
			return _ICON;
		}
	
		void setNAME(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NAME, value, forceSending);
		}

		uint32 getNAME(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NAME, value);

			return value;
		}
		
		ICDBStructNode *getNAMECDBNode()
		{
			return _NAME;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SESSION;
		ICDBStructNode	*_PAGE_ID;
		ICDBStructNode	*_HAS_NEXT;
		TArray _Array[8];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSESSION(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SESSION, value, forceSending);
		}

		uint16 getSESSION(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _SESSION, value);

			return value;
		}
		
		ICDBStructNode *getSESSIONCDBNode()
		{
			return _SESSION;
		}
	
		void setPAGE_ID(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<6)-1, "setPAGE_ID : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 6 bits");
				

			_setProp(dbGroup, _PAGE_ID, value, forceSending);
		}

		uint8 getPAGE_ID(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _PAGE_ID, value);

			return value;
		}
		
		ICDBStructNode *getPAGE_IDCDBNode()
		{
			return _PAGE_ID;
		}
	
		void setHAS_NEXT(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _HAS_NEXT, value, forceSending);
		}

		bool getHAS_NEXT(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _HAS_NEXT, value);

			return value;
		}
		
		ICDBStructNode *getHAS_NEXTCDBNode()
		{
			return _HAS_NEXT;
		}
	TArray &getArray(uint32 index)
		{
			nlassert(index < 8);
			return _Array[index];
		}
		
	};
		
	class TCHOOSE_MISSIONS
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_ICON;
		ICDBStructNode	*_TEXT;
		ICDBStructNode	*_DETAIL_TEXT;
		ICDBStructNode	*_PREREQ_STATE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setICON(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ICON, value, forceSending);
		}

		NLMISC::CSheetId getICON(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _ICON, value);

			return value;
		}
		
		ICDBStructNode *getICONCDBNode()
		{
			return _ICON;
		}
	
		void setTEXT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TEXT, value, forceSending);
		}

		uint32 getTEXT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TEXT, value);

			return value;
		}
		
		ICDBStructNode *getTEXTCDBNode()
		{
			return _TEXT;
		}
	
		void setDETAIL_TEXT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DETAIL_TEXT, value, forceSending);
		}

		uint32 getDETAIL_TEXT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _DETAIL_TEXT, value);

			return value;
		}
		
		ICDBStructNode *getDETAIL_TEXTCDBNode()
		{
			return _DETAIL_TEXT;
		}
	
		void setPREREQ_STATE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PREREQ_STATE, value, forceSending);
		}

		uint8 getPREREQ_STATE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _PREREQ_STATE, value);

			return value;
		}
		
		ICDBStructNode *getPREREQ_STATECDBNode()
		{
			return _PREREQ_STATE;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SESSION;
		ICDBStructNode	*_PAGE_ID;
		ICDBStructNode	*_HAS_NEXT;
		TArray _Array[8];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSESSION(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SESSION, value, forceSending);
		}

		uint16 getSESSION(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _SESSION, value);

			return value;
		}
		
		ICDBStructNode *getSESSIONCDBNode()
		{
			return _SESSION;
		}
	
		void setPAGE_ID(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<6)-1, "setPAGE_ID : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 6 bits");
				

			_setProp(dbGroup, _PAGE_ID, value, forceSending);
		}

		uint8 getPAGE_ID(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _PAGE_ID, value);

			return value;
		}
		
		ICDBStructNode *getPAGE_IDCDBNode()
		{
			return _PAGE_ID;
		}
	
		void setHAS_NEXT(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _HAS_NEXT, value, forceSending);
		}

		bool getHAS_NEXT(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _HAS_NEXT, value);

			return value;
		}
		
		ICDBStructNode *getHAS_NEXTCDBNode()
		{
			return _HAS_NEXT;
		}
	TArray &getArray(uint32 index)
		{
			nlassert(index < 8);
			return _Array[index];
		}
		
	};
		
	class TTRADING
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SHEET;
		ICDBStructNode	*_QUALITY;
		ICDBStructNode	*_QUANTITY;
		ICDBStructNode	*_USER_COLOR;
		ICDBStructNode	*_WEIGHT;
		ICDBStructNode	*_NAMEID;
		ICDBStructNode	*_INFO_VERSION;
		ICDBStructNode	*_ENCHANT;
		ICDBStructNode	*_RM_CLASS_TYPE;
		ICDBStructNode	*_RM_FABER_STAT_TYPE;
		ICDBStructNode	*_PREREQUISIT_VALID;
		ICDBStructNode	*_CURRENCY;
		ICDBStructNode	*_RRP_LEVEL;
		ICDBStructNode	*_MONEY_SHEET;
		ICDBStructNode	*_BASE_SKILL;
		ICDBStructNode	*_FACTION_TYPE;
		ICDBStructNode	*_PRICE;
		ICDBStructNode	*_PRICE_RETIRE;
		ICDBStructNode	*_RESALE_TIME_LEFT;
		ICDBStructNode	*_VENDOR_NAMEID;
		ICDBStructNode	*_FACTION_POINT_PRICE;
		ICDBStructNode	*_SLOT_TYPE;
		ICDBStructNode	*_SELLER_TYPE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSHEET(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SHEET, value, forceSending);
		}

		NLMISC::CSheetId getSHEET(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _SHEET, value);

			return value;
		}
		
		ICDBStructNode *getSHEETCDBNode()
		{
			return _SHEET;
		}
	
		void setQUALITY(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setQUALITY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _QUALITY, value, forceSending);
		}

		uint16 getQUALITY(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _QUALITY, value);

			return value;
		}
		
		ICDBStructNode *getQUALITYCDBNode()
		{
			return _QUALITY;
		}
	
		void setQUANTITY(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setQUANTITY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _QUANTITY, value, forceSending);
		}

		uint16 getQUANTITY(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _QUANTITY, value);

			return value;
		}
		
		ICDBStructNode *getQUANTITYCDBNode()
		{
			return _QUANTITY;
		}
	
		void setUSER_COLOR(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setUSER_COLOR : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _USER_COLOR, value, forceSending);
		}

		uint8 getUSER_COLOR(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _USER_COLOR, value);

			return value;
		}
		
		ICDBStructNode *getUSER_COLORCDBNode()
		{
			return _USER_COLOR;
		}
	
		void setWEIGHT(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _WEIGHT, value, forceSending);
		}

		uint16 getWEIGHT(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _WEIGHT, value);

			return value;
		}
		
		ICDBStructNode *getWEIGHTCDBNode()
		{
			return _WEIGHT;
		}
	
		void setNAMEID(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NAMEID, value, forceSending);
		}

		uint32 getNAMEID(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NAMEID, value);

			return value;
		}
		
		ICDBStructNode *getNAMEIDCDBNode()
		{
			return _NAMEID;
		}
	
		void setINFO_VERSION(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _INFO_VERSION, value, forceSending);
		}

		uint8 getINFO_VERSION(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _INFO_VERSION, value);

			return value;
		}
		
		ICDBStructNode *getINFO_VERSIONCDBNode()
		{
			return _INFO_VERSION;
		}
	
		void setENCHANT(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<10)-1, "setENCHANT : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 10 bits");
				

			_setProp(dbGroup, _ENCHANT, value, forceSending);
		}

		uint16 getENCHANT(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _ENCHANT, value);

			return value;
		}
		
		ICDBStructNode *getENCHANTCDBNode()
		{
			return _ENCHANT;
		}
	
		void setRM_CLASS_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setRM_CLASS_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _RM_CLASS_TYPE, value, forceSending);
		}

		uint8 getRM_CLASS_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _RM_CLASS_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getRM_CLASS_TYPECDBNode()
		{
			return _RM_CLASS_TYPE;
		}
	
		void setRM_FABER_STAT_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<6)-1, "setRM_FABER_STAT_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 6 bits");
				

			_setProp(dbGroup, _RM_FABER_STAT_TYPE, value, forceSending);
		}

		uint8 getRM_FABER_STAT_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _RM_FABER_STAT_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getRM_FABER_STAT_TYPECDBNode()
		{
			return _RM_FABER_STAT_TYPE;
		}
	
		void setPREREQUISIT_VALID(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PREREQUISIT_VALID, value, forceSending);
		}

		bool getPREREQUISIT_VALID(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _PREREQUISIT_VALID, value);

			return value;
		}
		
		ICDBStructNode *getPREREQUISIT_VALIDCDBNode()
		{
			return _PREREQUISIT_VALID;
		}
	
		void setCURRENCY(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setCURRENCY : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _CURRENCY, value, forceSending);
		}

		uint8 getCURRENCY(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _CURRENCY, value);

			return value;
		}
		
		ICDBStructNode *getCURRENCYCDBNode()
		{
			return _CURRENCY;
		}
	
		void setRRP_LEVEL(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setRRP_LEVEL : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _RRP_LEVEL, value, forceSending);
		}

		uint8 getRRP_LEVEL(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _RRP_LEVEL, value);

			return value;
		}
		
		ICDBStructNode *getRRP_LEVELCDBNode()
		{
			return _RRP_LEVEL;
		}
	
		void setMONEY_SHEET(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			_setProp(dbGroup, _MONEY_SHEET, value, forceSending);
		}

		NLMISC::CSheetId getMONEY_SHEET(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _MONEY_SHEET, value);

			return value;
		}
		
		ICDBStructNode *getMONEY_SHEETCDBNode()
		{
			return _MONEY_SHEET;
		}
	
		void setBASE_SKILL(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setBASE_SKILL : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _BASE_SKILL, value, forceSending);
		}

		uint8 getBASE_SKILL(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _BASE_SKILL, value);

			return value;
		}
		
		ICDBStructNode *getBASE_SKILLCDBNode()
		{
			return _BASE_SKILL;
		}
	
		void setFACTION_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setFACTION_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _FACTION_TYPE, value, forceSending);
		}

		uint8 getFACTION_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _FACTION_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getFACTION_TYPECDBNode()
		{
			return _FACTION_TYPE;
		}
	
		void setPRICE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PRICE, value, forceSending);
		}

		uint32 getPRICE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _PRICE, value);

			return value;
		}
		
		ICDBStructNode *getPRICECDBNode()
		{
			return _PRICE;
		}
	
		void setPRICE_RETIRE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PRICE_RETIRE, value, forceSending);
		}

		uint32 getPRICE_RETIRE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _PRICE_RETIRE, value);

			return value;
		}
		
		ICDBStructNode *getPRICE_RETIRECDBNode()
		{
			return _PRICE_RETIRE;
		}
	
		void setRESALE_TIME_LEFT(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _RESALE_TIME_LEFT, value, forceSending);
		}

		uint16 getRESALE_TIME_LEFT(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _RESALE_TIME_LEFT, value);

			return value;
		}
		
		ICDBStructNode *getRESALE_TIME_LEFTCDBNode()
		{
			return _RESALE_TIME_LEFT;
		}
	
		void setVENDOR_NAMEID(CCDBSynchronised &dbGroup, ucstring value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VENDOR_NAMEID, value, forceSending);
		}

		ucstring getVENDOR_NAMEID(const CCDBSynchronised &dbGroup)
		{
			ucstring value;
			_getProp(dbGroup, _VENDOR_NAMEID, value);

			return value;
		}
		
		void setVENDOR_NAMEID(CCDBSynchronised &dbGroup, uint32 stringId, bool forceSending = false)
		{
			_setProp(dbGroup, _VENDOR_NAMEID, stringId, forceSending);
		}
		uint32 getVENDOR_NAMEID_id(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _VENDOR_NAMEID, value);

			return value;
		}
		
		ICDBStructNode *getVENDOR_NAMEIDCDBNode()
		{
			return _VENDOR_NAMEID;
		}
	
		void setFACTION_POINT_PRICE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _FACTION_POINT_PRICE, value, forceSending);
		}

		uint32 getFACTION_POINT_PRICE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _FACTION_POINT_PRICE, value);

			return value;
		}
		
		ICDBStructNode *getFACTION_POINT_PRICECDBNode()
		{
			return _FACTION_POINT_PRICE;
		}
	
		void setSLOT_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<2)-1, "setSLOT_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 2 bits");
				

			_setProp(dbGroup, _SLOT_TYPE, value, forceSending);
		}

		uint8 getSLOT_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _SLOT_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getSLOT_TYPECDBNode()
		{
			return _SLOT_TYPE;
		}
	
		void setSELLER_TYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setSELLER_TYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _SELLER_TYPE, value, forceSending);
		}

		uint8 getSELLER_TYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _SELLER_TYPE, value);

			return value;
		}
		
		ICDBStructNode *getSELLER_TYPECDBNode()
		{
			return _SELLER_TYPE;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SESSION;
		ICDBStructNode	*_PAGE_ID;
		ICDBStructNode	*_HAS_NEXT;
		ICDBStructNode	*_ROLEMASTER_FLAGS;
		ICDBStructNode	*_ROLEMASTER_RACE;
		ICDBStructNode	*_BUILDING_LOSS_WARNING;
		ICDBStructNode	*_RAW_MATERIAL_SELLER;
		ICDBStructNode	*_ITEM_TYPE_SELLER_BITFILED_0_63;
		ICDBStructNode	*_ITEM_TYPE_SELLER_BITFILED_64_127;
		ICDBStructNode	*_FAME_PRICE_FACTOR;
		TArray _Array[8];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSESSION(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SESSION, value, forceSending);
		}

		uint16 getSESSION(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _SESSION, value);

			return value;
		}
		
		ICDBStructNode *getSESSIONCDBNode()
		{
			return _SESSION;
		}
	
		void setPAGE_ID(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setPAGE_ID : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _PAGE_ID, value, forceSending);
		}

		uint8 getPAGE_ID(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _PAGE_ID, value);

			return value;
		}
		
		ICDBStructNode *getPAGE_IDCDBNode()
		{
			return _PAGE_ID;
		}
	
		void setHAS_NEXT(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _HAS_NEXT, value, forceSending);
		}

		bool getHAS_NEXT(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _HAS_NEXT, value);

			return value;
		}
		
		ICDBStructNode *getHAS_NEXTCDBNode()
		{
			return _HAS_NEXT;
		}
	
		void setROLEMASTER_FLAGS(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<6)-1, "setROLEMASTER_FLAGS : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 6 bits");
				

			_setProp(dbGroup, _ROLEMASTER_FLAGS, value, forceSending);
		}

		uint8 getROLEMASTER_FLAGS(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _ROLEMASTER_FLAGS, value);

			return value;
		}
		
		ICDBStructNode *getROLEMASTER_FLAGSCDBNode()
		{
			return _ROLEMASTER_FLAGS;
		}
	
		void setROLEMASTER_RACE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<2)-1, "setROLEMASTER_RACE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 2 bits");
				

			_setProp(dbGroup, _ROLEMASTER_RACE, value, forceSending);
		}

		uint8 getROLEMASTER_RACE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _ROLEMASTER_RACE, value);

			return value;
		}
		
		ICDBStructNode *getROLEMASTER_RACECDBNode()
		{
			return _ROLEMASTER_RACE;
		}
	
		void setBUILDING_LOSS_WARNING(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BUILDING_LOSS_WARNING, value, forceSending);
		}

		bool getBUILDING_LOSS_WARNING(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _BUILDING_LOSS_WARNING, value);

			return value;
		}
		
		ICDBStructNode *getBUILDING_LOSS_WARNINGCDBNode()
		{
			return _BUILDING_LOSS_WARNING;
		}
	
		void setRAW_MATERIAL_SELLER(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _RAW_MATERIAL_SELLER, value, forceSending);
		}

		bool getRAW_MATERIAL_SELLER(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _RAW_MATERIAL_SELLER, value);

			return value;
		}
		
		ICDBStructNode *getRAW_MATERIAL_SELLERCDBNode()
		{
			return _RAW_MATERIAL_SELLER;
		}
	
		void setITEM_TYPE_SELLER_BITFILED_0_63(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ITEM_TYPE_SELLER_BITFILED_0_63, value, forceSending);
		}

		uint64 getITEM_TYPE_SELLER_BITFILED_0_63(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _ITEM_TYPE_SELLER_BITFILED_0_63, value);

			return value;
		}
		
		ICDBStructNode *getITEM_TYPE_SELLER_BITFILED_0_63CDBNode()
		{
			return _ITEM_TYPE_SELLER_BITFILED_0_63;
		}
	
		void setITEM_TYPE_SELLER_BITFILED_64_127(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ITEM_TYPE_SELLER_BITFILED_64_127, value, forceSending);
		}

		uint64 getITEM_TYPE_SELLER_BITFILED_64_127(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _ITEM_TYPE_SELLER_BITFILED_64_127, value);

			return value;
		}
		
		ICDBStructNode *getITEM_TYPE_SELLER_BITFILED_64_127CDBNode()
		{
			return _ITEM_TYPE_SELLER_BITFILED_64_127;
		}
	
		void setFAME_PRICE_FACTOR(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _FAME_PRICE_FACTOR, value, forceSending);
		}

		uint16 getFAME_PRICE_FACTOR(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _FAME_PRICE_FACTOR, value);

			return value;
		}
		
		ICDBStructNode *getFAME_PRICE_FACTORCDBNode()
		{
			return _FAME_PRICE_FACTOR;
		}
	TArray &getArray(uint32 index)
		{
			nlassert(index < 8);
			return _Array[index];
		}
		
	};
		
	class TBRICK_FAMILY
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_BRICKS;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setBRICKS(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BRICKS, value, forceSending);
		}

		uint64 getBRICKS(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _BRICKS, value);

			return value;
		}
		
		ICDBStructNode *getBRICKSCDBNode()
		{
			return _BRICKS;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[1024];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 1024);
			return _Array[index];
		}
		
	};
		
	class TFABER_PLANS
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_KNOWN;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setKNOWN(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _KNOWN, value, forceSending);
		}

		uint64 getKNOWN(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _KNOWN, value);

			return value;
		}
		
		ICDBStructNode *getKNOWNCDBNode()
		{
			return _KNOWN;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[64];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 64);
			return _Array[index];
		}
		
	};
		
	class TMISSIONS
	{
	public:
		
	class TArray
	{
	public:
		
	class TGOALS
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TEXT;
		ICDBStructNode	*_NPC_ALIAS;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTEXT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TEXT, value, forceSending);
		}

		uint32 getTEXT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TEXT, value);

			return value;
		}
		
		ICDBStructNode *getTEXTCDBNode()
		{
			return _TEXT;
		}
	
		void setNPC_ALIAS(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NPC_ALIAS, value, forceSending);
		}

		uint32 getNPC_ALIAS(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NPC_ALIAS, value);

			return value;
		}
		
		ICDBStructNode *getNPC_ALIASCDBNode()
		{
			return _NPC_ALIAS;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[20];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 20);
			return _Array[index];
		}
		
	};
		
	class TTARGET
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TITLE;
		ICDBStructNode	*_X;
		ICDBStructNode	*_Y;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTITLE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TITLE, value, forceSending);
		}

		uint32 getTITLE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TITLE, value);

			return value;
		}
		
		ICDBStructNode *getTITLECDBNode()
		{
			return _TITLE;
		}
	
		void setX(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _X, value, forceSending);
		}

		uint32 getX(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _X, value);

			return value;
		}
		
		ICDBStructNode *getXCDBNode()
		{
			return _X;
		}
	
		void setY(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _Y, value, forceSending);
		}

		uint32 getY(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _Y, value);

			return value;
		}
		
		ICDBStructNode *getYCDBNode()
		{
			return _Y;
		}
	
	};
		
	class THISTO
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TEXT;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTEXT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TEXT, value, forceSending);
		}

		uint32 getTEXT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TEXT, value);

			return value;
		}
		
		ICDBStructNode *getTEXTCDBNode()
		{
			return _TEXT;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[30];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 30);
			return _Array[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TYPE;
		ICDBStructNode	*_ICON;
		ICDBStructNode	*_TITLE;
		ICDBStructNode	*_DETAIL_TEXT;
		ICDBStructNode	*_BEGIN_DATE;
		ICDBStructNode	*_END_DATE;
		ICDBStructNode	*_OR_STEPS;
		ICDBStructNode	*_FINISHED;
		ICDBStructNode	*_ABANDONNABLE;
		ICDBStructNode	*_SLEEP;
		TGOALS	_GOALS;
		TTARGET _TARGET[8];
		THISTO	_HISTO;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<2)-1, "setTYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 2 bits");
				

			_setProp(dbGroup, _TYPE, value, forceSending);
		}

		uint8 getTYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _TYPE, value);

			return value;
		}
		
		ICDBStructNode *getTYPECDBNode()
		{
			return _TYPE;
		}
	
		void setICON(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ICON, value, forceSending);
		}

		NLMISC::CSheetId getICON(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _ICON, value);

			return value;
		}
		
		ICDBStructNode *getICONCDBNode()
		{
			return _ICON;
		}
	
		void setTITLE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TITLE, value, forceSending);
		}

		uint32 getTITLE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TITLE, value);

			return value;
		}
		
		ICDBStructNode *getTITLECDBNode()
		{
			return _TITLE;
		}
	
		void setDETAIL_TEXT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DETAIL_TEXT, value, forceSending);
		}

		uint32 getDETAIL_TEXT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _DETAIL_TEXT, value);

			return value;
		}
		
		ICDBStructNode *getDETAIL_TEXTCDBNode()
		{
			return _DETAIL_TEXT;
		}
	
		void setBEGIN_DATE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BEGIN_DATE, value, forceSending);
		}

		uint32 getBEGIN_DATE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _BEGIN_DATE, value);

			return value;
		}
		
		ICDBStructNode *getBEGIN_DATECDBNode()
		{
			return _BEGIN_DATE;
		}
	
		void setEND_DATE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _END_DATE, value, forceSending);
		}

		uint32 getEND_DATE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _END_DATE, value);

			return value;
		}
		
		ICDBStructNode *getEND_DATECDBNode()
		{
			return _END_DATE;
		}
	
		void setOR_STEPS(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _OR_STEPS, value, forceSending);
		}

		bool getOR_STEPS(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _OR_STEPS, value);

			return value;
		}
		
		ICDBStructNode *getOR_STEPSCDBNode()
		{
			return _OR_STEPS;
		}
	
		void setFINISHED(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<2)-1, "setFINISHED : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 2 bits");
				

			_setProp(dbGroup, _FINISHED, value, forceSending);
		}

		uint8 getFINISHED(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _FINISHED, value);

			return value;
		}
		
		ICDBStructNode *getFINISHEDCDBNode()
		{
			return _FINISHED;
		}
	
		void setABANDONNABLE(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ABANDONNABLE, value, forceSending);
		}

		bool getABANDONNABLE(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _ABANDONNABLE, value);

			return value;
		}
		
		ICDBStructNode *getABANDONNABLECDBNode()
		{
			return _ABANDONNABLE;
		}
	
		void setSLEEP(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SLEEP, value, forceSending);
		}

		bool getSLEEP(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _SLEEP, value);

			return value;
		}
		
		ICDBStructNode *getSLEEPCDBNode()
		{
			return _SLEEP;
		}
	TGOALS &getGOALS()
		{
			return _GOALS;
		}
		TTARGET &getTARGET(uint32 index)
		{
			nlassert(index < 8);
			return _TARGET[index];
		}
		THISTO &getHISTO()
		{
			return _HISTO;
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[15];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 15);
			return _Array[index];
		}
		
	};
		
	class TEXECUTE_PHRASE
	{
	public:
		
	class TLINK
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_PHRASE;
		ICDBStructNode	*_COUNTER;
		ICDBStructNode	*_HP_COST;
		ICDBStructNode	*_SAP_COST;
		ICDBStructNode	*_STA_COST;
		ICDBStructNode	*_TARGET_NAME;
		ICDBStructNode	*_TARGET_HP;
		ICDBStructNode	*_TARGET_SAP;
		ICDBStructNode	*_TARGET_STA;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setPHRASE(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PHRASE, value, forceSending);
		}

		uint16 getPHRASE(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _PHRASE, value);

			return value;
		}
		
		ICDBStructNode *getPHRASECDBNode()
		{
			return _PHRASE;
		}
	
		void setCOUNTER(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setCOUNTER : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _COUNTER, value, forceSending);
		}

		uint8 getCOUNTER(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _COUNTER, value);

			return value;
		}
		
		ICDBStructNode *getCOUNTERCDBNode()
		{
			return _COUNTER;
		}
	
		void setHP_COST(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _HP_COST, value, forceSending);
		}

		uint16 getHP_COST(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _HP_COST, value);

			return value;
		}
		
		ICDBStructNode *getHP_COSTCDBNode()
		{
			return _HP_COST;
		}
	
		void setSAP_COST(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SAP_COST, value, forceSending);
		}

		uint16 getSAP_COST(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _SAP_COST, value);

			return value;
		}
		
		ICDBStructNode *getSAP_COSTCDBNode()
		{
			return _SAP_COST;
		}
	
		void setSTA_COST(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _STA_COST, value, forceSending);
		}

		uint16 getSTA_COST(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _STA_COST, value);

			return value;
		}
		
		ICDBStructNode *getSTA_COSTCDBNode()
		{
			return _STA_COST;
		}
	
		void setTARGET_NAME(CCDBSynchronised &dbGroup, ucstring value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TARGET_NAME, value, forceSending);
		}

		ucstring getTARGET_NAME(const CCDBSynchronised &dbGroup)
		{
			ucstring value;
			_getProp(dbGroup, _TARGET_NAME, value);

			return value;
		}
		
		void setTARGET_NAME(CCDBSynchronised &dbGroup, uint32 stringId, bool forceSending = false)
		{
			_setProp(dbGroup, _TARGET_NAME, stringId, forceSending);
		}
		uint32 getTARGET_NAME_id(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _TARGET_NAME, value);

			return value;
		}
		
		ICDBStructNode *getTARGET_NAMECDBNode()
		{
			return _TARGET_NAME;
		}
	
		void setTARGET_HP(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setTARGET_HP : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _TARGET_HP, value, forceSending);
		}

		uint8 getTARGET_HP(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _TARGET_HP, value);

			return value;
		}
		
		ICDBStructNode *getTARGET_HPCDBNode()
		{
			return _TARGET_HP;
		}
	
		void setTARGET_SAP(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setTARGET_SAP : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _TARGET_SAP, value, forceSending);
		}

		uint8 getTARGET_SAP(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _TARGET_SAP, value);

			return value;
		}
		
		ICDBStructNode *getTARGET_SAPCDBNode()
		{
			return _TARGET_SAP;
		}
	
		void setTARGET_STA(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setTARGET_STA : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _TARGET_STA, value, forceSending);
		}

		uint8 getTARGET_STA(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _TARGET_STA, value);

			return value;
		}
		
		ICDBStructNode *getTARGET_STACDBNode()
		{
			return _TARGET_STA;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[10];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 10);
			return _Array[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_PHRASE;
		ICDBStructNode	*_SHEET;
		ICDBStructNode	*_NEXT_COUNTER;
		ICDBStructNode	*_CYCLE_COUNTER;
		TLINK	_LINK;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setPHRASE(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PHRASE, value, forceSending);
		}

		uint16 getPHRASE(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _PHRASE, value);

			return value;
		}
		
		ICDBStructNode *getPHRASECDBNode()
		{
			return _PHRASE;
		}
	
		void setSHEET(CCDBSynchronised &dbGroup, NLMISC::CSheetId value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SHEET, value, forceSending);
		}

		NLMISC::CSheetId getSHEET(const CCDBSynchronised &dbGroup)
		{
			NLMISC::CSheetId value;
			_getProp(dbGroup, _SHEET, value);

			return value;
		}
		
		ICDBStructNode *getSHEETCDBNode()
		{
			return _SHEET;
		}
	
		void setNEXT_COUNTER(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NEXT_COUNTER, value, forceSending);
		}

		uint8 getNEXT_COUNTER(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _NEXT_COUNTER, value);

			return value;
		}
		
		ICDBStructNode *getNEXT_COUNTERCDBNode()
		{
			return _NEXT_COUNTER;
		}
	
		void setCYCLE_COUNTER(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _CYCLE_COUNTER, value, forceSending);
		}

		uint8 getCYCLE_COUNTER(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _CYCLE_COUNTER, value);

			return value;
		}
		
		ICDBStructNode *getCYCLE_COUNTERCDBNode()
		{
			return _CYCLE_COUNTER;
		}
	TLINK &getLINK()
		{
			return _LINK;
		}
		
	};
		
	class TCHARACTER_INFO
	{
	public:
		
	class TCHARACTERISTICS
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_VALUE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setVALUE(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<12)-1, "setVALUE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 12 bits");
				

			_setProp(dbGroup, _VALUE, value, forceSending);
		}

		uint16 getVALUE(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _VALUE, value);

			return value;
		}
		
		ICDBStructNode *getVALUECDBNode()
		{
			return _VALUE;
		}
	
	};
		
	class TSCORES
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_Base;
		ICDBStructNode	*_Max;
		ICDBStructNode	*_BaseRegen;
		ICDBStructNode	*_Regen;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setBase(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<24)-1, "setBase : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 24 bits");
				

			_setProp(dbGroup, _Base, value, forceSending);
		}

		uint32 getBase(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _Base, value);

			return value;
		}
		
		ICDBStructNode *getBaseCDBNode()
		{
			return _Base;
		}
	
		void setMax(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<24)-1, "setMax : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 24 bits");
				

			_setProp(dbGroup, _Max, value, forceSending);
		}

		uint32 getMax(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _Max, value);

			return value;
		}
		
		ICDBStructNode *getMaxCDBNode()
		{
			return _Max;
		}
	
		void setBaseRegen(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<24)-1, "setBaseRegen : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 24 bits");
				

			_setProp(dbGroup, _BaseRegen, value, forceSending);
		}

		uint32 getBaseRegen(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _BaseRegen, value);

			return value;
		}
		
		ICDBStructNode *getBaseRegenCDBNode()
		{
			return _BaseRegen;
		}
	
		void setRegen(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<24)-1, "setRegen : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 24 bits");
				

			_setProp(dbGroup, _Regen, value, forceSending);
		}

		uint32 getRegen(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _Regen, value);

			return value;
		}
		
		ICDBStructNode *getRegenCDBNode()
		{
			return _Regen;
		}
	
	};
		
	class TMAGIC_RESISTANCE
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_VALUE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setVALUE(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<9)-1, "setVALUE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 9 bits");
				

			_setProp(dbGroup, _VALUE, value, forceSending);
		}

		uint16 getVALUE(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _VALUE, value);

			return value;
		}
		
		ICDBStructNode *getVALUECDBNode()
		{
			return _VALUE;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_MaxResistanceBonus;
		TArray _Array[5];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setMaxResistanceBonus(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<9)-1, "setMaxResistanceBonus : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 9 bits");
				

			_setProp(dbGroup, _MaxResistanceBonus, value, forceSending);
		}

		uint16 getMaxResistanceBonus(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _MaxResistanceBonus, value);

			return value;
		}
		
		ICDBStructNode *getMaxResistanceBonusCDBNode()
		{
			return _MaxResistanceBonus;
		}
	TArray &getArray(uint32 index)
		{
			nlassert(index < 5);
			return _Array[index];
		}
		
	};
		
	class TMAGIC_PROTECTION
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_VALUE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setVALUE(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<9)-1, "setVALUE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 9 bits");
				

			_setProp(dbGroup, _VALUE, value, forceSending);
		}

		uint16 getVALUE(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _VALUE, value);

			return value;
		}
		
		ICDBStructNode *getVALUECDBNode()
		{
			return _VALUE;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_MaxProtectionClampValue;
		ICDBStructNode	*_MaxAbsorptionFactor;
		TArray _Array[7];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setMaxProtectionClampValue(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<9)-1, "setMaxProtectionClampValue : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 9 bits");
				

			_setProp(dbGroup, _MaxProtectionClampValue, value, forceSending);
		}

		uint16 getMaxProtectionClampValue(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _MaxProtectionClampValue, value);

			return value;
		}
		
		ICDBStructNode *getMaxProtectionClampValueCDBNode()
		{
			return _MaxProtectionClampValue;
		}
	
		void setMaxAbsorptionFactor(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<9)-1, "setMaxAbsorptionFactor : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 9 bits");
				

			_setProp(dbGroup, _MaxAbsorptionFactor, value, forceSending);
		}

		uint16 getMaxAbsorptionFactor(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _MaxAbsorptionFactor, value);

			return value;
		}
		
		ICDBStructNode *getMaxAbsorptionFactorCDBNode()
		{
			return _MaxAbsorptionFactor;
		}
	TArray &getArray(uint32 index)
		{
			nlassert(index < 7);
			return _Array[index];
		}
		
	};
		
	class TDODGE
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_Base;
		ICDBStructNode	*_Current;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setBase(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<9)-1, "setBase : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 9 bits");
				

			_setProp(dbGroup, _Base, value, forceSending);
		}

		uint16 getBase(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _Base, value);

			return value;
		}
		
		ICDBStructNode *getBaseCDBNode()
		{
			return _Base;
		}
	
		void setCurrent(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<9)-1, "setCurrent : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 9 bits");
				

			_setProp(dbGroup, _Current, value, forceSending);
		}

		uint16 getCurrent(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _Current, value);

			return value;
		}
		
		ICDBStructNode *getCurrentCDBNode()
		{
			return _Current;
		}
	
	};
		
	class TPARRY
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_Base;
		ICDBStructNode	*_Current;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setBase(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<9)-1, "setBase : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 9 bits");
				

			_setProp(dbGroup, _Base, value, forceSending);
		}

		uint16 getBase(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _Base, value);

			return value;
		}
		
		ICDBStructNode *getBaseCDBNode()
		{
			return _Base;
		}
	
		void setCurrent(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<9)-1, "setCurrent : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 9 bits");
				

			_setProp(dbGroup, _Current, value, forceSending);
		}

		uint16 getCurrent(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _Current, value);

			return value;
		}
		
		ICDBStructNode *getCurrentCDBNode()
		{
			return _Current;
		}
	
	};
		
	class TSKILLS
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_SKILL;
		ICDBStructNode	*_BaseSKILL;
		ICDBStructNode	*_PROGRESS_BAR;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setSKILL(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<12)-1, "setSKILL : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 12 bits");
				

			_setProp(dbGroup, _SKILL, value, forceSending);
		}

		uint16 getSKILL(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _SKILL, value);

			return value;
		}
		
		ICDBStructNode *getSKILLCDBNode()
		{
			return _SKILL;
		}
	
		void setBaseSKILL(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<12)-1, "setBaseSKILL : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 12 bits");
				

			_setProp(dbGroup, _BaseSKILL, value, forceSending);
		}

		uint16 getBaseSKILL(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _BaseSKILL, value);

			return value;
		}
		
		ICDBStructNode *getBaseSKILLCDBNode()
		{
			return _BaseSKILL;
		}
	
		void setPROGRESS_BAR(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PROGRESS_BAR, value, forceSending);
		}

		uint8 getPROGRESS_BAR(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _PROGRESS_BAR, value);

			return value;
		}
		
		ICDBStructNode *getPROGRESS_BARCDBNode()
		{
			return _PROGRESS_BAR;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[225];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 225);
			return _Array[index];
		}
		
	};
		
	class TXP_CATALYSER
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_Level;
		ICDBStructNode	*_Count;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setLevel(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<12)-1, "setLevel : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 12 bits");
				

			_setProp(dbGroup, _Level, value, forceSending);
		}

		uint16 getLevel(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _Level, value);

			return value;
		}
		
		ICDBStructNode *getLevelCDBNode()
		{
			return _Level;
		}
	
		void setCount(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<12)-1, "setCount : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 12 bits");
				

			_setProp(dbGroup, _Count, value, forceSending);
		}

		uint16 getCount(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _Count, value);

			return value;
		}
		
		ICDBStructNode *getCountCDBNode()
		{
			return _Count;
		}
	
	};
		
	class TRING_XP_CATALYSER
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_Level;
		ICDBStructNode	*_Count;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setLevel(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<12)-1, "setLevel : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 12 bits");
				

			_setProp(dbGroup, _Level, value, forceSending);
		}

		uint16 getLevel(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _Level, value);

			return value;
		}
		
		ICDBStructNode *getLevelCDBNode()
		{
			return _Level;
		}
	
		void setCount(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<12)-1, "setCount : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 12 bits");
				

			_setProp(dbGroup, _Count, value, forceSending);
		}

		uint16 getCount(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _Count, value);

			return value;
		}
		
		ICDBStructNode *getCountCDBNode()
		{
			return _Count;
		}
	
	};
		
	class TPVP_FACTION_TAG
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_TAG_PVP;
		ICDBStructNode	*_ACTIVATION_TIME;
		ICDBStructNode	*_FLAG_PVP_TIME_LEFT;
		ICDBStructNode	*_COUNTER;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setTAG_PVP(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TAG_PVP, value, forceSending);
		}

		bool getTAG_PVP(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _TAG_PVP, value);

			return value;
		}
		
		ICDBStructNode *getTAG_PVPCDBNode()
		{
			return _TAG_PVP;
		}
	
		void setACTIVATION_TIME(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ACTIVATION_TIME, value, forceSending);
		}

		uint32 getACTIVATION_TIME(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _ACTIVATION_TIME, value);

			return value;
		}
		
		ICDBStructNode *getACTIVATION_TIMECDBNode()
		{
			return _ACTIVATION_TIME;
		}
	
		void setFLAG_PVP_TIME_LEFT(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _FLAG_PVP_TIME_LEFT, value, forceSending);
		}

		uint32 getFLAG_PVP_TIME_LEFT(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _FLAG_PVP_TIME_LEFT, value);

			return value;
		}
		
		ICDBStructNode *getFLAG_PVP_TIME_LEFTCDBNode()
		{
			return _FLAG_PVP_TIME_LEFT;
		}
	
		void setCOUNTER(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _COUNTER, value, forceSending);
		}

		uint8 getCOUNTER(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _COUNTER, value);

			return value;
		}
		
		ICDBStructNode *getCOUNTERCDBNode()
		{
			return _COUNTER;
		}
	
	};
		
	class TPVP_OUTPOST
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_FLAG_PVP;
		ICDBStructNode	*_RIGHT_TO_BANISH;
		ICDBStructNode	*_ROUND_LVL_CUR;
		ICDBStructNode	*_ROUND_END_DATE;
		ICDBStructNode	*_FLAG_PVP_TIME_END;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setFLAG_PVP(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _FLAG_PVP, value, forceSending);
		}

		bool getFLAG_PVP(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _FLAG_PVP, value);

			return value;
		}
		
		ICDBStructNode *getFLAG_PVPCDBNode()
		{
			return _FLAG_PVP;
		}
	
		void setRIGHT_TO_BANISH(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _RIGHT_TO_BANISH, value, forceSending);
		}

		bool getRIGHT_TO_BANISH(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _RIGHT_TO_BANISH, value);

			return value;
		}
		
		ICDBStructNode *getRIGHT_TO_BANISHCDBNode()
		{
			return _RIGHT_TO_BANISH;
		}
	
		void setROUND_LVL_CUR(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ROUND_LVL_CUR, value, forceSending);
		}

		uint32 getROUND_LVL_CUR(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _ROUND_LVL_CUR, value);

			return value;
		}
		
		ICDBStructNode *getROUND_LVL_CURCDBNode()
		{
			return _ROUND_LVL_CUR;
		}
	
		void setROUND_END_DATE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ROUND_END_DATE, value, forceSending);
		}

		uint32 getROUND_END_DATE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _ROUND_END_DATE, value);

			return value;
		}
		
		ICDBStructNode *getROUND_END_DATECDBNode()
		{
			return _ROUND_END_DATE;
		}
	
		void setFLAG_PVP_TIME_END(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _FLAG_PVP_TIME_END, value, forceSending);
		}

		uint32 getFLAG_PVP_TIME_END(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _FLAG_PVP_TIME_END, value);

			return value;
		}
		
		ICDBStructNode *getFLAG_PVP_TIME_ENDCDBNode()
		{
			return _FLAG_PVP_TIME_END;
		}
	
	};
		
	class TSUCCESS_MODIFIER
	{
	public:
		
	class TECO
	{
	public:
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_FORAGE;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setFORAGE(CCDBSynchronised &dbGroup, sint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _FORAGE, value, forceSending);
		}

		sint16 getFORAGE(const CCDBSynchronised &dbGroup)
		{
			sint16 value;
			_getProp(dbGroup, _FORAGE, value);

			return value;
		}
		
		ICDBStructNode *getFORAGECDBNode()
		{
			return _FORAGE;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TArray _Array[7];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TArray &getArray(uint32 index)
		{
			nlassert(index < 7);
			return _Array[index];
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_DODGE;
		ICDBStructNode	*_PARRY;
		ICDBStructNode	*_CRAFT;
		ICDBStructNode	*_MELEE;
		ICDBStructNode	*_RANGE;
		ICDBStructNode	*_MAGIC;
		TECO	_ECO;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setDODGE(CCDBSynchronised &dbGroup, sint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _DODGE, value, forceSending);
		}

		sint16 getDODGE(const CCDBSynchronised &dbGroup)
		{
			sint16 value;
			_getProp(dbGroup, _DODGE, value);

			return value;
		}
		
		ICDBStructNode *getDODGECDBNode()
		{
			return _DODGE;
		}
	
		void setPARRY(CCDBSynchronised &dbGroup, sint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PARRY, value, forceSending);
		}

		sint16 getPARRY(const CCDBSynchronised &dbGroup)
		{
			sint16 value;
			_getProp(dbGroup, _PARRY, value);

			return value;
		}
		
		ICDBStructNode *getPARRYCDBNode()
		{
			return _PARRY;
		}
	
		void setCRAFT(CCDBSynchronised &dbGroup, sint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _CRAFT, value, forceSending);
		}

		sint16 getCRAFT(const CCDBSynchronised &dbGroup)
		{
			sint16 value;
			_getProp(dbGroup, _CRAFT, value);

			return value;
		}
		
		ICDBStructNode *getCRAFTCDBNode()
		{
			return _CRAFT;
		}
	
		void setMELEE(CCDBSynchronised &dbGroup, sint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _MELEE, value, forceSending);
		}

		sint16 getMELEE(const CCDBSynchronised &dbGroup)
		{
			sint16 value;
			_getProp(dbGroup, _MELEE, value);

			return value;
		}
		
		ICDBStructNode *getMELEECDBNode()
		{
			return _MELEE;
		}
	
		void setRANGE(CCDBSynchronised &dbGroup, sint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _RANGE, value, forceSending);
		}

		sint16 getRANGE(const CCDBSynchronised &dbGroup)
		{
			sint16 value;
			_getProp(dbGroup, _RANGE, value);

			return value;
		}
		
		ICDBStructNode *getRANGECDBNode()
		{
			return _RANGE;
		}
	
		void setMAGIC(CCDBSynchronised &dbGroup, sint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _MAGIC, value, forceSending);
		}

		sint16 getMAGIC(const CCDBSynchronised &dbGroup)
		{
			sint16 value;
			_getProp(dbGroup, _MAGIC, value);

			return value;
		}
		
		ICDBStructNode *getMAGICCDBNode()
		{
			return _MAGIC;
		}
	TECO &getECO()
		{
			return _ECO;
		}
		
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TCHARACTERISTICS _CHARACTERISTICS[8];
		TSCORES _SCORES[4];
		TMAGIC_RESISTANCE	_MAGIC_RESISTANCE;
		TMAGIC_PROTECTION	_MAGIC_PROTECTION;
		TDODGE	_DODGE;
		TPARRY	_PARRY;
		TSKILLS	_SKILLS;
		TXP_CATALYSER	_XP_CATALYSER;
		TRING_XP_CATALYSER	_RING_XP_CATALYSER;
		TPVP_FACTION_TAG	_PVP_FACTION_TAG;
		TPVP_OUTPOST	_PVP_OUTPOST;
		TSUCCESS_MODIFIER	_SUCCESS_MODIFIER;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TCHARACTERISTICS &getCHARACTERISTICS(uint32 index)
		{
			nlassert(index < 8);
			return _CHARACTERISTICS[index];
		}
		TSCORES &getSCORES(uint32 index)
		{
			nlassert(index < 4);
			return _SCORES[index];
		}
		TMAGIC_RESISTANCE &getMAGIC_RESISTANCE()
		{
			return _MAGIC_RESISTANCE;
		}
		TMAGIC_PROTECTION &getMAGIC_PROTECTION()
		{
			return _MAGIC_PROTECTION;
		}
		TDODGE &getDODGE()
		{
			return _DODGE;
		}
		TPARRY &getPARRY()
		{
			return _PARRY;
		}
		TSKILLS &getSKILLS()
		{
			return _SKILLS;
		}
		TXP_CATALYSER &getXP_CATALYSER()
		{
			return _XP_CATALYSER;
		}
		TRING_XP_CATALYSER &getRING_XP_CATALYSER()
		{
			return _RING_XP_CATALYSER;
		}
		TPVP_FACTION_TAG &getPVP_FACTION_TAG()
		{
			return _PVP_FACTION_TAG;
		}
		TPVP_OUTPOST &getPVP_OUTPOST()
		{
			return _PVP_OUTPOST;
		}
		TSUCCESS_MODIFIER &getSUCCESS_MODIFIER()
		{
			return _SUCCESS_MODIFIER;
		}
		
	};
		
	class TPACK_ANIMAL
	{
	public:
		
	class TBEAST
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_UID;
		ICDBStructNode	*_TYPE;
		ICDBStructNode	*_STATUS;
		ICDBStructNode	*_HP;
		ICDBStructNode	*_BULK_MAX;
		ICDBStructNode	*_POS;
		ICDBStructNode	*_HUNGER;
		ICDBStructNode	*_DESPAWN;
		ICDBStructNode	*_NAME;

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setUID(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<20)-1, "setUID : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 20 bits");
				

			_setProp(dbGroup, _UID, value, forceSending);
		}

		uint32 getUID(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _UID, value);

			return value;
		}
		
		ICDBStructNode *getUIDCDBNode()
		{
			return _UID;
		}
	
		void setTYPE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setTYPE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _TYPE, value, forceSending);
		}

		uint8 getTYPE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _TYPE, value);

			return value;
		}
		
		ICDBStructNode *getTYPECDBNode()
		{
			return _TYPE;
		}
	
		void setSTATUS(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<4)-1, "setSTATUS : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 4 bits");
				

			_setProp(dbGroup, _STATUS, value, forceSending);
		}

		uint8 getSTATUS(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _STATUS, value);

			return value;
		}
		
		ICDBStructNode *getSTATUSCDBNode()
		{
			return _STATUS;
		}
	
		void setHP(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setHP : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _HP, value, forceSending);
		}

		uint8 getHP(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _HP, value);

			return value;
		}
		
		ICDBStructNode *getHPCDBNode()
		{
			return _HP;
		}
	
		void setBULK_MAX(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BULK_MAX, value, forceSending);
		}

		uint32 getBULK_MAX(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _BULK_MAX, value);

			return value;
		}
		
		ICDBStructNode *getBULK_MAXCDBNode()
		{
			return _BULK_MAX;
		}
	
		void setPOS(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _POS, value, forceSending);
		}

		uint64 getPOS(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _POS, value);

			return value;
		}
		
		ICDBStructNode *getPOSCDBNode()
		{
			return _POS;
		}
	
		void setHUNGER(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<5)-1, "setHUNGER : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 5 bits");
				

			_setProp(dbGroup, _HUNGER, value, forceSending);
		}

		uint8 getHUNGER(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _HUNGER, value);

			return value;
		}
		
		ICDBStructNode *getHUNGERCDBNode()
		{
			return _HUNGER;
		}
	
		void setDESPAWN(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<7)-1, "setDESPAWN : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 7 bits");
				

			_setProp(dbGroup, _DESPAWN, value, forceSending);
		}

		uint8 getDESPAWN(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _DESPAWN, value);

			return value;
		}
		
		ICDBStructNode *getDESPAWNCDBNode()
		{
			return _DESPAWN;
		}

		void setNAME(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			_setProp(dbGroup, _NAME, value, forceSending);
		}

		uint32 getNAME(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NAME, value);

			return value;
		}

		ICDBStructNode *getNAMECDBNode()
		{
			return _NAME;
		}
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TBEAST _BEAST[4];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TBEAST &getBEAST(uint32 index)
		{
			nlassert(index < 4);
			return _BEAST[index];
		}
		
	};
		
	class TDEBUG_INFO
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_Ping;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setPing(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _Ping, value, forceSending);
		}

		uint32 getPing(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _Ping, value);

			return value;
		}
		
		ICDBStructNode *getPingCDBNode()
		{
			return _Ping;
		}
	
	};
		
	class TMP_EVAL
	{
	public:
		
	class TRESULT
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_VALID;
		ICDBStructNode	*_SMALL_SEED;
		ICDBStructNode	*_MEDIUM_SEED;
		ICDBStructNode	*_BIG_SEED;
		ICDBStructNode	*_VERY_BIG_SEED;
		ICDBStructNode	*_EXPIRY_DATE;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setVALID(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VALID, value, forceSending);
		}

		bool getVALID(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _VALID, value);

			return value;
		}
		
		ICDBStructNode *getVALIDCDBNode()
		{
			return _VALID;
		}
	
		void setSMALL_SEED(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SMALL_SEED, value, forceSending);
		}

		uint32 getSMALL_SEED(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _SMALL_SEED, value);

			return value;
		}
		
		ICDBStructNode *getSMALL_SEEDCDBNode()
		{
			return _SMALL_SEED;
		}
	
		void setMEDIUM_SEED(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _MEDIUM_SEED, value, forceSending);
		}

		uint32 getMEDIUM_SEED(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _MEDIUM_SEED, value);

			return value;
		}
		
		ICDBStructNode *getMEDIUM_SEEDCDBNode()
		{
			return _MEDIUM_SEED;
		}
	
		void setBIG_SEED(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BIG_SEED, value, forceSending);
		}

		uint32 getBIG_SEED(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _BIG_SEED, value);

			return value;
		}
		
		ICDBStructNode *getBIG_SEEDCDBNode()
		{
			return _BIG_SEED;
		}
	
		void setVERY_BIG_SEED(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VERY_BIG_SEED, value, forceSending);
		}

		uint32 getVERY_BIG_SEED(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _VERY_BIG_SEED, value);

			return value;
		}
		
		ICDBStructNode *getVERY_BIG_SEEDCDBNode()
		{
			return _VERY_BIG_SEED;
		}
	
		void setEXPIRY_DATE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _EXPIRY_DATE, value, forceSending);
		}

		uint32 getEXPIRY_DATE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _EXPIRY_DATE, value);

			return value;
		}
		
		ICDBStructNode *getEXPIRY_DATECDBNode()
		{
			return _EXPIRY_DATE;
		}
	
	};
		
	class TRESULT_CRITICAL
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_VALID;
		ICDBStructNode	*_SMALL_SEED;
		ICDBStructNode	*_MEDIUM_SEED;
		ICDBStructNode	*_BIG_SEED;
		ICDBStructNode	*_VERY_BIG_SEED;
		ICDBStructNode	*_EXPIRY_DATE;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setVALID(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VALID, value, forceSending);
		}

		bool getVALID(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _VALID, value);

			return value;
		}
		
		ICDBStructNode *getVALIDCDBNode()
		{
			return _VALID;
		}
	
		void setSMALL_SEED(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _SMALL_SEED, value, forceSending);
		}

		uint32 getSMALL_SEED(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _SMALL_SEED, value);

			return value;
		}
		
		ICDBStructNode *getSMALL_SEEDCDBNode()
		{
			return _SMALL_SEED;
		}
	
		void setMEDIUM_SEED(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _MEDIUM_SEED, value, forceSending);
		}

		uint32 getMEDIUM_SEED(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _MEDIUM_SEED, value);

			return value;
		}
		
		ICDBStructNode *getMEDIUM_SEEDCDBNode()
		{
			return _MEDIUM_SEED;
		}
	
		void setBIG_SEED(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BIG_SEED, value, forceSending);
		}

		uint32 getBIG_SEED(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _BIG_SEED, value);

			return value;
		}
		
		ICDBStructNode *getBIG_SEEDCDBNode()
		{
			return _BIG_SEED;
		}
	
		void setVERY_BIG_SEED(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VERY_BIG_SEED, value, forceSending);
		}

		uint32 getVERY_BIG_SEED(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _VERY_BIG_SEED, value);

			return value;
		}
		
		ICDBStructNode *getVERY_BIG_SEEDCDBNode()
		{
			return _VERY_BIG_SEED;
		}
	
		void setEXPIRY_DATE(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _EXPIRY_DATE, value, forceSending);
		}

		uint32 getEXPIRY_DATE(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _EXPIRY_DATE, value);

			return value;
		}
		
		ICDBStructNode *getEXPIRY_DATECDBNode()
		{
			return _EXPIRY_DATE;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_COST;
		TRESULT	_RESULT;
		TRESULT_CRITICAL	_RESULT_CRITICAL;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setCOST(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _COST, value, forceSending);
		}

		uint32 getCOST(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _COST, value);

			return value;
		}
		
		ICDBStructNode *getCOSTCDBNode()
		{
			return _COST;
		}
	TRESULT &getRESULT()
		{
			return _RESULT;
		}
		TRESULT_CRITICAL &getRESULT_CRITICAL()
		{
			return _RESULT_CRITICAL;
		}
		
	};
		
	class TCOMPASS
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_HOME_POINT;
		ICDBStructNode	*_BIND_POINT;
		ICDBStructNode	*_TARGET;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setHOME_POINT(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _HOME_POINT, value, forceSending);
		}

		uint64 getHOME_POINT(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _HOME_POINT, value);

			return value;
		}
		
		ICDBStructNode *getHOME_POINTCDBNode()
		{
			return _HOME_POINT;
		}
	
		void setBIND_POINT(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BIND_POINT, value, forceSending);
		}

		uint64 getBIND_POINT(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _BIND_POINT, value);

			return value;
		}
		
		ICDBStructNode *getBIND_POINTCDBNode()
		{
			return _BIND_POINT;
		}
	
		void setTARGET(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TARGET, value, forceSending);
		}

		uint64 getTARGET(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _TARGET, value);

			return value;
		}
		
		ICDBStructNode *getTARGETCDBNode()
		{
			return _TARGET;
		}
	
	};
		
	class TFAME
	{
	public:
		
	class TPLAYER
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

		
		void setVALUE(CCDBSynchronised &dbGroup, sint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VALUE, value, forceSending);
		}

		sint8 getVALUE(const CCDBSynchronised &dbGroup)
		{
			sint8 value;
			_getProp(dbGroup, _VALUE, value);

			return value;
		}
		
		ICDBStructNode *getVALUECDBNode()
		{
			return _VALUE;
		}
	
		void setTHRESHOLD(CCDBSynchronised &dbGroup, sint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _THRESHOLD, value, forceSending);
		}

		sint8 getTHRESHOLD(const CCDBSynchronised &dbGroup)
		{
			sint8 value;
			_getProp(dbGroup, _THRESHOLD, value);

			return value;
		}
		
		ICDBStructNode *getTHRESHOLDCDBNode()
		{
			return _THRESHOLD;
		}
	
		void setTREND(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TREND, value, forceSending);
		}

		uint8 getTREND(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _TREND, value);

			return value;
		}
		
		ICDBStructNode *getTRENDCDBNode()
		{
			return _TREND;
		}
	
	};
		
	class TTRIBE
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

		
		void setVALUE(CCDBSynchronised &dbGroup, sint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VALUE, value, forceSending);
		}

		sint8 getVALUE(const CCDBSynchronised &dbGroup)
		{
			sint8 value;
			_getProp(dbGroup, _VALUE, value);

			return value;
		}
		
		ICDBStructNode *getVALUECDBNode()
		{
			return _VALUE;
		}
	
		void setTHRESHOLD(CCDBSynchronised &dbGroup, sint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _THRESHOLD, value, forceSending);
		}

		sint8 getTHRESHOLD(const CCDBSynchronised &dbGroup)
		{
			sint8 value;
			_getProp(dbGroup, _THRESHOLD, value);

			return value;
		}
		
		ICDBStructNode *getTHRESHOLDCDBNode()
		{
			return _THRESHOLD;
		}
	
		void setTREND(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _TREND, value, forceSending);
		}

		uint8 getTREND(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _TREND, value);

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
		ICDBStructNode	*_THRESHOLD_TRADE;
		ICDBStructNode	*_THRESHOLD_KOS;
		TPLAYER _PLAYER[6];
		TTRIBE _TRIBE[53];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setCULT_ALLEGIANCE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setCULT_ALLEGIANCE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _CULT_ALLEGIANCE, value, forceSending);
		}

		uint8 getCULT_ALLEGIANCE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _CULT_ALLEGIANCE, value);

			return value;
		}
		
		ICDBStructNode *getCULT_ALLEGIANCECDBNode()
		{
			return _CULT_ALLEGIANCE;
		}
	
		void setCIV_ALLEGIANCE(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			
			// Check that the value is not out of database precision
			STOP_IF(value > (1<<3)-1, "setCIV_ALLEGIANCE : Value out of bound : trying to store "<<value<<" in a unsigned field limited to 3 bits");
				

			_setProp(dbGroup, _CIV_ALLEGIANCE, value, forceSending);
		}

		uint8 getCIV_ALLEGIANCE(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _CIV_ALLEGIANCE, value);

			return value;
		}
		
		ICDBStructNode *getCIV_ALLEGIANCECDBNode()
		{
			return _CIV_ALLEGIANCE;
		}
	
		void setTHRESHOLD_TRADE(CCDBSynchronised &dbGroup, sint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _THRESHOLD_TRADE, value, forceSending);
		}

		sint8 getTHRESHOLD_TRADE(const CCDBSynchronised &dbGroup)
		{
			sint8 value;
			_getProp(dbGroup, _THRESHOLD_TRADE, value);

			return value;
		}
		
		ICDBStructNode *getTHRESHOLD_TRADECDBNode()
		{
			return _THRESHOLD_TRADE;
		}
	
		void setTHRESHOLD_KOS(CCDBSynchronised &dbGroup, sint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _THRESHOLD_KOS, value, forceSending);
		}

		sint8 getTHRESHOLD_KOS(const CCDBSynchronised &dbGroup)
		{
			sint8 value;
			_getProp(dbGroup, _THRESHOLD_KOS, value);

			return value;
		}
		
		ICDBStructNode *getTHRESHOLD_KOSCDBNode()
		{
			return _THRESHOLD_KOS;
		}
	TPLAYER &getPLAYER(uint32 index)
		{
			nlassert(index < 6);
			return _PLAYER[index];
		}
		TTRIBE &getTRIBE(uint32 index)
		{
			nlassert(index < 53);
			return _TRIBE[index];
		}
		
	};
		
	class TSTATIC_DATA
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_BAG_BULK_MAX;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setBAG_BULK_MAX(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _BAG_BULK_MAX, value, forceSending);
		}

		uint32 getBAG_BULK_MAX(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _BAG_BULK_MAX, value);

			return value;
		}
		
		ICDBStructNode *getBAG_BULK_MAXCDBNode()
		{
			return _BAG_BULK_MAX;
		}
	
	};
		
	class TDYN_CHAT
	{
	public:
		
	class TCHANNEL
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_NAME;
		ICDBStructNode	*_ID;
		ICDBStructNode	*_WRITE_RIGHT;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setNAME(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _NAME, value, forceSending);
		}

		uint32 getNAME(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _NAME, value);

			return value;
		}
		
		ICDBStructNode *getNAMECDBNode()
		{
			return _NAME;
		}
	
		void setID(CCDBSynchronised &dbGroup, uint64 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ID, value, forceSending);
		}

		uint64 getID(const CCDBSynchronised &dbGroup)
		{
			uint64 value;
			_getProp(dbGroup, _ID, value);

			return value;
		}
		
		ICDBStructNode *getIDCDBNode()
		{
			return _ID;
		}
	
		void setWRITE_RIGHT(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _WRITE_RIGHT, value, forceSending);
		}

		bool getWRITE_RIGHT(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _WRITE_RIGHT, value);

			return value;
		}
		
		ICDBStructNode *getWRITE_RIGHTCDBNode()
		{
			return _WRITE_RIGHT;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TCHANNEL _CHANNEL[8];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TCHANNEL &getCHANNEL(uint32 index)
		{
			nlassert(index < 8);
			return _CHANNEL[index];
		}
		
	};
		
	class TPVP_EFFECTS
	{
	public:
		
	class TPVP_FACTION_POINTS
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_CIV;
		ICDBStructNode	*_CIV_POINTS;
		ICDBStructNode	*_CULT;
		ICDBStructNode	*_CULT_POINTS;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setCIV(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _CIV, value, forceSending);
		}

		uint8 getCIV(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _CIV, value);

			return value;
		}
		
		ICDBStructNode *getCIVCDBNode()
		{
			return _CIV;
		}
	
		void setCIV_POINTS(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _CIV_POINTS, value, forceSending);
		}

		uint32 getCIV_POINTS(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _CIV_POINTS, value);

			return value;
		}
		
		ICDBStructNode *getCIV_POINTSCDBNode()
		{
			return _CIV_POINTS;
		}
	
		void setCULT(CCDBSynchronised &dbGroup, uint8 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _CULT, value, forceSending);
		}

		uint8 getCULT(const CCDBSynchronised &dbGroup)
		{
			uint8 value;
			_getProp(dbGroup, _CULT, value);

			return value;
		}
		
		ICDBStructNode *getCULTCDBNode()
		{
			return _CULT;
		}
	
		void setCULT_POINTS(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _CULT_POINTS, value, forceSending);
		}

		uint32 getCULT_POINTS(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _CULT_POINTS, value);

			return value;
		}
		
		ICDBStructNode *getCULT_POINTSCDBNode()
		{
			return _CULT_POINTS;
		}
	
	};
		
	class TArray
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_ID;
		ICDBStructNode	*_ISBONUS;
		ICDBStructNode	*_PARAM;
		

	public:
		void init(ICDBStructNode *parent, uint index);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setID(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ID, value, forceSending);
		}

		uint32 getID(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _ID, value);

			return value;
		}
		
		ICDBStructNode *getIDCDBNode()
		{
			return _ID;
		}
	
		void setISBONUS(CCDBSynchronised &dbGroup, bool value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _ISBONUS, value, forceSending);
		}

		bool getISBONUS(const CCDBSynchronised &dbGroup)
		{
			bool value;
			_getProp(dbGroup, _ISBONUS, value);

			return value;
		}
		
		ICDBStructNode *getISBONUSCDBNode()
		{
			return _ISBONUS;
		}
	
		void setPARAM(CCDBSynchronised &dbGroup, uint32 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _PARAM, value, forceSending);
		}

		uint32 getPARAM(const CCDBSynchronised &dbGroup)
		{
			uint32 value;
			_getProp(dbGroup, _PARAM, value);

			return value;
		}
		
		ICDBStructNode *getPARAMCDBNode()
		{
			return _PARAM;
		}
	
	};
		

	private:
		ICDBStructNode	*_BranchNode;

		TPVP_FACTION_POINTS	_PVP_FACTION_POINTS;
		TArray _Array[59];
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		TPVP_FACTION_POINTS &getPVP_FACTION_POINTS()
		{
			return _PVP_FACTION_POINTS;
		}
		TArray &getArray(uint32 index)
		{
			nlassert(index < 59);
			return _Array[index];
		}
		
	};
		
	class TWEATHER
	{
	public:
		

	private:
		ICDBStructNode	*_BranchNode;

		ICDBStructNode	*_VALUE;
		

	public:
		void init(ICDBStructNode *parent);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		
		void setVALUE(CCDBSynchronised &dbGroup, uint16 value, bool forceSending = false)
		{
			

			_setProp(dbGroup, _VALUE, value, forceSending);
		}

		uint16 getVALUE(const CCDBSynchronised &dbGroup)
		{
			uint16 value;
			_getProp(dbGroup, _VALUE, value);

			return value;
		}
		
		ICDBStructNode *getVALUECDBNode()
		{
			return _VALUE;
		}
	
	};
		
		static TGameTime	_GameTime;

		static TINTERFACES	_INTERFACES;

		static TUSER	_USER;

		static TDEFENSE	_DEFENSE;

		static TFLAGS	_FLAGS;

		static TTARGET	_TARGET;

		static TGROUP	_GROUP;

		static TDM_GIFT	_DM_GIFT;

		static TEXCHANGE	_EXCHANGE;

		static TINVENTORY	_INVENTORY;

		static TMODIFIERS	_MODIFIERS;

		static TDISABLE_CONSUMABLE	_DISABLE_CONSUMABLE;

		static TBOTCHAT	_BOTCHAT;

		static TASCENSOR	_ASCENSOR;

		static TCHOOSE_MISSIONS	_CHOOSE_MISSIONS;

		static TTRADING	_TRADING;

		static TBRICK_FAMILY	_BRICK_FAMILY;

		static TFABER_PLANS	_FABER_PLANS;

		static TMISSIONS	_MISSIONS;

		static TEXECUTE_PHRASE	_EXECUTE_PHRASE;

		static TCHARACTER_INFO	_CHARACTER_INFO;

		static TPACK_ANIMAL	_PACK_ANIMAL;

		static TDEBUG_INFO	_DEBUG_INFO;

		static TMP_EVAL	_MP_EVAL;

		static TCOMPASS	_COMPASS;

		static TFAME	_FAME;

		static TSTATIC_DATA	_STATIC_DATA;

		static TDYN_CHAT	_DYN_CHAT;

		static TPVP_EFFECTS	_PVP_EFFECTS;

		static TWEATHER	_WEATHER;


	public:

		// Constructor
		CBankAccessor_PLR()
		{
			// make sure the static tree is initialised (some kind of lazy initialisation)
			init();

			// init the base class
			CCDBSynchronised::init(BankTag);
		}

		
		static void init();

		static TGameTime &getGameTime()
		{
			return _GameTime;
		}
		static TINTERFACES &getINTERFACES()
		{
			return _INTERFACES;
		}
		static TUSER &getUSER()
		{
			return _USER;
		}
		static TDEFENSE &getDEFENSE()
		{
			return _DEFENSE;
		}
		static TFLAGS &getFLAGS()
		{
			return _FLAGS;
		}
		static TTARGET &getTARGET()
		{
			return _TARGET;
		}
		static TGROUP &getGROUP()
		{
			return _GROUP;
		}
		static TDM_GIFT &getDM_GIFT()
		{
			return _DM_GIFT;
		}
		static TEXCHANGE &getEXCHANGE()
		{
			return _EXCHANGE;
		}
		static TINVENTORY &getINVENTORY()
		{
			return _INVENTORY;
		}
		static TMODIFIERS &getMODIFIERS()
		{
			return _MODIFIERS;
		}
		static TDISABLE_CONSUMABLE &getDISABLE_CONSUMABLE()
		{
			return _DISABLE_CONSUMABLE;
		}
		static TBOTCHAT &getBOTCHAT()
		{
			return _BOTCHAT;
		}
		static TASCENSOR &getASCENSOR()
		{
			return _ASCENSOR;
		}
		static TCHOOSE_MISSIONS &getCHOOSE_MISSIONS()
		{
			return _CHOOSE_MISSIONS;
		}
		static TTRADING &getTRADING()
		{
			return _TRADING;
		}
		static TBRICK_FAMILY &getBRICK_FAMILY()
		{
			return _BRICK_FAMILY;
		}
		static TFABER_PLANS &getFABER_PLANS()
		{
			return _FABER_PLANS;
		}
		static TMISSIONS &getMISSIONS()
		{
			return _MISSIONS;
		}
		static TEXECUTE_PHRASE &getEXECUTE_PHRASE()
		{
			return _EXECUTE_PHRASE;
		}
		static TCHARACTER_INFO &getCHARACTER_INFO()
		{
			return _CHARACTER_INFO;
		}
		static TPACK_ANIMAL &getPACK_ANIMAL()
		{
			return _PACK_ANIMAL;
		}
		static TDEBUG_INFO &getDEBUG_INFO()
		{
			return _DEBUG_INFO;
		}
		static TMP_EVAL &getMP_EVAL()
		{
			return _MP_EVAL;
		}
		static TCOMPASS &getCOMPASS()
		{
			return _COMPASS;
		}
		static TFAME &getFAME()
		{
			return _FAME;
		}
		static TSTATIC_DATA &getSTATIC_DATA()
		{
			return _STATIC_DATA;
		}
		static TDYN_CHAT &getDYN_CHAT()
		{
			return _DYN_CHAT;
		}
		static TPVP_EFFECTS &getPVP_EFFECTS()
		{
			return _PVP_EFFECTS;
		}
		static TWEATHER &getWEATHER()
		{
			return _WEATHER;
		}
		

	};
	

#endif // INCLUDED_DATABASE_PLR_H
