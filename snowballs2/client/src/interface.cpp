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

#include <cmath>
#include <nel/misc/vectord.h>
#include <nel/misc/event_listener.h>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>

#include "snowballs_client.h"
#include "interface.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NL3D;

namespace SBCLIENT {

//
// Variables
//

static string QueryString, AnswerString;
static float DisplayWidth;
static float DisplayHeight, nbLines;
static CRGBA DisplayColor;

static sint Prompt;

static uint FontSize = 20;

//
// Functions
//

// Class that handle the keyboard input when an interface is displayed (login request for example)
class CInterfaceListener : public IEventListener
{
	virtual void	operator() ( const CEvent& event )
	{
		// ignore keys if interface is not open
		if (!interfaceOpen ()) return;

		CEventChar &ec = (CEventChar&)event;

		// prompt = 2 then we wait a any user key
		if (Prompt == 2)
		{
			QueryString = "";
			AnswerString = "";
			_Line = "";
			return;
		}

		switch ( ec.Char )
		{
		case 13 : // RETURN : Send the chat message
			
			if (_Line.size() == 0)
				break;

			QueryString = "";
			AnswerString = _Line;

			_LastCommand = _Line;
			_Line = "";
			_MaxWidthReached = false;
			break;

		case 8 : // BACKSPACE
			if ( _Line.size() != 0 )
			{
				_Line.erase( _Line.end()-1 );
				// _MaxWidthReached = false; // no need
			}
			break;
		
		case 27 : // ESCAPE
			QueryString = "";
			AnswerString = "";
			_LastCommand = _Line;
			_Line = "";
			_MaxWidthReached = false;
			break;

		default: 
			
			if ( ! _MaxWidthReached )
			{
				_Line += (char)ec.Char;
			}
		}
	}

public:
	CInterfaceListener() : _MaxWidthReached( false )
	{}

	const string&	line() const
	{
		return _Line;
	}

 	void setLine(const string &str) { _Line = str; }

	void			setMaxWidthReached( bool b )
	{
		_MaxWidthReached = b;
	}

private:
	string			_Line;
	bool			_MaxWidthReached;
	string			_LastCommand;
};

// Instance of the listener
static CInterfaceListener InterfaceListener;
static UMaterial InterfaceMaterial = NULL;


void	initInterface()
{
	// Add the keyboard listener in the event server
	Driver->EventServer.addListener (EventCharId, &InterfaceListener);

        InterfaceMaterial = Driver->createMaterial();
        InterfaceMaterial.initUnlit();
        InterfaceMaterial.setBlendFunc(UMaterial::srcalpha, UMaterial::invsrcalpha);
        InterfaceMaterial.setBlend(true);
}

void	updateInterface()
{
        if (QueryString.empty()) return;

	// Draw background
	Driver->setMatrixMode2D11 ();
	InterfaceMaterial.setColor(DisplayColor);
	CQuad quad;
	quad.V0.set(0.5f-DisplayWidth/2.0f, 0.5f+DisplayHeight/2.0f, 0);
	quad.V1.set(0.5f+DisplayWidth/2.0f, 0.5f+DisplayHeight/2.0f, 0);
	quad.V2.set(0.5f+DisplayWidth/2.0f, 0.5f-DisplayHeight/2.0f, 0);
	quad.V3.set(0.5f-DisplayWidth/2.0f, 0.5f-DisplayHeight/2.0f, 0);
	Driver->drawQuad(quad, InterfaceMaterial);
	//Driver->drawQuad (0.5f-DisplayWidth/2.0f, 0.5f-DisplayHeight/2.0f, 0.5f+DisplayWidth/2.0f, 0.5f+DisplayHeight/2.0f, DisplayColor);

	// Set the text context
	TextContext->setHotSpot (UTextContext::MiddleMiddle);
	TextContext->setColor (CRGBA (255,255,255,255));
	TextContext->setFontSize (FontSize);

	// Display the text
	string str;
	float y = (nbLines * FontSize / 2.0f) / 600.0f;

	for (uint i = 0; i < QueryString.size (); i++)
	{
		if (QueryString[i] == '\n')
		{
			TextContext->printfAt (0.5f, 0.5f+y, str.c_str());
			str = "";
			y -= FontSize / 600.0f;
		}
		else
		{
			str += QueryString[i];
		}
	}
	TextContext->printfAt (0.5f, 0.5f+y, str.c_str());

	// Display the user input line
	y-= (2.0f * FontSize) / 600.0f;
	string str2;
	switch (Prompt)
	{
	case 0:
		str2 = InterfaceListener.line();
		str2 += "_";
		break;
	case 1:
		str2.resize (InterfaceListener.line().size (), '*');
		str2 += "_";
		break;
	case 2:
		str2 = "";
		break;
	}
	TextContext->printfAt (0.5f, 0.5f+y, str2.c_str());
}

void	releaseInterface()
{
	// Delete the material
	Driver->deleteMaterial(InterfaceMaterial);

	// Remove the keyboard listener from the server
	Driver->EventServer.removeListener (EventCharId, &InterfaceListener);
}

void	askString (const string &queryString, const string &defaultString, sint prompt, const CRGBA &color)
{
  nldebug("called askString");
nlinfo(queryString.c_str());	
	QueryString = queryString;
	
	if (prompt == 2)
		QueryString += "\n\n(Press any key to continue)";
	else
		QueryString += "\n\n(Press enter to validate and ESC to cancel)";

	nbLines = 1;
	for (uint i = 0; i < QueryString.size (); i++)
	{
		if (QueryString[i] == '\n') nbLines++;
	}

	DisplayWidth = float(queryString.size () * FontSize) / 600.0f;
	DisplayHeight = float((nbLines + 4) * FontSize) / 600.0f;
	DisplayColor = color;

	InterfaceListener.setLine (defaultString);
	Prompt = prompt;
}

bool	haveAnswer (string &answer)
{
	bool haveIt = !AnswerString.empty();
	if (haveIt)
	{
		answer = AnswerString;
		AnswerString = "";
	}
	return haveIt;
}

bool	interfaceOpen ()
{
	return !QueryString.empty();
}

} /* namespace SBCLIENT */

/* end of file */
