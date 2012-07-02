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

#include "ai_script_data_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;




/*
	Persistent variables structures
*/


#define PERSISTENT_TOKEN_FAMILY AiTokenFamily

#define PERSISTENT_MACROS_AUTO_UNDEF


/*
	Persistent variables structures
*/


#define PERSISTENT_CLASS CPersistentVariables

#define PERSISTENT_PRE_APPLY nlinfo("Staring Persistent var apply.");

#define PERSISTENT_DATA\
    STRUCT_MAP( string, SPersistentVariableSet, _VariableSets ) \

#define PERSISTENT_POST_APPLY	\
	TVariableSets::iterator first(_VariableSets.begin()), last(_VariableSets.end());	\
	for (; first != last; ++first)	\
	{	\
		nldebug("End of variable set '%s'", first->first.c_str());	\
		TVariables::iterator first2(first->second.Variables.begin()), last2(first->second.Variables.end());	\
		for (; first2 != last2; ++first2)	\
		{	\
			nldebug("Var '%s' = '%s'", first2->first.c_str(), first2->second.c_str());	\
		}	\
	}	\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


/*
	Persistent variables structures
*/

#define PERSISTENT_CLASS SPersistentVariableSet

#define PERSISTENT_PRE_APPLY nlinfo("Staring Persistent variable set apply.");

#define PERSISTENT_DATA\
    PROP_MAP( string, string, Variables ) \
	
#define PERSISTENT_POST_APPLY	\
	nldebug("End of variable set, read %u var from PDR", Variables.size());	\
	TVariables::iterator first(Variables.begin()), last(Variables.end());	\
	for (; first != last; ++first)	\
	{	\
		nldebug("Var '%s' = '%s'", first->first.c_str(), first->second.c_str());	\
	}	\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"






// Automatic SaveShardRoot path standardization
void cbOnSaveShardRootModified( NLMISC::IVariable& var )
{
	var.fromString( CPath::standardizePath( var.toString() ) );
}


// (SaveShardRoot from game_share/backup_service_interface.cpp is not instanciated because nothing is used from that file)
extern NLMISC::CVariable<std::string> SaveShardRoot;
CVariable<string>	PdrFilename("ai", "PdrFilename", "Pdr file containing AIScript variables", string("ai_persistent_var.pdr"), 0, true);


CAIScriptDataManager* CAIScriptDataManager::_instance = 0;

CAIScriptDataManager* CAIScriptDataManager::getInstance()
{
	if (_instance==0)
	{
		_instance = new CAIScriptDataManager();
		_instance->init();
	}
	return _instance;
}

void CAIScriptDataManager::destroyInstance()
{
	delete _instance;
	_instance = 0;
}

CAIScriptDataManager::CAIScriptDataManager()
{
}

string CAIScriptDataManager::makePdrFileName()
{
	string aisName;
	// get the AIS local path
	CConfigFile::CVar *var = IService::getInstance()->ConfigFile.getVarPtr("AESAliasName");
	if (var)
		aisName = var->asString(0);
	else
		aisName = "unamed_ais";

	return string("ai_script_data/")+aisName+"_pdr.bin";
}

void CAIScriptDataManager::init()
{
	CFile::createDirectory(dirname());

	CAIVarUpdateCallback *cb = new CAIVarUpdateCallback();

	//Bsi.requestFile(PdrFilename, cb);
	Bsi.syncLoadFile(makePdrFileName(), cb);

}

CAIScriptDataManager::~CAIScriptDataManager()
{
//	save();
//	FOREACH(itFile, TFileContainer, _Files)
//	{
//		NLMISC::CConfigFile* file = itFile->second;
//		if (file)
//			delete file;
//	}
//	_Files.clear();
}

//void CAIScriptDataManager::save()
//{
//	FOREACH(itFile, TFileContainer, _Files)
//	{
//		NLMISC::CConfigFile* file = itFile->second;
//		if (file)
//			file->save();
//	}
//	// :TODO: It may be wise to close all the files (ie delete CConfigFile)
//	// when we save, to save memory and 'save time' if we open several files
//}

std::string CAIScriptDataManager::dirname()
{
	return SaveShardRoot.get()+"/"+IService::getInstance()->SaveFilesDirectory.toString()+"/ai_script_data";
}

