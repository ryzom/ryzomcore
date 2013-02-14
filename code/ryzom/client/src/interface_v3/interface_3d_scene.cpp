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

#include "interface_3d_scene.h"
#include "interface_manager.h"
#include "character_3d.h"
#include "../time_client.h"
#include "../entities.h"

#include "nel/3d/u_point_light.h"
#include "nel/3d/u_particle_system_instance.h"
#include "nel/3d/u_animation_set.h"

#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/action_handler.h"

#include "nel/gui/lua_ihm.h"

// ----------------------------------------------------------------------------
using namespace std;
using namespace NL3D;
using namespace NLMISC;

// ----------------------------------------------------------------------------
// CInterface3DScene
// ----------------------------------------------------------------------------
CInterface3DScene::CInterface3DScene(const TCtorParam &param)
: CInterfaceGroup(param)
{
	_Scene = NULL;
	_AutoAnimSet = NULL;
	_CurrentCS = 0;
	_CurrentCamera = 0;
	_MouseLDown = false;
	_MouseRDown = false;
	_UserInteraction = false;
	_RotYFactor = _RotZFactor = 0.005f;
	_RotYLimitMin = _RotZLimitMin = -(float)(180.0f / NLMISC::Pi);
	_RotYLimitMax = _RotZLimitMax = (float)(180.0f / NLMISC::Pi);
	_DistLimitMin = 0.1f;
	_DistLimitMax = 15.0f;
	_DistFactor = 0.005f;
}

// ----------------------------------------------------------------------------
CInterface3DScene::~CInterface3DScene()
{
	uint i;

	for (i = 0; i < _Characters.size(); ++i)
		delete _Characters[i];
	for (i = 0; i < _IGs.size(); ++i)
		delete _IGs[i];
	for (i = 0; i < _Cameras.size(); ++i)
		delete _Cameras[i];
	for (i = 0; i < _Lights.size(); ++i)
		delete _Lights[i];
	for (i = 0; i < _Shapes.size(); ++i)
		delete _Shapes[i];
	for (i = 0; i < _FXs.size(); ++i)
		delete _FXs[i];

	NL3D::UDriver *Driver = CViewRenderer::getInstance()->getDriver();
	if (_Scene != NULL)
		Driver->deleteScene (_Scene);

	if (_AutoAnimSet != NULL)
		Driver->deleteAnimationSet(_AutoAnimSet);
}

// ----------------------------------------------------------------------------
CInterface3DCharacter	*CInterface3DScene::getCharacter3D(uint index)
{
	nlassert(index < _Characters.size());
	return _Characters[index];
}

// ----------------------------------------------------------------------------
CInterface3DCamera	*CInterface3DScene::getCamera(uint index)
{
	nlassert(index < _Cameras.size());
	return _Cameras[index];
}

// ----------------------------------------------------------------------------
bool CInterface3DScene::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	if (!CInterfaceElement::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr ptr;
	double value;

	// Check for user interaction properties
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"user_interaction" );
	if (ptr) _UserInteraction = convertBool(ptr);

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"rotz_limit_min" );
	if (ptr)
	{
		fromString((const char*)ptr, value);
		_RotZLimitMin = (float)(value * (NLMISC::Pi/180.0));
	}

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"rotz_limit_max" );
	if (ptr)
	{
		fromString((const char*)ptr, value);
		_RotZLimitMax = (float)(value * (NLMISC::Pi/180.0));
	}

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"rotz_factor" );
	if (ptr) fromString((const char*)ptr, _RotZFactor);

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"roty_limit_min" );
	if (ptr)
	{
		fromString((const char*)ptr, value);
		_RotYLimitMin = (float)(value * (NLMISC::Pi/180.0));
	}

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"roty_limit_max" );
	if (ptr)
	{
		fromString((const char*)ptr, value);
		_RotYLimitMax = (float)(value * (NLMISC::Pi/180.0));
	}

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"roty_factor" );
	if (ptr) fromString((const char*)ptr, _RotYFactor);

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"dist_limit_min" );
	if (ptr) fromString((const char*)ptr, _DistLimitMin);

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"dist_limit_max" );
	if (ptr) fromString((const char*)ptr, _DistLimitMax);

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"dist_factor" );
	if (ptr) fromString((const char*)ptr, _DistFactor);

	// Check right now if this is a reference view
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"reference" );
	_Ref3DScene = NULL;
	if (ptr)
	{
		CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(this->getId(), ptr);
		_Ref3DScene = dynamic_cast<CInterface3DScene*>(pIE);
	}
	if (_Ref3DScene != NULL)
	{
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"curcam" );
		if (ptr) setCurrentCamera (ptr);
		return true;
	}

	NL3D::UDriver *Driver = CViewRenderer::getInstance()->getDriver();
	nlassert ( Driver != NULL);

	_Scene = Driver->createScene(true);

	_Scene->enableLightingSystem(true);

	CRGBA rgbaTmp;
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"ambient" );
	rgbaTmp = CRGBA::Black;
	if (ptr) rgbaTmp = convertColor(ptr);
	_Scene->setAmbientGlobal(rgbaTmp);

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"sun_ambient" );
	rgbaTmp = CRGBA(50,50,50);
	if (ptr) rgbaTmp = convertColor(ptr);
	_Scene->setSunAmbient(rgbaTmp);

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"sun_diffuse" );
	rgbaTmp = CRGBA::White;
	if (ptr) rgbaTmp = convertColor(ptr);
	_Scene->setSunDiffuse(rgbaTmp);

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"sun_specular" );
	rgbaTmp = CRGBA::White;
	if (ptr) rgbaTmp = convertColor(ptr);
	_Scene->setSunSpecular(rgbaTmp);

	CVector v(-1,1,-1);
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"sun_direction" );
	if (ptr) v = convertVector(ptr);
	_Scene->setSunDirection(v);

	// Read all children
