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



#ifndef CL_DOOR_MANAGER_H
#define CL_DOOR_MANAGER_H

#include "ig_enum.h"

// ///////////// //
// Doors Manager //
// ///////////// //
class CEntityCL;

class CDoorManager : public NL3D::ITransformName
{
	static uint32 s_nextId;

	struct SDoor
	{
		uint32					ID;
		std::string				Name;				// Name of the door (type_id ie: ma_asc_3portes_02)
		std::vector<NLPACS::UMovePrimitive *> Prims;// All collisions prims for that door
		NL3D::UInstanceGroup	*InstanceGroup;		// Instance Group where the door is.
		std::vector<uint32>		Instances;			// Shapes making the door
		std::vector<NLMISC::CMatrix> InstMat;		// Matrix of the shapes making the door
		bool					Opened;				// Is the door is opened or closed ?
		std::vector<CEntityCL*>	Entities;			// Entities in the trigger zone
		std::vector<uint8>		EntitiesMoved;		// Entities in the trigger zone that moved
		float					OCState;			// 0 closed 1 opened

		enum TAnimType {	Normal,
							Matis3Part,			// For Instance Parts: 0==left 1==right 2==down
							Matis3PartBourgeon,	// For Instance Parts: 0==left 1==right 2==down
							Zorai2Part
						};
		TAnimType				AnimType;


		SDoor()
		{
			ID = ++s_nextId;
			InstanceGroup = NULL;
			Opened = false;
			OCState = 0;
			AnimType = Normal;
		}

		void entityCollide(CEntityCL *pE);

		void checkToClose();

		// Animate the door's instances from current state to open state
		// Return true if the anim is finished
		bool open();

		// Animate the door's instances from current state to open state
		// Return true if the anim is finished
		bool close();

		// Called by open/close
		void anim();
	};

public:

	/// Singleton method : Get the unique interface loader instance
	static CDoorManager* getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CDoorManager();
		return _Instance;
	}

	// release memory
	static void releaseInstance();

	void loadedCallback (NL3D::UInstanceGroup *ig);

	// Check in the group if there are some doors
	void addedCallback (NL3D::UInstanceGroup *ig);

	// Remove all doors attached to this group
	void removedCallback (NL3D::UInstanceGroup *ig);

	// Copy triggers to be used in update
	void getPACSTriggers();

	// Check triggers in pacs to open/close the doors
	void update ();

private:

	virtual std::string transformName (uint index, const std::string &instanceName, const std::string &shapeName);

private:

	static CDoorManager *_Instance;

	std::vector<SDoor*> _Doors;
};
// shortcut to access the manager
inline CDoorManager &getDoorManager() { return *CDoorManager::getInstance(); }

// ///////// //
// Callbacks //
// ///////// //

class CIGDoorAddedCallback : public IIGObserver
{
private:
	// An IG has been added
	virtual void instanceGroupLoaded(NL3D::UInstanceGroup *ig)
	{
		getDoorManager().loadedCallback (ig);
	}
	// An IG has been added
	virtual void instanceGroupAdded(NL3D::UInstanceGroup *ig)
	{
		getDoorManager().addedCallback (ig);
	}
	// An IG will be removed
	virtual void instanceGroupRemoved(NL3D::UInstanceGroup *ig)
	{
		getDoorManager().removedCallback (ig);
	}
};

extern CIGDoorAddedCallback		IGDoorCallback;

#endif // CL_DOOR_MANAGER_H
