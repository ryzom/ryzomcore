// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#include "stdpch.h"


//////////////
// Includes //
//////////////
// Misc.
#include "nel/misc/path.h"
// 3D Interface.
#include "nel/3d/u_driver.h"
// Client.
#include "interfaces_manager.h"
#include "interf_script.h"
// Std.
#include <fstream>


///////////
// Using //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace std;


/////////////
// Externs //
/////////////
extern UDriver		*Driver;


////////////
// Global //
////////////

CInterfMngr::TMapCtrls		CInterfMngr::_Ctrls;
CInterfMngr::TMapTexts		CInterfMngr::_Texts;
CInterfMngr::TMapTextures	CInterfMngr::_Textures;
CInterfMngr::TMapPens		CInterfMngr::_Pens;
CPen						CInterfMngr::_DefaultPen;
CInterfMngr::TMapButtons	CInterfMngr::_Buttons;
CButtonBase					CInterfMngr::_DefaultButton;

CInterfMngr::TMapFuncCtrl	CInterfMngr::_CtrlFunctions;

CInterfMngr::TMapWindows	CInterfMngr::_Windows;
CInterfMngr::TOSD			CInterfMngr::_OSD;
CInterfMngr::TUint			CInterfMngr::_OSDWaitingDestruction;
CInterfMngr::TOSD			CInterfMngr::_OSDWaitingInsertion;

CInterfMngr::TMapOSDBase	CInterfMngr::_OSDLook;
COSDBase					CInterfMngr::_DefaultOSDLook;

uint32 CInterfMngr::_WindowSizeX;
uint32 CInterfMngr::_WindowSizeY;


///////////////
// Functions //
///////////////
//-----------------------------------------------
// init :
// Initialize the interfaces manager.
//-----------------------------------------------
void CInterfMngr::init(const CInterfMngr::TMapFuncCtrl &funcCtrlMap, uint32 width, uint32 height)
{
	_WindowSizeX = width;
	_WindowSizeY = height;

	loadTexts();
	loadTextures();
	loadPens();
	loadCtrls();
	loadButtons();
	loadOSDLook();
	setFuncCtrl(funcCtrlMap);
}// init //

//-----------------------------------------------
// release :
// Release all the interface.
//-----------------------------------------------
void CInterfMngr::release()
{
	// Release OSD Look.
	for(TMapOSDBase::iterator itOSDLook = _OSDLook.begin(); itOSDLook != _OSDLook.end(); ++itOSDLook)
	{
		if((*itOSDLook).second)
		{
			delete (*itOSDLook).second;
			(*itOSDLook).second = 0;
		}
	}


	// Delete OSDs.
	for(TMapWindows::iterator it = _Windows.begin(); it != _Windows.end(); ++it)
	{
		if((*it).second)
		{
			delete (*it).second;
			(*it).second = 0;
		}
	}
	// Clear all Container
	_Windows.clear();
	_OSD.clear();
	_OSDWaitingDestruction.clear();
	_OSDWaitingInsertion.clear();
	_CtrlFunctions.clear();
}// release //

