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

#ifndef NL_OCCLUSION_QUERY
#define NL_OCCLUSION_QUERY


namespace NL3D
{

/** Interface to an occlusion query object.
  * An occlusion query allows to know whether some pixels passed the z-test for a set of primitives.
  * Common usages include visibility determination, lens flares / coronas rendering etc.
  *
  * Usage :
  * \code
  * IOcclusionQuery *oq = drv->createOcclusionQuery();
  * oq->begin();
  * // render some primitives... usually with color/z-buffer writes  disabled
  * oq->end();
  *
  * switch(oq->getOcclusionType())
  * {
  *     case IOcclusionQuery::NotAvailable:
  *       // Defer test. One should ideally wait the next frame.
  *       // Note that polling the result in a wait loop isn't safe, unless the driver is flushed.
  *		break;
  *     case IOcclusionQuery::Occluded:
  *       // no pixels passed the z-test
  *     break;
  *     case IOcclusionQuery::NotOccluded:
  *       // some pixels passed the z-test
  *     break;
  * };
  *
  * \endcode
  *
  * After one's finished with IOcclusionQuery, IDriver::deleteOcclusionQuerry should be called
  *
  * IMPORTANT : - Occlusion query objects are invalid after a driver has been deleted, and thus must be recreated when driver changes.
  *             - Only one occlusion query object can be active at a time, e.g IOcclusionQuery::begin() should not be called
  *               within another begin() / end() pair. Such a situation will cause an assertion.
  *             - Not all drivers support the occlusion query mechanism. This should be checked by calling IDriver::supportOcclusionQuery.
  *               If not supported, the application can fallback to IDriver::getZBuffer, at the risk of stalling the GPU.
  *             - For a given query, result should NOT be retrieved between a begin()/end() pair, otherwise an assertion is raised
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2004
  */
struct IOcclusionQuery
{
	enum TOcclusionType
	{
		NotAvailable = 0, // occlusion test result isn't available yet
		Occluded,
		NotOccluded
	};

	virtual ~IOcclusionQuery() { }

	/** Begin occlusion test
      * Should be called prior to primitive rendering
	  */
	virtual void begin() = 0;
	// End occlusion test
	virtual void end() = 0;
	/** Get the result of the occlusion test
	  * NB Do not call this between a begin()/end() pair for that query object (ok for other querries, though).
	  */
	virtual TOcclusionType getOcclusionType() = 0;
	/** Get an indication about how many samples passed the z-test.
	  * Depending on the implementation ,this may be the right number of sample, or just 0/1
	  */
	virtual uint getVisibleCount() = 0;
};


} // NL3D

#endif
