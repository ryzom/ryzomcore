// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef NL_PS_QUAD_H
#define NL_PS_QUAD_H

#include "nel/3d/ps_particle_basic.h"
#include "nel/3d/vertex_buffer.h"

namespace NL3D
{


class IDriver;


/** This abstract class holds what is needed with quad particles (CPSFaceLookAt, CPSFace) e.g
 *  Index and vertex buffer and method to setup them
 *  Material, and method to setup them
 */

class CPSQuad : public CPSParticle
	           , public CPSColoredParticle
			   , public CPSTexturedParticle
			   , public CPSMultiTexturedParticle
			   , public CPSSizedParticle
			   , public CPSMaterial
{
public:
	/** create the quad by giving a texture. This can't be a CTextureGrouped (for animation)
	* animation must be set later by using setTextureScheme
	*/
	CPSQuad(CSmartPtr<ITexture> tex = NULL);


	/// return true if there are transparent faces in the object
	virtual bool hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool hasOpaqueFaces(void);

	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32 getNumWantedTris() const;

	/// init the vertex buffers
	static void initVertexBuffers();

	// from CPSParticle
	virtual bool supportGlobalColorLighting() const { return true; }

	// from CPSParticle
	virtual void setZBias(float value);
	virtual float getZBias() const { return CPSMaterial::getZBias(); }

	// from CPSLocatedBindable
	virtual void enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv);


protected:
	// dtor
	virtual ~CPSQuad();

	/// initialisations
	virtual void init(void);

	/// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void updateMatAndVbForColor(void);

	/// update the material and the vb so that they match the texture scheme.
	virtual void updateMatAndVbForTexture(void);

	/** update material before rendering
	  * This may also change the vb uv routing (if embm is used)
	  */
	void updateMatBeforeRendering(IDriver *drv, CVertexBuffer &vb);

	/// this is inlined to save cost of call by derived class
	void newElement(const CPSEmitterInfo &info)
	{
		newColorElement(info);
		newSizeElement(info);
		newTextureIndexElement(info);
	}

	/// this is inlined to save cost of call by derived class
	void deleteElement(uint32 index)
	{
		deleteColorElement(index);
		deleteSizeElement(index);
		deleteTextureIndexElement(index);
	}

	void resize(uint32 capacity);

	/// complete the bbox depending on the size of particles
	virtual bool completeBBox(NLMISC::CAABBox &box) const;

	/** calculate current color and texture coordinate before any rendering
	 *  size can't be higher that quadBufSize ...
	 */
	void updateVbColNUVForRender(CVertexBuffer &vb, uint32 startIndex, uint32 numQuad, uint32 srcStep, IDriver &drv);

	/// DERIVERS MUST CALL this
	void serial(NLMISC::IStream &f);
	virtual CPSLocated *getColorOwner(void) { return _Owner; }
	virtual CPSLocated *getSizeOwner(void) { return _Owner; }
	virtual CPSLocated *getTextureIndexOwner(void) { return _Owner; }

	enum VBType
	{
		VBCol			= 0x0001, // the vb has colors
		VBTex			= 0x0002, // the vb has textures coordinates
		VBTexAnimated	= 0x0004, // the texture coordinate can be animated (not precomputed)
		VBTex2		    = 0x0008, // the vb has second texture coordinates
		VBTex2Animated  = 0x0010, // the second texture coordinates can be animated (not precomputed)
		VBFullMask      = 0x001f
	};

	/// the various kind of vertex buffers we need
	static CVertexBuffer _VBPos;
	static CVertexBuffer _VBPosCol;
	static CVertexBuffer _VBPosTex1;
	static CVertexBuffer _VBPosTex1Col;
	static CVertexBuffer _VBPosTex1Anim;
	static CVertexBuffer _VBPosTex1AnimCol;
	//==========
	static CVertexBuffer _VBPosTex1Tex2;
	static CVertexBuffer _VBPosTex1ColTex2;
	static CVertexBuffer _VBPosTex1AnimTex2;
	static CVertexBuffer _VBPosTex1AnimColTex2;
	//==========
	static CVertexBuffer _VBPosTex1Tex2Anim;
	static CVertexBuffer _VBPosTex1ColTex2Anim;
	static CVertexBuffer _VBPosTex1AnimTex2Anim;
	static CVertexBuffer _VBPosTex1AnimColTex2Anim;

	/// get the vertex buffer that is needed for drawing
	CVertexBuffer &getNeededVB(IDriver &drv);


	/// used to get a pointer on the right vb dependant on its type (cf. values of VBType)
	static CVertexBuffer    * const _VbTab[];


	// from CPSTexturedParticle / CPSMultiTexturedParticle  : gives us the opportunity to update wrap mode for quad particles
	virtual void		 setTexture(CSmartPtr<ITexture> tex);
	virtual void		 setTextureGroup(NLMISC::CSmartPtr<CTextureGrouped> texGroup);
	virtual void		 setTexture2(ITexture *tex);
	virtual void		 setTexture2Alternate(ITexture *tex);
	virtual void		 updateTexWrapMode(IDriver &drv);


public:
	/// The number of quad to batche for each driver call.
	enum { quadBufSize = 800 };
};


} // NL3D


#endif // NL_PS_QUAD_H

/* End of ps_quad.h */
