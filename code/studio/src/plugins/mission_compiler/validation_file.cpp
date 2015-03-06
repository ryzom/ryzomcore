#include "validation_file.h"

#include <nel/misc/config_file.h>
#include <nel/misc/path.h>

void CValidationFile::loadMissionValidationFile(std::string filename)
{	
	// load the configuration file
	NLMISC::CConfigFile cf;
	std::string pathName = NLMISC::CPath::lookup(filename, false);
	
	if (pathName.empty())
	{
		nlwarning("Can't find index file '%s' in search path, no mission will be valid", filename.c_str());
		return;
	}
	cf.load(pathName);
	
	// get the variable
	NLMISC::CConfigFile::CVar* var = cf.getVarPtr("AuthorizedStates");
	if (var)
	{
		for (uint i=0; i<var->size(); ++i)
			_AuthorizedStates.push_back(var->asString(i));
	}
	int missionStatesFields = 3;
	var = cf.getVarPtr("MissionStatesFields");
	if (var)
		missionStatesFields = var->asInt();
	else
		nlwarning("Mission validation file does not contain MissionStatesFields variable. Parsing may fail and corrupt data.");

	var = cf.getVarPtr("MissionStates");
	if (var)
	{
		for (uint i=0; i<var->size()/missionStatesFields; ++i)
		{
			std::string mission = var->asString(i*missionStatesFields);
			std::string stateName = var->asString(i*missionStatesFields+1);
			std::string hashKey = var->asString(i*missionStatesFields+2);
			_MissionStates.insert(std::make_pair(mission, CMissionState(mission, stateName, hashKey)));
		}
	}
}

void CValidationFile::saveMissionValidationFile(std::string filename)
{
	// load the configuration file
	std::string pathName = NLMISC::CPath::lookup(filename, false);
	
	if (pathName.empty())
	{
		nlwarning("Can't find index file '%s' in search path, no mission will be valid", filename.c_str());
		return;
	}
	FILE* file = fopen(pathName.c_str(), "w");
	nlassert(file!=NULL);
	
	// AuthorizedStates
	fprintf(file, "%s",
		"// AuthorizedStates contains the list of authorized states. EGS mission\n"
		"// manager can accept any number of states. Default state is the first one.\n"
		"AuthorizedStates = {\n");
	std::deque<std::string>::iterator itAuth, itAuthEnd = _AuthorizedStates.end();
	for (itAuth=_AuthorizedStates.begin(); itAuth!=itAuthEnd; ++itAuth)
		fprintf(file, "\t\"%s\",\n", itAuth->c_str());
	fprintf(file, "%s", "};\n\n");
	
	// MissionStatesFields
	fprintf(file, "%s",
		"// MissionStatesFields contains the number of fields in MissionStates, for\n"
		"// future compatibility purpose.\n"
		"MissionStatesFields = ");
	fprintf(file, "%d", 3); // 3 fields: name, state, hash key
	fprintf(file, "%s", ";\n\n");
	
	// MissionStates
	fprintf(file, "%s",
		"// MissionStates contains a list of mission with for each the state of the\n"
		"// mission and its hash key. The tool will add new missions with the default\n"
		"// state. It will flag missions with a modified hash key with default state to\n"
		"// prevent untested modified missions to be published.\n"
		"// :NOTE: You can add a field to this structure without the need to modify EGS\n"
		"// code. Simply update MissionStatesFields.\n"
		"MissionStates = {\n");
	TMissionStateContainer::iterator itMission, itMissionEnd = _MissionStates.end();
	for (itMission=_MissionStates.begin(); itMission!=itMissionEnd; ++itMission)
		fprintf(file, "\t%-42s %-12s \"%s\",\n", ("\""+itMission->second.name+"\",").c_str(), ("\""+itMission->second.state+"\",").c_str(), itMission->second.hashKey.c_str());
	fprintf(file, "};\n\n");

	fclose(file);
}

// :NOTE: This function exists in mission_template.cpp. If you change it here modify the other file.
std::string buildHashKey(std::string const& content)
{
	uint32 sum = 0;
	size_t size = content.length()/4;
	for (size_t i=0; i<size; ++i)
	{
		uint32 val = 0;
		for (int j=0; j<4; ++j)
			val += content[4*i+j]<<8*j;
		sum += val;
		if (sum&1)
			sum = sum>>1 | 0x80000000;
		else
			sum = sum>>1;
	}
	return NLMISC::toString("0x%08X", sum);
}

bool CMission::parsePrim(NLLIGO::IPrimitive const* prim)
{
	// init default values
	std::vector<std::string>* params;
	// get the mission script
	if (!prim->getPropertyByName("script", params) || !params)
	{
		nlwarning("ERROR : cant find mission script!!!!!!");
		return false;
	}
	
	// parse them
	std::string content;
	std::vector<std::string>::iterator itParam, itParamEnd = params->end();
	for (itParam=params->begin(); itParam!=itParamEnd; ++itParam)
	{
		content += *itParam + "\n";
	}
	hashKey = buildHashKey(content);
	return true;
}