//	bool ok = true;
	cur = cur->children;
	while (cur)
	{
		// Check that this is a camera node
		if ( stricmp((char*)cur->name,"character3d") == 0 )
		{
			CInterface3DCharacter *pCha = new CInterface3DCharacter;
			if (!pCha->parse(cur,this))
			{
				delete pCha;
				nlwarning("character3d not added to scene3d");
			}
			else
			{
				_Characters.push_back(pCha);
			}
		}
		else if ( stricmp((char*)cur->name,"ig") == 0 )
		{
			CInterface3DIG *pIG = new CInterface3DIG;
			if (!pIG->parse(cur,this))
			{
				delete pIG;
				nlwarning("ig not added to scene3d");
			}
			else
			{
				_IGs.push_back(pIG);
			}
		}
		else if ( stricmp((char*)cur->name,"shape") == 0 )
		{
			CInterface3DShape *pShp = new CInterface3DShape;
			if (!pShp->parse(cur,this))
			{
				delete pShp;
				nlwarning("shape not added to scene3d");
			}
			else
			{
				_Shapes.push_back(pShp);
			}
		}
		else if ( stricmp((char*)cur->name,"camera") == 0 )
		{
			CInterface3DCamera *pCam = new CInterface3DCamera;
			if (!pCam->parse(cur,this))
			{
				delete pCam;
				nlwarning("camera not added to scene3d");
			}
			else
			{
				_Cameras.push_back(pCam);
			}
		}
		else if ( stricmp((char*)cur->name,"light") == 0 )
		{
			CInterface3DLight *pLig = new CInterface3DLight;
			if (!pLig->parse(cur,this))
			{
				delete pLig;
				nlwarning("light not added to scene3d");
			}
			else
			{
				_Lights.push_back(pLig);
			}
		}
		else if ( stricmp((char*)cur->name,"fx") == 0 )
		{
			CInterface3DFX *pFX = new CInterface3DFX;
			if (!pFX->parse(cur,this))
			{
				delete pFX;
				nlwarning("fx not added to scene3d");
			}
			else
			{
				_FXs.push_back(pFX);
			}
		}
		else if ( stricmp((char*)cur->name,"auto_anim") == 0 )
		{
			CXMLAutoPtr ptr((const char*)xmlGetProp (cur, (xmlChar*)"name"));
			string animName;
			if (ptr)
				animName = strlwr (CFile::getFilenameWithoutExtension(ptr));

			if (!animName.empty())
			{
				if (_AutoAnimSet == NULL)
					_AutoAnimSet = CViewRenderer::getInstance()->getDriver()->createAnimationSet();
				uint id = _AutoAnimSet->addAnimation (ptr, animName.c_str ());
				if (id == UAnimationSet::NotFound)
				{
					nlwarning ("Can't load automatic animation '%s'", animName.c_str());
				}
			}
			else
			{
				nlwarning ("Can't get automatic animation name");
			}
		}

		cur = cur->next;
	}

	// if some auto_anim, found, compile and set auto_anim
	if (_AutoAnimSet != NULL)
	{
		_AutoAnimSet->build ();
		_Scene->setAutomaticAnimationSet (_AutoAnimSet);
	}

	// If no camera create the default one
	if (_Cameras.size() == 0)
	{
		CInterface3DCamera *pCam = new CInterface3DCamera;
		_Cameras.push_back(pCam);
	}

	_CurrentCamera = 0;

	// Initialize all camera distance
	for (uint i = 0; i < _Cameras.size(); ++i)
	{
		CInterface3DCamera *pCam = _Cameras[i];
		pCam->setDist ((pCam->getPos() - pCam->getTarget()).norm());
	}

	// Get the current camera
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"curcam" );
	if (ptr) setCurrentCamera (ptr);

	return true;
}

// ----------------------------------------------------------------------------
void CInterface3DScene::checkCoords()
{
	uint i;

	for (i = 0; i < _Characters.size(); ++i)
	{
		_Characters[i]->checkCoords();
	}
	for (i = 0; i < _IGs.size(); ++i)
		_IGs[i]->checkCoords();
	for (i = 0; i < _Cameras.size(); ++i)
		_Cameras[i]->checkCoords();
	for (i = 0; i < _Lights.size(); ++i)
		_Lights[i]->checkCoords();
	for (i = 0; i < _FXs.size(); ++i)
		_FXs[i]->checkCoords();

	if (_Scene != NULL)
		_Scene->animate (TimeInSec-FirstTimeInSec);
}

// ----------------------------------------------------------------------------
void CInterface3DScene::updateCoords ()
{
	CViewBase::updateCoords();
}

