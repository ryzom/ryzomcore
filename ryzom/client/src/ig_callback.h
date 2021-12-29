// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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




#ifndef CL_IG_CALLBACK
#define CL_IG_CALLBACK

#include "nel/3d/u_instance_group.h"
#include "nel/misc/debug.h"

#include "ig_enum.h"
#include "timed_fx_manager.h"

#include <list>
#include <string>

namespace NL3D
{
	class UInstanceGroup;
}

namespace NLPACS
{
	class UMoveContainer;
	class UMovePrimitive;
	class UPrimitiveBlock;
}


class CEntitySheet;

/**  A class to manage callback (collisions, constructions ..) with igs of the landscape.
  *  Its purpose is to create pacs primitive for objects of the landscape that need them.
  *  The matching primitive for an instance is deduced from its shape name.
  *  Pacs primitives must be loaded at startup.
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CIGCallback	: public CIGNotifier
{
public:
	///\name Object
	//@{
		/// default ctor
		CIGCallback();
		/// dtor
		~CIGCallback();
	//@}

	///\name IG mgt
	//@{
		/** Add an ig. Its callbacks primitive will be created when it has been added
		  * to the scene
		  */
		void					addIG(NL3D::UInstanceGroup *ig);
		/** Add an ig with his num ZC. Its callbacks primitive will be created when it has been added
		  * to the scene
		  */
		void					addIGWithNumZC(NL3D::UInstanceGroup *ig, sint numZC);

		/** Add a vector of instance groups
		  */
		void					addIGs(const std::vector<NL3D::UInstanceGroup *> &igs);
		/** Add a vector of instance groups with the num ZC associated.
		  */
		void					addIGsWithNumZC(std::vector<std::pair<NL3D::UInstanceGroup *, sint> > &igs);
		/// Remove all IGs
		void					deleteIGs();
		/// Force creation of all zones (not to be used in client side..)
		void					forceAddAll();
	//@}

	///\name Collisions
	//@{
		/** Set the move container that should be used for collision. This MUST
		  * be called once and only once, unless it is reseted
		  */
		void					setMoveContainer(NLPACS::UMoveContainer *mc);
		// Get the move container associated with that object, or NULL if none.
		NLPACS::UMoveContainer  *getMoveContainer() const { return _MoveContainer; }

		/** Add a pacs primitive from its file name.
		  * This may raise an exception if loading failed.
		  */
		void					addPacsPrim(const std::string &fileName);
		/** Add all pacs primitives from the given directory.
		  */
		void					resetContainer();
	//@}

	///\name Enumeration
	//@{
		/** enumerate all currently loaded zone igs
		  * \return false if the enumeration has been stopped
		  */
		bool					enumIGs(IIGEnum *callback);
	//@}

	///\name Season change
	//@{
		// apply change of season on iug (for now, only change the fxs to match the season)
		void					changeSeason();
	//@}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
private:
	class CIGInstance;
	friend class CIGInstance;
	// instanciated igs
	typedef std::vector<CIGInstance *>    TIGInstanceList;
	/**
	 * An instanciated ig and its matching collisions primitives
	 */
	class CIGInstance : public NL3D::IAddRemoveInstance,
						public NL3D::ITransformName,
						public NL3D::IIGAddBegin
	{
		public:
			/// ctor
			CIGInstance(NL3D::UInstanceGroup *ig, CIGCallback	*owner);
			/** Dtor
			  * NB:
			  * -This release move primitives from the move container
			  */
			~CIGInstance();
			/// force this instance to be added
			void forceAdd() { instanceGroupAdded(); }
			void numZC(sint num) {_NumZC = num;}
			NL3D::UInstanceGroup *getIG() const { return _IG; }
			void updateManagedFXs();
			void shutDownFXs();
			bool hasManagedFXs() const { return _HasManagedFXs; }
			// force to rebuild the sheet vector
			void buildSheetVector();
			// earse the sheet vector
			void eraseSheetVector();
		private:
			typedef std::vector<NLPACS::UMovePrimitive *> TMovePrimitiveVect;
			typedef std::vector<CEntitySheet *>	          TEntitySheetVect;
		private:
			void releaseMovePrimitives();
			///\name from NL3D::IAddRemoveInstance
			//@{
				virtual void instanceGroupAdded();
				virtual void instanceGroupRemoved();
			//@}
			///\name from NL3D::ITransformName
			//@{
				virtual std::string transformName (uint index, const std::string &instanceName, const std::string &shapeName);
			//@}
			///\name from NL3D::IIGAddBegin
			//@{
				virtual void startAddingIG(uint numInstances);
			//@}
			/** Called after all instance have been added, and when their sheets have been retrieved.
			  * This is the place to setup parameters from sheets
			  */
			void updateFromSheets();
			//
		private:

			TMovePrimitiveVect				_MovePrimitives;
			TEntitySheetVect				_EntitySheets; // matching sheets for each instance (or NULL), they are used only after the instance have been added to the scene
			CIGCallback						*_Owner;
			NL3D::UInstanceGroup			*_IG;		// the IG we're looking at
			sint							_NumZC;		// >= 0 if valid.
			bool							_HasManagedFXs; // true if there are managed fx in the group, otherwise, _ManagedFXHandle is not used
			CTimedFXManager::TFXGroupHandle _ManagedFXHandle;
	};

private:
	NLPACS::UMoveContainer	*_MoveContainer;
	TIGInstanceList			_IGInstances;
};





/** Debug purpose only: create instances in a scene from the content of a move container.
  * The created instances are static. Their pointer can be retrieved in a pointer
  * The following shapes must be available :
  * unit_box.shape : a box of 1 x 1 x 1 centered at the origin
  * unit_cylinder.shape : a cylinder of height 1, radius 1
  */
 void createInstancesFromMoveContainer(NL3D::UScene *scene, NLPACS::UMoveContainer *mc, std::vector<NL3D::UInstance> *instances = NULL);





#endif
