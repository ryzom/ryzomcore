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

#ifndef NL_TRANSITION_H
#define NL_TRANSITION_H

#include "nel/misc/types_nl.h"

// Ligo include
#include "zone_edge.h"

// NeL include
//#include "3d/zone.h"

namespace NLLIGO
{

class CZoneTemplate;
class CMaterial;

/**
 * A transition template
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CTransition
{
public:

	/// Some define
	enum
	{
		ZoneEdgeCount = 4,
		TransitionZoneCount = 9,
	};


	/** Build method
	  *
	  *
		We need 9 zone templates to create a transition set

		0
		********
		*1*0000*
		*1*0000*
		*1*0000*
		*1*0000*
		*1******
		*111111*
		********

		1
		********
		*0*1111*
		*0*1111*
		*0*1111*
		*0*1111*
		***1111*
		*111111*
		********

		2
		********
		*0*1111*
		*0*1111*
		*0*1111*
		*0*1111*
		*0*1111*
		********

		3
		********
		*111111*
		*111111*
		*111111*
		*111111*
		***1111*
		*0*1111*
		********

		4
		********
		*0000*1*
		*0000*1*
		*0000*1*
		*0000*1*
		*0000***
		*000000*
		********

		5
		********
		*000000*
		*000000*
		*000000*
		*000000*
		*0000***
		*0000*1*
		********

		6
		********
		*000000*
		*000000*
		*000000*
		*000000*
		********
		*111111*
		********

		7
		********
		*000000*
		*0000***
		*000*11*
		*00*111*
		***1111*
		*111111*
		********

		8
		********
		*000000*
		******0*
		*1111*0*
		*1111*0*
		*1111*0*
		*1111*0*
		********

		The nine zones must be assembled like this:

		Oy

		^
		|
		+-+-+-+-+
		|5|6|7|8|
		+-+-+-+-+
		|4|3| | |
		+-+-+-+-+
		| |2| | |
		+-+-+-+-+
		|0|1| | |
		-----------> Ox
	  *
	  * \param tplt0 is the material lingo config file
	  * \param config is the current lingo config file
	  * \param arrayTemplate is an array of ligo zone template pointer of size 9. If a pointer is NULL, checks will be done on non NULL pointer
	  * but build will not be done.
	  * \param arrayZone is an array of nel zone pointer of size 9. If a pointer is NULL, checks will be done but build will not be done.
	  * \param config is the current lingo config file
	  * \param errors is an array of error structure of size 9. One error structure by zone.
	  * \return true if check success false if problem detected. Build is done if all the 18 pointers are not NULL.
	  */
	bool build (const CMaterial &mat0, const CMaterial &mat1, const std::vector<const CZoneTemplate*> &arrayTemplate,
				const CLigoConfig &config, CLigoError *errors, CLigoError &mainErrors);

	/**
	  * Check if a transition zone template match with this transition template.
	  *
	  * \param zoneTemplate is a zone template.
	  * \param transition number is the number of the transition to test (0 ~ 8)
	  * \param config is the current lingo config file
	  * \param errors is an error handler filled with error code and message if the method return false.
	  *
	  * \return true if check success false if problem detected. Errors are reported in the error[0].
	  */
	bool check (const CZoneTemplate &zoneTemplate, uint transitionNumber, const CLigoConfig &config, CLigoError &errors) const;

	/// Serial
	void serial (NLMISC::IStream &s);

private:

	/** The 4 Edges that define the transitions set
	  *
	  * the 4 transitions are:
	  * 0 : *000000*
	  * 1 : *111111*
	  * 2 : *0*1111*
	  * 3 : *0000*1*
	  */
	CZoneEdge		_EdgeZone[ZoneEdgeCount];

	/// Some static arries
	static	sint32 TransitionZoneEdges[TransitionZoneCount][4];
	static	sint32 TransitionZoneOffset[TransitionZoneCount][2];
};

}

#endif // NL_TRANSITION_H

/* End of transition.h */
