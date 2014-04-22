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
#include "view_bitmap_faber_mp.h"
#include "interface_manager.h"
#include "../sheet_manager.h"
#include "nel/misc/xml_auto_ptr.h"

extern CSheetManager SheetMngr;

using namespace std;
using namespace NLMISC;
using namespace NL3D;

NLMISC_REGISTER_OBJECT(CViewBase, CViewBitmapFaberMp, std::string, "bitmap_faber_mp");

/*
* parse an xml node and initialize the base view mambers. Must call CViewBase::parse
* \param cur : pointer to the xml node to be parsed
* \param parentGroup : the parent group of the view
* \partam id : a refence to the string that will receive the view ID
* \return true if success
*/
bool CViewBitmapFaberMp::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	if ( !CViewBitmap::parse(cur,parentGroup) )
		return false;

	//parse the sheet id
	CXMLAutoPtr prop((const char*) xmlGetProp( cur, (xmlChar*)"sheet" ));
	if (!prop)
	{
		nlinfo("CViewBitmapFaberMp: no sheet property");
		return false;
	}
	_SheetId.readSInt64(prop,_Id+":sheet");

	prop= (char*) xmlGetProp( cur, (xmlChar*)"col_noitem" );
	if (prop)
		_ColorNoItem.readRGBA (prop, _Id+":col_noitem");
	else
		_ColorNoItem.readRGBA ("255 255 255 255", _Id+":col_noitem");

	prop = (char*) xmlGetProp( cur, (xmlChar*)"tx_noitem" );
	if (prop)
	{
		_TextureNoItemName = (const char *) prop;
		_TextureNoItemName = strlwr (_TextureNoItemName);
		_TextureNoItemId = -2;
	}

	//parse the quantity
	prop= (char*) xmlGetProp( cur, (xmlChar*)"quantity" );
	if (!prop)
	{
		nlinfo("CViewBitmapFaberMp: no quantity property");
		return false;
	}
	_Quantity.readSInt64(prop,_Id+":quantity");

	//parse the needed quantity
	prop= (char*) xmlGetProp( cur, (xmlChar*)"needed_quantity" );
	if (!prop)
	{
		nlinfo("CViewBitmapFaberMp: no needed_quantity property");
		return false;
	}
	_NeededQuantity.readSInt64(prop,_Id+":needed_quantity");


	///\todo nico remove that when icon are here
/*	_SheetText = new CViewText(_Id + string(":sheettext"),"const 9","const 255 255 255 255","const false","const 0");
	_QualityText = new CViewText(_Id + string(":qualitytext"),"const 9","const 255 255 255 255","const false","const 0");
	_QuantityText = new CViewText(_Id + string(":quantitytext"),"const 9","const 255 255 255 255","const false","const 0");


	((CViewBitmapFaberMp*)_SheetText)->_X.readSInt32("const 0"," ");
	((CViewBitmapFaberMp*)_SheetText)->_Y.readSInt32("const 0"," ");
	((CViewBitmapFaberMp*)_QualityText)->_X.readSInt32("const 0"," ");
	((CViewBitmapFaberMp*)_QualityText)->_Y.readSInt32("const 0"," ");
	((CViewBitmapFaberMp*)_QuantityText)->_X.readSInt32("const 0"," ");
	((CViewBitmapFaberMp*)_QuantityText)->_Y.readSInt32("const 0"," ");*/


	return true;
}

