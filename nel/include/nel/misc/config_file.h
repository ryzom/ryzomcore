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

#ifndef NL_CONFIG_FILE_H
#define NL_CONFIG_FILE_H

#include "types_nl.h"
#include "common.h"
#include "debug.h"
#include "log.h"

#include <vector>
#include <string>
#include <cstdio>

namespace NLMISC
{

/**
 * CConfigFile class. Useful when you want to have a configuration file with variables.
 * It manages integers, real (double), and string basic types. A variable can be an array of
 * basic type. In this case, all elements of the array must have the same type.
 *
 * If you setup the global callback before loading, it'll be called after the load() function.
 *
 * Example:
 *\code
 * try
 * {
 * 	CConfigFile cf;
 *
 * 	// Load and parse "test.txt" file
 *  cf.load ("test.txt");
 *
 *	// Attach a callback to the var1 variable. When the var1 will change, this cvar1cb function will be called
 *	cf.setCallback ("var1", var1cb);
 *
 *	// Get the foo variable (suppose it's a string variable)
 *	CConfigFile::CVar &foo = cf.getVar ("foo");
 *
 *	// Display the content of the variable
 *	printf ("foo = %s\n", foo.asString ().c_str ());
 *
 * 	// Get the bar variable (suppose it's an array of int)
 * 	CConfigFile::CVar &bar = cf.getVar ("bar");
 *
 * 	// Display the content of all the elements of the bar variable
 * 	printf ("bar have %d elements : \n", bar.size ());
 * 	for (int i = 0; i < bar.size (); i++)
 * 		printf ("%d ", bar.asInt (i));
 * 	printf("\n");
 * }
 * catch (const EConfigFile &e)
 * {
 *	// Something goes wrong... catch that
 * 	printf ("%s\n", e.what ());
 * }
 *\endcode
 *
 * Example of config file:
 *\code
 * // one line comment
 * / * big comment
 *     on more than one line * /
 *
 * var1 = 123;                           // var1  type:int,         value:123
 * var2 = "456.25";                      // var2  type:string,      value:"456.25"
 * var3 = 123.123;                       // var3  type:real,        value:123.123
 *
 * // the resulting type is type of the first left value
 * var4 = 123.123 + 2;                   // var4  type:real,        value:125.123
 * var5 = 123 + 2.1;                     // var5  type:int,         value:125
 *
 * var6 = (-112+1) * 3 - 14;             // var6  type:int,         value:-347
 *
 * var7 = var1 + 1;                      // var7  type:int,         value:124
 *
 * var8 = var2 + 10;                     // var8  type:string,      value:456.2510 (convert 10 into a string and concat it)
 * var9 = 10.15 + var2;                  // var9  type:real,        value:466.4 (convert var2 into a real and add it)
 *
 * var10 = { 10.0, 51.1 };               // var10 type:realarray,   value:{10.0,51.1}
 * var11 = { "str1", "str2", "str3" };   // var11 type:stringarray, value:{"str1", "str2", "str3"}
 *
 * var12 = { 10+var1, var1-var7 };       // var12 type:intarray,    value:{133,-1}
 *\endcode
 *
 * Operators are '+', '-', '*', '/'.
 * You can't use operators on a array variable, for example, you can't do \cvar13=var12+1.
 * If you have 2 variables with the same name, the first value will be remplaced by the second one.
 *
 * \bug if you terminate the config file with a comment without carriage returns it'll generate an exception, add a carriage returns
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CConfigFile
{
public:

	/**
	 * CVar class. Used by CConfigFile. A CVar is returned when you want to have a variable.
	 *
	 * Example: see the CConfigFile example
	 *
	 * \author Vianney Lecroart
	 * \author Nevrax France
	 * \date 2000
	 */
	struct CVar
	{
	public:

		CVar () : Type(T_UNKNOWN), Root(false), Comp(false), FromLocalFile(true), SaveWrap(6) {}

		/// \name Access to the variable content.
		//@{
		/// Get the content of the variable as an integer
		int					asInt		(int index=0) const;
		/// Get	the content of the variable as a double
		double				asDouble	(int index=0) const;
		/// Get the content of the variable as a float
		float				asFloat		(int index=0) const;
		/// Get the content of the variable as a STL string
		std::string			asString	(int index=0) const;
		/// Get the content of the variable as a boolean
		bool				asBool		(int index=0) const;
		//@}

		/// \name Set the variable content.
		/// If the index is the size of the array, the value will be append at the end.
		//@{
		/// Set the content of the variable as an integer
		void				setAsInt	(int val, int index=0);
		/// Set	the content of the variable as a double
		void				setAsDouble	(double val, int index=0);
		/// Set the content of the variable as a float
		void				setAsFloat	(float val, int index=0);
		/// Set the content of the variable as a STL string
		void				setAsString	(const std::string &val, int index=0);

		/// Force the content of the variable to be a single integer
		void				forceAsInt	(int val);
		/// Force the content of the variable to be a single double
		void				forceAsDouble	(double val);
		/// Force the content of the variable to be a single string
		void				forceAsString	(const std::string &val);

		/// Set the content of the aray variable as an integer
		void				setAsInt	(const std::vector<int> &vals);
		/// Set the content of the aray variable as a double
		void				setAsDouble	(const std::vector<double> &vals);
		/// Set the content of the aray variable as a float
		void				setAsFloat	(const std::vector<float> &vals);
		/// Set the content of the aray variable as a string
		void				setAsString	(const std::vector<std::string> &vals);

		//@}

		bool		operator==	(const CVar& var) const;
		bool		operator!=	(const CVar& var) const;

		// add this variable with var
		void		add (const CVar &var);

