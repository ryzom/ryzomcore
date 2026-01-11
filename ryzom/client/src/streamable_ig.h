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




#ifndef CL_STREAMABLE_IG
#define CL_STREAMABLE_IG

#include "streamable_entity.h"
#include "ig_enum.h"

#include "nel/3d/u_scene.h"

#include <string>
#include <map>


namespace NL3D
{
	class UScene;
	class UInstanceGroup;
}

namespace NLMISC
{
	class CVector;
}

class CStreamableIG;

/** A streamable IG hierarchy (hierarchy is used to build clusters hierarchy). It may be loaded synchronously or asynchronously depending on the distance.
  */
class CStreamableIG	: public CStreamableEntity, public CIGNotifier
{
public:
	typedef std::map<std::string, NL3D::UInstanceGroup *> TString2IG;
	// for debug
	std::string				Name;
public:
	// default ctor
	CStreamableIG();
	// dtor
	~CStreamableIG();
	/** Init this streamable ig
	  * \param owningScene the scene in which that ig should be inserted
	  * \param pos center of the testing volume
	  * \param forceLoadRadius the radius at which loading should be forced
	  * \param loadRadius the radius at which asynchronous loading should start
	  * \param unloadRadius the radius at which the ig should be unloaded
	  */
	void						init(
									 NL3D::UScene *owningScene,
									 const NLMISC::CVector &pos,
									 float forceLoadRadius,
									 float loadRadius,
									 float unloadRadius
									);
	NL3D::UScene				*getScene() const { return _Scene; }

	/// Optimisation purpose. If you know how many calls to addIG will be done, you can call this
	void						reserve(uint size);
	/** Add an ig and the name of its parent ig (for clusters linking), or an empty name it no linking is done
	  */
	void						addIG(const std::string &name, const std::string &parentName, const NLMISC::CVector &pos, const NLMISC::CQuat &rot);
	/** Set an additionnal map (ig name -> ig pointer) that will be filled with loaded igs.
	  * Setting that pointer to NULL will remove currently loaded igs from the map and will disable it.
	  * By default, no map is used
	  */
	void						setLoadedIGMap(TString2IG *igMap);
	//
	virtual void				forceUnload();
	/** enum all currently instanciated IGs
	  * \return false if the enumeration has been stopped
	  */
	bool		enumIGs(IIGEnum *callback);

	// Change one ig
	bool		setIG(uint ig, const std::string &name, const std::string &parentName);

	// Get num of IG in the streamable IG..
	uint		getIGNum() const
	{
		return (uint)_IGs.size();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	class CCallback : public NL3D::IAsyncLoadCallback
	{
	public:
		CStreamableIG *Owner;
		virtual void InstanceGroupCreated(NL3D::UInstanceGroup *newVal)
		{
			Owner->notifyIGLoaded(newVal);
		}
	};

	struct CIGNode
	{
		std::string				Name;
		std::string				ParentName;
		NL3D::UInstanceGroup	*IG;
		bool					Loading;
		NLMISC::CVector			Pos;
		NLMISC::CQuat			Rot;
	};

	typedef std::vector<CIGNode> TIGArray;

private:

	NL3D::UScene			*_Scene;
	TIGArray				_IGs;
	bool					_Linked; // instance are linked in a tree of cluster
	TString2IG				*_IGMap;
	EGSPD::CSeason::TSeason		_Season;

	CCallback				_Callback;



private:

	//\name From CStreamableEntity
	//@{
		virtual void			loadAsync();
		virtual void			load(NLMISC::IProgressCallback &progress);
		virtual void			unload();
	//@}
		void					linkInstances();
		void					addLoadedIGToMap();
		void					removeLoadedIGFromMap();
};


#endif
