#ifndef VALIDATION_FILE_H
#define VALIDATION_FILE_H

#include <deque>
#include <map>
#include <string>

#include <nel/ligo/primitive.h>

struct CMissionState
{
	std::string name;
	std::string state;
	std::string hashKey;
	CMissionState(std::string _name, std::string _state, std::string _hashKey)
	: name(_name), state(_state), hashKey(_hashKey) { }
};

struct CMission
{
	std::string name;
	std::string hashKey;
	CMission(std::string _name, std::string _hashKey)
	: name(_name), hashKey(_hashKey) { }
	bool parsePrim(NLLIGO::IPrimitive const* prim);
};

class CValidationFile
{
public:
	typedef std::map<std::string, CMissionState> TMissionStateContainer;
	std::deque<std::string> _AuthorizedStates;
	TMissionStateContainer _MissionStates;
public:
	//	CValidationFile() { }
	void loadMissionValidationFile(std::string filename);
	void saveMissionValidationFile(std::string filename);
	void insertMission(std::string const& mission, std::string const& hashKey)
	{
		_MissionStates.insert(std::make_pair(mission, CMissionState(mission, defaultState(), hashKey)));
	}
	std::string defaultState()
	{
		if (!_AuthorizedStates.empty())
			return _AuthorizedStates.front();
		else
			return "";
	}
};

#endif // VALIDATION_FILE_H