// ----------------------------------------------------------------------------
void CInterface3DScene::draw ()
{
	H_AUTO( RZ_Interface_CInterface3DScene_draw  )

	NL3D::UDriver *Driver = CViewRenderer::getInstance()->getDriver();

	if ( Driver == NULL)
		return;

	// No Op if screen minimized
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	if(rVR.isMinimized())
		return;

	CInterface3DScene *pDisp = this;
	// If this is a reference view
	if (_Ref3DScene != NULL)
	{
		pDisp = _Ref3DScene;
		pDisp->setFlareContext(1);
	}
	else
	{
		pDisp->setFlareContext(0);
	}

	// This is not a reference view !
	if (pDisp->_Scene == NULL)
		return;

	CInterface3DCamera *pI3DCam = pDisp->_Cameras[_CurrentCamera];

	// TEMP TEMP TEMP DISPLAY BACKGROUND
	//rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal, _WReal, _HReal, 0, false,
	//						rVR.getBlankTextureId(), CRGBA(255,255,255,255) );
	// TEMP TEMP TEMP

	rVR.flush();

	// Viewport and frustrum
	uint32 wsw, wsh;

	sint32 oldSciX, oldSciY, oldSciW, oldSciH;
	makeNewClip (oldSciX, oldSciY, oldSciW, oldSciH);

	// Display sons only if not total clipped
	if( rVR.isClipWindowEmpty() )
	{
		restoreClip (oldSciX, oldSciY, oldSciW, oldSciH);
		return;
	}

	sint32 clipx,clipy,clipw,cliph;
	getClip (clipx, clipy, clipw, cliph);

	rVR.getScreenSize (wsw, wsh);
	NL3D::CViewport oldVP = Driver->getViewport();
	NL3D::CViewport newVP;
	float vpX = (float) (clipx) / iavoid0(wsw);
	float vpY = (float) (clipy) / iavoid0(wsh);
	float vpW = (float) clipw / iavoid0(wsw);
	float vpH = (float) cliph / iavoid0(wsh);
	newVP.init(vpX, vpY, vpW, vpH);
	NL3D::CFrustum  oldFrustum = CViewRenderer::getInstance()->getDriver()->getFrustum();
	NL3D::CFrustum  newFrustum;
	newFrustum.initPerspective (pI3DCam->getFOV() * (float) (NLMISC::Pi / 180), (float) _WReal / iavoid0(_HReal), 0.1f, 100.f);

	// Ajust frustum when there's clamping on border of screen
	float xLeft		= 0.f;
	float xRight	= 1.f;
	float yBottom	= 0.f;
	float yTop		= 1.f;

	// We assume that the viewport has dimensions < to those of the screen
	if ((_XReal+_WReal) > (clipx+clipw)) // right clamp ?
	{
		xRight = ((clipx+clipw) - _XReal) / (float) _WReal;
	}
	else if (_XReal < clipx) // left clamp
	{
		xLeft = (clipx - _XReal) / (float) _WReal;
	}
	if ((_YReal + _HReal) > (clipy+cliph)) // top clamp ?
	{
		yTop = ((clipy+cliph) - _YReal) / (float) _HReal;
	}
	else if (_YReal < clipy) // bottom clamp
	{
		yBottom = (clipy - _YReal) / (float) _HReal;
	}

	// adjust frustum
	float fWidth = newFrustum.Right - newFrustum.Left;
	float fLeft  = newFrustum.Left;
	newFrustum.Left = fLeft + fWidth * xLeft;
	newFrustum.Right = fLeft + fWidth * xRight;
	float fHeight = newFrustum.Top - newFrustum.Bottom;
	float fBottom = newFrustum.Bottom;
	newFrustum.Bottom = fBottom + fHeight * yBottom;
	newFrustum.Top    = fBottom + fHeight * yTop;

	pDisp->_Scene->setViewport(newVP);
	NL3D::UCamera cam = pDisp->_Scene->getCam();
	cam.setFrustum(newFrustum);

	// Rotate the camera position around the target with the rot parameters
	CVector pos = pI3DCam->getPos() - pI3DCam->getTarget();
//	float dist = pos.norm();
	pos.normalize();
	CMatrix m;
	m.identity();
	m.rotateZ(pI3DCam->getRotZ());
	m.rotateX(pI3DCam->getRotY());
	pos = m.mulVector(pos);
	pos = pos * pI3DCam->getDist();
	pos = pos + pI3DCam->getTarget();
	cam.lookAt (pos, pI3DCam->getTarget(), pI3DCam->getRoll() * (float) (NLMISC::Pi / 180));

	uint i;
	if (_IGs.size() > 0)
	{
		for (i = 0; i < _Characters.size(); ++i)
			_Characters[i]->setClusterSystem (_IGs[_CurrentCS]->getIG());
		for (i = 0; i < _Shapes.size(); ++i)
			_Shapes[i]->getShape().setClusterSystem (_IGs[_CurrentCS]->getIG());
		for (i = 0; i < _FXs.size(); ++i)
			if (!_FXs[i]->getPS().empty())
				_FXs[i]->getPS().setClusterSystem (_IGs[_CurrentCS]->getIG());
		cam.setClusterSystem (_IGs[_CurrentCS]->getIG());
	}
	else
	{
		for (i = 0; i < _Characters.size(); ++i)
			_Characters[i]->setClusterSystem ((UInstanceGroup*)-1);
		for (i = 0; i < _Shapes.size(); ++i)
		{
			if (!_Shapes[i]->getShape().empty())
				_Shapes[i]->getShape().setClusterSystem ((UInstanceGroup*)-1);
		}
		for (i = 0; i < _FXs.size(); ++i)
			if (!_FXs[i]->getPS().empty())
				_FXs[i]->getPS().setClusterSystem ((UInstanceGroup*)-1);
		cam.setClusterSystem ((UInstanceGroup*)-1);
	}

	////////////////////////
	// Clear the Z-Buffer //
	////////////////////////
		NL3D::CScissor oldScissor = Driver->getScissor();
		NL3D::CScissor newScissor;
		newScissor.X = vpX;
		newScissor.Y = vpY;
		newScissor.Width = vpW;
		newScissor.Height = vpH;
		Driver->setScissor(newScissor);
		Driver->clearZBuffer();
		Driver->setScissor(oldScissor);
	///////////////////////////////////////////////

	pDisp->_Scene->render();

	Driver->setViewport(oldVP);
	Driver->setFrustum(oldFrustum);

	// Restaure render states
	CViewRenderer::getInstance()->setRenderStates();

	restoreClip (oldSciX, oldSciY, oldSciW, oldSciH);
}

