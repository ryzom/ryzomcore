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

#ifndef NL_U_PARTICLE_SYSTEM_INSTANCE_H
#define NL_U_PARTICLE_SYSTEM_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "u_instance.h"


namespace NLMISC
{
	class CAABBox;
}

namespace NL3D {


/**
 * Interface to manipulate a particle system. Particle system are created from a UScene.
 * A system can be tuned by its user params (when it makes use of them). It has several states
 * invalid : the system is invalid, this tells the user that he can destroy this instance
 *
 * present : the system is available for modification. This may not be the case when the system has been temporarily remove because
 *           it is not visible anymore.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class UParticleSystemInstance : public UInstance
{
public:
	/** Tell whether the system is currently instanciated. This may not be the case when the system is not visible
	  * Unless specified otherwise, you must check this before you use any method that access the system.
	  * If you don't, there will be an assertion when you'll try to access it
	  *
	  * \see isValid()
	  */
	bool		isSystemPresent		(void) const;

	/** Get the bounding box of the system, when it is present.
	  * You should call this instead of UInstance::getShapeAABBox() because the bbox may change over time, and thusn its shape
	  * doesn't hold any info on it...
	  * \param bbox a reference to the bbox to fill
	  * \return true if the bbox has been filled
	  * \see isPresent()
	  */
	bool		getSystemBBox(NLMISC::CAABBox &bbox);



	/// \name System parameters
	//@{
		/** Set the user color of the system. This color will be used to modulate the color of the whole system
		  * NB : even if the system is not instanciated, this will be taken in account the next time it is , so no need to call isSystemPresent() before calling that method.
		  */
		void			setUserColor(NLMISC::CRGBA userColor);
		NLMISC::CRGBA	getUserColor() const;
		/** Set a user param of the system. Each user param must be >= 0 and <= 1
		  * NB : even if the system is not instanciated, this will be taken in account the next time it is, so no need to call isSystemPresent() before calling that method.
		  * \param index the index of the user param to modify. For now it ranges from 0 to 3
		  * \value the new value of the parameter
		  */
		void			setUserParam(uint index, float value);

		/// Set a global user param value. User param in a system can mirror global values, which are identified by their name
		static  void    setGlobalUserParamValue(const std::string &name, float value);
		static  float	getGlobalUserParamValue(const std::string &name);
		/** Set a global vector value in the system.
		  * Some object in the system can bind their parameters to such a global value
		  * Example : direction of wind could be stored in the global variable 'WIND'
		  */
		static  void			setGlobalVectorValue(const std::string &name, const NLMISC::CVector &v);
		static  NLMISC::CVector	getGlobalVectorValue(const std::string &name);

		/** Get the value of a user param
		  * \param index the index of the user param to get. For now it ranges from 0 to 3
		  * \return the value of the user param (>= 0 and <= 1)
		  */
		float		getUserParam(uint index) const;

		// bypass the update of a user param from a global value if there is one
		void        bypassGlobalUserParamValue(uint userParamIndex, bool byPass = true);
		bool        isGlobalUserParamValueBypassed(uint userParamIndex) const;
	//@}

	 //@{
			/** All the emitters that have the given ID emit their target.
			  * \return false if the id is invalid, or if it isn't an emitter ID
			  */
			bool	emit(uint32 id, uint quantity = 1);
			/** All the object with the given id are removed.
			  * \return false if the id is invalid.
			  */
			bool   removeByID(uint32 id);
			/// Return the number of objects in the system that are flagged with an ID, or 0 if the system is not present
			uint   getNumID() const;
			/// Get the nth ID, or 0 if index is invalid.
			uint32 getID(uint index) const;
			/** Get all the IDs in the system.
			  * \warning As IDs are not stored in a vector, it is faster than several calls to getID
			  *
			  */
			bool   getIDs(std::vector<uint32> &dest) const;

			// Deactivate an object with the given ID
			bool   setActive(uint32 id, bool active);
			/** special : Activate / Deactivate all emitters.
			  * If the system isn't instanciated, then emitters will be deactivated the next time it is
			  */
			void   activateEmitters(bool active);
			// test if there are active emitters in the system
			bool	hasActiveEmitters() const;
	 //@}

	 //@{
			// Test if there are particles left. Always return false if the system is not present.
			bool   hasParticles() const;
			// Test if there are emitters left. Always return false if the system is not present.
			bool   hasEmmiters() const;
	 //@}

	 // Test if the system is shared
	 bool   isShared() const;

	 // Test if the system is valid
	 bool   isValid() const;

	 /** Set user matrix of the system. Passing NULL causes this matrix to be the same than the particle system matrix
	   *
	   * Particle can be located in various coordinate system :
	   * - in world (identity matrix)
	   * - local to the particle system (matrix of the particle system)
	   * - local to the coord. sys. defined by the user matrix
	   *
	   * NB : matrix is updated at next 'render'
	   */
	 void	setUserMatrix(const NLMISC::CMatrix &userMat);
	 // set the user matrix with instant update (is system present)
	 void	forceSetUserMatrix(const NLMISC::CMatrix &userMat);

	 /** Force to instanciate the system resource even if not visible. Useful for 'spell like' effects that need accurate timing.
	   * If not used, the fx would only start when it enters the camera, and thus could be late.
	   * The system must have persistence when it is not visible (for example be flagged as 'SpellFX) or the system will only persist for 1 frame.
	   * NB : no effect if the fx has been invalidated (because it is finished) or if it is already instanciated
	   * \todo detect the 'SpellFX' flag of fx at loading to automate this ? (not useful for projectile, though, so it may be better to let the decision to the caller for now..)
	   */
	 void   forceInstanciate();

	 /** Set z-bias for all objects in the particle system (except for meshs). Works even is isPresent() returns false.
	   * Value is in world coordinates.
	   */
	 void	setZBias(float value);

	 // debug : force to display all system bboxs
	 static void forceDisplayBBox(bool on);

	// Cast methods. If the cast fails, the object is empty.
 	void	cast(UInstance object);


	/** Shut down all sources in a particle system. Looping sources can finish their current occurence.
	  * Sound won't be played until reactivateSound() is called.
	  * NB : no effect if the system is not present
	  */
	void	stopSound();
	/** Reactivate sound sources. \see stopSound()
	  * NB : no effect if the system is not present
	  */
	void	reactivateSound();


	/// Proxy interface

	/// Constructors
	UParticleSystemInstance() { _Object = NULL; }
	UParticleSystemInstance(class CParticleSystemModel *object) { _Object = (ITransformable*)object; };
	/// Attach an object to this proxy
	void			attach(class CParticleSystemModel *object) { _Object = (ITransformable*)object; }
	/// Detach the object
	void			detach() { _Object = NULL; }
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const {return _Object==NULL;}
	/// For advanced usage, get the internal object ptr
	class CParticleSystemModel	*getObjectPtr() const {return (CParticleSystemModel*)_Object;}
};


} // NL3D


#endif // NL_U_PARTICLE_SYSTEM_INSTANCE_H

/* End of u_particle_system_instance.h */
