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

#ifndef NL_PS_PARTICLE_BASIC_H
#define NL_PS_PARTICLE_BASIC_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/stream.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_plane_basis.h"
#include "nel/3d/material.h"
#include "nel/3d/ps_attrib_maker.h"

namespace NL3D
{


////////////////////////////////
// class forward declarations //
////////////////////////////////

class CTextureGrouped;

/**
 * This is the base class for all particles.
 * A deriver must provide a drawing method for his particle.
 * Not sharable accross systems.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSParticle : public CPSLocatedBindable
{
public:

	/// Constructor
	CPSParticle();

	/// return this bindable type
	uint32 getType(void) const { return PSParticle; }

	/// return priority
	virtual uint32 getPriority(void) const { return 1000; }

	/// return true if this located bindable derived class holds alive particles
	virtual bool hasParticles(void) const { nlassert(_Owner); return _Owner->getSize() != 0; }

	/**
	* process one pass for the particles. The default behaviour shows the particles
	*/
	virtual void step(TPSProcessPass pass)
	{
		if (
			(pass == PSBlendRender && hasTransparentFaces())
			|| (pass == PSSolidRender && hasOpaqueFaces())
			)
		{
			draw(pass == PSSolidRender);
		}
		else
		if (pass == PSToolRender) // edition mode only
		{
			showTool();
		}
	}

	/// return true if there are transparent faces in the object
	virtual bool hasTransparentFaces(void)  = 0;

	/// return true if there are Opaque faces in the object
	virtual bool hasOpaqueFaces(void)  = 0;

	/** Returns true if there are lightable faces in the object
	  */
	virtual bool hasLightableFaces() = 0;

	/** Returns true if the object can use global lighting color. (example : 'lookat' particle do not have
	  * normals, so they use global lighting color instead
	  */
	bool usesGlobalColorLighting() { return _UsesGlobalColorLighting; }
	// active / deactive global color lighting
	void enableGlobalColorLighting(bool enabled) { _UsesGlobalColorLighting = enabled; }
	// is global color lighting supported ?
	virtual bool supportGlobalColorLighting() const = 0;

	/// derivers draw the particles here
	virtual void draw(bool opaque) {}

	/// draw the particles for edition mode. The default behaviour just draw a wireframe model
	virtual void showTool();

	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32 getNumWantedTris() const = 0;

	/// serialisation. Derivers must override this, and call their parent version
	virtual void serial(NLMISC::IStream &f)
	{
		/// version 3 : global color lighting
		/// version 2 : auto-lod saved
		sint ver = f.serialVersion(3);
		CPSLocatedBindable::serial(f);
		if (ver >= 3)
		{
			f.serial(_UsesGlobalColorLighting);
		}
		if (ver >= 2)
		{
			f.serial(_DisableAutoLOD);
		}
	}


	/// Force the Auto-LOD to be disbaled. When set to false, the default behaviour set in the system is used
	void	disableAutoLOD(bool disable = true) { _DisableAutoLOD = disable; }

	/// Test whether Auto-LOD is disabled.
	bool    isAutoLODDisabled() const { return _DisableAutoLOD; }

	// Change z-bias of material. this must be redefined for all renderable particles
	virtual void			setZBias(float value) = 0;
	virtual float			getZBias() const = 0;

protected:

	/** Shortcut to notify that the max number of faces has changed
	  * This must be called when a geometric property of the particle has been modified
	  * This needn't to be called during CPSParticle::resize overrides
	  */
	/*
	void notifyOwnerMaxNumFacesChanged(void) const
	{
		if (_Owner)
		{
			_Owner->notifyMaxNumFacesChanged();
		}
	}*/

	/**	Generate a new element for this bindable. They are generated according to the properties of the class
	 */
	virtual void newElement(const CPSEmitterInfo &info) = 0;

	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index) = 0;

	/** Resize the bindable attributes containers. Size is the max number of element to be contained. DERIVERS MUST CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void resize(uint32 size) = 0;


	/** System may have hand-tuned LOD, or auto LOD.
	  * This compute the number of particles that must really be displayed, and the src step
	  * that allow to go through the whole collection. The step in the source is in a fixed point 16:16 format
	  */
	 void computeSrcStep(uint32 &step, uint &numToProcess);

