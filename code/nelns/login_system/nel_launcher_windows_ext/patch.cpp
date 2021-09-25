// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

//
// Includes
//

#include "stdafx.h"

#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/thread.h"

#include "patch.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;


//
// Variables
//

HINTERNET RootInternet = NULL;

static const string DirFilename = "dir.ngz";
static const string NelLauncherFilename = "nel_launcher.exe";
static const string NelLauncherConfigFilename = "nel_launcher.cfg";

// The file is auto generated by the nel launcher (and deleted) when the .cfg or .exe has been changed in the root directory
static const string UpdateNelLauncherBatchFilename = "updt_nl.bat";

// This file must be in the patch server, will be execute after killing nel_launcher in the root directory
static const string RelaunchNelLauncherBatchFilename = "rlnch_nl.bat";

// This file must be in the patch server, will be execute after the patching and waiting the end of the .bat to continue to execute the launcher in the root directory
static const string FinalizeNelLauncherBatchFilename = "fnlz_ptc.bat";

//
//
//

struct CEntry
{
	string Filename;
	uint32 Size;
	uint32 Date;
	CEntry(const string &fn, uint32 s, uint32 d) : Filename(fn), Size(s), Date(d) { }
};


//
// Functions
//

void setRWAccess (const string &filename)
{
	if (!NLMISC::CFile::setRWAccess(filename))
	{
		nlwarning ("Can't have read/write access to '%s' file : code=%d %s", filename.c_str(), errno, strerror(errno));
		throw Exception ("Can't have read/write access to '%s' file : code=%d %s", filename.c_str(), errno, strerror(errno));
	}
}

string deleteFile (const string &filename, bool throwException=true)
{
	if (!NLMISC::CFile::deleteFile(filename))
	{
		string str = toString("Can't delete '%s' file : code=%d %s", filename.c_str(), errno, strerror(errno));
		nlwarning (str.c_str());
		if(throwException)
			throw Exception (str);
		return str;
	}
	return "";
}

void setVersion(const std::string &version)
{
	string fn = "VERSION";
	
	setRWAccess(fn);
	FILE *fp = fopen (fn.c_str(), "wb");
	if (fp == NULL)
	{
		throw Exception ("Can't open file '%s' : code=%d %s", fn.c_str (), errno, strerror(errno));
	}

	if (fputs (version.c_str (), fp) == EOF)
	{
		throw Exception ("Can't write file '%s' : code=%d %s", fn.c_str (), errno, strerror(errno));
	}
	fclose (fp);
}

string getVersion()
{
	string fn = "VERSION";
	FILE *fp = fopen (fn.c_str (), "rb");
	if (fp != NULL)
	{
		char ver[1000];
		if (fgets (ver, 998, fp) != NULL)
		{
			return ver;
		}
		else
		{
			throw Exception ("Can't read file '%s' : code=%d %s", fn.c_str (), errno, strerror(errno));
		}
		fclose (fp);
	}
	else
	{
		nlwarning ("Can't open file '%s' : code=%d %s", fn.c_str (), errno, strerror(errno));
	}
	return "";
}

class CPatchThread : public IRunnable
{
public:
	
	CPatchThread(const string &sp, const string &sv, const std::string &urlOk, const std::string &urlFailed, const std::string &logSeparator) :
	  ServerPath (sp), ServerVersion(sv), UrlOk(urlOk), UrlFailed(urlFailed), Ended(false), StateChanged(true), LogSeparator(logSeparator)
	{
	}
	
	bool Ended;			// true if the thread have ended the patch
	bool PatchOk;		// true if the patch was good
	string Url;			// url to display after the patch
	
	string State;
	string StateLog;
	bool StateChanged;

private:

	string ClientRootPath;		// the root client path (c:\ryzom)
	string ClientPatchPath;		// the patch path (c:\ryzom\patch)
	string ServerRootPath;		// the root server path (http://www.toto.com)
	string DisplayedServerRootPath;		// contains the serverpath without login and password
	
	// get a file and decompress it in the patch directory
	void getFile (const CEntry &e)
	{
		string path = ClientPatchPath + e.Filename;
		nlinfo ("Get the file from '%s' to '%s'", string(DisplayedServerRootPath+e.Filename).c_str(), path.c_str());
		// get the new file
		downloadFile (ServerRootPath+e.Filename+".ngz", path+".ngz");
		// decompress it
		decompressFile (path+".ngz", e.Date);
	}


