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

#ifndef NL_PS_TAIL_DOT_H
#define NL_PS_TAIL_DOT_H

#include "nel/3d/ps_ribbon_base.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"

namespace NL3D
{

/**
 *  These particle are like dot, but a tail is following them. The number of segments in the tails can be tuned.
 */
class CPSTailDot : public  CPSRibbonBase, public CPSColoredParticle, public  CPSMaterial
{
public:
	///\name Object
	///@{
		/// ctor
		CPSTailDot();
		/// dtor
		~CPSTailDot();
		/// serialisation. Derivers must override this, and call their parent version
		virtual void		serial(NLMISC::IStream &f);
		//
		NLMISC_DECLARE_CLASS(CPSTailDot);
	///@}


	///\name Behaviour
	///@{
			/** (de)activate color fading
			* when its done, colors fades to black along the tail.
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

			/** tells in which basis is the tail
			*  It requires one transform per particle if it is not the same as the located that hold that particle
	  	    *  The default is false. With that you can control if a rotation of the system will rotate the tail
			*/
			virtual void setSystemBasis(bool yes) {}

			/// return true if the tails are in the system basis
			virtual bool isInSystemBasis(void) const { return true; }

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

	/// from CPSParticle : return true if there are lightable faces in the object
	virtual bool			hasLightableFaces() { 	return false; }

	virtual bool			supportGlobalColorLighting() const { return true; }

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
	virtual void					newElement(const CPSEmitterInfo &info);
	/// inherited from CPSLocatedBindable
	virtual void					deleteElement(uint32 index);
	/// inherited from CPSLocatedBindable
	virtual void					resize(uint32 size);
	virtual CPSLocated				*getSizeOwner(void) { return _Owner; }
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
			/** a struct containing a vertex buffer and the matching a primitive block
			  */
			class CVBnPB
			{
			public:
				CVertexBuffer		VB;
				CIndexBuffer		PB;
			public:
				CVBnPB()
				{
					NL_SET_IB_NAME(PB, "CPSTailDot");
					VB.setName("CPSTailDot");
				}
			};

			typedef CHashMap<uint, CVBnPB> TVBMap;

			static TVBMap					_VBMap;			  // index / vertex buffers with no color
			static TVBMap					_FadedVBMap;	  // index / vertex buffers for constant color with fading
			static TVBMap					_ColoredVBMap;    // index / vertex buffer + colors
			static TVBMap					_FadedColoredVBMap;    // index / vertex buffer + faded colors

			/// get a vertex buffer and a primitive suited for the current ribbon
			CVBnPB &getVBnPB();

			/// get the number of ribbons contained in a vb for a given length. (e.g the number of ribbons that can be batched)
			uint	getNumRibbonsInVB() const;
	//@}


	bool _ColorFading  : 1;
	bool _GlobalColor  : 1; // to see whether the system uses global color
	bool _Lighted      : 1;
	bool _ForceLighted : 1;
	bool _Touch		   : 1; // we use this to see if we must setup the material again

	void touch() { _Touch = true; }

	void	updateMaterial();
	void	setupGlobalColor();
};


} // NL3D


#endif // NL_PS_TAIL_DOT_H

/* End of ps_taildot.h */
