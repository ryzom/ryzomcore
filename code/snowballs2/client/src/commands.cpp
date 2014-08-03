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
#include <nel/3d/stereo_hmd.h>

#include "network.h"
#include "snowballs_client.h"
#include "interface.h"

#if SBCLIENT_DEV_PIXEL_PROGRAM
#include <nel/3d/driver_user.h>
#include <nel/3d/driver.h>
#include <nel/3d/pixel_program.h>
#include <nel/3d/material.h>
#include <nel/3d/u_texture.h>
#endif

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

#if SBCLIENT_DEV_PIXEL_PROGRAM
namespace {
CPixelProgram *a_DevPixelProgram = NULL;
UTextureFile *a_NelLogo;
}
#endif

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

#if SBCLIENT_DEV_PIXEL_PROGRAM
	CommandsMaterial.getObjectPtr()->setShader(NL3D::CMaterial::Program);
	a_NelLogo = Driver->createTextureFile("nel128.tga");
	CommandsMaterial.setTexture(dynamic_cast<NL3D::UTexture *>(a_NelLogo));
	static const char *program_arbfp1 =
		"!!ARBfp1.0\n"
		"PARAM c[1] = { { 1, 0 } };\n"
		"MOV result.color.xzw, c[0].xyyx;\n"
		"TEX result.color.y, fragment.texcoord[0], texture[0], 2D;\n"
		"END\n";
	static const char *program_fp40 =
		"!!ARBfp1.0\n"
		"OPTION NV_fragment_program2;\n"
		"PARAM c[1] = { { 1, 0 } };\n"
		"TEMP RC;\n"
		"TEMP HC;\n"
		"OUTPUT oCol = result.color;\n"
		"MOVR  oCol.xzw, c[0].xyyx;\n"
		"TEX   oCol.y, fragment.texcoord[0], texture[0], 2D;\n"
		"END\n";
	static const char *program_ps_1_1 = 
		"ps.1.1\n"
		"def c0, 0.000000, 0.000000, 1.000000, 0.000000\n"
		"def c1, 1.000000, 0.000000, 0.000000, 0.000000\n"
		"def c2, 0.000000, 1.000000, 0.000000, 0.000000\n"
		"tex t0\n"
		"mad r0.rgb, c2, t0, c1\n"
		"mov r0.a, c0.b\n";
	static const char *program_ps_2_0 = 
		"ps_2_0\n"
		"dcl_2d s0\n"
		"def c0, 1.00000000, 0.00000000, 0, 0\n"
		"dcl t0.xy\n"
		"texld r0, t0, s0\n"
		"mov r0.z, c0.y\n"
		"mov r0.xw, c0.x\n"
		"mov oC0, r0\n";
	static const char *program_ps_3_0 =
		"ps_3_0\n"
		"dcl_2d s0\n"
		"def c0, 1.00000000, 0.00000000, 0, 0\n"
		"dcl_texcoord0 v0.xy\n"
		"mov oC0.xzw, c0.xyyx\n"
		"texld oC0.y, v0, s0\n";
	NL3D::IDriver *d = dynamic_cast<NL3D::CDriverUser *>(Driver)->getDriver();
	a_DevPixelProgram = new CPixelProgram();
	// arbfp1
	{
		IProgram::CSource *source = new IProgram::CSource();
		source->Features.MaterialFlags = CProgramFeatures::TextureStages;
		source->Profile = IProgram::arbfp1;
		source->setSourcePtr(program_arbfp1);
		a_DevPixelProgram->addSource(source);
	}
	// ps_2_0
	{
		IProgram::CSource *source = new IProgram::CSource();
		source->Features.MaterialFlags = CProgramFeatures::TextureStages;
		source->Profile = IProgram::ps_2_0;
		source->setSourcePtr(program_ps_2_0);
		a_DevPixelProgram->addSource(source);
	}
	/*if (d->supportPixelProgram(CPixelProgram::fp40))
	{
		nldebug("fp40");
		a_DevPixelProgram = new CPixelProgram(program_fp40);
	}
	else if (d->supportPixelProgram(CPixelProgram::arbfp1))
	{
		nldebug("arbfp1");
		a_DevPixelProgram = new CPixelProgram(program_arbfp1);
	}*/
	/*else if (d->supportPixelProgram(CPixelProgram::ps_3_0))
	{
		nldebug("ps_3_0");
		a_DevPixelProgram = new CPixelProgram(program_ps_3_0);
		// Textures do not seem to work with ps_3_0...
	}*/
	/*else if (d->supportPixelProgram(CPixelProgram::ps_2_0))
	{
		nldebug("ps_2_0");
		a_DevPixelProgram = new CPixelProgram(program_ps_2_0);
	}
	else if (d->supportPixelProgram(CPixelProgram::ps_1_1))
	{
		nldebug("ps_1_1");
		a_DevPixelProgram = new CPixelProgram(program_ps_1_1);
	}*/
