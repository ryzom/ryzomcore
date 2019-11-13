// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_VARIABLE_H
#define NL_VARIABLE_H

#include "types_nl.h"
#include "command.h"
#include "value_smoother.h"
#include "sstring.h"


namespace NLMISC {

/** WARNING:
 *   This is POSIX C/C++ linker behavior: object files
 *   that are not referenced from outside are discarded. The
 *   file in which you run your constructor is thus simply
 *   thrown away by the linker, which explains why the constructor
 *   is not run.
 */


/**
 * Add a variable that can be modified in realtime. The variable must be global. If you must access the variable with
 * function, use NLMISC_DYNVARIABLE
 *
 * Example:
 * \code
	// I want to look and change the variable 'foobar' in realtime, so, first i create it:
	uint8 foobar;
	// and then, I add it
	NLMISC_VARIABLE(uint8, FooBar, "this is a dummy variable");
 * \endcode
 *
 * Please use the same casing than for the variable (first letter of each word in upper case)
 * ie: MyVariable, NetSpeedLoop, Time
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
#define NLMISC_VARIABLE(__type,__var,__help) NLMISC_CATEGORISED_VARIABLE(variables,__type,__var,__help)
#define NLMISC_CATEGORISED_VARIABLE(__category,__type,__var,__help) \
NLMISC::CVariablePtr<__type> __var##Instance(#__category,#__var, __help " (" #__type ")", &__var)



/**
 * Add a variable that can be modified in realtime. The code profide the way to access to the variable in the read
 * and write access (depending on the \c get boolean value)
 *
 * Example:
 * \code
	// a function to read the variable
	uint8 getVar() { return ...; }

	// a function to write the variable
	void setVar(uint8 val) { ...=val; }

	// I want to look and change the variable in realtime:
	NLMISC_DYNVARIABLE(uint8, FooBar, "this is a dummy variable")
	{
		// read or write the variable
		if (get)
			*pointer = getVar();
		else
			setVar(*pointer);
	}
 * \endcode
 *
 * Please use the same casing than for the variable (first letter of each word in upper case)
 * ie: MyVariable, NetSpeedLoop, Time
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
#define NLMISC_DYNVARIABLE(__type,__name,__help) NLMISC_CATEGORISED_DYNVARIABLE(variables,__type,__name,__help)
#define NLMISC_CATEGORISED_DYNVARIABLE(__category,__type,__name,__help) \
class __name##Class : public NLMISC::IVariable \
{ \
public: \
	__name##Class () : IVariable(#__category, #__name, __help) { } \
	 \
	virtual bool fromString(const std::string &val, bool human=false) \
	{ \
		/*std::stringstream ss (val);*/ \
		__type p; \
		/*ss >> p;*/ \
		bool ret = NLMISC::fromString(val, p) ; \
		ptr (&p, false, human); \
		return ret; \
	} \
	 \
	virtual std::string toString(bool human) const \
	{ \
		__type p; \
		ptr (&p, true, human); \
		/*std::stringstream ss;*/ \
		/*ss << p;*/ \
		/*return ss.str();*/ \
		return NLMISC::toString(p); \
	} \
	\
	void ptr(__type *pointer, bool get, bool human) const; \
}; \
__name##Class __name##Instance; \
void __name##Class::ptr(__type *pointer, bool get, bool human) const

/** Helper to declare a variable as friend of a class.
 *	Useful when you want to declare variable that need to access private data to act on or display internal state of the class
 */

#define NLMISC_DYNVARIABLE_FRIEND(__name) NLMISC_CATEGORISED_DYNVARIABLE_FRIEND(variables, __name)
#define NLMISC_CATEGORISED_DYNVARIABLE_FRIEND(__category, __name) friend class __name##Class

//
//
//
//

class IVariable : public ICommand
{
	friend class CCommandRegistry;
public:

	IVariable(const char *categoryName, const char *commandName, const char *commandHelp, const char *commandArgs = "[<value>]", bool useConfigFile = false, void (*cc)(IVariable &var)=NULL) :
		ICommand(categoryName,commandName, commandHelp, commandArgs), _UseConfigFile(useConfigFile), ChangeCallback(cc)
	{
		Type = Variable;
	}

