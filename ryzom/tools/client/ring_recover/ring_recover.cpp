/** \file ring_recover.cpp
 * App to setup minimal ryzom ring context
 *
 * $Id: ring_recover.cpp,v 1.3 2007/06/07 13:34:25 boucher Exp $
 */

/* ----------------------------------------------------------
 *
 * Ring Installer
 *
 * This app is a direct copy of the Ryzom Installer app. For more information on it's workings see ryzom_recover.cpp
 *
 * ---------------------------------------------------------- */



//
// Includes
//

#include <conio.h>
#include <io.h>
#include <errno.h>
#include <direct.h>
#include <windows.h>
#include <wininet.h>
#include <process.h>
//#include <sys/stat.h>
//#include <sys/utime.h>
#include <sys/types.h>

#undef min
#undef max

#include <vector>
#include <queue>
#include <string>

#include <zlib.h>
#include <curl/curl.h>

#include <nel/misc/path.h>
#include <nel/misc/debug.h>
#include <nel/misc/report.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/sstring.h>

#include <nel/net/email.h>


//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//
// External function prototypes
//

void addShortcutToDesktop();


//
// Variables
//

string currentFile;

string DirFilename = "ring_dir.gz";

string NameOfThisApplication;

// These paths are for the *gold* version
const char*BaseURI[] = {	// don't forget the final /
	"http://recover.ryzom.com:43434/ring/",
};

/**
 * Download Timeout (in seconds)
 * Checked when all downloading sources have failed to be reached
 */
const uint		DownloadTimeout = 600;

/**
 * Time to wait between each attempt to download from sources.
 */
const uint		MinFailureWait = 10;
const uint		MaxFailureWait = 20;

/**
 * Extended URIs, based on BaseURI plus those downloaded from 'dir.gz' if RECOV2 present
 */
vector<string>	ExtURI;


// Don't forget the blank character at the end
const string ApplCouldNot = "The application could not ";
const string Explain1Download = ApplCouldNot + "download a file. ";
const string Explain1OpenRead = ApplCouldNot + "decompress a file. ";
const string Explain1OpenWrite = ApplCouldNot + "create a file. ";
const string Explain1MkDir = ApplCouldNot + "create a folder. ";
const string Explain1Decompress = ApplCouldNot + "decompress a file. ";
const string Explain1Access = ApplCouldNot + "get read/write access to a file. ";
const string Explain1Delete = ApplCouldNot + "delete a file. ";
const string Explain1InvalidContent = ApplCouldNot + "download a valid file list. ";
const string Explain1Internal = "An internal error occured. ";
const string Explain2Account = "If you use multiple Windows user accounts, you may have to log in with an administrator account, or with the same account that was used to install The Saga Of Ryzom. ";
const string Explain2ProgramLocking = "Please close any other program that could be using the file. ";
const string Explain2DiskSpace = "Please make sure you have sufficient disk space. ";
const string Explain2Network = "Please make sure your network settings are properly configured. If you have a firewall, you may have to configure it to accept an outgoing connection. ";

enum TErrorStatus {
	ESRecoverableAtOnce,	// ask to retry
	ESRecoverableLater,		// ask to run again
	ESUnrecoverable			// ask to contact support
};

class ERestartApplication : public NLMISC::Exception {};

//
// Functions
//

/*
 * Display a user-friendly error message, suggest to send a nlerror report, then stop the application.
 */