//-----------------------------------------------
// loadCtrls :
// Load the array to convert a control id into a control type.
//-----------------------------------------------
void CInterfMngr::loadCtrls()
{
	string interfaceFile = CPath::lookup("ctrls.txt");
	ifstream file(interfaceFile.c_str(), ios::in);

	// Open the file.
	if(file.is_open())
	{
		char tmpBuff[_MAX_LINE_SIZE];
		char delimiterBox[] = "[] \t";
		uint line = 0;

		while(!file.eof())
		{
			file.getline(tmpBuff, _MAX_LINE_SIZE);
			line++;
			char *str;
			char *key = strtok(tmpBuff, delimiterBox);

			// the character '/' is used to indicate comments in the controls txt file
			if( (key != NULL) && (*key) != '/' )
			{
				uint id = atoi(key);
				// If the ID != 0 -> ID Valide.
				if(id != 0)
				{
					str = strtok(NULL, delimiterBox);
					if(str != NULL)
					{
						if(strcmp(str, "TEXT") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlText));
						else if(strcmp(str, "CAPTURE") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlCapture));
						else if(strcmp(str, "BUTTON") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlButton));
						else if(strcmp(str, "RADIO_CONTROLLER") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlRadioController));
						else if(strcmp(str, "RADIO_BUTTON") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlRadioButton));
						else if(strcmp(str, "BITMAP") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlBitmap));
						else if(strcmp(str, "LIST") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlList));
						else if(strcmp(str, "MULTI_LIST") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlMultiList));
						else if(strcmp(str, "CHAT") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlChat));
						else if(strcmp(str, "CHAT_INPUT") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlChatInput));
						else if(strcmp(str, "CHOICE_LIST") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlChoiceList));
						else if(strcmp(str, "CANDIDATE_LIST") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlCandidateList));
						else if(strcmp(str, "HORIZONTAL_LIST") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlHorizontalList));
						else if(strcmp(str, "CONTROL_LIST") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlControlList));
						else if(strcmp(str, "SPELL_LIST") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlSpellList));
						else if(strcmp(str, "PROGRESS_BAR") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlProgressBar));
						else if(strcmp(str, "CASTING_BAR") == 0) // TEMP
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlCastingBar));
						else if(strcmp(str, "BRICK_CONTROL") == 0)
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlBrick));
						else
							_Ctrls.insert(TMapCtrls::value_type(id, CtrlUnknown));
					}
				}
				// ID is not Valide 0 is reserved for the OSD.
				else
				{
					nlerror("\"ctrls.txt\": Line %d : 0 must not be used for a control ID !!", line);
				}
			}
		}
		file.close();
	}
}// loadCtrls //


