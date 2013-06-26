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

#include <nel/misc/types_nl.h>
#include <nel/3d/stereo_ovr.h>

// STL includes

// External includes
#include <OVR.h>

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/3d/u_camera.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace NL3D {

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

class CStereoOVRDeviceHandle : public NLMISC::CRefCount
{
public:
	OVR::DeviceEnumerator<OVR::HMDDevice> DeviceHandle;
};

sint s_DeviceCounter = 0;

}

class CStereoOVRDevicePtr
{
public:
	OVR::Ptr<OVR::HMDDevice> HMDDevice;
	OVR::Ptr<OVR::SensorDevice> SensorDevice;
	OVR::SensorFusion SensorFusion;
	OVR::HMDInfo HMDInfo;
};

CStereoOVR::CStereoOVR(const CStereoDeviceInfo &deviceInfo) : m_Stage(0), m_OrientationCached(false)
{
	++s_DeviceCounter;
	m_DevicePtr = new CStereoOVRDevicePtr();

	CStereoOVRDeviceHandle *handle = static_cast<CStereoOVRDeviceHandle *>(deviceInfo.Factory.getPtr());
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

void CStereoOVR::getScreenResolution(uint &width, uint &height)
{
	width = m_DevicePtr->HMDInfo.HResolution;
	height = m_DevicePtr->HMDInfo.VResolution;
}

void CStereoOVR::initCamera(const NL3D::UCamera *camera)
{
	float ar = (float)m_DevicePtr->HMDInfo.HResolution / ((float)m_DevicePtr->HMDInfo.VResolution * 2.0f);
	float fov = 2.0f * atanf((m_DevicePtr->HMDInfo.VScreenSize / 2.0f) / m_DevicePtr->HMDInfo.EyeToScreenDistance); //(float)NLMISC::Pi/2.f; // 2.0f * atanf(m_DevicePtr->HMDInfo.VScreenSize / 2.0f * m_DevicePtr->HMDInfo.EyeToScreenDistance);
	m_LeftFrustum.initPerspective(fov, ar, camera->getFrustum().Near, camera->getFrustum().Far);
	m_RightFrustum = m_LeftFrustum;
	float viewCenter = m_DevicePtr->HMDInfo.HScreenSize * 0.25f;
	float eyeProjectionShift = viewCenter - m_DevicePtr->HMDInfo.LensSeparationDistance * 0.5f;
	float projectionCenterOffset = 4.0f * eyeProjectionShift / m_DevicePtr->HMDInfo.HScreenSize;
	nldebug("OVR: projectionCenterOffset = %f", projectionCenterOffset);
	projectionCenterOffset *= (m_LeftFrustum.Left - m_LeftFrustum.Right) * 0.5f; // made this up ...
	m_LeftFrustum.Left += projectionCenterOffset;
	m_LeftFrustum.Right += projectionCenterOffset;
	m_RightFrustum.Left -= projectionCenterOffset;
	m_RightFrustum.Right -= projectionCenterOffset;
}

void CStereoOVR::updateCamera(const NL3D::UCamera *camera)
{
	if (camera->getFrustum().Near != m_LeftFrustum.Near
		|| camera->getFrustum().Far != m_LeftFrustum.Far)
		CStereoOVR::initCamera(camera);
	m_CameraMatrix = camera->getMatrix();
}

bool CStereoOVR::nextPass()
{
	switch (m_Stage)
	{
	case 0:
		++m_Stage;
		// stage 1:
		// (initBloom)
		// clear buffer
		// draw scene left
		return true;
	case 1:
		++m_Stage;
		// stage 2:
		// draw scene right
		return true;
	case 2:
		++m_Stage;
		// stage 3:
		// (endBloom)
		// draw interface 3d left
		return true;
	case 3:
		++m_Stage;
		// stage 4:
		// draw interface 3d right
		return true;
	case 4:
		++m_Stage;
		// stage 5:
		// (endInterfacesDisplayBloom)
		// draw interface 2d left
		return true;
	case 5:
		++m_Stage;
		// stage 6:
		// draw interface 2d right
		return true;
	case 6:
		m_Stage = 0;
		// present
		m_OrientationCached = false;
		return false;
	}
	nlassert(false);
	m_Stage = 0;
	m_OrientationCached = false;
	return false;
}

const NL3D::CViewport &CStereoOVR::getCurrentViewport() const
{
	if (m_Stage % 2) return m_LeftViewport;
	else return m_RightViewport;
}

const NL3D::CFrustum &CStereoOVR::getCurrentFrustum() const
{
	if (m_Stage % 2) return m_LeftFrustum;
	else return m_RightFrustum;
}

void CStereoOVR::getCurrentFrustum(NL3D::UCamera *camera) const
{
	if (m_Stage % 2) camera->setFrustum(m_LeftFrustum);
	else camera->setFrustum(m_RightFrustum);
}

void CStereoOVR::getCurrentMatrix(NL3D::UCamera *camera) const
{
	CMatrix translate;
	if (m_Stage % 2) translate.translate(CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * -0.5f, 0.f, 0.f));
	else translate.translate(CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * 0.5f, 0.f, 0.f));
	camera->setTransformMode(NL3D::UTransformable::DirectMatrix);
	camera->setMatrix(m_CameraMatrix * translate);
}

