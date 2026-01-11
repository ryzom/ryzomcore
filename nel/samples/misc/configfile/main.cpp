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

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>

// look at the debug example
#include "nel/misc/debug.h"

// contains the config class
#include "nel/misc/config_file.h"

// contains the cpath class
#include "nel/misc/path.h"

#ifndef NL_SAMPLE_CFG
#define NL_SAMPLE_CFG "."
#endif // NL_SAMPLE_CFG

using namespace std;
using namespace NLMISC;

// this function will be called when the configfile will be modified by
// an external program
void mainCallback ()
{
	nlinfo ("The file was modified by external program!");
}

// this function will be called when the variable var12 will be modified by
// an external program
void var12Callback (CConfigFile::CVar &var)
{
	stringstream str;
	
	for (uint i = 0; i < var.size (); i++)
		str << var.asInt (i) << " ";

	nlinfo("%s modified, new value: %s\n", var.Name.c_str (), str.str().c_str());
}


// the configfile
CConfigFile cf;

int main (int /* argc */, char ** /* argv */)
{

	// look at the debug example
	createDebug();

	// Add the data directory to the search path.
	CPath::addSearchPath(NL_SAMPLE_CFG);

	// load and parse the configfile
	cf.load (CPath::lookup("simpletest.txt"));

	// CConfigFile checks if the config file has been modified by an external program.
	// in this case, the configfile automatically reloads and reparses the file.
	// you can connect a callback function that will be called in this case if
	// you want, for example, reset variables
	
	// link a callback with this configfile.
	cf.setCallback (mainCallback);

	// link a callback with the var12. If and only if the var12 changed, this
	// callback will be called
	cf.setCallback ("var12", var12Callback);

	// display all variables
	cf.display(InfoLog);

	// get the value of var1 as int
	int var1 = cf.getVar ("var1").asInt();
	nlinfo("var1 (int) = %d", var1);

	// get the value of var1 as double, in this case, a conversion from int
	// to double will be done
	double var2 = cf.getVar ("var1").asDouble();
	nlinfo("var1 (double) = %lf", var2);

	// get the value of var2 as int, in this case, a conversion from string
	// to int will be done
	int var3 = cf.getVar ("var2").asInt();
	nlinfo("var2 = %d", var3);

	// if the variable is an array of values, you can access them putting the
	// index of the variable you want. Example, get and print all value of var12:
	for (uint i = 0; i < cf.getVar ("var12").size(); i++)
	{
		int val = cf.getVar ("var12").asInt(i);
		nlinfo("%d -> %d", i, val);
	}

	// if you try to access an unknown variable or if something goes wrong, an
	// exception will be called, you can catch them individually or all together in a try/catch block

	try
	{
		// try to access an unknown variable
		int val = cf.getVar ("unknown_variable").asInt();
		nlinfo("unknown_variable = %d", val);
	}
	catch (const EConfigFile &e)
	{
		nlinfo("something goes wrong with configfile: %s", e.what());
	}

	cf.getVar ("var13").asInt (0);
	cf.getVar ("var13").asInt (1);
	cf.getVar ("var13").asInt (2);
	
	nlinfo("Try to modify the var12 in the configfile or any other variable.\n\nPress CTRL-C to exit\n");

	while(true)
	{
		CConfigFile::checkConfigFiles();
	}
	return EXIT_SUCCESS;
}
