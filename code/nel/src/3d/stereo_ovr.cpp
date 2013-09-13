/**
 * \file stereo_ovr.cpp
 * \brief CStereoOVR
 * \date 2013-06-25 22:22GMT
 * \author Jan Boon (Kaetemi)
 * CStereoOVR
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
 * 
 * Linking this library statically or dynamically with other modules
 * is making a combined work based on this library.  Thus, the terms
 * and conditions of the GNU General Public License cover the whole
 * combination.
 * 
 * As a special exception, the copyright holders of this library give
 * you permission to link this library with the Oculus SDK to produce
 * an executable, regardless of the license terms of the Oculus SDK,
 * and distribute linked combinations including the two, provided that
 * you also meet the terms and conditions of the license of the Oculus
 * SDK.  You must obey the GNU General Public License in all respects
 * for all of the code used other than the Oculus SDK.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to do
 * so, delete this exception statement from your version.
 */

#ifdef HAVE_LIBOVR

#include <nel/misc/types_nl.h>
#include <nel/3d/stereo_ovr.h>

// STL includes
#include <sstream>

// External includes
#include <OVR.h>

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

extern const char *g_StereoOVR_fp40;
extern const char *g_StereoOVR_arbfp1;
extern const char *g_StereoOVR_ps_2_0;
extern const char *g_StereoOVR_glsl330f;

namespace {

class CStereoOVRLog : public OVR::Log
{
public:
	CStereoOVRLog(unsigned logMask = OVR::LogMask_All) : OVR::Log(logMask)
	{

	}

	virtual void LogMessageVarg(OVR::LogMessageType messageType, const char* fmt, va_list argList)
	{
		if (NLMISC::INelContext::isContextInitialised())
		{
			char buffer[MaxLogBufferMessageSize];
			FormatLog(buffer, MaxLogBufferMessageSize, messageType, fmt, argList);
			if (IsDebugMessage(messageType))
				NLMISC::INelContext::getInstance().getDebugLog()->displayNL("OVR: %s", buffer);
			else
				NLMISC::INelContext::getInstance().getInfoLog()->displayNL("OVR: %s", buffer);
		}
	}
};

CStereoOVRLog *s_StereoOVRLog = NULL;
OVR::Ptr<OVR::DeviceManager> s_DeviceManager;

class CStereoOVRSystem
{
public:
	~CStereoOVRSystem()
	{
		Release();
	}

	void Init()
	{
		if (!s_StereoOVRLog)
		{
			nldebug("Initialize OVR");
			s_StereoOVRLog = new CStereoOVRLog();
		}
		if (!OVR::System::IsInitialized())
			OVR::System::Init(s_StereoOVRLog);
		if (!s_DeviceManager)
			s_DeviceManager = OVR::DeviceManager::Create();
	}

	void Release()
	{
		if (s_DeviceManager)
		{
			nldebug("Release OVR");
			s_DeviceManager->Release();
		}
		s_DeviceManager.Clear();
		if (OVR::System::IsInitialized())
			OVR::System::Destroy();
		if (s_StereoOVRLog)
			nldebug("Release OVR Ok");
		delete s_StereoOVRLog;
		s_StereoOVRLog = NULL;
	}
};

CStereoOVRSystem s_StereoOVRSystem;

sint s_DeviceCounter = 0;

}

class CStereoOVRDeviceHandle : public IStereoDeviceFactory
{
public:
	// fixme: virtual destructor???
	OVR::DeviceEnumerator<OVR::HMDDevice> DeviceHandle;
	IStereoDisplay *createDevice() const
	{
		CStereoOVR *stereo = new CStereoOVR(this);
		if (stereo->isDeviceCreated())
			return stereo;
		delete stereo;
		return NULL;
	}
};

class CStereoOVRDevicePtr
{
public:
	OVR::Ptr<OVR::HMDDevice> HMDDevice;
	OVR::Ptr<OVR::SensorDevice> SensorDevice;
	OVR::SensorFusion SensorFusion;
	OVR::HMDInfo HMDInfo;
};

