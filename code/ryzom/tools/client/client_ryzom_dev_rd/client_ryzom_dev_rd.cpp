/** \file client_ryzom_dev_rd.cpp
 *
 * This app is just a launcher for 'client_ryzom_rd' - Written by Sadge to allow us to
 * patch alpha tester clients that are currently running dev exes with non-dev exes
 *
 */

#include <windows.h>
#include <process.h>

int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// calculate length of file name (with path)
	int i=0;
	while(i<lpCmdLine[i]!=0)
		++i;

	// copy out the string...
	LPTSTR cmdLine= new char[i+1];

	bool quote= false;
	int numArgs=0;
	for (int j=0;j<=i;++j)
	{
		cmdLine[j]= lpCmdLine[j];
		if (cmdLine[j]!=' ' && (j==0 || cmdLine[j-1]==' ') && !quote)
			++numArgs;
		if (cmdLine[j]=='\"') quote= !quote;
	}

	// generate an array of pointers to hold the argument vector
	char** argv= new char*[numArgs+2];
	argv[0]= "client_ryzom_rd.exe";
	argv[numArgs+1]= NULL;

	// build the argument vector
	int k=1;
	for (int jj=0;jj<=i;++jj)
	{
		if (cmdLine[jj]!=' ' && (jj==0 || cmdLine[jj-1]==0) && !quote)
		{
			argv[k]= cmdLine+jj+(cmdLine[jj]=='\"'?1:0);
			++k;
		}
		if (cmdLine[jj]=='\"') quote= !quote;
		if ( (cmdLine[jj]==' ' && !quote) || (cmdLine[jj]=='\"' && quote) )
		{
			cmdLine[jj]=0;
		}
	}

	// execute the client_ryzom_rd executable and terminate
	_execv(argv[0],argv);
	return 0;
}