//-----------------------------------------------
// loadOSDLook :
// Load The look of OSDs.
//-----------------------------------------------
void CInterfMngr::loadOSDLook()
{
	string interfaceFile = CPath::lookup("OSDs.txt");
	ifstream file(interfaceFile.c_str(), ios::in);

	// Open the file.
	if(file.is_open())
	{
		char tmpBuff[_MAX_LINE_SIZE];
		char delimiter[] = "[] \t";
		char *ptr = 0;

		bool		defaultCase = false;
		uint		id;

		COSD::TBG	bgMode	= COSD::BG_none;
		uint		bg		= 0;
		CRGBA		bgColor;

		COSD::TTB	tbMode	= COSD::TB_none;
		uint		tb		= 0;
		CRGBA		tbColor;
		uint		tbPen	= 0;

		uint		hlSize	= 1;
		CRGBA		hlColor;

		uint		rsSize	= 4;
		CRGBA		rsColor;


		// While the end of the file is not reached.
		while(!file.eof())
		{
			// Get a line (the line should not be more than _MAX_LINE_SIZE).
			file.getline(tmpBuff, _MAX_LINE_SIZE);

			// Get the Id
			ptr = strtok(tmpBuff, delimiter);
			// If empty line -> next line.
			if(ptr == NULL)
				continue;

			// If the id == "..." -> default case.
			if(strcmp(ptr, "...") == 0)
			{
				defaultCase = true;
			}
			else
			{
				// Make the id.
				id = atoi(ptr);
				defaultCase = false;
			}

			// Get the first token delimited by the characters in delimiter.
			ptr = strtok(NULL, delimiter);
			while(ptr != NULL)
			{
				// Get the Background Mode.
				if(strcmp(ptr, "BG_Mode:") == 0)
					bgMode = getBGMode();
				// Get BG Bitmap.
				else if(strcmp(ptr, "BG_Bitmap:") == 0)
					bg = getInt();
				// Get BG Color.
				else if(strcmp(ptr, "BG_Color:") == 0)
					bgColor = getRGBA();

				// Get the Title Bar Mode.
				else if(strcmp(ptr, "TB_Mode:") == 0)
					tbMode = getTBMode();
				// Get TB Bitmap.
				else if(strcmp(ptr, "TB_Bitmap:") == 0)
					tb = getInt();
				// Get TB Color.
				else if(strcmp(ptr, "TB_Color:") == 0)
					tbColor = getRGBA();
				// Get TB Pen.
				else if(strcmp(ptr, "TB_Pen:") == 0)
					tbPen = getInt();

				// Get HL Size.
				else if(strcmp(ptr, "HL_Pen:") == 0)
					hlSize = getInt();
				// Get HL Color.
				else if(strcmp(ptr, "HL_Color:") == 0)
					hlColor = getRGBA();

				// Get RS Size.
				else if(strcmp(ptr, "RS_Pen:") == 0)
					rsSize = getInt();
				// Get RS Color.
				else if(strcmp(ptr, "RS_Color:") == 0)
					rsColor = getRGBA();

				// Next Token.
				ptr = strtok(NULL, delimiter);

			}

			// Set the Default Pen.
			if(defaultCase)
			{
				_DefaultOSDLook._BG_Mode	= bgMode;
				_DefaultOSDLook._BG			= bg;
				_DefaultOSDLook._BG_Color	= bgColor;

				_DefaultOSDLook._TB_Mode	= tbMode;
				_DefaultOSDLook._TB			= tb;
				_DefaultOSDLook._TB_Color	= tbColor;
				_DefaultOSDLook._TB_Pen		= getPen(tbPen);

				_DefaultOSDLook._HL_Size	= (float)hlSize;
				_DefaultOSDLook._HL_Color	= hlColor;

				_DefaultOSDLook._RS_Size	= (float)rsSize;
				_DefaultOSDLook._RS_Color	= rsColor;
			}
			// Set specific Pens.
			else
			{
				COSDBase *osdBase = new COSDBase;
				osdBase->_BG_Mode	= bgMode;
				osdBase->_BG		= bg;
				osdBase->_BG_Color	= bgColor;

				osdBase->_TB_Mode	= tbMode;
				osdBase->_TB		= tb;
				osdBase->_TB_Color	= tbColor;
				osdBase->_TB_Pen	= getPen(tbPen);

				osdBase->_HL_Size	= (float)hlSize;
				osdBase->_HL_Color	= hlColor;

				osdBase->_RS_Size	= (float)rsSize;
				osdBase->_RS_Color	= rsColor;

				// Insert the text.
				_OSDLook.insert(TMapOSDBase::value_type(id, osdBase));
			}
		}
		// Close the file.
		file.close();
	}
}// loadOSDLook //


//-----------------------------------------------
// loadTexts :
// Load all texts.
//-----------------------------------------------
void CInterfMngr::loadTexts()
{
	string interfaceFile = CPath::lookup("texts.txt");
	ifstream file(interfaceFile.c_str(), ios::in);

	// Open the file.
	if(file.is_open())
	{
		char tmpBuff[_MAX_LINE_SIZE];
		char delimiterBox[] = "[]";

		while(!file.eof())
		{
			file.getline(tmpBuff, _MAX_LINE_SIZE);
			char *str;
			char *key = strtok( tmpBuff, delimiterBox);
			if(key != NULL)
			{
				uint id = atoi(key);
				str = strtok( NULL, delimiterBox);
				if(str != NULL)
					_Texts.insert(TMapTexts::value_type(id, ucstring(str)));
			}
		}
		file.close();
	}
}// loadTexts //

