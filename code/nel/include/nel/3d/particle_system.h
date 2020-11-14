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

#ifndef NL_PARTICLE_SYSTEM_H
#define NL_PARTICLE_SYSTEM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/object_arena_allocator.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/animated_value.h"
#include "nel/3d/particle_system_process.h"
#include "nel/3d/ps_lod.h"
#include "nel/3d/ps_attrib_maker.h"
#include "nel/3d/ps_spawn_info.h"

#include <map>



namespace NL3D
{

class CParticleSystemShape;
class CParticleSystemModel;
class CPSLocated;
class CPSLocatedBindable;
class CFontGenerator;
class CFontManager;
class CPSCopyHelper;
class CScene;
class CPSLocated;
class IDriver;
struct UPSSoundServer;




/// number user params for a particle system
const uint MaxPSUserParam = 4;


/**
 * This class holds a particle system. Most of the time it is used with a particle system model.
 * See particle_system_shape.h and particle_system_model.h for more details.
 * It can be used directly to create a shape.
 * If you plan to use this without a particle system model, make sure :
 * - you've setup the driver before calls to step()
 * - you've setup the font manager if you want to display font information
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CParticleSystem : public NLMISC::CRefCount
{
public:
	PS_FAST_OBJ_ALLOC
	// the pass that is applied on particles
	enum TPass { Anim, SolidRender, BlendRender, ToolRender };
public:
	// *****************************************************************************************************

	///\name Object
		//@{
			/// ctor
			CParticleSystem();
			/// dtor
			virtual ~CParticleSystem();
			/// serialize this particle system
			void serial(NLMISC::IStream &f);
			/** Merge this system with a system instanciated from the given shape
			  * NB : This is for edition purpose, this is slow
			  * \return true if the operation could be performed. It can fail when this cause the system the system to last forever,
			  *              which is incompatible with the 'BypassMaxNumIntegrationSteps' in CParticleSystem.
			  */
			bool merge(CParticleSystemShape *toMerge);

			/*** duplication method NOT SUPPORTED for now (duplication is using streams, but it may not last)
			 * \param ch for private use, set to null by default
			 */
			//	CParticleSystem *clone(CPSCopyHelper *ch = NULL) ;
		//@}

	// *****************************************************************************************************

	///\name Sharing system
		//@{
			/** Enable/Disable sharing. When sharing is enabled, the state of a particle system is the same for
			  * all the system that have the same shape. This allow to gain memory.
			  * However, such system should not be built with LOD in mind (for example, less emission with distance)
			  * LOD should be automatic for these system (see Auto-Lod). This means that sharing is only useful for system that
			  * have the same state, and if they are numerous : motion is performed once, but only for one system with no LOD.
			  * LOD is done during display only (when activated).
			  * The default for systems is to have no sharing.
			  */
			void	enableSharing(bool enabled = true) { _Sharing = enabled; }

			/// Test whether sharing is enabled
			bool	isSharingEnabled() const { return _Sharing; }
		//@}

	// *****************************************************************************************************

	///\name Driver setup
		//@{
		/// set the driver use to render the system
		void setDriver(IDriver *driver) { _Driver = driver; }

		/// return the driver that will be used for rendering
		IDriver *getDriver(void) { return _Driver; }
		//@}
	// *****************************************************************************************************

	///\name Scene setup
		//@{
		/** Set the scene in which the particle system is inserted. This is needed when
		 * system must add objects to the scene (for particle that are mesh for instance)
		 */
		void setScene(CScene *scene) { _Scene = scene; }

		//// get the scene set by setScene()
		CScene *getScene() const { return _Scene; }
		//@}

	// *****************************************************************************************************

	///\name Position of the system
		//@{
			/** Hide / show the system
			  * This just duplicates the 'hiden' flag of matching transform (instance of CParticleSystemModel)
			  * If the system goes from 'hide' to 'show', then no trails are generated
			  */
			void	hide(bool hidden) {	_HiddenAtCurrentFrame = hidden; }
			/** Called by owner model, when the visibility of this ps has changed
			  * (that is, the show / hide flag, not the 'clipped' state)
			  */
			void onShow(bool shown);
			/** Set the matrix for elements with matrixMode == PSFXMatrix.
			  * NB: The previous matrix position is backuped during this call (used to interpolate the system position during integration),
			  * so this should be called only once per frame
			  * NB : pointer to the matrix should remains valid as long as that particle system exists (no copy of the matrix is kept)
			  */
			void setSysMat(const NLMISC::CMatrix *m);

			/** The same as 'setSysMat', but to set the matrix for elements with matrixMode == PSUserMatrix
			  * NB : pointer to the matrix should remains valid as long as that particle system exists (no copy of the matrix is kept)
			  */
			void setUserMatrix(const NLMISC::CMatrix *m);

			/// return the matrix of the system
			const NLMISC::CMatrix &getSysMat() const
			{
				return _CoordSystemInfo.Matrix ? *_CoordSystemInfo.Matrix : NLMISC::CMatrix::Identity;
			}

			/// return the inverted matrix of the system
			const NLMISC::CMatrix &getInvertedSysMat() const { return _CoordSystemInfo.InvMatrix; }

			/** return the user matrix
			  * NB : to save memory, the user matrix is actually saved when at least one instance of CPSLocated that belongs to the system
			  * makes a reference on it. This is usually the case when CPSLocated::setMatrixMode(PSUserMatrix) is called.
			  * If no reference is made, then the fx matrix is returned instead
			  */
			const NLMISC::CMatrix &getUserMatrix() const
			{
				NL_PS_FUNC_MAIN(getUserMatrix)
				return (_UserCoordSystemInfo && _UserCoordSystemInfo->CoordSystemInfo.Matrix) ? *(_UserCoordSystemInfo->CoordSystemInfo.Matrix) : getSysMat();
			}

			/** return the inverted user matrix
			  * NB : to save memory, the user matrix is actually saved when at least one instance of CPSLocated that belongs to the system
			  * makes a reference on it. This is usually the case when CPSLocated::setMatrixMode(PSUserMatrix) is called.
			  * If no reference is made, then the inverted system matrix is returned instead.
			  */
			const NLMISC::CMatrix &getInvertedUserMatrix() const { return (_UserCoordSystemInfo && _UserCoordSystemInfo->CoordSystemInfo.Matrix) ? _UserCoordSystemInfo->CoordSystemInfo.InvMatrix : getInvertedSysMat(); }