private:
	 /// Disable Auto-LOD flag
	 bool	_DisableAutoLOD;
	 bool   _UsesGlobalColorLighting;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// this class adds tunable color to a particle. Can be added using public multiple inheritance
class CPSColoredParticle
{
	public:

		/** Set an attribute maker that produce a color
		 *  It must have been allocated by new
		 * It will be deleted by this object
		 */
		void setColorScheme(CPSAttribMaker<CRGBA> *col);

		/// get the color scheme (NULL if none)
		CPSAttribMaker<CRGBA> *getColorScheme(void) { return _ColorScheme; }

		/// get the color scheme (NULL if none) const version
		const CPSAttribMaker<CRGBA> *getColorScheme(void) const { return _ColorScheme; }

		/// Set a constant color for the particles. remove any previous scheme
		void setColor(NLMISC::CRGBA col);

		/// Get the color
		NLMISC::CRGBA getColor(void) const { return _Color; }

		/// ctor : default are white particles (constant color)
		CPSColoredParticle();

		/// dtor
		virtual ~CPSColoredParticle();

		/// serialization.
		void serialColorScheme(NLMISC::IStream &f);

	protected:

		/// deriver must return their owner there
		virtual CPSLocated *getColorOwner(void) = 0;
		CRGBA _Color;

		CPSAttribMaker<CRGBA> *_ColorScheme;

		/// Update the material and the vb and the like so that they match the color scheme
		virtual void updateMatAndVbForColor(void) = 0;

		void newColorElement(const CPSEmitterInfo &info)
		{
			if (_ColorScheme && _ColorScheme->hasMemory()) _ColorScheme->newElement(info);
		}
		void deleteColorElement(uint32 index)
		{
			if (_ColorScheme && _ColorScheme->hasMemory()) _ColorScheme->deleteElement(index);
		}
		void resizeColor(uint32 size)
		{
			nlassert(size < (1 << 16));
			if (_ColorScheme && _ColorScheme->hasMemory()) _ColorScheme->resize(size, getColorOwner()->getSize());
		}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// this class adds tunable size to a particle. Can be added using public multiple inheritance
class CPSSizedParticle
{
	public:

		/** Set an attribute maker that produce a size
		 *  It must have been allocated by new
		 *  It will be deleted by this object
		 */
		void setSizeScheme(CPSAttribMaker<float> *size);



		/// get the size scheme (NULL if none)
		CPSAttribMaker<float> *getSizeScheme(void) { return _SizeScheme; }

		/// get the size scheme (NULL if none) const version
		const CPSAttribMaker<float> *getSizeScheme(void) const { return _SizeScheme; }

		/// Set a constant size for the particles
		void setSize(float size);

		/// get the constant size
		float getSize(void) const { return _ParticleSize; }

		/// ctor : default are 0.1f sized particles
		CPSSizedParticle();

		/// dtor
		virtual ~CPSSizedParticle();

		/// serialization. We choose a different name because of multiple-inheritance
		void serialSizeScheme(NLMISC::IStream &f);

	protected:

		/// deriver must return their owner there
		virtual CPSLocated *getSizeOwner(void) = 0;
		float _ParticleSize;
		CPSAttribMaker<float> *_SizeScheme;
		void newSizeElement(const CPSEmitterInfo &info)
		{
			if (_SizeScheme && _SizeScheme->hasMemory()) _SizeScheme->newElement(info);
		}
		void deleteSizeElement(uint32 index)
		{
			if (_SizeScheme && _SizeScheme->hasMemory()) _SizeScheme->deleteElement(index);
		}
		void resizeSize(uint32 size)
		{
			nlassert(size < (1 << 16));
			if (_SizeScheme && _SizeScheme->hasMemory()) _SizeScheme->resize(size, getSizeOwner()->getSize());
		}
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// this class adds tunable 2D rotation to a particle, it can be used by public multiple inheritance
class CPSRotated2DParticle
{
	public:

		/** Set an attribute maker that produce a float
		 *  It must have been allocated by new
		 *  It will be deleted by this object
		 *  Output angles must range from 0.0f to 256.0f
		 */
		void setAngle2DScheme(CPSAttribMaker<float> *scheme);

		/// get the angle 2D scheme (NULL if none)
		CPSAttribMaker<float> *getAngle2DScheme(void) { return _Angle2DScheme; }

		/// get the angle 2D scheme (NULL if none) const version
		const CPSAttribMaker<float> *getAngle2DScheme(void) const { return _Angle2DScheme; }


		/** Set a constant angle for the particle. Angles range from  0.0f to 256.0f (2 pi)
		 *	This discrad any previous scheme
		 * \see setAngle2DScheme()
		 */
		void setAngle2D(float angle);

		/// get the constant
		float getAngle2D(void) const { return _Angle2D; }

		/// ctor : default are unrotated particles (angle = 0.0f)
		CPSRotated2DParticle();

		/// dtor
		virtual ~CPSRotated2DParticle();

		/// serialization. We choose a different name because of multiple-inheritance
		void serialAngle2DScheme(NLMISC::IStream &f);



		/** this return a float table used to speed up rotations of face look at and the like
		 * for each angle, there are 4 float : 2 couple of float : a1, b1, a2, b2
		 * a1 * I + b1 * K = up left corner, a2 * I + b2 * K = up right corner,
		 * This table must have been initialized with initRotTable
		 */
		static inline const float *getRotTable(void)
		{
			nlassert(_InitializedRotTab); // must have called initRotTable at the start of the apply
			return _RotTable;
		}

		/// init the rotation table
		static void initRotTable(void);

	protected:
		/// deriver must return their owner there
		virtual CPSLocated *getAngle2DOwner(void) = 0;

		float _Angle2D;
		CPSAttribMaker<float> *_Angle2DScheme;
		static float _RotTable[4 * 256];

		//#ifdef NL_DEBUG
			/// it is true if the table has been initialized, for debug purposes
			static bool _InitializedRotTab;
		//#endif

		void newAngle2DElement(const CPSEmitterInfo &info)
		{
			if (_Angle2DScheme && _Angle2DScheme->hasMemory()) _Angle2DScheme->newElement(info);
		}
		void deleteAngle2DElement(uint32 index)
		{
			if (_Angle2DScheme && _Angle2DScheme->hasMemory()) _Angle2DScheme->deleteElement(index);
		}
		void resizeAngle2D(uint32 size)
		{
			nlassert(size < (1 << 16));
			if (_Angle2DScheme && _Angle2DScheme->hasMemory()) _Angle2DScheme->resize(size, getAngle2DOwner()->getSize());
		}
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// this class is an interface for particles that have unanimated textures
struct CPSTexturedParticleNoAnim
{
		virtual ~CPSTexturedParticleNoAnim() {}
		/// set the texture for this particle
		virtual void						setTexture(CSmartPtr<ITexture> tex) = 0;
		/// get the texture used for this particle
		virtual ITexture					*getTexture(void) = 0;
		virtual const ITexture				*getTexture(void) const = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** this class adds a texture to a particle. The texture can be animated or not. it can be used by public multiple inheritance.
 *  The frame animation are all stored in the same texture for optimisation so it's not suited for large anim...
 */
class CPSTexturedParticle
{
	public:

		/** Set an attribute maker that produce a sint32
		 *  It must have been allocated by new
		 *  It will be deleted by this object
		 *  a texture group must have been set before this, an assertion occurs otherwise
		 *  The integer is used as an index in a grouped texture. It tells which frame to use
		 */
		void setTextureIndexScheme(CPSAttribMaker<sint32> *animOrder);

		/// get the texture scheme (null if none)
		CPSAttribMaker<sint32> *getTextureIndexScheme(void) { return _TextureIndexScheme; }

		/// get the texture scheme (null if none) const version
		const CPSAttribMaker<sint32> *getTextureIndexScheme(void) const { return _TextureIndexScheme; }

		/// set a constant index for the current texture. not very useful, but available...
		void setTextureIndex(sint32 index);