CStereoOVR::CStereoOVR(const CStereoOVRDeviceHandle *handle) : m_Stage(0), m_SubStage(0), m_OrientationCached(false), m_Driver(NULL), m_BarrelTexU(NULL), m_PixelProgram(NULL), m_EyePosition(0.0f, 0.09f, 0.15f), m_Scale(1.0f)
{
	++s_DeviceCounter;
	m_DevicePtr = new CStereoOVRDevicePtr();

	OVR::DeviceEnumerator<OVR::HMDDevice> dh = handle->DeviceHandle;
	m_DevicePtr->HMDDevice = dh.CreateDevice();

	if (m_DevicePtr->HMDDevice)
	{
		m_DevicePtr->HMDDevice->GetDeviceInfo(&m_DevicePtr->HMDInfo);
		nldebug("OVR: HScreenSize: %f, VScreenSize: %f", m_DevicePtr->HMDInfo.HScreenSize, m_DevicePtr->HMDInfo.VScreenSize);
		nldebug("OVR: VScreenCenter: %f", m_DevicePtr->HMDInfo.VScreenCenter);
		nldebug("OVR: EyeToScreenDistance: %f", m_DevicePtr->HMDInfo.EyeToScreenDistance);
		nldebug("OVR: LensSeparationDistance: %f", m_DevicePtr->HMDInfo.LensSeparationDistance);
		nldebug("OVR: InterpupillaryDistance: %f", m_DevicePtr->HMDInfo.InterpupillaryDistance);
		nldebug("OVR: HResolution: %i, VResolution: %i", m_DevicePtr->HMDInfo.HResolution, m_DevicePtr->HMDInfo.VResolution);
		nldebug("OVR: DistortionK[0]: %f, DistortionK[1]: %f", m_DevicePtr->HMDInfo.DistortionK[0], m_DevicePtr->HMDInfo.DistortionK[1]);
		nldebug("OVR: DistortionK[2]: %f, DistortionK[3]: %f", m_DevicePtr->HMDInfo.DistortionK[2], m_DevicePtr->HMDInfo.DistortionK[3]);
		//2013/06/26 05:31:51 DBG 17a0 snowballs_client.exe stereo_ovr.cpp 160 NL3D::CStereoOVR::CStereoOVR : OVR: HScreenSize: 0.149760, VScreenSize: 0.093600
		//2013/06/26 05:31:51 DBG 17a0 snowballs_client.exe stereo_ovr.cpp 161 NL3D::CStereoOVR::CStereoOVR : OVR: VScreenCenter: 0.046800
		//2013/06/26 05:31:51 DBG 17a0 snowballs_client.exe stereo_ovr.cpp 162 NL3D::CStereoOVR::CStereoOVR : OVR: EyeToScreenDistance: 0.041000
		//2013/06/26 05:31:51 DBG 17a0 snowballs_client.exe stereo_ovr.cpp 163 NL3D::CStereoOVR::CStereoOVR : OVR: LensSeparationDistance: 0.063500
		//2013/06/26 05:31:51 DBG 17a0 snowballs_client.exe stereo_ovr.cpp 164 NL3D::CStereoOVR::CStereoOVR : OVR: InterpupillaryDistance: 0.064000
		//2013/06/26 05:31:51 DBG 17a0 snowballs_client.exe stereo_ovr.cpp 165 NL3D::CStereoOVR::CStereoOVR : OVR: HResolution: 1280, VResolution: 800
		//2013/06/26 05:31:51 DBG 17a0 snowballs_client.exe stereo_ovr.cpp 166 NL3D::CStereoOVR::CStereoOVR : OVR: DistortionK[0]: 1.000000, DistortionK[1]: 0.220000
		//2013/06/26 05:31:51 DBG 17a0 snowballs_client.exe stereo_ovr.cpp 167 NL3D::CStereoOVR::CStereoOVR : OVR: DistortionK[2]: 0.240000, DistortionK[3]: 0.000000
		m_DevicePtr->SensorDevice = m_DevicePtr->HMDDevice->GetSensor();
		m_DevicePtr->SensorFusion.AttachToSensor(m_DevicePtr->SensorDevice);
		m_DevicePtr->SensorFusion.SetGravityEnabled(true);
		m_DevicePtr->SensorFusion.SetPredictionEnabled(true);
		m_DevicePtr->SensorFusion.SetYawCorrectionEnabled(true);
		m_LeftViewport.init(0.f, 0.f, 0.5f, 1.0f);
		m_RightViewport.init(0.5f, 0.f, 0.5f, 1.0f);
	}
}