			// conversion matrix (from user matrix to fx matrix)
			const NLMISC::CMatrix &getUserToFXMatrix() const { return (_UserCoordSystemInfo && _UserCoordSystemInfo->CoordSystemInfo.Matrix) ? _UserCoordSystemInfo->UserBasisToFXBasis : NLMISC::CMatrix::Identity; }
			// conversion matrix (from fx matrix to user matrix)
			const NLMISC::CMatrix &getFXToUserMatrix() const { return (_UserCoordSystemInfo && _UserCoordSystemInfo->CoordSystemInfo.Matrix) ? _UserCoordSystemInfo->FXBasisToUserBasis : NLMISC::CMatrix::Identity; }

			/** set the view matrix
			  * This must be called otherwise results can't be correct
			  */
			void setViewMat(const NLMISC::CMatrix &m);

			/// get the view matrix .
			const NLMISC::CMatrix &getViewMat(void) const { return _ViewMat; }

			/// get the inverted view matrix . It is stored each time a new frame is processed
			const NLMISC::CMatrix &getInvertedViewMat(void) const { return _InvertedViewMat; }
		//@}


	// *****************************************************************************************************

	///\name Execution of the system
		//@{

		/**
		* execute all the process of the system. It uses the driver that was set by a call to setDriver.
		* \param ellapsedTime The ellapsed time since the last call
		* \param pass the pass to be executed
		* \see setDriver
		*/
		virtual void step(TPass pass, TAnimationTime ellapsedTime, CParticleSystemShape &shape, CParticleSystemModel &model);
		//@}



		/// used for benchs. must be reset by the user
		static uint32 NbParticlesDrawn;

	// *****************************************************************************************************

	/**\name Process attachment. Most process are located : set of objects of the same type that have a position
	  * in space
	  */

		//@{
		/** Attach a process (such as a located : see particle_system_process.h, and ps_located.h) to the system.
		 *  It is then owned by the process and will be deleted by it.
		 *  if already present -> nl assert
		 * \return true if the operation could be performed. It can fail when this cause the system the system to last forever,
		 *              which is incompatible with the 'BypassMaxNumIntegrationSteps' in CParticleSystem
		 */
		bool						attach(CParticleSystemProcess *process);

		/** Detach a process from the system (but do not delete it)
		  */
		CParticleSystemProcess		*detach(uint index);

		/** Test whether a process is part of this system
		  */
		bool						isProcess(const CParticleSystemProcess *process) const;

		/** Given its pointer, return an index to a process.
		  * The process must be part of the system, otherwise an assertion is raised
		  */
		uint						getIndexOf(const CParticleSystemProcess &process) const;

		/** Remove a process
		 * It is deleted by the system
		 *  if not present -> nl assert
		 */
		void						remove(CParticleSystemProcess *process);

		/// get the number of process that are attached to the system
		uint32						getNbProcess(void) const { return (uint32)_ProcessVect.size(); }

		/**
		 *  Get a pointer to the nth process.
		 *  Out of range -> nlassert
		 */
		CParticleSystemProcess		*getProcess(uint32 index)
		{
			nlassert(index < _ProcessVect.size());
			return _ProcessVect[index];
		}

		/**
		 *  Get a const pointer to the nth process.
		 *  Out of range -> nlassert
		 */
		const CParticleSystemProcess *getProcess(uint32 index) const
		{
			nlassert(index < _ProcessVect.size());
			return _ProcessVect[index];
		}

		//@}

	// *****************************************************************************************************

	///\name Date / Time
		//@{

		/// get the time ellapsed since the system was created.
		TAnimationTime getSystemDate(void) const { return _SystemDate; }

		/// St the time of the system.
		void setSystemDate(float date);

		/** Get the date of the system (the number of time it has been drawn in fact)
		 *  This may be used to skip frames in an animation for example.
		 */

		uint64 getDate(void) const
		{
			NL_PS_FUNC_MAIN(getDate)
			return _Date;
		}
		//@}

	// *****************************************************************************************************

	/**\name User parameters. They may be or not used by the system. Their meaning is defined during the construction
	  * of the system
	  */

	//@{
		/** Set the value of a user parameter. It must range from 0 to 1. The user value are not saved, and their default value is 0.f.
		  * The max number of user param is MaxPSUserParam.
		  */
		void setUserParam(uint userParamIndex, float value)
		{
			NL_PS_FUNC_MAIN(setUserParam)
			nlassert(userParamIndex < MaxPSUserParam);
			NLMISC::clamp(value, 0, MaxInputValue);
			_UserParam[userParamIndex] = value;
		}

		/** Get a user param.
		  * The max number of user param is in MaxPSUserParam.
		  */
		float getUserParam(uint userParamIndex) const
		{
			NL_PS_FUNC_MAIN(getUserParam)
			nlassert(userParamIndex < MaxPSUserParam);
			return _UserParam[userParamIndex];
		}

		/** Bind/Unbind a global value to a user param.
		  * Any further call to setUserParam will then be overriden by the user param.
		  * Example of use : global strenght of wind.
		  * \param globalValueName NULL to unbind the value, or the name of the value
		  */
		void  bindGlobalValueToUserParam(const std::string &globalValueName, uint userParamIndex);
		// Get name of a global value, or NULL if no global value is bound to that user param
		std::string getGlobalValueName(uint userParamIndex) const;
		// Set a global value
		static void  setGlobalValue(const std::string &name, float value);
		// Get a global value
		static float getGlobalValue(const std::string &name);
		/** Set a global vector value. Global vector values are set to (0, 0, 0) by default
		  * Global vector values are used in some places. For example, direction for a directionnal force.
		  */
		static void					 setGlobalVectorValue(const std::string &name, const NLMISC::CVector &value);
		// Get a global vector value
		static NLMISC::CVector		 getGlobalVectorValue(const std::string &name);
		// define a handle to a global value
		class CGlobalVectorValueHandle
		{
		public:
			CGlobalVectorValueHandle() { reset(); }
			const NLMISC::CVector &get() const						 { nlassert(_Value); return *_Value; }
			void				   set(const NLMISC::CVector &value) { nlassert(_Value); *_Value = value; }
			const std::string     &getName() const					 { nlassert(_Name); return *_Name; }
			bool				   isValid() const { return _Name != NULL && _Value != NULL; }
			void                   reset() { _Name = NULL; _Value = NULL; }
		/////////////////////////////
		private:
			friend class CParticleSystem;
			const std::string *_Name;
			NLMISC::CVector   *_Value;
		};
		// Get a handle on a global value that provide a quick access on it (no map lookup)
		static CGlobalVectorValueHandle     getGlobalVectorValueHandle(const std::string &name);
	//@}