#endif
}

void	updateCommands()
{
	// Snap to pixels (kind of ugly code, but looks better ingame)
	uint32 _width, _height;
	Driver->getWindowSize(_width, _height);
	float width = (float)_width, height = (float)_height;
	NL3D::CViewport vp = Driver->getViewport();
	width *= vp.getWidth();
	height *= vp.getHeight();
	float CommandsLineHeight = CommandsFontSize / height;
	float CommandsBoxX = ((float)(sint32)(SBCLIENT::CommandsBoxX * width)) / width;
	float CommandsBoxWidth = ((float)(sint32)(SBCLIENT::CommandsBoxWidth * width)) / width;
	float CommandsBoxY = ((float)(sint32)(SBCLIENT::CommandsBoxY * height)) / height;
	float CommandsBoxHeight = ((float)(sint32)((CommandsNbLines + 1) * CommandsLineHeight * width)) / width;
	float CommandsBoxBorderX = ((float)(sint32)(SBCLIENT::CommandsBoxBorder * width)) / width;
	float CommandsBoxBorderY = ((float)(sint32)(SBCLIENT::CommandsBoxBorder * height)) / height;
	if (StereoHMD)
	{
		float xshift, yshift;
		StereoHMD->getInterface2DShift(0, xshift, yshift, 4.f);
		// snap to pixels
		xshift = ((float)(sint32)(xshift * width)) / width;
		yshift = ((float)(sint32)(yshift * height)) / height;
		// adjust
		CommandsBoxX += xshift;
		CommandsBoxY += yshift;
	}

	// Display the background
	Driver->setMatrixMode2D11 ();
#if SBCLIENT_DEV_PIXEL_PROGRAM
	CommandsMaterial.setColor(CRGBA::Blue); // Test to check which shader is displaying.
#else
	CommandsMaterial.setColor(CommandsBackColor);
#endif
	float x0 = CommandsBoxX - CommandsBoxBorderX;
	float y0 = CommandsBoxY - CommandsBoxBorderY;
	float x1 = CommandsBoxX + CommandsBoxWidth + CommandsBoxBorderX;
	float y1 = CommandsBoxY + CommandsBoxHeight + CommandsBoxBorderY;

#if SBCLIENT_DEV_PIXEL_PROGRAM
	NL3D::IDriver *d = dynamic_cast<NL3D::CDriverUser *>(Driver)->getDriver();
	d->activePixelProgram(a_DevPixelProgram);
	bool fogEnabled = d->fogEnabled();
	d->enableFog(false);

	// Driver->drawQuad(CQuad(CVector(x0, y0, 0), CVector(x1, y0, 0), CVector(x1, y1, 0), CVector(x0, y1, 0)), CommandsMaterial);
	CQuadUV quadUV;
	quadUV.V0 = CVector(x0, y0, 0);
	quadUV.V1 = CVector(x1, y0, 0);
	quadUV.V2 = CVector(x1, y1, 0);
	quadUV.V3 = CVector(x0, y1, 0);
	quadUV.Uv0 = CUV(0, 1);
	quadUV.Uv1 = CUV(1, 1);
	quadUV.Uv2 = CUV(1, 0);
	quadUV.Uv3 = CUV(0, 0);
	Driver->drawQuad(quadUV, CommandsMaterial);
	//Driver->drawBitmap(x0, y0, x1 - x0, y1 - y0, *a_NelLogo);

	d->enableFog(fogEnabled);
	d->activePixelProgram(NULL);
#else

	Driver->drawQuad(CQuad(CVector(x0, y0, 0), CVector(x1, y0, 0), CVector(x1, y1, 0), CVector(x0, y1, 0)), CommandsMaterial);

#endif

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
#if SBCLIENT_DEV_PIXEL_PROGRAM
	delete a_DevPixelProgram;
	a_DevPixelProgram = NULL;
#endif
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
