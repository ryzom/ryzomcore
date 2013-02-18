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






#ifndef CL_VILLAGE_H
#define CL_VILLAGE_H


#include "nel/misc/types_nl.h"
#include "streamable_ig.h"
#include "nel/misc/cdb.h"
#include "client_sheets/continent_sheet.h"
#include "game_share/misc_const.h"
#include <vector>

namespace NL3D
{
	class UScene;
}

namespace NLGEORGES
{
	class UFormElm;
}

struct IIGAdded;


struct CVillageSheet;

/** A village in a continent.
  * This class holds a village instance group, and manage its loading / unloading.
  */
class CVillage : public IStreamableEntity
{
public:
	//\name Object
	//@{
		// ctor
		CVillage();
		// setup the village from a sheet
		/** Build that village from an external script.
		  * And insert in the given scene
		  * \param scene The scene in which this village is
		  * \param sheet Sheet describing the village
		  * \param loadedIGMap If not NULL, a map that will be filled with ig that are currently loaded
		  * \return true if the build succeed
		  */
		bool setupFromSheet(NL3D::UScene *scene, const CVillageSheet &sheet, CStreamableIG::TString2IG *loadedIGMap = NULL);

		// dtor
		~CVillage();
	//@}

	//\name From IStreamableEntity
	//@{
		/** Given a pos, test whether the village needs to be loaded now.
		 * It it returns false, it means that the village is too far or that asynchronous loading suffice.
		 * It it returns true, the next call to update will return only when the loading is completed.
		 */
		virtual bool		 needCompleteLoading(const NLMISC::CVector &pos) const;
		/** Test that village against the player position, and load / unload it (using synchronous or asynchronous loading)
		  */
		virtual void		 update(const NLMISC::CVector &pos);
		// The same, but force synchronous loading
		virtual void		 forceUpdate(const NLMISC::CVector &pos, NLMISC::IProgressCallback &progress);
		virtual void		 forceUnload();
	//@}

	// The scene in which this object is inserted
		NL3D::UScene		 *getScene() const { return _Scene; }

	///\name ig added observers
	//@{
		void registerObserver(IIGObserver *obs);
		void removeObserver(IIGObserver *obs);
		bool isObserver(IIGObserver *obs) const;
	//@}

	// ig enumeration
	bool		enumIGs(IIGEnum *callback);

	const CStreamableIG &getIG() const { return _IG; }

	///\name Special Outpost
	//@{
	// setup the village as an Outpost (similar to setupFromSheet())
	bool setupOutpost(NL3D::UScene *scene, const CContinentParameters::CZC &zone, sint32 outpost, CStreamableIG::TString2IG *loadedIGMap);
	// setup a building pos in this village
	void setBuildingPosition (uint building, const NLMISC::CQuat &rot, const NLMISC::CVector &pos);
	// after outpost and building position setuped, can init the outpost into 3D
	void initOutpost ();
	// remove the outpost from 3D
	void removeOutpost ();
	// Is this village an outpost?
	bool isOutpost() const {return _IsOutpost;}
	//@}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	// TODO: if slow, make the pos test only once for all IGs...
	CStreamableIG			_IG;
	NL3D::UScene			*_Scene;

	///\name Outpost
	// @{
	bool					_IsOutpost;
	class CBuilding
	{
	public:
		uint				Id;
		NLMISC::CSheetId	CurrentSheetId;
		NLMISC::CVector		Position;
		NLMISC::CQuat		Rotation;
		CBuilding()
		{
			Position= NLMISC::CVector::Null;
			Rotation= NLMISC::CQuat::Identity;
			Id= -1;
		}
	};
	CBuilding				_Buildings[RZ_MAX_BUILDING_PER_OUTPOST];
	void	updateBuilding(uint building, NLMISC::CSheetId newForm);
	// @}

};


#endif

