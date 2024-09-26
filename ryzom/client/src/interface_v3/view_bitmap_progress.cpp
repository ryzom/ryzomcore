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

#include "dbview_bar.h"
#include "interface_manager.h"



using namespace NL3D;


CDBViewBar::CDBViewBar : CViewBitmap()
{
}

/**
* parse an xml node and initialize the base view mambers. Must call CViewBase::parse
* \param cur : pointer to the xml node to be parsed
* \param parentGroup : the parent group of the view
* \partam id : a refence to the string that will receive the view ID
* \return true if success
*/
bool CDBViewBar::parse(xmlNodePtr cur,CInterfaceGroup * parentGroup)
{
	if (!CViewBitmap::parse(cur,parentGroup))
	{
		string tmp = "cannot parse view:"+getId()+", parent:"+parentGroup->getId();
		nlinfo(tmp.c_str());
		return false;
	}


	//try to get the NEEDED specific props
	CXMLAutoPtr prop= (char*) xmlGetProp( cur, (xmlChar*)"range" );
	if (prop)
	{
		_Range.readSInt32(prop,_Id+":range");
	}
	else
	{
		string tmp = "cannot getprop:range, view:"+getId()+", parent:"+parentGroup->getId();
		nlinfo(tmp.c_str());
		return false;
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"rangemax" );
	if (prop)
	{
		_RangeMax.readSInt32(prop,_Id+":rangemax");
	}
	else
	{
		string tmp = "cannot getprop:rangemax, view:"+getId()+", parent:"+parentGroup->getId();
		nlinfo(tmp.c_str());
		return false;
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"vertical" );
	if (prop)
		_Vertical.readBool (prop, _Id+":vertical");
	else
		_Vertical.readBool ("false", _Id+":vertical");

	return true;
}

/**
* draw the view
*/
void CDBViewBar::draw ()
{
	float wBar = (float)_WReal, hBar = (float)_HReal;
	if (_Vertical.getBool())
	{
		if (_RangeMax.getSInt32())
			hBar = _HReal * ( (float)_Range.getSInt32() / (float)_RangeMax.getSInt32() );
		else
			hBar = 0.0f;
	}
	else
	{
		if (_RangeMax.getSInt32())
			wBar = _WReal * ( (float)_Range.getSInt32() / (float)_RangeMax.getSInt32() );
		else
			wBar = 0.0f;
	}

	// Backup scissor and create the new scissor to clip the bar correctly.
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();

	sint32 oldScissorX, oldScissorY, oldScissorW, oldScissorH;
	rVR.getClipWindow (oldScissorX, oldScissorY, oldScissorW, oldScissorH);

	sint32 scisX, scisY, scisWidth, scisHeight;
	scisX		= oldScissorX;
	scisY		= oldScissorY;
	scisWidth	= oldScissorW;
	scisHeight	= oldScissorH;

	//the previous scissor must be taken in account.
	sint32 xabs = _XReal;
	sint32 yabs = _YReal;

	if( xabs > scisX )
		scisX = xabs;
	if( yabs > scisY )
		scisY = yabs;

	scisWidth = std::min(scisWidth + oldScissorX , (sint32)(xabs + wBar)) - scisX;
	scisHeight = std::min(scisHeight + oldScissorY , (sint32)(yabs + hBar)) - scisY;

	rVR.setClipWindow (scisX, scisY, scisWidth, scisHeight);

	// display progress bitmap
	//if (_TextureId == -2)
//		_TextureId = rVR.getTextureIdFromName (_TextureName);
	rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal,
							_WReal, _HReal, (uint8)_Rot, _Flip,
							_TextureId, _Color );

	// restore old scissor
	rVR.setClipWindow (oldScissorX, oldScissorY, oldScissorW, oldScissorH);
}