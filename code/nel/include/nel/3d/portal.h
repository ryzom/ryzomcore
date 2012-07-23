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

#ifndef NL_PORTAL_H
#define NL_PORTAL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"
#include "nel/misc/quat.h"
#include <vector>

namespace NLMISC
{
class CRGBA;
class IStream;
struct EStream;
class CMatrix;
}

namespace NL3D {

class CCluster;

/**
  * CPortal
  *
  * \author Matthieu Besson
  * \author Nevrax France
  * \date 2001
  */
class CPortal
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/


public:

	CPortal();

	/**
	 * Clip the view pyramid (all planes pass through observer point) against the portal
	 * \return false if the pyramid is completly clipped
	 */
	bool clipPyramid (NLMISC::CVector &observer, std::vector<NLMISC::CPlane> &pyramid);

	/// Return true if the vertex v is in front of the portal
	bool isInFront (NLMISC::CVector &v);

	/// Accessors
	/// ---------

	/// set all link to clusters to NULL.
	void resetClusterLinks();

	/// return false if more than 2 clusters added
	bool setCluster (CCluster *cluster);

	/// return the number of clusters linked to the portal
	uint8 getNbCluster ();

	/// return the cluster linked to the portal
	CCluster* getCluster (uint pos) { return _Clusters[pos]; }

	/// return false if the polygon is not convex
	bool setPoly (const std::vector<NLMISC::CVector> &poly);

	/// get this cluster poly in local coordinates
	void getPoly (std::vector<NLMISC::CVector> &dest) const;
	const std::vector<NLMISC::CVector> &getPoly()		{return _LocalPoly;}

	/// Serial
	void serial (NLMISC::IStream& f);

	void setWorldMatrix (const NLMISC::CMatrix &WM);


	void setName (std::string &name) { _Name = name; }

	std::string getName () { return _Name; }

	void open (bool opened) { _Opened = opened; }
	bool isOpened () { return _Opened; }

	//\name Sound related
	//@{
	void				setOcclusionModel(const std::string &occlusionModel);
	const std::string	&getOcclusionModel();
	NLMISC::TStringId	getOcclusionModelId();
	void				setOpenOcclusionModel(const std::string &occlusionModel);
	const std::string	&getOpenOcclusionModel();
	NLMISC::TStringId	getOpenOcclusionModelId();
	//@}

	/// return true if the portal clip the segment
	bool				clipRay(const NLMISC::CVector &start, const NLMISC::CVector &end);

private:

	CCluster* _Clusters[2];

	std::string _Name;
	bool _Opened;

	std::vector<NLMISC::CVector> _LocalPoly;
	std::vector<NLMISC::CVector> _Poly;

//	std::string		_OcclusionModel;
//	std::string		_OpenOcclusionModel;
	NLMISC::TStringId		_OcclusionModelId;
	NLMISC::TStringId		_OpenOcclusionModelId;


	/// Friend class
	friend class CInstanceGroup;
};

// ***************************************************************************



} // NL3D


#endif // NL_PORTAL_H

/* End of portal.h */