//-----------------------------------------------
// Load Textures
// Load all textures needed for the interfaces.
//-----------------------------------------------
void CInterfMngr::loadTextures()
{
/*	string interfaceFile = CPath::lookup("textures.txt");
	ifstream file(interfaceFile.c_str(), ios::in);

	// Open the file.
	if(file.is_open())
	{
		char tmpBuff[_MAX_LINE_SIZE];
		char delimiterBox[] = "[]";

		while(!file.eof())
		{
			file.getline(tmpBuff, _MAX_LINE_SIZE);
			char *str;
			char *key = strtok( tmpBuff, delimiterBox);
			if(key != NULL)
			{
				uint id = atoi(key);
				str = strtok( NULL, delimiterBox);
				if(str != NULL)
				{
					_Textures.insert(TMapTextures::value_type(id, Driver->createTextureFile(string(str))));
				}
			}
		}
		file.close();
	}*/
}// loadTextures //



//-----------------------------------------------
// loadPens :
// Load Pens from file.
//-----------------------------------------------
void CInterfMngr::loadPens()
{
	string interfaceFile = CPath::lookup("pens.txt");
	ifstream file(interfaceFile.c_str(), ios::in);

	// Try to open the file.
	if(file.is_open())
	{
		char tmpBuff[_MAX_LINE_SIZE];
		char delimiterBox[] = "[] \t";

		// While the end of the file is not reached.
		while(!file.eof())
		{
			// Get a line (teh line should not be more than _MAX_LINE_SIZE).
			file.getline(tmpBuff, _MAX_LINE_SIZE);
			// Get the first token delimited by the characters in delimiterBox.
			char *token = strtok(tmpBuff, delimiterBox);
			if(token != NULL)
			{
				uint32	fontSize	= 20;
				uint8	r			= 255;
				uint8	g			= 255;
				uint8	b			= 255;
				uint8	a			= 255;
				bool	shadow		= false;
				sint	tmp			= 0;
				bool	defaultCase = false;
				uint	id;

				// If the id == "..." -> default case.
				if(strcmp(token, "...") == 0)
				{
					defaultCase = true;
				}
				else
				{
					// Make the id.
					id = atoi(token);
					defaultCase = false;
				}

				// Get the font size.
				token = strtok(NULL, delimiterBox);
				if(token != NULL)
					fontSize = atoi(token);

				// Get the text color
				token = strtok(NULL, delimiterBox);
				if(token != NULL)
					r = atoi(token);
				token = strtok(NULL, delimiterBox);
				if(token != NULL)
					g = atoi(token);
				token = strtok(NULL, delimiterBox);
				if(token != NULL)
					b = atoi(token);
				token = strtok(NULL, delimiterBox);
				if(token != NULL)
					a = atoi(token);

				// Get the shadow.
				token = strtok(NULL, delimiterBox);
				if(token != NULL)
				{
					tmp = atoi(token);
					if(tmp)
						shadow = true;
					else
						shadow = false;
				}

				// Set the Default Pen.
				if(defaultCase)
				{
					_DefaultPen.color(CRGBA(r,g,b,a));
					_DefaultPen.fontSize(fontSize);
					_DefaultPen.shadow(shadow);
				}
				// Set specific Pens.
				else
				{
					// Insert the text.
					_Pens.insert(TMapPens::value_type(id, new CPen(fontSize, CRGBA(r,g,b,a), shadow)));
				}
			}
		}
		file.close();
	}
}// loadPens //


