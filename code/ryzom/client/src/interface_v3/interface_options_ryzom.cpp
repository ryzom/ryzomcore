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


#include "interface_options_ryzom.h"

#include "stdpch.h"

#include "interface_options.h"
#include "interface_manager.h"
#include "group_menu.h"
#include "nel/misc/xml_auto_ptr.h"
#include "../net_manager.h"
#include "../sheet_manager.h"
#include "../entity_animation_manager.h"
#include "../client_sheets/animation_set_list_sheet.h"
#include "../client_sheets/emot_list_sheet.h"
#include "nel/3d/u_animation_set.h"
#include "nel/misc/algo.h"

// ----------------------------------------------------------------------------
using namespace std;
using namespace NL3D;
using namespace NLMISC;

extern CEntityAnimationManager *EAM;


// ----------------------------------------------------------------------------
// CInterfaceLayer
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
COptionsLayer::COptionsLayer()
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

// ----------------------------------------------------------------------------
bool COptionsLayer::parse (xmlNodePtr cur)
{
	if (!CInterfaceOptions::parse (cur))
		return false;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
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
	TxId_T_HighLight  = rVR.getTextureIdFromName (getValStr("tx_t_highlight"));
	TxId_TR_HighLight = rVR.getTextureIdFromName (getValStr("tx_tr_highlight"));
	TxId_L_HighLight  = rVR.getTextureIdFromName (getValStr("tx_l_highlight"));
	TxId_R_HighLight  = rVR.getTextureIdFromName (getValStr("tx_r_highlight"));
	TxId_BL_HighLight = rVR.getTextureIdFromName (getValStr("tx_bl_highlight"));
	TxId_B_HighLight  = rVR.getTextureIdFromName (getValStr("tx_b_highlight"));
	TxId_BR_HighLight = rVR.getTextureIdFromName (getValStr("tx_br_highlight"));

	//
	HeaderH = getValSInt32("header_h");

	return true;
}

// ----------------------------------------------------------------------------
COptionsContainerInsertion::COptionsContainerInsertion()
{
	TxId_R_Arrow = -2;
	TxId_L_Arrow = -2;
	TxId_T_Arrow = -2;
	TxId_B_Arrow = -2;
	TxId_InsertionBar = -2;
}

// ----------------------------------------------------------------------------
bool COptionsContainerInsertion::parse(xmlNodePtr cur)
{
	if (!CInterfaceOptions::parse (cur))
		return false;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	TxId_T_Arrow =  rVR.getTextureIdFromName (getValStr("arrow_top"));
	TxId_B_Arrow =  rVR.getTextureIdFromName (getValStr("arrow_down"));
	TxId_L_Arrow =  rVR.getTextureIdFromName (getValStr("arrow_left"));
	TxId_R_Arrow =  rVR.getTextureIdFromName (getValStr("arrow_right"));
	TxId_InsertionBar =  rVR.getTextureIdFromName (getValStr("insertion_bar"));

	return true;
}


