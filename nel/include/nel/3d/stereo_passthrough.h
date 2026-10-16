// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2025  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL3D_STEREO_PASSTHROUGH_H
#define NL3D_STEREO_PASSTHROUGH_H
#include <nel/misc/types_nl.h>

// Project includes
#include <nel/3d/stereo_display.h>
#include <nel/3d/frustum.h>
#include <nel/3d/viewport.h>

#define NL_STEREO_MAX_USER_CAMERAS 8

namespace NL3D {

/// No-op IStereoDisplay implementation. Produces identical rendering
/// to the null-StereoDisplay path: one pass, no render target
/// manipulation, all want*() return true. Used as the default when
/// no VR device is configured.
class CStereoPassthrough : public IStereoDisplay
{
public:
	CStereoPassthrough();
	virtual ~CStereoPassthrough() NL_OVERRIDE;

	virtual void setDriver(NL3D::UDriver *driver) NL_OVERRIDE;

	virtual bool attachToDisplay() NL_OVERRIDE;
	virtual void detachFromDisplay() NL_OVERRIDE;

	virtual bool getScreenResolution(uint &width, uint &height) NL_OVERRIDE;
	virtual void updateCamera(uint cid, const NL3D::UCamera *camera) NL_OVERRIDE;
	virtual void getClippingFrustum(uint cid, NL3D::UCamera *camera) const NL_OVERRIDE;
	virtual void getOriginalFrustum(uint cid, NL3D::UCamera *camera) const NL_OVERRIDE;

	virtual bool nextPass() NL_OVERRIDE;
	virtual const NL3D::CViewport &getCurrentViewport() const NL_OVERRIDE;
	virtual const NL3D::CFrustum &getCurrentFrustum(uint cid) const NL_OVERRIDE;
	virtual void getCurrentFrustum(uint cid, NL3D::UCamera *camera) const NL_OVERRIDE;
	virtual void getCurrentMatrix(uint cid, NL3D::UCamera *camera) const NL_OVERRIDE;

	virtual bool wantClear() NL_OVERRIDE;
	virtual bool wantSceneReflections() NL_OVERRIDE;
	virtual bool wantScene() NL_OVERRIDE;
	virtual bool wantSceneEffects() NL_OVERRIDE;
	virtual bool wantInterface3D() NL_OVERRIDE;
	virtual bool wantInterface2D() NL_OVERRIDE;

	virtual bool isSceneFirst() NL_OVERRIDE;
	virtual bool isSceneLast() NL_OVERRIDE;

	virtual uint getSceneReflectionPass() const NL_OVERRIDE;
	virtual uint getFlareContext() NL_OVERRIDE;

	virtual bool beginRenderTarget() NL_OVERRIDE;
	virtual bool endRenderTarget() NL_OVERRIDE;

private:
	UDriver *m_Driver;
	CViewport m_Viewport;
	CFrustum m_Frustum[NL_STEREO_MAX_USER_CAMERAS];
	int m_Stage; // 0: before first pass, 1: reflection pass, 2: normal pass
	uint m_ReflPass; // current reflection pass index during stage 1

}; /* class CStereoPassthrough */

} /* namespace NL3D */

#endif /* #ifndef NL3D_STEREO_PASSTHROUGH_H */

/* end of file */
