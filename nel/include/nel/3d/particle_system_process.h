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

#ifndef NL_PARTICLE_SYSTEM_PROCESS_H
#define NL_PARTICLE_SYSTEM_PROCESS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/texture.h"


namespace NLMISC
{
	class CAABBox ;
}


// append debuging header to ps functions/methods
//#define NL_PS_DEBUG

// debug class
class CPSEnterLeave
{
public:
	CPSEnterLeave(const char *name);
	~CPSEnterLeave();
	std::string Name;
};

#ifdef NL_PS_DEBUG
	extern std::string PSCurrName;
	#define NL_PS_FUNC(name) CPSEnterLeave __psDebugHeader(#name);
	#define NL_PS_FUNC_MAIN(name) CPSEnterLeave __psDebugHeader(#name); PSCurrName = _Name;
#else
	#define NL_PS_FUNC(name)
	#define NL_PS_FUNC_MAIN(name)
#endif


namespace NL3D {




class CParticleSystem ;
class CFontGenerator ;
class CFontManager ;


/** rendering and process passes for a particle system.
 *  PSMotion      : called after each simutlation step
 *  PSSolidRender : render particle that can modify z-buffer
 *  PSBlendRender : render transparency (no z-buffer write)
 *  PSToolRender  : for edition purpose, show representations for forces, emitters...
 */
enum TPSProcessPass
{ PSMotion, PSSolidRender, PSBlendRender, PSToolRender } ;


/** Objects of particle systems can be local to various matrixs defined by the following enum
  */
enum TPSMatrixMode
{
	PSFXWorldMatrix = 0,
	PSIdentityMatrix,
	PSUserMatrix,
	PSMatrixModeCount
};


/**
 *	A system particle process; A process is anything that can be called at each update of the system
 */

class CParticleSystemProcess : public NLMISC::IStreamable
{
	public:

		/// \name Object
		/// @{
			/// ctor
			CParticleSystemProcess() : _Owner(NULL), _MatrixMode(PSFXWorldMatrix), _Index(0) {}

			/// dtor
			virtual ~CParticleSystemProcess()  {}

			/** Serialize this object.
			* Everything is saved, except for the fontManager and the fontGenerator.
			* They must be set again if the PSToolRender pass is used.
			*/
			virtual void			serial(NLMISC::IStream &f);
		/// @}


		/**
		* execute this process, telling how much time ellapsed must be used for motion, and the real time ellapsed
		* (for lifetime managment)
		*/
		virtual void			step(TPSProcessPass pass) = 0 ;


		/** Compute the aabbox of this process, (expressed in world basis).
		*  \return true if there is any aabbox
		*  \param aabbox a ref to the result box
		*/
		virtual bool			computeBBox(NLMISC::CAABBox &aabbox) const = 0 ;

		/// Set the process owner. Called by the particle system during attachment.
		void					setOwner(CParticleSystem *ps);

		/// Retrieve the particle system that owns this process
		CParticleSystem			*getOwner(void) { return _Owner ; }

		/// retrieve the particle system that owns this process (const version)
		const CParticleSystem	*getOwner(void) const { return _Owner ; }

		/** Release any reference this process may have on the given process.
		  * Force example, this may be used to remove a target from a force.
		  * For example, this is used when detaching a process of a system.
		  */
		virtual	void			 releaseRefTo(const CParticleSystemProcess *other) = 0;

		/** Release any reference this process may have to other process of the system
		  * For example, this is used when detaching a process of a system.
		  */
		virtual void			 releaseAllRef() = 0;

		/// \name Useful methods for edition
		//@{
			/// Shortcut to get a font generator if one was set (edition mode)
			CFontGenerator			*getFontGenerator(void) ;

			/// Shortcut to get a font generator if one was set, const version  (edition mode)
			const CFontGenerator	*getFontGenerator(void) const ;

			/// Shortcut to get a font Manager if one was set (edition mode)
			CFontManager			*getFontManager(void) ;

			/// Shortcut to get a font Manager if one was set, const version  (edition mode)
			const CFontManager		*getFontManager(void) const ;
		//@}

		// get matrix used for that object
		TPSMatrixMode getMatrixMode() const { return _MatrixMode; }

		/** Choose the basis for this process. NB: This won't change any existing coordinate
		 *  By default, all process are expressed in the world basis
		 */
		virtual void			setMatrixMode(TPSMatrixMode matrixMode);

		/// tells whether there are alive entities / particles in the system
		virtual bool			hasParticles() const { return false ; }

		/// tells whether there are alive emitters / particles in the system
		virtual bool			hasEmitters() const { return false ; }


		/// max number of faces wanted by this process (for load balancing)
		//virtual uint			querryMaxWantedNumFaces(void) = 0 ;
		virtual uint			getNumWantedTris() const = 0 ;



		/// test whether parametric motion is enabled
		virtual bool			isParametricMotionEnabled(void) const { return false;}

		/// perform parametric motion if enabled
		virtual void			performParametricMotion(TAnimationTime /* date */) { nlassert(0);}


		// Called by the system when its date has been manually changed
		virtual void			systemDateChanged() {}

		// Helper to know if this class can be downcasted to a CPSLocated class
		virtual bool            isLocated() const { return false; }

		// returns the number of sub-objects (including this one, that requires the user matrix for its computations)
		virtual uint			getUserMatrixUsageCount() const;

		// append all tex in the given vector
		virtual void enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv) = 0;

		// Force z-bias for all material.
		virtual void setZBias(float value) = 0;

		// get the index of the process in the system
		uint		 getIndex() const { return _Index; }

		// called by owner miodel when the show / hide flag has changed
		virtual void onShow(bool shown) = 0;

	protected:
		CParticleSystem *_Owner ;

		// true if the system basis is used for display and motion
		TPSMatrixMode	 _MatrixMode;

		// index of the process in the system, should be updated by the system
		uint			_Index;
	public:
		// for use by CParticleSystem only
		void	setIndex(uint32 index) { _Index = index; }
} ;


} // NL3D


#endif // NL_PARTICLE_SYSTEM_PROCESS_H

/* End of particle_system_process.h */