// ***************************************************************************
COptionsContainerMove::COptionsContainerMove()
{
	TrackW = -8;
	TrackH = 22;
	TrackY = -4;
	TrackYWithTopResizer = -8;
	TrackHWithTopResizer = 18;
	ResizerSize = 8;
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
COptionsList::COptionsList()
{
	_NumParams= 0;
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


// ***************************************************************************
bool CMissionIconList::parse(xmlNodePtr cur)
{
	bool result = CInterfaceOptions::parse(cur);
	if (!result) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CViewRenderer &vr = *CViewRenderer::getInstance();
	for(std::map<std::string, CInterfaceOptionValue>::iterator it = _ParamValue.begin(); it != _ParamValue.end(); ++it)
	{
		int index;
		if (fromString(it->first, index))
		{
			if (index > 255)
			{
				nlwarning("bad index for texture");
			}
			else
			{
				string sTmp = it->second.getValStr();
				string sBack, sIcon;
				if (sTmp.find('|') != string::npos)
				{
					sBack = sTmp.substr(0,sTmp.find('|'));
					sIcon = sTmp.substr(sTmp.find('|')+1,sTmp.size());
				}
				else
				{
					sBack = sTmp;
				}

				sint32 texID = vr.getTextureIdFromName(sBack);
				if (texID != -1)
				{
					IconBackTexID.resize(std::max((int) IconBackTexID.size(), index + 1), -1);
					IconBackTexID[index] = texID;
				}
				texID = vr.getTextureIdFromName(sIcon);
				if (texID != -1)
				{
					IconTexID.resize(std::max((int) IconTexID.size(), index + 1), -1);
					IconTexID[index] = texID;
				}
			}
		}
	}
	NLMISC::contReset(_ParamValue); // not needed anymore
	return true;
}


// ***************************************************************************
COptionsAnimationSet::COptionsAnimationSet()
{
	AnimationSet= NULL;
}

// ***************************************************************************
COptionsAnimationSet::~COptionsAnimationSet()
{
	if(AnimationSet)
	{
		/* Important Note: this CInterfaceOptions is released BEFORE any CCharacter3d is released himself
			BUT this is OK, since the actual animationSet is kept by SmartPtr through UPlayList
			(see deleteAnimationSet() doc)
		 */
		CViewRenderer::getInstance()->getDriver()->deleteAnimationSet(AnimationSet);
		AnimationSet= NULL;
	}
}

// ***************************************************************************
bool COptionsAnimationSet::parse (xmlNodePtr cur)
{
	bool result = CInterfaceOptions::parse(cur);
	if (!result) return false;
	nlassert( CViewRenderer::getInstance()->getDriver() );

	// create the animation set
	AnimationSet= CViewRenderer::getInstance()->getDriver()->createAnimationSet();
	nlassert(AnimationSet);

	AnimMale.clear();
	AnimFemale.clear();

	// Add all male/female animations
	string sTmp;
	for(uint gender=0; gender<2; gender++)
	{
		string				prefix= (gender==0)?"m":"f";

		uint i = 0;
		do
		{
			sTmp = getValStr(prefix+toString(i));
			if (!sTmp.empty())
			{
				// get params
				vector<string>	params;
				splitString(sTmp, "|", params);
				// if error or first param empty, abort all
				if(params.empty() || params[0].empty())
				{
					sTmp.clear();
				}
				else
				{
					string	animName= params[0];
					animName += ".anim";
					uint animID = AnimationSet->addAnimation (animName.c_str(), animName.c_str());
					if (animID == UAnimationSet::NotFound)
						nlwarning ("Character3D : not found anim : %s", animName.c_str());
					// try to add the Face animation for this one (not important if failed)
					string	faceAnimName= getFaceAnimName(animName);
					AnimationSet->addAnimation (faceAnimName.c_str(), faceAnimName.c_str());

					// append the new anim desc
					CAnim	newAnim;
					newAnim.AnimId= animID;
					newAnim.ApplyRaceScalePos= true;
					// parse param
					for(uint p=1;p<params.size();p++)
					{
						if(params[p]=="no_race_scale_pos")
							newAnim.ApplyRaceScalePos= false;
					}
					// append to the correct anim list
					if(gender==0)
						AnimMale.push_back(newAnim);
					else
						AnimFemale.push_back(newAnim);
				}
			}
			++i;
		} while(!sTmp.empty());
	}

	// build
	AnimationSet->build ();

	return true;
}

// ***************************************************************************
string	COptionsAnimationSet::getFaceAnimName(const std::string &animName)
{
	string	faceAnimName= animName;
	string::size_type extPos= faceAnimName.find(".anim");
	if(extPos!=string::npos)
		faceAnimName= faceAnimName.substr(0, extPos);
	faceAnimName+= "_face";
	if(extPos!=string::npos)
		faceAnimName+= ".anim";
	return faceAnimName;
}



