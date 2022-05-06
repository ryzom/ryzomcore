/**
 * \file stereo_ovr.h
 * \brief CStereoOVR
 * \date 2013-06-25 22:22GMT
 * \author Jan Boon (Kaetemi)
 * CStereoOVR
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2013-2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
// 
// Linking this library statically or dynamically with other modules
// is making a combined work based on this library.  Thus, the terms
// and conditions of the GNU General Public License cover the whole
// combination.
// 
// As a special exception, the copyright holders of this library give
// you permission to link this library with the Oculus SDK to produce
// an executable, regardless of the license terms of the Oculus SDK,
// and distribute linked combinations including the two, provided that
// you also meet the terms and conditions of the license of the Oculus
// SDK.  You must obey the GNU General Public License in all respects
// for all of the code used other than the Oculus SDK.  If you modify
// this file, you may extend this exception to your version of the
// file, but you are not obligated to do so.  If you do not wish to do
// so, delete this exception statement from your version.

#ifndef NL3D_STEREO_OVR_H
#define NL3D_STEREO_OVR_H

#ifdef HAVE_LIBOVR_02

#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/smart_ptr.h>
#include <nel/misc/geom_ext.h>

// Project includes
#include <nel/3d/stereo_hmd.h>
#include <nel/3d/frustum.h>
#include <nel/3d/viewport.h>
#include <nel/3d/u_material.h>

namespace NL3D {

class ITexture;
class CTextureUser;
class CStereoOVRDevicePtr;
class CStereoOVRDeviceHandle;
class CPixelProgramOVR;

#define NL_STEREO_MAX_USER_CAMERAS 8

/**
 * \brief CStereoOVR
 * \date 2013-06-25 22:22GMT
 * \author Jan Boon (Kaetemi)
 * CStereoOVR
 */
class CStereoOVR : public IStereoHMD
{
public:
	CStereoOVR(const CStereoOVRDeviceHandle *handle);
	virtual ~CStereoOVR();

	/// Sets driver and generates necessary render targets
	virtual void setDriver(NL3D::UDriver *driver);

	/// Gets the required screen resolution for this device
	virtual bool getScreenResolution(uint &width, uint &height);
	/// Set latest camera position etcetera
	virtual void updateCamera(uint cid, const NL3D::UCamera *camera);
	/// Get the frustum to use for clipping
	virtual void getClippingFrustum(uint cid, NL3D::UCamera *camera) const;
	/// Get the original frustum of the camera
	virtual void getOriginalFrustum(uint cid, NL3D::UCamera *camera) const;

	/// Is there a next pass
	virtual bool nextPass();
	/// Gets the current viewport
	virtual const NL3D::CViewport &getCurrentViewport() const;
	/// Gets the current camera frustum
	virtual const NL3D::CFrustum &getCurrentFrustum(uint cid) const;
	/// Gets the current camera frustum
	virtual void getCurrentFrustum(uint cid, NL3D::UCamera *camera) const;
	/// Gets the current camera matrix
	virtual void getCurrentMatrix(uint cid, NL3D::UCamera *camera) const;

	/// At the start of a new render target
	virtual bool wantClear();	
	/// The 3D scene
	virtual bool wantScene();
	/// Interface within the 3D scene
	virtual bool wantInterface3D();	
	/// 2D Interface
	virtual bool wantInterface2D();

	/// Returns true if a new render target was set, always fase if not using render targets
	virtual bool beginRenderTarget();
	/// Returns true if a render target was fully drawn, always false if not using render targets
	virtual bool endRenderTarget();


	/// Get the HMD orientation
	virtual NLMISC::CQuat getOrientation() const;

	/// Set the GUI reference
	virtual void setInterfaceMatrix(const NL3D::CMatrix &matrix);

	/// Get GUI center (1 = width, 1 = height, 0 = center)
	virtual void getInterface2DShift(uint cid, float &x, float &y, float distance) const;

	/// Set the head model, eye position relative to orientation point
	virtual void setEyePosition(const NLMISC::CVector &v);
	/// Get the head model, eye position relative to orientation point
	virtual const NLMISC::CVector &getEyePosition() const;

	/// Set the scale of the game in units per meter
	virtual void setScale(float s);

	/// Calculates internal camera information based on the reference camera
	void initCamera(uint cid, const NL3D::UCamera *camera);
	/// Render GUI
	void renderGUI();

	/// Checks if the device used by this class was actually created
	bool isDeviceCreated();

	static void listDevices(std::vector<CStereoDeviceInfo> &devicesOut);
	static bool isLibraryInUse();
	static void releaseLibrary();

private:
	CStereoOVRDevicePtr *m_DevicePtr;
	int m_Stage;
	int m_SubStage;
	CViewport m_RegularViewport;
	CViewport m_LeftViewport;
	CViewport m_RightViewport;
	CFrustum m_ClippingFrustum[NL_STEREO_MAX_USER_CAMERAS];
	CFrustum m_LeftFrustum[NL_STEREO_MAX_USER_CAMERAS];
	CFrustum m_RightFrustum[NL_STEREO_MAX_USER_CAMERAS];
	CFrustum m_OriginalFrustum[NL_STEREO_MAX_USER_CAMERAS];
	CMatrix m_CameraMatrix[NL_STEREO_MAX_USER_CAMERAS];
	CMatrix m_InterfaceCameraMatrix;
	mutable bool m_OrientationCached;
	mutable NLMISC::CQuat m_OrientationCache;
	UDriver *m_Driver;
	NL3D::CTextureUser *m_SceneTexture;
	NL3D::UMaterial m_BarrelMat;
	NL3D::CTextureUser *m_GUITexture;
	NLMISC::CQuadUV m_BarrelQuadLeft;
	NLMISC::CQuadUV m_BarrelQuadRight;
	NLMISC::CRefPtr<CPixelProgramOVR> m_PixelProgram;
	NLMISC::CVector m_EyePosition;
	float m_Scale;

}; /* class CStereoOVR */

} /* namespace NL3D */

#endif /* HAVE_LIBOVR */

#endif /* #ifndef NL3D_STEREO_OVR_H */

/* end of file */