void userFriendlyError( const string& explanation, bool hasDirectionsInExplanation, TErrorStatus errorStatus, const string& techReason, const string& later="" )
{
	nlwarning( "ERROR: %s %s %u", explanation.c_str(), techReason.c_str(), (uint)errorStatus );

	string text = explanation;
	if ( hasDirectionsInExplanation )
		text += "\r\n\nThen ";
	else
		text += "\r\n\nPlease ";
	UINT mbButtons;

	// Display message(s) boxe(s):
	//
	//	 Recoverable at once        Recoverable later                    Unrecoverable
	//           |                          |                                |
	//           v                          v                                v
	//     Retry / Cancel --> Directions to run again or contact support --> OK
	//           |                                                           |
	//           v                                                           v
	//   Restart application                                         NeL error message box

	// Ask to retry, if not done too many times
	if ( errorStatus == ESRecoverableAtOnce )
	{
		static uint nbRetries = 0;
		++nbRetries;
		if ( nbRetries < 4 )
		{
			text += "click Retry to restart the application now.\r\n";
			mbButtons = MB_RETRYCANCEL;
		}
		else
			errorStatus = ESRecoverableLater;
	}

	int clicked;
	do
	{
		// Ask to restart later and/or to contact support
		if ( errorStatus != ESRecoverableAtOnce )
		{
			switch ( errorStatus )
			{
			case ESRecoverableLater:
				text += "run this application again" + later + ", by executing " + NameOfThisApplication + "\r\n\n";
				text += "If running several times this application does not resolve the problem, please ";
				break;
			default: // ARContactSupport
				break;
			}
			text += "contact technical support at http://atys.ryzom.com/?page=support\r\n\n";
			mbButtons = MB_OK;
		}

		// Present the next NeL Error message box
		if ( mbButtons == MB_OK )
			text += "Now you'll see the detailed technical reason of the error. You will have the possibility to send a report to Nevrax.\r\n";

		// Display the message box (ansi mode, no unicode string)
		clicked = ::MessageBox( NULL, text.c_str(), "Ring Installer Application: Error", mbButtons | MB_ICONINFORMATION | MB_SETFOREGROUND );
		switch ( clicked )
		{
		case IDRETRY:
			throw ERestartApplication();
			break;
		case IDCANCEL:
			text = "If you want to retry later, ";
			errorStatus = ESRecoverableLater;
			break;
		default: // IDOK
			nlerror( techReason.c_str() );
		}
	}
	while ( clicked != IDOK );

	// Do not let the user 'ignore' the error
	printf("\n\nRing Installer failed. ");
	exit( 1 );
}


std::string dashes(uint32 n)
{
	std::string result;
	result.resize(n,'-');
	return result;
}

void setRWAccess (const string &filename)
{
	nlinfo("setRWAccess to '%s'", filename.c_str());
	
	if (!NLMISC::CFile::setRWAccess(filename))
	{
		userFriendlyError( Explain1Access + Explain2Account, true, ESRecoverableLater,
			toString("Can't have read/write access to '%s' file : code=%d %s (error code 18)", filename.c_str(), errno, strerror(errno)) );
	}
}

string deleteFile (const string &filename, bool failSilently=false)
{
	nlinfo("delete file '%s'", filename.c_str());
	
	if (!NLMISC::CFile::deleteFile(filename))
	{
		string str = toString("Can't delete '%s' file : code=%d %s (error code 19)", filename.c_str(), errno, strerror(errno));
		if ( ! failSilently )
			userFriendlyError( Explain1Delete + Explain2ProgramLocking + Explain2Account, true,
				ESRecoverableAtOnce, str.c_str() );
		return str;
	}
	return "";
}

void decompressFile (const string &filename)
{
	nlinfo("Decompressing %s...", NLMISC::CFile::getFilename(filename).c_str ());
	
	nlinfo("Calling gzopen('%s','rb')", filename.c_str());
	gzFile gz = gzopen (filename.c_str (), "rb");
	if (gz == 0)
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
		err += " (error code 31)";
		deleteFile (filename, true);
		userFriendlyError( Explain1OpenRead, false, ESUnrecoverable, err.c_str() );
	}

	string dest = filename.substr(0, filename.size ()-3);
	setRWAccess(dest);
	nlinfo("Calling fopen('%s','wb')", dest.c_str());
	FILE *fp = fopen (dest.c_str(), "wb");
	if (fp == 0)
	{
		string err = toString("Can't open file '%s' : code=%d %s, (error code 32)", dest.c_str(), errno, strerror(errno));

		gzclose(gz);
		deleteFile (filename, true);
		userFriendlyError( Explain1Decompress + Explain2DiskSpace + Explain2Account, true, ESRecoverableAtOnce, err.c_str() );
	}
	
	nlinfo("Entering the while loop decompression");
	
	uint32 currentSize = 0;
	uint8 buffer[10000];
	while (!gzeof(gz))
	{
		nlinfo("Calling gzread");
		int res = gzread (gz, buffer, 10000);
		nlinfo("gzread returns %d", res);
		if (res == -1)
		{
			int gzerrno;
			const char *gzerr = gzerror (gz, &gzerrno);
			gzclose(gz);
			fclose(fp);
			//deleteFile (filename);
			userFriendlyError( Explain1Decompress, false, ESUnrecoverable,
				toString("Can't read compressed file '%s' (after %d bytes) : code=%d %s, (error code 33)", filename.c_str(), currentSize, gzerrno, gzerr) );
		}
		
		currentSize += res;
		
		nlinfo("Calling fwrite for %d bytes", res);
		int res2 = fwrite (buffer, 1, res, fp);
		nlinfo("fwrite returns %d", res2);
		if (res2 != res)
		{
			string err = toString("Can't write file '%s' : code=%d %s (error code 34)", dest.c_str(), errno, strerror(errno));
			
			gzclose(gz);
			fclose(fp);
			deleteFile (filename, true);
			userFriendlyError( Explain1Decompress + Explain2DiskSpace + Explain2Account, true, ESRecoverableAtOnce, err.c_str() );
		}
	}
	
	nlinfo("Exiting the while loop decompression");
	
	nlinfo("Calling gzclose");
	gzclose(gz);
	nlinfo("Calling fclose");
	fclose(fp);
	deleteFile(filename, true);
	
	nlinfo("Exiting the decompressing file");
}