		/// get the animated texture index. MeaningFul only if a texture group was set
		sint32 getTextureIndex(void) const { return _TextureIndex; }

		/// set the texture group being used. It toggles animation on
		virtual void setTextureGroup(NLMISC::CSmartPtr<CTextureGrouped> texGroup);

		/// get the texture group used. it discard any previous single texture. (if null, there's no texture animation)
		CTextureGrouped *getTextureGroup(void) {  return _TexGroup; }

		/// get the texture group used if there's a texture scheme, const version. (if null, there's no texture animation)
		const CTextureGrouped *getTextureGroup(void) const { return _TexGroup; }

		/** Set a constant texture for the particle
		 *	This discard any previous scheme
		 * \see setTextureScheme()
		 */
		virtual void setTexture(CSmartPtr<ITexture> tex);

		/// get the constant texture
		ITexture *getTexture(void) { return _Tex; }
		// get the texture (const version)
		const ITexture *getTexture(void) const { return _Tex; }

		/// ctor : default have no texture. You must set it, otherwise you'll get an assertion when it's drawn
		CPSTexturedParticle();

		/// dtor
		virtual ~CPSTexturedParticle();

		/// serialization. We choose a different name because of multiple-inheritance
		void serialTextureScheme(NLMISC::IStream &f);

		void			enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest);


	protected:
		/// deriver must return their owner there
		virtual CPSLocated *getTextureIndexOwner(void) = 0;

		// a single texture
		CSmartPtr<ITexture> _Tex;

		// a grouped texture
		CSmartPtr<CTextureGrouped> _TexGroup;

		CPSAttribMaker<sint32> *_TextureIndexScheme;

		// a texture index. Most of the time, a scheme of index will be used instead of that
		sint32 _TextureIndex;

		/// Update the material so that it match the texture scheme
		virtual void updateMatAndVbForTexture(void) = 0;

		void newTextureIndexElement(const CPSEmitterInfo &info)
		{
			if (_TextureIndexScheme && _TextureIndexScheme->hasMemory()) _TextureIndexScheme->newElement(info);
		}
		void deleteTextureIndexElement(uint32 index)
		{
			if (_TextureIndexScheme && _TextureIndexScheme->hasMemory()) _TextureIndexScheme->deleteElement(index);
		}
		void resizeTextureIndex(uint32 size)
		{
			nlassert(size < (1 << 16));
			if (_TextureIndexScheme && _TextureIndexScheme->hasMemory()) _TextureIndexScheme->resize(size, getTextureIndexOwner()->getSize() );
		}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** This class add multitexturing support to particles. It doesn't support texture animation however.
  * It adds a second texture that is combined with the main texture by using a given operation.
  * An alternate mode must be provided, for gfx boards that doesn't support the op.
  * For now, 2 stages only are supported.
  */
class CPSMultiTexturedParticle
{
public:
	/// ctor
	CPSMultiTexturedParticle();
	virtual ~CPSMultiTexturedParticle() {}

	/// we only use a useful set of operations
	enum TOperator { Add = 0, Modulate, Decal, EnvBumpMap, Last = 0xff };

	/// when set to false, this discard the textures that have been set
	void						enableMultiTexture(bool enabled = true);
	bool						isMultiTextureEnabled() const	{ return (_MultiTexState & (uint8) MultiTextureEnabled) != 0; }

	/// Set the main texture for multitexturing. Convert the texture to / from a bumpmap if needed (so you just provide its heightmap)
	void						setTexture2(ITexture *tex);

	/// Get the main texture for multitexturing
	const ITexture				*getTexture2() const { return _Texture2; }
	ITexture					*getTexture2() { return _Texture2; }

	/** Set the operation for the main texture. When EnvBumpMap is used, setTexture2 must be called with a bump map,
	  * and the primary texture must be convertible to rgba. Convert the texture to / from a bumpmap if needed
	  */
	void						setMainTexOp(TOperator op);

	TOperator					getMainTexOp() const	   { return _MainOp; }

	// Enable the use of an alternate texture for multitexturing. When disabled, this discard the textures that may have been set.
	void						enableAlternateTex(bool enabled = true);
	bool						isAlternateTexEnabled() const { return (_MultiTexState & (uint8) AlternateTextureEnabled) != 0; }

