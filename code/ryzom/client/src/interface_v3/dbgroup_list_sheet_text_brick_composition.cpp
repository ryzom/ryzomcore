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
#include "dbgroup_list_sheet_text_brick_composition.h"
#include "../client_sheets/sbrick_sheet.h"
#include "nel/misc/xml_auto_ptr.h"
#include "../string_manager_client.h"
#include "sbrick_manager.h"

using namespace std;
using namespace NLMISC;


NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetTextBrickComposition, std::string, "list_sheet_compo_brick");

// ***************************************************************************
CDBGroupListSheetTextBrickComposition::CDBGroupListSheetTextBrickComposition(const TCtorParam &param)
:CDBGroupListSheetText(param)
{
	_XCost= 0;
	_YCost= 0;
	_BrickParameterDeltaX= 0;
}


// ***************************************************************************
bool CDBGroupListSheetTextBrickComposition::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if(!CDBGroupListSheetText::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr prop;

	// Parse XCost/YCost
	prop = (char*) xmlGetProp( cur, (xmlChar*)"xcost" );
	if(prop)	fromString((const char*)prop, _XCost);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"ycost" );
	if(prop)	fromString((const char*)prop, _YCost);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"param_deltax" );
	if(prop)	fromString((const char*)prop, _BrickParameterDeltaX);


	return true;
}


// ***************************************************************************
CDBGroupListSheetTextBrickComposition::CSheetChildBrick::CSheetChildBrick()
{
	CostView= NULL;
}

// ***************************************************************************
CDBGroupListSheetTextBrickComposition::CSheetChildBrick::~CSheetChildBrick()
{
	CostView= NULL;
}


// ***************************************************************************
void CDBGroupListSheetTextBrickComposition::CSheetChildBrick::init(CDBGroupListSheetText *pFather, uint index)
{
	// init my parent
	CSheetChild::init(pFather, index);

	// **** Init the CostView
	CDBGroupListSheetTextBrickComposition	*compoList= (CDBGroupListSheetTextBrickComposition*)pFather;
	// get the scrolled list.
	CInterfaceGroup		*parentList= compoList->getList();

	// create the cost Text View
	CViewText		*text= new CViewText(TCtorParam());
	text->setId(parentList->getId()+":cost_text"+toString(index));
	text->setParent (parentList);
	text->setParentPos (parentList);
	text->setParentPosRef (Hotspot_TL);
	text->setPosRef (Hotspot_TL);
	text->setActive(true);
	// set text aspect
	text->setFontSize(compoList->getTextTemplate().getFontSize());
	text->setColor(compoList->getTextTemplate().getColor());
	text->setShadow(compoList->getTextTemplate().getShadow());
	text->setShadowOutline(compoList->getTextTemplate().getShadowOutline());
	text->setMultiLine(false);
	text->setModulateGlobalColor(compoList->getTextTemplate().getModulateGlobalColor());

	// Add it to the scrolled list.
	CostView= text;
	parentList->addView(CostView);
}


// ***************************************************************************
bool	hasOnlyBlankChars(const ucstring &str)
{
	for(uint i=0;i!=str.size();++i)
	{
		if(str[i]!=' ')
			return false;
	}

	return true;
}


// ***************************************************************************
void CDBGroupListSheetTextBrickComposition::CSheetChildBrick::updateViewText(CDBGroupListSheetText *pFather)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CDBGroupListSheetTextBrickComposition	*compoList= (CDBGroupListSheetTextBrickComposition*)pFather;

	ucstring	text;
	if(Ctrl->getType()!=CCtrlSheetInfo::SheetType_SBrick)
		return;

	// Get the compo description of the phrase (Desc2)
	CSheetId	brickSheetId= CSheetId(Ctrl->getSheetId());
	// Temp if the Desc2 is empty, set Name
	ucstring desc2(STRING_MANAGER::CStringManagerClient::getSBrickLocalizedCompositionDescription(brickSheetId));
	if( !desc2.empty() && !hasOnlyBlankChars(desc2))	// tolerate Blank error in translation
		Text->setText(desc2);
	else
	{
		desc2 = STRING_MANAGER::CStringManagerClient::getSBrickLocalizedName(brickSheetId);
		Text->setText(desc2);
	}


	// Update the Cost View.
	if(CostView)
	{
		// show and place
		CostView->setActive(true);
		CostView->setX( compoList->getXCost() );
		CostView->setY( compoList->getYCost() - YItem*compoList->getHSlot() );

		// set the cost
		const CSBrickSheet	*brick= Ctrl->asSBrickSheet();
		if(brick)
		{
			// Special Case for the "Remove Brick" brick. No Cost (not revelant)
			if( brick->Id==pBM->getInterfaceRemoveBrick() )
				CostView->setText( ucstring() );
			else if( brick->SabrinaCost == 0 && brick->SabrinaRelativeCost != 0.f )
				CostView->setText( toString("%+d", (sint32)(brick->SabrinaRelativeCost * 100.f) ) + string("%") );
			else
				CostView->setText( toString("%+d", brick->SabrinaCost) );
		}
	}
}

// ***************************************************************************
void CDBGroupListSheetTextBrickComposition::CSheetChildBrick::hide(CDBGroupListSheetText *pFather)
{
	CSheetChild::hide(pFather);

	// hide cost view
	if(CostView)
		CostView->setActive(false);
}

// ***************************************************************************
bool CDBGroupListSheetTextBrickComposition::CSheetChildBrick::isInvalidated(CDBGroupListSheetText * /* pFather */)
{
	return false;
}

// ***************************************************************************
void CDBGroupListSheetTextBrickComposition::CSheetChildBrick::update(CDBGroupListSheetText *pFather)
{
	CSheetChild::update(pFather);
}

// ***************************************************************************
sint CDBGroupListSheetTextBrickComposition::CSheetChildBrick::getDeltaX(CDBGroupListSheetText *pFather) const
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CDBGroupListSheetTextBrickComposition	*compoList= (CDBGroupListSheetTextBrickComposition*)pFather;

	if(Ctrl->getType()!=CCtrlSheetInfo::SheetType_SBrick)
		return 0;

	CSBrickSheet	*brick= pBM->getBrick( CSheetId(Ctrl->getSheetId()) );

	if(!brick || !brick->isParameter())
		return 0;
	else
		return compoList->_BrickParameterDeltaX;
}

