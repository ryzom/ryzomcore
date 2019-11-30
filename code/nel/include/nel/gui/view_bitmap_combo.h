// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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




#ifndef NL_VIEW_BITMAP_COMBO_H
#define NL_VIEW_BITMAP_COMBO_H

#include "nel/misc/cdb.h"
#include "nel/gui/view_base.h"
#include "nel/3d/u_texture.h"
#include <string>
#include <vector>

namespace NLGUI
{

	/** Description of a combo box, this can be parsed from an xml node
	  * The bitmap layout is as follow :
	  *
	  *                I   W    I   W   I   W
	  *                t   G    t   G   t   G
	  *                e   a    e   a   e   a
	  *                m   p    m   p   m   p
	  *                W   S    W       W
	  *                i   e    i       i
	  *                d   l    d       d
	  *                t   e    t       t
	  *                h   c    h       h
	  *                    t
	  *                    e
	  *                    d
	  *             +----+---+----+---+---+---+---+--...
	  *             |ssss|   |****|   |***|   |***|
	  * ItemHeight  |ssss|   |****|   |***|   |***
	  *             +----+---+----+---+---+---+---+--...
	  *             |    |   |    |   |   |   |   |
	  * HGapSeleted |    |   |    |   |   |   |   |
	  *             +----+---+----+---+---+---+---+--...
	  *             |****|   |****|   |***|   |***|
	  * ItemHeight  |****|   |****|   |***|   |***|
	  *             +----+---+----+---+---+---+---+--...
	  *             |    |   |    |   |   |   |   |
	  * HGap        |    |   |    |   |   |   |   |
	  *             +----+---+----+---+---+---+---+--...
	  *             |****|   |****|   |***|   |***|
	  * ItemHeight  |****|   |****|   |***|   |***|
	  *             .    .   .    .   .   .   .   .
	  *             .    .   .    .   .   .   .   .
	  * s : selected item.   .    .   .   .   .   .
	  * * : where bitmap are displayed
	  */


	struct CComboBoxDesc
	{
		bool	parse(xmlNodePtr cur, CInterfaceElement *owner);
		void	addObserver(NLMISC::ICDBNode::IPropertyObserver *obs);
		void	getGridSize(uint &numRow,uint &numCol) const;
		void    getDimensions(uint &width, uint &height) const;
		CInterfaceProperty      NumRow;
		CInterfaceProperty      NumCol;
		CInterfaceProperty      CurrSelected;
		CInterfaceProperty		ItemWidth;
		CInterfaceProperty		ItemHeight;
		CInterfaceProperty		Unrolled;
		CInterfaceProperty      WGapSelected;
		CInterfaceProperty      WGap;
		CInterfaceProperty      HGapSelected;
		CInterfaceProperty      HGap;
		CInterfaceProperty      NumSel;
		CInterfaceProperty      Align;

	};




	/**
	 * A combo box with several bitmaps in it
	 * \author Nicolas Vizerie
	 * \author Nevrax France
	 * \date 2002
	 */
	class CViewBitmapCombo : public CViewBase, public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
        DECLARE_UI_CLASS( CViewBitmapCombo )

		typedef std::vector<sint32> TIdArray;
		typedef std::vector<std::string> TStringArray;
		typedef std::vector<NLMISC::CRGBA>		 TColorArray;
	public:
		/// ctor
		CViewBitmapCombo(const TCtorParam &param);


		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;


		/**
		 * parse an xml node and initialize the base view members. Must call CViewBase::parse
		 * \param cur : pointer to the xml node to be parsed
		 * \param parentGroup : the parent group of the view
		 * \partam id : a refence to the string that will receive the view ID
		 * \return true if success
		 */
		bool parse(xmlNodePtr cur,CInterfaceGroup * parentGroup);
		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }
		/**
		 * draw the view
		 */
		void draw();

		// access to texture & colors
		const TStringArray &getTexs() const { return _Texs; }
		const TStringArray &getTexsOver() const { return _TexsOver; }
		const TStringArray &getTexsPushed() const { return _TexsPushed; }
		//
		const TColorArray &getColors() const { return _Col; }
		const TColorArray &getColorsOver() const { return _ColOver; }
		const TColorArray &getColorsPushed() const { return _ColPushed; }
		//
		void  setTexs(const char * const tex[], uint numTex);
		void  setTexsOver(const char * const tex[], uint numTex);
		void  setTexsPushed(const char * const tex[], uint numTex);
		//
		void  setColors(const NLMISC::CRGBA colors[], uint numColors);
		void  setColorsOver(const NLMISC::CRGBA colors[], uint numColors);
		void  setColorsPushed(const NLMISC::CRGBA colors[], uint numColors);







	///////////////////////////////////////////////////////////////////////////////////
	private:
		//
		TStringArray				_Texs;
		TStringArray				_TexsOver;
		TStringArray				_TexsPushed;
		TIdArray					_TexsId;
		TIdArray					_TexsOverId;
		TIdArray					_TexsPushedId;
		TColorArray				    _Col;
		TColorArray				    _ColOver;
		TColorArray				    _ColPushed;
		CComboBoxDesc				_CD;
		CInterfaceElement *_Owner;
	private:
		void	parseTexList(const std::string &names, TStringArray &dest);
		void	parseColList(const std::string &names, TColorArray &dest);
		void    getTexList( const TStringArray &arr, std::string &dest ) const;
		void    getColList( const TColorArray  &arr, std::string &dest ) const;
		void	setupSize();
		void	getDimensions(uint &numRow, uint &numCol);
		// From ICDBNode::IPropertyObserver
		void update(NLMISC::ICDBNode *leaf);
		// Return a color from the array, or white if it is empty
		static NLMISC::CRGBA getCol(const TColorArray &array, uint index);
		static const std::string   *getTex(const TStringArray &array, uint index);
		static sint32 getTexId(const TIdArray &array, uint index);
	};

}

#endif
