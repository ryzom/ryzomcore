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
#ifndef RYAI_AI_SCRIPT_DATA_MANAGER_H
#define RYAI_AI_SCRIPT_DATA_MANAGER_H



//#include "nel/misc/variable.h"
#include "game_share/backup_service_interface.h"

extern NLMISC::CVariable<std::string> PdrFilename;




/*
	Persistent variables structures
*/
typedef std::string TVariableName;
typedef std::string TVariableValue;
typedef std::string TVariableSetName;

typedef std::map<TVariableName,TVariableValue> TVariables;

struct SPersistentVariableSet
{
public:
    DECLARE_PERSISTENCE_METHODS 
    TVariables Variables;
};



typedef std::map<TVariableSetName,SPersistentVariableSet> TVariableSets;

class CPersistentVariables
{
public:
    DECLARE_PERSISTENCE_METHODS   
    TVariableValue get(const TVariableSetName& setName, const TVariableName& variableName) const;
    void set(const TVariableSetName& setName, const TVariableName& variableName, const TVariableValue& value);
	void deleteVar(const TVariableSetName& setName, const TVariableName& variableName);
    bool isDirty() const;
    void clearDirtyFlag();
	const TVariableSets &getVariableSet() const { return _VariableSets; };

private:
    TVariableSets _VariableSets;
    bool _Dirty;
};

/**	Manager for AI script data (persistent variables)
	
	This class is a singleton.
	
	@author vuarand
*/
class CAIScriptDataManager
{
public:
	/// Returns a pointer to the manager.
	static CAIScriptDataManager* getInstance();
	/// Destroys the manager.
	static void destroyInstance();
	
	CAIScriptDataManager();
	~CAIScriptDataManager();
	
	std::string getVar_s(const std::string &name) const;
	float getVar_f(const std::string &name) const;
	
	void setVar(const std::string &name, const std::string &value);
	void setVar(const std::string &name, float value);
	void deleteVar(const std::string &name);
	
//	void save();
	
	/// Build the directory name where the old persistent cfg file are stored
	std::string dirname();
	/// Build the name of the pdr file for backup service storage.
	std::string makePdrFileName();
	
	/*persistent variables methods*/
	bool needsPersistentVarUpdate();	
	void clearDirtyFlag();
	const CPersistentVariables &getPersistentVariables() const;

		
private:
	NLMISC::CConfigFile* createFile(const std::string &name);
	NLMISC::CConfigFile* getFile(const std::string &name);
	void init();
	
private:
	static CAIScriptDataManager* _instance;
	
//	typedef std::map<std::string, NLMISC::CConfigFile*> TFileContainer;
//	typedef std::map<std::pair<std::string, std::string>, NLMISC::CConfigFile::CVar*> TVarContainer;
//	TFileContainer _Files;

	CPersistentVariables _PersistentVariables;

	friend class CAIVarUpdateCallback;
	NLMISC_CATEGORISED_COMMAND_FRIEND(ais, convertPersistentVarFiles);
};

//-------------------------------------------------------------------------------------------------
// class CAIVarUpdateCallback
//-------------------------------------------------------------------------------------------------

class CAIVarUpdateCallback: public IBackupFileReceiveCallback
{
public:
	CAIVarUpdateCallback() {}
	void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream);
};




#endif
