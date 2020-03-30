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

#ifndef NL_U_GLOBAL_RETRIEVER_H
#define NL_U_GLOBAL_RETRIEVER_H

#include "nel/misc/types_nl.h"

#include "u_retriever_bank.h"
#include "u_global_position.h"

namespace NLMISC
{
class CVector;
class CVectorD;
class CAABBox;
class CLine;
}

namespace NLPACS
{

class UGlobalPosition;

/**
 * A class that allows to retrieve surface in a large amount of zones (referred as instances.)
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UGlobalRetriever
{
public:
	virtual ~UGlobalRetriever() {}

	/// Make a raytrace test. For the time, always return false.
	virtual bool					testRaytrace (const NLMISC::CVectorD &v0, const NLMISC::CVectorD &v1) =0;

	/**
	  * Return the bounding box of the global retriever.
	  */
	virtual const NLMISC::CAABBox	&getBBox() const=0;

	/**
	  * Return the average height for a global position
	  */
	virtual float					getMeanHeight(const UGlobalPosition &pos) const =0;

	/**
	  * Return the retriever id from the string id
	  * \return a valid retriever id or -1 if failed
	  */
	virtual sint32					getIdentifier(const std::string &id) const =0;

	/**
	  * Returns a human readable identifier of the global position
	  */
	virtual const std::string		&getIdentifier(const UGlobalPosition &pos) const =0;

	/// Get the LocalRetrieverId of the global position.
	virtual sint32					getLocalRetrieverId(const UGlobalPosition &position) const =0;

	/**
	  * Builds an instance of retriever, and link it on the ground (or wherever)
	  * \param id a valid retriever id to be instanciated
	  * \param a valid position where the retriever should be instanciated
	  * \return false if failed
	  */
	virtual bool					buildInstance(const std::string &id, const NLMISC::CVectorD &position, sint32 &instanceId) =0;

	/**
	  * Removes an instance of retriever (perform all unlinks necessary)
	  */
	virtual void					removeInstance(sint32 instanceId) =0;

	/**
	  * Returns the material corresponding to the global position
	  */
	virtual uint32					getMaterial(const UGlobalPosition &pos) const =0;

	/**
	  * Retrieves the position of an estimated point in the global retriever (double instead.),
	  * with specification of whether it is on landscape or in interior
	  */
	UGlobalPosition					retrievePosition(const NLMISC::CVectorD &estimated, double threshold, UGlobalPosition::TType retrieveSpec) const;

	/**
	  * Retrieves the position of an estimated point in the global retriever.
	  */
	virtual UGlobalPosition			retrievePosition(const NLMISC::CVector &estimated) const =0;

	/**
	  * Retrieves the position of an estimated point in the global retriever (double instead.)
	  */
	virtual UGlobalPosition			retrievePosition(const NLMISC::CVectorD &estimated) const =0;

	/**
	  * Retrieves the position of an estimated point in the global retriever. Uses a snapping threshold.
	  */
	virtual UGlobalPosition			retrievePosition(const NLMISC::CVector &estimated, float threshold) const =0;

	/**
	  * Retrieves the position of an estimated point in the global retriever (double instead.) with a snapping threshold
	  */
	virtual UGlobalPosition			retrievePosition(const NLMISC::CVectorD &estimated, double threshold) const =0;

	/**
	  * Checks pos is valid (e.g. is really inside the surface it points to)
	  */
	virtual bool					testPosition(UGlobalPosition &pos) const =0;

	/**
	  * Insure position inside a surface
	  */
	virtual bool					insurePosition(UGlobalPosition &pos) const =0;

	/**
	  * Tests if the global position is a interior position
	  */
	virtual bool					isInterior(const UGlobalPosition &pos) const =0;
	/**
	  * Tests if the global position is immerged
	  */
	virtual bool					isWaterPosition(const UGlobalPosition &pos, float &waterHeight) const =0;

	///
	virtual float					distanceToBorder(const UGlobalPosition &pos) const =0;
	///
	virtual void					getBorders(const UGlobalPosition &pos, std::vector<std::pair<NLMISC::CLine, uint8> > &edges) =0;
	virtual void					getBorders(const NLMISC::CAABBox &box, std::vector<std::pair<NLMISC::CLine, uint8> > &edges) =0;

	/**
	  * For interior position only, snap the position to the ground.
	  */
//	virtual void					snapToInteriorGround(UGlobalPosition &pos) const = 0;

	/**
	  * Converts a global position object into a 'human-readable' CVector.
	  */
	virtual NLMISC::CVector			getGlobalPosition(const UGlobalPosition &global) const =0;

	/**
	  * Converts a global position object into a 'human-readable' CVector (double instead.)
	  */
	virtual NLMISC::CVectorD		getDoubleGlobalPosition(const UGlobalPosition &global) const =0;

	/**
	  * Refresh loaded retrievers around a position (one retriever is loaded at a time)
	  */
	virtual void					refreshLrAround(const NLMISC::CVector &position, float radius) =0;

	/**
	  * Refresh loaded retrievers around a position (all retrievers are updated at this time -- used at startup)
	  */
	virtual void					refreshLrAroundNow(const NLMISC::CVector &position, float radius) =0;

	/**
	  * Create a global retriever.
	  *
	  * \param globalRetriver is the global retriever path file name. This method use the CPath to find the file.
	  * \param retrieverBank is the global retriever bank associated to the global retriever.
	  * \return the pointer on the global retriever or NULL if the file is not found.
	  */
	static UGlobalRetriever	*		createGlobalRetriever (const char* globalRetriever, const URetrieverBank* retrieverBank);

	/**
	  * Delete a global retriever.
	  */
	static void						deleteGlobalRetriever (UGlobalRetriever *retriever);
};


} // NLPACS


#endif // NL_U_GLOBAL_RETRIEVER_H

/* End of u_global_retriever.h */
