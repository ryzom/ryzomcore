/**
 * \file fxaa.h
 * \brief CFXAA
 * \date 2014-08-03 21:41GMT
 * \author Jan Boon (Kaetemi)
 * CFXAA
 */

/* 
 * Copyright (C) 2014  by authors
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

#ifndef NL3D_FXAA_H
#define NL3D_FXAA_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/smart_ptr.h>
#include <nel/misc/geom_ext.h>

// Project includes
#include <nel/3d/u_material.h>
#include <nel/3d/vertex_buffer.h>

#define NL_STEREO_MAX_USER_CAMERAS 8

namespace NL3D {

class ITexture;
class CTextureUser;
class CPixelProgram;

/**
 * \brief CFXAA
 * \date 2014-08-03 21:41GMT
 * \author Jan Boon (Kaetemi)
 * CFXAA
 */
class CFXAA
{
public:
	CFXAA(NL3D::UDriver *driver);
	virtual ~CFXAA();
	
	/// Apply effect to current render target. Render target must be managed by render target manager
	virtual void applyEffect();

private:
	UDriver *m_Driver;

	NL3D::UMaterial m_Mat;
	NL3D::CVertexBuffer m_VB;
	NLMISC::CQuadUV m_QuadUV;
	CPixelProgram *m_PP;

	uint m_Width;
	uint m_Height;

}; /* class CFXAA */

} /* namespace NL3D */

#endif /* #ifndef NL3D_FXAA_H */

/* end of file */