//-----------------------------------------------
// loadButtons :
// Load Buttons from file.
//-----------------------------------------------
void CInterfMngr::loadButtons()
{
	string interfaceFile = CPath::lookup("buttons.txt");
	ifstream file(interfaceFile.c_str(), ios::in);

	// Try to open the file.
	if(file.is_open())
	{
		char tmpBuff[_MAX_LINE_SIZE];
		char delimiter[] = "[] \t";

		// While the end of the file is not reached.
		while(!file.eof())
		{
			// Get a line (teh line should not be more than _MAX_LINE_SIZE).
			file.getline(tmpBuff, _MAX_LINE_SIZE);
			// Get the first token delimited by the characters in delimiter.
			char *token = strtok(tmpBuff, delimiter);
			if(token != NULL)
			{
				CRGBA	rgbaOn;
				CRGBA	rgbaOff;
				CRGBA	rgbaDisable;

				bool	defaultCase = false;
				uint	id;
				uint	tON			= 0;
				uint	tOFF		= 0;
				uint	tDIS		= 0;

				CButtonBase::TBG	bgOn = CButtonBase::BG_stretch;
				CButtonBase::TBG	bgOff = CButtonBase::BG_stretch;
				CButtonBase::TBG	bgDisable = CButtonBase::BG_stretch;


				// If the id == "..." -> default case.
				if(strcmp(token, "...") == 0)
				{
					defaultCase = true;
				}
				else
				{
					// Make the id.
					id = atoi(token);
					defaultCase = false;
				}

				char *ptr = strtok(NULL, delimiter);
				while(ptr != NULL)
				{
					if     (strcmp(ptr, "BG_Mode_On:") == 0)
						bgOn = getBGMode2();
					else if(strcmp(ptr, "BG_Mode_Off:") == 0)
						bgOff = getBGMode2();
					else if(strcmp(ptr, "BG_Mode_Dis:") == 0)
						bgDisable = getBGMode2();

					else if(strcmp(ptr, "t_On:") == 0)
						tON = getInt();
					else if(strcmp(ptr, "t_Off:") == 0)
						tOFF = getInt();
					else if(strcmp(ptr, "t_Dis:") == 0)
						tDIS = getInt();

					else if(strcmp(ptr, "RGBA_On:") == 0)
						rgbaOn = getRGBA();
					else if(strcmp(ptr, "RGBA_Off:") == 0)
						rgbaOff = getRGBA();
					else if(strcmp(ptr, "RGBA_Dis:") == 0)
						rgbaDisable = getRGBA();

					// Next token
					ptr = strtok(NULL, delimiter);
				}

				// Set the Default Pen.
				if(defaultCase)
				{
					_DefaultButton.colorOn(rgbaOn);
					_DefaultButton.colorOff(rgbaOff);
					_DefaultButton.colorDisable(rgbaDisable);
					_DefaultButton.textureOn(tON);
					_DefaultButton.textureOff(tOFF);
					_DefaultButton.textureDisable(tDIS);
					_DefaultButton.bgModeOn(bgOn);
					_DefaultButton.bgModeOff(bgOff);
					_DefaultButton.bgModeDisable(bgDisable);
				}
				// Set specific Pens.
				else
				{
					// Insert the button.
					CButtonBase *button = new CButtonBase(tON, tOFF, tDIS, rgbaOn, rgbaOff, rgbaDisable);
					if(button)
					{
						button->bgModeOn(bgOn);
						button->bgModeOff(bgOff);
						button->bgModeDisable(bgDisable);
					}
					_Buttons.insert(TMapButtons::value_type(id, button));
				}
			}
		}
		file.close();
	}
}// loadButtons //

//-----------------------------------------------
// setFuncCtrl :
// Pointer on an array of TFuncCtrl.
//-----------------------------------------------
void CInterfMngr::setFuncCtrl(const TMapFuncCtrl &funcCtrlMap)
{
	_CtrlFunctions = funcCtrlMap;
}// setFuncCtrl //




//-----------------------------------------------
// getType :
// Get the type of the control.
//-----------------------------------------------
CInterfMngr::TTypCtrl CInterfMngr::getType(uint id)
{
	TMapCtrls::iterator it = _Ctrls.find(id);
	if(it != _Ctrls.end())
		return (*it).second;
	else
		return CtrlUnknown;
}// getType //

//-----------------------------------------------
// getText :
// Return the text corresponding to the Id.
//-----------------------------------------------
ucstring CInterfMngr::getText(uint id)
{
	TMapTexts::iterator it = _Texts.find(id);
	if(it != _Texts.end())
		return (*it).second;
	else
		return ucstring("");
}// getText //

