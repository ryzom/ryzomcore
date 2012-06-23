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

#include <windows.h>
#include <string>
#include <stdio.h>

using namespace std;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	char *filename;
	if (filename = strstr (lpCmdLine, "-f "))
	{
		filename += 3;
		FILE *file = fopen (filename, "r");
		if (file)
		{
			string content;
			char buffer[512];
			while (fgets (buffer, sizeof(buffer), file))
				content += buffer;
			fclose (file);
			MessageBox (NULL, content.c_str (), "message_box", MB_OK);
		}
	}
	else
		MessageBox (NULL, lpCmdLine, "message_box", MB_OK);

	return 0;
}