	/// Set the alternate texture for multitexturing. It is used when the main operator is not supported by the gfx board.
	// Convert the texture to / from a bumpmap if needed. (so you just provide its heightmap)
	void						setTexture2Alternate(ITexture *tex);

	/// Get the alternate texture for multitexturing.
	const ITexture				*getTexture2Alternate() const { return _AlternateTexture2; }
	ITexture					*getTexture2Alternate() { return _AlternateTexture2; }

	/// Set the operation for the alternate texture. Convert the texture to / from a bumpmap if needed.
	void						setAlternateTexOp(TOperator op);

	TOperator					getAlternateTexOp() const
	{
		return _AlternateOp;
	}

	/** set the scroll speed for tex 1 & 2 when the main op is used
	  * \param stage can be set to 0 or one
	  */
	void						setScrollSpeed(uint stage, const NLMISC::CVector2f &sp)
	{
		nlassert(stage < 2);
		_TexScroll[stage] = sp;
	}
	const NLMISC::CVector2f		&getScrollSpeed(uint stage) const
	{
		nlassert(stage < 2);
		return _TexScroll[stage];
	}

	/** set the scroll speed for tex 1 & 2 when the alternate op is used
	  * \param stage can be set to 0 or one
	  */
	void						setAlternateScrollSpeed(uint stage, const NLMISC::CVector2f &sp)
	{
		nlassert(stage < 2);
		_TexScrollAlternate[stage] = sp;
	}
	const NLMISC::CVector2f		&getAlternateScrollSpeed(uint stage) const
	{
		nlassert(stage < 2);
		return _TexScrollAlternate[stage];
	}

	/// serial this object
	void serialMultiTex(NLMISC::IStream &f);

	/** setup a material from this object and a primary texture
	  * drv is used to check the device caps.
	  * Must be called before display when multitextureing is used
	  * vb is needed because uv routing may be changed because of embm
	  */
	void setupMaterial(ITexture *primary, IDriver *drv, CMaterial &mat, CVertexBuffer &vb);

	/** this act as if the system had the most basic caps supported (no EMBM for example...)
	  * Should be used only in edition mode for test
	  */
	static void forceBasicCaps(bool force = true) { _ForceBasicCaps =  force; }

	/// test whether basic caps are forced
	static  bool areBasicCapsForced() { return _ForceBasicCaps; }

	/// Use the particle age rather than the global time to compute textures coordinates.
	void	setUseLocalDate(bool use);
	bool	getUseLocalDate() { return (_MultiTexState & ScrollUseLocalDate) != 0; }

	/// Use the particle age rather than the global time to compute textures coordinates. (when alternate texture is used)
	void	setUseLocalDateAlt(bool use);
	bool	getUseLocalDateAlt() { return (_MultiTexState & ScrollUseLocalDateAlternate) != 0; }

	/// Set a bump factor (when embm is used)
	void	setBumpFactor(float bumpFactor) { _BumpFactor = bumpFactor; touch(); }
	float	getBumpFactor() const { return _BumpFactor; }

	void			enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv);


protected:
	void						setupMultiTexEnv(TOperator op, ITexture *tex1, ITexture *tex2, CMaterial &mat, IDriver &drv);
	TOperator					_MainOp, _AlternateOp;
	NLMISC::CSmartPtr<ITexture> _Texture2;
	NLMISC::CSmartPtr<ITexture> _AlternateTexture2;

	/// texture scrolling
	NLMISC::CVector2f _TexScroll[2];
	/// alternate texture scrollMultiTextureEnabled
	NLMISC::CVector2f _TexScrollAlternate[2];

	enum TMultiTexState {  TouchFlag = 0x01, MultiTextureEnabled = 0x02, AlternateTextureEnabled = 0x04, AlternateTextureUsed = 0x08, EnvBumpMapUsed = 0x10, BasicCapsForced = 0x20,
							ScrollUseLocalDate = 0x40, ScrollUseLocalDateAlternate = 0x80
						};
	uint8   _MultiTexState;