	// *****************************************************************************************************

	///\name Edition methods : provides some tools for an external editor
		// @{
		/** For edition purposes only : this allow to highlight in red the current element being edited.
		 *  \param located The located the current element belongs to, or NULL if no element is selected.
		 *  \index the index of the element in the located.
		 *  \lb the located bindable that is selected into a located (NULL = all)
		 */
		 void setCurrentEditedElement(CPSLocated *loc = NULL , uint32 index = 0, class CPSLocatedBindable *bd = NULL )
		 {
			NL_PS_FUNC_MAIN(setCurrentEditedElement)
			_CurrEditedElementLocated = loc;
			_CurrEditedElementLocatedBindable = bd;
			_CurrEditedElementIndex = index;
		 }

		/** Retrieve the current edited element
		 *  \see setCurrentEditedElement()
		 */
		 void getCurrentEditedElement(CPSLocated *&loc , uint32 &index, CPSLocatedBindable *&lb)
		 {
			NL_PS_FUNC_MAIN(getCurrentEditedElement)
			loc = _CurrEditedElementLocated;
			index = _CurrEditedElementIndex;
			lb = _CurrEditedElementLocatedBindable;
		 }

		/// Set a font generator. Useful only for edition. don't need that in runtime.
		void setFontGenerator(CFontGenerator *fg) { _FontGenerator = fg; }

		/// Retrieve the font generator. Edition purpose only.
		CFontGenerator *getFontGenerator(void) { return _FontGenerator; }

		/// Retrieve the font generator (const version). Edition purpose only.
		const CFontGenerator *getFontGenerator(void) const { return _FontGenerator; }

		/// Set a font Manager. Useful only for edition. don't need that in runtime
		void setFontManager(CFontManager *fg) { _FontManager = fg; }

		/// Retrieve the font Manager. Edition purpose only.
		CFontManager *getFontManager(void) { return _FontManager; }

		/// Retrieve the font Manager (const version). Edition purpose only.
		const CFontManager *getFontManager(void) const { return _FontManager; }
		// @}

		/// Set the name of the system.
		void setName(const std::string &s) { _Name = s; }

		/// Get the name of the system.
		std::string getName(void) const { return _Name; }

	// *****************************************************************************************************

	///\name Transparency / opacity
	// @{
		/// returns 'true' if the system has opaque object in it.
		bool hasOpaqueObjects(void) const;

		/// returns 'true' if the system has transparent objects in it.
		bool hasTransparentObjects(void) const;
	// @}

	// *****************************************************************************************************

	///\name Lighting
	// @{
		/// returns 'true' if the system has lightable objects in it
		bool hasLightableObjects() const;
	// @}

	// *****************************************************************************************************

	///\name Integration parameters
	// @{
		/** This enable for more accurate integrations of movement. When this is activated,
		  *  integration is performed in a more accurate way when the ellapsed time goes over a threshold, but it is more slow to perform.
		  */
		void enableAccurateIntegration(bool enable = true) { _AccurateIntegration = enable; }
		bool isAccurateIntegrationEnabled(void) const { return _AccurateIntegration; }

		/** the time threshold and the max number of integration to perform, when accurate integration is activated.
		  * The default is 0.15 for time threshold and 2 for max NbIntegrations
		  * \param canSlowDown : Allow the system to slow down in speed but to keep accuracy in its movement.
		  *  It is useful for critical situations where the framerate is very low. The default is true.
		  */
		void setAccurateIntegrationParams(TAnimationTime threshold,
										  uint32 maxNbIntegrations,
										  bool canSlowDown,
										  bool keepEllapsedTimeForLifeUpdate
										 )
		{
			NL_PS_FUNC_MAIN(setAccurateIntegrationParams)
			_TimeThreshold = threshold;
			_MaxNbIntegrations = maxNbIntegrations;
			_CanSlowDown = canSlowDown;
			if (_KeepEllapsedTimeForLifeUpdate != keepEllapsedTimeForLifeUpdate)
			{
				_KeepEllapsedTimeForLifeUpdate = keepEllapsedTimeForLifeUpdate;
				_PresetBehaviour = UserBehaviour;
			}
		}

		/** get the parameters used for integration.
		  * \see setAccurateIntegrationParams()
		  */
		void getAccurateIntegrationParams(TAnimationTime &threshold,
										  uint32 &maxNbIntegrations,
										  bool &canSlowDown,
										  bool &keepEllapsedTimeForLifeUpdate
										 )
		{
			NL_PS_FUNC_MAIN(getAccurateIntegrationParams)
			threshold = _TimeThreshold;
			maxNbIntegrations = _MaxNbIntegrations;
			canSlowDown = _CanSlowDown;
			keepEllapsedTimeForLifeUpdate = _KeepEllapsedTimeForLifeUpdate;
		}

		// get time thrhsold / integration time
		float getTimeTheshold() const { return _TimeThreshold; }
		/** get max nb integrations
		  * meaningful only if 'setBypassMaxNumIntegrationSteps' is false
		  */
		uint  getMaxNbIntegrations() const { return _MaxNbIntegrations; }

		/** When activated, this bypass the limit on the max number of integration steps
		  * This should NOT be used on FXs that are looping, because it would slow endlessly
		  * Anyway if you try to do that an assertion will ocurrs
		  * Typically, this is useful for spell fx because they are short, and it is important that they don't slow down
		  * when framerate is too choppy
		  */
		void	setBypassMaxNumIntegrationSteps(bool bypass = true)
		{
			NL_PS_FUNC_MAIN(setBypassMaxNumIntegrationSteps)
			if (_BypassIntegrationStepLimit != bypass)
			{
				if (bypass)
				{
					if (!canFinish())
					{
						nlwarning("<CParticleSystem::setBypassMaxNumIntegrationSteps> Can't set flag to true. The system must have a finite duration");
						nlassert(0); // the system can't stop!
						return;
					}
				}
				_BypassIntegrationStepLimit = bypass;
				_PresetBehaviour = UserBehaviour;
			}
		}
		bool	getBypassMaxNumIntegrationSteps() const { return _BypassIntegrationStepLimit; }

		/** Test if the system can finish (e.g it doesn't loop, doesn't have emitter with illimited lifetime)
		  * NB : we assume that all emitters in the system are accessible, e.g that the located graph is connex
		  * \param lastingForeverObj, if not NULL, the pointer will be filled with the first object that last or emit forever, or create a loop.
		  */

		bool canFinish(CPSLocatedBindable **lastingForeverObj = NULL) const;

