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
#include "dbgroup_list_sheet_bonus_malus.h"
#include "interface_manager.h"


using namespace std;
using namespace NLMISC;

// ***************************************************************************
#define DISABLE_LEAF "DISABLED"

NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetBonusMalus, std::string, "list_sheet_bonus_malus");

// ***************************************************************************
CDBGroupListSheetBonusMalus::CDBGroupListSheetBonusMalus(const TCtorParam &param)
:	CDBGroupListSheet(param)
{
	_TextId= -1;

	// want leave space between controls in the list
	// Yoyo: I think it's better like this, + this is important for space consideration and because of XPCat/PVPOutpost
	//_ListLeaveSpace= false;
}


// ***************************************************************************
bool CDBGroupListSheetBonusMalus::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if(!CDBGroupListSheet::parse(cur, parentGroup))
		return false;

	// read the texture
	CXMLAutoPtr	prop;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"disable_texture" );
	if (prop)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		_TextId= rVR.getTextureIdFromName ((const char *)prop);
	}

	// get the Node leaves to be tested each frame
	uint	i= 0;
	for(;;)
	{
		string	db= toString("%s:%d:" DISABLE_LEAF, _DbBranchName.c_str(), i);
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(db, false);
		if(!node)
		{
			break;
		}
		else
		{
			_DisableStates.push_back(node);
			i++;
		}
	}

	return true;
}

// ***************************************************************************
void CDBGroupListSheetBonusMalus::draw ()
{
	CDBGroupListSheet::draw();

//	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
//	CViewRenderer		&rVR= *CViewRenderer::getInstance();

//	sint32	drl= getRenderLayer()+1;

	// May draw disable bitmaps on the ctrl sheets if disabled.
	uint	numCtrls= (uint)min(_SheetChildren.size(), _DisableStates.size());
	for(uint i=0;i<numCtrls;i++)
	{
		CDBCtrlSheet	*ctrl= _SheetChildren[i]->Ctrl;
		// if the ctrl is displayed, and if the state is disabled
		if(ctrl->getActive())
		{
			if(_DisableStates[i]->getValue32()!=0)
			{
				ctrl->setGrayed(true);
				/*
				// YOYO: for now, don't display the gray bitmap. cross not cool.
				CRGBA	crossColor= ctrl->getSheetColor();
				crossColor.A>>= 2;
				// Draw the disable bitmap on this control. +1 for the slot (ugly)
				rVR.drawRotFlipBitmap(drl, ctrl->getXReal()+1, ctrl->getYReal()+1,
					CCtrlSheetInfo::BrickSheetWidth, CCtrlSheetInfo::BrickSheetHeight, 0, 0, _TextId, crossColor);
				*/
			}
			else
				ctrl->setGrayed(false);
		}
	}
}