	void run ()
	{
		try
		{
			CurrentFilesToGet = 0;
			CurrentBytesToGet = 0;
			TotalFilesToGet = 0;
			TotalBytesToGet = 0;

			bool executeFinalizeBat = false;

			ClientRootPath = "./";
			ClientPatchPath = "./patch/";
			ServerRootPath = CPath::standardizePath (ServerPath);
			DisplayedServerRootPath;		// contains the serverpath without login and password
			
			uint pos = ServerRootPath.find ("@");
			if (pos != string::npos)
			{
				DisplayedServerRootPath = "http://"+ServerRootPath.substr (pos+1);
			}
			else
			{
				DisplayedServerRootPath = ServerRootPath;
			}

			setState(true, true, "Patching from '%s'", DisplayedServerRootPath.c_str());

			// create the patch directory if not exists
			if (!NLMISC::CFile::isExists ("patch"))
			{
				setState(true, true, "Creating patch directory");
				if (_mkdir ("patch") == -1)
				{
					throw Exception ("Can't create patch directory : code=%d %s", errno, strerror(errno));
				}
			}

			// first, get the file that contains all files (dir.ngz)
			deleteFile (DirFilename.c_str(), false);
			downloadFile (ServerRootPath+DirFilename, DirFilename);

			// now parse the file
			gzFile gz = gzopen (DirFilename.c_str (), "rb");
			if (gz == NULL)
			{
				int gzerrno;
				const char *gzerr = gzerror (gz, &gzerrno);
				throw Exception ("Can't open file '%s': code=%d %s", DirFilename.c_str(), gzerrno, gzerr);
			}

			vector<CEntry> filesList;
			vector<CEntry> needToGetFilesList;

			setState(true, true, "Parsing %s...", DirFilename.c_str());

			char buffer[2000];
			if (gzgets (gz, buffer, 2000) == NULL)
			{
				int gzerrno;
				const char *gzerr = gzerror (gz, &gzerrno);
				throw Exception ("Can't read header of'%s' : code=%d %s", DirFilename.c_str(), gzerrno, gzerr);
			}

			if (string(buffer) != "FILESLIST\n")
			{
				throw Exception ("%s has not a valid content '%s' : code=8888", DirFilename.c_str(), buffer);
			}

			while (!gzeof(gz))
			{
				if (gzgets (gz, buffer, 2000) == NULL)
				{
					int gzerrno;
					const char *gzerr = gzerror (gz, &gzerrno);
					throw Exception ("Can't read '%s' : code=%d %s", DirFilename.c_str(), gzerrno, gzerr);
				}
				
				string b = buffer;
				uint pos1 = b.find ("/");
				uint pos2 = b.find ("/", pos1+1);

				if (pos1 != string::npos || pos2 != string::npos)
				{
					string filename = b.substr (0, pos1);
					uint32 size = atoi(b.substr (pos1+1, pos2-pos1).c_str());
					uint32 date = atoi(b.substr (pos2+1).c_str());

					string path = ClientRootPath+filename;
					if (!NLMISC::CFile::fileExists (path))
					{
						path = ClientPatchPath + filename;
					}

					if (NLMISC::CFile::getFileModificationDate (path) != date || NLMISC::CFile::getFileSize (path) != size)
					{
						needToGetFilesList.push_back (CEntry(filename, size, date));
						TotalFilesToGet++;
						TotalBytesToGet += size;
					}
					filesList.push_back (CEntry(filename, size, date));
				}
			}
			gzclose (gz);

			// if we need to update nel_launcher.exe don't patch other file now, get
			// nel_launcher.exe and relaunch it now

			bool patchExe = false, patchCfg = false, patchBat = false;
	
			uint i;

			for (i = 0; i < needToGetFilesList.size(); i++)
			{
				string fn = needToGetFilesList[i].Filename;

				if (fn == NelLauncherFilename)
				{
					getFile (needToGetFilesList[i]);
					patchExe = true;
				}
				else if (fn == NelLauncherConfigFilename)
				{
					getFile (needToGetFilesList[i]);
					patchCfg = true;
				}
				else if (fn == RelaunchNelLauncherBatchFilename)
				{
					getFile (needToGetFilesList[i]);
					patchBat = true;
				}
			}

			if (patchBat)
			{
				setState (true, true, "Launching %s", RelaunchNelLauncherBatchFilename.c_str());
				
				//if (_execlp ("update_nel_launcher.bat", "update_nel_launcher.bat", NULL) == -1)
				STARTUPINFO si;
				PROCESS_INFORMATION pi;
				
				ZeroMemory( &si, sizeof(si) );
				// Flag permettant de prendre en compte wShowWindow
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_HIDE;
				
				si.cb = sizeof(si);
				
				ZeroMemory( &pi, sizeof(pi) );
				
				// Start the child process. 
				if( !CreateProcess( NULL, // No module name (use command line). 
					(char*)RelaunchNelLauncherBatchFilename.c_str(), // Command line. 
					NULL,             // Process handle not inheritable. 
					NULL,             // Thread handle not inheritable. 
					FALSE,            // Set handle inheritance to FALSE. 
					0,                // No creation flags. 
					NULL,             // Use parent's environment block. 
					NULL,             // Use parent's starting directory. 
					&si,              // Pointer to STARTUPINFO structure.
					&pi )             // Pointer to PROCESS_INFORMATION structure.
					) 
				{
					// error occurs during the launch
					string str = toString("Can't execute '%s': code=%d %s", RelaunchNelLauncherBatchFilename.c_str(), errno, strerror(errno));
					throw Exception (str);
				}
				
				// Close process and thread handles. 
				CloseHandle( pi.hProcess );
				CloseHandle( pi.hThread );
				
				exit(0);
			}
			else if (patchExe || patchCfg)
			{
				FILE *fp = fopen (UpdateNelLauncherBatchFilename.c_str(), "wt");
				if (fp == NULL)
				{
					string err = toString("Can't open file '%s' for writing: code=%d %s", UpdateNelLauncherBatchFilename.c_str(), errno, strerror(errno));
					throw Exception (err);
				}

				fprintf(fp, "@echo off\n");
				
				if (patchExe)
				{
					nlinfo ("Need to special patch '%s'",NelLauncherFilename.c_str());

					fprintf(fp, ":loopexe\n");
					fprintf(fp, "attrib -r -a -s -h %s\n", NelLauncherFilename.c_str());
					fprintf(fp, "del %s\n", NelLauncherFilename.c_str());
					fprintf(fp, "if exist %s goto loopexe\n", NelLauncherFilename.c_str());
					fprintf(fp, "move patch\\%s .\n", NelLauncherFilename.c_str());
				}

				if (patchCfg)
				{
					nlinfo ("Need to special patch '%s'",NelLauncherConfigFilename.c_str());

					fprintf(fp, ":loopcfg\n");
					fprintf(fp, "attrib -r -a -s -h %s\n", NelLauncherConfigFilename.c_str());
					fprintf(fp, "del %s\n", NelLauncherConfigFilename.c_str());
					fprintf(fp, "if exist %s goto loopcfg\n", NelLauncherConfigFilename.c_str());
					fprintf(fp, "move patch\\%s .\n", NelLauncherConfigFilename.c_str());
				}

				fprintf(fp, "start %s\n", NelLauncherFilename.c_str());
				
				fclose (fp);

				// remove the files list file
				setState (true, true, "Deleting %s", DirFilename.c_str());
				string err = deleteFile (DirFilename, false);
				if (!err.empty()) setState(true, true, err.c_str());
				
				// launching the .bat
				setState (true, true, "Launching %s", UpdateNelLauncherBatchFilename.c_str());
				
				//if (_execlp ("update_nel_launcher.bat", "update_nel_launcher.bat", NULL) == -1)
				STARTUPINFO si;
				PROCESS_INFORMATION pi;
				
				ZeroMemory( &si, sizeof(si) );
				// Flag permettant de prendre en compte wShowWindow
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_HIDE;
				
				si.cb = sizeof(si);
				
				ZeroMemory( &pi, sizeof(pi) );
				
				// Start the child process. 
				if( !CreateProcess( NULL, // No module name (use command line). 
					(char*)UpdateNelLauncherBatchFilename.c_str(), // Command line. 
					NULL,             // Process handle not inheritable. 
					NULL,             // Thread handle not inheritable. 
					FALSE,            // Set handle inheritance to FALSE. 
					0,                // No creation flags. 
					NULL,             // Use parent's environment block. 
					NULL,             // Use parent's starting directory. 
					&si,              // Pointer to STARTUPINFO structure.
					&pi )             // Pointer to PROCESS_INFORMATION structure.
					) 
				{
					// error occurs during the launch
					string str = toString("Can't execute '%s': code=%d %s", UpdateNelLauncherBatchFilename.c_str(), errno, strerror(errno));
					throw Exception (str);
				}
				
				// Close process and thread handles. 
				CloseHandle( pi.hProcess );
				CloseHandle( pi.hThread );
				
				exit(0);
			}

			// get files if necessary
			for (i = 0; i < needToGetFilesList.size (); i++)
			{
				// we already get these file, don't get it again (should never happen because already get on the last launch)
				if (needToGetFilesList[i].Filename == NelLauncherFilename ||
					needToGetFilesList[i].Filename == NelLauncherConfigFilename ||
					needToGetFilesList[i].Filename == RelaunchNelLauncherBatchFilename
					)
					continue;

				if (needToGetFilesList[i].Filename == FinalizeNelLauncherBatchFilename)
				{
					executeFinalizeBat = true;
				}
				
				// put the file in the ryzom patch directory
				string path = ClientPatchPath + needToGetFilesList[i].Filename;

				//nldebug ("path '%s' -> %d %s", path.c_str(), NLMISC::CFile::fileExists (ClientRootPath + needToGetFilesList[i].Filename), strlwr(NLMISC::CFile::getExtension(needToGetFilesList[i].Filename)).c_str());
				
				// move dll exe and already existing file in the root directory
				if (NLMISC::CFile::fileExists (ClientRootPath + needToGetFilesList[i].Filename) ||
					strlwr(NLMISC::CFile::getExtension(needToGetFilesList[i].Filename)) == "dll" ||
					strlwr(NLMISC::CFile::getExtension(needToGetFilesList[i].Filename)) == "exe")
				{
					path = ClientRootPath + needToGetFilesList[i].Filename;
				}

				nlinfo ("Get the file from '%s' to '%s'", string(DisplayedServerRootPath+needToGetFilesList[i].Filename).c_str(), path.c_str());

				// get the new file
				downloadFile (ServerRootPath+needToGetFilesList[i].Filename+".ngz", path+".ngz");
				// decompress it
				decompressFile (path+".ngz", needToGetFilesList[i].Date);
			}

			if (RootInternet != NULL)
			{
				InternetCloseHandle(RootInternet);
				RootInternet = NULL;
			}

			// now, we have to delete files that are not in the server list
	
			setState(true, true, "Scanning patch directory");
			vector<string> res;
			CPath::getPathContent(ClientPatchPath, false, false, true, res);

			for (i = 0; i < res.size (); i++)
			{
				string fn = NLMISC::CFile::getFilename (res[i]);
				uint j;
				for (j = 0; j < filesList.size (); j++)
				{
					if (fn == filesList[j].Filename)
					{
						break;
					}
				}
				if (j == filesList.size ())
				{
					string file = ClientPatchPath+fn;
					setState(true, true, "Deleting %s", file.c_str());
					string err = deleteFile (file, false);
					if (!err.empty()) setState(true, true, err.c_str());
				}
			}

			// remove the files list file
			setState (true, true, "Deleting %s", DirFilename.c_str());
			string err = deleteFile (DirFilename, false);
			if (!err.empty()) setState(true, true, err.c_str());

			// now that all is ok, we set the new client version
			setState (true, true, "set client version to %s", ServerVersion.c_str ());
			setVersion (ServerVersion);

			if (executeFinalizeBat)
			{
				// execute the configurator if necessary
				string fn = "patch\\"+FinalizeNelLauncherBatchFilename;
				setState (true, true, "Launching %s", fn.c_str());
				system(fn.c_str());
			}
			
			// it s the end of the patch process
			setState (true, true, "Patching completed");
		
			Url = UrlOk;
			PatchOk = true;
			Ended = true;
		}
		catch (Exception &e)
		{
			Url = UrlFailed;
			Url += e.what();
			PatchOk = false;
			Ended = true;
		}
	}