CStereoOVR::~CStereoOVR()
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

	if (m_DevicePtr->SensorDevice)
		m_DevicePtr->SensorDevice->Release();
	m_DevicePtr->SensorDevice.Clear();
	if (m_DevicePtr->HMDDevice)
		m_DevicePtr->HMDDevice->Release();
	m_DevicePtr->HMDDevice.Clear();

	delete m_DevicePtr;
	m_DevicePtr = NULL;
	--s_DeviceCounter;
}

class CPixelProgramOVR : public CPixelProgram
{
public:
	struct COVRIndices
	{
		uint LensCenter;
		uint ScreenCenter;
		uint Scale;
		uint ScaleIn;
		uint HmdWarpParam;
	};

	CPixelProgramOVR()
	{
		{
			CSource *source = new CSource();
			source->Profile = glsl330f;
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->setSourcePtr(g_StereoOVR_glsl330f);
			addSource(source);
		}
		{
			CSource *source = new CSource();
			source->Profile = fp40;
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->setSourcePtr(g_StereoOVR_fp40);
			source->ParamIndices["cLensCenter"] = 0;
			source->ParamIndices["cScreenCenter"] = 1;
			source->ParamIndices["cScale"] = 2;
			source->ParamIndices["cScaleIn"] = 3;
			source->ParamIndices["cHmdWarpParam"] = 4;
			addSource(source);
		}
		{
			CSource *source = new CSource();
			source->Profile = arbfp1;
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->setSourcePtr(g_StereoOVR_arbfp1);
			source->ParamIndices["cLensCenter"] = 0;
			source->ParamIndices["cScreenCenter"] = 1;
			source->ParamIndices["cScale"] = 2;
			source->ParamIndices["cScaleIn"] = 3;
			source->ParamIndices["cHmdWarpParam"] = 4;
			addSource(source);
		}
		{
			CSource *source = new CSource();
			source->Profile = ps_2_0;
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->setSourcePtr(g_StereoOVR_ps_2_0);
			source->ParamIndices["cLensCenter"] = 0;
			source->ParamIndices["cScreenCenter"] = 1;
			source->ParamIndices["cScale"] = 2;
			source->ParamIndices["cScaleIn"] = 3;
			source->ParamIndices["cHmdWarpParam"] = 4;
			addSource(source);
		}
	}

	virtual ~CPixelProgramOVR()
	{
		
	}

	virtual void buildInfo()
	{
		CPixelProgram::buildInfo();

		m_OVRIndices.LensCenter = getUniformIndex("cLensCenter");
		nlassert(m_OVRIndices.LensCenter != ~0);
		m_OVRIndices.ScreenCenter = getUniformIndex("cScreenCenter");
		nlassert(m_OVRIndices.ScreenCenter != ~0);
		m_OVRIndices.Scale = getUniformIndex("cScale");
		nlassert(m_OVRIndices.Scale != ~0);
		m_OVRIndices.ScaleIn = getUniformIndex("cScaleIn");
		nlassert(m_OVRIndices.ScaleIn != ~0);
		m_OVRIndices.HmdWarpParam = getUniformIndex("cHmdWarpParam");
		nlassert(m_OVRIndices.HmdWarpParam != ~0);
	}

	inline const COVRIndices &ovrIndices() { return m_OVRIndices; }

private:
	COVRIndices m_OVRIndices;

};

