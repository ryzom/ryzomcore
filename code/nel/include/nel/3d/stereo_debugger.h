/**
 * \file stereo_debugger.h
 * \brief CStereoDebugger
 * \date 2013-07-03 20:17GMT
 * \author Jan Boon (Kaetemi)
 * CStereoDebugger
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

#if !FINAL_VERSION
#ifndef NL3D_STEREO_DEBUGGER_H
#define NL3D_STEREO_DEBUGGER_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/smart_ptr.h>
#include <nel/misc/geom_ext.h>

// Project includes
#include <nel/3d/stereo_display.h>
#include <nel/3d/frustum.h>
#include <nel/3d/viewport.h>
#include <nel/3d/u_material.h>

#define NL_STEREO_MAX_USER_CAMERAS 8

namespace NL3D {

class ITexture;
class CTextureUser;
class CPixelProgram;

/**
 * \brief CStereoDebugger
 * \date 2013-07-03 20:17GMT
 * \author Jan Boon (Kaetemi)
 * CStereoDebugger
 */
class CStereoDebugger : public IStereoDisplay
{
public:
	CStereoDebugger();
	virtual ~CStereoDebugger();


	/// Sets driver and generates necessary render targets
	virtual void setDriver(NL3D::UDriver *driver);
	void releaseTextures();
	void initTextures();
	void setTextures();
	void verifyTextures();

	/// Gets the required screen resolution for this device
	virtual bool getScreenResolution(uint &width, uint &height);
	/// Set latest camera position etcetera
	virtual void updateCamera(uint cid, const NL3D::UCamera *camera);
	/// Get the frustum to use for clipping
	virtual void getClippingFrustum(uint cid, NL3D::UCamera *camera) const;

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


	static void listDevices(std::vector<CStereoDeviceInfo> &devicesOut);

private:
	UDriver *m_Driver;

	int m_Stage;
	int m_SubStage;

	CViewport m_LeftViewport;
	CViewport m_RightViewport;
	CFrustum m_Frustum[NL_STEREO_MAX_USER_CAMERAS];
	CMatrix m_CameraMatrix[NL_STEREO_MAX_USER_CAMERAS];

	NLMISC::CSmartPtr<NL3D::ITexture> m_LeftTex;
	NL3D::CTextureUser *m_LeftTexU;
	NLMISC::CSmartPtr<NL3D::ITexture> m_RightTex;
	NL3D::CTextureUser *m_RightTexU;
	NL3D::UMaterial m_Mat;
	NLMISC::CQuadUV m_QuadUV;
	CPixelProgram *m_PixelProgram;

}; /* class CStereoDebugger */

} /* namespace NL3D */

#endif /* #ifndef NL3D_STEREO_DEBUGGER_H */
#endif /* #if !FINAL_VERSION */

/* end of file */
