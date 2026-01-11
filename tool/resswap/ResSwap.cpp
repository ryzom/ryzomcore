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

// ResSwap.cpp : Defines the entry point for the application.
//

#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	DEVMODE		bkupMode, devMode;
	devMode.dmSize= sizeof(DEVMODE);
	devMode.dmDriverExtra= 0;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode);
	devMode.dmFields= DM_BITSPERPEL;

	bkupMode= devMode;

	if(devMode.dmBitsPerPel==16)
	{
		devMode.dmBitsPerPel=32;
	}
	else
	{
		devMode.dmBitsPerPel=16;
	}
	ChangeDisplaySettings(&devMode, 0);
	ChangeDisplaySettings(&bkupMode, 0);

	return 0;
}