void CStereoOVR::setDriver(NL3D::UDriver *driver)
{
	nlassert(!m_PixelProgram);

	NL3D::IDriver *drvInternal = (static_cast<CDriverUser *>(driver))->getDriver();

	if (drvInternal->supportBloomEffect() && drvInternal->supportNonPowerOfTwoTextures())
	{
		m_PixelProgram = new CPixelProgramOVR();
		if (!drvInternal->compilePixelProgram(m_PixelProgram))
		{
			m_PixelProgram.kill();
		}
	}

	if (m_PixelProgram)
	{
		m_Driver = driver;

		m_BarrelTex = new CTextureBloom(); // lol bloom
		m_BarrelTex->setRenderTarget(true);
		m_BarrelTex->setReleasable(false);
		m_BarrelTex->resize(m_DevicePtr->HMDInfo.HResolution, m_DevicePtr->HMDInfo.VResolution);
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
		barrelMat->setShader(NL3D::CMaterial::Normal);
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

bool CStereoOVR::getScreenResolution(uint &width, uint &height)
{
	width = m_DevicePtr->HMDInfo.HResolution;
	height = m_DevicePtr->HMDInfo.VResolution;
	return true;
}

void CStereoOVR::initCamera(uint cid, const NL3D::UCamera *camera)
{
	float ar = (float)m_DevicePtr->HMDInfo.HResolution / ((float)m_DevicePtr->HMDInfo.VResolution * 2.0f);
	float fov = 2.0f * atanf((m_DevicePtr->HMDInfo.HScreenSize * 0.5f * 0.5f) / (m_DevicePtr->HMDInfo.EyeToScreenDistance)); //(float)NLMISC::Pi/2.f; // 2.0f * atanf(m_DevicePtr->HMDInfo.VScreenSize / 2.0f * m_DevicePtr->HMDInfo.EyeToScreenDistance);
	m_LeftFrustum[cid].initPerspective(fov, ar, camera->getFrustum().Near, camera->getFrustum().Far);
	m_RightFrustum[cid] = m_LeftFrustum[cid];
	
	float viewCenter = m_DevicePtr->HMDInfo.HScreenSize * 0.25f;
	float eyeProjectionShift = viewCenter - m_DevicePtr->HMDInfo.LensSeparationDistance * 0.5f; // docs say LensSeparationDistance, why not InterpupillaryDistance? related to how the lenses work?
	float projectionCenterOffset = (eyeProjectionShift / (m_DevicePtr->HMDInfo.HScreenSize * 0.5f)) * (m_LeftFrustum[cid].Right - m_LeftFrustum[cid].Left); // used logic for this one, but it ends up being the same as the one i made up
	nldebug("OVR: projectionCenterOffset = %f", projectionCenterOffset);

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
void CStereoOVR::getClippingFrustum(uint cid, NL3D::UCamera *camera) const
{
	camera->setFrustum(m_ClippingFrustum[cid]);
}

void CStereoOVR::updateCamera(uint cid, const NL3D::UCamera *camera)
{
	if (camera->getFrustum().Near != m_LeftFrustum[cid].Near
		|| camera->getFrustum().Far != m_LeftFrustum[cid].Far)
		CStereoOVR::initCamera(cid, camera);
	m_CameraMatrix[cid] = camera->getMatrix();
}

bool CStereoOVR::nextPass()
{
	// Do not allow weird stuff.
	uint32 width, height;
	m_Driver->getWindowSize(width, height);
	nlassert(width == m_DevicePtr->HMDInfo.HResolution);
	nlassert(height == m_DevicePtr->HMDInfo.VResolution);

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

const NL3D::CViewport &CStereoOVR::getCurrentViewport() const
{
	if (m_Stage % 2) return m_LeftViewport;
	else return m_RightViewport;
}

const NL3D::CFrustum &CStereoOVR::getCurrentFrustum(uint cid) const
{
	if (m_Stage % 2) return m_LeftFrustum[cid];
	else return m_RightFrustum[cid];
}

void CStereoOVR::getCurrentFrustum(uint cid, NL3D::UCamera *camera) const
{
	if (m_Stage % 2) camera->setFrustum(m_LeftFrustum[cid]);
	else camera->setFrustum(m_RightFrustum[cid]);
}

void CStereoOVR::getCurrentMatrix(uint cid, NL3D::UCamera *camera) const
{
	CMatrix translate;
	if (m_Stage % 2) translate.translate(CVector((m_DevicePtr->HMDInfo.InterpupillaryDistance * m_Scale) * -0.5f, 0.f, 0.f));
	else translate.translate(CVector((m_DevicePtr->HMDInfo.InterpupillaryDistance * m_Scale) * 0.5f, 0.f, 0.f));
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

bool CStereoOVR::wantClear()
{
	switch (m_Stage)
	{
	case 1:
		m_SubStage = 1;
		return true;
	}
	return m_Driver->getPolygonMode() != UDriver::Filled;
}
	
bool CStereoOVR::wantScene()
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

bool CStereoOVR::wantInterface3D()
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

bool CStereoOVR::wantInterface2D()
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
bool CStereoOVR::beginRenderTarget()
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
bool CStereoOVR::endRenderTarget()
{
	// after rendering of course
	// nlassert(m_SubStage > 1);
	if (m_Driver && m_Stage == 6 && (m_Driver->getPolygonMode() == UDriver::Filled)) // set to 4 to turn off distortion of 2d gui
	{
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

		float lensOffset = m_DevicePtr->HMDInfo.LensSeparationDistance * 0.5f;
		float lensShift = m_DevicePtr->HMDInfo.HScreenSize * 0.25f - lensOffset;
		float lensViewportShift = 4.0f * lensShift / m_DevicePtr->HMDInfo.HScreenSize;

		float lensCenterX = x + (w + lensViewportShift * 0.5f) * 0.5f;
		float lensCenterY = y + h * 0.5f;
		float screenCenterX = x + w * 0.5f;
		float screenCenterY = y + h * 0.5f;
		float scaleX = (w / 2);
		float scaleY = (h / 2);
		float scaleInX = (2 / w);
		float scaleInY = (2 / h);
		

		drvInternal->setUniform2f(IDriver::PixelProgram, 
			m_PixelProgram->ovrIndices().LensCenter, 
			lensCenterX, lensCenterY);

		drvInternal->setUniform2f(IDriver::PixelProgram, 
			m_PixelProgram->ovrIndices().ScreenCenter, 
			screenCenterX, screenCenterY);

		drvInternal->setUniform2f(IDriver::PixelProgram, 
			m_PixelProgram->ovrIndices().Scale, 
			scaleX, scaleY);

		drvInternal->setUniform2f(IDriver::PixelProgram, 
			m_PixelProgram->ovrIndices().ScaleIn, 
			scaleInX, scaleInY);


		drvInternal->setUniform4fv(IDriver::PixelProgram, 
			m_PixelProgram->ovrIndices().HmdWarpParam, 
			1, m_DevicePtr->HMDInfo.DistortionK);

		m_Driver->drawQuad(m_BarrelQuadLeft, m_BarrelMat);

		x = w;
		lensCenterX = x + (w - lensViewportShift * 0.5f) * 0.5f;
		screenCenterX = x + w * 0.5f;


		drvInternal->setUniform2f(IDriver::PixelProgram, 
			m_PixelProgram->ovrIndices().LensCenter, 
			lensCenterX, lensCenterY);

		drvInternal->setUniform2f(IDriver::PixelProgram, 
			m_PixelProgram->ovrIndices().ScreenCenter, 
			screenCenterX, screenCenterY);


		m_Driver->drawQuad(m_BarrelQuadRight, m_BarrelMat);

		drvInternal->activePixelProgram(NULL);
		m_Driver->enableFog(fogEnabled);

		return true;
	}
	return false;
}

NLMISC::CQuat CStereoOVR::getOrientation() const
{
	if (m_OrientationCached)
		return m_OrientationCache;

	OVR::Quatf quatovr = m_DevicePtr->SensorFusion.GetPredictedOrientation();
	NLMISC::CMatrix coordsys;
	float csys[] = {
		1.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, -1.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f, 
	};
	coordsys.set(csys);
	NLMISC::CMatrix matovr;
	matovr.setRot(NLMISC::CQuat(quatovr.x, quatovr.y, quatovr.z, quatovr.w));
	NLMISC::CMatrix matr;
	matr.rotateX(NLMISC::Pi * 0.5f); // fix this properly... :) (note: removing this allows you to use rift while lying down)
	NLMISC::CMatrix matnel = matr * matovr * coordsys;
	NLMISC::CQuat finalquat = matnel.getRot();
	m_OrientationCache = finalquat;
	m_OrientationCached = true;
	return finalquat;
}

/// Get GUI shift
void CStereoOVR::getInterface2DShift(uint cid, float &x, float &y, float distance) const
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
	if (m_Stage % 2) ipd = CVector((m_DevicePtr->HMDInfo.InterpupillaryDistance * m_Scale) * -0.5f, 0.f, 0.f);
	else ipd = CVector((m_DevicePtr->HMDInfo.InterpupillaryDistance * m_Scale) * 0.5f, 0.f, 0.f);
	

	NLMISC::CQuat rot = getOrientation();
	NLMISC::CQuat modrot = NLMISC::CQuat(CVector(0.f, 1.f, 0.f), NLMISC::Pi);
	rot = rot * modrot;
	float p = NLMISC::Pi + atan2f(2.0f * ((rot.x * rot.y) + (rot.z * rot.w)), 1.0f - 2.0f * ((rot.y * rot.y) + (rot.w * rot.w)));
	if (p > NLMISC::Pi) p -= NLMISC::Pi * 2.0f;	
	float t = -atan2f(2.0f * ((rot.x * rot.w) + (rot.y * rot.z)), 1.0f - 2.0f * ((rot.z * rot.z) + (rot.w * rot.w)));// // asinf(2.0f * ((rot.x * rot.z) - (rot.w * rot.y)));
	
	CVector rotshift = CVector(p, 0.f, t) * -distance;
	
	CVector proj = CStereoOVR::getCurrentFrustum(cid).project(vec + ipd + rotshift);

	x = (proj.x - 0.5f);
	y = (proj.y - 0.5f);

#endif
}

void CStereoOVR::setEyePosition(const NLMISC::CVector &v)
{
	m_EyePosition = v;
}

const NLMISC::CVector &CStereoOVR::getEyePosition() const
{
	return m_EyePosition;
}

void CStereoOVR::setScale(float s)
{
	m_EyePosition = m_EyePosition * (s / m_Scale);
	m_Scale = s;
}

void CStereoOVR::listDevices(std::vector<CStereoDeviceInfo> &devicesOut)
{
	s_StereoOVRSystem.Init();
	OVR::DeviceEnumerator<OVR::HMDDevice> devices = s_DeviceManager->EnumerateDevices<OVR::HMDDevice>();
	uint id = 1;
	do
	{
		CStereoDeviceInfo deviceInfoOut;
		OVR::DeviceInfo deviceInfo;
		if (devices.IsAvailable())
		{
			devices.GetDeviceInfo(&deviceInfo);
			CStereoOVRDeviceHandle *handle = new CStereoOVRDeviceHandle();
			deviceInfoOut.Factory = static_cast<IStereoDeviceFactory *>(handle);
			handle->DeviceHandle = devices;
			deviceInfoOut.Class = CStereoDeviceInfo::StereoHMD; // 1; // OVR::HMDDevice
			deviceInfoOut.Library = CStereoDeviceInfo::OVR; // "Oculus SDK";
			deviceInfoOut.Manufacturer = deviceInfo.Manufacturer;
			deviceInfoOut.ProductName = deviceInfo.ProductName;
			stringstream ser;
			ser << id;
			deviceInfoOut.Serial = ser.str(); // can't get the real serial from the sdk...
			devicesOut.push_back(deviceInfoOut);
			++id;
		}

	} while (devices.Next());
}

bool CStereoOVR::isLibraryInUse()
{
	nlassert(s_DeviceCounter >= 0);
	return s_DeviceCounter > 0;
}

void CStereoOVR::releaseLibrary()
{
	nlassert(s_DeviceCounter == 0);
	s_StereoOVRSystem.Release();
}

bool CStereoOVR::isDeviceCreated()
{
	return m_DevicePtr->HMDDevice != NULL;
}

} /* namespace NL3D */

#endif /* HAVE_LIBOVR */

/* end of file */