	void decompressFile (const string &filename, uint32 date)
	{
		setState(true, true, "Decompressing %s...", NLMISC::CFile::getFilename(filename).c_str ());

		gzFile gz = gzopen (filename.c_str (), "rb");
		if (gz == NULL)
		{
			string err = toString("Can't open compressed file '%s' : ", filename.c_str());
			if(errno == 0)
			{
				// gzerror
				int gzerrno;
				const char *gzerr = gzerror (gz, &gzerrno);
				err += toString("code=%d %s", gzerrno, gzerr);
			}
			else
			{
				err += toString("code=%d %s", errno, strerror (errno));
			}
			deleteFile (filename);
			throw Exception (err);
		}

		string dest = filename.substr(0, filename.size ()-3);
		setRWAccess(dest);
		FILE *fp = fopen (dest.c_str(), "wb");
		if (fp == NULL)
		{
			string err = toString("Can't open file '%s' : code=%d %s", dest.c_str(), errno, strerror(errno));
			
			gzclose(gz);
			deleteFile (filename);
			throw Exception (err);
		}
		
		uint32 currentSize = 0;
		uint8 buffer[10000];
		while (!gzeof(gz))
		{
			int res = gzread (gz, buffer, 10000);
			if (res == -1)
			{
				int gzerrno;
				const char *gzerr = gzerror (gz, &gzerrno);
				gzclose(gz);
				fclose(fp);
				//deleteFile (filename);
				throw Exception ("Can't read compressed file '%s' (after %d bytes) : code=%d %s", filename.c_str(), currentSize, gzerrno, gzerr);
			}

			currentSize += res;

			int res2 = fwrite (buffer, 1, res, fp);
			if (res2 != res)
			{
				string err = toString("Can't write file '%s' : code=%d %s", dest.c_str(), errno, strerror(errno));

				gzclose(gz);
				fclose(fp);
				deleteFile (filename);
				throw Exception (err);
			}
		}

		gzclose(gz);
		fclose(fp);
		deleteFile (filename);

		// change the file time for having the same as the server side

		if(date != 0)
		{
			_utimbuf utb;
			utb.actime = utb.modtime = date;
			setRWAccess(dest);
			if (_utime (dest.c_str (), &utb) == -1)
			{
				nlwarning ("Can't change file time for '%s' : code=%d %s", dest.c_str (), errno, strerror(errno));
			}
		}
	}


