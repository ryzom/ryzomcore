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
	virtual ~CStereoPassthrough();

	virtual void setDriver(NL3D::UDriver *driver);

	virtual bool attachToDisplay();
	virtual void detachFromDisplay();

	virtual bool getScreenResolution(uint &width, uint &height);
	virtual void updateCamera(uint cid, const NL3D::UCamera *camera);
	virtual void getClippingFrustum(uint cid, NL3D::UCamera *camera) const;
	virtual void getOriginalFrustum(uint cid, NL3D::UCamera *camera) const;

	virtual bool nextPass();
	virtual const NL3D::CViewport &getCurrentViewport() const;
	virtual const NL3D::CFrustum &getCurrentFrustum(uint cid) const;
	virtual void getCurrentFrustum(uint cid, NL3D::UCamera *camera) const;
	virtual void getCurrentMatrix(uint cid, NL3D::UCamera *camera) const;

	virtual bool wantClear();
	virtual bool wantSceneReflections();
	virtual bool wantScene();
	virtual bool wantSceneEffects();
	virtual bool wantInterface3D();
	virtual bool wantInterface2D();

	virtual bool isSceneFirst();
	virtual bool isSceneLast();
	virtual uint getFlareContext();

	virtual bool beginRenderTarget();
	virtual bool endRenderTarget();

private:
	UDriver *m_Driver;
	CViewport m_Viewport;
	CFrustum m_Frustum[NL_STEREO_MAX_USER_CAMERAS];
	int m_Stage; // 0: before first pass, 1: reflection pass, 2: normal pass

}; /* class CStereoPassthrough */

} /* namespace NL3D */

#endif /* #ifndef NL3D_STEREO_PASSTHROUGH_H */

/* end of file */
