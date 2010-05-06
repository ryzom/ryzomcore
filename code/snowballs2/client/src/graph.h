/** \file graph.h
 * Snowballs 2 specific code for managing the graph (network traffic, fps, etc).
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GRAPH_H
#define GRAPH_H

//
// Includes
//

#include <deque>
#include <string>

#include <nel/misc/rgba.h>
#include <nel/misc/time_nl.h>

namespace SBCLIENT {

//
// External classes
//

class CGraph
{
public:
	std::string Name;
	float X, Y, Width, Height;
	NLMISC::CRGBA BackColor;
	float MaxValue;
	float Peak;

	std::deque<float> Values;

	NLMISC::TTime Quantum;

	NLMISC::TTime CurrentQuantumStart;

	CGraph (std::string name, float x, float y, float width, float height, NLMISC::CRGBA backColor, NLMISC::TTime quantum, float maxValue)
		: Name(name), X(x), Y(y), Width(width), Height(height), BackColor(backColor), MaxValue(maxValue),
		Peak(0.0f), Quantum(quantum), CurrentQuantumStart(NLMISC::CTime::getLocalTime())
	{
	}

	void render ();
	void addOneValue (float value = 0.0f);
	void addValue (float value);
};

//
// External variables
//

extern CGraph FpsGraph;
extern CGraph SpfGraph;
extern CGraph DownloadGraph;
extern CGraph UploadGraph;

//
// External functions
//

void initGraph ();
void updateGraph ();
void releaseGraph ();

} /* namespace SBCLIENT */

#endif // GRAPH_H

/* End of graph.h */

/* end of file */