		/** Test if there are loops in the system. E.g A emit B emit A
		  * NB : we assume that all emitters in the system are accessible, e.g that the located graph is connex
		  * \param loopingObj, if not NULL, will be filled with the first object that creates a loop.
		  */
		bool hasLoop(CPSLocatedBindable **loopingObj = NULL) const;


	// @}

	// *****************************************************************************************************

	/**\name LOD managment. LOD, when used  can be performed in 2 ways :
	  *						- Hand tuned LOD (for emission, color, size : this uses LOD as an input for attribute makers).
	  *                     - Auto LOD : - With non-shared systems, it modulates the emission period, quantity etc.. to get the desired result.
	  *									 - With shared systems : One version is animated with full LOD (no hand tuned LOD should be applied !).
	  *                                                          All version are displayed with fewer particle than the full LOD, depending on their distance. Visually, this is not as good as hand-tuned system, or auto-LOD on non-shared systems, however ..
	  */


		// @{

		/// set the max view distance for the system (in meters) . The default is 50 meters.
		void setMaxViewDist(float maxDist)
		{
			NL_PS_FUNC_MAIN(setMaxViewDist)
			nlassert(maxDist > 0.f);
			_MaxViewDist = maxDist;
			_InvCurrentViewDist = _InvMaxViewDist = 1.f / maxDist;
		}

		/// get the max view distance
		float getMaxViewDist(void) const { return _MaxViewDist; }

		/// set a percentage that indicate where the 2nd LOD is located. Default is 0.5.
		void setLODRatio(float ratio) { nlassert(ratio > 0 && ratio <= 1.f); _LODRatio =  ratio; }

		/// get the lod ratio.
		float getLODRatio(void) const  { return _LODRatio; }


		/** compute a vector and a distance that are used for LOD computations.
		  * You'll have for a given pos : pos * v + offset  = 0 at the nearest point, and 1 when
		  * pos is at maxDist from the viewer. This is used by sub-component of the system.
		  */
		void getLODVect(NLMISC::CVector &v, float &offset, TPSMatrixMode matrixMode);

		/// get the current LOD of the system. It is based on the distance of the center of the system to the viewer
		TPSLod getLOD(void) const;

		/// get 1.f - the current lod ratio (it is updated at each motion pass)
		float getOneMinusCurrentLODRatio(void) const { return _OneMinusCurrentLODRatio; }

		/** Enable / disbale auto-lod.
		  * When auto-LOD is enabled, less particles are displayed when the system is far.
		  * This apply to all particles in the systems (unless they override that behaviour).
		  * The default is AutoLOD off.
		  */
		void	enableAutoLOD(bool enabled = true) { _AutoLOD = enabled; }

		/// test whether Auto-LOD is enabled
		bool    isAutoLODEnabled() const { return _AutoLOD; }

		/** Setup auto lod parameters.
		  * \param start A percentage of the max view dist that tells when the auto-lod must start.
		  * \param strenght The degradation speed. It is interpreted as an exponent.
		  */
		void    setupAutoLOD(float startDistPercent, uint8 degradationExponent)
		{
			NL_PS_FUNC_MAIN(setupAutoLOD)
			nlassert(startDistPercent < 1.f);
			nlassert(degradationExponent > 0);
			_AutoLODStartDistPercent    = 	startDistPercent;
			_AutoLODDegradationExponent =	degradationExponent;
		}

		/** when auto-lod on a non shared system is used, this set the degradation of the system when it is far
		  * A value of 0 mean no more emissions at all.
		  * A value of 0.1 means 10% of emission and so on.
		  * A value of 1 means there's no LOD at all..
		  */
		void    setMaxDistLODBias(float lodBias);
		float   getMaxDistLODBias() const { return _MaxDistLODBias; }



		float	getAutoLODStartDistPercent() const { return _AutoLODStartDistPercent; }
		uint8   getAutoLODDegradationExponent() const { return _AutoLODDegradationExponent; }

		/** There are 2 modes for the auto-LOD (apply to shared systems only) :
		  * - Particle are skip in the source container when display is performed (the default)
		  * - There are just less particles displayed, but this can lead to 'pulse effect'. This is faster, though.
		  */
		void	setAutoLODMode(bool skipParticles) { _AutoLODSkipParticles = skipParticles; }
		bool    getAutoLODMode() const { return _AutoLODSkipParticles; }

		/** Setup a color attenuation scheme with the distance from the viewer. This doesn't act on a particle basis,
		  * the whole color of the system is changed in an uniform way so it is fast (the same can be achieved on a particle basis).
		  * This bypass the source of the scheme : it is set to 0 when the system is on the user, and to 1 when
		  * it is at its max distance.
		  * \param scheme A color scheme, that is then owned by this object. NULL disable color attenuation. Any previous scheme is removed
		  */
		  void	setColorAttenuationScheme(CPSAttribMaker<NLMISC::CRGBA> *colScheme)
		  {
			NL_PS_FUNC_MAIN(setColorAttenuationScheme)
			delete _ColorAttenuationScheme;
			_ColorAttenuationScheme = colScheme;
			if (!colScheme)
			{
				_GlobalColor = NLMISC::CRGBA::White;
			}
		  }

		  /// Get the global color attenuation scheme
		  CPSAttribMaker<NLMISC::CRGBA> *getColorAttenuationScheme() { return _ColorAttenuationScheme; }
		  const CPSAttribMaker<NLMISC::CRGBA> *getColorAttenuationScheme() const { return _ColorAttenuationScheme; }

		  /** Set the user color of the system
		    * Final color is the color due to attenuationByDistance (\see getColorAttenuationScheme() modulated
			* by the user color
			* NB : that state is not serialized
            */
		  void				setUserColor(NLMISC::CRGBA userColor) { _UserColor = userColor; }
		  NLMISC::CRGBA		getUserColor() const { return _UserColor; }

		  // shortcut : test il global color is used (not white)
		  bool				isUserColorUsed() const { return _UserColor != NLMISC::CRGBA::White; }

		  /** Get the current global color of the system. (It is updated just before drawing...). It there's
		    * no color attenuation scheme it can be assumed to be the same than the user color
			*/
		  NLMISC::CRGBA		getGlobalColor() const { return _GlobalColor; }
		  /** Get the current global color of the system with lighting included.
			*/
		  NLMISC::CRGBA		getGlobalColorLighted() const { return _GlobalColorLighted; }
		  // test if all objects should use global color lighting
		  bool				getForceGlobalColorLightingFlag() { return _ForceGlobalColorLighting; }
		  // force global color lighting
		  void				setForceGlobalColorLightingFlag(bool enable) { _ForceGlobalColorLighting = enable; }
		  // set lighting color (used by look at, and object that don't have normals)
		  void				setLightingColor(NLMISC::CRGBA col) { _LightingColor = col; }
		  NLMISC::CRGBA		getLightingColor() const { return _LightingColor; }
		// @}

