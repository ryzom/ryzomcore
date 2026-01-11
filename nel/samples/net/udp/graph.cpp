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

#include "graph.h"

#ifdef USE_3D

#include <deque>

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/command.h>

#include <nel/3d/u_material.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_texture.h>
#include <nel/3d/driver.h>
#include <nel/3d/vertex_buffer.h>
#include <nel/3d/material.h>
#include <nel/3d/driver_user.h>

//
// Namespaces
//

using namespace NLMISC;
using namespace NL3D;
using namespace std;

//
// Variables
//

vector<CGraph*> *CGraph::_Graphs = NULL;

bool CGraph::Display = true;
bool CGraph::DisplayAverageValue = true;

//
// Classes
//

void CGraph::render (NL3D::UDriver *Driver, NL3D::UTextContext *TextContext)
{
	// Display the background
	uint32 w, h;
	Driver->getWindowSize (w, h);
	float ScreenWidth = (float) w;
	float ScreenHeight = (float) h;
	Driver->setMatrixMode2D (CFrustum (0.0f, ScreenWidth, 0.0f, ScreenHeight, 0.0f, 1.0f, false));
	Driver->drawQuad (X, Y, X+Width, Y+Height, BackColor);

	Peak = 0.0f;
	float sum = 0.0f;
	
	CMaterial		material;
	material.initUnlit ();
	material.setColor (CRGBA (255,255,255,BackColor.A));
	material.setBlend (true);

	
	CVertexBuffer	vbuffer;
	vbuffer.setVertexFormat (CVertexBuffer::PositionFlag);
	vbuffer.setNumVertices ((uint32)Values.size() * 2);

	float pos = X+Width-1;
	uint i = 0;
	for (deque<float>::reverse_iterator it = Values.rbegin(); it != Values.rend(); it++)
	{
		// get a read accessor to the VB
		CVertexBufferRead vba;
		vbuffer.lock (vba);

		float value = (*it) * Height / MaxValue;
		if (value > Height) value = Height;

		CVector *vect1 = (CVector*)vba.getVertexCoordPointer(i*2+0);
		vect1->x = pos;
		vect1->y = Y;
		CVector *vect2 = (CVector*)vba.getVertexCoordPointer(i*2+1);
		vect2->x = pos;
		vect2->y = Y+value;

//		Driver->drawLine (pos, Y, pos, Y+value, CRGBA (255,255,255,BackColor.A));
		pos--;
		if ((*it) > Peak) Peak = *it;
		sum += *it;
		i++;
	}

	// Render
	IDriver	*drv = ((CDriverUser*)Driver)->getDriver();
	drv->activeVertexBuffer(vbuffer);

	// Display max
	float value = Peak * Height / MaxValue;
	if (value > Height) value = Height;
	CRGBA frontCol (min(BackColor.R*2,255),min(BackColor.G*2,255),min(BackColor.B*2,255),min(BackColor.A*2,255));
	float peakval = Y+value;
	Driver->drawLine (X, peakval, X+Width, peakval, frontCol);
	
	TextContext->setHotSpot (UTextContext::MiddleLeft);
	TextContext->setColor (frontCol);
	TextContext->setFontSize (10);
	TextContext->printfAt ((X+Width+2)/ScreenWidth, (Y+value)/ScreenHeight, "%.2f", Peak);

	// Display average
	float average = sum / (float)Values.size();
	value = average * Height / MaxValue;
	if (value > Height) value = Height;
	float avrval = Y+value;
	Driver->drawLine (X, avrval, X+Width, avrval, frontCol);

	if (DisplayAverageValue)
	{
		if (avrval+10<peakval-10 || avrval-10>peakval+10)
		{
			TextContext->setHotSpot (UTextContext::MiddleLeft);
			TextContext->setColor (frontCol);
			TextContext->setFontSize (10);
			TextContext->printfAt ((X+Width+2)/ScreenWidth, (Y+value)/ScreenHeight, "%.2f", average);
		}
	}

	// Display name
	TextContext->setHotSpot (UTextContext::TopLeft);
	TextContext->printfAt ((X+1)/ScreenWidth, (Y+Height-1)/ScreenHeight, Name.c_str());
}

void CGraph::addOneValue (float value)
{
	if (value < 0.0f) value = 0.0f;

	Values.push_back (value);
	while (Values.size () > Width)
		Values.pop_front ();

//	if (Values.back() > Peak)
//		Peak = Values.back();
}


void CGraph::addValue (float value)
{
	TTime currentTime = CTime::getLocalTime ();

	while (Values.size () == 0 || currentTime > CurrentQuantumStart + Quantum)
	{
		CurrentQuantumStart += Quantum;
		addOneValue ();
	}

	Values.back() += value;

//	if (Values.back() > Peak)
//		Peak = Values.back();
}

//
// Variables
//
/*
CGraph FpsGraph ("fps", 10.0f, 110.0f, 100.0f, 100.0f, CRGBA(128,0,0,128), 1000, 150.0f);
CGraph SpfGraph ("spf", 10.0f, 10.0f, 100.0f, 100.0f, CRGBA(0,128,0,128), 0, 100.0f);

CGraph DownloadGraph ("download", 10.0f, 260.0f, 100.0f, 100.0f, CRGBA(0,0,128,128), 1000, 2000.0f);
CGraph UploadGraph ("upload", 10.0f, 360.0f, 100.0f, 100.0f, CRGBA(0,128,128,128), 1000, 2000.0f);

CGraph DpfGraph ("dpf", 150.0f, 260.0f, 100.0f, 100.0f, CRGBA(128,0,128,128), 100000, 180.0f);
CGraph UpfGraph ("upf", 150.0f, 360.0f, 100.0f, 100.0f, CRGBA(128,128,0,128), 100000, 180.0f);


CGraph UserLWatchGraph ("ul", 10.0f, 490.0f, 100.0f, 100.0f, CRGBA(0,64,128,128), 100000, 1000.0f);
CGraph CycleWatchGraph ("cl", 150.0f, 490.0f, 100.0f, 100.0f, CRGBA(0,128,64,128), 100000, 1000.0f);
CGraph ReceiveWatchGraph ("rl", 300.0f, 490.0f, 100.0f, 100.0f, CRGBA(128,64,0,128), 100000, 100.0f);
CGraph SendWatchGraph ("sl", 450.0f, 490.0f, 100.0f, 100.0f, CRGBA(128,0,64,128), 100000, 100.0f);
CGraph PriorityAmountGraph ("prio amount", 150.0f, 110.0f, 100.0f, 100.0f, CRGBA(128,64,64,128), 100000, 16.0f);
CGraph SeenEntitiesGraph( "seen entities", 150.0f, 10.0f, 100.0f, 100.0f, CRGBA(64,128,64,128), 100000, 256);
*/

//
// Functions
//

void CGraph::render (NL3D::UDriver &driver, NL3D::UTextContext &tc)
{
	if (!Display) return;

	if (_Graphs == NULL) return;

	for (uint i = 0; i < _Graphs->size(); i++)
	{
		(*_Graphs)[i]->render (&driver, &tc);
	}
}

/*
NLMISC_VARIABLE(bool, GraphShow, "Display or not graphs");
NLMISC_VARIABLE(bool, GraphDisplayAverageValue, "Display or not average values");
*/

#endif
