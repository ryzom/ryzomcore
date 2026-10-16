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

#include "std3d.h"
#include "nel/3d/stereo_passthrough.h"

// Project includes
#include "nel/3d/u_camera.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

CStereoPassthrough::CStereoPassthrough() : m_Driver(NULL), m_Stage(0)
{

}

CStereoPassthrough::~CStereoPassthrough()
{
	m_Driver = NULL;
}

void CStereoPassthrough::setDriver(NL3D::UDriver *driver)
{
	m_Driver = driver;
}

bool CStereoPassthrough::attachToDisplay()
{
	return false;
}

void CStereoPassthrough::detachFromDisplay()
{

}

bool CStereoPassthrough::getScreenResolution(uint &width, uint &height)
{
	return false;
}

void CStereoPassthrough::updateCamera(uint cid, const NL3D::UCamera *camera)
{
	if (cid < NL_STEREO_MAX_USER_CAMERAS)
		m_Frustum[cid] = camera->getFrustum();
}

void CStereoPassthrough::getClippingFrustum(uint cid, NL3D::UCamera *camera) const
{
	// do nothing
}

void CStereoPassthrough::getOriginalFrustum(uint cid, NL3D::UCamera *camera) const
{
	// do nothing, as we never modified it
}

bool CStereoPassthrough::nextPass()
{
	// Stage 0 -> 1 (reflection pass), 1 -> 2 (normal pass), 2 -> 0 (done)
	++m_Stage;
	if (m_Stage > 2)
	{
		m_Stage = 0;
		return false;
	}
	return true;
}

const NL3D::CViewport &CStereoPassthrough::getCurrentViewport() const
{
	return m_Viewport;
}

const NL3D::CFrustum &CStereoPassthrough::getCurrentFrustum(uint cid) const
{
	return m_Frustum[cid];
}

void CStereoPassthrough::getCurrentFrustum(uint cid, NL3D::UCamera *camera) const
{
	// do nothing
}

void CStereoPassthrough::getCurrentMatrix(uint cid, NL3D::UCamera *camera) const
{
	// do nothing
}

bool CStereoPassthrough::wantClear()
{
	return m_Stage == 2; // only on normal pass
}

bool CStereoPassthrough::wantSceneReflections()
{
	return m_Stage == 1; // only on reflection pass
}

bool CStereoPassthrough::wantScene()
{
	return m_Stage == 2; // only on normal pass
}

bool CStereoPassthrough::wantSceneEffects()
{
	return m_Stage == 2; // only on normal pass
}

bool CStereoPassthrough::wantInterface3D()
{
	return m_Stage == 2; // only on normal pass
}

bool CStereoPassthrough::wantInterface2D()
{
	return m_Stage == 2; // only on normal pass
}

bool CStereoPassthrough::isSceneFirst()
{
	return true;
}

bool CStereoPassthrough::isSceneLast()
{
	return true;
}

uint CStereoPassthrough::getFlareContext()
{
	return 0;
}

bool CStereoPassthrough::beginRenderTarget()
{
	return false;
}

bool CStereoPassthrough::endRenderTarget()
{
	return false;
}

} /* namespace NL3D */

/* end of file */