// source is an uri, dest is a full path
bool downloadFileWithCurl(const string &source, const string &dest)
{
	nlinfo("downloadFileWithCurl file src '%s' dst '%s'", source.c_str(), dest.c_str());

	// delete the file if already exists
	if(CFile::fileExists(dest))
		deleteFile(dest);

	CURL *curl;
	CURLcode res;
	
	nlinfo("Getting %s", NLMISC::CFile::getFilename (source).c_str ());
	currentFile = NLMISC::CFile::getFilename (source);

	// create the local file
	setRWAccess(dest);
	FILE *fp = fopen (dest.c_str(), "wb");
	if(!fp)
	{
		userFriendlyError( Explain1OpenWrite + Explain2ProgramLocking + Explain2DiskSpace + Explain2Account, true, ESRecoverableAtOnce,
			toString("Can't open file '%s' for writing: code=%d %s (error code 37)", dest.c_str (), errno, strerror(errno)) );
	}

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(!curl)
	{
		userFriendlyError( Explain1Internal, false, ESUnrecoverable, "curl init failed" );
	}
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(curl, CURLOPT_URL, source.c_str());
	curl_easy_setopt(curl, CURLOPT_FILE, fp);

	res = curl_easy_perform(curl);
	
	long r;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &r);
	
	curl_easy_cleanup(curl);
	
	fclose(fp);
	curl_global_cleanup();

	currentFile = "";

	if(CURLE_FTP_COULDNT_RETR_FILE == res)
	{
		// file not found, delete local file
		NLMISC::CFile::deleteFile(dest.c_str());
		nlwarning("curl download failed: (ec %d %d)", res, r);
		return false;
	}

	if(CURLE_WRITE_ERROR == res)
	{
		userFriendlyError( Explain1Decompress + Explain2DiskSpace, true, ESRecoverableAtOnce,
			toString("Curl can't write '%s'", dest.c_str()) );
	}

	if(CURLE_OK != res)
	{
		NLMISC::CFile::deleteFile(dest.c_str());
		nlwarning("curl download failed: (ec %d %d)", res, r);
		return false;
	}
	
	if(r == 404)
	{
		// file not found, delete it
		NLMISC::CFile::deleteFile(dest.c_str());
		nlwarning("curl download failed because url '%s' it not found: (ec %d %d)", source.c_str(), res, r);
		return false;
	}	

	if(r < 200 || r >= 300)
	{
		// file not found, delete it
		NLMISC::CFile::deleteFile(dest.c_str());
		nlwarning("return code is not in 200 range OK: (ec %d %d)", res, r);
		return false;
	}
	return true;
}


/*
 * Download a file from the last valid URI
 */
uint32 LastURINumber = 0;

void download(const string &rawsrc, const string &dest)
{
	// add a log message
	nlinfo("Downloading file rawsrc '%s' dst '%s'", rawsrc.c_str(), dest.c_str());

	// display a friendly message for the person downloading to read
	static uint count=0;
	NLMISC::CSString displayedName= rawsrc;
	if (displayedName.right(3)==".gz")	displayedName= displayedName.rightCrop(3);
	std::string message= NLMISC::toString("\nDownloading %d: %s", ++count, displayedName.c_str());
	printf("%s\n", message.c_str());
	printf("%s\n", dashes(message.size()).c_str());

	TTime	start = CTime::getLocalTime();
	TTime	timeout = start + DownloadTimeout*1000;

	srand((uint32)start);

	// try to download file until timeout
	while (CTime::getLocalTime() < timeout)
	{
		uint32 NbURI = ExtURI.size();
		for (uint i = 0; i < NbURI; i++)
		{
			// clamp to table
			LastURINumber = LastURINumber % NbURI;

			if (downloadFileWithCurl(ExtURI[LastURINumber]+rawsrc, dest))
			{
				nlinfo("Successfully downloaded file '%s' from '%s'", rawsrc.c_str(), ExtURI[LastURINumber].c_str());
				printf("\n%s\n\n", dashes(79).c_str());
				return;
			}

			// download failed, try with the next one
			++LastURINumber;
		}

		// download failed on all sources, retry till timeout expires
		// wait some random seconds to avoid flooding servers
		uint	wait = MinFailureWait + (uint)frand(MaxFailureWait-MinFailureWait);
		TTime	lock = CTime::getLocalTime();
		while (CTime::getLocalTime() - lock < wait*1000)
			nlSleep(100);
	}

	// no uri works, can't do anything
	userFriendlyError( Explain1Download + Explain2Network, true, ESRecoverableAtOnce,
		toString("No URI can retrieve the file '%s'", rawsrc.c_str()) );

	printf("\n%s\n\n", dashes(79).c_str());
}



