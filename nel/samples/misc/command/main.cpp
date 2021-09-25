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

#include <iostream>
#include <sstream>

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/log.h"
#include "nel/misc/displayer.h"

// include the command system
#include "nel/misc/command.h"
#include "nel/misc/variable.h"


//
// Commands
//

// with this macro, you can execute a specific command in real time.
// this is an example of a command, the first param is the name of the command
// (note that it is *not* a string). the second param is the help string that will
// be displayed when the user calls the help command. the third param is the
// usage of the command. it can the an empty string when the command doesn't
// need any parameters.
// when the program is launched, you can type the name of the command with
// parameters if needed and it will be executed.
NLMISC_COMMAND(square,"display the square of the parameter","<value>")
{
	// this code will be executed when the user will call this command.
	// you have access to some variables:
	// - args is a vector of strings. they are the user parameters, you have to
	//   check if enough parameters are present and if they are in a good
	//   format.
	// - log is a CLog object that is used to display the result of the command,
	//   if any. see the log example to know how to use CLog.

	// this function must return true if the command successfully executed
	// or false is something wrong happens. for example, if there are not enough
	// parameters, return false. 

	// check args, if there is not the right number of parameters, return bad status.
	if (args.size() != 1) return false;

	uint32 val;
	NLMISC::fromString(args[0], val);

	// display the result.
	log.displayNL("The square of %d is %d", val, val*val);

	// return ok status.
	return true;
}

//
// Dummy variables
//

bool MyVar1;
uint8 MyVar2;
sint8 MyVar3;
uint16 MyVar4;
sint16 MyVar5;
uint32 MyVar6;
sint32 MyVar7;
uint64 MyVar8;
sint64 MyVar9;
std::string MyVar10;
ucchar MyVar13;
float MyVar14;
double MyVar15;

//
// Create commands on all variables
//

// with this macro, you can get or set the variable in real time.
// this is the easiest way to set or get a variable in real time. you have a direct
// access to the variable so you can use this macro.
// first param is the type of the variable. second one is the variable name,
// and the third one is an help string that describes what the variable does.
// when the program is executed, you can type the variable name with no parameter
// to see the value of the variable or type the variable name with one parameter
// (the value you want to set) to change the variable contents.
NLMISC_VARIABLE(bool, MyVar1, "a dummy variable");
NLMISC_VARIABLE(uint8, MyVar2, "a dummy variable");
NLMISC_VARIABLE(sint8, MyVar3, "a dummy variable");
NLMISC_VARIABLE(uint16, MyVar4, "a dummy variable");
NLMISC_VARIABLE(sint16, MyVar5, "a dummy variable");
NLMISC_VARIABLE(uint32, MyVar6, "a dummy variable");
NLMISC_VARIABLE(sint32, MyVar7, "a dummy variable");
NLMISC_VARIABLE(uint64, MyVar8, "a dummy variable");
NLMISC_VARIABLE(sint64, MyVar9, "a dummy variable");
NLMISC_VARIABLE(std::string, MyVar10, "a dummy variable");
NLMISC_VARIABLE(ucchar, MyVar13, "a dummy variable");
NLMISC_VARIABLE(float, MyVar14, "a dummy variable");
NLMISC_VARIABLE(double, MyVar15, "a dummy variable");

//
// DynVariable
//

class CDynVar
{
private:
	int _PrivateVar;
public:
	int get() { return _PrivateVar; }
	void set(int val) { _PrivateVar = val; }
};

CDynVar dv;

// with this macro, you can get or set the variable in real time.
// this is an example of dynamic variable. the goal of this macro is to provide an
// easy way to get or set a variable when you don't have a direct access on this variable.
// first param is the type of the variable. second one is the name of the variable.
// the third one is the help string about this variable (what this variable does).
// when the program is executed, you can type the variable name with no parameter
// to see the value of the variable or type the variable name with one parameter
// (the value you want to set) to change the variable contents.
NLMISC_DYNVARIABLE(int,PrivVar,"dummy variable")
{
	// this code will be executed to get or set the variable.
	// you have to provide a way to do that.
	// the 'get' boolean says if you have to get or set the variable.
	// the 'pointer' is a pointer on the variable you have to read or write.
	// if 'get' is true, you have to set '*pointer' with the value of your variable.
	// if 'get' is false, you have to set your variable with the value of *pointer
	if (get)
		*pointer = dv.get();
	else
		dv.set(*pointer);
}


//
// Main
//

// look at the log sample
NLMISC::CLog mylog;
// look at the log sample
NLMISC::CStdDisplayer mysd;

int main (int argc, char **argv)
{
	// look at the debug sample
	NLMISC::createDebug();

	// look at the log sample
	mylog.addDisplayer(&mysd);

	printf("Type a command, 'help' or 'quit'\n");

	while (true)
	{
		// display prompt
		std::cout << "> ";
		fflush(stdout);

		// get command line
		std::string commandLine;
		std::getline(std::cin, commandLine, '\n');

		if (commandLine == "quit")
			break;

		// execute the command line. it will try to find the command or the
		// variable and call the associated code
		NLMISC::ICommand::execute(commandLine, mylog);
	}
	
	return EXIT_SUCCESS;
}