	// *****************************************************************************************************

		// \name Load balancing
		// @{
			// get an evaluation of how many tris are needed with the system for the given distance
			float getWantedNumTris(float dist);

			/// set the number of tree the system may use. If not clled this will be the max
			void setNumTris(uint numFaces);

			/** Ask for the particle system to reevaluate the max number of faces it may need.
			  * You don't usually need to call this
			  */
			//void notifyMaxNumFacesChanged(void);

			/// Test whether load balancing has been activated for that system
			bool isLoadBalancingEnabled() const { return _EnableLoadBalancing; }

			/// Enable / disable load balancing. By default its on
			void enableLoadBalancing(bool enabled = true);
		// @}

	// *****************************************************************************************************

	///\name Bounding box managment
		// @{
		/** Compute the aabbox of this system, (expressed in thesystem basis)
		 * If the bbox is precomputed, the precomputed bbox is returned
		 *  \param aabbox a ref to the result box
		 */
		void computeBBox(NLMISC::CAABBox &aabbox);

		/** Force computation of current bbox, even if a precomputed bbox has been set
          */
		void forceComputeBBox(NLMISC::CAABBox &aabbox);

		/** When this is set to false, the system will recompute his bbox each time it is querried
		  * This may be needed for systems that move fast.
		  */
		void setAutoComputeBBox(bool enable = true) { _ComputeBBox = enable; }


		/// test whether the system compute himself his bbox
		bool getAutoComputeBBox(void) const { return _ComputeBBox; }


		/** set a precomputed bbox (expressed in the system basis). This is allowed only when setAutoComputeBBox
		  * is called with false (nlassert otherwise).
		  */

		void setPrecomputedBBox(const NLMISC::CAABBox &precompBBox)
		{
			NL_PS_FUNC_MAIN(setPrecomputedBBox)
			nlassert(!_ComputeBBox);
			_PreComputedBBox = precompBBox;
		}

		/// get the last computed bbox
		void getLastComputedBBox(NLMISC::CAABBox &dest) { dest = _PreComputedBBox; }
		// @}

	// *****************************************************************************************************

	///\name Invalidity flags (no direct effect, just indications for a third party, a model holding the system for example)
		// @{
		/** Tell the system that it is invalid when its out of range. The default is false.
		  * This is only a indication flag and must be checked by third party (a model holding the system for example)
		  */
		void				setDestroyModelWhenOutOfRange(bool enable = true)
		{
			NL_PS_FUNC_MAIN(setDestroyModelWhenOutOfRange)
			_DestroyModelWhenOutOfRange  = enable;
			_PresetBehaviour = UserBehaviour;
		}

		/// check whether the system is invalid it's out of range.
		bool				getDestroyModelWhenOutOfRange(void) const { return _DestroyModelWhenOutOfRange; }


		/// this enum give consitions on which the system may be invalid
		enum TDieCondition { none, noMoreParticles, noMoreParticlesAndEmitters   };


		/** when != to none, the Model hodling this sytem will be considered invalid when dieCondition is met
		  * This is only an indication flag and must be checked by third party (a model holding it for example)
		  * that must then use the right methods
		  * \see hasEmitters
		  * \see hasParticles
		  */
		void				setDestroyCondition(TDieCondition dieCondition)
		{
			NL_PS_FUNC_MAIN(setDestroyCondition)
			_DieCondition = dieCondition;
			_PresetBehaviour = UserBehaviour;
		}

		// test if the destroy condition is currently verified
		bool                isDestroyConditionVerified() const;

		/// get the destroy condition
		TDieCondition		getDestroyCondition(void) const { return _DieCondition; }

		/** Set a delay before to apply the death condition test
		  * This may be necessary : the system could be destroyed because there are no particles, but no particles were emitted yet
		  *
		  * This is an indication, and has no direct effect, and must be check by calling isDestroyConditionVerified()
		  *
		  * If -1 is set (or a negative value), then the system will compute that delay itself
		  *
		  * \see hasEmitters()
		  * \see hasParticles()
		  * \see getDelayBeforeDeathConditionTest()
		  */
		void setDelayBeforeDeathConditionTest(TAnimationTime delay)
		{
			NL_PS_FUNC_MAIN(setDelayBeforeDeathConditionTest)

			_DelayBeforeDieTest  = delay;
		}

		/** Must be called by a sub component of the system to tell that the duration of the system may have changed.
		  * This can happen typically if :
		  * - The system structure is changed (located added, merge ..)
		  * - The lifetime of a located is changed
		  * - Emitter parameters are modified
		  * In this case, if the system must stop when there are no particle left, the delay before that test must be recomputed
		  */
		void systemDurationChanged();

		/** Get the a delay before to apply the death condition test
		  * If the delay was set to -1 or a negative value, this will compute the delay.
		  */
		TAnimationTime		getDelayBeforeDeathConditionTest() const;

		/** Tells that the system should recompute the delay before death test itself
		  * This delay is updated when :
		  * - The system structure is changed (located added, merge ..)
		  * - The lifetime of a located is changed
		  * - Emitter parameters are modified
		  */
		void				setAutoComputeDelayBeforeDeathConditionTest(bool computeAuto);
		bool				getAutoComputeDelayBeforeDeathConditionTest() const { return _AutoComputeDelayBeforeDeathTest; }

		/** tells the model holding this system that he become invalid when its out of the view frustum.
		  * This is only an indication flag and must be checked by third party (a model holding it for example)
		  * It has no direct effects
		  * \see doesDestroyWhenOutOfRange()
		  */
		void				destroyWhenOutOfFrustum(bool enable = true)
		{
			_DestroyWhenOutOfFrustum = enable;
			_PresetBehaviour = UserBehaviour;
		}

		/** check whether the system must be destroyed when it goes out of the frustum
		  * \see getDelayBeforeDeathConditionTest()
		  */
		bool				doesDestroyWhenOutOfFrustum(void) const { return _DestroyWhenOutOfFrustum; }


		/// return true when there are still emitters in the system
		bool				hasEmitters() const;

		/// return true when there are still particles
		bool				hasParticles() const;

		/// return true when there are still temporary particles
		bool				hasTemporaryParticles() const;

