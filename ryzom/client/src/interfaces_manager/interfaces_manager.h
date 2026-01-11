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



#ifndef CL_INTERFACES_MANAGER_H
#define CL_INTERFACES_MANAGER_H


//////////////
// Includes //
//////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/ucstring.h"
// Interface 3D.
#include "nel/3d/u_texture.h"
// Client.
#include "pen.h"
#include "button_base.h"
#include "osd_base.h"
#include "osd.h"
#include <list>
#include <vector>
#include <map>
#include <string>


///////////
// Using //
///////////
/*using std::list;
using std::map;
using std::vector;
using std::pair;
using std::string;*/
using NL3D::UTextureFile;


/**
 * Class to manage the interfaces.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CInterfMngr
{
public:
	enum TTypCtrl
	{
		CtrlUnknown = 0,
		CtrlText,
		CtrlCapture,
		CtrlBitmap,
		CtrlButton,
		CtrlRadioController,
		CtrlRadioButton,
		CtrlList,
		CtrlMultiList,
		CtrlChat,
		CtrlChatInput,
		CtrlChoiceList,
		CtrlCandidateList,
		CtrlHorizontalList,
		CtrlControlList,
		CtrlSpellList,
		CtrlProgressBar,
		CtrlCastingBar,
		CtrlBrick,
	};

	typedef std::map<uint, TTypCtrl>			TMapCtrls;
	typedef std::map<uint, ucstring>			TMapTexts;
	typedef std::map<uint, UTextureFile*>	TMapTextures;
	typedef std::map<uint, CPen*>			TMapPens;
	typedef std::map<uint, CButtonBase*>		TMapButtons;
	typedef std::map<uint, COSDBase*>		TMapOSDBase;

	typedef std::map<uint, COSD*>			TMapWindows;
	typedef std::list<COSD*>					TOSD;
	typedef std::vector<uint>				TUint;

	/// Type of the callback function parameter.
	typedef void * TParam;
	/// Callback function for the interface controls.
	typedef void (*TFuncCtrl) (uint, TParam);
	// map callback functions with their id (uint16)
	typedef map<uint16, TFuncCtrl>		TMapFuncCtrl;

	static uint32 _WindowSizeX;
	static uint32 _WindowSizeY;

private:
	/// Map all ctrl functions with their number (defined in functions.cpp)
	static TMapFuncCtrl	_CtrlFunctions;

	/// A Map to contain all the texts in the Interfaces.
	static TMapCtrls	_Ctrls;
	static TMapTexts	_Texts;
	static TMapTextures	_Textures;
	/// Default Pen Properties.
	static CPen			_DefaultPen;
	/// Some Specific Pen Properties.
	static TMapPens		_Pens;
	/// Default Button Look.
	static CButtonBase	_DefaultButton;
	/// Some specific Button Look
	static TMapButtons	_Buttons;
	/// Default OSD Look.
	static COSDBase		_DefaultOSDLook;
	/// Some specific OSD Look.
	static TMapOSDBase	_OSDLook;

	static TMapWindows	_Windows;
	static TOSD			_OSD;

	/// The list of OSD (id) that are waiting destruction (they are destroyed at the beginning of the 'update' method)
	static TUint		_OSDWaitingDestruction;

	/// list of the OSD (and their id) that were created since the last call to update and that are waiting insertion into _OSD and _Windows
	static TOSD			_OSDWaitingInsertion;


	/// Load all texts for the interfaces. CALL THIS FUNCTION BEFORE.
	static void loadTexts();
	/// Load all textures needed for the interfaces.
	static void loadTextures();
	/// Load Pens from file.
	static void loadPens();
	/// Load the array to convert a control id into a control type.
	static void loadCtrls();
	/// Load Buttons from file.
	static void loadButtons();
	/// Load The look of OSDs.
	static void loadOSDLook();

	/**
	* set the controls function map
	* \param a map of ctrl functions
	*/
	static void setFuncCtrl(const TMapFuncCtrl &funcCtrlMap);

public:
	/// Initialize the interfaces manager.
	static void init(const TMapFuncCtrl &funcCtrlMap, uint32 width, uint32 height);
	/// Release all the interface.
	static void release();

	/// Get the type of the control.
	static TTypCtrl getType(uint id);
	/// Return the text corresponding to the Id.
	static ucstring getText(uint id);
	/// Return the texture file Corresponding to the id.
	static UTextureFile *getTexture(uint id);
	/// Get the right Pen for the control.
	static CPen getPen(uint id);
	/// Get the right Button for the control.
	static CButtonBase getButton(uint id);

	/// Get the OSD Look.
	static COSDBase getOSDLook(uint id);

	///
	static void runFuncCtrl(uint num, uint ctrlId, TParam param = 0)
	{
		TMapFuncCtrl::iterator it = _CtrlFunctions.find( num );
		if (it != _CtrlFunctions.end() )
		{
			TFuncCtrl func = (*it).second;
			func(ctrlId, param);
		}
		else
		{
			nlwarning("Invalid function called - Num function: %d, Ctrl: %d", num, ctrlId);
		}
	}

	/**
	* Create the OSD "id", NB : the OSD is inserted into the _OSD list at the beginning of the next call of 'update()'
	* \param uint the OSD 'id'
	* \param bool true if the window is a pop-up window (default = false)
	* \return COSD* adress of the newly created OSD, or NULL if creation failed
	*/
	static COSD* createOSD(uint id, bool popUp = false);

	/**
	* get the OSD which 'id' is specified
	* \param uint the OSD 'id'
	* \return COSD* adress of the OSD if found, NULL otherwise
	*/
	static COSD* getOSD(uint id);

	/// Delete the OSD "id".
	static void deleteOSD(uint id);

	static CControl * getCtrl(uint idCtrl);

	/// Update Interfaces (for timer, etc.).
	static void update(float x, float y);
	/// Display the interfaces in progress.
	static void display();

	/**
	 * Return the cursor used by the interface at the moment.
	 * \return ECursor : 'Cur_None' if no cursor needed for the interface.
	 * \warning This method should be called after the update one to be up to date.
	 */
	static COSD::ECursor cursor();

	/// Manage the mouse click.
	static bool click(float x, float y);

	/// Manage the mouse click with the right button
	static bool clickRight(float x, float y);

	/// The window size has changed -> resize interface.
	static void resize(uint32 width, uint32 height);

	static void getWindowSize(uint32 &x, uint32 &y)
	{
		x = _WindowSizeX;
		y = _WindowSizeY;
	}
};



#endif // CL_INTERFACES_MANAGER_H

/* End of interfaces_manager.h */