//-----------------------------------------------
// getTexture :
// Return the texture file Corresponding to the id.
//-----------------------------------------------
UTextureFile * CInterfMngr::getTexture(uint id)
{
	TMapTextures::iterator it = _Textures.find(id);
	if(it != _Textures.end())
		return (*it).second;
	else
		return 0;
}// getTexture //

//-----------------------------------------------
// getPen :
// Get the right Pen for the control.
//-----------------------------------------------
CPen CInterfMngr::getPen(uint id)
{
	TMapPens::iterator it = _Pens.find(id);
	if(it != _Pens.end())
		return *((*it).second);
	else
		return _DefaultPen;
}// getPen //

//-----------------------------------------------
// getButton :
// Get the right Button for the control.
//-----------------------------------------------
CButtonBase CInterfMngr::getButton(uint id)
{
	TMapButtons::iterator it = _Buttons.find(id);
	if(it != _Buttons.end())
		return *((*it).second);
	else
		return _DefaultButton;
}// getButton //

//-----------------------------------------------
// getCtrl :
// ...
//-----------------------------------------------
CControl * CInterfMngr::getCtrl(uint idCtrl)
{
	CControl *ctrl = 0;
	TMapWindows::iterator itE = _Windows.end();
	for(TMapWindows::iterator it = _Windows.begin(); it != itE ; ++it)
	{
		ctrl = ((*it).second)->getCtrl(idCtrl);
		if(ctrl)
			break;
	}

	// Return the pointer of the control "id" or 0 if the control "id" doesn't exit.
	return ctrl;
}// getCtrl //


//-----------------------------------------------
// getOSDLook :
// Get the OSD Look.
//-----------------------------------------------
COSDBase CInterfMngr::getOSDLook(uint id)
{
	TMapOSDBase::iterator it = _OSDLook.find(id);
	if(it != _OSDLook.end())
		return *((*it).second);
	else
		return _DefaultOSDLook;
}// getOSDLook //




