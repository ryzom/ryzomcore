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
	OVR::DeviceHandle DeviceHandle;
};

sint s_DeviceCounter = 0;

}

class CStereoOVRDevicePtr
{
public:
	OVR::Ptr<OVR::HMDDevice> HMDDevice;
};

CStereoOVR::CStereoOVR(const CStereoDeviceInfo &deviceInfo)
{
	++s_DeviceCounter;
	m_DevicePtr = new CStereoOVRDevicePtr();

	// CStereoOVRDeviceHandle *handle = static_cast<CStereoOVRDeviceHandle *>(deviceInfo.Factory.getPtr());
	// OVR::DeviceHandle dh = handle->DeviceHandle;
	// dh.CreateDevice();
}

CStereoOVR::~CStereoOVR()
{
	// ...

	delete m_DevicePtr;
	m_DevicePtr = NULL;
	--s_DeviceCounter;
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

} /* namespace NL3D */

/* end of file */
