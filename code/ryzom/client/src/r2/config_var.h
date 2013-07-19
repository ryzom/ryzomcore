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

#ifndef R2_CONFIG_VAR_H
#define R2_CONFIG_VAR_H

#include "nel/gui/lua_object.h"
#include "editor.h"

namespace R2
{

// base class for he config file variables
class CConfigVarBase
{
public:
	CConfigVarBase(const char *varName);
protected:
	mutable uint32	_TimeStamp; // If this date is not the same than the config file date, then we know the
							  // value should be updated (so, more a counter than a timestamp, actually)
	std::string	  _VarName;
private:
	// not copyable
	CConfigVarBase(const CConfigVarBase &/* other */) { nlassert(0); }
	CConfigVarBase &operator = (const CConfigVarBase &/* other */) { nlassert(0); return *this; }
public:
	static uint32 &getConfigFileTimeStamp();
};



/** Quick access to variables defined inside r2_config.lua.
  *
  * Variables are updated when the editor is reseted.
  * They are usually created at global scope.
  *
  * Example :
  *
  * CConfigVarRGBA CV_CreatureDefaultColor("CreatureDefaultColor", CRGBA(255, 255, 255, 0))); // r2_config.lua may contains a 'CreatureDefaultColor' config value
  *
  * creature.setColor(CV_CreatureDefaultColor.get());
  *
  */
template <class T>
class CConfigVar : public CConfigVarBase
{
public:
	CConfigVar(const char *varName, const T &defaultValue = T()) : CConfigVarBase(varName), _Default(defaultValue) {}
	CConfigVar(const CConfigVar &other) : CConfigVarBase(other._VarName.c_str()), _Value(other._Value), _Default(other._Default) {}
	const T &get() const;
private:
	mutable T	  _Value;
	const   T     _Default;
};

// forward declarations for specialisations
std::string getConfigVarTypename(const float &dummy);
bool getConfigVarValue(CLuaObject &luaValue, float &dest);

std::string getConfigVarTypename(const double &dummy);
bool getConfigVarValue(CLuaObject &luaValue, double &dest);

std::string getConfigVarTypename(const sint32 &dummy);
bool getConfigVarValue(CLuaObject &luaValue, sint32 &dest);

std::string getConfigVarTypename(const std::string &dummy);
bool getConfigVarValue(CLuaObject &luaValue, std::string &dest);

std::string getConfigVarTypename(const NLMISC::CRGBA &dummy);
bool getConfigVarValue(CLuaObject &luaValue, NLMISC::CRGBA &dest);

template <class T> const T &CConfigVar<T>::get() const
{
	// Relies on R2 'getConfigVarValue' and 'getConfigVarTypename' functions specialization to retrieve the value (see below)
	if (_TimeStamp != CConfigVarBase::getConfigFileTimeStamp())
	{
		const CLuaObject &config = getEditor().getConfig();
		if (config.isNil())
		{
			nlwarning("Trying to access the config file value '%s' of type '%s' from r2_config.lua, but the file hasn't been parsed yet, or its parsing failed, using default value.", _VarName.c_str(), getConfigVarTypename(_Default).c_str());
			_Value = _Default;
		}
		else
		{
			CLuaObject luaValue = config[_VarName];
			if (!getConfigVarValue(luaValue, _Value))
			{
				nlwarning("Can't retrieve value of R2 config var '%s', of type '%s', using default value", _VarName.c_str(), getConfigVarTypename(_Default).c_str());
				_Value = _Default;
			}
		}
		_TimeStamp = CConfigVarBase::getConfigFileTimeStamp();
	}
	return _Value;
};

//////////////////////
// Specialisations  //
//////////////////////

//------------------------------------------------------------------------------------------------
// Float
inline std::string getConfigVarTypename(const float &/* dummy */) { return "float"; }
inline bool getConfigVarValue(CLuaObject &luaValue, float &dest)
{
	if (luaValue.isNumber())
	{
		dest = (float) luaValue.toNumber();
		return true;
	}
	return false;
}
typedef CConfigVar<float> CConfigVarFloat;

//------------------------------------------------------------------------------------------------
// Double
inline std::string getConfigVarTypename(const double &/* dummy */) { return "float"; }
inline bool getConfigVarValue(CLuaObject &luaValue, double &dest)
{
	if (luaValue.isNumber())
	{
		dest = luaValue.toNumber();
		return true;
	}
	return false;
}
typedef CConfigVar<double> CConfigVarDouble;

//------------------------------------------------------------------------------------------------
// sint32
inline std::string getConfigVarTypename(const sint32 &/* dummy */) { return "sint32"; }
inline bool getConfigVarValue(CLuaObject &luaValue, sint32 &dest)
{
	if (luaValue.isNumber())
	{
		dest = (sint32) luaValue.toNumber();
		return true;
	}
	return false;
}
typedef CConfigVar<sint32> CConfigVarSInt32;


//------------------------------------------------------------------------------------------------
// String
inline std::string getConfigVarTypename(const std::string &/* dummy */) { return "string"; }
inline bool getConfigVarValue(CLuaObject &luaValue, std::string &dest)
{
	if (luaValue.isString())
	{
		dest = luaValue.toString();
		return true;
	}
	return false;
}
typedef CConfigVar<std::string> CConfigVarString;

//------------------------------------------------------------------------------------------------
// RGBA
inline std::string getConfigVarTypename(const NLMISC::CRGBA &/* dummy */) { return "rgba"; }
inline bool getConfigVarValue(CLuaObject &luaValue, NLMISC::CRGBA &dest)
{
	if (luaValue.isRGBA())
	{
		dest = luaValue.toRGBA();
		return true;
	}
	return false;
}
typedef CConfigVar<NLMISC::CRGBA> CConfigVarRGBA;

} // R2

#endif