	void downloadFile (const string &source, const string &dest)
	{
		const uint32 bufferSize = 8000;
		uint8 buffer[bufferSize];

		if (RootInternet == NULL)
		{
			RootInternet = InternetOpen("nel_launcher", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
			if (RootInternet == NULL)
			{
				// error
				LPVOID lpMsgBuf;
				string errorstr;
				DWORD errcode = GetLastError ();
				if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
					errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf, 0, NULL) == 0)
				{
					errorstr = (LPCTSTR)lpMsgBuf;
				}
				LocalFree(lpMsgBuf);
				
				throw Exception ("InternetOpen() failed: %s (ec %d)", errorstr.c_str(), errcode);
			}
		}

		HINTERNET hUrlDump = InternetOpenUrl(RootInternet, source.c_str(), NULL, NULL, INTERNET_FLAG_NO_AUTO_REDIRECT | INTERNET_FLAG_RAW_DATA, 0);
		if (hUrlDump == NULL)
		{
			// error
			LPVOID lpMsgBuf;
			string errorstr;
			DWORD errcode = GetLastError ();
			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
				errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf, 0, NULL) == 0)
			{
				errorstr = (LPCTSTR)lpMsgBuf;
			}
			LocalFree(lpMsgBuf);

			throw Exception ("InternetOpenUrl() failed on file '%s': %s (ec %d)", source.c_str (), errorstr.c_str(), errcode);
		}

		setRWAccess(dest);
		FILE *fp = fopen (dest.c_str(), "wb");
		if (fp == NULL)
		{
			throw Exception ("Can't open file '%s' for writing: code=%d %s", dest.c_str (), errno, strerror(errno));
		}

		CurrentFilesToGet++;

		setState(true, true, "Getting %s", NLMISC::CFile::getFilename (source).c_str ());

		do
		{
			DWORD realSize;

			if(!InternetReadFile(hUrlDump,(LPVOID)buffer, bufferSize, &realSize))
			{
				LPVOID lpMsgBuf;
				string errorstr;
				DWORD errcode = GetLastError ();
				if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
					errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf, 0, NULL) == 0)
				{
					errorstr = (LPCTSTR)lpMsgBuf;
				}
				LocalFree(lpMsgBuf);
				
				throw Exception ("InternetOpenUrl() failed on file '%s': %s (ec %d)", source.c_str (), errorstr.c_str(), errcode);
			}
			else
			{
				if (realSize == 0)
				{
					// download complete successfully
					break;
				}

				int res2 = fwrite (buffer, 1, realSize, fp);
				if ((DWORD)res2 != realSize)
				{
					string err = toString("Can't write file '%s' : code=%d %s", dest.c_str(), errno, strerror(errno));

					fclose(fp);
					deleteFile (dest);
					throw Exception (err);
				}
				
				CurrentBytesToGet += realSize;

				if (TotalBytesToGet == 0 && TotalFilesToGet == 0)
					setState(false, false, "Getting %s, %d bytes downloaded", NLMISC::CFile::getFilename (source).c_str (), CurrentBytesToGet);
				else
					setState(false, false, "Getting file %d on %d, %d bytes, filename %s", CurrentFilesToGet, TotalFilesToGet, CurrentBytesToGet, NLMISC::CFile::getFilename (source).c_str ());

			}
		}
		while (true);

		fclose (fp);
		if (!InternetCloseHandle(hUrlDump))
		{
			LPVOID lpMsgBuf;
			string errorstr;
			DWORD errcode = GetLastError ();
			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
				errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf, 0, NULL) == 0)
			{
				errorstr = (LPCTSTR)lpMsgBuf;
			}
			LocalFree(lpMsgBuf);
			
			throw Exception ("InternetCloseHandle() failed on file '%s': %s (ec %d)", source.c_str (), errorstr.c_str(), errcode);
		}
	}


	void setState (bool info, bool log, const char *format, ...)
	{
		char *str;
		NLMISC_CONVERT_VARGS (str, format, 256);
		if (info)
			nlinfo (str);
		State = str;
		if(log)
		{ 
			StateLog += str;
			StateLog += LogSeparator;
		}
		StateChanged = true;
	}

	string LogSeparator;

	string ServerPath;
	string ServerVersion;

	string UrlOk;
	string UrlFailed;

