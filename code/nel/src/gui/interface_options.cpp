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
#include "nel/gui/interface_options.h"

#include "nel/gui/interface_element.h"
#include "nel/gui/view_renderer.h"
#include "nel/misc/factory.h"
#include <string>

using namespace std;
using namespace NLMISC;

namespace NLGUI
{

	const CInterfaceOptionValue	CInterfaceOptionValue::NullValue;

	// ***************************************************************************
	void		CInterfaceOptionValue::init(const std::string &str)
	{
		_Str= str;
		fromString(str, _Int);
		fromString(str, _Float);
		_Color= CInterfaceElement::convertColor (str.c_str());
		_Boolean= CInterfaceElement::convertBool(str.c_str());
	}


	// ----------------------------------------------------------------------------
	// CInterfaceOptions
	// ----------------------------------------------------------------------------

	// ----------------------------------------------------------------------------
	CInterfaceOptions::CInterfaceOptions( const TCtorParam &/* param */ )
	{
	}

	// ----------------------------------------------------------------------------
	CInterfaceOptions::~CInterfaceOptions()
	{
	}

	// ----------------------------------------------------------------------------
	bool CInterfaceOptions::parse (xmlNodePtr cur)
	{
		cur = cur->children;
		bool ok = true;
		while (cur)
		{
			if ( !stricmp((char*)cur->name,"param") )
			{
				CXMLAutoPtr ptr, val;
				ptr = xmlGetProp (cur, (xmlChar*)"name");
				val = xmlGetProp (cur, (xmlChar*)"value");
				if (!ptr || !val)
				{
					nlinfo("param with no name or no value");
					ok = false;
				}
				else
				{
					string name = NLMISC::toLower(string((const char*)ptr));
					string value = (string((const char*)val));
					_ParamValue[name].init(value);
				}
			}
			cur = cur->next;
		}
		return ok;
	}

	xmlNodePtr CInterfaceOptions::serialize( xmlNodePtr parentNode, const std::string &name ) const
	{
		if( parentNode == NULL )
			return NULL;

		if( name.empty() )
			return NULL;

		xmlNodePtr node = xmlNewNode( NULL, BAD_CAST "options" );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "name", BAD_CAST name.c_str() );
		xmlAddChild( parentNode, node );

		std::map< std::string, CInterfaceOptionValue >::const_iterator itr;
		for( itr = _ParamValue.begin(); itr != _ParamValue.end(); ++itr )
		{
			xmlNodePtr n = xmlNewNode( NULL, BAD_CAST "param" );
			if( n == NULL )
			{
				xmlFreeNode( node );
				return NULL;
			}
			
			xmlSetProp( n, BAD_CAST "name", BAD_CAST itr->first.c_str() );
			xmlSetProp( n, BAD_CAST "value", BAD_CAST itr->second.getValStr().c_str() );
			xmlAddChild( node, n );
		}

