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



#ifndef RY_ROLE_PERSISTANT_DATA_H
#define RY_ROLE_PERSISTANT_DATA_H
//
//stdpch //#include "nel/misc/types_nl.h"
//#include "nel/misc/stream.h"
//
////#include "game_share/jobs.h"
//#include "game_share/skills.h"
//
//
//class CRolePersistantData
//{
//public:
//	/// Constructor
//	CRolePersistantData();
//
//	struct SSkillsProgress
//	{
//		uint16 LevelReached;
//		uint16 SkillCap;
//		
//		SSkillsProgress() { LevelReached = SkillCap = 0; }
//		
//		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
//		{
//			f.serial( LevelReached );
//			f.serial( SkillCap );
//		}
//	};
//	
//	typedef std::map< std::string, SSkillsProgress > TSkillLevelReachedContainer;
//	
//	struct SRole
//	{
//		JOBS::TJob			Job;
//		JOBS::TJobStatus	JobStatus;
//		NLMISC::TGameCycle	BeginFreezeTime;
//		NLMISC::TGameCycle	UnfreezeTime;
//		uint16				JobLevel;
//		NLMISC::CSheetId	RoleSheet;
//		
//		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
//		{
//			f.serialEnum( Job );
//			f.serialEnum( JobStatus );
//			f.serial( BeginFreezeTime );
//			f.serial( UnfreezeTime );
//			f.serial( JobLevel );
//			std::string sheetName;
//			if( f.isReading() )
//			{
//				f.serial( sheetName );
//				RoleSheet = NLMISC::CSheetId( sheetName );
//			}
//			else
//			{
//				sheetName = RoleSheet.toString();
//				f.serial( sheetName );
//			}
//		}
//	};	
//	
//	// Serial
//	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
//	{
//		f.serialCont( _Roles );
//		f.serialCont( _LevelReached );
//	}
//
//	//public attributes
//	std::vector< SRole > _Roles;
//	TSkillLevelReachedContainer _LevelReached; 
//
//}; //CRolePersistantData

#endif // RY_ROLE_PERSISTANT_DATA_H
/* role_persistant_data.h */
