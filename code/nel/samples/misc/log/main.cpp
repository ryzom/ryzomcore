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

// contains the logger
#include "nel/misc/log.h"
#include "nel/misc/string_common.h"

// contains standard displayers
#include "nel/misc/displayer.h"

using namespace NLMISC;

// create a displayer that displays on stdout and, on Windows, if the
// application is in debug mode and launched with the debugger (F5), it displays
// on the output windows of Visual C++.
CStdDisplayer sd;

// create a displayer that displays in a file. The first parameter is the filename
// and the second one says that we want to clear the file before logging.
// If you put false (default value), then, new logs are append to the file.
CFileDisplayer fd("main.log",true);

// create a displayer that displays in a message box. It actually works only on
// Windows, it does nothing on other systems.
CMsgBoxDisplayer mbd;

int main (int /* argc */, char ** /* argv */)
{
	// create a logger; it's an information logger.
	CLog logger (CLog::LOG_INFO);

	// the process name is used when we have more than one process logging stuffs
	// in the same displayer (for example for centralized displayer)
	logger.setProcessName ("my_process_name");

	// add displayers for the logger
	logger.addDisplayer (&sd);
	logger.addDisplayer (&fd);
	logger.addDisplayer (&mbd);

	// so, now, if we send something on the logger, it will be displayed on the 3 displayers
	// (stdout, file and message box)

	// display the string with decoration.
	// The decoration is the text that displayers add before the string.
	// They could add the date, the process name and so on.
	// Before each display/displayNL, you have to set the position of where the display
	// occurs. If you don't do that, the position will not be displayed on the displayers.
	logger.setPosition (__LINE__, __FILE__);
	logger.display ("display using display() %d\n", 1);

	// display the string with decoration and a new line at the end of the string.
	logger.setPosition (__LINE__, __FILE__);
	logger.displayNL ("display using displayNL() %d", 2);

	// display the string without decoration.
	logger.displayRaw ("display using displayRaw() %d\n", 3);

	// display the string without decoration and with a new line at the end of the string.
	logger.displayRawNL ("display using displayRawNL() %d", 4);

	printf("\nPress <return> to exit\n");
	getchar();

	return EXIT_SUCCESS;
}