		return node;
	}

	// ***************************************************************************
	void	CInterfaceOptions::copyBasicMap(const CInterfaceOptions &other)
	{
		_ParamValue= other._ParamValue;
	}

	// ***************************************************************************
	const CInterfaceOptionValue	 &CInterfaceOptions::getValue(const string &sParamName) const
	{
		string sLwrParamName = strlwr (sParamName);
		std::map<std::string, CInterfaceOptionValue>::const_iterator it = _ParamValue.find (sLwrParamName);
		if (it != _ParamValue.end())
			return it->second;
		else
			return CInterfaceOptionValue::NullValue;
	}

	// ***************************************************************************
	const std::string		&CInterfaceOptions::getValStr(const std::string &sParamName) const
	{
		return getValue(sParamName).getValStr();
	}
	// ***************************************************************************
	sint32					CInterfaceOptions::getValSInt32(const std::string &sParamName) const
	{
		return getValue(sParamName).getValSInt32();
	}
	// ***************************************************************************
	float					CInterfaceOptions::getValFloat(const std::string &sParamName) const
	{
		return getValue(sParamName).getValFloat();
	}
	// ***************************************************************************
	NLMISC::CRGBA			CInterfaceOptions::getValColor(const std::string &sParamName) const
	{
		return getValue(sParamName).getValColor();
	}
	// ***************************************************************************
	bool					CInterfaceOptions::getValBool(const std::string &sParamName) const
	{
		return getValue(sParamName).getValBool();
	}



	// ----------------------------------------------------------------------------
	// CInterfaceLayer
	// ----------------------------------------------------------------------------

	// ----------------------------------------------------------------------------
	NLMISC_REGISTER_OBJECT(CInterfaceOptions, COptionsLayer, std::string, "layer");
	COptionsLayer::COptionsLayer( const TCtorParam &param ) :
	CInterfaceOptions( param )
	{
		TxId_TL = TxId_T = TxId_TR = TxId_L = TxId_R = TxId_Blank = TxId_BL = TxId_B = -2;
		TxId_BR = TxId_BL_Open = TxId_B_Open = TxId_BR_Open = TxId_EL_Open = TxId_EM_Open = TxId_ER_Open =-2;
		Tile_Blank = 0;
		Tile_M_Header = Tile_M_Scrollbar = 0;
		Tile_T = Tile_B = Tile_L = Tile_R = 0;
		Tile_B_Open = Tile_EM_Open = Tile_M_Open = 0;
		Scrollbar_Offset_X = 4;
		Scrollbar_W = 8;
	}

	// ----------------------------------------------------------------------------
	COptionsLayer::~COptionsLayer()
	{
	}

	xmlNodePtr COptionsLayer::serialize( xmlNodePtr parentNode, const std::string &name ) const
	{
		xmlNodePtr node = CInterfaceOptions::serialize( parentNode, name );
		if( node == NULL )
			return NULL;
		
		xmlSetProp( node, BAD_CAST "type", BAD_CAST "layer" );

		return node;
	}

	// ----------------------------------------------------------------------------
	bool COptionsLayer::parse (xmlNodePtr cur)
	{
		if (!CInterfaceOptions::parse (cur))
			return false;

		CViewRenderer &rVR = *CViewRenderer::getInstance();

		Tile_Blank = getValSInt32("tile_blank");
		Tile_M_Header = getValSInt32("tile_m_header");
		Tile_M_Scrollbar = getValSInt32("tile_m_scrollbar");
		Tile_T = getValSInt32("tile_t");
		Tile_B = getValSInt32("tile_b");
		Tile_L = getValSInt32("tile_l");
		Tile_R = getValSInt32("tile_r");
		Tile_B_Open = getValSInt32("tile_b_open");
		Tile_EM_Open = getValSInt32("tile_em_open");
		Tile_M_Open = getValSInt32("tile_m_open");

		Scrollbar_Offset_X = getValSInt32("scrollbar_offset_x");
		Scrollbar_W = getValSInt32("scrollbar_size_w");
		TxId_B_Scrollbar = rVR.getTextureIdFromName (getValStr("scrollbar_tx_b"));
		rVR.getTextureSizeFromId(TxId_B_Scrollbar, W_B_Scrollbar, H_B_Scrollbar);
		TxId_M_Scrollbar = rVR.getTextureIdFromName (getValStr("scrollbar_tx_m"));
		rVR.getTextureSizeFromId(TxId_M_Scrollbar, W_M_Scrollbar, H_M_Scrollbar);
		TxId_T_Scrollbar = rVR.getTextureIdFromName (getValStr("scrollbar_tx_t"));
		rVR.getTextureSizeFromId(TxId_T_Scrollbar, W_T_Scrollbar, H_T_Scrollbar);

		TxId_L_Header	= rVR.getTextureIdFromName (getValStr("tx_l_header"));
		rVR.getTextureSizeFromId(TxId_L_Header, W_L_Header, H_L_Header);
		TxId_M_Header	= rVR.getTextureIdFromName (getValStr("tx_m_header"));
		rVR.getTextureSizeFromId(TxId_M_Header, W_M_Header, H_M_Header);
		TxId_R_Header	= rVR.getTextureIdFromName (getValStr("tx_r_header"));
		rVR.getTextureSizeFromId(TxId_R_Header, W_R_Header, H_R_Header);

		TxId_TL			= rVR.getTextureIdFromName (getValStr("tx_tl"));
		rVR.getTextureSizeFromId(TxId_TL, W_TL, H_TL);
		TxId_T			= rVR.getTextureIdFromName (getValStr("tx_t"));
		rVR.getTextureSizeFromId(TxId_T, W_T, H_T);
		TxId_TR			= rVR.getTextureIdFromName (getValStr("tx_tr"));
		rVR.getTextureSizeFromId(TxId_TR, W_TR, H_TR);
		TxId_L			= rVR.getTextureIdFromName (getValStr("tx_l"));
		rVR.getTextureSizeFromId(TxId_L, W_L, H_L);
		TxId_R			= rVR.getTextureIdFromName (getValStr("tx_r"));
		rVR.getTextureSizeFromId(TxId_R, W_R, H_R);
		TxId_Blank		= rVR.getTextureIdFromName (getValStr("tx_blank"));
		rVR.getTextureSizeFromId(TxId_Blank, W_Blank, H_Blank);
		TxId_BL			= rVR.getTextureIdFromName (getValStr("tx_bl"));
		rVR.getTextureSizeFromId(TxId_BL, W_BL, H_BL);
		TxId_B			= rVR.getTextureIdFromName (getValStr("tx_b"));
		rVR.getTextureSizeFromId(TxId_B, W_B, H_B);
		TxId_BR			= rVR.getTextureIdFromName (getValStr("tx_br"));
		rVR.getTextureSizeFromId(TxId_BR, W_BR, H_BR);
		//
		TxId_BL_Open	= rVR.getTextureIdFromName (getValStr("tx_bl_open"));
		rVR.getTextureSizeFromId(TxId_BL_Open, W_BL_Open, H_BL_Open);
		TxId_B_Open		= rVR.getTextureIdFromName (getValStr("tx_b_open"));
		rVR.getTextureSizeFromId(TxId_B_Open, W_B_Open, H_B_Open);
		TxId_BR_Open	= rVR.getTextureIdFromName (getValStr("tx_br_open"));
		rVR.getTextureSizeFromId(TxId_BR_Open, W_BR_Open, H_BR_Open);
		TxId_EL_Open	= rVR.getTextureIdFromName (getValStr("tx_el_open"));
		rVR.getTextureSizeFromId(TxId_EL_Open, W_EL_Open, H_EL_Open);
		TxId_EM_Open	= rVR.getTextureIdFromName (getValStr("tx_em_open"));
		rVR.getTextureSizeFromId(TxId_EM_Open, W_EM_Open, H_EM_Open);
		TxId_ER_Open	= rVR.getTextureIdFromName (getValStr("tx_er_open"));
		rVR.getTextureSizeFromId(TxId_ER_Open, W_ER_Open, H_ER_Open);
		TxId_M_Open		= rVR.getTextureIdFromName (getValStr("tx_m_open"));
		rVR.getTextureSizeFromId(TxId_M_Open, W_M_Open, H_M_Open);
		TxId_E_Open		= rVR.getTextureIdFromName (getValStr("tx_e_open"));
		rVR.getTextureSizeFromId(TxId_E_Open, W_E_Open, H_E_Open);
		//

		TxId_TL_HighLight = rVR.getTextureIdFromName (getValStr("tx_tl_highlight"));
		rVR.getTextureSizeFromId(TxId_TL_HighLight, W_TL_HighLight, H_TL_HighLight);
		TxId_T_HighLight  = rVR.getTextureIdFromName (getValStr("tx_t_highlight"));
		rVR.getTextureSizeFromId(TxId_T_HighLight, W_T_HighLight, H_T_HighLight);
		TxId_TR_HighLight = rVR.getTextureIdFromName (getValStr("tx_tr_highlight"));
		rVR.getTextureSizeFromId(TxId_TR_HighLight, W_TR_HighLight, H_TR_HighLight);
		TxId_L_HighLight  = rVR.getTextureIdFromName (getValStr("tx_l_highlight"));
		rVR.getTextureSizeFromId(TxId_L_HighLight, W_L_HighLight, H_L_HighLight);
		TxId_R_HighLight  = rVR.getTextureIdFromName (getValStr("tx_r_highlight"));
		rVR.getTextureSizeFromId(TxId_R_HighLight, W_R_HighLight, H_R_HighLight);
		TxId_BL_HighLight = rVR.getTextureIdFromName (getValStr("tx_bl_highlight"));
		rVR.getTextureSizeFromId(TxId_BL_HighLight, W_BL_HighLight, H_BL_HighLight);
		TxId_B_HighLight  = rVR.getTextureIdFromName (getValStr("tx_b_highlight"));
		rVR.getTextureSizeFromId(TxId_B_HighLight, W_B_HighLight, H_B_HighLight);
		TxId_BR_HighLight = rVR.getTextureIdFromName (getValStr("tx_br_highlight"));
		rVR.getTextureSizeFromId(TxId_BR_HighLight, W_BR_HighLight, H_BR_HighLight);

		//
		HeaderH = getValSInt32("header_h");
		InsetT = getValSInt32("inset_t");

		return true;
	}

	// ----------------------------------------------------------------------------
	NLMISC_REGISTER_OBJECT(CInterfaceOptions, COptionsContainerInsertion, std::string, "container_insertion_opt");
	COptionsContainerInsertion::COptionsContainerInsertion( const TCtorParam &param ) :
	CInterfaceOptions( param )
	{
		TxId_R_Arrow = -2;
		TxId_L_Arrow = -2;
		TxId_T_Arrow = -2;
		TxId_B_Arrow = -2;
		TxId_InsertionBar = -2;
	}

	xmlNodePtr COptionsContainerInsertion::serialize( xmlNodePtr parentNode, const std::string &name ) const
	{
		xmlNodePtr node = CInterfaceOptions::serialize( parentNode, name );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "container_insertion_opt" );

		return node;
	}

	// ----------------------------------------------------------------------------
	bool COptionsContainerInsertion::parse(xmlNodePtr cur)
	{
		if (!CInterfaceOptions::parse (cur))
			return false;

		CViewRenderer &rVR = *CViewRenderer::getInstance();
		TxId_T_Arrow =  rVR.getTextureIdFromName (getValStr("arrow_top"));
		TxId_B_Arrow =  rVR.getTextureIdFromName (getValStr("arrow_down"));
		TxId_L_Arrow =  rVR.getTextureIdFromName (getValStr("arrow_left"));
		TxId_R_Arrow =  rVR.getTextureIdFromName (getValStr("arrow_right"));
		TxId_InsertionBar =  rVR.getTextureIdFromName (getValStr("insertion_bar"));

		return true;
	}


	// ***************************************************************************
	NLMISC_REGISTER_OBJECT(CInterfaceOptions, COptionsContainerMove, std::string, "container_move_opt");
	COptionsContainerMove::COptionsContainerMove( const TCtorParam &param ) :
	CInterfaceOptions( param )
	{
		TrackW = -8;
		TrackH = 22;
		TrackY = -4;
		TrackYWithTopResizer = -8;
		TrackHWithTopResizer = 18;
		ResizerSize = 8;
	}

	xmlNodePtr COptionsContainerMove::serialize( xmlNodePtr parentNode, const std::string &name ) const
	{
		xmlNodePtr node = CInterfaceOptions::serialize( parentNode, name );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "container_move_opt" );

		return node;
	}

	// ***************************************************************************
	bool COptionsContainerMove::parse(xmlNodePtr cur)
	{
		if (!CInterfaceOptions::parse (cur))
			return false;
		fromString(getValStr("track_w"), TrackW);
		fromString(getValStr("track_h"), TrackH);
		fromString(getValStr("track_y"), TrackY);
		fromString(getValStr("track_y_with_top_resizer"), TrackYWithTopResizer);
		fromString(getValStr("track_h_with_top_resizer"), TrackHWithTopResizer);
		fromString(getValStr("resizer_size"), ResizerSize);
		return true;
	}

	// ***************************************************************************
	NLMISC_REGISTER_OBJECT(CInterfaceOptions, COptionsList, std::string, "list");
	COptionsList::COptionsList( const TCtorParam &param ) :
	CInterfaceOptions( param )
	{
		_NumParams= 0;
	}


	xmlNodePtr COptionsList::serialize( xmlNodePtr parentNode, const std::string &name ) const
	{
		if( parentNode == NULL )
			return NULL;

		if( name.empty() )
			return NULL;

		xmlNodePtr node = xmlNewNode( NULL, BAD_CAST "options" );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "name", BAD_CAST name.c_str() );
		xmlSetProp( node, BAD_CAST "type", BAD_CAST "list" );
		xmlAddChild( parentNode, node );

		std::map< std::string, CInterfaceOptionValue >::const_iterator itr;
		for( itr = _ParamValue.begin(); itr != _ParamValue.end(); ++itr )
		{
			xmlNodePtr n = xmlNewNode( NULL, BAD_CAST "param" );
			if( n == NULL )
			{
				xmlFreeNode( node );
				return NULL;
			}
			xmlSetProp( n, BAD_CAST "value", BAD_CAST itr->second.getValStr().c_str() );
			xmlAddChild( node, n );
		}

		return node;
	}

	// ***************************************************************************
	bool COptionsList::parse (xmlNodePtr cur)
	{
		cur = cur->children;
		bool ok = true;
		uint	id= 0;
		while (cur)
		{
			if ( !stricmp((char*)cur->name,"param") )
			{
				CXMLAutoPtr ptr, val;
				val = xmlGetProp (cur, (xmlChar*)"value");
				if (!val)
				{
					nlinfo("param with no name or no value");
					ok = false;
				}
				else
				{
					string value = (string((const char*)val));
					_ParamValue[toString(id)].init(value);
					id++;
				}
			}
			cur = cur->next;
		}

		_NumParams= id;

		return ok;
	}


	// ***************************************************************************
	const CInterfaceOptionValue		&COptionsList::getValue(uint paramId) const
	{
		return CInterfaceOptions::getValue(toString(paramId));
	}


}
