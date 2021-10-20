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




//////////////
// INCLUDES //
//////////////
#include "stdpch.h"
//// Application
//#include "starting_roles.h"
//// Game Share
//#include "game_share/skills_build.h"
//#include "game_share/characs_build.h"
//// Georges
//#include "nel/georges/u_form_loader.h"
//#include "nel/georges/u_form.h"
//#include "nel/georges/u_form_elm.h"
//// Misc
//#include "nel/misc/progress_callback.h"
//#include "client_sheets/starting_role_sheet.h"
//#include "sheet_manager.h"
//
//
//
//// global instance for starting roles
//CStartingRoleSet StartingRoleSet;
//
//
//using namespace NLMISC;
//using namespace std;
//
//
//// ***************************************************************************
//CStartingRoleSet::CStartingRoleSet()
//{
//	memset(_StartingRole, 0, sizeof(CStartingRoleSheet	*)*JOBS::NbJobs);
//}
//
//// ***************************************************************************
//void CStartingRoleSet::init (NLMISC::IProgressCallback &progress)
//{
//	// Get list of sheet of interest
//	vector<CSheetId> sheetIDs;
//	vector<string> fileNames;
//	CSheetId::buildIdVector(sheetIDs, fileNames, "starting_role");
//	for(uint k = 0; k < sheetIDs.size(); ++k)
//	{
//		// Progress bar
//		progress.progress ((float)k/(float)sheetIDs.size());
//
//		//	get the sheet
//		CStartingRoleSheet	*sroleSheet= dynamic_cast<CStartingRoleSheet*>(SheetMngr.get(sheetIDs[k]));
//		if( sroleSheet )
//		{
//			uint32	role= JOBS::getAssociatedRole(sroleSheet->Job);
//			sint	jobDBId= JOBS::getJobDBIndex(sroleSheet->Job);
//			if (jobDBId >=0 && role < ROLES::NB_ROLES) // valid role and race
//			{
//				_StartingRole[sroleSheet->Job]= sroleSheet;
//			}
//		}
//		else
//		{
//			nlwarning("Could get correct sheet %s for starting roles", fileNames[k].c_str());
//		}
//	}
//}
//
//
//// ***************************************************************************
//const CStartingRoleSheet *CStartingRoleSet::getStartingRole(JOBS::TJob eJob) const
//{
//	if (eJob == JOBS::Unknown)
//		return NULL;
//	else
//		return _StartingRole[eJob];
//}
