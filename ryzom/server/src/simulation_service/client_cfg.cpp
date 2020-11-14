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
//#include "stdpch.h"
// Misc.
#include "nel/misc/config_file.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/hierarchical_timer.h"
// Client.
#include "client_cfg.h"
#include "nel/misc/path.h"
//#include "entities.h"
//#include "debug_client.h"
// Game Share.
//#include "game_share/time_weather_season/time_and_season.h"
//#include "game_share/ryzom_version.h"

///////////
// MACRO //
///////////
//-----------------------------------------------
/// Macro to read a Bool from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_BOOL(variableName)											\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asInt() ? true : false;		\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read an Int from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_INT(variableName)											\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asInt();						\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a Float from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_FLOAT(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asFloat();						\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a Float from the CFG.
/// cfgVariableName : Variable Name to Read.
/// variableName : Variable Name to Set.
//-----------------------------------------------
#define _READ_FLOAT2(cfgVariableName,variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#cfgVariableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asFloat();						\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#cfgVariableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a Double from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_DOUBLE(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asDouble();					\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a String from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_STRING(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asString();					\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\
		
//-----------------------------------------------
/// Macro to read a CVector from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_CVECTOR(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	if(varPtr)															\
	{																	\
		/* Check params */												\
		if(varPtr->size()==3)											\
		{																\
			ClientCfg.variableName.x = varPtr->asFloat(0);				\
			ClientCfg.variableName.y = varPtr->asFloat(1);				\
			ClientCfg.variableName.z = varPtr->asFloat(2);				\
		}																\
		else															\
			cfgWarning("CFG: Bad params for '"#variableName"' !!!");		\
	}																	\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\


//-----------------------------------------------
/// Macro to read an Enum, as int from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_ENUM_ASINT(type, variableName)							\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = (type)varPtr->asInt();					\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\


//-----------------------------------------------
// Macro for the dev version
//-----------------------------------------------

#if !FINAL_VERSION
#define READ_BOOL_DEV(variableName) _READ_BOOL(variableName)
#define READ_INT_DEV(variableName) _READ_INT(variableName)
#define READ_FLOAT_DEV(variableName) _READ_FLOAT(variableName)
#define READ_FLOAT2_DEV(cfgVariableName,variableName) _READ_FLOAT2(cfgVariableName,variableName)
#define READ_DOUBLE_DEV(variableName) _READ_DOUBLE(variableName)
#define READ_STRING_DEV(variableName) _READ_STRING(variableName)
#define READ_CVECTOR_DEV(variableName) _READ_CVECTOR(variableName)
#define READ_ENUM_ASINT_DEV(type, variableName) _READ_ENUM_ASINT(type, variableName)
#else // !FINAL_VERSION
#define READ_BOOL_DEV(variableName)
#define READ_INT_DEV(variableName)
#define READ_FLOAT_DEV(variableName)
#define READ_FLOAT2_DEV(cfgVariableName,variableName)
#define READ_DOUBLE_DEV(variableName)
#define READ_STRING_DEV(variableName)
#define READ_CVECTOR_DEV(variableName)
#define READ_ENUM_ASINT_DEV(type, variableName)
#endif // !FINAL_VERSION

//-----------------------------------------------
// Macro for the dev & final version
//-----------------------------------------------

#define READ_BOOL_FV(variableName) _READ_BOOL(variableName)
#define READ_INT_FV(variableName) _READ_INT(variableName)
#define READ_FLOAT_FV(variableName) _READ_FLOAT(variableName)
#define READ_FLOAT2_FV(cfgVariableName,variableName) _READ_FLOAT2(cfgVariableName,variableName)
#define READ_DOUBLE_FV(variableName) _READ_DOUBLE(variableName)
#define READ_STRING_FV(variableName) _READ_STRING(variableName)
#define READ_CVECTOR_FV(variableName) _READ_CVECTOR(variableName)
#define READ_ENUM_ASINT_FV(type, variableName) _READ_ENUM_ASINT(type, variableName)

///////////
// USING //
///////////
using namespace NLMISC;


////////////
// GLOBAL //
////////////
//CClientConfig LastClientCfg;
CClientConfig ClientCfg;
//const string ConfigFileName = "client.cfg";


////////////
// EXTERN //
////////////
//extern CRyzomTime		RT;
//extern string Cookie, FSAddr;

/////////////
// METHODS //
/////////////

// display only one time warning "Default values...."
static bool	DisplayCFGWarning = false;
static void	cfgWarning(const char *s)
{
	if(DisplayCFGWarning)
		nlwarning(s);
}

//---------------------------------------------------
// CClientConfig :
// Constructor.
//---------------------------------------------------
CClientConfig::CClientConfig()
{
	CConfigFile::setTimeout( 2000 );
	IsInvalidated		= false;
	SelectCharacter		= -1;						// Default is no auto select
	FSHost				= "localhost";				// Default Host.
	Local				= false;					// True => no server connection
//	NbConnections		= 0;
	//ForceDeltaTime		= 0;						// Default ForceDeltaTime, disabled by default
//	DataPath.push_back("data/");					// Default Path for the Data.
//	DataPath.push_back("data_leveldesign/");		// Default Path for the Level Design Directory.
//	DataPath.push_back("data_common/");				// Default Path for the Level Design Directory.
//	DataPathNoRecurse.push_back("data_leveldesign/leveldesign/Game_Elem");
	//UpdatePackedSheetPath.push_back("data_leveldesign");
	//UpdatePackedSheet	= false;						// Update packed sheet if needed
	//IdFilePath		= "sheet_id.bin";
	//Sleep				= -1;						// Default : client does not sleep.
	//ProcessPriority		= 0;						// Default : NORMAL
	//UserSheet			= "fyros.race_stats";	// Default sheet used.
	//AttackDist			= 0.5f;
	//RyzomDay			= 0;
	//RyzomTime			= 0.0f;	
	//ChangeDirAngle		= 1.70f;				// Default Angle.
	//SelectionDist		= 150.f;					// Default dist in meter.
	//LootDist			= 4.f;						// Default loot/harvest distance (in meter).
	//ForceLanguage		= false;
	LanguageCode		= "en";						// Default to english
	//DebugStringManager	= false;					// Default to no debug
//	VerboseVP				= false;
//	VerboseAllTraffic		= false;
	//DefaultEntity = "ccafb1.creature";
//	RestrainPI = true;
	//DumpVSIndex = false;
//	Check          = false;
	//FPExceptions			= false;				// Disable Floating Point Exceptions.
	//NeedComputeVS			= false;				// Do not need to compute Visual Slots.
//	AutoReloadFiles = false;
	//SimulateServerTick = false;
	SimulatePacketLossRatio= 0;
	//LogMemoryAllocation=false;
	//LogMemoryAllocationSize=8;
//	RequestQuit = false;
}// CClientConfig //

/*
//---------------------------------------------------
// load :
// Load the client config file.
//---------------------------------------------------
void CClientConfig::setValuesOnFileChange()
{
	//	ClientCfg.ConfigFile.print (InfoLog);
	
	// display an info only when the file change
	nlinfo ("reloading the config file!");
	
	setValues();
}
*/	
//---------------------------------------------------
// load :
// Load the client config file.
//---------------------------------------------------
void CClientConfig::setValues()
{
	CConfigFile::CVar *varPtr = 0;
//	static bool	firstTimeSetValues= true;

	READ_INT_DEV(SelectCharacter);
	
	READ_STRING_FV(FSHost);
	READ_BOOL_DEV(Local);

//	READ_INT_DEV(NbConnections);
//	READ_INT_DEV(RequestQuit);

	//////////
	// MISC //
	// Pre Data Path.
	/*try
	{
		CConfigFile::CVar &cvPreDataPath = ClientCfg.ConfigFile.getVar("PreDataPath");
		ClientCfg.PreDataPath.clear ();
		for (sint i = 0; i < cvPreDataPath.size(); i++)
			ClientCfg.PreDataPath.push_back(cvPreDataPath.asString(i));
	}
	catch(EUnknownVar &) {cfgWarning("Default value used for 'PreDataPath' !!!");}*/
/*	
	// Data Path.
	try
	{
		CConfigFile::CVar &cvDataPath = ClientCfg.ConfigFile.getVar("DataPath");
		ClientCfg.DataPath.clear ();
		for (sint i = 0; i < cvDataPath.size(); i++)
			ClientCfg.DataPath.push_back(cvDataPath.asString(i));
	}
	catch(EUnknownVar &) {cfgWarning("Default value used for 'DataPath' !!!");}

	// Data Path no recurse.
	try
	{
		CConfigFile::CVar &cvDataPathNoRecurse = ClientCfg.ConfigFile.getVar("DataPathNoRecurse");
		ClientCfg.DataPathNoRecurse.clear ();
		for (sint i = 0; i < cvDataPathNoRecurse.size(); i++)
			ClientCfg.DataPathNoRecurse.push_back(cvDataPathNoRecurse.asString(i));
	}
	catch(EUnknownVar &) {cfgWarning("Default value used for 'DataPathNoRecurse' !!!");}
	
	{
		H_AUTO(InitRZAddSearchPath2)
		uint i;
		for (i = 0; i < ClientCfg.DataPath.size(); i++)
		{
			CPath::addSearchPath(ClientCfg.DataPath[i], true, false);
		}
		for (i = 0; i < ClientCfg.DataPathNoRecurse.size(); i++)
		{
			CPath::addSearchPath(ClientCfg.DataPathNoRecurse[i], false, false);
		}
	}
*/	
//	READ_BOOL_DEV(VerboseVP)
//	READ_BOOL_DEV(VerboseAllTraffic)

//	READ_BOOL_DEV(RestrainPI)
//	READ_BOOL_DEV(Check)
	READ_INT_DEV(SimulatePacketLossRatio)
	
//	READ_BOOL_FV(AutoReloadFiles)

	// for reset effect of variable in mainLoop(), set true
	ClientCfg.IsInvalidated= true;

	// Allow warning display only first time.
	DisplayCFGWarning= false;

	// If it is the load time, bkup the ClientCfg into LastClientCfg
//	if(firstTimeSetValues)
//		LastClientCfg = ClientCfg;

	// no more true.
//	firstTimeSetValues= false;

}// load //


//-----------------------------------------------
// init :
//-----------------------------------------------
void CClientConfig::init(const string &configFileName)
{
	if(!NLMISC::CFile::fileExists(configFileName))
		nlwarning("CFG::init: '%s' Not Found !!!", configFileName.c_str ());

	// if the config file will be modified, it calls automatically the function setValuesOnFileChange()
//	ClientCfg.ConfigFile.setCallback (CClientConfig::setValuesOnFileChange);

	// load the config files
	ClientCfg.ConfigFile.load (configFileName);

	setValues();

}// init //

static const char* NegFiltersNames[] =
{
   "NegFiltersDebug",
   "NegFiltersInfo",
   "NegFiltersWarning",
   "NegFiltersAssert",
   "NegFiltersError",
   0
};


void cbLogFilter (CConfigFile::CVar &var)
{
	CLog *log = NULL;
	if (var.Name == "NegFiltersDebug")
	{
		log = DebugLog;
	}
	else if (var.Name == "NegFiltersInfo")
	{
		log = InfoLog;
	}
	else if (var.Name == "NegFiltersWarning")
	{
		log = WarningLog;
	}
	else if (var.Name == "NegFiltersAssert")
	{
		log = AssertLog;
	}
	else if (var.Name == "NegFiltersError")
	{
		log = ErrorLog;
	}
	else
	{
		nlstop;
	}

	nlinfo ("Updating %s from config file", var.Name.c_str());
	
	// remove all old filters from config file
	CConfigFile::CVar &oldvar = ClientCfg.ConfigFile.getVar (var.Name);
	for (sint j = 0; j < oldvar.size(); j++)
	{
		log->removeFilter (oldvar.asString(j).c_str());
	}

	// add all new filters from config file
	for (sint i = 0; i < var.size(); i++)
	{
		log->addNegativeFilter (var.asString(i).c_str());
	}
}