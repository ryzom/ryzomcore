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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"

#if defined(NL_OS_WINDOWS)
#include <windows.h>
#endif
// Misc.
#include "nel/misc/config_file.h"
#include "nel/misc/bit_mem_stream.h"
// Client.
#include "sheets_packer_cfg.h"
#include "debug_client.h"
// 3D Interface.
//#include "nel/3d/u_driver.h"
//#include "nel/3d/u_scene.h"

// Game Share.
#include "game_share/time_weather_season/time_and_season.h"

///////////
// MACRO //
///////////
//-----------------------------------------------
/// Macro to read a Bool from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define READ_BOOL(variableName)											\
	/* Read the Variable Value From Script */							\
	varPtr = AppCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		AppCfg.variableName = varPtr->asInt() ? true : false;		\
	/* Use the Default Value */											\
	else																\
		nlwarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read an Int from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define READ_INT(variableName)											\
	/* Read the Variable Value From Script */							\
	varPtr = AppCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		AppCfg.variableName = varPtr->asInt();						\
	/* Use the Default Value */											\
	else																\
		nlwarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a Float from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define READ_FLOAT(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = AppCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		AppCfg.variableName = varPtr->asFloat();						\
	/* Use the Default Value */											\
	else																\
		nlwarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a Double from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define READ_DOUBLE(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = AppCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		AppCfg.variableName = varPtr->asDouble();					\
	/* Use the Default Value */											\
	else																\
		nlwarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a String from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define READ_STRING(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = AppCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		AppCfg.variableName = varPtr->asString();					\
	/* Use the Default Value */											\
	else																\
		nlwarning("CFG: Default value used for '"#variableName"' !!!");	\
		
//-----------------------------------------------
/// Macro to read a CVector from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define READ_CVECTOR(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = AppCfg.ConfigFile.getVarPtr(#variableName);				\
	if(varPtr)															\
	{																	\
		/* Check params */												\
		if(varPtr->size()==3)											\
		{																\
			AppCfg.variableName.x = varPtr->asFloat(0);				\
			AppCfg.variableName.y = varPtr->asFloat(1);				\
			AppCfg.variableName.z = varPtr->asFloat(2);				\
		}																\
		else															\
			nlwarning("CFG: Bad params for '"#variableName"' !!!");		\
	}																	\
	else																\
		nlwarning("CFG: Default value used for '"#variableName"' !!!");	\


///////////
// USING //
///////////
using namespace NLMISC;
//using namespace NL3D;


////////////
// GLOBAL //
////////////
CClientConfig AppCfg;
const std::string ConfigFileName = "sheets_packer.cfg";


////////////
// EXTERN //
////////////
//extern NL3D::UScene		*Scene;
//extern NL3D::UDriver	*Driver;
extern CRyzomTime		RT;

/////////////
// METHODS //
/////////////
//---------------------------------------------------
// CClientConfig :
// Constructor.
//---------------------------------------------------
CClientConfig::CClientConfig()
{
	SaveConfig			= false;
	PositionX			= 0;
	PositionY			= 0;
	Frequency			= 60;
	Windowed			= false;					// Default is windowed mode.
	Width				= 800;						// Default Width for the window.
	Height				= 600;						// Default Height for the window.
	Depth				= 32;						// Default Bit per Pixel.
	Contrast			= 0.f;						// Default Monitor Contrast.
	Luminosity			= 0.f;						// Default Monitor Luminosity.
	Gamma				= 0.f;						// Default Monitor Gamma.
	PreDataPath.push_back("../../client/data/gamedev/language/");	// Default Path for the language data
	DataPath.push_back("../../client/data/");					// Default Path for the Data.
	DataPath.push_back("../../common/data_leveldesign/");		// Default Path for the Level Design Directory.
	DataPath.push_back("../../common/data_common/");				// Default Path for the Level Design Directory.
	UpdatePackedSheet	= true;						// Update packed sheet if needed
	SceneName			= "";
	IdFilePath			= "";
	LanguageCode		= "en";						// Default to english
	FPExceptions		= false;				// Disable Floating Point Exceptions.
}// CClientConfig //


//---------------------------------------------------
// load :
// Load the client config file.
//---------------------------------------------------
void setValues()
{
	nlinfo ("reloading the config file!");

	CConfigFile::CVar *varPtr = 0;

	
	///////////////////
	// WINDOW CONFIG //
	// Mode.

	// SaveConfig
	varPtr = AppCfg.ConfigFile.getVarPtr ("SaveConfig");
	if (varPtr)
		AppCfg.SaveConfig = varPtr->asInt() ? true : false;
	else
		nlwarning ("Default value used for 'SaveConfig' !!!");

	// Window Positon
	varPtr = AppCfg.ConfigFile.getVarPtr ("PositionX");
	if (varPtr)
		AppCfg.PositionX = varPtr->asInt();
	else
		nlwarning ("Default value used for 'PositionX' !!!");
	varPtr = AppCfg.ConfigFile.getVarPtr ("PositionY");
	if (varPtr)
		AppCfg.PositionY = varPtr->asInt();
	else
		nlwarning ("Default value used for 'PositionY' !!!");

	// Window frequency
	varPtr = AppCfg.ConfigFile.getVarPtr ("Frequency");
	if (varPtr)
		AppCfg.Frequency = varPtr->asInt();
	else
		nlwarning ("Default value used for 'Frequency' !!!");

	try
	{
		CConfigFile::CVar &cvFullScreen = AppCfg.ConfigFile.getVar("FullScreen");
		AppCfg.Windowed = cvFullScreen.asInt() ? false : true;
	}
	catch(EUnknownVar &) {nlwarning("Default value used for 'Fullscreen' !!!");}
	// Width
	READ_INT(Width)
	// Height
	READ_INT(Height)
	// Depth : Bit Per Pixel
	READ_INT(Depth)
	// Contrast
	READ_FLOAT(Contrast)
	// Luminosity
	READ_FLOAT(Luminosity)
	// Gamma
	READ_FLOAT(Gamma)


	//////////
	// MISC //
	// Pre Data Path.
	try
	{
		CConfigFile::CVar &cvPreDataPath = AppCfg.ConfigFile.getVar("PreDataPath");
		AppCfg.PreDataPath.clear ();
		for (uint i = 0; i < cvPreDataPath.size(); i++)
			AppCfg.PreDataPath.push_back(cvPreDataPath.asString(i));
	}
	catch(EUnknownVar &) {nlwarning("Default value used for 'PreDataPath' !!!");}
	
	// Data Path.
	try
	{
		AppCfg.DataPath.clear ();
		CConfigFile::CVar &cvDataPath = AppCfg.ConfigFile.getVar("DataPath");
		for (uint i = 0; i < cvDataPath.size(); i++)
			AppCfg.DataPath.push_back(cvDataPath.asString(i));
	}
	catch(EUnknownVar &) {nlwarning("Default value used for 'DataPath' !!!");}

	// UpdatePackedSheet
	READ_BOOL(UpdatePackedSheet)

	// SceneName
	READ_STRING(SceneName)
	// IdFile Path
	READ_STRING(IdFilePath)

	/////////////
	// FILTERS //
	createDebug ();
	try
	{
		CConfigFile::CVar &cvTmp = AppCfg.ConfigFile.getVar("NegFiltersDebug");
		for(uint k = 0; k < (uint)cvTmp.size(); ++k)
		{
			DebugLog->addNegativeFilter (cvTmp.asString(k).c_str());
		}
	}
	catch(EUnknownVar &) {}
	try
	{
		CConfigFile::CVar &cvTmp = AppCfg.ConfigFile.getVar("NegFiltersInfo");
		for(uint k = 0; k < (uint)cvTmp.size(); ++k)
		{
			InfoLog->addNegativeFilter (cvTmp.asString(k).c_str());
		}
	}
	catch(EUnknownVar &) {}
	try
	{
		CConfigFile::CVar &cvTmp = AppCfg.ConfigFile.getVar("NegFiltersWarning");
		for(uint k = 0; k < (uint)cvTmp.size(); ++k)
		{
			WarningLog->addNegativeFilter (cvTmp.asString(k).c_str());
		}
	}
	catch(EUnknownVar &) {}
	
	READ_STRING(LigoPrimitiveClass)

	// LanguageCode
	READ_STRING(LanguageCode)


	READ_BOOL(FPExceptions)



	//////////
	// INIT //
	// FPU
#ifdef NL_OS_WINDOWS
	if(AppCfg.FPExceptions)
		_control87(_EM_INVALID|_EM_DENORMAL/*|_EM_ZERODIVIDE|_EM_OVERFLOW*/|_EM_UNDERFLOW|_EM_INEXACT, _MCW_EM);
	else
		_control87(_EM_INVALID|_EM_DENORMAL|_EM_ZERODIVIDE|_EM_OVERFLOW|_EM_UNDERFLOW|_EM_INEXACT, _MCW_EM);
#endif // NL_OS_WINDOWS

/*	if(Driver)
	{
		// Set the monitor color properties
		CMonitorColorProperties monitorColor;
		for(uint i=0; i<3; i++)
		{
			monitorColor.Contrast[i]	= AppCfg.Contrast;
			monitorColor.Luminosity[i]	= AppCfg.Luminosity;
			monitorColor.Gamma[i]		= AppCfg.Gamma;
		}
		if(!Driver->setMonitorColorProperties(monitorColor))
			nlwarning("reloadCFG: setMonitorColorProperties fails");
	}
*/
}// load //


//-----------------------------------------------
// serial :
// Serialize CFG.
//-----------------------------------------------
void CClientConfig::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Start the opening of a new node named ClientCFG.
	f.xmlPush("ClientCFG");

		f.xmlPushBegin("Windowed");
		f.xmlPushEnd();
		f.serial(Windowed);
		f.xmlPop();

		f.xmlPushBegin("Width");
		f.xmlPushEnd();
		f.serial(Width);
		f.xmlPop();

		f.xmlPushBegin("Height");
		f.xmlPushEnd();
		f.serial(Height);
		f.xmlPop();

		f.xmlPushBegin("Depth");
		f.xmlPushEnd();
		f.serial(Depth);
		f.xmlPop();

		f.xmlPushBegin("Contrast");
		f.xmlPushEnd();
		f.serial(Contrast);
		f.xmlPop();

		f.xmlPushBegin("Luminosity");
		f.xmlPushEnd();
		f.serial(Luminosity);
		f.xmlPop();

		f.xmlPushBegin("Gamma");
		f.xmlPushEnd();
		f.serial(Gamma);
		f.xmlPop();

	// Close the serial for hte Client CFG.
	f.xmlPop();
}// serial //


//-----------------------------------------------
// init :
//-----------------------------------------------
void CClientConfig::init(const std::string &configFileName)
{
	if(!CFile::fileExists(configFileName))
		nlwarning("CFG::init: '%s' Not Found !!!", configFileName.c_str ());

	// if the config file will be modified, it calls automatically the function setValues()
	AppCfg.ConfigFile.setCallback (setValues);

	// load the config files
	AppCfg.ConfigFile.load (configFileName);
}// init //


//-----------------------------------------------
// init :
//-----------------------------------------------
void CClientConfig::release ()
{
	// Do we have to save the cfg file ?
	if (AppCfg.SaveConfig)
	{
		// Are we in window mode ?
		if (AppCfg.Windowed)
		{
/*			// Driver still alive ?
			if (Driver && Driver->isActive ())
			{
#ifdef NL_OS_WINDOWS
				HWND hWnd = (HWND)Driver->getDisplay ();

				// Get the window position
				RECT window;
				RECT client;
				GetWindowRect (hWnd, &window);
				GetClientRect (hWnd, &client);

				// Save values
				try
				{
					CConfigFile::CVar *varPtr = AppCfg.ConfigFile.getVarPtr ("PositionX");
					if (varPtr)
						varPtr->setAsInt (window.left);
					varPtr = AppCfg.ConfigFile.getVarPtr ("PositionY");
					if (varPtr)
						varPtr->setAsInt (window.top);
					varPtr = AppCfg.ConfigFile.getVarPtr ("Width");
					if (varPtr)
						varPtr->setAsInt (client.right - client.left);
					varPtr = AppCfg.ConfigFile.getVarPtr ("Height");
					if (varPtr)
						varPtr->setAsInt (client.bottom - client.top);
				}
				catch (Exception &e)
				{
					nlwarning ("Error while set config file variables : %s", e.what ());
				}
#endif // NL_OS_WINDOWS
			}
*/		}

		// Save it
		AppCfg.ConfigFile.save ();
	}
}
