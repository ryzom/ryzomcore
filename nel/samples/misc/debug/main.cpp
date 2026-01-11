// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// contains all debug features
#include <nel/misc/debug.h>
#include <nel/misc/report.h>

void repeatederror()
{
	// hit always ignore to surpress this error for the duration of the program
	nlassert(false && "hit always ignore");
}

int main(int /* argc */, char ** /* argv */)
{
	// all debug functions have different behaviors in debug and in release mode.
	// in general, in debug mode, all debug functions are active, they display
	// what happens and some break the program to debug it. In release mode, they often
	// do nothing to increase the execution speed.


	// this function initializes debug functions. it adds displayers into the debug
	// logger.
	// in debug mode, all debug functions display on the std output.
	// in release mode, this function does nothing by default. you have to add a displayer
	// manually, or put true in the parameter to say to the function that you want it to
	// add the default displayers
	NLMISC::createDebug();

	// enable the crash report tool
	NLMISC::INelContext::getInstance().setWindowedApplication(true);
	NLMISC::setReportPostUrl("http://ryzomcore.org/crash_report/");

	// display debug information, that will be skipped in release mode.
	nldebug("nldebug() %d", 1);

	// display the string
	nlinfo("nlinfo() %d", 2);

	// when something not normal, but that the program can manage, occurs, call nlwarning()
	nlwarning("nlwarning() %d", 3);

	// nlassert() is like assert but do more powerful things. in release mode, the test is
	// not executed and nothing will happen. (Press F5 in Visual C++ to continue the execution)
	nlassert(true == false);

	// in a switch case or when you want that the program never executes a part of code, use stop.
	// in release, nlstop does nothing. in debug mode,
	// if the code reaches the nlstop, a breakpoint will be set. (In Visual C++ press F5 to continue)
	nlstop;

	// when the program failed, call nlerror(), it displays the message and throws a EFatalError to
	// exit the program. don't forget to put a try/catch block everywhere an nlerror could
	// occurs. (In Visual C++ press F5 to continue)
	try
	{
		nlerror("nlerror() %d", 4);
	}
	catch (const NLMISC::EFatalError &)
	{
		// just continue...
		nlinfo("nlerror() generated an EFatalError exception, just ignore it");
	}

	// keep repeating the same error
	for (int i = 0; i < 32; ++i)
		repeatederror();

	printf("\nPress <return> to exit\n");
	getchar();

	return EXIT_SUCCESS;
}
