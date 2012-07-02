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

#ifndef PROFILE_OWNER_H
#define PROFILE_OWNER_H

#include <algorithm>
#include "alias_tree_owner.h"
#include "profile.h"

//////////////////////////////////////////////////////////////////////////////
// CProfileParameters                                                       //
//////////////////////////////////////////////////////////////////////////////

class CProfileParameters
{
public:
	struct TProfileParameter
	{
		std::string	Name;
		std::string	ValueStr;
		float		ValueFloat;
		
		TProfileParameter(std::string name, std::string valueStr, float valueFloat)
		: Name(name)
		, ValueStr(valueStr)
		, ValueFloat(valueFloat)
		{
		}
	};
	
	typedef std::vector<TProfileParameter> TProfileParameters;
	
private:
	struct TFindParameter
	: public std::unary_function<TProfileParameter, bool>
	{
		std::string _Value;
		TFindParameter(std::string const& value)
		: _Value(value)
		{
		}
		bool operator ()(TProfileParameter const& param) const
		{
			return param.Name==_Value;
		}
	};
	
public:
	void setProfileParameters(std::vector<std::string> const& params);
	
	void setProfileParameters(TProfileParameters const& parameters) { _profileParameters = parameters; }
	
	TProfileParameters const& profileParameters() const { return _profileParameters; }
	
	/** Ask for a profile parameter. 
	 *	Return true if the parameter is found and fill 'value' with parameter value.
	 */
	bool getProfileParameter(std::string const& paramName, std::string& value) const;
	
	/** Ask for a profile parameter. 
	 *	Return true if the parameter is found and fill 'value' with parameter value.
	 */
	bool getProfileParameter(std::string const& paramName, float& value) const;
	
	/** Ask for a profile parameter. 
	 *	Return true if the parameter is found.
	 */
	bool checkProfileParameter(std::string const& paramName) const;
	
	void addProfileParameter(std::string name, std::string valueStr, float valueFloat);
	
	void removeProfileParameter(std::string name);
	
	void mergeProfileParameter(const TProfileParameter &parameter);
	
	void mergeProfileParameters(const TProfileParameters &parameters);
	
	void parseParameters(std::vector<std::string> const& params, TProfileParameters& parsed);
	TProfileParameters _profileParameters;
};

//////////////////////////////////////////////////////////////////////////////
// CProfileInState                                                          //
//////////////////////////////////////////////////////////////////////////////

class CProfileInState
: public CProfileParameters
#ifdef NL_DEBUG
, public NLMISC::IDbgPtrData
#endif
{
public:
#ifdef NL_DEBUG
	CProfileInState()
	{
		_MoveProfile.setData( this );
		_ActivityProfile.setData( this );
	}
#endif
	void setMoveProfile(IAIProfileFactory* profile) { _MoveProfile = profile; }
	IAIProfileFactory* moveProfile() const { return _MoveProfile; }
	void setActivityProfile(IAIProfileFactory* profile) { _ActivityProfile = profile; }
	IAIProfileFactory* activityProfile() const { return _ActivityProfile; }
	
private:
	NLMISC::CDbgPtr<IAIProfileFactory> _MoveProfile;
	NLMISC::CDbgPtr<IAIProfileFactory> _ActivityProfile;
};

/****************************************************************************/
/* Inlined methods                                                          */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CProfileParameters                                                       //
//////////////////////////////////////////////////////////////////////////////

inline
void CProfileParameters::setProfileParameters(std::vector<std::string> const& params)
{
	parseParameters(params, _profileParameters);
}

inline
bool CProfileParameters::getProfileParameter(std::string const& paramName, std::string& value) const
{
	TProfileParameters::const_iterator it(std::find_if(_profileParameters.begin(), _profileParameters.end(), TFindParameter(paramName)));
	if (it != _profileParameters.end())
	{
		value = it->ValueStr;
		return true;
	}
	else
		return false;
}

inline
bool CProfileParameters::getProfileParameter(std::string const& paramName, float& value) const
{
	TProfileParameters::const_iterator it(std::find_if(_profileParameters.begin(), _profileParameters.end(), TFindParameter(paramName)));
	if (it == _profileParameters.end())
		return	false;
	
	value = it->ValueFloat;
	return true;
}

inline
bool CProfileParameters::checkProfileParameter(std::string const& paramName) const
{
	return std::find_if(_profileParameters.begin(), _profileParameters.end(), TFindParameter(paramName)) != _profileParameters.end();
}

inline
void CProfileParameters::addProfileParameter(std::string name, std::string valueStr, float valueFloat)
{
	mergeProfileParameter(TProfileParameter(name, valueStr, valueFloat));
}

inline
void CProfileParameters::removeProfileParameter(std::string name)
{
	TProfileParameters::iterator it(std::find_if(_profileParameters.begin(), _profileParameters.end(), TFindParameter(name)));
	if (it != _profileParameters.end())
		_profileParameters.erase(it);
}

inline
void CProfileParameters::mergeProfileParameter(const TProfileParameter &parameter)
{
	// remove duplicate
	TProfileParameters::iterator it = std::find_if(_profileParameters.begin(), _profileParameters.end(), TFindParameter(parameter.Name));
	if (it != _profileParameters.end())
		_profileParameters.erase(it);
	// merge value
	_profileParameters.push_back(parameter);
}

inline
void CProfileParameters::mergeProfileParameters(const TProfileParameters &parameters)
{
	for (uint i=0; i<parameters.size(); ++i)
	{
		// remove duplicate
		_profileParameters.erase(std::remove_if(_profileParameters.begin(), _profileParameters.end(), TFindParameter(parameters[i].Name)), _profileParameters.end());
	}
	// merge value
	_profileParameters.insert(_profileParameters.end(), parameters.begin(), parameters.end());
}

inline
void CProfileParameters::parseParameters(std::vector<std::string> const& params, TProfileParameters& parsed)
{
	for (uint i=0; i<params.size(); ++i)
	{
		std::string key, tail;
		if (AI_SHARE::stringToKeywordAndTail(params[i], key, tail))
		{
			parsed.push_back(TProfileParameter(key, tail, float(atof(tail.c_str()))));
			continue;
		}
		
		if (AI_SHARE::stringToWordAndTail(params[i], key, tail))
		{
			parsed.push_back(TProfileParameter(key, "", 0.0f));
			continue;
		}
	}
}

#endif // PROFILE_OWNER_H
