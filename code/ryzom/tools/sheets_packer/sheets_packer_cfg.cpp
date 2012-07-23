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

// Misc.
#include "nel/misc/config_file.h"
#include "nel/misc/bit_mem_stream.h"
// Client.
#include "sheets_packer_cfg.h"


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


////////////
// GLOBAL //
////////////
CClientConfig AppCfg;
const std::string ConfigFileName = "sheets_packer.cfg";


/////////////
// METHODS //
/////////////
//---------------------------------------------------
// CClientConfig :
// Constructor.
//---------------------------------------------------
CClientConfig::CClientConfig()
{
}// CClientConfig //


//---------------------------------------------------
// load :
// Load the client config file.
//---------------------------------------------------
void setValues()
{
	nlinfo ("reloading the config file!");

	CConfigFile::CVar *varPtr = 0;

	//////////
	// MISC //
	// Data Path.
	try
	{
		AppCfg.DataPath.clear ();
		CConfigFile::CVar &cvDataPath = AppCfg.ConfigFile.getVar("DataPath");
		for (uint i = 0; i < cvDataPath.size(); i++)
			AppCfg.DataPath.push_back(cvDataPath.asString(i));
	}
	catch(const EUnknownVar &) {nlwarning("Default value used for 'DataPath' !!!");}

	// World sheet name
	READ_STRING(WorldSheet)
	// Primitives path
	READ_STRING(PrimitivesPath)
	// Output data path
	READ_STRING(OutputDataPath)
	// Ligo primitive class
	READ_STRING(LigoPrimitiveClass)

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
	catch(const EUnknownVar &) {}
	try
	{
		CConfigFile::CVar &cvTmp = AppCfg.ConfigFile.getVar("NegFiltersInfo");
		for(uint k = 0; k < (uint)cvTmp.size(); ++k)
		{
			InfoLog->addNegativeFilter (cvTmp.asString(k).c_str());
		}
	}
	catch(const EUnknownVar &) {}
	try
	{
		CConfigFile::CVar &cvTmp = AppCfg.ConfigFile.getVar("NegFiltersWarning");
		for(uint k = 0; k < (uint)cvTmp.size(); ++k)
		{
			WarningLog->addNegativeFilter (cvTmp.asString(k).c_str());
		}
	}
	catch(const EUnknownVar &) {}
}// load //


//-----------------------------------------------
// serial :
// Serialize CFG.
//-----------------------------------------------
void CClientConfig::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Start the opening of a new node named ClientCFG.
	f.xmlPush("ClientCFG");

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
// release :
//-----------------------------------------------
void CClientConfig::release ()
{
}
