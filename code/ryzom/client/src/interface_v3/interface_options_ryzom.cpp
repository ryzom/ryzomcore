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

#include "nel/gui/interface_options.h"
#include "interface_manager.h"
#include "nel/gui/group_menu.h"
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

NLMISC_REGISTER_OBJECT(CInterfaceOptions, CMissionIconList, std::string, "mission_icons");

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
NLMISC_REGISTER_OBJECT(CInterfaceOptions, COptionsAnimationSet, std::string, "animation_set");
COptionsAnimationSet::COptionsAnimationSet( const TCtorParam &param ) :
CInterfaceOptions( param )
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



