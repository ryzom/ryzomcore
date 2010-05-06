// message_box.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
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
