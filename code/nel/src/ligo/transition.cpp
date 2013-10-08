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

#include "stdligo.h"
#include "transition.h"

// Ligo include
#include "zone_template.h"
#include "ligo_error.h"
#include "ligo_material.h"

namespace NLLIGO
{

// ***************************************************************************

sint32 CTransition::TransitionZoneEdges[TransitionZoneCount][4]=
{
	{+2, +2, -4, +4},	// 0
	{+4, +2, +2, -3},	// 1
	{+1, +3, +2, -3},	// 2
	{-3, +3, +2, +2},	// 3
	{+1, +1, +3, -4},	// 4
	{+1, +4, -4, +1},	// 5
	{+4, +2, -4, +1},	// 6
	{+4, +2, -3, +1},	// 7
	{+3, -3, +1, +1},	// 8
};

// ***************************************************************************

sint32 CTransition::TransitionZoneOffset[TransitionZoneCount][2]=
{
	{0, 1},		// 0
	{1, 1},		// 1
	{1, 2},		// 2
	{1, 3},		// 3
	{0, 3},		// 4
	{0, 4},		// 5
	{1, 4},		// 6
	{2, 4},		// 7
	{3, 4},		// 8
};

// ***************************************************************************

bool CTransition::build (const CMaterial &mat0, const CMaterial &mat1, const std::vector<const CZoneTemplate*> &arrayTemplate,
						 const CLigoConfig &config, CLigoError *errors, CLigoError &mainErrors)
{
	// Check size
	if ((arrayTemplate.size() != TransitionZoneCount))
	{
		// Error message
		mainErrors.MainError = CLigoError::UnknownError;
		return false;
	}

	// Ok ?
	bool ok=true;

	// Check zone template edge count
	uint i;
	for (i=0; i<TransitionZoneCount; i++)
	{
		// Must have 4 edges
		if (arrayTemplate[i]&&(arrayTemplate[i]->getEdges ().size()!=4))
		{
			// Error code
			mainErrors.MainError = CLigoError::MustHave4Edges;
			errors[i].MainError = CLigoError::MustHave4Edges;
		}
	}

	// continue ?
	if (ok)
	{
		// Get first edge
		_EdgeZone[0]=mat0.getEdge ();

		// Get second edge
		_EdgeZone[1]=mat1.getEdge ();

		// For the two others edges
		for (sint32 k=2; k<(sint32)ZoneEdgeCount; k++)
		{
			// Get the first third edge found
			for (i=0; i<TransitionZoneCount; i++)
			{
				// This template exist ?
				if (arrayTemplate[i])
				{
					uint j;
					for (j=0; j<4; j++)
					{
						// Get edge
						sint32 edge = TransitionZoneEdges[i][j];

						// The good edge ?
						if (((edge==k+1)||(edge==-(k+1))))
						{
							// Back the ege
							_EdgeZone[k]=arrayTemplate[i]->getEdges ()[j];

							// Invert it if negative
							if (edge<0)
							{
								// Invert
								_EdgeZone[k].invert (config);
							}

							// Break
							break;
						}
					}
					if (j<4)
						break;
				}
			}
		}

		// Get the inverted edges
		CZoneEdge invertedEdges[4];
		for (i=0; i<4; i++)
		{
			// Copy the edge
			invertedEdges[i]=_EdgeZone[i];

			// Invert it
			invertedEdges[i].invert(config);
		}

		// false if can't build because some template are missing
		bool build=true;

		// Now check each zones against the edges
		for (i=0; i<TransitionZoneCount; i++)
		{
			// Template present ?
			if (arrayTemplate[i])
			{
				// For each edge
				for (uint j=0; j<4; j++)
				{
					// Get the edge number
					sint32 edge=TransitionZoneEdges[i][j];

					// Compare the edge
					if (edge<0)
					{
						// The same edge ?
						if (!invertedEdges[-edge-1].isTheSame (arrayTemplate[i]->getEdges()[j], config, errors[i]))
						{
							ok=false;
						}
					}
					else
					{
						// The same edge ?
						if (!_EdgeZone[edge-1].isTheSame (arrayTemplate[i]->getEdges()[j], config, errors[i]))
						{
							ok=false;
						}
					}
				}
			}
			else
				// Can't build
				build=false;
		}

		// Ok to build ?
		if (ok && build)
		{
		}
	}

	// Return error code
	return ok;
}

// ***************************************************************************

void CTransition::serial (NLMISC::IStream &s)
{
	// Serial the main node
	s.xmlPush ("LIGO_TRANSITION");

		// Serial the header
		s.serialCheck (NELID("STGL"));

		// Serial the version
		/*sint ver =*/ s.serialVersion (0);

		// Serial the edgezones
		uint i;
		s.xmlPush ("EDGE_ZONES");
			for (i=0; i<ZoneEdgeCount; i++)
				s.xmlSerial (_EdgeZone[i], "ELM");
		s.xmlPop ();

	// Close the main node
	s.xmlPop ();
}

// ***************************************************************************

bool CTransition::check (const CZoneTemplate &zoneTemplate, uint transitionNumber, const CLigoConfig &config, CLigoError &errors) const
{
	// Return value
	bool ok = true;

	// For each edge
	for (uint j=0; j<4; j++)
	{
		// Get the edge number
		sint32 edge=TransitionZoneEdges[transitionNumber][j];

		// Compare the edge
		if (edge<0)
		{
			// Invert the edge
			CZoneEdge invertedEdges = _EdgeZone[-edge-1];
			invertedEdges.invert(config);

			// The same edge ?
			if (!invertedEdges.isTheSame (zoneTemplate.getEdges()[j], config, errors))
			{
				ok=false;
			}
		}
		else
		{
			// The same edge ?
			if (!_EdgeZone[edge-1].isTheSame (zoneTemplate.getEdges()[j], config, errors))
			{
				ok=false;
			}
		}
	}

	// Return status
	return ok;
}

// ***************************************************************************

}
