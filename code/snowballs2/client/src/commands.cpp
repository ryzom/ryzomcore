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

#include <list>

#include <nel/misc/event_listener.h>
#include <nel/misc/command.h>
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>

#include "network.h"
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

CLog CommandsLog;

static list <string> StoredLines;
static uint32 NbStoredLines = 100;

// These variables are automatically set with the config file

static float CommandsBoxX, CommandsBoxY, CommandsBoxWidth;
static float CommandsBoxBorder;
static int CommandsNbLines;
static float CommandsLineHeight;
static int CommandsFontSize;
static CRGBA CommandsBackColor, CommandsFrontColor;
static UMaterial CommandsMaterial = NULL;

//
// Functions
//

// Display a string to the commands interface
void addLine (const string &line)
{
	// Add the line
	StoredLines.push_back (line);

	// Clear old lines if too much lines are stored
	while (StoredLines.size () > NbStoredLines)
	{
		StoredLines.pop_front ();
	}
}

// Display used to display on the commands interface
class CCommandsDisplayer : public IDisplayer
{
	virtual void doDisplay (const CLog::TDisplayInfo &args, const char *message)
	{
		bool needSpace = false;
		string str;

		if (args.LogType != CLog::LOG_NO)
		{
			str += logTypeToString(args.LogType);
			needSpace = true;
		}

		if (needSpace) { str += ": "; needSpace = false; }
		str += message;
		addLine (str);
	}
};

// Instance of the displayer
static CCommandsDisplayer CommandsDisplayer;

// Check if the user line is a command or not (a commands precede by a '/')
bool commandLine (const string &str)
{
	string command = "";

	if (str[0]=='/')
	{
		// If it's a command call it
		command = str.substr(1);
		// add the string in to the chat
		addLine (string ("command> ") + str);
		ICommand::execute (command, CommandsLog);
		return true;
	}
	else
	{
		return false;
	}
}

// Manage the user keyboard input
class CCommandsListener : public IEventListener
{
	virtual void	operator() ( const CEvent& event )
	{
		// If the interface is open, ignore keys for the command interface
		if (interfaceOpen ()) return;

		// Get the key
		CEventChar &ec = (CEventChar&)event;

		switch ( ec.Char )
		{
		case 13 : // RETURN : Send the chat message
			
			// If the line is empty, do nothing
			if ( _Line.size() == 0 ) break;

			// If it's a command, execute it and don't send the command to the network
			if ( ! commandLine( _Line ) )
			{
				// If online, send the chat line, otherwise, locally displays it
				if (isOnline ())
					sendChatLine (_Line);
				else
					addLine (string ("you said> ") + _Line);
			}
			// Reset the command line
			_LastCommand = _Line;
			_Line = "";
			_MaxWidthReached = false;
			break;

		case 8 : // BACKSPACE : remove the last character

			if ( _Line.size() != 0 )
			{
				_Line.erase( _Line.end()-1 );
			}
			break;

		case 9 : // TAB : If it's a command, try to auto complete it
			
			if (_Line.empty())
			{
				_Line = _LastCommand;
			}
			else if (!_Line.empty() && _Line[0] == '/')
			{
				string command = _Line.substr(1);
				ICommand::expand(command);
				_Line = '/' + command;
			}
			break;

		case 27 : // ESCAPE : clear the command
		
			_Line = "";
			_MaxWidthReached = false;
			break;

		default: // OTHERWISE : add the character to the line

			if (! _MaxWidthReached)
			{
				_Line += (char)ec.Char;
			}
		}
	}

public:
	CCommandsListener() : _MaxWidthReached( false )
	{}

	const string&	line() const
	{
		return _Line;
	}

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
static CCommandsListener CommandsListener;

// This functions is automatically called when the config file changed (dynamically)
void cbUpdateCommands (CConfigFile::CVar &var)
{
	if (var.Name == "CommandsBoxX") CommandsBoxX = var.asFloat ();
	else if (var.Name == "CommandsBoxY") CommandsBoxY = var.asFloat ();
	else if (var.Name == "CommandsBoxWidth") CommandsBoxWidth = var.asFloat ();
	else if (var.Name == "CommandsBoxBorder") CommandsBoxBorder = var.asFloat ();
	else if (var.Name == "CommandsNbLines") CommandsNbLines = var.asInt ();
	else if (var.Name == "CommandsLineHeight") CommandsLineHeight = var.asFloat ();
	else if (var.Name == "CommandsBackColor") CommandsBackColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "CommandsFrontColor") CommandsFrontColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "CommandsFontSize") CommandsFontSize = var.asInt ();
	else nlwarning ("Unknown variable update %s", var.Name.c_str());
}

