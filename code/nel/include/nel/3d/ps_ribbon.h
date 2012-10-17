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

#ifndef NL_PS_RIBBON_H
#define NL_PS_RIBBON_H

#include "nel/3d/ps_particle_basic.h"
#include "nel/3d/ps_ribbon_base.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/misc/vector.h"


namespace NL3D
{

/** 'Ribbon' particle : a shape is extruded while it follows the particle.
 * It replaces the old ribbon version. It has more limitations : no 2d rotations and it has no propagation of size and color.
 * It doesn't take as much memory, and displays better  (the length is not dependent on framerate as it was the case before)
 */
class CPSRibbon : public CPSRibbonBase,
				  public CPSColoredParticle,
				  public CPSSizedParticle,
				  public CPSMaterial,
				  public CPSTexturedParticleNoAnim
{
public:
	// Orientation mode
	enum TOrientation { FollowPath = 0, FollowPathXY, Identity, OrientationCount };

	///\name Object
	///@{

		/** ctor.
		 * \param nbSegmentInTail the number of segment the ribbon has in its tail
		 *  \param shape pointer to a shape that will be extruded along the ribbon. It must have a unit size
		 *         and be located in the x-y plane. This will be copied
		 *  \param nbPointsInShape : the number of points in the shape
		 */
		CPSRibbon();
		/// dtor
		~CPSRibbon();
		/// serialisation. Derivers must override this, and call their parent version
		virtual void		serial(NLMISC::IStream &f) throw(NLMISC::EStream);
		//
		NLMISC_DECLARE_CLASS(CPSRibbon);
	///@}


	///\name Behaviour
	///@{
			/** (de)activate color fading
			* when its done, colors fades to black along the ribbon.
			*/
			virtual void setColorFading(bool onOff = true)
			{
				_ColorFading = onOff;
				touch();
			}

			/** Test whether color fading is activated.
			  */
			virtual bool getColorFading(void) const
			{
				return _ColorFading;
			}

			// Set orienation of slices
			void		 setOrientation(TOrientation orientation) { _Orientation = orientation; }
			TOrientation getOrientation() const { return _Orientation; }

		//void setPersistAfterDeath(bool persit = true);

		/** return true if the ribbon light persist after death
		 *  \see _PersistAfterDeath()
		 */
		//bool getPersistAfterDeath(void) const { return _DyingRibbons != NULL; }

	///@}

	/// inherited from CPSParticle
	virtual void			step(TPSProcessPass pass);

	/// return true if there are transparent faces in the object
	virtual bool			hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool			hasOpaqueFaces(void);

	virtual uint32			getNumWantedTris() const;


	/// set a texture
	void setTexture(NLMISC::CSmartPtr<ITexture> tex) { _Tex = tex; touch(); }
	/// Set texture factors.
	void setTexFactor(float uFactor = 1.f, float vFactor = 1.f)
	{
		_UFactor = uFactor;
		_VFactor = vFactor;
		touch();
	}
	// get the u-factor for texture mapping
	float getUFactor(void) const { return _UFactor; }
	// get the v-factor for texture mapping
	float getVFactor(void) const { return _VFactor; }
	/// get the texture used
	ITexture *getTexture(void) { return _Tex; }
	const ITexture *getTexture(void) const { return _Tex; }

	/** set a new shape for the ribbon
	  *  \param points A list of points that define the shape of the slice of the ribbon
	  *         These points must be located in the x-y plane (z can be used for effects).
	  *  \param nbPointsInShape The number of points in the shape
	  *  \param braceMode In this mode, the ribbon mesh is a brace. The number of vertices must be even. Each pair of vertices defines a segment whose V tex coord goes from 0 to 1
	  *                   When set to false, the list of vertices define a closed loop shape
	  */
	void setShape(const NLMISC::CVector *shape, uint32 nbPointsInShape, bool braceMode = false);
	// See if shape is in brace mode (rather then in closed loop mode)
	bool getBraceMode() const { return _BraceMode; }
	/// get the number of vertice in the shape used for ribbons
	uint32 getNbVerticesInShape(void) const { return (uint32)_Shape.size(); }

	/** Get a copy of the shape used for ribbon
	 *  \param dest a table of cvector that has the right size, it will be filled with vertices
	 *  \see getNbVerticesInShape()
	 */
	void getShape(NLMISC::CVector *shape) const;

	///\name Predefined shapes
	///@{
		// TODO : replace that ugly stuff with std::vectors

		/// Predefined shape : a regular losange shape
		static const NLMISC::CVector  Losange[];
		/// number of vertices in the losange
		static const uint NbVerticesInLosange;
		/// Predefined shape : height sides
		static const NLMISC::CVector  HeightSides[];
		/// number of vertices in the height side (must be 8 ... :)  )
		static const uint NbVerticesInHeightSide;
		/// Predifined shape : pentagram
		static const NLMISC::CVector Pentagram[];
		static const uint NbVerticesInPentagram;
		/// Predifined shape : triangle
		static const NLMISC::CVector Triangle[];
		static const uint NbVerticesInTriangle;
		// Simple segments
		static const NLMISC::CVector SimpleSegmentX[];
		static const uint NbVerticesInSimpleSegmentX;
		static const NLMISC::CVector SimpleSegmentY[];
		static const uint NbVerticesInSimpleSegmentY;
		static const NLMISC::CVector SimpleSegmentZ[];
		static const uint NbVerticesInSimpleSegmentZ;
		// Simple brace
		static const NLMISC::CVector SimpleBrace[];
		static const uint NbVerticesInSimpleBrace;
	///@}

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
/// interface to derived classes

	// the number of dying ribbons that are present
	//uint32							_NbDyingRibbons;
	// a counter to tell how much frame is left for each ribbon
	//std::vector<uint32>				_DyingRibbonsLifeLeft;

	/// inherited from CPSLocatedBindable
	virtual void					newElement(const CPSEmitterInfo &info) ;
	/// inherited from CPSLocatedBindable
	virtual void					deleteElement(uint32 index);
	/// inherited from CPSLocatedBindable
	virtual void					resize(uint32 size);
	/// From CPSSizedParticle
	virtual CPSLocated				*getSizeOwner(void) { return _Owner; }
	/// From CPSColoredParticle
	virtual CPSLocated				*getColorOwner(void) { return _Owner; }

private:


	/// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void					updateMatAndVbForColor(void);

	/// display a set of ribbons
	void							displayRibbons(uint32 nbRibbons, uint32 srcStep);

	/**\name Vertex buffers & their corresponding index buffers. We keep a map of pretextured vertex buffer (with or without colors).
	  * Vb for ribbons that have the same size are shared.
	  */

	//@{
			/** a struct containing a vertex buffer and the matching primitive block
			  * The number of slice is encoded in the upper word of the vb index (the int used to lookup in the map)
			  * The number of vertices per slices is encoded in the lower word
			  */
			class CVBnPB
			{
			public:
				CVertexBuffer		VB;
				CIndexBuffer		PB;
			public:
				CVBnPB()
				{
					NL_SET_IB_NAME(PB, "CPSRibbon::CVBnPB::PB");
					VB.setName("CPSRibbon::CVBnPB::VB");
				}
			};
			typedef CHashMap<uint, CVBnPB> TVBMap;
			//
			static TVBMap _VBMaps[16];  // 4 bits defines the display mode :
			                            // - color / no color
			                            // - faded / not faded
			                            // - textured / not textured
				                        // - closed loop shape / brace shape
			/// get a vertex buffer and a primitive suited for the current ribbon
			CVBnPB &getVBnPB();

			/// get the number of ribbons contained in a vb for a given length. (e.g the number of ribbons that can be batched)
			uint	getNumRibbonsInVB() const;
	//@}

	CSmartPtr<ITexture>				_Tex;
	CPSVector<NLMISC::CVector>::V	_Shape;
	float							_UFactor, _VFactor;
	TOrientation					_Orientation;


	bool _BraceMode				: 1; // the ribbon mesh is shaped like a brace
	bool _ColorFading           : 1;
	bool _GlobalColor		    : 1; // to see whether the system uses global color
	bool _Lighted	            : 1;
	bool _ForceLighted	        : 1;
	bool _Touch		            : 1; // we use this to see if we must setup the material again

	void touch() { _Touch = true; }

	void		updateMaterial();
	void		updateTexturedMaterial();
	void		updateUntexturedMaterial();
	void		setupGlobalColor();
	void		setupTexturedGlobalColor();
	void		setupUntexturedGlobalColor();
	void		setupTextureMatrix();

	/// Get the number of vertices in each slices (depends on whether the ribbon is textured or not)
	uint 		getNumVerticesInSlice() const;


};


} // NL3D


#endif // NL_PS_RIBBON_H

/* End of ps_ribbon.h */
