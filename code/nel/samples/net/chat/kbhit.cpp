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

#include "nel/misc/types_nl.h"

#ifndef NL_OS_WINDOWS
#include "kbhit.h"
#include <termios.h>
#include <unistd.h>   // for read()
#include <stdio.h>

static struct termios initial_settings, new_settings;
static int peek_character = -1;

void init_keyboard()
{
	tcgetattr(STDIN_FILENO,&initial_settings);
	new_settings = initial_settings;
	new_settings.c_lflag &= ~ICANON;
	new_settings.c_lflag &= ~ECHO;
	new_settings.c_lflag &= ~ISIG;
	new_settings.c_cc[VMIN] = 1;
	new_settings.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
}

void close_keyboard()
{
	tcsetattr(STDIN_FILENO, TCSANOW, &initial_settings);
}

int kbhit()
{
	unsigned char ch;
	int nread;

	if (peek_character != -1) return 1;

	new_settings.c_cc[VMIN]=0;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
	nread = read(STDIN_FILENO,&ch,1);
	new_settings.c_cc[VMIN]=1;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

	if(nread == 1)
	{
		peek_character = ch;
		return 1;
	}
	return 0;
}

int getch()
{
	char ch;

	if(peek_character != -1)
	{
		ch = peek_character;
		peek_character = -1;
		return ch;
	}
	if (read(STDIN_FILENO,&ch,1) != 1) return ' ';

	return ch;
}

#endif // __GNUC__
