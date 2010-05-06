#ifdef __GNUC__
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
	read(STDIN_FILENO,&ch,1);
	return ch;
}

#endif // __GNUC__
