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

#ifndef NL_PS_DOT_H
#define NL_PS_DOT_H

#include "nel/3d/ps_particle_basic.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/particle_system.h"


namespace NL3D {


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *	this is just a coloured dot
 */

class CPSDot : public CPSParticle, public CPSColoredParticle, public CPSMaterial
{
public:
	/// ctor
	CPSDot()
	{
		init();
		CParticleSystem::getSerializeIdentifierFlag();
		_Name = std::string("point");
	}

	NLMISC_DECLARE_CLASS(CPSDot);

	///serialisation
	void serial(NLMISC::IStream &f);

	/// return true if there are transparent faces in the object
	virtual bool hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool hasOpaqueFaces(void);

	/// from CPSParticle : return true if there are lightable faces in the object
	virtual bool hasLightableFaces() { 	return false; }

	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32 getNumWantedTris() const;

	/// init the vertex buffers
	static void initVertexBuffers();

	// from CPSParticle
	virtual bool supportGlobalColorLighting() const { return true; }

	// from CPSParticle
	virtual void setZBias(float value) { CPSMaterial::setZBias(value); }
	virtual float getZBias() const { return CPSMaterial::getZBias(); }

protected:
	virtual void draw(bool opaque);
	virtual CPSLocated *getColorOwner(void) { return _Owner; }
	void	init(void);
	static CVertexBuffer _DotVb;
	static CVertexBuffer _DotVbColor;

	/// update the material and the vb so that they match the color scheme
	virtual void updateMatAndVbForColor(void);

	/** Set the max number of dot
	*/
	void resize(uint32 size);

	/// we don't save datas so it does nothing for now
	void newElement(const CPSEmitterInfo &info);

	/// we don't save datas so it does nothing for now
	void deleteElement(uint32);


};


} // NL3D


#endif // NL_PS_DOT_H

/* End of ps_dot.h */