//CConfigFile* CAIScriptDataManager::createFile(string name)
//{
//	CConfigFile* file = new CConfigFile;
//	string fullfilename = dirname() + "/" + name + ".ai_script_data";
//	if (!CFile::fileExists(fullfilename) || CFile::getFileSize(fullfilename)==0)
//	{
//		FILE* fp = fopen(fullfilename.c_str(), "w");
//		if (fp)
//		{
//			fprintf(fp, "// This file contains data for the AI script\n");
//			fprintf(fp, "foo = 0;\n");
//			fclose(fp);
//		}
//	}
//	try
//	{
//		file->load(fullfilename);
//	}
//	catch (const EFileNotFound &e)
//	{
//		nlwarning("File not found while trying to load an AI script data file %s", fullfilename.c_str());
//	}
//	catch (const NLMISC::Exception &e)
//	{
//		nlwarning("Error while loading AI script data file %s", fullfilename.c_str());
//	}
//	_Files.insert(make_pair(name, file));
//	
//	return file;
//}

//CConfigFile* CAIScriptDataManager::getFile(string name)
//{
//	TFileContainer::iterator itFile = _Files.find(name);
//	if (itFile!=_Files.end())
//		return itFile->second;
//	else
//		return createFile(name);
//}

string CAIScriptDataManager::getVar_s(const string &name) const
{
	// Split name in filename and varname
	string::size_type pos = name.find(':');
	if (pos!=string::npos && pos!=(name.length()-1))
	{
		string filename = name.substr(0, pos);
		string varname = name.substr(pos+1);
		
		// Get a file ptr
		/*CConfigFile* file = getFile(filename);
		if (!file)
		{
			nlwarning("Unable to access ai script data file '%s'", filename.c_str());
			return "";
		}
		// Create var
		CConfigFile::CVar* var = file->getVarPtr(varname);
		*/
		TVariableValue var = _PersistentVariables.get(filename, varname);
		return var;
		/*if (var)
			return var->asString();*/
		// :NOTE: If var don't exists we simply return default value
	}
	else
	{
		nlwarning("AI script data variable name '%s' is misformed. Should be filename:varname", name.c_str());
	}
	return "";
}

float CAIScriptDataManager::getVar_f(const string &name) const
{
	return (float)atof(getVar_s(name).c_str());
//	// Split name in filename and varname
//	string::size_type pos = name.find(':');
//	if (pos!=string::npos && pos!=(name.length()-1))
//	{
//		string filename = name.substr(0, pos);
//		string varname = name.substr(pos+1);
//		// Get a file ptr
//		/*CConfigFile* file = getFile(filename);
//		if (!file)
//		{
//			nlwarning("Unable to access ai script data file '%s'", filename.c_str());
//			return 0.f;
//		}
//		// Create var
//		CConfigFile::CVar* var = file->getVarPtr(varname);
//		if (var)
//			return var->asFloat();*/
//		TVariableValue varValue = _PersistentVariables.get(filename, varname);
//		return (float)atof(varValue.c_str());
//		// :NOTE: If var don't exists we simply return default value
//	}
//	else
//	{
//		nlwarning("AI script data variable name is misformed. Should be filename:varname");
//	}
//	return 0.f;
}

void CAIScriptDataManager::setVar(const string &name, const string &value)
{
	// Split name in filename and varname
	string::size_type pos = name.find(':');
	if (pos!=string::npos && pos!=(name.length()-1))
	{
		string filename = name.substr(0, pos);
		string varname = name.substr(pos+1);
		// Get a file ptr
		/*CConfigFile* file = getFile(filename);
		if (!file)
		{
			nlwarning("Unable to access ai script data file '%s'", filename.c_str());
			return;
		}
		// Create var
		CConfigFile::CVar* var = file->getVarPtr(varname);
		if (!var)
			createVar(file, varname, value);
		else
		//	var->setAsString(value);
			var->forceAsString(value);*/
		_PersistentVariables.set(filename, varname, value);
	}
	else
	{
		nlwarning("AI script data variable name is misformed. Should be filename:varname");
	}
}

void CAIScriptDataManager::setVar(const string &name, float value)
{
	// Split name in filename and varname
	string::size_type pos = name.find(':');
	if (pos!=string::npos && pos!=(name.length()-1))
	{
		string filename = name.substr(0, pos);
		string varname = name.substr(pos+1);
		// Get a file ptr
		/*CConfigFile* file = getFile(filename);
		if (!file)
		{
			nlwarning("Unable to access ai script data file '%s'", filename.c_str());
			return;
		}
		// Create var
		CConfigFile::CVar* var = file->getVarPtr(varname);
		if (!var)
			createVar(file, varname, value);
		else
		//	var->setAsFloat(value);
			var->forceAsDouble((double)value);*/
		_PersistentVariables.set(filename, varname, toString(value));
	}
	else
	{
		nlwarning("AI script data variable name is misformed. Should be filename:varname");
	}
}

