/**
 * \file stereo_libvr.cpp
 * \brief CStereoLibVR
 * \date 2013-08-19 19:17MT
 * \author Thibaut Girka (ThibG)
 * CStereoLibVR
 */

/* 
 * Copyright (C) 2013  by authors
 * 
 * This file is part of NL3D.
 * NL3D is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * NL3D is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with NL3D.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_LIBVR

#include "std3d.h"
#include <nel/misc/time_nl.h>
#include <nel/3d/stereo_libvr.h>

// STL includes
#include <sstream>

// External includes
extern "C" {
#include <hmd.h>
}

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/material.h>
#include <nel/3d/texture_bloom.h>
#include <nel/3d/texture_user.h>
#include <nel/3d/driver_user.h>
#include <nel/3d/u_texture.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace NL3D {

extern const char *g_StereoOVR_fp40; //TODO: what?
extern const char *g_StereoOVR_arbfp1; //TODO: what?
extern const char *g_StereoOVR_ps_2_0; //TODO: what?

namespace {
sint s_DeviceCounter = 0;
};

class CStereoLibVRDeviceHandle : public IStereoDeviceFactory
{
public:
	// fixme: virtual destructor???
	IStereoDisplay *createDevice() const
	{
		CStereoLibVR *stereo = new CStereoLibVR(this);
		if (stereo->isDeviceCreated())
			return stereo;
		delete stereo;
		return NULL;
	}
};

class CStereoLibVRDevicePtr
{
public:
    struct hmd *HMDDevice;
    struct display_info HMDInfo;
    float InterpupillaryDistance;
};

CStereoLibVR::CStereoLibVR(const CStereoLibVRDeviceHandle *handle) : m_Stage(0), m_SubStage(0), m_OrientationCached(false), m_Driver(NULL), m_BarrelTexU(NULL), m_PixelProgram(NULL), m_EyePosition(0.0f, 0.09f, 0.15f), m_Scale(1.0f)
{
	struct stereo_config st_conf;

	++s_DeviceCounter;
	// For now, LibVR doesn't support multiple devices...
	m_DevicePtr = new CStereoLibVRDevicePtr();
	m_DevicePtr->HMDDevice = hmd_open_first(0);
	m_DevicePtr->InterpupillaryDistance = 0.0647; //TODO

	if (m_DevicePtr->HMDDevice)
	{
		hmd_get_display_info(m_DevicePtr->HMDDevice, &m_DevicePtr->HMDInfo);
		hmd_get_stereo_config(m_DevicePtr->HMDDevice, &st_conf);
		nldebug("LibVR: HScreenSize: %f, VScreenSize: %f", m_DevicePtr->HMDInfo.h_screen_size, m_DevicePtr->HMDInfo.v_screen_size);
		nldebug("LibVR: VScreenCenter: %f", m_DevicePtr->HMDInfo.v_center);
		nldebug("LibVR: EyeToScreenDistance: %f", m_DevicePtr->HMDInfo.eye_to_screen[0]);
		nldebug("LibVR: LensSeparationDistance: %f", m_DevicePtr->HMDInfo.lens_separation);
		nldebug("LibVR: HResolution: %i, VResolution: %i", m_DevicePtr->HMDInfo.h_resolution, m_DevicePtr->HMDInfo.v_resolution);
		nldebug("LibVR: DistortionK[0]: %f, DistortionK[1]: %f", m_DevicePtr->HMDInfo.distortion_k[0], m_DevicePtr->HMDInfo.distortion_k[1]);
		nldebug("LibVR: DistortionK[2]: %f, DistortionK[3]: %f", m_DevicePtr->HMDInfo.distortion_k[2], m_DevicePtr->HMDInfo.distortion_k[3]);
		nldebug("LibVR: Scale: %f", st_conf.distort.scale);
		m_LeftViewport.init(0.f, 0.f, 0.5f, 1.0f);
		m_RightViewport.init(0.5f, 0.f, 0.5f, 1.0f);
	}
}

CStereoLibVR::~CStereoLibVR()
{
	if (!m_BarrelMat.empty())
	{
		m_BarrelMat.getObjectPtr()->setTexture(0, NULL);
		m_Driver->deleteMaterial(m_BarrelMat);
	}
	delete m_BarrelTexU;
	m_BarrelTexU = NULL;
	m_BarrelTex = NULL; // CSmartPtr

	delete m_PixelProgram;
	m_PixelProgram = NULL;

	m_Driver = NULL;

	if (m_DevicePtr->HMDDevice)
		hmd_close(m_DevicePtr->HMDDevice);

	delete m_DevicePtr;
	m_DevicePtr = NULL;

	--s_DeviceCounter;
}

void CStereoLibVR::setDriver(NL3D::UDriver *driver)
{
	nlassert(!m_PixelProgram);

	NL3D::IDriver *drvInternal = (static_cast<CDriverUser *>(driver))->getDriver();	
	if (drvInternal->supportPixelProgram(CPixelProgram::fp40) && drvInternal->supportBloomEffect() && drvInternal->supportNonPowerOfTwoTextures())
	{
		nldebug("VR: fp40");
		m_PixelProgram = new CPixelProgram(g_StereoOVR_fp40);		
	}
	else if (drvInternal->supportPixelProgram(CPixelProgram::arbfp1) && drvInternal->supportBloomEffect() && drvInternal->supportNonPowerOfTwoTextures())
	{
		nldebug("VR: arbfp1");
		m_PixelProgram = new CPixelProgram(g_StereoOVR_arbfp1);		
	}
	else if (drvInternal->supportPixelProgram(CPixelProgram::ps_2_0))
	{
		nldebug("VR: ps_2_0");
		m_PixelProgram = new CPixelProgram(g_StereoOVR_ps_2_0);	
	}

	if (m_PixelProgram)
	{
		m_Driver = driver;

		m_BarrelTex = new CTextureBloom(); // lol bloom
		m_BarrelTex->setRenderTarget(true);
		m_BarrelTex->setReleasable(false);
		m_BarrelTex->resize(m_DevicePtr->HMDInfo.h_resolution, m_DevicePtr->HMDInfo.v_resolution);
		m_BarrelTex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
		m_BarrelTex->setWrapS(ITexture::Clamp);
		m_BarrelTex->setWrapT(ITexture::Clamp);
		drvInternal->setupTexture(*m_BarrelTex);
		m_BarrelTexU = new CTextureUser(m_BarrelTex);

		m_BarrelMat = m_Driver->createMaterial();
		m_BarrelMat.initUnlit();
		m_BarrelMat.setColor(CRGBA::White);
		m_BarrelMat.setBlend (false);
		m_BarrelMat.setAlphaTest (false);
		NL3D::CMaterial *barrelMat = m_BarrelMat.getObjectPtr();
		barrelMat->setShader(NL3D::CMaterial::PostProcessing);
		barrelMat->setBlendFunc(CMaterial::one, CMaterial::zero);
		barrelMat->setZWrite(false);
		barrelMat->setZFunc(CMaterial::always);
		barrelMat->setDoubleSided(true);
		barrelMat->setTexture(0, m_BarrelTex);

		m_BarrelQuadLeft.V0 = CVector(0.f, 0.f, 0.5f);
		m_BarrelQuadLeft.V1 = CVector(0.5f, 0.f, 0.5f);
		m_BarrelQuadLeft.V2 = CVector(0.5f, 1.f, 0.5f);
		m_BarrelQuadLeft.V3 = CVector(0.f, 1.f, 0.5f);

		m_BarrelQuadRight.V0 = CVector(0.5f, 0.f, 0.5f);
		m_BarrelQuadRight.V1 = CVector(1.f, 0.f, 0.5f);
		m_BarrelQuadRight.V2 = CVector(1.f, 1.f, 0.5f);
		m_BarrelQuadRight.V3 = CVector(0.5f, 1.f, 0.5f);

		nlassert(!drvInternal->isTextureRectangle(m_BarrelTex)); // not allowed

		m_BarrelQuadLeft.Uv0 = CUV(0.f,  0.f);
		m_BarrelQuadLeft.Uv1 = CUV(0.5f, 0.f);
		m_BarrelQuadLeft.Uv2 = CUV(0.5f, 1.f);
		m_BarrelQuadLeft.Uv3 = CUV(0.f,  1.f);

		m_BarrelQuadRight.Uv0 = CUV(0.5f,  0.f);
		m_BarrelQuadRight.Uv1 = CUV(1.f, 0.f);
		m_BarrelQuadRight.Uv2 = CUV(1.f, 1.f);
		m_BarrelQuadRight.Uv3 = CUV(0.5f, 1.f);
	}
	else
	{
		nlwarning("VR: No pixel program support");
	}
}

bool CStereoLibVR::getScreenResolution(uint &width, uint &height)
{
	width = m_DevicePtr->HMDInfo.h_resolution;
	height = m_DevicePtr->HMDInfo.v_resolution;
	return true;
}

void CStereoLibVR::initCamera(uint cid, const NL3D::UCamera *camera)
{
	struct stereo_config st_conf;
	hmd_get_stereo_config(m_DevicePtr->HMDDevice, &st_conf);

	float ar = st_conf.proj.aspect_ratio;
	float fov = st_conf.proj.yfov;
	m_LeftFrustum[cid].initPerspective(fov, ar, camera->getFrustum().Near, camera->getFrustum().Far);
	m_RightFrustum[cid] = m_LeftFrustum[cid];

	float projectionCenterOffset = st_conf.proj.projection_offset * 0.5 * (m_LeftFrustum[cid].Right - m_LeftFrustum[cid].Left);
	nldebug("LibVR: projectionCenterOffset = %f", projectionCenterOffset);

	m_LeftFrustum[cid].Left -= projectionCenterOffset;
	m_LeftFrustum[cid].Right -= projectionCenterOffset;
	m_RightFrustum[cid].Left += projectionCenterOffset;
	m_RightFrustum[cid].Right += projectionCenterOffset;

	// TODO: Clipping frustum should also take into account the IPD
	m_ClippingFrustum[cid] = m_LeftFrustum[cid];
	m_ClippingFrustum[cid].Left = min(m_LeftFrustum[cid].Left, m_RightFrustum[cid].Left);
	m_ClippingFrustum[cid].Right = max(m_LeftFrustum[cid].Right, m_RightFrustum[cid].Right);
}

/// Get the frustum to use for clipping
void CStereoLibVR::getClippingFrustum(uint cid, NL3D::UCamera *camera) const
{
	camera->setFrustum(m_ClippingFrustum[cid]);
}

void CStereoLibVR::updateCamera(uint cid, const NL3D::UCamera *camera)
{
	if (camera->getFrustum().Near != m_LeftFrustum[cid].Near
		|| camera->getFrustum().Far != m_LeftFrustum[cid].Far)
		CStereoLibVR::initCamera(cid, camera);
	m_CameraMatrix[cid] = camera->getMatrix();
}

bool CStereoLibVR::nextPass()
{
	// Do not allow weird stuff.
	uint32 width, height;
	m_Driver->getWindowSize(width, height);
	nlassert(width == m_DevicePtr->HMDInfo.h_resolution);
	nlassert(height == m_DevicePtr->HMDInfo.v_resolution);

	if (m_Driver->getPolygonMode() == UDriver::Filled)
	{
		switch (m_Stage)
		{
		case 0:
			++m_Stage;
			m_SubStage = 0;
			// stage 1:
			// (initBloom)
			// clear buffer
			// draw scene left
			return true;
		case 1:
			++m_Stage;
			m_SubStage = 0;
			// stage 2:
			// draw scene right
			return true;
		case 2:
			++m_Stage;
			m_SubStage = 0;
			// stage 3:
			// (endBloom)
			// draw interface 3d left
			return true;
		case 3:
			++m_Stage;
			m_SubStage = 0;
			// stage 4:
			// draw interface 3d right
			return true;
		case 4:
			++m_Stage;
			m_SubStage = 0;
			// stage 5:
			// (endInterfacesDisplayBloom)
			// draw interface 2d left
			return true;
		case 5:
			++m_Stage;
			m_SubStage = 0;
			// stage 6:
			// draw interface 2d right
			return true;
		case 6:
			m_Stage = 0;
			m_SubStage = 0;
			// present
			m_OrientationCached = false;
			return false;
		}
	}
	else
	{
		switch (m_Stage)
		{
		case 0:
			++m_Stage;
			m_SubStage = 0;
			return true;
		case 1:
			m_Stage = 0;
			m_SubStage = 0;
			return false;
		}
	}
	nlerror("Invalid stage");
	m_Stage = 0;
	m_SubStage = 0;
	m_OrientationCached = false;
	return false;
}

const NL3D::CViewport &CStereoLibVR::getCurrentViewport() const
{
	if (m_Stage % 2) return m_LeftViewport;
	else return m_RightViewport;
}

const NL3D::CFrustum &CStereoLibVR::getCurrentFrustum(uint cid) const
{
	if (m_Stage % 2) return m_LeftFrustum[cid];
	else return m_RightFrustum[cid];
}

void CStereoLibVR::getCurrentFrustum(uint cid, NL3D::UCamera *camera) const
{
	if (m_Stage % 2) camera->setFrustum(m_LeftFrustum[cid]);
	else camera->setFrustum(m_RightFrustum[cid]);
}

void CStereoLibVR::getCurrentMatrix(uint cid, NL3D::UCamera *camera) const
{
	CMatrix translate;
	if (m_Stage % 2) translate.translate(CVector((m_DevicePtr->InterpupillaryDistance * m_Scale) * -0.5f, 0.f, 0.f));
	else translate.translate(CVector((m_DevicePtr->InterpupillaryDistance * m_Scale) * 0.5f, 0.f, 0.f));
	CMatrix mat = m_CameraMatrix[cid] * translate;
	if (camera->getTransformMode() == NL3D::UTransformable::RotQuat)
	{
		camera->setPos(mat.getPos());
		camera->setRotQuat(mat.getRot());
	}
	else
	{
		// camera->setTransformMode(NL3D::UTransformable::DirectMatrix);
		camera->setMatrix(mat);
	}
}

bool CStereoLibVR::wantClear()
{
	switch (m_Stage)
	{
	case 1:
		m_SubStage = 1;
		return true;
	}
	return m_Driver->getPolygonMode() != UDriver::Filled;
}
	
bool CStereoLibVR::wantScene()
{
	switch (m_Stage)
	{
	case 1:
	case 2:
		m_SubStage = 2;
		return true;
	}
	return m_Driver->getPolygonMode() != UDriver::Filled;
}

bool CStereoLibVR::wantInterface3D()
{
	switch (m_Stage)
	{
	case 3:
	case 4:
		m_SubStage = 3;
		return true;
	}
	return m_Driver->getPolygonMode() != UDriver::Filled;
}

bool CStereoLibVR::wantInterface2D()
{
	switch (m_Stage)
	{
	case 5:
	case 6:
		m_SubStage = 4;
		return true;
	}
	return m_Driver->getPolygonMode() != UDriver::Filled;
}


/// Returns non-NULL if a new render target was set
bool CStereoLibVR::beginRenderTarget()
{
	// render target always set before driver clear
	// nlassert(m_SubStage <= 1);
	if (m_Driver && m_Stage == 1 && (m_Driver->getPolygonMode() == UDriver::Filled))
	{
		static_cast<CDriverUser *>(m_Driver)->setRenderTarget(*m_BarrelTexU, 0, 0, 0, 0);
		return true;
	}
	return false;
}

/// Returns true if a render target was fully drawn
bool CStereoLibVR::endRenderTarget()
{
	// after rendering of course
	// nlassert(m_SubStage > 1);
	if (m_Driver && m_Stage == 6 && (m_Driver->getPolygonMode() == UDriver::Filled)) // set to 4 to turn off distortion of 2d gui
	{
		struct stereo_config st_conf;
		hmd_get_stereo_config(m_DevicePtr->HMDDevice, &st_conf);
		CTextureUser cu;
		(static_cast<CDriverUser *>(m_Driver))->setRenderTarget(cu);
		bool fogEnabled = m_Driver->fogEnabled();
		m_Driver->enableFog(false);

		m_Driver->setMatrixMode2D11();
		CViewport vp = CViewport();
		m_Driver->setViewport(vp);
		uint32 width, height;
		m_Driver->getWindowSize(width, height);
		NL3D::IDriver *drvInternal = (static_cast<CDriverUser *>(m_Driver))->getDriver();
		NL3D::CMaterial *barrelMat = m_BarrelMat.getObjectPtr();
		barrelMat->setTexture(0, m_BarrelTex);
		drvInternal->activePixelProgram(m_PixelProgram);

		float w = float(m_BarrelQuadLeft.V1.x),// / float(width),
			h = float(m_BarrelQuadLeft.V2.y),// / float(height),
			x = float(m_BarrelQuadLeft.V0.x),/// / float(width),
			y = float(m_BarrelQuadLeft.V0.y);// / float(height);

		//TODO: stereo_config stuff
		float lensViewportShift = st_conf.proj.projection_offset;

		float lensCenterX = x + (w + lensViewportShift * 0.5f) * 0.5f;
		float lensCenterY = y + h * 0.5f;
		float screenCenterX = x + w * 0.5f;
		float screenCenterY = y + h * 0.5f;
		float scaleX = (w / 2 / st_conf.distort.scale);
		float scaleY = (h / 2 / st_conf.distort.scale);
		float scaleInX = (2 / w);
		float scaleInY = (2 / h);
		drvInternal->setPixelProgramConstant(0, lensCenterX, lensCenterY, 0.f, 0.f);
		drvInternal->setPixelProgramConstant(1, screenCenterX, screenCenterY, 0.f, 0.f);
		drvInternal->setPixelProgramConstant(2, scaleX, scaleY, 0.f, 0.f);
		drvInternal->setPixelProgramConstant(3, scaleInX, scaleInY, 0.f, 0.f);
		drvInternal->setPixelProgramConstant(4, 1, st_conf.distort.distortion_k);


		m_Driver->drawQuad(m_BarrelQuadLeft, m_BarrelMat);

		x = w;
		lensCenterX = x + (w - lensViewportShift * 0.5f) * 0.5f;
		screenCenterX = x + w * 0.5f;
		drvInternal->setPixelProgramConstant(0, lensCenterX, lensCenterY, 0.f, 0.f);
		drvInternal->setPixelProgramConstant(1, screenCenterX, screenCenterY, 0.f, 0.f);

		m_Driver->drawQuad(m_BarrelQuadRight, m_BarrelMat);

		drvInternal->activePixelProgram(NULL);
		m_Driver->enableFog(fogEnabled);

		return true;
	}
	return false;
}

NLMISC::CQuat CStereoLibVR::getOrientation() const
{
	if (m_OrientationCached)
		return m_OrientationCache;

	unsigned int t = NLMISC::CTime::getLocalTime();
	hmd_update(m_DevicePtr->HMDDevice, &t);

	float quat[4];
	hmd_get_rotation(m_DevicePtr->HMDDevice, quat);
	NLMISC::CMatrix coordsys;
	float csys[] = {
		1.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, -1.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f, 
	};
	coordsys.set(csys);
	NLMISC::CMatrix matovr;
	matovr.setRot(NLMISC::CQuat(quat[1], quat[2], quat[3], quat[0]));
	NLMISC::CMatrix matr;
	matr.rotateX(NLMISC::Pi * 0.5f); // fix this properly... :) (note: removing this allows you to use rift while lying down)
	NLMISC::CMatrix matnel = matr * matovr * coordsys;
	NLMISC::CQuat finalquat = matnel.getRot();
	m_OrientationCache = finalquat;
	m_OrientationCached = true;
	return finalquat;
}

/// Get GUI shift
void CStereoLibVR::getInterface2DShift(uint cid, float &x, float &y, float distance) const
{
#if 0

	// todo: take into account m_EyePosition

	NLMISC::CVector vector = CVector(0.f, -distance, 0.f);
	NLMISC::CQuat rot = getOrientation();
	rot.invert();
	NLMISC::CMatrix mat;
	mat.rotate(rot);
	//if (m_Stage % 2) mat.translate(CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * -0.5f, 0.f, 0.f));
	//else mat.translate(CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * 0.5f, 0.f, 0.f));
	mat.translate(vector);
	CVector proj = CStereoOVR::getCurrentFrustum(cid).project(mat.getPos());

	NLMISC::CVector ipd;
	if (m_Stage % 2) ipd = CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * -0.5f, 0.f, 0.f);
	else ipd = CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * 0.5f, 0.f, 0.f);
	CVector projipd = CStereoOVR::getCurrentFrustum(cid).project(vector + ipd);
	CVector projvec = CStereoOVR::getCurrentFrustum(cid).project(vector);

	x = (proj.x + projipd.x - projvec.x - 0.5f);
	y = (proj.y + projipd.y - projvec.y - 0.5f);

#elif 1

	// Alternative method
	// todo: take into account m_EyePosition

	NLMISC::CVector vec = CVector(0.f, -distance, 0.f);
	NLMISC::CVector ipd;
	if (m_Stage % 2) ipd = CVector((m_DevicePtr->InterpupillaryDistance * m_Scale) * -0.5f, 0.f, 0.f);
	else ipd = CVector((m_DevicePtr->InterpupillaryDistance * m_Scale) * 0.5f, 0.f, 0.f);
	

	NLMISC::CQuat rot = getOrientation();
	NLMISC::CQuat modrot = NLMISC::CQuat(CVector(0.f, 1.f, 0.f), NLMISC::Pi);
	rot = rot * modrot;
	float p = NLMISC::Pi + atan2f(2.0f * ((rot.x * rot.y) + (rot.z * rot.w)), 1.0f - 2.0f * ((rot.y * rot.y) + (rot.w * rot.w)));
	if (p > NLMISC::Pi) p -= NLMISC::Pi * 2.0f;	
	float t = -atan2f(2.0f * ((rot.x * rot.w) + (rot.y * rot.z)), 1.0f - 2.0f * ((rot.z * rot.z) + (rot.w * rot.w)));// // asinf(2.0f * ((rot.x * rot.z) - (rot.w * rot.y)));
	
	CVector rotshift = CVector(p, 0.f, t) * -distance;
	
	CVector proj = CStereoLibVR::getCurrentFrustum(cid).project(vec + ipd + rotshift);

	x = (proj.x - 0.5f);
	y = (proj.y - 0.5f);

#endif
}

void CStereoLibVR::setEyePosition(const NLMISC::CVector &v)
{
	m_EyePosition = v;
}

const NLMISC::CVector &CStereoLibVR::getEyePosition() const
{
	return m_EyePosition;
}

void CStereoLibVR::setScale(float s)
{
	m_EyePosition = m_EyePosition * (s / m_Scale);
	m_Scale = s;
}

void CStereoLibVR::listDevices(std::vector<CStereoDeviceInfo> &devicesOut)
{
	// For now, LibVR doesn't support multiple devices
	struct hmd *hmd = hmd_open_first(0);
	if (hmd)
	{
                CStereoDeviceInfo deviceInfoOut;
		CStereoLibVRDeviceHandle *handle = new CStereoLibVRDeviceHandle();
		deviceInfoOut.Factory = static_cast<IStereoDeviceFactory *>(handle);
		deviceInfoOut.Class = CStereoDeviceInfo::StereoHMD;
		deviceInfoOut.Library = CStereoDeviceInfo::LibVR;
		deviceInfoOut.AllowAuto = true;
		//TODO: manufacturer, produc name
		//TODO: serial
		devicesOut.push_back(deviceInfoOut);
		hmd_close(hmd);
	}
}

bool CStereoLibVR::isLibraryInUse()
{
	nlassert(s_DeviceCounter >= 0);
	return s_DeviceCounter > 0;
}

void CStereoLibVR::releaseLibrary()
{
	nlassert(s_DeviceCounter == 0);
}

bool CStereoLibVR::isDeviceCreated()
{
	return m_DevicePtr->HMDDevice != NULL;
}

} /* namespace NL3D */

#endif /* HAVE_LIBVR */

/* end of file */
