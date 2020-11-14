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
#include "nel/gui/view_bitmap_combo.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/db_manager.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_pointer_base.h"
#include "nel/gui/interface_group.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	//=======================================================================================
	bool CComboBoxDesc::parse(xmlNodePtr cur, CInterfaceElement*owner)
	{
		nlassert(owner);
		const std::string &ownerId = owner->getId();
		CXMLAutoPtr prop;
		//
		prop = xmlGetProp(cur, (const xmlChar *) "selected");
		if (!prop)
		{
			nlwarning((ownerId + " : Couldn't read 'selected' field").c_str());
			return false;
		}
		owner->relativeSInt64Read(CurrSelected, "selected", prop, "0");
		//
		prop = xmlGetProp(cur, (const xmlChar *) "num_row");
		owner->relativeSInt64Read(NumRow, "num_row", prop, "1");
		//
		prop = xmlGetProp(cur, (const xmlChar *) "num_col");
		owner->relativeSInt64Read(NumCol, "num_col", prop, "1");
		//
		prop = xmlGetProp(cur, (const xmlChar *) "itemw");
		owner->relativeSInt64Read(ItemWidth, "itemw", prop, "32");
		//
		prop = xmlGetProp(cur, (const xmlChar *) "itemh");
		owner->relativeSInt64Read(ItemHeight, "itemh", prop, "32");
		//
		prop = xmlGetProp(cur, (const xmlChar *) "unrolled");
		owner->relativeBoolRead(Unrolled, "unrolled", prop, "false");
		//
		prop = xmlGetProp(cur, (xmlChar *) "wgap");
		owner->relativeSInt64Read(WGap, "wgap", prop, "0");
		//
		prop = xmlGetProp(cur, (xmlChar *) "hgap");
		owner->relativeSInt64Read(HGap, "hgap", prop, "0");
		//
		prop = xmlGetProp(cur, (xmlChar *) "wgap_selected");
		owner->relativeSInt64Read(WGapSelected, "wgap_selected", prop, "0");
		//
		prop = xmlGetProp(cur, (xmlChar *) "hgap_selected");
		owner->relativeSInt64Read(HGapSelected, "hgap_selected", prop, "0");
		//
		//
		prop = xmlGetProp(cur, (xmlChar *) "num_sel");
		owner->relativeSInt64Read(NumSel, "num_sel", prop, "1");
		//
		prop = (char*) xmlGetProp (cur, (xmlChar*)"align");
		Align.readSInt32 ("0", ownerId + ":align");
		if (prop)
		{
			const char *seekPtr = prop.getDatas();
			while (*seekPtr != 0)
			{
				if ((*seekPtr=='l')||(*seekPtr=='L'))
				{
					Align.setSInt32 (Align.getSInt32()&(~1));
				}
				if ((*seekPtr=='r')||(*seekPtr=='R'))
				{
					Align.setSInt32 (Align.getSInt32()|1);
				}
				if ((*seekPtr=='b')||(*seekPtr=='B'))
				{
					Align.setSInt32 (Align.getSInt32()&(~2));
				}
				if ((*seekPtr=='t')||(*seekPtr=='T'))
				{
					Align.setSInt32 (Align.getSInt32()|2);
				}
				++seekPtr;
			}
		}
		//
		return true;
	}


	//=========================================================================================
	void CComboBoxDesc::addObserver(ICDBNode::IPropertyObserver *obs)
	{
		// Add observers on dimensions
		if (NumRow.getNodePtr())
		{
			ICDBNode::CTextId textId;
			NumRow.getNodePtr()->addObserver(obs, textId);
		}
		if (NumCol.getNodePtr())
		{
			ICDBNode::CTextId textId;
			NumCol.getNodePtr()->addObserver(obs, textId);
		}
		if (ItemWidth.getNodePtr())
		{
			ICDBNode::CTextId textId;
			ItemWidth.getNodePtr()->addObserver(obs, textId);
		}
		if (ItemHeight.getNodePtr())
		{
			ICDBNode::CTextId textId;
			ItemHeight.getNodePtr()->addObserver(obs, textId);
		}
		if (Unrolled.getNodePtr())
		{
			ICDBNode::CTextId textId;
			Unrolled.getNodePtr()->addObserver(obs, textId);
		}
	}

	//=======================================================================================
	void CComboBoxDesc::getGridSize(uint &numRow, uint &numCol) const
	{
		numRow = NumRow.getSInt32();
		numCol = NumCol.getSInt32();
		if (numRow == 0 || numCol == 0) return;
		if (!Unrolled.getBool())
		{
			numRow = numCol = 1;
		}
	}

	//=======================================================================================
	void CComboBoxDesc::getDimensions(uint &w, uint &h) const
	{
		uint numRow, numCol;
		getGridSize(numRow, numCol);
		w = numCol * (ItemWidth.getSInt32() + WGap.getSInt32());
		h = numRow * (ItemHeight.getSInt32() + HGap.getSInt32());
		//
		if (numCol == 1) w -= WGap.getSInt32();
		else w += WGapSelected.getSInt32() - WGap.getSInt32();
		//
		if (numRow == 1) h -= HGap.getSInt32();
		else h += HGapSelected.getSInt32() - HGap.getSInt32();
	}

	//=======================================================================================
	NLMISC_REGISTER_OBJECT(CViewBase, CViewBitmapCombo, std::string, "bitmap_combo");

	CViewBitmapCombo::CViewBitmapCombo(const TCtorParam &param) : CViewBase(param)
	{
	}

	std::string CViewBitmapCombo::getProperty( const std::string &name ) const
	{
		if( name == "tx_normal" )
		{
			std::string normal;
			getTexList( _Texs, normal );
			return normal;
		}
		else
		if( name == "tx_over" )
		{
			std::string over;
			getTexList( _TexsOver, over );
			return over;
		}
		else
		if( name == "tx_pushed" )
		{
			std::string pushed;
			getTexList( _TexsPushed, pushed );
			return pushed;
		}
		else
		if( name == "col_normal" )
		{
			std::string normal;
			getColList( _Col, normal );
			return normal;
		}
		else
		if( name == "col_over" )
		{
			std::string over;
			getColList( _ColOver, over );
			return over;
		}
		else
		if( name == "col_pushed" )
		{
			std::string pushed;
			getColList( _ColPushed, pushed );
			return pushed;
		}
		else
			return CViewBase::getProperty( name );
	}

	void CViewBitmapCombo::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "tx_normal" )
		{
			parseTexList( value, _Texs );
			return;
		}
		else
		if( name == "tx_over" )
		{
			parseTexList( value, _TexsOver );
			return;
		}
		else
		if( name == "tx_pushed" )
		{
			parseTexList( value, _TexsPushed );
			return;
		}
		else
		if( name == "col_normal" )
		{
			parseColList( value, _Col );
			return;
		}
		else
		if( name == "col_over" )
		{
			parseColList( value, _ColOver );
			return;
		}
		else
		if( name == "col_pushed" )
		{
			parseColList( value, _ColPushed );
			return;
		}
		else
			CViewBase::setProperty( name, value );
	}

	xmlNodePtr CViewBitmapCombo::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CViewBase::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "bitmap_combo" );

		std::string normal;
		std::string over;
		std::string pushed;

		getTexList( _Texs, normal );
		getTexList( _TexsOver, over );
		getTexList( _TexsPushed, pushed );
		xmlSetProp( node, BAD_CAST "tx_normal", BAD_CAST normal.c_str() );
		xmlSetProp( node, BAD_CAST "tx_over", BAD_CAST over.c_str() );
		xmlSetProp( node, BAD_CAST "tx_pushed", BAD_CAST pushed.c_str() );

		getColList( _Col, normal );
		getColList( _ColOver, over );
		getColList( _ColPushed, pushed );
		xmlSetProp( node, BAD_CAST "col_normal", BAD_CAST normal.c_str() );
		xmlSetProp( node, BAD_CAST "col_over", BAD_CAST over.c_str() );
		xmlSetProp( node, BAD_CAST "col_pushed", BAD_CAST pushed.c_str() );


		return node;
	}

	//=======================================================================================
	bool CViewBitmapCombo::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		if (! CViewBase::parse(cur, parentGroup) )
		{
			parseError(parentGroup);
			return false;
		}

		CXMLAutoPtr prop;

		std::string texs;
		prop = xmlGetProp(cur, (xmlChar *)"tx_normal");
		if (prop) texs = (const char*)prop;
		std::string texsOver;
		prop = xmlGetProp(cur, (xmlChar *)"tx_over");
		if (prop) texsOver = (const char*)prop;
		std::string texsPushed;
		prop = xmlGetProp(cur, (xmlChar *)"tx_pushed");
		if (prop) texsPushed = (const char*)prop;
		//
		// for colors, an empty strings means all colors are white
		//
		std::string col;
		prop = xmlGetProp(cur, (xmlChar *)"col_normal");
		if (prop) col = (const char*)prop;
		std::string colOver;
		prop = xmlGetProp(cur, (xmlChar *)"col_over");
		if (prop) colOver = (const char*)prop;
		std::string colPushed;
		prop = xmlGetProp(cur, (xmlChar *)"col_pushed");
		if (prop) colPushed = (const char*)prop;

	/*	if (texs.empty() || texsOver.empty() || texsPushed.empty())
		{
			parseError(parentGroup, "Cannot read tx_normal, tx_over, or tx_pushed");
			return false;
		}*/

		parseTexList(texs, _Texs);
		parseTexList(texsOver, _TexsOver);
		parseTexList(texsPushed, _TexsPushed);

		parseColList(col, _Col);
		parseColList(colOver, _ColOver);
		parseColList(colPushed, _ColPushed);
		//
	/*	if (_Texs.size() != _TexsOver.size() || _TexsOver.size() != _TexsPushed.size())
		{
			parseError(parentGroup, "Texture names arrays do not have the same size");
			return false;
		}
		//
		uint numCols = NLMISC::maxof(_Col.size(), _ColOver.size(), _ColPushed.size());
		if (!
			(
				(_Col.empty() || _Col.size() == numCols)
				&& (_ColOver.empty() || _ColOver.size() == numCols)
				&& (_ColPushed.empty() || _ColPushed.size() == numCols)
			)
		   )
		{
			parseError(parentGroup, "Color names arrays do not have the same size (note an empty array is valid, means all color are 255 255 255");
			return false;
		}*/
		//
		if (!_CD.parse(cur, this))
		{
			return false;
		}
		//
		setupSize();
		//
		_CD.addObserver(this);

		return true;
	}

	//=======================================================================================
	NLMISC::CRGBA CViewBitmapCombo::getCol(const CViewBitmapCombo::TColorArray &array,uint index)
	{
		if (array.empty()) return CRGBA::White;
		return array[index % array.size()];
	}

	//=======================================================================================
	const std::string *CViewBitmapCombo::getTex(const TStringArray &array,uint index)
	{
		if (array.empty()) return NULL;
		return &array[index % array.size()];
	}

	//=======================================================================================
	sint32 CViewBitmapCombo::getTexId(const TIdArray &array, uint index)
	{
		if (array.empty()) return -1;
		return array[index % array.size()];
	}

	//=======================================================================================
	void CViewBitmapCombo::draw()
	{
		if (_Texs.empty()) return;
		uint numRow, numCol;
		_CD.getGridSize(numRow, numCol);
		if (numRow == 0 || numCol == 0) return;

		sint32 mx = 0, my = 0;
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		const std::vector<CViewBase *> &rVB = CWidgetManager::getInstance()->getViewsUnderPointer();
		if (!CWidgetManager::getInstance()->getPointer()) return;
		CWidgetManager::getInstance()->getPointer()->getPointerDispPos(mx, my);
		bool over = false;
		uint32 i;
		for (i = 0; i < rVB.size(); ++i)
		{
			if (rVB[i] == this)
			{
				over = true;
				break;
			}
		}

		if (_TexsId.empty())
		{
			for (i = 0; i < _Texs.size(); ++i)
				_TexsId.push_back(rVR.getTextureIdFromName(_Texs[i]));
			for (i = 0; i < _TexsOver.size(); ++i)
				_TexsOverId.push_back(rVR.getTextureIdFromName(_TexsOver[i]));
			for (i = 0; i < _TexsPushed.size(); ++i)
				_TexsPushedId.push_back(rVR.getTextureIdFromName(_TexsPushed[i]));
		}

		sint32 textId;
		CRGBA color;
		uint selectedTexIndex = _CD.CurrSelected.getSInt32();
		uint itemw = _CD.ItemWidth.getSInt32() + _CD.WGap.getSInt32();
		uint itemh = _CD.ItemHeight.getSInt32() + _CD.HGap.getSInt32();
		uint counter = 0;

		bool overItem = false;
		for(uint x = 0; x < numCol; ++x)
		{
			for(uint y = 0; y < numRow; ++y)
			{
				uint texIndex = counter;
				if (counter != 0)
				{
					if (counter == selectedTexIndex)
					{
						texIndex = 0;
					}
					sint px;
					sint py;
					// get the right position depending on alignment
					if (_CD.Align.getSInt32() & 1) // right align ?
					{
						px = _XReal + _WReal - (x + 1) * itemw;
					}
					else
					{
						px = _XReal + x * itemw;
					}
					// top align ?
					if (_CD.Align.getSInt32() & 2)
					{
						py = _YReal + _HReal - (y + 1) * itemh;
					}
					else
					{
						py = _YReal + y * itemh;
					}

					if (x != 0)
					{
						if (_CD.Align.getSInt32() & 1)
						px -= _CD.WGapSelected.getSInt32() - _CD.WGap.getSInt32();
						else px += _CD.WGapSelected.getSInt32() - _CD.WGap.getSInt32();
					}
					if (y != 0)
					{
						if (_CD.Align.getSInt32() & 2)
						py -= _CD.HGapSelected.getSInt32() - _CD.HGap.getSInt32();
						else py += _CD.HGapSelected.getSInt32() - _CD.HGap.getSInt32();
					}
					// is the mouse on current item ?
					if (over
						&& mx >= px
						&& my >= py
						&& mx <  px + (sint32) itemw
						&& my <  py + (sint32) itemh)
					{
						overItem = true;
						if ( CWidgetManager::getInstance()->getPointer()->getButtonState() & NLMISC::leftButton)
						{
							textId = getTexId(_TexsPushedId, texIndex);
							color  = getCol(_ColPushed, texIndex);
						}
						else
						{
							textId = getTexId(_TexsOverId, texIndex);
							color  = getCol(_ColOver, texIndex);
						}
					}
					else
					{
						textId = getTexId(_TexsId, texIndex);
						color  = getCol(_Col, texIndex);
					}
					CViewRenderer::getInstance()->drawRotFlipBitmap (_RenderLayer, px, py, itemw, itemh, 0, false,
															 textId,
															 color);
				}
				++counter;
				if ((sint32) counter == _CD.NumSel.getSInt32())
					break;
			}
		}

		if ((sint32) selectedTexIndex >= _CD.NumSel.getSInt32())
		{
			return;
		}

		// draw current selection
		sint32 px;
		sint32 py;
		//
		if (_CD.Align.getSInt32() & 1)
		{
			px = _XReal + _WReal - itemw;
		}
		else
		{
			px = _XReal;
		}
		//
		if (_CD.Align.getSInt32() & 2)
		{
			py = _YReal + _HReal - itemh;
		}
		else
		{
			py = _YReal;
		}
		//
		if (_CD.Unrolled.getBool())
		{
			if (overItem && CWidgetManager::getInstance()->getPointer()->getButtonState() & NLMISC::leftButton)
			{
				textId = getTexId(_TexsId, selectedTexIndex);
				color  = getCol(_Col, selectedTexIndex);
			}
			else
			{
				textId = getTexId(_TexsPushedId, selectedTexIndex);
				color  = getCol(_ColPushed, selectedTexIndex);
			}
		}
		else
		{
			if (over
				&& mx >= px
				&& my >= py
				&& mx <  px + (sint32) itemw
				&& my <  py + (sint32) itemh
			   )
			{
				if ( CWidgetManager::getInstance()->getPointer()->getButtonState() & NLMISC::leftButton)
				{
					textId = getTexId(_TexsPushedId, selectedTexIndex);
					color  = getCol(_ColPushed, selectedTexIndex);
				}
				else
				{
					textId = getTexId(_TexsOverId, selectedTexIndex);
					color  = getCol(_ColOver, selectedTexIndex);
				}
			}
			else
			{
				textId = getTexId(_TexsId, selectedTexIndex);
				color  = getCol(_Col, selectedTexIndex);
			}
		}

		CViewRenderer::getInstance()->drawRotFlipBitmap (_RenderLayer, px, py, itemw, itemh, 0, false,
												  textId,
												  color);
	}

	//=======================================================================================
	void CViewBitmapCombo::parseTexList(const std::string &names, TStringArray &dest)
	{
		static const char sep[] = " ,\t";
		std::string::size_type pos = 0, nextPos;
		dest.clear();
		do
		{
			nextPos = names.find_first_of(sep, pos);
			if (pos != nextPos)
			{
				dest.push_back(names.substr(pos, nextPos - pos));
			}
			pos = names.find_first_not_of(sep, nextPos);
		}
		while (pos != std::string::npos);
	}

	//=======================================================================================
	void CViewBitmapCombo::parseColList(const std::string &names,TColorArray &dest)
	{
		static const char sep[] = ",\t";
		std::string::size_type pos = 0, nextPos;
		dest.clear();
		std::string col;
		do
		{
			nextPos = names.find_first_of(sep, pos);
			if (pos != nextPos)
			{
				col = names.substr(pos, nextPos - pos);
				int r = 0, g = 0, b = 0, a = 255;
				sscanf (col.c_str(), "%d %d %d %d", &r, &g, &b, &a);
				NLMISC::clamp (r, 0, 255);
				NLMISC::clamp (g, 0, 255);
				NLMISC::clamp (b, 0, 255);
				NLMISC::clamp (a, 0, 255);
				dest.push_back(NLMISC::CRGBA((uint8) r, (uint8) g, (uint8) b, (uint8) a));
			}
			pos = names.find_first_not_of(sep, nextPos);
		}
		while (pos != std::string::npos);
	}

	void CViewBitmapCombo::getTexList( const TStringArray &arr, std::string &dest ) const
	{
		dest.clear();
		TStringArray::const_iterator itr;
		for( itr = arr.begin(); itr != arr.end(); ++itr )
		{
			dest += *itr;
			dest += " ";
		}
	}

	void CViewBitmapCombo::getColList( const TColorArray &arr, std::string &dest ) const
	{
		dest.clear();
		TColorArray::const_iterator itr;
		for( itr = arr.begin(); itr != arr.end(); ++itr )
		{
			dest += toString( *itr );
			dest += " ";
		}
	}

	//=======================================================================================
	void CViewBitmapCombo::setupSize()
	{
		uint w, h;
		_CD.getDimensions(w, h);
		setW(w);
		setH(h);
		invalidateCoords();
	}

	//=======================================================================================
	void CViewBitmapCombo::update(ICDBNode * /* leaf */)
	{
		setupSize();
	}


	//=======================================================================================
	/** copy an array of char * into an array of strings
	  */
	static void copyStrArrayFromChar(CViewBitmapCombo::TStringArray &dest, const char * const src[], uint numColor)
	{
		dest.resize(numColor);
		for(uint k = 0; k < numColor; ++k)
		{
			dest[k] = src[k];
		}
	}


	static void copyRGBAVectorFromRGBAArray(CViewBitmapCombo::TColorArray &dest, const NLMISC::CRGBA src[], uint numColor)
	{
		dest.resize(numColor);
		std::copy(src, src + numColor, dest.begin());
	}


	//=======================================================================================
	void CViewBitmapCombo::setTexs(const char * const tex[], uint numTex)
	{
		copyStrArrayFromChar(_Texs, tex, numTex);
	}

	//=======================================================================================
	void CViewBitmapCombo::setTexsOver(const char * const tex[], uint numTex)
	{
		copyStrArrayFromChar(_TexsOver, tex, numTex);
	}


	//=======================================================================================
	void CViewBitmapCombo::setTexsPushed(const char * const tex[], uint numTex)
	{
		copyStrArrayFromChar(_TexsPushed, tex, numTex);
	}

	//=======================================================================================
	void CViewBitmapCombo::setColors(const NLMISC::CRGBA colors[], uint numColors)
	{
		copyRGBAVectorFromRGBAArray(_Col, colors, numColors);
	}


	//=======================================================================================
	void CViewBitmapCombo::setColorsOver(const NLMISC::CRGBA colors[], uint numColors)
	{
		copyRGBAVectorFromRGBAArray(_ColOver, colors, numColors);
	}


	//=======================================================================================
	void CViewBitmapCombo::setColorsPushed(const NLMISC::CRGBA colors[], uint numColors)
	{
		copyRGBAVectorFromRGBAArray(_ColPushed, colors, numColors);
	}

}