void CAIScriptDataManager::deleteVar(const std::string &name)
{
	string::size_type pos = name.find(':');
	if (pos!=string::npos && pos!=(name.length()-1))
	{
		string filename = name.substr(0, pos);
		string varname = name.substr(pos+1);
		_PersistentVariables.deleteVar(filename, varname);
	}
	else
	{
		nlwarning("AI script data variable name is misformed. Should be filename:varname");
	}
	
}


bool CAIScriptDataManager::needsPersistentVarUpdate()
{
	return _PersistentVariables.isDirty();
}

void CAIScriptDataManager::clearDirtyFlag()
{
	_PersistentVariables.clearDirtyFlag();
}

const CPersistentVariables &CAIScriptDataManager::getPersistentVariables() const
{
	return _PersistentVariables;
}

TVariableValue CPersistentVariables::get(const TVariableSetName& setName, const TVariableName& variableName) const
{
	TVariableSets::const_iterator it(_VariableSets.find(setName));
	if (it != _VariableSets.end())
	{
		TVariables::const_iterator it2(it->second.Variables.find(variableName));
		if (it2 != it->second.Variables.end())
			return it2->second;
	}
	// no match, return an empty string.
	return string();
}

void CPersistentVariables::set(const TVariableSetName& setName, const TVariableName& variableName, const TVariableValue& value)
{
	TVariableValue& theValue= _VariableSets[setName].Variables[variableName];
	if (value != theValue)
	{
		theValue = value;
		_Dirty = true; 
	}

}

void CPersistentVariables::deleteVar(const TVariableSetName& setName, const TVariableName& variableName)
{
	_VariableSets[setName].Variables.erase(variableName);
	if (_VariableSets[setName].Variables.size() == 0)
	{
		_VariableSets.erase(setName);
	}
	_Dirty = true;
	
}

bool CPersistentVariables::isDirty() const
{
	return _Dirty;
}
    

void CPersistentVariables::clearDirtyFlag()
{
	_Dirty = false;
}

//
//called once at AIScriptDataManager singleton instanciation
//
void CAIVarUpdateCallback::callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
{
	nldebug("CAIVarUpdateCallback::callback : loading persistent var from pdr file");
	// on receive file from backup
	CPersistentDataRecord    pdr;

	if (pdr.fromBuffer(dataStream))
	{
		nldebug("CAIVarUpdateCallback::callback : buffer is a valid pdr");
		// apply the loaded pdr record to the new character
		CAIScriptDataManager::getInstance()->_PersistentVariables.apply(pdr);
	}
}


NLMISC_CATEGORISED_COMMAND (ais, convertPersistentVarFiles, "Convert all old persistent var to the new system", "<no arg>")
{
	// load all the legacy cfg files containing the persistent var and
	// fill the new persistent var structure, then save the new persistent
	// var file using the BS.

	log.displayNL("Converting all local script data file from '%s'...", CAIScriptDataManager::getInstance()->dirname().c_str());
	// get all the ai data files
	vector<string>	files;
	CPath::getPathContent(CAIScriptDataManager::getInstance()->dirname(), false, false, true, files);

	log.displayNL("Found %u files", files.size());
	for (uint i=0; i<files.size(); ++i)
	{
		if (files[i].find(".ai_script_data") != string::npos)
		{
			log.displayNL("Loading script data file '%s'", files[i].c_str());
			// extract the filename
			string filename = CFile::getFilenameWithoutExtension(files[i]);

			// ok, load this one
			CConfigFile cfg;
			cfg.load(files[i], false);

			// parse all the 
			for (uint j=0; j<cfg.getNumVar(); ++j)
			{
				CConfigFile::CVar *var = cfg.getVar(j);

				log.displayNL("  Adding var '%s' = '%s'", (filename+":"+var->Name).c_str(), var->asString(0).c_str());
				CAIScriptDataManager::getInstance()->setVar(filename+":"+var->Name, var->asString(0));
			}
		}
		else
		{
			log.displayNL("Ignoring unreconized file '%s'", files[i].c_str());
		}
	}

	// at next tick update, the AIS will write the changed var file
	
	return true;
}