bool CStereoOVR::beginClear()
{
	switch (m_Stage)
	{
	case 1:
		return true;
	}
	return false;
}

void CStereoOVR::endClear()
{

}
	
bool CStereoOVR::beginScene()
{
	switch (m_Stage)
	{
	case 1:
	case 2:
		return true;
	}
	return false;
}

void CStereoOVR::endScene()
{

}

bool CStereoOVR::beginInterface3D()
{
	switch (m_Stage)
	{
	case 3:
	case 4:
		return true;
	}
	return false;
}

void CStereoOVR::endInterface3D()
{

}

bool CStereoOVR::beginInterface2D()
{
	switch (m_Stage)
	{
	case 5:
	case 6:
		return true;
	}
	return false;
}

void CStereoOVR::endInterface2D()
{

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
void CStereoOVR::getInterface2DShift(float &x, float &y, float distance)
{
#if 0

	NLMISC::CVector vector = CVector(0.f, -distance, 0.f);
	NLMISC::CQuat rot = getOrientation();
	rot.invert();
	NLMISC::CMatrix mat;
	mat.rotate(rot);
	//if (m_Stage % 2) mat.translate(CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * -0.5f, 0.f, 0.f));
	//else mat.translate(CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * 0.5f, 0.f, 0.f));
	mat.translate(vector);
	CVector proj = CStereoOVR::getCurrentFrustum().project(mat.getPos());

	NLMISC::CVector ipd;
	if (m_Stage % 2) ipd = CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * -0.5f, 0.f, 0.f);
	else ipd = CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * 0.5f, 0.f, 0.f);
	CVector projipd = CStereoOVR::getCurrentFrustum().project(vector + ipd);
	CVector projvec = CStereoOVR::getCurrentFrustum().project(vector);

	x = (proj.x + projipd.x - projvec.x - 0.5f);
	y = (proj.y + projipd.y - projvec.y - 0.5f);

#elif 1

	// Alternative method

	NLMISC::CVector vec = CVector(0.f, -distance, 0.f);
	NLMISC::CVector ipd;
	if (m_Stage % 2) ipd = CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * -0.5f, 0.f, 0.f);
	else ipd = CVector(m_DevicePtr->HMDInfo.InterpupillaryDistance * 0.5f, 0.f, 0.f);
	

	NLMISC::CQuat rot = getOrientation();
	NLMISC::CQuat modrot = NLMISC::CQuat(CVector(0.f, 1.f, 0.f), NLMISC::Pi);
	rot = rot * modrot;
	float p = NLMISC::Pi + atan2f(2.0f * ((rot.x * rot.y) + (rot.z * rot.w)), 1.0f - 2.0f * ((rot.y * rot.y) + (rot.w * rot.w)));
	if (p > NLMISC::Pi) p -= NLMISC::Pi * 2.0f;	
	float t = -atan2f(2.0f * ((rot.x * rot.w) + (rot.y * rot.z)), 1.0f - 2.0f * ((rot.z * rot.z) + (rot.w * rot.w)));// // asinf(2.0f * ((rot.x * rot.z) - (rot.w * rot.y)));
	
	CVector rotshift = CVector(p, 0.f, t) * -distance;
	
	CVector proj = CStereoOVR::getCurrentFrustum().project(vec + ipd + rotshift);

	x = (proj.x - 0.5f);
	y = (proj.y - 0.5f);

#endif
}

void CStereoOVR::listDevices(std::vector<CStereoDeviceInfo> &devicesOut)
{
	s_StereoOVRSystem.Init();
	OVR::DeviceEnumerator<OVR::HMDDevice> devices = s_DeviceManager->EnumerateDevices<OVR::HMDDevice>();
	uint8 id = 1;
	do
	{
		CStereoDeviceInfo deviceInfoOut;
		OVR::DeviceInfo deviceInfo;
		if (devices.IsAvailable())
		{
			devices.GetDeviceInfo(&deviceInfo);
			CStereoOVRDeviceHandle *handle = new CStereoOVRDeviceHandle();
			deviceInfoOut.Factory = static_cast<NLMISC::CRefCount *>(handle);
			handle->DeviceHandle = devices;
			deviceInfoOut.Class = 1; // OVR::HMDDevice
			deviceInfoOut.Library = "Oculus SDK";
			deviceInfoOut.Identifier = id;
			deviceInfoOut.Manufacturer = deviceInfo.Manufacturer;
			deviceInfoOut.ProductName = deviceInfo.ProductName;
			devicesOut.push_back(deviceInfoOut);
			++id;
		}

	} while (devices.Next());
}

CStereoOVR *CStereoOVR::createDevice(const CStereoDeviceInfo &deviceInfo)
{
	CStereoOVR *stereo = new CStereoOVR(deviceInfo);
	if (stereo->isDeviceCreated())
		return stereo;
	delete stereo;
	return NULL;
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

/* end of file */