	/// test whether the alternate texture is used
	bool	isAlternateTextureUsed(IDriver &driver) const;
	bool	isEnvBumpMapUsed() const { return (_MultiTexState & EnvBumpMapUsed) != 0; }


	// update wrap mode for all textures
	virtual void updateTexWrapMode(IDriver &drv) = 0;
	void touch()		{ _MultiTexState |= (uint8) TouchFlag; }
	void unTouch()		{ _MultiTexState &= ~ (uint8) TouchFlag; }
	bool isTouched()	const { return (_MultiTexState & TouchFlag) != 0; }
	bool areBasicCapsForcedLocal() const { return (_MultiTexState & BasicCapsForced) != 0; }
	void forceBasicCapsLocal(bool force)
	{
		if (force) _MultiTexState |= BasicCapsForced;
		else _MultiTexState &= ~BasicCapsForced;
	}
	float		_BumpFactor;
	static bool _ForceBasicCaps;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** this class adds tunable 3D rotation to a PLANE particle, it can be used by public multiple inheritance
 *  It must just produce 2 vectors that give the x and y vector of the local basis.
 */
class CPSRotated3DPlaneParticle
{
	public:

		/** Set an attribute maker that produce a basis
		 *  It must have been allocated by new
		 *  It will be deleted by this object
		 */
		void setPlaneBasisScheme(CPSAttribMaker<CPlaneBasis> *basisMaker);

		/** Set a constant basis for all particles
		 * \see setPlaneBasisSchemeScheme()
		 */

		/// get the plane basis scheme, (NULL if none)
		CPSAttribMaker<CPlaneBasis> *getPlaneBasisScheme(void) { return _PlaneBasisScheme; }

		/// get the plane basis scheme, (NULL if none) const version
		const CPSAttribMaker<CPlaneBasis> *getPlaneBasisScheme(void) const { return _PlaneBasisScheme; }

		void setPlaneBasis(const CPlaneBasis &basis);

		/// get the constant basis
		CPlaneBasis getPlaneBasis(void) const { return _PlaneBasis; }

		/// ctor : default have constant basis that map to the I & J vector (e.g identity)
		CPSRotated3DPlaneParticle();

		/// dtor
		virtual ~CPSRotated3DPlaneParticle();

		/// serialization. We choose a different name because of multiple-inheritance
		void serialPlaneBasisScheme(NLMISC::IStream &f);

	protected:
		/// if this is false, constant size will be used instead of a scheme

		/// deriver must return their owner there
		virtual CPSLocated *getPlaneBasisOwner(void) = 0;

		CPSAttribMaker<CPlaneBasis> *_PlaneBasisScheme;

		CPlaneBasis _PlaneBasis; // constant basis..

		void newPlaneBasisElement(const CPSEmitterInfo &info)
		{
			if (_PlaneBasisScheme && _PlaneBasisScheme->hasMemory()) _PlaneBasisScheme->newElement(info);
		}
		void deletePlaneBasisElement(uint32 index)
		{
			if (_PlaneBasisScheme && _PlaneBasisScheme->hasMemory()) _PlaneBasisScheme->deleteElement(index);
		}
		void resizePlaneBasis(uint32 size)
		{
			nlassert(size < (1 << 16));
			if (_PlaneBasisScheme && _PlaneBasisScheme->hasMemory()) _PlaneBasisScheme->resize(size, getPlaneBasisOwner()->getSize());
		}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** This add a hint to rotated particle : only a few one are rotated, and the other are duplcated
 *
 */

struct CPSHintParticleRotateTheSame
{
	virtual ~CPSHintParticleRotateTheSame() {}

	/** Tells that all particles are turning in the same manner, and only have a rotationnal bias
	 *  This is faster then other method. Any previous set scheme for 3d rotation is kept.
	 *	\param: the number of rotation configuration we have. The more high it is, the slower it'll be
	 *          If this is too low, a lot of particles will have the same orientation
	 *          If it is 0, then the hint is disabled
 	 *  \param  minAngularVelocity : the maximum angular velocity for particle rotation
	 *  \param  maxAngularVelocity : the maximum angular velocity for particle rotation
	 *  \see    CPSRotated3dPlaneParticle
	 */
	virtual void hintRotateTheSame(uint32 nbConfiguration
							, float minAngularVelocity = NLMISC::Pi
							, float maxAngularVelocity = NLMISC::Pi
						  ) = 0;

