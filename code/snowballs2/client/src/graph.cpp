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

//
// Includes
//

#include <nel/misc/types_nl.h>

#include <deque>

#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/command.h>

#include <nel/3d/u_material.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_texture.h>

#include "graph.h"
#include "snowballs_client.h"

//
// Namespaces
//

using namespace NLMISC;
using namespace NL3D;
using namespace std;


namespace SBCLIENT {

//
// Classes
//

void CGraph::render ()
{
	// Display the background
	uint32 w, h;
	Driver->getWindowSize (w, h);
	float ScreenWidth = (float) w;
	float ScreenHeight = (float) h;
	Driver->setMatrixMode2D (CFrustum (0.0f, ScreenWidth, 0.0f, ScreenHeight, 0.0f, 1.0f, false));
	Driver->drawQuad (X, Y, X+Width, Y+Height, BackColor);

	float pos = X+Width-1;
	for (deque<float>::reverse_iterator it = Values.rbegin(); it != Values.rend(); it++)
	{
		float value = (*it) * Height / MaxValue;
		if (value > Height) value = Height;
		Driver->drawLine (pos, Y, pos, Y+value, CRGBA (255,255,255,BackColor.A));
		pos--;
	}

	float value = Peak * Height / MaxValue;
	if (value > Height) value = Height;
	CRGBA frontCol (min(BackColor.R*2,255),min(BackColor.G*2,255),min(BackColor.B*2,255),min(BackColor.A*2,255));
	Driver->drawLine (X, Y+value, X+Width, Y+value, frontCol);

	TextContext->setHotSpot (UTextContext::MiddleLeft);
	TextContext->setColor (frontCol);
	TextContext->setFontSize (10);
	TextContext->printfAt ((X+Width+2)/ScreenWidth, (Y+value)/ScreenHeight, toString(Peak).c_str());

	TextContext->setHotSpot (UTextContext::TopLeft);
	TextContext->printfAt ((X+1)/ScreenWidth, (Y+Height-1)/ScreenHeight, Name.c_str());
}

void CGraph::addOneValue (float value)
{
	Values.push_back (value);
	while (Values.size () > Width)
		Values.pop_front ();

	if (Values.back() > Peak)
		Peak = Values.back();
}


void CGraph::addValue (float value)
{
	TTime currentTime = CTime::getLocalTime ();

	while (currentTime > CurrentQuantumStart + Quantum)
	{
		CurrentQuantumStart += Quantum;
		addOneValue ();
	}

	Values.back() += value;

	if (Values.back() > Peak)
		Peak = Values.back();
}

//
// Variables
//

CGraph FpsGraph ("fps", 10.0f, 10.0f, 100.0f, 100.0f, CRGBA(128,0,0,128), 1000, 40.0f);
CGraph SpfGraph ("spf", 10.0f, 110.0f, 100.0f, 100.0f, CRGBA(0,128,0,128), 0, 0.1f);

CGraph DownloadGraph ("download", 10.0f, 260.0f, 100.0f, 100.0f, CRGBA(0,0,128,128), 1000, 1000.0f);
CGraph UploadGraph ("upload", 10.0f, 360.0f, 100.0f, 100.0f, CRGBA(0,128,128,128), 1000, 1000.0f);

bool ShowGraph;

//
// Functions
//

void cbUpdateGraph (CConfigFile::CVar &var)
{
	if (var.Name == "ShowGraph") ShowGraph = var.asInt() == 1;
	else nlwarning ("Unknown variable update %s", var.Name.c_str());
}

void initGraph ()
{
	ConfigFile->setCallback ("ShowGraph", cbUpdateGraph);

	cbUpdateGraph (ConfigFile->getVar ("ShowGraph"));
}

void updateGraph ()
{
	if (!ShowGraph) return;
	
	FpsGraph.render ();
	SpfGraph.render ();

	DownloadGraph.render ();
	UploadGraph.render ();
}

void releaseGraph ()
{
	ConfigFile->setCallback("ShowGraph", NULL);
}

NLMISC_COMMAND(graph,"swith on/off graphs","")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() != 0) return false;

	ShowGraph = !ShowGraph;
	return true;
}

} /* namespace SBCLIENT */

/* end of file */
