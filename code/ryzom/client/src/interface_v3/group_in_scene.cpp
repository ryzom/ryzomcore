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

#include "group_in_scene.h"
#include "interface_manager.h"
#include "nel/gui/view_renderer.h"
#include "nel/misc/xml_auto_ptr.h"

using namespace std;
using namespace NLMISC;

extern CMatrix	MainSceneViewMatrix;
extern CMatrix	InvMainSceneViewMatrix;


// ***************************************************************************
const float CGroupInScene::NearDrawClip= 1.f;


// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupInScene, std::string, "in_scene");

REGISTER_UI_CLASS(CGroupInScene)

CGroupInScene::CGroupInScene(const TCtorParam &param)
: CInterfaceGroup(param)
{
	// Group in scene must update each frame
	_NeedFrameUpdatePos= true;

	_OffsetX = 0;
	_OffsetY = 0;

	Scale= 1;
	_UserScale= false;

	_ZBias= 0.f;

	Position= CVector::Null;

	_ProjCenter= CVector::Null;
	_IsGroupInScene = true;
}

// ***************************************************************************
CGroupInScene::~CGroupInScene()
{
}

// ***************************************************************************
void CGroupInScene::computeWindowPos(sint32 &newX, sint32 &newY, CVector &newProjCenter)
{
	// don't change X/Y by default
	newX= _X;
	newY= _Y;
	newProjCenter.x= float(_X) - _OffsetX;
	newProjCenter.y= float(_Y) - _OffsetY;

	if(getActive())
	{
		CViewRenderer &pVR = *CViewRenderer::getInstance();
		nlassert(isValidDouble(Position.x) && isValidDouble(Position.y) && isValidDouble(Position.z));
		CVector tmp = MainSceneViewMatrix * Position;
		if (tmp.y>=0.001)
		{
			tmp = pVR.getFrustum().projectZ (tmp);

			// Get the width and height
			tmp.x *= (float)CViewRenderer::getInstance()->getDriver()->getWindowWidth();
			tmp.y *= (float)CViewRenderer::getInstance()->getDriver()->getWindowHeight();

			// position without offset, in float
			newProjCenter.x= tmp.x;
			newProjCenter.y= tmp.y;

			// Set the current Z
			newProjCenter.z = -tmp.z;
			_DepthForZSort= newProjCenter.z;
			// Add ZBias (only if won't be clipped)
			if(newProjCenter.z > NearDrawClip)
			{
				newProjCenter.z+= getZBias();
				const float	zthreshold= 0.01f;
				newProjCenter.z= max(newProjCenter.z, pVR.getFrustum().Near+zthreshold);
				newProjCenter.z= max(newProjCenter.z, NearDrawClip + zthreshold);
			}

			// Set the position
			newX = (sint)floor (tmp.x+0.5f) + _OffsetX;
			newY = (sint)floor (tmp.y+0.5f) + _OffsetY;
		}
		else
		{
			_DepthForZSort= newProjCenter.z = tmp.y;
		}
	}
	else
	{
		_DepthForZSort= newProjCenter.z = -1.f;
	}

	nlassert (isValidDouble (newProjCenter.z));
}

// ***************************************************************************
void CGroupInScene::updateCoords()
{
	// Get the x and the y
	computeWindowPos(_X, _Y, _ProjCenter);

	CInterfaceGroup::updateCoords();
}

// ***************************************************************************
void CGroupInScene::onFrameUpdateWindowPos (sint dx, sint dy)
{
	// I am the root group, so I decide of the new window position.
	sint32	newX, newY;
	computeWindowPos(newX, newY, _ProjCenter);

	dx= newX - _X;
	dy= newY - _Y;

	// Change the X and Y only here
	_X= newX;
	_Y= newY;

	// CInterfaceGroup::onFrameUpdateWindowPos will apply the delta on XReal / YReal
	CInterfaceGroup::onFrameUpdateWindowPos(dx, dy);
}

// ***************************************************************************
void CGroupInScene::draw()
{
	H_AUTO( RZ_Interface_CGroupInScene_draw )

	if (_ProjCenter.z > NearDrawClip)
	{
		CViewRenderer &pVR = *CViewRenderer::getInstance();

		// Set the current Z, and projCenter / scale
		if(_UserScale)
			pVR.setInterfaceDepth (_ProjCenter, Scale);
		else
			pVR.setInterfaceDepth (_ProjCenter, 1);

		CInterfaceGroup::draw();
	}
}

// ***************************************************************************

bool CGroupInScene::parse (xmlNodePtr cur,  CInterfaceGroup *parent)
{
	if (!CInterfaceGroup::parse (cur,  parent))
		return false;

	CXMLAutoPtr ptr;

	_OffsetX = 0;
	ptr = xmlGetProp (cur, (xmlChar*)"in_scene_offset_x");
	if (ptr)
		fromString((const char*)ptr, _OffsetX);
	_OffsetY = 0;
	ptr = xmlGetProp (cur, (xmlChar*)"in_scene_offset_y");
	if (ptr)
		fromString((const char*)ptr, _OffsetY);
	_UserScale= false;
	ptr = xmlGetProp (cur, (xmlChar*)"user_scale");
	if (ptr)
		_UserScale= convertBool(ptr);

	return true;
}

// ***************************************************************************
void CGroupInScene::setUserScale(bool swd)
{
	_UserScale= swd;
}

// ***************************************************************************
void CGroupInScene::serial(NLMISC::IStream &f)
{
	CInterfaceGroup::serial(f);
	f.serial(_OffsetX);
	f.serial(_OffsetY);
	f.serial(_UserScale);
}