//-----------------------------------------------
// createOSD
// Create the OSD "id".
//-----------------------------------------------
COSD* CInterfMngr::createOSD(uint id, bool popUp)
{
	COSD *win = NULL;

	char filename[20];
	sprintf(filename,"%d.txt", id);
	string interfaceFile = CPath::lookup(filename);
	ifstream file(interfaceFile.c_str(), ios::in);

	// Open the file.
	if(file.is_open())
	{
		char tmpBuff[_MAX_LINE_SIZE];
		char delimiter[] = "[] \t";

		float x			= 0.f;
		float y			= 0.f;
		float width		= 1.f;
		float height	= 1.f;
		float minWidth	= 0.f;
		float minHeight	= 0.f;
		uint func		= 0;
		uint name		= 0;
		COSD::TBG bgMode	= COSD::BG_none;

		file.getline(tmpBuff, _MAX_LINE_SIZE);

		char *ptr = strtok(tmpBuff, delimiter);
		while(ptr != NULL)
		{
			// Get the pos X of the OSD.
			if(strcmp(ptr, "X:") == 0)
				x = getFloat();
			// Get the pos Y of the OSD.
			else if(strcmp(ptr, "Y:") == 0)
				y = getFloat();
			// Get the Width of the OSD.
			else if(strcmp(ptr, "Width:") == 0)
				width = getFloat();
			// Get the Height of the OSD.
			else if(strcmp(ptr, "Height:") == 0)
				height = getFloat();
			// Get the MinWidth of the OSD.
			else if(strcmp(ptr, "MinWidth:") == 0)
				minWidth = getFloat();
			// Get the MinHeight of the OSD.
			else if(strcmp(ptr, "MinHeight:") == 0)
				minHeight = getFloat();
			// Get the function to call each frame.
			else if(strcmp(ptr, "Function:") == 0)
				func = getInt();
			// Get the function to call each frame.
			else if(strcmp(ptr, "Name:") == 0)
				name = getInt();
			// Get Background Mode.
			else if(strcmp(ptr, "BG_Mode:") == 0)
				bgMode = getBGMode();

			// Next Token
			ptr = strtok(NULL, delimiter);
		}

		// Create the window.
		win = new COSD(id, x, y, 0.f, 0.f, 0.f, 0.f, width, height, minWidth, minHeight, popUp);
		if(win != 0)
		{
			COSDBase osdLook = getOSDLook(id);

			// Set the Background display mode.
			win->bgMode(osdLook._BG_Mode);
			// Set the BG Texture.
			win->bg(osdLook._BG);
			// Set the BG Color
			win->bgColor(osdLook._BG_Color);

			// Set the Title Bar display mode.
			win->tbMode(osdLook._TB_Mode);
			// Set the TB Texture.
			win->tb(osdLook._TB);
			// Set the TB Color.
			win->tbColor(osdLook._TB_Color);
			// Set the TB Pen.
			win->tbPen(osdLook._TB_Pen);

			// Set the HighLight Size.
			win->hlSize(osdLook._HL_Size);
			// Set the HL Color.
			win->hlColor(osdLook._HL_Color);

			// Set the Resize Size.
			win->rsSize(osdLook._RS_Size);
			// Set the RS Color.
			win->rsColor(osdLook._RS_Color);

//			win->osdMode(COSD::locked);
			// Change the OSD Name.
			win->osdName(getText(name));

			// Add the OSD to the OSD that are waiting insertion
			_Windows.insert( TMapWindows::value_type( id, win) );
			_OSDWaitingInsertion.push_back( win );

			// Open the OSD.
			win->open(file);

//			win->osdSetPosition(0.f, 0.f);
//			win->osdSetSize(1.f,1.f);
		}

		// Close the File.
		file.close();
	}

	return win;
}// createOSD //

//-----------------------------------------------
// deleteOSD :
// Delete the OSD "id".
//-----------------------------------------------
void CInterfMngr::deleteOSD(uint id)
{
	// store it the list of OSD waiting destruction
	_OSDWaitingDestruction.push_back(id);
}// deleteOSD //



//-----------------------------------------------
// resize :
// The window size has changed -> resize interface.
//-----------------------------------------------
void CInterfMngr::resize(uint32 width, uint32 height)
{
	_WindowSizeX = width;
	_WindowSizeY = height;

	// If the window is too small -> return;
	if(_WindowSizeX==0 || _WindowSizeY==0)
		return;

	// Update the interface.
	for(TOSD::iterator it = _OSD.begin(); it != _OSD.end(); it++)
	{
		// Resize all controls.
		(*it)->resize(width, height);
	}
}// resize //


//-----------------------------------------------
// update :
// Update Interfaces (for timer, etc.).
//-----------------------------------------------
void CInterfMngr::update(float x, float y)
{
	bool fullUse = true;

// delete all the OSD waiting destruction
	TUint::iterator itEID = _OSDWaitingDestruction.end();
	TMapWindows::iterator itW;

	for (TUint::iterator itID = _OSDWaitingDestruction.begin() ; itID != itEID ; ++itID)
	{

		itW = _Windows.find(*itID);
		if(itW != _Windows.end())
		{
			// Get the pointer.
			COSD *OSD = (*itW).second;

			// erase the OSD.
			_Windows.erase(itW);

			// Find the OSD in the display list.
			for(TOSD::iterator itOSD = _OSD.begin(); itOSD != _OSD.end(); itOSD++)
			{
				// if it's the right OSD -> erase.
				if((*itOSD) == OSD)
				{
					_OSD.erase(itOSD);
					break;
				}
			}

			// delete the OSD.
			if(OSD)
			{
				delete OSD;
				OSD = 0;
			}
		}
	}

	_OSDWaitingDestruction.clear();


// insert all OSD newly created
	const TOSD::const_iterator itNewEnd = _OSDWaitingInsertion.end();

	for (TOSD::const_iterator itNew = _OSDWaitingInsertion.begin() ; itNew != itNewEnd ; ++itNew)
	{
			_OSD.push_front((*itNew));
	}

	_OSDWaitingInsertion.clear();

// Update the interface.
//	const TOSD::iterator itE = _OSD.end();
	for(TOSD::iterator it = _OSD.begin(); it != _OSD.end() ; ++it)
	{
		if((*it)->update(x, y, fullUse))
			fullUse = false;
	}
}// update //