public:
	uint TotalFilesToGet;
	uint TotalBytesToGet;
	uint CurrentFilesToGet;
	uint CurrentBytesToGet;
};

CPatchThread *PatchThread = NULL;

void startPatchThread (const std::string &serverPath, const std::string &serverVersion, const std::string &urlOk, const std::string &urlFailed, const std::string &logSeparator)
{
	if (PatchThread != NULL)
	{
		nlwarning ("patch thread already running");
		return;
	}
	
	PatchThread = new CPatchThread (serverPath, serverVersion, urlOk, urlFailed, logSeparator);
	nlassert (PatchThread != NULL);

	IThread *thread = IThread::create (PatchThread);
	nlassert (thread != NULL);
	thread->start ();
}

bool patchEnded (string &url, bool &ok)
{
	nlassert (PatchThread != NULL);

	bool end = PatchThread->Ended;
	if (end)
	{
		url = PatchThread->Url;
		ok = PatchThread->PatchOk;

		delete PatchThread;
		PatchThread = NULL;
	}

	return end;
}

bool patchState (string &state, std::string &stateLog)
{
	if (PatchThread == NULL)
		return false;

	bool statechanged = PatchThread->StateChanged;
	if (statechanged)
	{
		state = PatchThread->State;
		stateLog = PatchThread->StateLog;
		PatchThread->StateChanged = false;
	}

	return statechanged;
}

int getTotalFilesToGet()
{
	if(PatchThread)
		return PatchThread->TotalFilesToGet;
	return 0;
}

int getCurrentFilesToGet()
{
	if(PatchThread)
		return PatchThread->CurrentFilesToGet;
	return 0;
}

int getTotalBytesToGet()
{
	if(PatchThread)
		return PatchThread->TotalBytesToGet;
	return 0;
}

int getCurrentBytesToGet()
{
	if(PatchThread)
		return PatchThread->CurrentBytesToGet;
	return 0;
}