/**
 * \file stereo_debugger.h
 * \brief CStereoDebugger
 * \date 2013-07-03 20:17GMT
 * \author Jan Boon (Kaetemi)
 * CStereoDebugger
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

// NB: in FINAL_VERSION builds the class exists but is not listed as a
// selectable device (see IStereoDisplay::listDevices)
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
	virtual ~CStereoDebugger() NL_OVERRIDE;


	/// Sets driver and generates necessary render targets
	virtual void setDriver(NL3D::UDriver *driver) NL_OVERRIDE;
	/*void releaseTextures();
	void initTextures();
	void setTextures();
	void verifyTextures();*/
	void getTextures();
	void recycleTextures();

	/// Attach the driver to the display
	virtual bool attachToDisplay() NL_OVERRIDE;
	/// Detach the driver from the display
	virtual void detachFromDisplay() NL_OVERRIDE;

	/// Gets the required screen resolution for this device
	virtual bool getScreenResolution(uint &width, uint &height) NL_OVERRIDE;
	/// Set latest camera position etcetera
	virtual void updateCamera(uint cid, const NL3D::UCamera *camera) NL_OVERRIDE;
	/// Get the frustum to use for clipping
	virtual void getClippingFrustum(uint cid, NL3D::UCamera *camera) const NL_OVERRIDE;
	/// Get the original frustum of the camera
	virtual void getOriginalFrustum(uint cid, NL3D::UCamera *camera) const NL_OVERRIDE;

	/// Is there a next pass
	virtual bool nextPass() NL_OVERRIDE;
	/// Gets the current viewport
	virtual const NL3D::CViewport &getCurrentViewport() const NL_OVERRIDE;
	/// Gets the current camera frustum
	virtual const NL3D::CFrustum &getCurrentFrustum(uint cid) const NL_OVERRIDE;
	/// Gets the current camera frustum
	virtual void getCurrentFrustum(uint cid, NL3D::UCamera *camera) const NL_OVERRIDE;
	/// Gets the current camera matrix
	virtual void getCurrentMatrix(uint cid, NL3D::UCamera *camera) const NL_OVERRIDE;

	/// At the start of a new render target
	virtual bool wantClear() NL_OVERRIDE;
	/// Render scene reflections
	virtual bool wantSceneReflections() NL_OVERRIDE;
	/// The 3D scene
	virtual bool wantScene() NL_OVERRIDE;
	/// Scene post processing effects
	virtual bool wantSceneEffects() NL_OVERRIDE;
	/// Interface within the 3D scene
	virtual bool wantInterface3D() NL_OVERRIDE;	
	/// 2D Interface
	virtual bool wantInterface2D() NL_OVERRIDE;

	/// Is this the first 3D scene of the frame
	virtual bool isSceneFirst() NL_OVERRIDE;

	/// The current reflection pass index during a wantSceneReflections() stage
	virtual uint getSceneReflectionPass() const NL_OVERRIDE;

	/// The view (eye) index of the current scene or reflections stage
	virtual uint getSceneView() const NL_OVERRIDE;
	/// Is this the last 3D scene of the frame
	virtual bool isSceneLast() NL_OVERRIDE;
	/// Get the flare context for the current pass
	virtual uint getFlareContext() NL_OVERRIDE;

	/// Returns true if a new render target was set, always fase if not using render targets
	virtual bool beginRenderTarget() NL_OVERRIDE;
	/// Returns true if a render target was fully drawn, always false if not using render targets
	virtual bool endRenderTarget() NL_OVERRIDE;


	static void listDevices(std::vector<CStereoDeviceInfo> &devicesOut);

private:
	UDriver *m_Driver;

	int m_Stage;
	int m_SubStage;
	uint m_ReflPass; // current reflection pass index during stages 1-2

	CViewport m_LeftViewport;
	CViewport m_RightViewport;
	CFrustum m_Frustum[NL_STEREO_MAX_USER_CAMERAS];
	CMatrix m_CameraMatrix[NL_STEREO_MAX_USER_CAMERAS];

	NL3D::CTextureUser *m_LeftTexU;
	NL3D::CTextureUser *m_RightTexU;
	NL3D::UMaterial m_Mat;
	NLMISC::CQuadUV m_QuadUV;
	CPixelProgram *m_PixelProgram;

}; /* class CStereoDebugger */

} /* namespace NL3D */

#endif /* #ifndef NL3D_STEREO_DEBUGGER_H */

/* end of file */
