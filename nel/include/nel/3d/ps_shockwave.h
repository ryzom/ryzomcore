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

#ifndef NL_PS_SHOCKWAVE_H
#define NL_PS_SHOCKWAVE_H

#include "nel/3d/ps_particle_basic.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"

namespace NL3D
{

class CPSShockWave : public CPSParticle, public CPSSizedParticle
					, public CPSColoredParticle, public CPSTexturedParticle
					, public CPSRotated3DPlaneParticle, public CPSRotated2DParticle
					, public CPSMaterial
{
public:
	/** ctor
	 *  \param nbSeg : number of seg for the circonference of the shockwave. must be >= 3 and <= 64.
	 *  \param radiusCut : indicate how much to subtract to the outter radius to get the inner radius
	 *  \param  tex : the texture that must be applied to the shockwave
	 */
	CPSShockWave(uint nbSeg = 9, float radiusCut = 0.8f , CSmartPtr<ITexture> tex = NULL);

	/** set a new number of seg (mus be >= 3 and <= 64)
	 *  \see CPSShockWave()
	 */
	void setNbSegs(uint nbSeg);

	/// retrieve the number of segs
	uint getNbSegs(void) const { return _NbSeg; }

	/** set a new radius cut
	 *  \see CPSShockWave()
	 */
	void setRadiusCut(float aRatio);

	/// get the radius ratio
	float getRadiusCut(void) const { return _RadiusCut; }

	/// serialisation. Derivers must override this, and call their parent version
	virtual void serial(NLMISC::IStream &f);

	NLMISC_DECLARE_CLASS(CPSShockWave);

	/// complete the bbox depending on the size of particles
	virtual bool completeBBox(NLMISC::CAABBox &box) const  ;

	/// return true if there are transparent faces in the object
	virtual bool hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool hasOpaqueFaces(void);

	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32 getNumWantedTris() const;

	/// set the U factor for textures
	void			setUFactor(float value);

	/// get the U factor for textures
	float			getUFactor(void) const { return _UFactor; }

	/// from CPSParticle : return true if there are lightable faces in the object
	virtual bool hasLightableFaces() { 	return false; }

	// from CPSParticle
	virtual bool supportGlobalColorLighting() const { return true; }

	// from CPSLocatedBindable
	virtual void enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv);

	// from CPSParticle
	virtual void setZBias(float value) { CPSMaterial::setZBias(value); }
	virtual float getZBias() const { return CPSMaterial::getZBias(); }

protected:

	/** calculate current color and texture coordinate before any rendering
	 *  size can't be higher that shockWaveBufSize ...
	 */
	void			updateVbColNUVForRender(uint32 startIndex, uint32 size, uint32 srcStep, CVertexBuffer &vb, IDriver &drv);

	/// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void	updateMatAndVbForColor(void);

	/// update the material and the vb so that they match the texture scheme.
	virtual void	updateMatAndVbForTexture(void);

	/**	Generate a new element for this bindable. They are generated according to the properties of the class
	 */
	virtual void	newElement(const CPSEmitterInfo &info);

	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for the index given
	 *  index out of range -> nl_assert
	 */
	virtual void	deleteElement(uint32 index) ;

	/** Resize the bindable attributes containers. Size is the max number of element to be contained. DERIVERS MUST CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void	resize(uint32 size);

	virtual CPSLocated *getColorOwner(void) { return _Owner; }
	virtual CPSLocated *getSizeOwner(void) { return _Owner; }
	virtual CPSLocated *getAngle2DOwner(void) { return _Owner; }
	virtual CPSLocated *getPlaneBasisOwner(void) { return _Owner; }
	virtual CPSLocated *getTextureIndexOwner(void) { return _Owner; }

private:
	typedef CHashMap<uint, CVertexBuffer> TVBMap;
	typedef CHashMap<uint, CIndexBuffer> TPBMap;
private:
	static TPBMap _PBMap; // the primitive blocks
	static TVBMap _VBMap; // vb ith unanimated texture
	static TVBMap _AnimTexVBMap; // vb ith unanimated texture
	static TVBMap _ColoredVBMap; // vb ith unanimated texture
	static TVBMap _ColoredAnimTexVBMap; // vb ith unanimated texture
	// the number of seg in the shockwave
	uint32 _NbSeg;
	// ratio to get the inner circle radius from the outter circle radius
	float _RadiusCut;
	// texture factor
	float		 _UFactor;
private:
	friend class CPSShockWaveHelper;
	// setup and get the needed vb for display
	void getVBnPB(CVertexBuffer *&vb, CIndexBuffer *&pb);
	// get the number of shockwave that can be stored in the current vb
	uint getNumShockWavesInVB() const;
	//
	void setupUFactor();
	virtual void draw(bool opaque);
	/// initialisations
	virtual void	init(void);

};

} // NL3D


#endif // NL_PS_SHOCKWAVE_H

/* End of ps_particle.h */