// ----------------------------------------------------------------------------
bool CInterface3DScene::handleEvent (const NLGUI::CEventDescriptor &event)
{
	if (!_UserInteraction)
		return false;

	if (!_Active)
		return false;
	// if focus is lost then cancel rotation / zoom
	if (event.getType() == NLGUI::CEventDescriptor::system)
	{
		const NLGUI::CEventDescriptorSystem &eds = (const NLGUI::CEventDescriptorSystem &) event;
		if (eds.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::setfocus)
		{
			const NLGUI::CEventDescriptorSetFocus &edsf = (const NLGUI::CEventDescriptorSetFocus &) eds;
			if (edsf.hasFocus() == false)
			{
				_MouseLDown = false;
				_MouseRDown = false;
				return true;
			}
		}
	}
	if (event.getType() == NLGUI::CEventDescriptor::mouse)
	{
		const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
		if ((CWidgetManager::getInstance()->getCapturePointerLeft() != this) &&
			(CWidgetManager::getInstance()->getCapturePointerRight() != this) &&
			(!((eventDesc.getX() >= _XReal) &&
			(eventDesc.getX() < (_XReal + _WReal))&&
			(eventDesc.getY() > _YReal) &&
			(eventDesc.getY() <= (_YReal+ _HReal)))))
			return false;

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown)
		{
			_MouseLDown = true;
			_MouseLDownX = eventDesc.getX();
			_MouseLDownY = eventDesc.getY();
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			CWidgetManager::getInstance()->setCapturePointerLeft(this); // Because we are not just a control
			return true;
		}
		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
		{
			_MouseLDown = false;
			return true;
		}
		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightdown)
		{
			_MouseRDown = true;
			_MouseRDownX = eventDesc.getX();
			_MouseRDownY = eventDesc.getY();
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			CWidgetManager::getInstance()->setCapturePointerRight(this); // Because we are not just a control
			return true;
		}
		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightup)
		{
			_MouseRDown = false;
			return true;
		}
		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousemove)
		{
			if (_MouseLDown)
			{
				sint32 dx = eventDesc.getX() - _MouseLDownX;
				sint32 dy = eventDesc.getY() - _MouseLDownY;
				mouseLMove (dx,dy);
				_MouseLDownX = eventDesc.getX();
				_MouseLDownY = eventDesc.getY();
			}
			if (_MouseRDown)
			{
				sint32 dx = eventDesc.getX() - _MouseRDownX;
				sint32 dy = eventDesc.getY() - _MouseRDownY;
				mouseRMove (dx,dy);
				_MouseRDownX = eventDesc.getX();
				_MouseRDownY = eventDesc.getY();
			}
			return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------------
void CInterface3DScene::mouseLMove (sint32 dx, sint32 dy)
{
	const CInterface3DScene *pI3DS = (_Ref3DScene != NULL) ? _Ref3DScene : this;
	CInterface3DCamera *pI3DCam = pI3DS->_Cameras[_CurrentCamera];
	float ang = pI3DCam->getRotY() + ((float)dy)*_RotYFactor;
	clamp (ang, _RotYLimitMin, _RotYLimitMax);
	pI3DCam->setRotY (ang);
	ang = pI3DCam->getRotZ() - ((float)dx)*_RotZFactor;
	clamp (ang ,_RotZLimitMin, _RotZLimitMax);
	pI3DCam->setRotZ (ang);
}

// ----------------------------------------------------------------------------
void CInterface3DScene::mouseRMove (sint32 /* dx */, sint32 dy)
{
	const CInterface3DScene *pI3DS = (_Ref3DScene != NULL) ? _Ref3DScene : this;
	CInterface3DCamera *pI3DCam = pI3DS->_Cameras[_CurrentCamera];
	float dist = pI3DCam->getDist() - ((float)dy)*_DistFactor;
	clamp (dist, _DistLimitMin, _DistLimitMax);
	pI3DCam->setDist (dist);
}

// ----------------------------------------------------------------------------
CInterfaceElement* CInterface3DScene::getElement (const string &id)
{
	if (id == getId())
		return this;

	string sTmp = id.substr(0, getId().size());
	if (sTmp != getId()) return NULL;

	uint i;

	for (i = 0; i < _Characters.size(); ++i)
		if (id == _Characters[i]->getId())
			return _Characters[i];

	for (i = 0; i < _IGs.size(); ++i)
		if (id == _IGs[i]->getId())
			return _IGs[i];

	for (i = 0; i < _Shapes.size(); ++i)
		if (id == _Shapes[i]->getId())
			return _Shapes[i];

	for (i = 0; i < _Cameras.size(); ++i)
		if (id == _Cameras[i]->getId())
			return _Cameras[i];

	for (i = 0; i < _Lights.size(); ++i)
		if (id == _Lights[i]->getId())
			return _Lights[i];

	for (i = 0; i < _FXs.size(); ++i)
		if (id == _FXs[i]->getId())
			return _FXs[i];

	return NULL;
}
// ----------------------------------------------------------------------------
string CInterface3DScene::getCurrentCamera() const
{
	const CInterface3DScene *pI3DS = (_Ref3DScene != NULL) ? _Ref3DScene : this;
	string name = pI3DS->_Cameras[_CurrentCamera]->getId();
	name = name.substr(name.rfind(':'));
	return name;
}

// ----------------------------------------------------------------------------
void CInterface3DScene::setCurrentCamera (const string &name)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CInterface3DScene *pI3DS = (_Ref3DScene != NULL) ? _Ref3DScene : this;
	CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(pI3DS->getId(), name);
	CInterface3DCamera *pI3DCam = dynamic_cast<CInterface3DCamera*>(pIE);
	if (pI3DCam != NULL)
	{
		uint i = 0;
		for (i = 0; i < pI3DS->_Cameras.size(); ++i)
			if (pI3DS->_Cameras[i] == pI3DCam)
				break;
		if (i != pI3DS->_Cameras.size())
			_CurrentCamera = i;
	}
}

// ----------------------------------------------------------------------------
string CInterface3DScene::getCurrentClusterSystem () const
{
	const CInterface3DScene *pI3DS = (_Ref3DScene != NULL) ? _Ref3DScene : this;
	string name = pI3DS->_IGs[_CurrentCS]->getId();
	name = name.substr(name.rfind(':'));
	return name;
}

// ----------------------------------------------------------------------------
void CInterface3DScene::setCurrentClusterSystem(const string &sCSName)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CInterface3DScene *pI3DS = (_Ref3DScene != NULL) ? _Ref3DScene : this;
	CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(pI3DS->getId(), sCSName);
	CInterface3DIG *pI3DIG = dynamic_cast<CInterface3DIG*>(pIE);
	if (pI3DIG != NULL)
	{
		uint i = 0;
		for (i = 0; i < pI3DS->_IGs.size(); ++i)
			if (pI3DS->_IGs[i] == pI3DIG)
				break;
		if (i != pI3DS->_IGs.size())
			_CurrentCS = i;
	}
}

// ----------------------------------------------------------------------------
void CInterface3DScene::remove(NL3D::UInstanceGroup *pIG)
{
	uint32 i;
	for (i = 0; i < _Characters.size(); ++i)
		_Characters[i]->setClusterSystem ((UInstanceGroup*)NULL);
	for (i = 0; i < _Shapes.size(); ++i)
		_Shapes[i]->getShape().setClusterSystem ((UInstanceGroup*)NULL);
	for (i = 0; i < _FXs.size(); ++i)
		if (!_FXs[i]->getPS().empty())
			_FXs[i]->getPS().setClusterSystem ((UInstanceGroup*)NULL);

	CInterface3DScene *pDisp = this;
	if (_Ref3DScene != NULL)
		pDisp = _Ref3DScene;
	if (pDisp->_Scene == NULL)
		return;
	NL3D::UCamera cam = pDisp->_Scene->getCam();
	cam.setClusterSystem ((UInstanceGroup*)NULL);

	pIG->removeFromScene(*_Scene);
	_Scene->deleteInstanceGroup(pIG);
}