void	initCommands()
{
	// Add the keyboard listener in the event server
	Driver->EventServer.addListener (EventCharId, &CommandsListener);

	// Add the command displayer to the standard log (to display NeL info)
	CommandsLog.addDisplayer (&CommandsDisplayer);
#ifndef NL_RELEASE
	DebugLog->addDisplayer (&CommandsDisplayer);
	InfoLog->addDisplayer (&CommandsDisplayer);
	WarningLog->addDisplayer (&CommandsDisplayer);
	AssertLog->addDisplayer (&CommandsDisplayer);
	ErrorLog->addDisplayer (&CommandsDisplayer);
#endif

	// Add callback for the config file
	ConfigFile->setCallback ("CommandsBoxX", cbUpdateCommands);
	ConfigFile->setCallback ("CommandsBoxY", cbUpdateCommands);
	ConfigFile->setCallback ("CommandsBoxWidth", cbUpdateCommands);
	ConfigFile->setCallback ("CommandsBoxBorder", cbUpdateCommands);
	ConfigFile->setCallback ("CommandsNbLines", cbUpdateCommands);
	ConfigFile->setCallback ("CommandsLineHeight", cbUpdateCommands);
	ConfigFile->setCallback ("CommandsBackColor", cbUpdateCommands);
	ConfigFile->setCallback ("CommandsFrontColor", cbUpdateCommands);
	ConfigFile->setCallback ("CommandsFontSize", cbUpdateCommands);
  
	// Init the config file variable
	cbUpdateCommands (ConfigFile->getVar ("CommandsBoxX"));
	cbUpdateCommands (ConfigFile->getVar ("CommandsBoxY"));
	cbUpdateCommands (ConfigFile->getVar ("CommandsBoxWidth"));
	cbUpdateCommands (ConfigFile->getVar ("CommandsBoxBorder"));
	cbUpdateCommands (ConfigFile->getVar ("CommandsNbLines"));
	cbUpdateCommands (ConfigFile->getVar ("CommandsLineHeight"));
	cbUpdateCommands (ConfigFile->getVar ("CommandsBackColor"));
	cbUpdateCommands (ConfigFile->getVar ("CommandsFrontColor"));
	cbUpdateCommands (ConfigFile->getVar ("CommandsFontSize"));

	CommandsMaterial = Driver->createMaterial();
    CommandsMaterial.initUnlit();
    CommandsMaterial.setBlendFunc(UMaterial::srcalpha, UMaterial::invsrcalpha);
    CommandsMaterial.setBlend(true);
}

void	updateCommands()
{
	// Snap to pixels (kind of ugly code, but looks better ingame)
	uint32 _width, _height;
	Driver->getWindowSize(_width, _height);
	float width = (float)_width, height = (float)_height;
	float CommandsLineHeight = CommandsFontSize / height;
	float CommandsBoxX = ((float)(sint32)(SBCLIENT::CommandsBoxX * width)) / width;
	float CommandsBoxWidth = ((float)(sint32)(SBCLIENT::CommandsBoxWidth * width)) / width;
	float CommandsBoxY = ((float)(sint32)(SBCLIENT::CommandsBoxY * height)) / height;
	float CommandsBoxHeight = ((float)(sint32)((CommandsNbLines + 1) * CommandsLineHeight * width)) / width;
	float CommandsBoxBorderX = ((float)(sint32)(SBCLIENT::CommandsBoxBorder * width)) / width;
	float CommandsBoxBorderY = ((float)(sint32)(SBCLIENT::CommandsBoxBorder * height)) / height;

	// Display the background
	Driver->setMatrixMode2D11 ();
	CommandsMaterial.setColor(CommandsBackColor);
	float x0 = CommandsBoxX - CommandsBoxBorderX;
	float y0 = CommandsBoxY - CommandsBoxBorderY;
	float x1 = CommandsBoxX + CommandsBoxWidth + CommandsBoxBorderX;
	float y1 = CommandsBoxY + CommandsBoxHeight + CommandsBoxBorderY;
	Driver->drawQuad(CQuad(CVector(x0, y0, 0), CVector(x1, y0, 0), CVector(x1, y1, 0), CVector(x0, y1, 0)), CommandsMaterial);

	// Set the text context
	TextContext->setHotSpot (UTextContext::BottomLeft);
	TextContext->setColor (CommandsFrontColor);
	TextContext->setFontSize (CommandsFontSize);

	// Display the user input line
	ucstring line = ucstring("> ") + ucstring(CommandsListener.line()) + ucstring("_");
	uint32 csi = TextContext->textPush(line);	
	float sw = TextContext->getStringInfo(csi).StringWidth / width; // make sure newly typed text is visible
	TextContext->printAt(sw > CommandsBoxWidth ? CommandsBoxX - sw + CommandsBoxWidth : CommandsBoxX, CommandsBoxY, csi);
	TextContext->erase(csi);

	// Display stored lines
	float yPos = CommandsBoxY;
	list<string>::reverse_iterator rit = StoredLines.rbegin();
	for (sint32 i = 0; i < CommandsNbLines; ++i)
	{
		yPos += CommandsLineHeight;
		if (rit == StoredLines.rend()) break;
		TextContext->printfAt(CommandsBoxX, yPos, (*rit).c_str());
		rit++;
	}
}

void	clearCommands ()
{
	StoredLines.clear ();
}

void releaseCommands()
{
	// Remove the displayers
	CommandsLog.removeDisplayer(&CommandsDisplayer);
#ifndef NL_RELEASE
	DebugLog->removeDisplayer(&CommandsDisplayer);
	InfoLog->removeDisplayer(&CommandsDisplayer);
	WarningLog->removeDisplayer(&CommandsDisplayer);
	AssertLog->removeDisplayer(&CommandsDisplayer);
	ErrorLog->removeDisplayer(&CommandsDisplayer);
#endif

	// Remove callbacks for the config file
	ConfigFile->setCallback("CommandsBoxX", NULL);
	ConfigFile->setCallback("CommandsBoxY", NULL);
	ConfigFile->setCallback("CommandsBoxWidth", NULL);
	ConfigFile->setCallback("CommandsBoxBorder", NULL);
	ConfigFile->setCallback("CommandsNbLines", NULL);
	ConfigFile->setCallback("CommandsLineHeight", NULL);
	ConfigFile->setCallback("CommandsBackColor", NULL);
	ConfigFile->setCallback("CommandsFrontColor", NULL);
	ConfigFile->setCallback("CommandsFontSize", NULL);

	// Remove the keyboard listener from the server
	Driver->EventServer.removeListener(EventCharId, &CommandsListener);

	// Remove the material
	Driver->deleteMaterial(CommandsMaterial);
}

NLMISC_COMMAND(clear,"clear the chat history","")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 0) return false;
	clearCommands ();
	return true;
}

} /* namespace SBCLIENT */

/* end of file */