	virtual bool fromString(const std::string &val, bool human=false) = 0;

	virtual std::string toString(bool human=false) const = 0;

	virtual bool execute(const std::string &/* rawCommandString */, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human)
	{
		if (args.size() > 1)
			return false;

		if (args.size() == 1)
		{
			// set the value
			fromString (args[0], human);
		}

		// display the value
		if (quiet)
		{
			log.displayNL(toString(human).c_str());
		}
		else
		{
			log.displayNL("Variable %s = %s", _CommandName.c_str(), toString(human).c_str());
		}
		return true;
	}

	static void init (CConfigFile &configFile);

private:

	bool _UseConfigFile;

protected:

	// TODO: replace by interface (see IVariableChangedCallback)
	void (*ChangeCallback)(IVariable &var);

};




template <class T>
class CVariablePtr : public IVariable
{
public:

	CVariablePtr (const char *categoryName, const char *commandName, const char *commandHelp, T *valueptr, bool useConfigFile = false, void (*cc)(IVariable &var)=NULL) :
		IVariable (categoryName, commandName, commandHelp, "[<value>]", useConfigFile, cc), _ValuePtr(valueptr)
	{
	}

	virtual bool fromString (const std::string &val, bool /* human */=false)
	{
		//std::stringstream ss (val);
		//ss >> *_ValuePtr;
		bool ret = NLMISC::fromString(val, *_ValuePtr);
		if (ChangeCallback) ChangeCallback (*this);
		return ret;
	}

	virtual std::string toString (bool /* human */) const
	{
		//std::stringstream ss;
		//ss << *_ValuePtr;
		//return ss.str();
		return NLMISC::toString(*_ValuePtr);
	}

private:

	T *_ValuePtr;
};


template <class T>
class CVariable : public IVariable
	{
public:

	CVariable (	const char *categoryName,
				const char *commandName,
				const char *commandHelp,
				const T &defaultValue,
				uint nbMeanValue = 0,
				bool useConfigFile = false,
				void (*cc)(IVariable &var)=NULL,
				bool executeCallbackForDefaultValue=false ) :
		IVariable (categoryName, commandName, commandHelp, "[<value>|stat|mean|min|max]", useConfigFile, cc), _Mean(nbMeanValue), _First(true)
	{
		set (defaultValue, executeCallbackForDefaultValue);
	}

	virtual bool fromString (const std::string &val, bool /* human */=false)
	{
		T v;
		bool ret = NLMISC::fromString(val, v);
		set (v);
		return ret;
	}

	virtual std::string toString (bool /* human */) const
	{
		return NLMISC::toString(_Value);
	}

	CVariable<T> &operator= (const T &val)
	{
		set (val);
		return *this;
	}

	operator T () const
	{
		return get ();
	}

	void set (const T &val, bool executeCallback = true)
	{
		_Value = val;
		_Mean.addValue (_Value);
		if (_First)
		{
			_First = false;
			_Min = _Value;
			_Max = _Value;
		}
		else
		{
			if (_Value > _Max) _Max = _Value;
			if (_Value < _Min) _Min = _Value;
		}
		if (ChangeCallback && executeCallback) ChangeCallback (*this);
	}

	const T &get () const
	{
		return _Value;
	}

	std::string getStat (bool light = false) const
	{
		CSString str;
		str << _CommandName << "=" << _Value << " Min=" << _Min;
		if (_Mean.getNumFrame()>0)
		{
			const std::vector<T>& v = _Mean.getLastFrames();
			T theMin = *std::min_element(v.begin(), v.end());
			str << " RecentMin=" << theMin;
		}
		str << " Max=" << _Max;
		if (_Mean.getNumFrame()>0)
		{
			const std::vector<T>& v = _Mean.getLastFrames();
			T theMax = *std::max_element(v.begin(), v.end());
			str << " RecentMax=" << theMax;
		}
		if (_Mean.getNumFrame()>0)
		{
			str << " RecentMean=" << _Mean.getSmoothValue();
			if(!light)
			{
				str << " RecentValues=";
				// output the oldest part of the buffer first
				for (uint i=_Mean.getCurrentFrame(); i<_Mean.getNumFrame(); ++i)
				{
					str << (T)_Mean.getLastFrames()[i];
					if (i < _Mean.getNumFrame()-1 || _Mean.getCurrentFrame() != 0)
						str << ",";
				}
				// then output the newest part
				for (uint i = 0; i < _Mean.getCurrentFrame(); i++)
				{
					str << (T)_Mean.getLastFrames()[i];
					if (i < _Mean.getCurrentFrame()-1)
						str << ",";
				}
			}
		}
		return str;
	}

	virtual bool execute (const std::string &/* rawCommandString */, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human)
	{
		if (args.size() > 1)
			return false;

		bool haveVal=false;
		std::string val;

		if (args.size() == 1)
		{
			if (args[0] == "stat")
			{
				// display the stat value
				std::string stat = getStat();
				// cut the stat line in lines of 80 chars
				std::string::size_type pos = 0;
				while (pos < stat.size())
				{
					log.displayNL(getStat().substr(pos, 80).c_str());
					pos += 80;
				}
				return true;
			}
			else if (args[0] == "mean")
			{
				haveVal = true;
				val = NLMISC::toString(_Mean.getSmoothValue());
			}
			else if (args[0] == "min")
			{
				haveVal = true;
				val = NLMISC::toString(_Min);
			}
			else if (args[0] == "max")
			{
				haveVal = true;
				val = NLMISC::toString(_Max);
			}
			else
			{
				// set the value
				fromString (args[0], human);
			}
		}

		// display the value
		if (!haveVal)
		{
			val = toString(human);
		}

		if (quiet)
		{
			log.displayNL(val.c_str());
		}
		else
		{
			log.displayNL("Variable %s = %s", _CommandName.c_str(), val.c_str());
		}
		return true;
	}

private:

	T _Value;
	CValueSmootherTemplate<T> _Mean;
	T _Min, _Max;
	bool _First;
};