//-----------------------------------------------
// display :
// Display the interfaces in progress.
//-----------------------------------------------
void CInterfMngr::display()
{
	// If the window is too small -> return;
	if(_WindowSizeX==0 || _WindowSizeY==0)
		return;

	const TOSD::reverse_iterator itRE = _OSD.rend();
	for(TOSD::reverse_iterator it = _OSD.rbegin(); it != itRE ; ++it)
	{
		(*it)->display();
	}
}// display //

//-----------------------------------------------
// cursor :
// Return the cursor used by the interface at the moment.
// \return ECursor : 'Cur_None' if no cursor needed for the interface.
// \warning This method should be called after the update one to be up to date.
//-----------------------------------------------
COSD::ECursor CInterfMngr::cursor()
{
	// If the window is too small -> return;
	if(_WindowSizeX==0 || _WindowSizeY==0)
		return COSD::Cur_None;

	for(TOSD::iterator it = _OSD.begin(); it != _OSD.end() ; ++it)
	{
		COSD::ECursor curs = (*it)->cursor();
		if(curs != COSD::Cur_None)
			return curs;
	}

	return COSD::Cur_None;
}// cursor //

//-----------------------------------------------
// click :
// Manage the mouse click.
//-----------------------------------------------
bool CInterfMngr::click(float x, float y)
{
	// If the window is too small -> return;
	if(_WindowSizeX==0 || _WindowSizeY==0)
		return false;

	bool taken = false;
	bool focus = false;

	const TOSD::iterator itE = _OSD.end();
	TOSD::iterator itFrontOSD = itE;

	for(TOSD::iterator it = _OSD.begin(); it != itE ; ++it)
	{
		(*it)->click(x, y, taken);
		if( taken && (!focus) )
		{
			itFrontOSD = it;
			focus = true;
		}
	}

	if (itFrontOSD != itE)
	{
		_OSD.push_front(*itFrontOSD);
		_OSD.erase(itFrontOSD);
	}

	// Return if the click is taken by the interface.
	return taken;
}// click //



//-----------------------------------------------
// clickRight :
// Manage the mouse right click.
//-----------------------------------------------
bool CInterfMngr::clickRight(float x, float y)
{
	// If the window is too small -> return;
	if(_WindowSizeX==0 || _WindowSizeY==0)
		return false;

	bool taken = false;
	bool focus = false;

	const TOSD::iterator itE = _OSD.end();
	TOSD::iterator itFrontOSD = itE;

	for(TOSD::iterator it = _OSD.begin(); it != itE ; ++it)
	{
		(*it)->clickRight(x, y, taken);
		if( taken && (!focus) )
		{
			itFrontOSD = it;
			focus = true;
		}
	}

	if (itFrontOSD != itE)
	{
		_OSD.push_front(*itFrontOSD);
		_OSD.erase(itFrontOSD);
	}

	// Return if the click is taken by the interface.
	return taken;
}// clickRight //


//-----------------------------------------------
// getOSD :
// get the specified OSD adress
//-----------------------------------------------
COSD* CInterfMngr::getOSD( uint id)
{
	TMapWindows::const_iterator it, itE = _Windows.end();

	it = _Windows.find( id );

	if (it != itE)
		return (*it).second;
	else
		return NULL;

}// getOSD //
