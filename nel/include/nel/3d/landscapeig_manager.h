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

#ifndef NL_LANDSCAPEIG_MANAGER_H
#define NL_LANDSCAPEIG_MANAGER_H

#include "nel/misc/types_nl.h"
#include <vector>
#include <map>
#include <string>


namespace NLMISC
{
class IProgressCallback;
}

namespace NL3D
{
class	UInstanceGroup;
class	UScene;
class	UDriver;


// ***************************************************************************
/**
 * This class is used to load and unload IG of landscape. Used in conjunction with ULandscape.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLandscapeIGManager
{
public:

	/// Constructor
	CLandscapeIGManager();
	~CLandscapeIGManager();

	/** Initialize and load Instances Group from a text file. no-op if not found.
	 *	the file is just a list of filename eg: "150_EM.ig", separated with return.
	 *	At this time, All UInstanceGroup are loaded, but none are addToScene() ed.
	 */
	void	initIG(UScene *scene, const std::string &igDesc, UDriver *driver, uint selectedTexture, NLMISC::IProgressCallback *callBack);

	/** load of an instanceGroup of a zone. name is like "150_EM". no-op if "".
	 *	If exist (see initIG), The instanceGroup is added to the scene.
	 *	call after ULandscape::refreshZonesAround()
	 */
	UInstanceGroup *loadZoneIG(const std::string &name);

	/** same as prec loadZoneIG, but with an array of name. no-op if "".
	 *	call after ULandscape::loadAllZonesAround().
	 * \param names the igs to load
	 * \param dest if not NUL, pointer to the created igs
	 */
	void	loadArrayZoneIG(const std::vector<std::string> &names, std::vector<UInstanceGroup *> *dest = NULL);

	/** unload of an instanceGroup of a zone. name is like "150_EM". no-op if "".
	 *	If exist (see initIG), The instanceGroup is removed from the scene.
	 *	call after ULandscape::refreshZonesAround()
	 */
	void	unloadZoneIG(const std::string &name);

	/** same as prec unloadZoneIG, but with an array of name. no-op if "".
	 *	call after ULandscape::refreshAllZonesAround().
	 */
	void	unloadArrayZoneIG(const std::vector<std::string> &names);

	/// is the Ig added to scene? name is like "150_EM". false if not found.
	bool	isIGAddedToScene(const std::string &name) const;

	/// get the Ig. name is like "150_EM". NULL if not found.
	UInstanceGroup	*getIG(const std::string &name) const;

	/// unload and delete all the zoneIgs. Call before deletion of the scene (else exception).
	void	reset();


	/// reload all the Igs and re-add to scene.
	void	reloadAllIgs();

	/// get a list of the ig
	void			getAllIG(std::vector<UInstanceGroup	*> &dest) const;

	/// get a list of the ig, with their names
	void			getAllIGWithNames(std::vector<std::pair<UInstanceGroup *, std::string> > &dest) const;


private:
	UScene			*_Scene;
	struct			CInstanceGroupElement
	{
		UInstanceGroup	*Ig;
		bool			AddedToScene;
		std::string		FileName;

		CInstanceGroupElement(UInstanceGroup	*ig= NULL, const char *fileName= NULL);

		// delete the ig.
		void	release();
	};

	typedef	std::map<std::string, void*>	TShapeMap;
	typedef	std::map<std::string, CInstanceGroupElement>	TZoneInstanceGroupMap;
	typedef	TZoneInstanceGroupMap::iterator					ItZoneInstanceGroupMap;
	typedef	TZoneInstanceGroupMap::const_iterator			ConstItZoneInstanceGroupMap;
	TZoneInstanceGroupMap			_ZoneInstanceGroupMap;
	TShapeMap						_ShapeAdded;

	std::string		translateName(const std::string &name) const;
};


} // NL3D


#endif // NL_LANDSCAPEIG_MANAGER_H

/* End of landscapeig_manager.h */