		/// This enum tells when animation must be performed
		enum TAnimType
		{  AnimVisible = 0,   /* visible systems only are animated */
		   AnimInCluster, /* systems that are in cluster are animated */
		   AnimAlways,    /* animate always when not too far */
		   LastValue
		};

		/** Deprecated. This set the animation type to AnimInCluster.
          * \see setAnimType(TAnimType animType)
		  */
		void				performMotionWhenOutOfFrustum(bool enable = true)
		{
			NL_PS_FUNC_MAIN(performMotionWhenOutOfFrustum)
			_AnimType = enable ? AnimInCluster : AnimVisible;
			_PresetBehaviour = UserBehaviour;
		}

		/** Deprecated. Test if animType == AnimInCluster.
		  * * \see setAnimType(TAnimType animType)
		  */
		bool				doesPerformMotionWhenOutOfFrustum(void) const { return _AnimType == AnimInCluster; }

		/// Tells when animation must be done
		void				setAnimType(TAnimType animType)
		{
			NL_PS_FUNC_MAIN(setAnimType)
			nlassert(animType < LastValue);
			_AnimType = animType;
			_PresetBehaviour = UserBehaviour;
		}

		/// Test what the animation type is
		TAnimType			getAnimType() const { return _AnimType; }

		/** Because choosing the previous parameters can be difficult, this define
		  * presets hat can be used to tune the system easily.
		  * Any call to :
		  * - setDestroyModelWhenOutOfRange
		  * - setAnimType
		  * - setDestroyCondition
		  * - destroyWhenOutOfFrustum
		  * - performMotionWhenOutOfFrustum
		  *
		  * will set the behaviour to 'user'
		  */
		enum TPresetBehaviour
		{
			EnvironmentFX = 0,    // environment FX, not animated when not visible, persistent.
			RunningEnvironmentFX, /* an environment fx that should
								   * run when in parsed cluster : cascade for example,
								   * so that it doesn't start when the player first see
								   * it
								   */
			SpellFX,              // always animated, not persistent, garanteed to match the good frame even if framerate is low
			LoopingSpellFX,       // alway animated, persistent until emitter are stopped
			MinorFX,              // animated when visible, discarded when not visible
			UserBehaviour,
			MovingLoopingFX,       // persistent, moving fx
			SpawnedEnvironmentFX,  // environment fx, not animated when not visible, not persistent
			GroundFX,			   /** usually fx of foot steps (dust clouds etc.). Always animated, persistents, duration of fxs is garanteed,
									 * but not velocity of particle if framerate is too choppy (usually ok because particle stay in place with those fxs)
									 */
			Projectile,            // like moving looping fx, but not persistent
			PresetLast
		};

		void activatePresetBehaviour(TPresetBehaviour behaviour);

		TPresetBehaviour getBehaviourType() const
		{
			NL_PS_FUNC_MAIN(getBehaviourType)
			return _PresetBehaviour;
		}




		// @}

	// *****************************************************************************************************
	///\name sound managment
		// @{
		/// register a Sound server to this system. All systems share the same sound server.
		static void					registerSoundServer(UPSSoundServer *soundServer);

		/// get the current sound server used by this system. NULL if none
		static UPSSoundServer *		getSoundServer(void)
		{
			NL_PS_FUNC(getSoundServer)
			return _SoundServer;
		}

		/// immediatly shut down all the sound in this system
		void stopSound();

		// reactivate all sound in this system
		void reactivateSound();

		// @}

	// *****************************************************************************************************
	///\name external access to locatedBindable. PRIVATE PART (to avoid the use of friend)
		// @{
			/** register a locatedBindable, and allow it to be referenced by the given ID
			  * this locatedBindable must belong to this system.
			  * each pair <id, locatedBindable> must be unique, but there may be sevral LB for the same key
			  */
			void registerLocatedBindableExternID(uint32 id, CPSLocatedBindable *lb);

			/// unregister the given located bindable. An assertion is raised if it has not been registered before
			void unregisterLocatedBindableExternID(CPSLocatedBindable *lb);
		// @}

	// *****************************************************************************************************
	///\name external access to locatedBindable. PUBLIC PART
		// @{
				/// return the number the number of located bindable bound with this ID
				uint			   getNumLocatedBindableByExternID(uint32 id) const;
				/** return the nth locatedBindable associtaed with this ID.
				  * \return NULL if it doesn't exist
				  */
				CPSLocatedBindable       *getLocatedBindableByExternID(uint32 id, uint index);
				const CPSLocatedBindable *getLocatedBindableByExternID(uint32 id, uint index) const;
				/// Get the number of IDs in the system
				uint   getNumID() const;
				/// Get the nth ID, or 0 if index is invalid.
				uint32 getID(uint index) const;
				/** Get all the IDs in the system.
				  * \warning As IDs are not internally stored in a vector, it is faster than several calls to getID
				  */
				void getIDs(std::vector<uint32> &dest) const;
		// @}

	// *****************************************************************************************************
	///\name Misc. options / functions
		// @{
			/** When using an emitter, it is allowed to have a period of '0'. This special value means that the emitter
			  * should emit at each frame. This is deprecated now (not framerate independent ..), but some previous systems may use it.
			  * This option force a minimum period for all emitters.
			  * NB : the system should be restarted for this to work correctly
			  * The default is true
			  */
			void enableEmitThreshold(bool enabled = true) { _EmitThreshold = enabled; }
			bool isEmitThresholdEnabled() const { return _EmitThreshold; }

