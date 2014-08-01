/**
 * \file stereo_display.h
 * \brief IStereoDisplay
 * \date 2013-06-27 16:29GMT
 * \author Jan Boon (Kaetemi)
 * IStereoDisplay
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

#ifndef NL3D_STEREO_DISPLAY_H
#define NL3D_STEREO_DISPLAY_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/smart_ptr.h>

// Project includes

namespace NL3D {
	
class UCamera;
class CViewport;
class CFrustum;
class IStereoDisplay;
class UTexture;
class UDriver;

class IStereoDeviceFactory : public NLMISC::CRefCount
{
public:
	IStereoDeviceFactory() { }
	virtual ~IStereoDeviceFactory() { }
	virtual IStereoDisplay *createDevice() const = 0;
};

struct CStereoDeviceInfo
{
public:
	enum TStereoDeviceClass
	{
		StereoDisplay, 
		StereoHMD, 
	};
	
	enum TStereoDeviceLibrary
	{
		NeL3D, 
		OVR, 
		LibVR, 
		OpenHMD, 
	};
	
	NLMISC::CSmartPtr<IStereoDeviceFactory> Factory;
	
	TStereoDeviceLibrary Library;
	TStereoDeviceClass Class;
	std::string Manufacturer;
	std::string ProductName;
	std::string Serial; // A unique device identifier
	bool AllowAuto; // Allow this device to be automatically selected when no device is configured
};

/**
 * \brief IStereoDisplay
 * \date 2013-06-27 16:29GMT
 * \author Jan Boon (Kaetemi)
 * IStereoDisplay
 */
class IStereoDisplay
{
public:
	IStereoDisplay();
	virtual ~IStereoDisplay();

	/// Sets driver and generates necessary render targets
	virtual void setDriver(NL3D::UDriver *driver) = 0;
	
	/// Gets the required screen resolution for this device
	virtual bool getScreenResolution(uint &width, uint &height) = 0;
	/// Set latest camera position etcetera
	virtual void updateCamera(uint cid, const NL3D::UCamera *camera) = 0;
	/// Get the frustum to use for clipping
	virtual void getClippingFrustum(uint cid, NL3D::UCamera *camera) const = 0;
	/// Get the original frustum of the camera
	virtual void getOriginalFrustum(uint cid, NL3D::UCamera *camera) const = 0;

	/// Is there a next pass
	virtual bool nextPass() = 0;
	/// Gets the current viewport
	virtual const NL3D::CViewport &getCurrentViewport() const = 0;
	/// Gets the current camera frustum
	virtual const NL3D::CFrustum &getCurrentFrustum(uint cid) const = 0;
	/// Gets the current camera frustum
	virtual void getCurrentFrustum(uint cid, NL3D::UCamera *camera) const = 0;
	/// Gets the current camera matrix
	virtual void getCurrentMatrix(uint cid, NL3D::UCamera *camera) const = 0;

	/// At the start of a new render target
	virtual bool wantClear() = 0;	
	/// The 3D scene
	virtual bool wantScene() = 0;
	/// Interface within the 3D scene
	virtual bool wantInterface3D() = 0;	
	/// 2D Interface
	virtual bool wantInterface2D() = 0;

	/// Returns true if a new render target was set, always fase if not using render targets
	virtual bool beginRenderTarget() = 0;
	/// Returns true if a render target was fully drawn, always false if not using render targets
	virtual bool endRenderTarget() = 0;
	
	static const char *getLibraryName(CStereoDeviceInfo::TStereoDeviceLibrary library);
	static void listDevices(std::vector<CStereoDeviceInfo> &devicesOut);
	static IStereoDisplay *createDevice(const CStereoDeviceInfo &deviceInfo);
	static void releaseUnusedLibraries();
	static void releaseAllLibraries();
	
}; /* class IStereoDisplay */

} /* namespace NL3D */

#endif /* #ifndef NL3D_STEREO_DISPLAY_H */

/* end of file */