	/** disable the hint 'hintRotateTheSame'
	 *  The previous set scheme for roation is used
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */
	virtual void disableHintRotateTheSame(void) = 0;


	/** check whether a call to hintRotateTheSame was performed
	 *  \return 0 if the hint is disabled, the number of configurations else
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */
	virtual uint32 checkHintRotateTheSame(float &minAngularVelocity, float &maxAngularVelocity) const = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// base struct for particle that have a tail
struct CPSTailParticle
{
	virtual ~CPSTailParticle() {}

	/** (de)activate color fading
	 * when its done, colors fades to black along the tail
	 */
	virtual void setColorFading(bool onOff = true) = 0;

	/// test whether color fading is activated
	virtual bool getColorFading(void) const = 0;


	/// there may be a maximum with some particles
	virtual void setTailNbSeg(uint32 nbSeg) = 0;

	// get the number of segments in the tail
	virtual	uint32 getTailNbSeg(void) const = 0;


};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// base struct for particles that can have a shape (e.g mesh...)
struct CPSShapeParticle
{
	virtual ~CPSShapeParticle() {}

	/// set a new shape
	virtual void setShape(const std::string &shape) = 0;

	/// get the shape used for those particles
	virtual std::string getShape(void) const = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** this contains material of a particle, this doesn't initiliaze anything, this just give the abylity to
  * change the blending mode
  */
class CPSMaterial
{
public:
	/// ctor : the default is additive blending
	CPSMaterial();

	/// this enum summarize the useful modes for blending to the framebuffer
	enum TBlendingMode { add, modulate, alphaBlend, alphaTest };

	/// serialization (not named 'serial' because it will be used via multiple-inheritance)
	void serialMaterial(NLMISC::IStream &f);

	/// set the blending mode. The default is ass
	void setBlendingMode(CPSMaterial::TBlendingMode mode);

	/// return the blending mode currently used
	CPSMaterial::TBlendingMode getBlendingMode(void) const;


	/** Force the material to have one texture that is modulated by diffuse, and a constant color
	  * and its diffuse color. This is not compatible with multitextureing, however.
	  * \param force true to force constant color modulation
	  */
	void forceModulateConstantColor(bool force, const NLMISC::CRGBA &col = NLMISC::CRGBA::White);

	/** This setup n stage of a material with at least texture.
	 * - If a texture was present for a given stage it still is
	 * - If a texture wasn't present, it create a dummy white texture there
	 * - Above numStages, textures are disabled.
	 * It can be used to do extra math with stages that have no textures (required by the driver)
	 */
	void forceTexturedMaterialStages(uint numStages);

	// enable / disable z-test (default is enabled)
	void enableZTest(bool enabled);
	// test if z test is enabled
	bool isZTestEnabled() const;

	// set z-bias
	void setZBias(float value) { _Mat.setZBias(value); }
	float getZBias() const { return _Mat.getZBias(); }



protected:
	CMaterial _Mat;
};


//==========================================================================
/// setup a stage as modulate, by specifying the source and destination
inline void SetupModulatedStage(CMaterial &m, uint stage, CMaterial::TTexSource src1, CMaterial::TTexSource src2)
{
	m.texEnvOpRGB(stage, CMaterial::Modulate);
	m.texEnvOpAlpha(stage, CMaterial::Modulate);
	m.texEnvArg0RGB(stage, src1, CMaterial::SrcColor);
	m.texEnvArg1RGB(stage, src2, CMaterial::SrcColor);
	m.texEnvArg0Alpha(stage, src1, CMaterial::SrcAlpha);
	m.texEnvArg1Alpha(stage, src2, CMaterial::SrcAlpha);
}




} // NL3D


#endif // NL_PS_PARTICLE_BASIC_H

/* End of ps_particle_basic.h */