			// activate // deactivate all emitters in the system
			void activateEmitters(bool active);
			// test is there are active emitters in the system
			bool hasActiveEmitters() const;
			// test if there are emitters in the system (e.g. derivers of CPSEmitter bound to the system)
			bool hasEmittersTemplates() const;
			/** Eval a duration of the system (duration after which there are no particle lefts).
			  * It is meaningful only if the system can finish.
			  * After the given duration particle will have been spawn, and will possibly have finished their life.
			  * The system may last longer (case where an emitter is triggered when it bounce cannot be evaluated correclty without running the system)
			  * NB : calling this is costly
			  */
			float evalDuration() const;
			/** Enable/Disable auto-count mode. The default is disabled
			  * In this mode, when a particle is spawned it is guaranteed to be created. Particle array are resized if necessary for this.
			  * This helps to tune the size of arrays that contains particle
			  * This is well adapted for edition, but shouldn't be use at runtime because array are reallocated
			  * which doesn't give the best performance. It is why that state isn't serialized.
			  * When the system is modified by the user, he should call resetAutoCount, so that array match the current number of particles.
			  * This is useful if the user modifiy the system and that the number of particles decreases
			  */
			void setAutoCountFlag(bool enabled) { _AutoCount = enabled; }
			bool getAutoCountFlag() const { return _AutoCount; }
			/** Ensure that getMaxSize() == getSize() for each particle array.
			  * It is usefule for edition, when the system is in auto-count mode
			  * See setAutoCountFlag
              */
			void matchArraySize();
			// get max number of individual particles in the system
			uint getMaxNumParticles() const;
			// get current number of particles in the system
			uint getCurrNumParticles() const;
			// tool fct : get the list of located that target another located
			void getTargeters(const CPSLocated *target, std::vector<CPSTargetLocatedBindable *> &targeters);
			// allow to serialize identifiers. The default is false (to speedup instanciation of ps by not creating unecessary strings)
			static void setSerializeIdentifierFlag(bool on) { _SerialIdentifiers = on; }
			static bool getSerializeIdentifierFlag() { return _SerialIdentifiers; }
			// list all textures used by the ps, and append them in the given vector
			void enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv);
			// Change z-bias of all materials. The z-bias value is given in world coordinates
			void setZBias(float value);
			// debug : force to display bbox of all particle systems
			static void forceDisplayBBox(bool on) { _ForceDisplayBBox = on; }
			/** get sorting of the system emitter precedence (it is topological sort of the graph of emitters)
			  * The result vector contains the index of each process (see getProcess) in order
			  */
			void		getSortingByEmitterPrecedence(std::vector<uint> &result) const;
		// @}



private:
	typedef std::map<std::string, float> TGlobalValuesMap;
	typedef std::map<std::string, NLMISC::CVector> TGlobalVectorValuesMap;
private:
	friend class CParticleSystemModel;
	/// process a pass on the bound located
	void					stepLocated(TPSProcessPass pass);
	// update the lod & return the distance from viewer
	float					updateLODRatio();
	float					getDistFromViewer() const;
	void					updateColor(float distFromViewer);
	void					updateNumWantedTris();
	// update index of all the process
	void					updateProcessIndices();
	// for debug only
	void					checkIntegrity();
	// map that contain global value that can be affected to user param
	static TGlobalValuesMap		  _GlobalValuesMap;
	// map that contain global vector values
	static TGlobalVectorValuesMap _GlobalVectorValuesMap;
	// A bbox that has been specified by the user
	NLMISC::CAABBox			 _PreComputedBBox;
	// the driver used for rendering
	IDriver					 *_Driver;

	typedef CPSVector<CParticleSystemProcess *>::V TProcessVect;
	TProcessVect			 _ProcessVect;
	CFontGenerator			 *_FontGenerator;
	CFontManager			 *_FontManager;

	// Infos about a coordinate system (current matrix, previous pos...)
	class CCoordSystemInfo
	{
	public:
		const NLMISC::CMatrix *Matrix;			// gives the matrix to use
		NLMISC::CMatrix		   InvMatrix;       // inverted matrix for that coord system
		NLMISC::CVector		   OldPos;			// translation part of matrix at previous frame
		NLMISC::CVector        CurrentDeltaPos;	// current pos (for integration step) of the coord system relative to its final pos
	public:
		CCoordSystemInfo()
		{
			Matrix = NULL;
		}
	};


	class CUserCoordSystemInfo
	{
	public:
		CCoordSystemInfo	CoordSystemInfo;
		NLMISC::CMatrix		UserBasisToFXBasis; // conversion matrix : from user basis to FX Basis
		NLMISC::CMatrix		FXBasisToUserBasis; // conversion matrix : from FX basis to user Basis
		uint16				NumRef; // number of objects in the system that use position of the user matrix
		                            // because this is used rarely, we allocate memory to track the user matrix position only when needed
	public:
		CUserCoordSystemInfo()
		{
			NumRef = 0;
		}
	};

	CCoordSystemInfo		   _CoordSystemInfo;				// coordinate system infos for this fx
	CUserCoordSystemInfo	   *_UserCoordSystemInfo;           // coordinate system infos for an hypothetic user matrix






	// the view matrix (TODO : this is duplcated from CScene)
	NLMISC::CMatrix			 _ViewMat;

	// the inverted view matrix (TODO : this is duplcated from CScene)
	NLMISC::CMatrix			 _InvertedViewMat;


	// number of rendered pass on the system, incremented each time the system is redrawn
	uint64					 _Date;

	/// Last update date of the system. Useful with sharing only, to avoid several motions.
	sint64					 _LastUpdateDate;

	// current edited element located (edition purpose only)
	CPSLocated				 *_CurrEditedElementLocated;
	// current edited located bindable, NULL means all binadable of a located. (edition purpose only)
	CPSLocatedBindable		 *_CurrEditedElementLocatedBindable;
	// current edited element index in its located (edition purpose only)
	uint32					 _CurrEditedElementIndex;


	/** the scene in which the particle system is inserted. This is needed because
	 * the system may add objects to the scene (for particle that are meshs for instance)
	 */

	CScene					*_Scene;


	// contains the name of the system. (VERSION >= 2 only)
	std::string _Name;

	TAnimationTime								_TimeThreshold;
	TAnimationTime								_SystemDate;
	uint32										_MaxNbIntegrations;


	float										_LODRatio;
	float										_OneMinusCurrentLODRatio;
	float										_MaxViewDist;
	float										_MaxDistLODBias;
	float										_InvMaxViewDist;
	float										_InvCurrentViewDist; // inverse of the current view dist. It can be the same than _InvMaxViewDist
														        // but when there's LOD, the view distance may be reduced
	float										_AutoLODEmitRatio;

	TDieCondition								_DieCondition;
	mutable TAnimationTime						_DelayBeforeDieTest;
	uint										_NumWantedTris;
	TAnimType									_AnimType;

	static UPSSoundServer                      *_SoundServer;

	float										_UserParam[MaxPSUserParam];
	const TGlobalValuesMap::value_type		    **_UserParamGlobalValue; // usually set to NULL unless some user params mirror a global value,
	                                                                     // in this case this contains as many pointer into the global map as there are user params
	uint8                                       _BypassGlobalUserParam;  // mask to bypass a global user param. This state is not serialized

	TPresetBehaviour							_PresetBehaviour;

	typedef
		CPSMultiMap<uint32, CPSLocatedBindable *>::M TLBMap;
	TLBMap											_LBMap;

	CPSAttribMaker<NLMISC::CRGBA>				*_ColorAttenuationScheme;
	NLMISC::CRGBA								_GlobalColor;
	NLMISC::CRGBA								_GlobalColorLighted;
	NLMISC::CRGBA								_LightingColor;
	NLMISC::CRGBA								_UserColor;

	bool										_ComputeBBox                         : 1;	/// when set to true, the system will compute his BBox every time computeBBox is called
	bool										_BBoxTouched                         : 1;
	bool										_AccurateIntegration                 : 1;
	bool										_CanSlowDown                         : 1;
	bool										_DestroyModelWhenOutOfRange          : 1;
	bool										_DestroyWhenOutOfFrustum             : 1;
	bool										_Sharing                             : 1;
	bool										_AutoLOD                             : 1;
	bool										_KeepEllapsedTimeForLifeUpdate       : 1;
	bool										_AutoLODSkipParticles                : 1;
	bool										_EnableLoadBalancing                 : 1;
	bool										_EmitThreshold                       : 1;
	bool										_BypassIntegrationStepLimit          : 1;
	bool										_ForceGlobalColorLighting            : 1;
	bool										_AutoComputeDelayBeforeDeathTest     : 1;
	bool										_AutoCount							 : 1;
	bool										_HiddenAtCurrentFrame				 : 1;
	bool										_HiddenAtPreviousFrame				 : 1;

	// The two following members have been moved after the bitfield to workaround a MSVC 64-bit compiler bug (fixed in VS2013)
	// For more info, see: http://connect.microsoft.com/VisualStudio/feedback/details/777184/c-compiler-bug-vtable-pointer-put-at-wrong-offset-in-64-bit-mode
	float										_AutoLODStartDistPercent;
	uint8										_AutoLODDegradationExponent;

	static bool									_SerialIdentifiers;
	static bool									_ForceDisplayBBox;


	#ifdef NL_DEBUG
		static uint									_NumInstances;
	#endif