template<> class CVariable<std::string> : public IVariable
{
public:

	CVariable (const char *categoryName, const char *commandName, const char *commandHelp, const std::string &defaultValue, uint /* nbMeanValue */ = 0, bool useConfigFile = false, void (*cc)(IVariable &/* var */)=NULL, bool executeCallbackForDefaultValue=false) :
		IVariable (categoryName, commandName, commandHelp, "[<value>]", useConfigFile, cc)
	{
		set (defaultValue, executeCallbackForDefaultValue);
	}

	virtual bool fromString (const std::string &val, bool /* human */=false)
	{
		set (val);
		return true;
	}

	virtual std::string toString (bool /* human */=false) const
	{
		return _Value;
	}

	CVariable<std::string> &operator= (const std::string &val)
	{
		set (val);
		return *this;
	}

	operator std::string () const
	{
		return get();
	}

	operator const char * () const
	{
		return get().c_str();
	}

	const char *c_str () const
	{
		return get().c_str();
	}

	void set (const std::string &val, bool executeCallback = true)
	{
		_Value = val;
		static bool RecurseSet = false;
		if (ChangeCallback && !RecurseSet && executeCallback)
		{
			RecurseSet = true;
			ChangeCallback(*this);
			RecurseSet = false;
		}
	}

	const std::string &get () const
	{
		return _Value;
	}

	virtual bool execute (const std::string &/* rawCommandString */, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human)
	{
		if (args.size () > 1)
			return false;

		if (args.size () == 1)
		{
			// set the value
			fromString (args[0], human);
		}

		// convert the string from utf-8 to ascii (thrue unicode)
		ucstring temp;
		temp.fromUtf8(toString(human));
		std::string disp = temp.toString();
		// display the value
		if (quiet)
		{
			log.displayNL (disp.c_str());
		}
		else
		{
			log.displayNL ("Variable %s = %s", _CommandName.c_str(), disp.c_str());
		}
		return true;
	}

private:

	std::string _Value;
};


/// This class can provide a callback called when the value of a variable has been changed
class IVariableChangedCallback
{
public:
	virtual ~IVariableChangedCallback() {}
	virtual void onVariableChanged(NLMISC::IVariable& var) = 0;
};


} // NLMISC


#endif // NL_VARIABLE_H

/* End of variable.h */