/*
 * Append Base URIs in ExtURI list
 */
void appendBaseURIsToExtURIs()
{
	uint32 NbURI = sizeof(BaseURI)/sizeof(BaseURI[0]);
	for (uint i=0; i<NbURI; ++i)
	{
		ExtURI.push_back(string(BaseURI[i]));
	}
}


/*int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)*/
int main(int argc, char **argv)
{
	printf("\n");
	printf("  **************************************************************************\n");
	printf("  ** We are now patching The Ryzom Ring to your computer.                 **\n");
	printf("  ** This might take several minutes depending on your connection.        **\n");
	printf("  ** Please be patient. This window will close when the patching is over. **\n");
	printf("  **************************************************************************\n");
	printf("\n");

	createDebug(0, true);

	setReportEmailFunction ((void*)sendEmail);
	setDefaultEmailParams ("gw.nevrax.com", "ring_recover@recover.com", "miller@nevrax.com");

	DebugLog->removeDisplayer("DEFAULT_SD");
	InfoLog->removeDisplayer("DEFAULT_SD");
	WarningLog->removeDisplayer("DEFAULT_SD");
	AssertLog->removeDisplayer("DEFAULT_SD");
	ErrorLog->removeDisplayer("DEFAULT_SD");

	NameOfThisApplication = argv[0];


	// assume that this exe is being run in the directory where ryzom is installed
	std::string ryzomInstallPath= ".";

	// if the ryzom exe isn't found here then try looking in the directory where this app is installed
	if (!NLMISC::CFile::fileExists(ryzomInstallPath+"/client_ryzom_rd.exe"))
	{
		ryzomInstallPath= NLMISC::CPath::standardizePath(NLMISC::CFile::getPath(NameOfThisApplication));
	}

	// if we still can't find the ryzom client exe then give up
	if (!NLMISC::CFile::fileExists(ryzomInstallPath+"/client_ryzom_rd.exe"))
	{
		printf("\n");
		printf("ERROR: This program must be run in the directory where ryzom is installed\n");
		printf("\n");
		getch();
		exit(1);
	}

	// create the sub directory that we need to setup the ring and change directory to there
	NLMISC::CFile::createDirectoryTree(ryzomInstallPath+"/ring");
	NLMISC::CPath::setCurrentPath((ryzomInstallPath+"/ring").c_str());
//	if (!ok)
//	{
//		printf("\n");
//		printf("ERROR: Failed to create directory: %s\n",(ryzomInstallPath+"/ring").c_str());
//		printf("\n");
//		getch();
//		exit(1);
//	}

	// get the file that contains the list of files to download

	// first time, init URIs first base URIs

	gzFile gz = 0;
	bool mustRestart = false;

	do
	{
		try
		{
			ExtURI.clear();
			appendBaseURIsToExtURIs();

			bool mustReDownloadNewDir = false;
			do
			{
				if ( gz != 0 )
				{
					gzclose(gz);
					gz = 0;
				}
				download(DirFilename, DirFilename);

				gz = gzopen (DirFilename.c_str (), "rb");
				if (gz == 0)
				{
					int gzerrno;
					const char *gzerr = gzerror (gz, &gzerrno);
					userFriendlyError( Explain1OpenRead, false, ESUnrecoverable,
						toString("Can't open file '%s': code=%d %s (error code 24)", DirFilename.c_str(), gzerrno, gzerr) );
				}

				char buffer[2000];
				if (gzgets (gz, buffer, 2000) == 0)
				{
					int gzerrno;
					const char *gzerr = gzerror (gz, &gzerrno);
					userFriendlyError( Explain1Decompress, false, ESUnrecoverable,
						toString("Can't read header of'%s' : code=%d %s (error code 25)", DirFilename.c_str(), gzerrno, gzerr) );
				}

				// check the header file
				if (string(buffer) == "RECOV\n")
				{
					/*
					 * Nothing to do, no new URIs provided
					 */
					nlinfo("Identified recover scheme 1 in %s", DirFilename.c_str());
				}
				else if (string(buffer) == "RECOV2\n")
				{
					/*
					 * Read URI list till we find "FILELIST" token
					 * then proceed normally (read file list)
					 */
					nlinfo("Identified recover scheme 2 in %s, retrieving latest URI", DirFilename.c_str());

					// reset URIs
					ExtURI.clear();
					LastURINumber = 0;

					do
					{
						// get next uri
						if (gzgets (gz, buffer, 2000) == 0)
						{
							int gzerrno;
							const char *gzerr = gzerror (gz, &gzerrno);
							userFriendlyError( Explain1Decompress, false, ESUnrecoverable,
								toString("Can't read '%s' : code=%d %s (error code 27)", DirFilename.c_str(), gzerrno, gzerr) );
						}

						// is it end of list?
						if (string(buffer) == "FILELIST\n")
							break;

						// remove if \n if necessary
						if(strlen(buffer)>0 && buffer[strlen(buffer)-1] == '\n')
							buffer[strlen(buffer)-1] = '\0';

						ExtURI.push_back(buffer);
					}
					while (!gzeof(gz));

					// append to extURI list
					appendBaseURIsToExtURIs();
				}
				else
				{
					gzclose(gz);
					deleteFile(DirFilename, true);
					++LastURINumber;
					if ( LastURINumber >= ExtURI.size() )
					{
						userFriendlyError( Explain1InvalidContent, false, ESRecoverableLater,
							toString("%s has not a valid content '%s' : code=8888 (error code 26)", DirFilename.c_str(), buffer),
							" later" );
					}
					mustReDownloadNewDir = true;
					continue;
				}

				while (!gzeof(gz))
				{
					// get a line
					if (gzgets (gz, buffer, 2000) == 0)
					{
						int gzerrno;
						const char *gzerr = gzerror (gz, &gzerrno);
						userFriendlyError( Explain1Decompress, false, ESUnrecoverable,
							toString("Can't read '%s' : code=%d %s (error code 27)", DirFilename.c_str(), gzerrno, gzerr) );
					}

					// remove if \n if necessary
					if(strlen(buffer)>0 && buffer[strlen(buffer)-1] == '\n')
						buffer[strlen(buffer)-1] = '\0';

					// explode the string into | sep
					string b = buffer;
					vector<string> res;
					explode(b, "|", res);

					// error if doesn't contain 2 fields
					if(res.size() != 2)
					{
						gzclose(gz);
						gz=0;
						deleteFile(DirFilename, true);
						++LastURINumber;
						if ( LastURINumber >= ExtURI.size() )
						{
							userFriendlyError( Explain1InvalidContent, false, ESRecoverableLater, toString("bad entry '%s'", b.c_str()) );
						}
						mustReDownloadNewDir = true; // if RECOV2, will search the dir in the new set of URIs, but skips the first one!
						break;
					}

					// create the destination directory if necessary
					string path = CFile::getPath(res[1]);
					if( (!path.empty()) && (!CFile::isExists(path)) )
					{
						if ( ! CFile::createDirectory(path) )
							userFriendlyError( Explain1MkDir + Explain2Account, true, ESRecoverableAtOnce,
								toString("Can't create dir '%s'", path.c_str()) );
					}
					
					// download the file
					download(res[0], res[1]);
					// decompress it
					decompressFile(res[1]);
				}
				if ( mustReDownloadNewDir )
					continue;
				gzclose(gz);
				gz = 0;
				deleteFile(DirFilename, true);
			}
			while ( mustReDownloadNewDir );
		}
		catch ( ERestartApplication& )
		{
			mustRestart = true;
		}
		catch ( ... )
		{
			userFriendlyError( Explain1Internal, false, ESUnrecoverable, "Caught '...'" );
		}
	}
	while ( mustRestart );

	printf("\n");
	printf("Installing shortcuts on desktop and in start menu...\n");

	addShortcutToDesktop();

	printf("\n");
	printf("The Ryzom Ring was successfully patched by Ring Installer.\n" );
	printf("Please wait while Ryzom launches and begins patching.\n" );

    int result= _spawnl(_P_WAIT, ".\\client_ryzom_dev_rd.exe", ".\\client_ryzom_dev_rd.exe", NULL );

	return result;
}