public:
	// For use by emitters only : This compute a delta of position of the fx matrix to ensure that spawning position are correct when the system moves
	void		interpolateFXPosDelta(NLMISC::CVector &dest, TAnimationTime deltaT);
	// For use by emitters only : This compute a delta of position of the user matrix to ensure that spawning position are correct when the system moves
	void		interpolateUserPosDelta(NLMISC::CVector &dest, TAnimationTime deltaT);
	// For use by emitters only : Get the current emit ratio when auto-LOD is used. Valid only during the 'Emit' pass
	float		getAutoLODEmitRatio() const { return _AutoLODEmitRatio; }
	// For private used by CPSLocated instances : should be called when the matrix mode of a located has changed
	void		matrixModeChanged(CParticleSystemProcess *proc, TPSMatrixMode oldMode, TPSMatrixMode newMode);
	// FOR PRIVATE USE : called when one more object of the system needs the _UserCoordSystemInfo field => so allocate it if needed.
	void		addRefForUserSysCoordInfo(uint numRefs = 1);
	// FOR PRIVATE USE : called when one less object of the system needs the _UserCoordSystemInfo field => deallocate it when there are no references left
	void		releaseRefForUserSysCoordInfo(uint numRefs = 1);
	// tmp for debug : dump hierarchy of fx
	void		dumpHierarchy();
public:
	// spawn info. They are shared by all PS ! This is because there can be only one PS processed at a time in the lib, so no need to store that per instance
	typedef std::vector<CPSSpawnInfo> TSpawnInfoVect;
	struct CSpawnVect : public NLMISC::CRefCount
	{
		TSpawnInfoVect SpawnInfos;
		uint		   MaxNumSpawns;
	};
	static std::vector<NLMISC::CSmartPtr<CSpawnVect> > _Spawns; // for each process of the system (the array is ordered as CParticleSystem::_ProcessVect is) , store a list of particles to spawn. We can't spawn them directly,
															    // because ageing particle must be processed and removed first to make room
															    // for new particles and thus avoid a 'pulse' effect when framerate is low or when spawning is fast  (see the update loop CParticleSystem::step for details)
															    // NB : we use a smart pointer to avoid resize of a vector of vector, which is baaad...
															    // This is public but intended to be used by CPSLocated only.
	static std::vector<uint>						   _ParticleToRemove;			// used during the update step, contains the indices of the particles to remove
	static std::vector<sint>						   _ParticleRemoveListIndex; 	// for each particle, -1 if it hasn't been removed, or else give the insertion number in _ParticleToRemove
	static std::vector<uint>						   _CollidingParticles; // index of particle that collided
	static std::vector<NLMISC::CVector>				   _SpawnPos;			// spawn position of newly created particles
public:
	// current sim steps infos
	static TAnimationTime								EllapsedTime;
	static TAnimationTime								InverseTotalEllapsedTime;
	static TAnimationTime								RealEllapsedTime;
	static float										RealEllapsedTimeRatio;
	static bool											InsideSimLoop;
	static bool											InsideRemoveLoop;
	static bool											InsideNewElementsLoop;
	static CParticleSystemModel							*OwnerModel; // owner model for that system
};

// NOT USED FOR NOW
/**
 *	This class holds infos needed to duplicate a particle system
 *  Because of cross referencement, an object of the system may need referencment before it is created
 *  With map holding pointer to already created object, we can duplicate the system safely
 *  for now it is for PRIVATE USE...
 *  may be useful in NLMISC later as it could be used with other kind of objects ...
 */
/*
class CPSCopyHelper
{
	public:
		// duplicate an object using the copy ctor, if it has not been before
		template <class T> T *ctorCopy(const T &src)
		{
			TCopiedIt it = _Alreadycopied.find(src);
			if (it  != _AlreadyCopied.end())
			{
				return (T *) it;
			}
			else
			{
				T *result = new T(src);
				_AlreadyCopied.insert((void *) result);
				return result;
			}
		}
		// duplicate an object using its clone method, if it has not been before

		template <class T> T *clone(const T &src)
		{
			TCopiedIt it = _AlreadyCopied.find(src);
			if (it  != _AlreadyCopied.end())
			{
				return (T *) *it;
			}
			else
			{
				T *result = src.clone(this);
				_AlreadyCopied.insert((void *) result);
				return result;
			}
		}


		// insert a value that has been copied by other means
		void insert(void *ptr)
		{
			std::pair<TCopiedIt, bool> result = _AlreadyCopied.insert(ptr);
			nlassert(result.second);
		}

	private:
		typedef std::set<void *> TAlreadyCopied;
		typedef TAlreadyCopied::iterator TCopiedIt;
		TAlreadyCopied _AlreadyCopied;
};

*/



} // NL3D


#endif // NL_PARTICLE_SYSTEM_H

/* End of particle_system.h */