/*
* draw the view
*/
void CViewBitmapFaberMp::draw ()
{
	///\todo nico : draw icon
	/*xOffset+=_XReal;
	yOffset+=_YReal;
	_SheetText->setText(ucstring(toString(_SheetId.getSInt32())));
	_SheetText->draw(xOffset,yOffset+20);

	_QuantityText->setText(ucstring(toString(_Quantity.getSInt32())));
	_QuantityText->draw(xOffset,yOffset+10);

	_QualityText->setText(ucstring(toString(_Quality.getSInt32())));
	_QualityText->draw(xOffset,yOffset);
*/

	//get the item
	CViewRenderer &rVR = *CViewRenderer::getInstance();

	uint32 sheet = (uint32)_SheetId.getSInt64();
	CSheetId sheetId(sheet);
	CEntitySheet *pES = SheetMngr.get (sheetId);

	if ((pES != NULL) && (pES->type() == CEntitySheet::ITEM))
	{
		CItemSheet *pIS = (CItemSheet*)pES;

		if (pIS->getIconBack() != "")
		{
			if (_AccIconBackString != pIS->getIconBack())
			{
				_AccIconBackString = pIS->getIconBack();
				_AccIconBackId = rVR.getTextureIdFromName (_AccIconBackString);
			}
			rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal,
							_WReal, _HReal, (uint8)_Rot, _Flip,
							_AccIconBackId);
		}

		if (pIS->getIconMain() != "")
		{
			if (_AccIconMainString != pIS->getIconMain())
			{
				_AccIconMainString = pIS->getIconMain();
				_AccIconMainId = rVR.getTextureIdFromName (_AccIconMainString);
			}
			rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal,
							_WReal, _HReal, (uint8)_Rot, _Flip,
							_AccIconMainId);
		}

		if (pIS->getIconOver() != "")
		{
			if (_AccIconOverString != pIS->getIconOver())
			{
				_AccIconOverString = pIS->getIconOver();
				_AccIconOverId = rVR.getTextureIdFromName (_AccIconOverString);
			}
			rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal,
							_WReal, _HReal, (uint8)_Rot, _Flip,
							_AccIconOverId);
		}

/*		// draw wanted quantity
		{
		//draw the units
		uint units = _NeededQuantity.getSInt32() % 10;
		rVR.drawRotFlipBitmap (_RenderLayer, _XReal+xOffset+13, _YReal+yOffset,
				8, 8, (uint8)_Rot.getSInt32(), _Flip.getBool(),
				rVR.getFigurTextureId(units), _Color.getRGBA());

		//draw the tens
		uint tens = _NeededQuantity.getSInt32() / 10;
		sint32 tensId;
		if (tens > 9 || tens == 0)
			tensId = rVR.getFigurBlankTextureId();
		else
			tensId = rVR.getFigurTextureId(tens);
		rVR.drawRotFlipBitmap (_RenderLayer, _XReal+xOffset+8, _YReal+yOffset,
			8, 8, (uint8)_Rot.getSInt32(), _Flip.getBool(),
			tensId, _Color.getRGBA());
		}


		// draw current quantity
		if (_Quantity.getSInt32() > 1)
		{
			//draw the units
			uint units = _Quantity.getSInt32() % 10;
			rVR.drawRotFlipBitmap (_RenderLayer, _XReal+xOffset+_WReal-8, _YReal+yOffset,
					8, 8, (uint8)_Rot.getSInt32(), _Flip.getBool(),
					rVR.getFigurTextureId(units), _Color.getRGBA());

			//draw the tens
			uint tens = _Quantity.getSInt32() / 10;
			sint32 tensId;
			if (tens > 9 || tens == 0)
				tensId = rVR.getFigurBlankTextureId();
			else
				tensId = rVR.getFigurTextureId(tens);
			rVR.drawRotFlipBitmap (_RenderLayer, _XReal+xOffset+_WReal-13, _YReal+yOffset,
				8, 8, (uint8)_Rot.getSInt32(), _Flip.getBool(),
				tensId, _Color.getRGBA());
		}
		*/

		uint16 qty = (uint16) ( _NeededQuantity.getSInt32() - _Quantity.getSInt32() );
		// draw missing quantity
		if (qty > 1)
		{
			//draw the units
			uint units = qty % 10;
			rVR.drawRotFlipBitmap (_RenderLayer, _XReal+_WReal-8, _YReal,
					8, 8, (uint8)_Rot, _Flip,
					rVR.getFigurTextureId(units), _Color);

			//draw the tens
			uint tens = qty / 10;
			sint32 tensId;
			if (tens > 9 || tens == 0)
				tensId = rVR.getFigurBlankTextureId();
			else
				tensId = rVR.getFigurTextureId(tens);
			rVR.drawRotFlipBitmap (_RenderLayer, _XReal+_WReal-13, _YReal,
				8, 8, (uint8)_Rot, _Flip,
				tensId, _Color);
		}
	}
	else
	{
		if (_TextureNoItemId == -2)
			_TextureNoItemId = rVR.getTextureIdFromName (_TextureNoItemName);
		rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal,
						_WReal, _HReal, (uint8)_Rot, _Flip,
						_TextureNoItemId, _ColorNoItem.getRGBA());
	}
}