		// Get the size of the variable. It's the number of element of the array or 1 if it's not an array.
		uint		size () const;

		/// \name Internal use
		//@{
		static const char *TypeName[];

		enum TVarType { T_UNKNOWN, T_INT, T_STRING, T_REAL, T_BOOL };

		std::string					Name;
		TVarType					Type;
		bool						Root;		// true if this var comes from the root document. false else.
		bool						Comp;		// true if the parser found a 'complex' var (ie an array)
		bool						FromLocalFile;	// Used during cfg parsing. True if the var has been created from the currently parsed cfg
		std::vector<int>			IntValues;
		std::vector<double>			RealValues;
		std::vector<std::string>	StrValues;

		int							SaveWrap;

		void						(*Callback)(CVar &var);
		//@}
	};

	CConfigFile() : _Callback(NULL) {}

	virtual ~CConfigFile ();

	/// Get a variable with the variable name
	CVar &getVar (const std::string &varName);

	/// Get a variable pointer with the variable name, without throwing exception. Return NULL if not found.
	CVar *getVarPtr (const std::string &varName);

	/// Get the variable count.
	uint	getNumVar () const;

	/// Get a variable.
	CVar	*getVar (uint varId);

	/// Add a variable. If the variable already exists, return it.
	CVar	*insertVar (const std::string &varName, const CVar &varToCopy);

	/// Return true if the variable exists, false otherwise
	bool exists (const std::string &varName);

	/// load and parse the file
	void load (const std::string &fileName, bool lookupPaths = false);

	/// save the config file
	void save () const;

	/// Clear all the variable array (including information on variable callback etc)
	void clear ();

	/// set to 0 or "" all variable in the array (but not destroy them)
	void clearVars ();

	/// Returns true if the file has been loaded
	bool loaded();

	/// Returns the number of variables in the configuration
	uint32 getVarCount();

	/// reload and reparse the file
	void reparse (bool lookupPaths = false);

	/// display all variables with nlinfo (debug use)
	void display () const;

	/// display all variables with nlinfo (debug use)
	void display (CLog *log) const;

	/// set a callback function that is called when the config file is modified
	void setCallback (void (*cb)());

	/// set a callback function to a variable, it will be called when this variable is modified
	void setCallback (const std::string &VarName, void (*cb)(CConfigFile::CVar &var));

	/// contains the variable names that getVar() and getVarPtr() tried to access but not present in the cfg
	std::vector<std::string> UnknownVariables;

	/// returns the config file name
	std::string getFilename () const { return FileNames[0]; }

	/// set the time between 2 file checking (default value is 1 second)
	/// \param timeout time in millisecond, if timeout=0, the check will be made each "frame"
	static void setTimeout (uint32 timeout);

	/// Internal use only
	static void checkConfigFiles ();

private:

	/// Internal use only
	void (*_Callback)();

	/// Internal use only
	std::vector<CVar>	_Vars;

	// contains the configfilename (0) and roots configfilenames
//	std::string	_FileName;
//	std::vector<uint32>			_LastModified;

	// contains the configfilename (0) and roots configfilenames
	std::vector<std::string>	FileNames;
	std::vector<uint32>			LastModified;

	static uint32	_Timeout;

	static std::vector<CConfigFile *> *_ConfigFiles;
};

struct EConfigFile : public Exception
{
	EConfigFile() { _Reason = "Unknown Config File Exception";}
};

struct EBadType : public EConfigFile
{
	EBadType (const std::string &varName, int varType, int wantedType)
	{
		static char str[NLMISC::MaxCStringSize];
		smprintf (str, NLMISC::MaxCStringSize, "Bad variable type, variable \"%s\" is a %s and not a %s", varName.c_str (), CConfigFile::CVar::TypeName[varType], CConfigFile::CVar::TypeName[wantedType]);
		_Reason = str;
		nlinfo("CF: Exception will be launched: %s", _Reason.c_str());
	}
};

struct EBadSize : public EConfigFile
{
	EBadSize (const std::string &varName, int varSize, int varIndex)
	{
		static char str[NLMISC::MaxCStringSize];
		smprintf (str, NLMISC::MaxCStringSize, "Trying to access to the index %d but the variable \"%s\" size is %d", varIndex, varName.c_str (), varSize);
		_Reason = str;
		nlinfo("CF: Exception will be launched: %s", _Reason.c_str());
	}
};

struct EUnknownVar : public EConfigFile
{
	EUnknownVar (const std::string &filename, const std::string &varName)
	{
		static char str[NLMISC::MaxCStringSize];
		smprintf (str, NLMISC::MaxCStringSize, "variable \"%s\" not found in file \"%s\"", varName.c_str (), filename.c_str());
		_Reason = str;
		nlinfo("CF: Exception will be launched: %s", _Reason.c_str());
	}
};

struct EParseError : public EConfigFile
{
	EParseError (const std::string &fileName, int currentLine)
	{
		static char str[NLMISC::MaxCStringSize];
		smprintf (str, NLMISC::MaxCStringSize, "Parse error on the \"%s\" file, line %d", fileName.c_str (), currentLine);
		_Reason = str;
		nlinfo("CF: Exception will be launched: %s", _Reason.c_str());
	}
};

struct EFileNotFound : public EConfigFile
{
	EFileNotFound (const std::string &fileName)
	{
		static char str[NLMISC::MaxCStringSize];
		smprintf (str, NLMISC::MaxCStringSize, "File \"%s\" not found", fileName.c_str ());
		_Reason = str;
	}
};

} // NLMISC

#endif // NL_CONFIG_FILE_H

/* End of config_file.h */
