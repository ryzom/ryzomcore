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



// ----------------------------------------------------------------------------

#include "stdpch.h"

#include "view_radar.h"
#include "interface_manager.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/group_container.h"
#include "../npc_icon.h"
#include "nel/misc/fast_floor.h"

#include "../entities.h"

// ----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NL3D;

NLMISC_REGISTER_OBJECT(CViewBase, CViewRadar, std::string, "radar");

// ----------------------------------------------------------------------------

CViewRadar::CViewRadar(const TCtorParam &param)
	: CViewBase(param)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf *pUIMI = CDBManager::getInstance()->getDbProp( "UI:SAVE:INSCENE:FRIEND:MISSION_ICON" );
	if (pUIMI)
	{
		ICDBNode::CTextId textId;
		pUIMI->addObserver( &_MissionIconsObs, textId);
	}
	
	CCDBNodeLeaf *pUIMMI = CDBManager::getInstance()->getDbProp( "UI:SAVE:INSCENE:FRIEND:MINI_MISSION_ICON" );	
	if (pUIMMI)
	{
		ICDBNode::CTextId textId;
		pUIMMI->addObserver( &_MiniMissionSpotsObs, textId);
	}
}

bool CViewRadar::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	CXMLAutoPtr prop;

	//try to get props that can be inherited from groups
	//if a property is not defined, try to find it in the parent group.
	//if it is undefined, set it to zero
	if (! CViewBase::parse(cur,parentGroup) )
	{
		string tmp = string("cannot parse view:")+getId()+", parent:"+parentGroup->getId();
		nlinfo (tmp.c_str());
		return false;
	}

	// World size
	_WorldSize = 100.0;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"world_size" );
	if (prop) fromString((const char*)prop, _WorldSize);

	// Spot textures
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	
	// Large missions Icons
	const char *spotTextureNames[NbRadarSpotIds] = { "texture_std", "texture_missionlist", "texture_missionauto", "texture_missionstep" };

	// Mini missions Icons
	const char *spotMiniTextureNames[NbRadarSpotIds] = { "texture_std", "mini_texture_missionlist", "mini_texture_missionauto", "mini_texture_missionstep" };

	for (uint i=0; i!=NbRadarSpotIds; ++i)
	{
		string txName;

		// Large missions Icons
		prop = (char*) xmlGetProp( cur, (xmlChar*)spotTextureNames[i] );
		if (prop)
		{
			txName = (const char *) prop;
			txName = strlwr (txName);
		}
		_SpotDescriptions[i].TextureId.setTexture(txName.c_str());
		rVR.getTextureSizeFromId (_SpotDescriptions[i].TextureId, _SpotDescriptions[i].TxW, _SpotDescriptions[i].TxH);

		// Mini missions Icons
		prop = (char*) xmlGetProp( cur, (xmlChar*)spotMiniTextureNames[i] );
		if (prop)
		{
			txName = (const char *) prop;
			txName = strlwr (txName);
		}
		_SpotDescriptions[i].MiniTextureId.setTexture(txName.c_str());
		rVR.getTextureSizeFromId (_SpotDescriptions[i].MiniTextureId, _SpotDescriptions[i].MTxW, _SpotDescriptions[i].MTxH);

		if (i == 0)
			_SpotDescriptions[i].isMissionSpot = false;
		else
			_SpotDescriptions[i].isMissionSpot = true;
	}

	return true;
}

// ----------------------------------------------------------------------------
void CViewRadar::draw ()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();

	CEntityCL *user = EntitiesMngr.entity(0);
	if (user == NULL) return;

	CVectorD xyzRef = user->pos();
	const CVector dir = user->front();

	float angle = (float)(atan2(dir.y, dir.x) - (Pi / 2.0));
	CMatrix mat;
	mat.identity();
	// Scale to transform from world to interface screen
	mat.scale( CVector((float)(_WReal / _WorldSize), (float)(_HReal / _WorldSize), 1) );
	// local to user
	mat.rotateZ(-angle);
	xyzRef.z = 0;
	mat.translate(-xyzRef);

	float	maxSqrRadius= (float)sqr(_WorldSize/2);

	for (sint32 i = 1; i < 256; ++i)
	{
		CEntityCL *entity = EntitiesMngr.entity(i);
		if (entity == NULL) continue;

		// if the entity must not be shown in radar
		if(!entity->getDisplayInRadar())
			continue;

		// get entity pos
		CVectorD xyz = entity->pos();

		xyz.z = 0;
		// if the distance is too big so do not display the entity
		if ((sqr(xyz.x - xyzRef.x)+sqr(xyz.y - xyzRef.y)) > maxSqrRadius) continue;

		// Transform the dot
		xyz = mat * xyz;

		// Convert to screen
		sint32 x = OptFastFloor((float)xyz.x);
		sint32 y = OptFastFloor((float)xyz.y);

		CRGBA col = entity->getColor();

		if(getModulateGlobalColor())
			col.modulateFromColor (col, CWidgetManager::getInstance()->getGlobalColorForContent());
		else
			col.A = (uint8)(((sint32)col.A*((sint32)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);

		// Select the icon to display and draw it
		uint spotId = CNPCIconCache::getInstance().getNPCIcon(entity).getSpotId();
		CRadarSpotDesc spotDesc = _SpotDescriptions[spotId];
			
		if (!_MissionIconsObs._displayMissionSpots)
			spotDesc = _SpotDescriptions[0];

		if (spotDesc.isMissionSpot)
			col = CRGBA(255, 255, 255, 255);

		if (entity->isTarget())
			spotId = 4; // to make it over other spots

		// Draw it (and make sure mission icons are drawn over regular dot; caution: don't exceed the render layer range)
		if (spotDesc.isMissionSpot && _MiniMissionSpotsObs._displayMiniMissionSpots)
			rVR.drawRotFlipBitmap (_RenderLayer+spotId, _XReal+x-(spotDesc.MTxW/2)+(_WReal/2), _YReal+y-(spotDesc.MTxH/2)+(_HReal/2),
								spotDesc.MTxW, spotDesc.MTxH, 0, false, spotDesc.MiniTextureId, col );
		else
			rVR.drawRotFlipBitmap (_RenderLayer+spotId, _XReal+x-(spotDesc.TxW/2)+(_WReal/2), _YReal+y-(spotDesc.TxH/2)+(_HReal/2),
							spotDesc.TxW, spotDesc.TxH, 0, false, spotDesc.TextureId, col );
	}
}

// ----------------------------------------------------------------------------
void CViewRadar::updateCoords()
{
	CViewBase::updateCoords();
}

void CViewRadar::CDBMissionIconqObs::update(ICDBNode *node)
{
	_displayMissionSpots = ((CCDBNodeLeaf*)node)->getValueBool();
}

void CViewRadar::CDBMiniMissionSpotsObs::update(ICDBNode *node)
{
	_displayMiniMissionSpots = ((CCDBNodeLeaf*)node)->getValueBool();
}