// ----------------------------------------------------------------------------
// CInterface3DCharacter
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
CInterface3DCharacter::CInterface3DCharacter()
{
	_Char3D = NULL;
}

// ----------------------------------------------------------------------------
CInterface3DCharacter::~CInterface3DCharacter()
{
	delete _Char3D;
}

// ----------------------------------------------------------------------------
bool CInterface3DCharacter::parse (xmlNodePtr cur, CInterface3DScene *parentGroup)
{
	if (!CInterfaceElement::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr ptr((const char*)xmlGetProp (cur, (xmlChar*)"dblink"));
	_DBLink = "";
	if (ptr) _DBLink = (const char *)ptr;

	CVector pos(0,0,0), rot(0,0,0);
	ptr = xmlGetProp (cur, (xmlChar*)"pos");
	if (ptr) pos = convertVector(ptr);

	ptr = xmlGetProp (cur, (xmlChar*)"rot");
	if (ptr) rot = convertVector(ptr);

	bool copyAnim = false;
	ptr = xmlGetProp (cur, (xmlChar*)"copy_anim");
	if (ptr) copyAnim = convertBool(ptr);

	_Char3D = new CCharacter3D;
	_Char3D->copyAnimation(copyAnim);
	_Char3D->init (parentGroup->getScene());
	_Char3D->setPos (pos.x, pos.y, pos.z);
	_Char3D->setRotEuler (	rot.x * ((float)(NLMISC::Pi / 180)),
							rot.y * ((float)(NLMISC::Pi / 180)),
							rot.z * ((float)(NLMISC::Pi / 180))	);
	checkCoords();

	return true;
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::checkCoords()
{
	if (_Char3D)
	{
		SCharacter3DSetup c3Ds = _Char3D->getCurrentSetup();
		if ((_DBLink.empty()) || (_DBLink == "player"))
			c3Ds.setupFromSERVERDataBase();
		else if (_DBLink == "target")
		{
			if (UserEntity != NULL)
			{
				CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());
				if (selection != NULL)
					c3Ds.setupFromSERVERDataBase(selection->slot());
			}
		}
		else
			c3Ds.setupFromDataBase (_DBLink);
		_Char3D->setup (c3Ds);
		_Char3D->animate (TimeInSec);
	}
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::setupCharacter3D(sint32 slot)
{
	SCharacter3DSetup c3Ds = _Char3D->getCurrentSetup();
	c3Ds.setupFromSERVERDataBase((uint8)slot);
	_Char3D->setup(c3Ds);
}

// ----------------------------------------------------------------------------
int CInterface3DCharacter::luaSetupCharacter3D(CLuaState &ls)
{
	const char *funcName = "setupCharacter3D";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
	setupCharacter3D((sint32) ls.toNumber(1));
	return 0;
}

// ----------------------------------------------------------------------------
int CInterface3DCharacter::luaEnableLOD(CLuaState &ls)
{
	const char *funcName = "enableLOD";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TBOOLEAN);
	if (!_Char3D->getSkeleton().empty())
	{
		_Char3D->getSkeleton().enableLOD(ls.toBoolean(1));
	}
	return 0;
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::setClusterSystem (UInstanceGroup *pIG)
{
	if (_Char3D != NULL)
		_Char3D->setClusterSystem (pIG);
}

// ----------------------------------------------------------------------------
float CInterface3DCharacter::getPosX () const
{
	if (_Char3D == NULL) return 0.0;
	float x, y ,z;
	_Char3D->getPos (x, y, z);
	return x;
}

// ----------------------------------------------------------------------------
float CInterface3DCharacter::getPosY () const
{
	if (_Char3D == NULL) return 0.0;
	float x, y ,z;
	_Char3D->getPos (x, y, z);
	return y;
}

// ----------------------------------------------------------------------------
float CInterface3DCharacter::getPosZ () const
{
	if (_Char3D == NULL) return 0.0;
	float x, y ,z;
	_Char3D->getPos (x, y, z);
	return z;
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::setPosX (float f)
{
	if (_Char3D == NULL) return;
	float x, y ,z;
	_Char3D->getPos(x, y, z);
	x = f;
	_Char3D->setPos(x, y, z);
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::setPosY (float f)
{
	if (_Char3D == NULL) return;
	float x, y ,z;
	_Char3D->getPos(x, y, z);
	y = f;
	_Char3D->setPos(x, y, z);
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::setPosZ (float f)
{
	if (_Char3D == NULL) return;
	float x, y ,z;
	_Char3D->getPos(x, y, z);
	z = f;
	_Char3D->setPos(x, y, z);
}

// ----------------------------------------------------------------------------
float CInterface3DCharacter::getRotX () const
{
	if (_Char3D == NULL) return 0.0;
	float x, y ,z;
	_Char3D->getRotEuler(x, y, z);
	return x / ((float)(NLMISC::Pi / 180));
}

// ----------------------------------------------------------------------------
float CInterface3DCharacter::getRotY () const
{
	if (_Char3D == NULL) return 0.0;
	float x, y ,z;
	_Char3D->getRotEuler(x, y, z);
	return y / ((float)(NLMISC::Pi / 180));
}

// ----------------------------------------------------------------------------
float CInterface3DCharacter::getRotZ () const
{
	if (_Char3D == NULL) return 0.0;
	float x, y ,z;
	_Char3D->getRotEuler(x, y, z);
	return z / ((float)(NLMISC::Pi / 180));
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::setRotX (float f)
{
	if (_Char3D == NULL) return;
	float x, y ,z;
	_Char3D->getRotEuler(x, y, z);
	x = f * ((float)(NLMISC::Pi / 180));
	_Char3D->setRotEuler(x, y, z);
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::setRotY (float f)
{
	if (_Char3D == NULL) return;
	float x, y ,z;
	_Char3D->getRotEuler(x, y, z);
	y = f * ((float)(NLMISC::Pi / 180));
	_Char3D->setRotEuler(x, y, z);
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::setRotZ (float f)
{
	if (_Char3D == NULL) return;
	float x, y ,z;
	_Char3D->getRotEuler(x, y, z);
	z = f * ((float)(NLMISC::Pi / 180));
	_Char3D->setRotEuler(x, y, z);
}

// ----------------------------------------------------------------------------
float CInterface3DCharacter::getHeadX () const
{
	if (_Char3D == NULL) return 0.0;
	float x, y ,z;
	_Char3D->getHeadPos (x, y, z);
	return x;
}

// ----------------------------------------------------------------------------
float CInterface3DCharacter::getHeadY () const
{
	if (_Char3D == NULL) return 0.0;
	float x, y ,z;
	_Char3D->getHeadPos (x, y, z);
	return y;
}

// ----------------------------------------------------------------------------
float CInterface3DCharacter::getHeadZ () const
{
	if (_Char3D == NULL) return 0.0;
	float x, y ,z;
	_Char3D->getHeadPos (x, y, z);
	return z;
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::setAnim (sint32 anim)
{
	if (_Char3D)
		_Char3D->setAnim(anim);

	checkCoords();
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::setPeople(const std::string & people)
{
	_Char3D->setPeople(EGSPD::CPeople::fromString(people));
}

// ----------------------------------------------------------------------------
std::string	CInterface3DCharacter::getPeople() const
{
	return EGSPD::CPeople::toString(_Char3D->getPeople());
}

// ----------------------------------------------------------------------------
void CInterface3DCharacter::setSex(bool male)
{
	_Char3D->setSex(male);
}

// ----------------------------------------------------------------------------
bool	CInterface3DCharacter::getSex() const
{
	return _Char3D->getSex();
}


// ----------------------------------------------------------------------------
// CInterface3DIG
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
CInterface3DIG::~CInterface3DIG()
{
	CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene*>(_Parent);
	nlassert(pI3DS != NULL);
	if (_IG)
	{
		_IG->removeFromScene(*pI3DS->getScene());
		pI3DS->getScene()->deleteInstanceGroup(_IG);
		_IG = NULL;
	}
}

// ----------------------------------------------------------------------------
bool CInterface3DIG::parse (xmlNodePtr cur, CInterface3DScene *parentGroup)
{
	if (!CInterfaceElement::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr ptr((const char*)xmlGetProp (cur, (xmlChar*)"pos"));
	if (ptr) _Pos = convertVector(ptr);

	ptr = xmlGetProp (cur, (xmlChar*)"rot");
	if (ptr) _Rot = convertVector(ptr);

	string name;
	ptr = xmlGetProp (cur, (xmlChar*)"name");
	if (ptr) name = (const char*)ptr;

	_Name = strlwr(name);
	_IG = UInstanceGroup::createInstanceGroup(_Name);
	if (_IG == NULL)
		return true; // Create anyway
	_IG->setPos (_Pos);
	//_IG->setRot (_Rot);
	setRotX (_Rot.x);
	setRotY (_Rot.y);
	setRotZ (_Rot.z);
	_IG->addToScene (*parentGroup->getScene(), CViewRenderer::getInstance()->getDriver() );
	parentGroup->getScene()->setToGlobalInstanceGroup (_IG);

	return true;
}

// ----------------------------------------------------------------------------
float CInterface3DIG::getPosX () const
{
	return _Pos.x;
}

// ----------------------------------------------------------------------------
float CInterface3DIG::getPosY () const
{
	return _Pos.y;
}

// ----------------------------------------------------------------------------
float CInterface3DIG::getPosZ () const
{
	return _Pos.z;
}

// ----------------------------------------------------------------------------
void CInterface3DIG::setPosX (float f)
{
	_Pos.x = f;
	if (_IG != NULL) _IG->setPos(_Pos);
}

// ----------------------------------------------------------------------------
void CInterface3DIG::setPosY (float f)
{
	_Pos.y = f;
	if (_IG != NULL) _IG->setPos(_Pos);
}

// ----------------------------------------------------------------------------
void CInterface3DIG::setPosZ (float f)
{
	_Pos.z = f;
	if (_IG != NULL) _IG->setPos(_Pos);
}

// ----------------------------------------------------------------------------
float CInterface3DIG::getRotX () const
{
	return _Rot.x;
}

// ----------------------------------------------------------------------------
float CInterface3DIG::getRotY () const
{
	return _Rot.y;
}

// ----------------------------------------------------------------------------
float CInterface3DIG::getRotZ () const
{
	return _Rot.z;
}

// ----------------------------------------------------------------------------
void CInterface3DIG::setRotX (float f)
{
	_Rot.x = f;
	CMatrix m;
	m.identity();
	m.setRot (_Rot,CMatrix::XYZ);
	CQuat q = m.getRot();
	_IG->setRotQuat (q);
}

// ----------------------------------------------------------------------------
void CInterface3DIG::setRotY (float f)
{
	_Rot.y = f;
	CMatrix m;
	m.identity();
	m.setRot (_Rot,CMatrix::XYZ);
	CQuat q = m.getRot();
	_IG->setRotQuat (q);
}

// ----------------------------------------------------------------------------
void CInterface3DIG::setRotZ (float f)
{
	_Rot.z = f;
	CMatrix m;
	m.identity();
	m.setRot (_Rot,CMatrix::XYZ);
	CQuat q = m.getRot();
	_IG->setRotQuat (q);
}

// ----------------------------------------------------------------------------
std::string CInterface3DIG::getName() const
{
	return _Name;
}

// ----------------------------------------------------------------------------
void CInterface3DIG::setName (const std::string &ht)
{
	string lwrname = strlwr(ht);
	if (lwrname != _Name)
	{
		CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene*>(_Parent);
		nlassert(pI3DS != NULL);

		if (_IG != NULL)
		{
			pI3DS->remove(_IG);
			_IG = NULL;
		}

		_Name = lwrname;
		_IG = UInstanceGroup::createInstanceGroup(_Name);
		if (_IG == NULL) return;
		_IG->setPos (_Pos);
		_IG->addToScene (*pI3DS->getScene(), CViewRenderer::getInstance()->getDriver() );
		pI3DS->getScene()->setToGlobalInstanceGroup (_IG);
	}
}

// ----------------------------------------------------------------------------
// CInterface3DShape
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
CInterface3DShape::~CInterface3DShape()
{
	CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene*>(_Parent);
	nlassert(pI3DS != NULL);
	if (!_Instance.empty())
		pI3DS->getScene()->deleteInstance(_Instance);
}

// ----------------------------------------------------------------------------
bool CInterface3DShape::parse (xmlNodePtr cur, CInterface3DScene *parentGroup)
{
	if (!CInterfaceElement::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr ptr((const char*)xmlGetProp (cur, (xmlChar*)"pos"));
	if (ptr) _Pos = convertVector(ptr);

	ptr = xmlGetProp (cur, (xmlChar*)"rot");
	if (ptr) _Rot = convertVector(ptr);

	string name;
	ptr = xmlGetProp (cur, (xmlChar*)"name");
	if (ptr) name = (const char*)ptr;

	_Name = strlwr(name);
	_Instance = parentGroup->getScene()->createInstance(_Name);
	if (_Instance.empty())
		return false;

	_Instance.setTransformMode(UTransformable::RotEuler);
	_Instance.setPos (_Pos);
	_Instance.setRotEuler (_Rot.x, _Rot.y, _Rot.z);

	return true;
}

// ----------------------------------------------------------------------------
float CInterface3DShape::getPosX () const
{
	return _Pos.x;
}

// ----------------------------------------------------------------------------
float CInterface3DShape::getPosY () const
{
	return _Pos.y;
}

// ----------------------------------------------------------------------------
float CInterface3DShape::getPosZ () const
{
	return _Pos.z;
}

// ----------------------------------------------------------------------------
void CInterface3DShape::setPosX (float f)
{
	_Pos.x = f;
	if (!_Instance.empty()) _Instance.setPos(_Pos);
}

// ----------------------------------------------------------------------------
void CInterface3DShape::setPosY (float f)
{
	_Pos.y = f;
	if (!_Instance.empty()) _Instance.setPos(_Pos);
}

// ----------------------------------------------------------------------------
void CInterface3DShape::setPosZ (float f)
{
	_Pos.z = f;
	if (!_Instance.empty()) _Instance.setPos(_Pos);
}

// ----------------------------------------------------------------------------
float CInterface3DShape::getRotX () const
{
	return _Rot.x / ((float)(NLMISC::Pi / 180));
}

// ----------------------------------------------------------------------------
float CInterface3DShape::getRotY () const
{
	return _Rot.y / ((float)(NLMISC::Pi / 180));
}

// ----------------------------------------------------------------------------
float CInterface3DShape::getRotZ () const
{
	return _Rot.z / ((float)(NLMISC::Pi / 180));
}

// ----------------------------------------------------------------------------
void CInterface3DShape::setRotX (float f)
{
	_Rot.x = f * ((float)(NLMISC::Pi / 180));
	if (!_Instance.empty()) _Instance.setRotEuler (_Rot.x, _Rot.y, _Rot.z);
}

// ----------------------------------------------------------------------------
void CInterface3DShape::setRotY (float f)
{
	_Rot.y = f * ((float)(NLMISC::Pi / 180));
	if (!_Instance.empty()) _Instance.setRotEuler (_Rot.x, _Rot.y, _Rot.z);
}

// ----------------------------------------------------------------------------
void CInterface3DShape::setRotZ (float f)
{
	_Rot.z = f * ((float)(NLMISC::Pi / 180));
	if (!_Instance.empty()) _Instance.setRotEuler (_Rot.x, _Rot.y, _Rot.z);
}

// ----------------------------------------------------------------------------
std::string CInterface3DShape::getName() const
{
	return _Name;
}

// ----------------------------------------------------------------------------
void CInterface3DShape::setName (const std::string &ht)
{
	if (ht.empty())
	{
		CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene*>(_Parent);
		nlassert(pI3DS != NULL);

		if (!_Instance.empty())
		{
			pI3DS->getScene()->deleteInstance(_Instance);
		}
		return;
		_Name.clear();
	}

	string lwrname = toLower(ht);
	if (lwrname != _Name)
	{
		CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene*>(_Parent);
		nlassert(pI3DS != NULL);

		if (!_Instance.empty())
		{
			pI3DS->getScene()->deleteInstance(_Instance);
		}

		_Name = lwrname;
		_Instance = pI3DS->getScene()->createInstance(_Name);
		if (_Instance.empty()) return;
		_Instance.setTransformMode(UTransformable::RotEuler);
		_Instance.setPos (_Pos);
		_Instance.setRotEuler (_Rot.x, _Rot.y, _Rot.z);
	}
}

// ----------------------------------------------------------------------------
// CInterface3DCamera
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
bool CInterface3DCamera::parse (xmlNodePtr cur, CInterface3DScene *parentGroup)
{
	if (!CInterfaceElement::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr ptr((const char*)xmlGetProp (cur, (xmlChar*)"pos"));
	if (ptr) _Pos = convertVector(ptr);
	ptr = xmlGetProp (cur, (xmlChar*)"target");
	if (ptr) _Target = convertVector(ptr);
	ptr = xmlGetProp (cur, (xmlChar*)"fov");
	if (ptr) fromString((const char*)ptr, _FOV);
	ptr = xmlGetProp (cur, (xmlChar*)"roll");
	if (ptr) fromString((const char*)ptr, _Roll);

	return true;
}

// ----------------------------------------------------------------------------
void CInterface3DCamera::reset()
{
	setTgtX(getTgtX());
	_Rot = NLMISC::CVector(0,0,0);
}

// ----------------------------------------------------------------------------
// CInterface3DLight
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
CInterface3DLight::~CInterface3DLight()
{
	CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene *>(_Parent);
	nlassert(pI3DS != NULL);
	pI3DS->getScene()->deletePointLight(_Light);
}

// ----------------------------------------------------------------------------
bool CInterface3DLight::parse (xmlNodePtr cur, CInterface3DScene *parentGroup)
{
	if (!CInterfaceElement::parse(cur, parentGroup))
		return false;

	_Light = parentGroup->getScene()->createPointLight();

	CXMLAutoPtr ptr((const char*)xmlGetProp (cur, (xmlChar*)"pos"));
	if (ptr) _Pos = convertVector(ptr);
	ptr = xmlGetProp (cur, (xmlChar*)"color");
	if (ptr) _Color = convertColor(ptr);
	ptr = xmlGetProp (cur, (xmlChar*)"near");
	if (ptr) fromString((const char*)ptr, _Near);
	ptr = xmlGetProp (cur, (xmlChar*)"far");
	if (ptr) fromString((const char*)ptr, _Far);

	_Light.setPos(_Pos);
	_Light.setAmbient (CRGBA(0,0,0));
	_Light.setDiffuse (CRGBA(255,255,255));
	_Light.setSpecular (CRGBA(255,255,255));
	_Light.setColor (_Color);
	_Light.setupAttenuation (_Near, _Far);

	return true;
}

// ----------------------------------------------------------------------------
void CInterface3DLight::setPosX(float f)
{
	_Pos.x = f;
	_Light.setPos(_Pos);
}

// ----------------------------------------------------------------------------
void CInterface3DLight::setPosY(float f)
{
	_Pos.y = f;
	_Light.setPos(_Pos);
}

// ----------------------------------------------------------------------------
void CInterface3DLight::setPosZ(float f)
{
	_Pos.z = f;
	_Light.setPos(_Pos);
}

// ----------------------------------------------------------------------------
void CInterface3DLight::setNear(float f)
{
	_Near = f;
	_Light.setupAttenuation (_Near, _Far);
}

// ----------------------------------------------------------------------------
void CInterface3DLight::setFar(float f)
{
	_Far = f;
	_Light.setupAttenuation (_Near, _Far);
}

// ----------------------------------------------------------------------------
void CInterface3DLight::setColR(sint32 f)
{
	_Color.R = (uint8)f;
	_Light.setColor (_Color);
}

// ----------------------------------------------------------------------------
void CInterface3DLight::setColG(sint32 f)
{
	_Color.G = (uint8)f;
	_Light.setColor (_Color);
}

// ----------------------------------------------------------------------------
void CInterface3DLight::setColB(sint32 f)
{
	_Color.B = (uint8)f;
	_Light.setColor (_Color);
}

// ----------------------------------------------------------------------------
// CInterface3DFX
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
CInterface3DFX::~CInterface3DFX()
{
}

// ----------------------------------------------------------------------------
bool CInterface3DFX::parse (xmlNodePtr cur, CInterface3DScene *parentGroup)
{
	if (!CInterfaceElement::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr ptr((const char*)xmlGetProp (cur, (xmlChar*)"pos"));
	if (ptr) _Pos = convertVector(ptr);

	ptr = xmlGetProp (cur, (xmlChar*)"rot");
	if (ptr) _Rot = convertVector(ptr);

	string name;
	ptr = xmlGetProp (cur, (xmlChar*)"name");
	if (ptr) name = (const char*)ptr;

	_Name = strlwr(name);

	return true;
}

// ----------------------------------------------------------------------------
void CInterface3DFX::checkCoords()
{
	if (!_FX.empty())
	{
		if (!_FX.isValid())
		{
			CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene*>(_Parent);
			nlassert(pI3DS != NULL);
			pI3DS->getScene()->deleteInstance(_FX);
		}
	}
}

// ----------------------------------------------------------------------------
float CInterface3DFX::getPosX () const
{
	return _Pos.x;
}

// ----------------------------------------------------------------------------
float CInterface3DFX::getPosY () const
{
	return _Pos.y;
}

// ----------------------------------------------------------------------------
float CInterface3DFX::getPosZ () const
{
	return _Pos.z;
}

// ----------------------------------------------------------------------------
void CInterface3DFX::setPosX (float f)
{
	_Pos.x = f;
	if (!_FX.empty()) _FX.setPos (_Pos);
}

// ----------------------------------------------------------------------------
void CInterface3DFX::setPosY (float f)
{
	_Pos.y = f;
	if (!_FX.empty()) _FX.setPos (_Pos);
}

// ----------------------------------------------------------------------------
void CInterface3DFX::setPosZ (float f)
{
	_Pos.z = f;
	if (!_FX.empty()) _FX.setPos (_Pos);
}

// ----------------------------------------------------------------------------
float CInterface3DFX::getRotX () const
{
	return _Rot.x / ((float)(NLMISC::Pi / 180));
}

// ----------------------------------------------------------------------------
float CInterface3DFX::getRotY () const
{
	return _Rot.y / ((float)(NLMISC::Pi / 180));
}

// ----------------------------------------------------------------------------
float CInterface3DFX::getRotZ () const
{
	return _Rot.z / ((float)(NLMISC::Pi / 180));
}

// ----------------------------------------------------------------------------
void CInterface3DFX::setRotX (float f)
{
	_Rot.x = f;
	if (!_FX.empty()) _FX.setRotEuler(_Rot);
}

// ----------------------------------------------------------------------------
void CInterface3DFX::setRotY (float f)
{
	_Rot.y = f;
	if (!_FX.empty()) _FX.setRotEuler(_Rot);
}

// ----------------------------------------------------------------------------
void CInterface3DFX::setRotZ (float f)
{
	_Rot.z = f;
	if (!_FX.empty()) _FX.setRotEuler(_Rot);
}

// ----------------------------------------------------------------------------
std::string CInterface3DFX::getName() const
{
	return _Name;
}

// ----------------------------------------------------------------------------
void        CInterface3DFX::setName (const std::string &ht)
{
	_Name = ht;
	if (!_FX.empty())
		setStarted (true);
}

// ----------------------------------------------------------------------------
bool CInterface3DFX::getStarted() const
{
	return (!_FX.empty());
}

// ----------------------------------------------------------------------------
void CInterface3DFX::setStarted (bool b)
{
	if (b == true)
	{
		CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene*>(_Parent);
		nlassert(pI3DS != NULL);
		if (!_FX.empty())
			pI3DS->getScene()->deleteInstance(_FX);
		_FX.cast (pI3DS->getScene()->createInstance(_Name));
		if (_FX.empty())
			return;
		_FX.setTransformMode(UTransformable::RotEuler);
		_FX.setPos (_Pos);
		_FX.setRotEuler (_Rot.x, _Rot.y, _Rot.z);
	}
	else
	{
		CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene*>(_Parent);
		nlassert(pI3DS != NULL);
		if (!_FX.empty())
			pI3DS->getScene()->deleteInstance(_FX);
		_FX = NULL;
	}
}




/* end of interface_3d_scene.cpp */
