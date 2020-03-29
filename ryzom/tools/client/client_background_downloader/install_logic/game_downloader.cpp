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

#include "stdpch.h"



#include "game_downloader.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include <exception>


#include "ryzom_control.h"
#include "torrent_control.h"
#include <string.h>


#include "client/http_client.h"
#include "game_share/xml_auto_ptr.h"
#include "nel/misc/file.h"
#include "nel/misc/algo.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/i18n.h"


#include "client_background_downloader/resource.h"
#include "client_background_downloader/choose_package_dlg.h"
#include "client_background_downloader/install_task_dlg.h"
#include "client_background_downloader/install_success_dlg.h"
#include "client_background_downloader/error_box_dlg.h"
#include "client_background_downloader/client_background_downloader.h"
#include "client/login_progress_post_thread.h"
#include "game_share/login_registry.h"


// transform an int to a string
static std::string toString(int value)
{
	static char buffer[256];
	sprintf(buffer, "%d", (int)value);	
	return std::string(buffer);
}

//transform an uint64 to a Hex string
static std::string uint64ToHex(uint64 size)
{
	char data[256];
	sprintf(data, "%llX", size);
	return std::string(data);
}


//transform a size to a Human readable size ex "1.83GB"
static std::string sizeToHumanStd(uint64 size)
{
		double total = static_cast<double>(size);		
		std::string unit;
		//dispaly 2 digit after .
		if (total >= 1000000000.0)
		{
			total /= 10000000.0;
			total =  (int)total ;
			total /= 100.0;
			unit = "GB";
		} 
		else if (total >= 1000000.0)
		{
			total /= 10000.0;
			total = (int)total ;
			total /= 100.0;
			unit = "MB";
		}
		else if (total >= 1000.0)
		{			
			total = int(total/1000.0);
			unit = "KB";
		}
		else
		{
			unit = "B";
		}
		static char buffer[256];
		if ( total > 100.0)
		{		
			sprintf(buffer, "%0.0f", (double)total);
		}
		else if (total > 10.0)
		{
			sprintf(buffer, "%0.1f", (double)total);
		}
		else
		{
			sprintf(buffer, "%0.2f", (double)total);
		}		
		std::string ret(buffer);
		ret+=unit;

		return ret;
}



static ucstring wrapText(ucstring content)
{
	NLMISC::strFindReplace(content, "\\n", ucstring(std::string("\n")));
	// replace "\\n" by "\n"
	
	ucstring tmp;

	uint32 first = 0, last = content.size();
	int count =0;
	for ( ; first != last; ++first)
	{
		if (content[first] == '\n') { count = 0;}
		else { ++count; }
		tmp.push_back(content[first]);
		// if ligne is longer than 50 cut at the next ' '
		if (count >= 50 && content[first] == ' ')
		{
			tmp.push_back('\r');
			tmp.push_back('\n');
			count = 0;
		}
	}
	content = tmp;
	return content;
}


/** Extract a value from an simple xml vlaue "<token>value</token>"
*/
static std::string extractToken(const std::string& res, const std::string& token)
{
	std::string ret("");
	std::string tokenBegin = std::string("<") + token + ">";
	std::string::size_type tokenBeginLen = tokenBegin.size();
	std::string tokenEnd = std::string("</") + token + ">";
	std::string::size_type tokenEndLen = tokenBegin.size();

	// if res looks like "<token>value</token>" extract value and put it into ret
	std::string::size_type begin = res.find(tokenBegin);
	std::string::size_type end = begin != std::string::npos && res.size() > tokenBeginLen + tokenEndLen ? res.find(tokenEnd, begin+tokenBeginLen) : std::string::npos;
	if (begin != std::string::npos && end != std::string::npos)
	{
		ret = res.substr(begin+tokenBeginLen, end - (begin+tokenBeginLen));
	}
	return ret;
}


//! Store patch info (from version description file given By Ryzom module)
class CEntry
{
public:
	CEntry(){ Timestamp = 0; Size = uint32(-1); }

	CEntry(const char* patchName, const char* sourceName, uint32 timestamp, const char* extractPath, uint32 sevenZipFileSize,  uint32 size)
		:PatchName(patchName), SourceName(sourceName), Timestamp(timestamp),ExtractPath(extractPath), SevenZipFileSize(sevenZipFileSize), Size(size){}

	std::string PatchName; //!< The name of the patch "0422/exedll.bnp"
	std::string SourceName; //!< The name of the source "data/exedll.bnp"
	uint32 Timestamp; //!< The timestamp of the unpacked file
	std::string ExtractPath;  //!< the directory whe data can be unpack eg "data/" 
	uint32 Size;//!< The size of the bnp file after depacking the lzma
	uint32 SevenZipFileSize; //!The size of the lzma
};

//! Use to feed IRyzomControl to specify the list of file we want to download/intall
class CStringContainer : public IStringContainer
{
public:
	CStringContainer(std::vector<std::string>* data):_Data(data){}
	virtual uint32 getCount() const { return _Data->size(); }
	virtual const char * getName(uint32 index) const { return (*_Data)[index].c_str(); }
private:
	std::vector<std::string>* _Data;
};

//! Adapter use to feed ITorrentmControl to specify the list of file we want to download form the torrent (other are ignored)
class CTorrentStringContainer : public ITorrentStringContainer
{
public:
	/** Create the Adapter for ITorrentControl
	\param data The native container use by CGameDownloader
	\pram prefix The torrent directory
	*/
	CTorrentStringContainer(std::vector<CEntry>* data, const std::string& prefix):_Data(data), _Prefix(prefix){}
	//! \seed ITorrentStringContainer::getCount
	virtual uint32 getCount() const { return _Data->size(); }

	//! \seed ITorrentStringContainer::getName
	virtual const char * getName(uint32 index) const 
	{ 
		static std::string ret;
		ret = _Prefix;
		ret += (*_Data)[index].PatchName;
		return ret.c_str(); 
	}

	//! \seed ITorrentStringContainer::setSize
	virtual void setSize(uint32 index, uint32 size)
	{ 
		(*_Data)[index].Size = size;	
	}
private:
	std::vector<CEntry>* _Data; //!< Ptr to real container
	std::string _Prefix;	//!< torrent directory
};


//! Adapter to inform IRyzomControl of info from idx file
class CInstallEntryViewer : public IInstallEntryViewer
{
public:
	//! Use the download list from CGameDownloader
	CInstallEntryViewer(std::vector<CEntry>* data):_Data(data){}
	//! \see IInstallEntryViewer::getCount
	virtual uint32 getCount() const { return _Data->size(); }

	//! \see IInstallEntryViewer::getPatchName
	virtual const char * getPatchName(uint32 index) const { return (*_Data)[index].PatchName.c_str(); } 
	
	//! \see IInstallEntryViewer::getSourceName
	virtual const char * getSourceName(uint32 index) const { return (*_Data)[index].SourceName.c_str(); } 

	//! \see IInstallEntryViewer::getTimestamp
	virtual uint32 getTimestamp(uint32 index) const { return (*_Data)[index].Timestamp; } 

	//! \see IInstallEntryViewer::getExtractPath
	virtual const char * getExtractPath(uint32 index) const { return (*_Data)[index].ExtractPath.c_str(); } 

	//! \see IInstallEntryViewer::getSize
	virtual uint32 getSize(uint32 index) const { return (*_Data)[index].Size; } 

	//! \see IInstallEntryViewer::getSZipFileSize
	virtual uint32 getSZipFileSize(uint32 index) const { return (*_Data)[index].SevenZipFileSize; } 
private:
	std::vector<CEntry>* _Data;
};


/** The core of the Downloader / installer software
This module send and receive message from the Gui, The torrent module, the ryzom patch module
*/
class CGameDownloader : public IGameDownloader, public IRyzomControlListener, public ITorrentListener
{
public:
	CGameDownloader();
	virtual void getDownloadList();
	//! translate a string
	ucstring getString(const std::string& msg);
	//! update the Label "content" text
	void setLabelContentText( const ucstring& literal);
	//! update the Label "time left" text
	void setLabelTimeLeftText( const ucstring& literal);
	//! update the Label "state" text
	void setLabelStateText( const ucstring& literal);

	virtual bool getMustStartBrowser() const { return _MustStartBrowser; }
	virtual bool getBrowserActive() const { return _BrowserActive; }
	void setMustStartBrowser(bool state) { _MustStartBrowser = state;  askGuiUpdate(); }
	void setBrowserActive(bool state) { _BrowserActive = state; askGuiUpdate();}

	//! update the Label "content" visibility
	void setLabelContentVisible( bool){}
	

	//! called when the lable fullVersion must be updated;
	void setLabelTimeLeftVisible(bool visible);
	
	//! called when the lable state must be updated;
	void setLabelStateVisible(bool visible);

	void setProgressBarVisible( bool){}

	//! will be used to display GUI
	void setProgressBarValue( int progress );
	//! return the Balue of the progress
	virtual sint32 getProgressBarValue() const { return _ProgresBarValue; }
	void displayMsgClientUpdated(bool startClient);


	//! update the title and content text of the install windows
	void setInstallText(const std::string&title, const std::string& content);
	//! called when install text must be updated
	void updateStartProcessText();
	//! ask the gui to display the "do you want to process install" window
	void packageSelectionUpdated(ICkeckInfoViewer* viewer);	
	//! tel the gui that he has to update is label / text
	void askGuiUpdate();
	//! tel the gui to close the application
	void closeApplication();

	virtual ucstring getStateLabelText() const
	{
		return _StateLabel;
	}

	virtual ucstring getContentLabelText() const
	{
		return _ContentLabel;
	}

	virtual ucstring getTimeLeftLabelText() const
	{
		return _TimeLeftLabel;
	}
	
	/** Conevert time in second to string understandable by human (language dependent)
	Returns string to display text "3 mintues 4 seconds"
	*/
	ucstring CGameDownloader::timeToHuman(double t);

	/** Generic function to update progress bar and label of install, download, scan...
	The time estimation is made in function of the rate (after smooth function) and 
	- Main label "download of toto.bnp at 14MB/S (1/4) -( 200MB/1.6GB)"
	- Time estimation label: "(3minutes 15 seconds left)
	- Progressbar
	*/
	void onFileUpdate(const char * trad, const char * filename, uint32 rate, uint32 index, uint32 filecount,  uint32 firstValue, uint32 lastValue, uint64 size, uint64 fullSize, const char * category  );
	//! log a message to the stats.php
	std::string logOnServer(const std::string& str);
	virtual void registerChoosePackageDlg(CChoosePackageDlg* dlg);

	//! returns an aproximation of the install Rate (to add the install time while the download)
	double getInstallRate() const { return _InstallRate;}
	virtual void setInstallRate(double rate) { _InstallRate = rate; }
	virtual bool getUpdateDownloadIcon() const { return _UpdateDownloadIcone;}


	//----------------- IGameDownloader --------------------
	//! \see IRyzomControlListener::doAction
	virtual void doAction(const char* actionName, bool state);
	//! \see IGameDownloader::update
	virtual void update() ;	
	//! \see IGameDownloader::getPackageSelection
	virtual void getPackageSelection(bool mustRepair);
	//! \see IGameDownloader::setForm
	//! \see IGameDownloader::getState
	virtual EState getState() const { return _State; }
	//! \see IGameDownloader::setState
	virtual void setState(EState state)
	{ 
		if (_State != InstallFinished){	_State = state; }
	}
	//! \see IGameDownloader::updateLauncher
	virtual bool updateLauncher();
	//! \see IGameDownloader::updateTorrent
	virtual bool updateTorrent();
	//! \see IGameDownloader::setTorrentInfo
	virtual void setTorrentInfo(const std::string & torrentPatchFilename,
		const std::string & torrentFilename,
		uint32  torrentPatchTimestamp);
	//! \see IGameDownloader::setInstallInfo
	virtual void setInstallInfo(const std::string & installPatchFilename,
		const std::string & installFilename,
		const std::string & installPatchExtract, uint32 installPatchTimestamp) ;
	//! \see IGameDownloader::setState
	void setState (bool bOutputToLog, const char* newState);
	//! \see IGameDownloader::getTmpInstallDirectory
	virtual const char *getTmpInstallDirectory(void) const { return _InstallDir.c_str(); }
	//! \see IGameDownloader::setTorrentExist
	virtual void setTorrentExist(bool exist)  {	_TorrentExist = exist; }
	//! \see IGameDownloader::getTorrentExist
	virtual bool getTorrentExist() const {	return _TorrentExist; }
	//! \see IGameDownloader::setAutoInstall
	virtual void setAutoInstall(bool autoInstall) {	_AutoInstall = autoInstall; }
	//! \see IGameDownloader::getAutoInstall
	virtual bool getAutoInstall() const { return _AutoInstall; }
	//! \see IGameDownloader::getStarted
	virtual bool getStarted() const { return _Started; }
	//! \see IGameDownloader::setStarted
	virtual void setStarted(bool started) { _Started = started; }
	//! \see IGameDownloader::getLanguage
	virtual std::string getLanguage() const { return std::string(_RyzomControl->getLanguage()); }
	//! \see IGameDownloader::getPatchUrl
	virtual std::string getPatchUrl() const {  return std::string(_RyzomControl->getPatchUrl());}
	virtual void resetCfgAndStringCache() {_RyzomControl->resetCfgAndStringCache(); } 
	//! \see IGameDownloader::getInstallHttpBackground
	virtual std::string getInstallHttpBackground() const;
	
	//! \see IGameDownloader::getLogUrl
	virtual std::string getLogUrl(const std::string& action) const ;
	//! \see IGameDownloader::setFirstInstall
	virtual void setFirstInstall(bool firstInstall) { _FirstTimeInstall = firstInstall; }
	//! \see IGameDownloader::getFirstInstall
	virtual bool getFirstInstall() const { return _FirstTimeInstall; }
	//! \see IGameDownloader::getRosInstallSize
	virtual  uint64 getRosInstallSize() const {	return _RosInstallSize;	}
	//! \see IGameDownloader::getRyzomInstallSize
	virtual uint64 getRyzomInstallSize() const {return _RyzomInstallSize; }
	//! \see IGameDownloader::getRosPatchSize
	virtual  uint64 getRosPatchSize() const { return _RosPatchSize;	}
	//! \see IGameDownloader::getRyzomPatchSize
	virtual uint64 getRyzomPatchSize() const { return _RyzomPatchSize; }
	//! \see IGameDownloader::getStartRyzomAtEnd
	virtual bool getStartRyzomAtEnd() const { return _StartRyzomAtEnd; }
	//! \see IGameDownloader::setStartRyzomAtEnd
	virtual void setStartRyzomAtEnd(bool ok);
	//! \see IGameDownloader::setSessionId
	virtual void setSessionId(const std::string& sessionId) { _SessionId = sessionId; }
	//! \see IGameDownloader::setInstallId
	virtual void setInstallId(const std::string& installId) { _InstallId = installId; }
	//! \see IGameDownloader::setUserId
	virtual void setUserId(const std::string& userId) {	_UserId = userId; }
	//! \see IGameDownloader::setDownloadOnly
	virtual void setDownloadOnly(bool downloadOnly) {_DownloadOnly = downloadOnly; }
	//! \see IGameDownloader::getDownloadOnly
	virtual bool getDownloadOnly() const { return _DownloadOnly; }
	//! \see IGameDownloader::setUseFullVersion
	virtual void setUseFullVersion(bool fullVersion) { _UseFullVersion = fullVersion; }
	//! \see IGameDownloader::getUseFullVersion
	virtual bool getUseFullVersion() const { return _UseFullVersion; }
	//! \see IGameDownloader::getUseTorrent
	virtual bool getUseTorrent() const {return _UseTorrent && getTorrentExist(); }
	//! \see IGameDownloader::setInstalTaskDlg
	virtual void setInstallTaskDlg(CInstallTaskDlg* dlg);

	//----------------- ITorrentListener --------------------
	//! \see ITorrentListener::progressInfo
	virtual void progressInfo(const char* msg);
	//! \see ITorrentListener::torrentFinished
	virtual void torrentFinished();
	//! \see ITorrentListener::getTorrentTmpDir
	virtual const char * getTorrentTmpDir() const { return _InstallDir.c_str(); }
	
	//----------------- IRyzomControlListener --------------------
	//! \see IRyzomControlListener::onCheckFinished
	virtual void onCheckFinished(ICkeckInfoViewer* viewer);
	//! \see IRyzomControlListener::onScanDataFinished
	virtual void onScanDataFinished();
	//! \see IRyzomControlListener::onDownloadListFinished
	virtual void onDownloadListFinished();
	//! \see IRyzomControlListener::addToDownloadList
	virtual void addToDownloadList(const char* patchName, const char* sourceName, uint32 timestamp, const char* extractPath, uint32 sZipFileSize, uint32 size);
	//! \see IRyzomControlListener::selectPackage
	virtual void selectPackage(bool mainland);
	//! \see IRyzomControlListener::setUseTorrent
	virtual void setUseTorrent(bool useTorrent);
	//! \see IRyzomControlListener::setVersionInfo
	virtual void setVersionInfo(const std::vector<std::string>& patchUris, const std::string& serverPath) ;
	//! \see IRyzomControlListener::onFileInstallFinished
	virtual void onFileInstallFinished();	
	//! \see IRyzomControlListener::onFileDownloading
	virtual void onFileDownloading(const char* sourceName, uint32  rate, uint32  fileIndex, uint32  fileCount, uint64  fileSize, uint64 fullSize);	
	//! \see IRyzomControlListener::onFileInstalling
	virtual void onFileInstalling(const char* sourceName, uint32  rate, uint32  fileIndex, uint32  fileCount, uint64  fileSize, uint64 fullSize);		
	//! \see IRyzomControlListener::onFileDownloadFinished
	virtual void onFileDownloadFinished();
	//! \see IRyzomControlListener::getVersion
	virtual const char * getVersion() const { return _Version.c_str();; }
	//! \see IRyzomControlListener::getPatchUriCount
	virtual uint32 getPatchUriCount() const { return _PatchUris.size(); }
	//! \see IRyzomControlListener::getPatchUriValue
	virtual const char * getPatchUriValue(uint32 index) const { return _PatchUris[index].c_str(); }
	//! \see IRyzomControlListener::getServerPath
	virtual const char * getServerPath() const { return _ServerPath.c_str(); }
	//! \see IRyzomControlListener::getLauncherName
	virtual  const char * getLauncherName() const { return "install.bat"; }
	//! \see IRyzomControlListener::requestStop
	virtual void requestStop();
	//! \see IRyzomControlListener::initI18n
	virtual void initI18n(const char* lang, unsigned char *data, uint32 length);
	//! \see IRyzomControlListener::updateFuntion
	void updateFuntion(TUpdateType updateType, uint32 index, uint32 max);
	//! \see IRyzomControlListener::fatalError
	virtual void fatalError(const char* msg, const char* param1, const char* param2);
	


private:
	IRyzomControl* _RyzomControl; //!< Control the ryzom patch protocol
	ITorrentControl* _TorrentControl; //!< Control the torrent patch protocol
//	gcroot<client_install::CClientInstallForm ^> _Form; //!< The Gui form
	std::vector<std::string> _ToDownloadCategory; //!< The list of category of file to download
	EState _State; //!< The current state of the instance 
	std::vector<CEntry> _Entries; //! tbe list of entries in the Download / Install list
	bool _Mainland; //!< Use full version or only download Ruine of Silan
	std::string _Version;//!< The version used "r2", "head"
	std::vector<std::string> _PatchUris; //The uri of the pach server
	std::string _ServerPath; //!< The backup url of the patch server
	std::string _TmpDir; //!< The tempory directory "patch/client_install"
	std::string _InstallDir; // The install directory 
	std::string _PatchRootDir; //!< The patch root directory "/patch"
	std::string _TorrentPatchFilename; //!< The torrent patchfilename eg "version_01.torrent.lzma"
	std::string _TorrentFilename;//!< The name of the torrent version_01.torrent
	uint32 _TorrentPatchTimestamp;//!< The timestamp of the torrent file
	std::string _InstallPatchFilename; //!< obsolete
	std::string _InstallFilename;//!< obsolete
	uint32  _InstallPatchTimestamp;//!< obsolete
	std::string _InstallPatchExtract;//!< obsolete
	bool _UseTorrent; //!< Is torrent protocol used
	bool _TorrentExist; //!< Torrent file exist on server
	bool _FirstTimeInstall; //!< is it the first install (or repair)
	bool _StartRyzomAtEnd; //!< Must strat ryzom at end of install
	uint64 _RosPatchSize; //!< Size of the patch when not in full version
	uint64 _RyzomPatchSize; //!< Size of the patch in full version
	uint64 _RosInstallSize; //!< Size of data use on dique after depack on initial version
	uint64 _RyzomInstallSize; //!< Size of data use on dique after depack on full version
	uint64 	_PreviousDownloaded; //!< Size of data downloaded on previous session
	std::string _SessionId; //!< number of session (given by stats.php)
	std::string _InstallId; //!< install id (given by registy)
	std::string _UserId; //!<user id (given by stats.php)
	bool _AutoInstall; //!< obsolete
	bool _Started;
	bool _DownloadOnly; //!< obsolete
	bool _UseFullVersion; //same as _Mainland but only for GUI
	CInstallTaskDlg* _InstallTaskDlg; //! MFC Windows
	ucstring _ContentLabel; //!must be display into the MFC Window
	ucstring _StateLabel; //!< must be display into the MFC Window
	ucstring _TimeLeftLabel; //!< must be display into the MFC Window
	sint32 _ProgresBarValue;
	NLMISC::CFairMutex _WaitForInstallLock;//!< block the install thread to the end of the install finished windows	
	CChoosePackageDlg* _ChoosePackageDlg;//!< MFC Windows of the "Choose package windows" (where we select "use torrent" and "full version")
	double _InstallRate;
	bool _MustStartBrowser;
	bool _BrowserActive;
	bool _UpdateDownloadIcone; //!< if true the turning arrow icon  must continue to turn
	
};


CGameDownloader::CGameDownloader()
{	
	setState(Init);
	_Mainland = false;
	_InstallDir = "patch/client_install/"; 
	
	_TmpDir = _InstallDir + std::string( "tmp/");
	_PatchRootDir = "patch/"; 
	_UseTorrent = true;
	_InstallPatchTimestamp = 0;
	_TorrentPatchTimestamp = 0;
	_TorrentExist = false;
	_FirstTimeInstall  = !NLMISC::CFile::fileExists("unpack/exedll.bnp");


	_InstallRate = 7000000.0;

	_UpdateDownloadIcone = true;

	

	_RosPatchSize = 0;
	_RyzomPatchSize = 0;
	
	_RosInstallSize = 0;
	_RyzomInstallSize = 0;
	_PreviousDownloaded = 0;
	_SessionId="0";
	_AutoInstall = false;
	_Started = false;
	_DownloadOnly = false;
	_UseFullVersion = false;
	_InstallTaskDlg = 0;
	_BrowserActive = false;
	_MustStartBrowser = false;
	_StartRyzomAtEnd = true;

	_RyzomControl =  IRyzomControl::init(this);
	_TorrentControl = ITorrentControl::getInstance();
	if (_TorrentControl) { _TorrentControl->setListener(this); }

	_WaitForInstallLock.enter();

}


void CGameDownloader::setVersionInfo(const std::vector<std::string>& patchUris, const std::string& serverPath)
{
	_PatchUris = patchUris;
	_ServerPath = serverPath;
	// save Version info
	std::string versionInfo = std::string(_RyzomControl->getApplication()) + ".version";
	std::string finalPath = _TmpDir + versionInfo;
	
	nldebug("Looking for version specific Web site url");
	nldebug("Looking for default Web site url");
	_RyzomControl->initPatchManager(); //update ServerVersion
	this->setMustStartBrowser(true);

	nldebug("Creating default directories %s %s if not exist",_InstallDir.c_str(), _TmpDir.c_str() );
	if ( ! NLMISC::CFile::isDirectory(_InstallDir) )
	{
		NLMISC::CFile::createDirectoryTree(_InstallDir);
	}

	if ( ! NLMISC::CFile::isDirectory(_TmpDir) )
	{
		NLMISC::CFile::createDirectoryTree(_TmpDir);
	}


	// save version 
	{
		std::string version = _InstallDir + "version.txt";
		nldebug("Saving '%s'",version.c_str());
		FILE*  pFile = fopen(version.c_str(), "wb");
		fprintf(pFile, "%s", versionInfo.c_str() );
		fclose(pFile);
	}
	
	_Version = _RyzomControl->getServerVersion();
	
	nldebug("Server Version is '%s' '%s'",_RyzomControl->getApplication(), _Version.c_str() );
	{
		uint32 versionId = atoi(_Version.c_str());
		char bufferStr [1024];
		sprintf(bufferStr, "%05d/ryzom_%05d.torrent", versionId, versionId);
		_TorrentPatchFilename = bufferStr;
		sprintf(bufferStr, "%spatch/%05d/ryzom_%05d.torrent",_InstallDir.c_str(), versionId, versionId);
		_TorrentFilename = bufferStr;	
	}

}

void CGameDownloader::onScanDataFinished()
{

}

void CGameDownloader::onCheckFinished(ICkeckInfoViewer* viewer)
{

	nldebug("Check Thread has finished");
	_ToDownloadCategory.clear();
	// Add all optional category to Category to download 
	{
		uint32 i = 0;
		uint32 count = viewer->getCount(ICkeckInfoViewer::OptCat);
		nldebug("Optional categories available %d", count);
		for (; i != count; ++i)
		{			
			_ToDownloadCategory.push_back(std::string(viewer->getName(ICkeckInfoViewer::OptCat, i) ) );
			nldebug("Optional categories %d %s", i,  _ToDownloadCategory[i].c_str());
		}
		
	}
	


	/* Calculate patch and instal size
	Ros size is only  Non optional Category
	Ryzom size is the sum of Non optional Category, Optional Category and Required
	*/
	_RosPatchSize = 0;
	_RosInstallSize = 0;
	// Calculate patch and instal size of Non optional Category
	{
		uint32 i = 0;
		uint32 count = viewer->getCount(ICkeckInfoViewer::NonOptCat);
		for (; i != count; ++i)
		{
			const char * str = viewer->getName(ICkeckInfoViewer::NonOptCat, i);
			_RosPatchSize+= viewer->getSZipFileSize(ICkeckInfoViewer::NonOptCat, i);
			_RosInstallSize += viewer->getFileSize(ICkeckInfoViewer::NonOptCat, i);
		}
	}


	_RyzomPatchSize = _RosPatchSize;
	_RyzomInstallSize = _RosInstallSize;
	// Calculate patch and instal size of optional Category
	{
		uint32 i = 0;
		uint32 count = viewer->getCount(ICkeckInfoViewer::OptCat);
		for (; i != count; ++i)
		{
			_RyzomPatchSize+= viewer->getSZipFileSize(ICkeckInfoViewer::OptCat, i);
			_RyzomInstallSize+= viewer->getFileSize(ICkeckInfoViewer::OptCat, i);
		}
	}
	// Calculate patch and install size of ReqCat Category
	{
		uint32 i = 0;
		uint32 count = viewer->getCount(ICkeckInfoViewer::ReqCat);
		for (; i != count; ++i)
		{
			_RyzomPatchSize += viewer->getSZipFileSize(ICkeckInfoViewer::ReqCat, i);
			_RyzomInstallSize +=  viewer->getFileSize(ICkeckInfoViewer::ReqCat, i);
		}
	}
	std::string str;
	str = sizeToHumanStd(_RosPatchSize);
	nldebug("Ros Patch Size %s", str.c_str());
	str = sizeToHumanStd(_RosInstallSize);
	nldebug("Ros Install Size %s", str.c_str());
	str = sizeToHumanStd(_RyzomPatchSize);
	nldebug("Ryzom Patch Size %s", str.c_str());
	str = sizeToHumanStd(_RyzomInstallSize);
	nldebug("Ryzom Install Size %s", str.c_str());
	
	// if there is no category to download then the client is up to date display the message
	if (viewer->getCount(ICkeckInfoViewer::OptCat) == 0 && viewer->getCount(ICkeckInfoViewer::NonOptCat ) == 0 && viewer->getCount(ICkeckInfoViewer::ReqCat ) == 0 )
	{
		nldebug("No file to download so display the end of install msg");
		setState(IGameDownloader::InstallFinishing);		
		displayMsgClientUpdated(true);		
		return;
	}
	nldebug("Optional file to patch %d", viewer->getCount(ICkeckInfoViewer::OptCat));
	nldebug("Non Optional file to patch %d", viewer->getCount(ICkeckInfoViewer::NonOptCat)) ;
	nldebug("Required file to patch %d", viewer->getCount(ICkeckInfoViewer::ReqCat ));

	setState(CheckFinished);
	// display message you're about to downlaod ryzom ...
	packageSelectionUpdated(viewer);	
}



void CGameDownloader::onDownloadListFinished()
{

	// compute the size of date of file downloaded in the previous sessions
	uint32 first(0), last(_Entries.size());
	for (; first != last; ++first)
	{
		std::string tmpName = this->getTorrentTmpDir() + _PatchRootDir + _Entries[first].PatchName;	
		if (NLMISC::CFile::fileExists(tmpName))
		{
			_PreviousDownloaded += NLMISC::CFile::getFileSize(tmpName);
		}

	}

	// As sone as the list of file to download is given lauch the download process
	//_Form->onDownloadListFinished();


	

	// Wait that the message is realy send
	std::string res = logOnServer("start_download");
	setSessionId( extractToken(res, "session_id"));


	if (getUseTorrent())
	{	//torrent protocol
		CTorrentStringContainer container(&_Entries, this->getTorrentTmpDir() + _PatchRootDir);
		_TorrentControl->startDownloading(_TorrentFilename.c_str(),  &container);
	}
	else
	{	//ryzom patch protocol
		CInstallEntryViewer viewer(&_Entries);
		_RyzomControl->install(&viewer, true); // download
	}
	setState(Downloading);
}


void CGameDownloader::addToDownloadList(const char* patchName, const char* sourceName, uint32 timestamp, const char* extractPath, uint32 sevenZipFileSize, uint32 size)
{
	//creating the list of file to download
	std::string newPatchName =  patchName;
	_Entries.push_back( CEntry(newPatchName.c_str(), sourceName, timestamp, extractPath, sevenZipFileSize, size) );		
}


void CGameDownloader::update()
{
	if (_TorrentControl){ _TorrentControl->update(); }
	_RyzomControl->update();

	static IGameDownloader::EState lastState = IGameDownloader::Init;
	if (lastState != this->getState())
	{
		static const char * trad[] = {"Init", "InitFinished", "UpdateVersion", "UpdateVersionFinished", "UpdateLauncher", "UpdateLauncherFinished", "UpdateTorrent", "UpdateTorrentFinished",
		"UpdatePackage", "UpdatePackageFinished", 
		"CheckFinished", "RepositoryUpdated",  "Downloading",  "DownloadFinished", "AskInstalling", "InstallOk", "Installing", "AskInstallFinishing", "InstallFinishing", "InstallFinished"};
	
		nldebug("Current State: %s", trad[this->getState()] );
		lastState = this->getState();
	}
	


	if (_State == Downloading)
	{
		if (getUseTorrent())
		{
			// In case of torrent downloading we have to pull to have info on donload stat
			if ( _TorrentControl->isCurrentDownloadValid())
			{
				onFileUpdate(
					_TorrentControl->getCurrentDownloadStateLabel(),
					_TorrentControl->getCurrentDownloadName(), 
					static_cast<uint32>(_TorrentControl->getCurrentDownloadDownloadRate()), 
					0, 
					0,  
					0, 
					50, 
					_TorrentControl->getCurrentDownloadTotalDone(), 
					_TorrentControl->getCurrentDownloadTotalWanted(),
					"uiBGD_CategoryDownload"
				);
				return;
			}
		}
		else
		{
			//information comme from RyzomControl->update()
		}
	}

	if (_State == DownloadFinished)
	{
		setState(AskInstalling);
		/*
		uint32 first = 0, last= _Entries.size();
		uint64 fullSize = 0;
		for ( ; first != last; ++first)
		{
			fullSize += _Entries[first].Size;
		}
		previously was displaying "Do you want to install"
		*/
		setProgressBarValue(50);	
		logOnServer("stop_download");		
		doAction("install_continue", true);
		logOnServer("start_install");

		return;
	}

	if (_State == InstallOk)
	{
		nldebug("Will proced to the installation of %d files", _Entries.size()); 
		_State = Installing;
		CInstallEntryViewer viewer(&_Entries);
		_RyzomControl->install(&viewer, false);
		return;
	}

	if (getState() == IGameDownloader::InitFinished)
	{				
		setState(IGameDownloader::UpdateVersion);
		std::string adresse = getPatchUrl();
		std::string serverPathStr;
		std::vector<std::string> patchUris;
	
		nldebug("Try to get Patch Url from '%s'", adresse.c_str() );
		try
		{
			std::string::size_type it = adresse.find("/", strlen("http://"));
			std::string server = adresse.substr(0, it);
					
			std::string option;
			std::string page;
			std::string::size_type optionIt = adresse.find("?");		
			if (optionIt != std::string::npos)
			{			
				page = adresse.substr(0, optionIt);
				option = adresse.substr(optionIt+1);
			}
			else
			{
				page = adresse;
				option = "";
			}

			// Ask web page from php server to have starting url		
			CHttpClient httpClient;	
			if (!httpClient.connect(server))
			{
				throw std::exception("Can't connect to http server");
			}
			if (!httpClient.sendPost(page, option))
			{
				throw std::exception("Post failed");
			}
			std::string ret;
			if (!httpClient.receive(ret))
			{			
				throw std::exception("Receive failed");
			}
		
		
			static std::string contentSep="<version";
			std::string::size_type posIt = ret.find(contentSep);
			// cut unnecessary HTML header
			if (posIt != std::string::npos)
			{
				ret = ret.substr(posIt);
			}
			httpClient.disconnect();
			nldebug("Parsing Infos provided by '%s'", adresse.c_str() );

			NLMISC::CMemStream	stream;
			// answer will looks like
			/*
			ret = "<version serverPath=\"\\\\dailyclient\\nevrax\\output\\patch\">"
				"	<patchURI>test1</patchURI>"
				"	<patchURI>test2</patchURI>"
				"</version>";
			*/

		

			stream.serialBuffer((uint8*)&ret[0], ret.length());
			stream.invert();
			NLMISC::CIXml read;
			
		
			read.init(stream);
		


			xmlNodePtr version  = read.getRootNode();
			if (version)
			{
				std::string versionName = (const char*)version->name;		
				xmlNodePtr curSon = version;
			
				if (versionName == "version")
				{
						// setup node description
					CXMLAutoPtr serverPath(xmlGetProp (version, (xmlChar*)"serverPath"));
					if (bool(serverPath))
					{
						serverPathStr = (const char*)serverPath;
					}
	
					curSon = curSon->children;			
					while (curSon)
					{				
						// extract inner text of child of element version (element patchURI)
						std::string curSonStr = (const char *) curSon->name;
						if (curSonStr == "patchURI")
						{
							const char* text = (const char*)curSon->children->content;
							if (text)
							{
								patchUris.push_back(text);
							}
						}
						curSon = curSon->next;
					}			
				}	
			}
		}
		catch(...)
		{
			fatalError("uiBGD_LoadVersionError", "", "");
			return;
		}

		nldebug("Backup url is '%s'. There are %d other urls", serverPathStr.c_str(), patchUris.size() );
		uint32 i= 0;
		for (i=0; i < patchUris.size() ; ++i)
		{
			nldebug("Patch Url %d '%s'", i, patchUris[i].c_str());
		}

		if (patchUris.empty()  && serverPathStr == "")
		{
			fatalError("uiBGD_LoadVersionError", "", "");
			return;
		}
		// update core dpwnload  patch url
		setVersionInfo( patchUris, serverPathStr);		

		// go to next state
		setState(IGameDownloader::UpdateVersionFinished);
		return;
	}


	if (this->getState() == IGameDownloader::UpdateVersionFinished)
	{	//obsolete code to update patch by depacking a bnp that could contain himself
		this->setState(IGameDownloader::UpdateLauncher);		
		if (!this->updateLauncher())
		{  // not used
			this->setState(IGameDownloader::UpdateLauncherFinished);		
			this->setLabelContentText( getString("uiBGD_Restarting"));
		}
		else
		{	// alway used
			this->setState(IGameDownloader::InstallFinished);			
			this->setLabelContentText( getString("uiBGD_LoadTorrent"));
		}
		return;
	}

	if (this->getState() == IGameDownloader::UpdateLauncherFinished)
	{	
		nldebug("Looking if torrent file '%s' exist in order to allow P2P protocol", _TorrentFilename.c_str());
		// Look if torrent file exist on server
		this->setState(IGameDownloader::UpdateTorrent);				
		if (! this->updateTorrent() )
		{
			this->setTorrentExist(false);
		}
		else
		{
			this->setTorrentExist(true);
		}

		//go next state
		
		this->setLabelContentText(  getString("uiBGD_LoadPackageList"));
		this->setState(IGameDownloader::UpdateTorrentFinished);		
		return;
	}

	if (this->getState() == IGameDownloader::UpdateTorrentFinished)
	{	// start the loading of different package
		// if _NeedRepair (client already downloaded) a MD5 check would be added
		this->setState(IGameDownloader::UpdatePackage);
		if (_FirstTimeInstall)
		{
			nldebug("Install mode: do check data");
		}
		else
		{
			nldebug("Repair mode: do scan data");
		}
		this->getPackageSelection(!_FirstTimeInstall);		
		this->setLabelContentVisible(true);
		this->setProgressBarValue(0);
		this->setProgressBarVisible( true);
		return;
	}

	if (this->getState() == IGameDownloader::AskInstallFinishing)
	{
		// automatic after download
		this->setState(IGameDownloader::InstallFinishing);
		logOnServer("stop_install");
		this->displayMsgClientUpdated(false);		
		return;
	}
	if (this->getState() == IGameDownloader::InstallFinished)
	{	
		static NLMISC::TTime first =0;
		
		if (first == 0)
		{	
			 first = NLMISC::CTime::getLocalTime();

		}
		// Wait 5 second
		if ((NLMISC::CTime::getLocalTime() - first) > 5000 )
		{
			closeApplication();
		}
		return;
	}

	if (this->getState() == IGameDownloader::Downloading)
	{	
		static NLMISC::TTime first =0;
		if (first == 0)
		{	
			 first = NLMISC::CTime::getLocalTime();

		}
		// Wait 10 second
		if ((NLMISC::CTime::getLocalTime() - first) > 10000 )
		{
			int value = _ProgresBarValue * 2;
			logOnServer(NLMISC::toString("update_download&percent=%d", value));
			first = NLMISC::CTime::getLocalTime();
		}


		return;
	}
	if (this->getState() == IGameDownloader::Installing)
	{
		static NLMISC::TTime first =0;
		if (first == 0)
		{	
			 first = NLMISC::CTime::getLocalTime();

		}
		// Wait 10 second
		if ((NLMISC::CTime::getLocalTime() - first) > 10000 )
		{
			int value = (_ProgresBarValue -50) * 2;
			logOnServer(NLMISC::toString("update_install&percent=%d", value));
			first = NLMISC::CTime::getLocalTime();
		}
		return;
	}
}

void CGameDownloader::getPackageSelection(bool mustRepair)
{
	//forward to RyzomControl that will call the onCheckFinished
	_RyzomControl->getPackageSelection(mustRepair );
}

void CGameDownloader::torrentFinished()
{
	_State = DownloadFinished;
}

void CGameDownloader::getDownloadList()
{
	// Do not download optional categories if mainland
	if (!_Mainland)
	{
		_ToDownloadCategory.clear();	
	}
	CStringContainer container(&_ToDownloadCategory);
	_RyzomControl->getDownloadList(&container);
}

void CGameDownloader::setUseTorrent(bool useTorrent) 
{
	_UseTorrent = useTorrent;
}

void CGameDownloader::selectPackage(bool mainland) 
{
	_Mainland = mainland;
}

void CGameDownloader::progressInfo(const char* msg)
{
	//System::Diagnostics::Debug::WriteLine(gcnew System::String(msg));
}

void CGameDownloader::onFileDownloading(const char* sourceName, uint32  rate, uint32  fileIndex, uint32  fileCount, uint64  fileSize, uint64 fullSize)
{	// from ryzom protocol to gui
	if (fullSize == 0)
	{
		fullSize = 100;
		fileSize = 0;
	}
	uint32 max = 50;
	uint32 index = uint32( double(max) * fileSize / fullSize);
	//display the download bar from 0 to 50 percent

	onFileUpdate("uiBGD_Download", sourceName, rate, fileIndex, fileCount, 0, 50, fileSize, fullSize, "uiBGD_CategoryDownload");
}


void CGameDownloader::onFileInstalling(const char* sourceName, uint32  rate, uint32  fileIndex, uint32  fileCount, uint64  fileSize, uint64 fullSize)
{	// from ryzom protocol to gui
	if (fullSize == 0)
	{
		fullSize = 100;
		fileSize = 0;
	}
	//display the isntall  bar from 50 to 100 percent
	uint32 max = 50;
	uint32 index = 50 + uint32( double(max) * fileSize / fullSize);
	
	onFileUpdate("uiBGD_Installing", sourceName, rate, fileIndex, fileCount, 50, 100, fileSize, fullSize, "uiBGD_CategoryInstall");
}

void CGameDownloader::onFileDownloadFinished()
{
	_State = DownloadFinished;
}


void CGameDownloader::onFileInstallFinished()
{
	// will display the windows at the next update
	setState (AskInstallFinishing);
	//Wait for the install Windows
	nldebug("Wait for the gui validation before laucnching the install batch (if necessary)");
	
	_WaitForInstallLock.enter();
	//ok we can leave no
	_WaitForInstallLock.leave();


}

void CGameDownloader::requestStop()
{
	_State = InstallFinished;
}

bool CGameDownloader::updateLauncher()
{
	// deactivated (code for askin the installer to update himself)
	if (_InstallPatchFilename.empty() || _InstallFilename.empty() || _InstallPatchTimestamp == 0)
	{
		return false;
	}
	if ( _RyzomControl->download(_InstallPatchFilename.c_str(), _InstallFilename.c_str(), _TmpDir.c_str(),  _InstallPatchTimestamp) )
	{
		char* install[1];
		char* extract[1];
		install[0] = (char*)_InstallPatchFilename.c_str();
		extract[0] = (char*)_InstallPatchExtract.c_str();

		if ( _RyzomControl->extract(&install[0], &extract[0], 1) )
		{
			return true;
		}
	}
	return false;
}

bool  CGameDownloader::updateTorrent()
{
	//deactivated
	return _RyzomControl->download(_TorrentPatchFilename.c_str(), _TorrentFilename.c_str(), _TmpDir.c_str(),  _TorrentPatchTimestamp);
}

void CGameDownloader::initI18n(const char* lang, unsigned char *data, uint32 length)
{
	_RyzomControl->initI18n(lang, data, length);
}

void CGameDownloader::setState (bool bOutputToLog, const char* newState)
{
	//display ryzom log info in display form if bOutputToLog is true
	std::string str(newState);
	// do not display warniing for .torrent file because its not a bug if the torrent is not on server
	if (str.find("cannot copy file") != std::string::npos && str.find(".torrent.tmp")) { return; }

	if (bOutputToLog)
	{
		ucstring str;
		str.fromUtf8(newState);
		this->setLabelStateText(str);		
	}
}


void CGameDownloader::updateFuntion(IRyzomControlListener::TUpdateType updateType, uint32 index, uint32 max)
{
	// from ryzom module to gui
	switch (updateType)
	{
		case IRyzomControlListener::ScanData: onFileUpdate("uiBGD_ScanData", 0, 0, index, max, 0, 100, 0, 0, 0); break;
		case IRyzomControlListener::CheckData: onFileUpdate("uiBGD_CheckData", 0, 0, index, max, 0, 100, 0, 0, 0); break;
		case IRyzomControlListener::Patch: onFileUpdate("uiBGD_Patch", 0, 0, index, max, 0, 0, 0, 0, 0); break;
		default:;
	}
}

// obosolete
void CGameDownloader::setTorrentInfo(const std::string & torrentPatchFilename,
	const std::string & torrentFilename,
	uint32  torrentPatchTimestamp)
{
	_TorrentPatchFilename = torrentPatchFilename;
	_TorrentFilename = torrentFilename;
	_TorrentPatchTimestamp = torrentPatchTimestamp;
}

// obosolete
void CGameDownloader::setInstallInfo(const std::string & installPatchFilename,
	const std::string & installFilename,
	const std::string & installPatchExtract, uint32 installPatchTimestamp) 
{
	_InstallPatchFilename = installPatchFilename;
	_InstallFilename = installFilename;
	_InstallPatchTimestamp = installPatchTimestamp;
	_InstallPatchExtract = installPatchExtract;	
}

//create url to ask stats.php infos
std::string CGameDownloader::getLogUrl(const std::string& action) const  
{
	std::string copy (action);
	
	
	if (action.find("login") != std::string::npos )
	{			
		copy += std::string("&install_id=") + _InstallId;
	}
	else if (action.find("init") != std::string::npos )
	{			
		//obsolete made by login progress thread		
		copy += std::string("&install_id=") + _InstallId;
		copy += std::string("&os=") + std::string(INelControl::getOS ());
		copy += std::string("&proc=") + std::string(INelControl::getProc ());
		//use totalPhysicalMemory instread
		copy += std::string("&memory=") + std::string(sizeToHumanStd(INelControl::availablePhysicalMemory ()));
		//copy += std::string("&hd=") + std::string(INelControl::availableHDSpace ());
		copy += std::string("&video_card=") + std::string(INelControl::getVideoInfoDeviceName ());
		copy += std::string("&driver_version=") + std::string(uint64ToHex(INelControl::driverVersion ()));			
	}
	else
	{


		if (action.find("start_download") != std::string::npos)
		{
			uint64 installSize = _Mainland ? getRyzomInstallSize() : getRosInstallSize();
			uint64  downloadSize = _Mainland ? getRyzomPatchSize() : getRosPatchSize();			
			copy += std::string("&server=") + std::string( _RyzomControl->getStartupHost() );
			copy += std::string("&application=") + std::string( _RyzomControl->getApplication() );
			copy += std::string("&version=") + std::string( _RyzomControl->getServerVersion() );	
			copy += std::string("&lang=") + std::string( _RyzomControl->getLanguage() );						
			copy += std::string("&type=") + (_FirstTimeInstall?std::string("install"):std::string("repair"));
			copy += std::string("&package=") + ((_Mainland)?std::string("full"):std::string("ros"));
			copy += std::string("&protocol=") + ((getUseTorrent())?std::string("torrent"):std::string("patch"));
			copy += std::string("&size_download=") + sizeToHumanStd(downloadSize);
			copy += std::string("&size_download=") + sizeToHumanStd(downloadSize);
			copy += std::string("&size_install=") + sizeToHumanStd(installSize);			
			copy += std::string("&previous_download=") + sizeToHumanStd(_PreviousDownloaded);			
		}

		else
		{
				copy += std::string("&session_id=") + _SessionId;			
		}
	//	copy += std::string("&user_id=") + _UserId;	//made by login progress thread		
	}


	return std::string(_RyzomControl->getLogUrl(copy.c_str()));
}

void CGameDownloader::setStartRyzomAtEnd(bool ok) 
{
	_StartRyzomAtEnd = ok;
	_RyzomControl->setStartRyzomAtEnd(ok);
}




void CGameDownloader::askGuiUpdate()
{
	if (_InstallTaskDlg)
	{
		_InstallTaskDlg->askGuiUpdate();
	}
}

void CGameDownloader::setProgressBarValue( int progress )
{
	
	bool mustUpdate = (_ProgresBarValue != progress);
	_ProgresBarValue = progress; 
	if (mustUpdate) {  askGuiUpdate(); }	

}

void CGameDownloader::setLabelContentText( const ucstring& literal)
{

	bool mustUpdate = (_ContentLabel != literal);
	_ContentLabel = literal;	
	if (mustUpdate) {  askGuiUpdate(); }	
}

void CGameDownloader::setLabelStateText( const ucstring& literal)
{
	bool mustUpdate = (_StateLabel != literal);
	_StateLabel = literal;
	if (mustUpdate) {  askGuiUpdate(); }	
}

void CGameDownloader::setLabelTimeLeftText( const ucstring& literal)
{
	bool mustUpdate = (_TimeLeftLabel != literal);
	_TimeLeftLabel = literal;
	if (mustUpdate) {  askGuiUpdate(); }	
}



void CGameDownloader::setInstallText(const std::string&title, const std::string& content)
{

}

ucstring empty;

ucstring format(const ucstring& format, const ucstring& arg1=empty, const ucstring& arg2=empty, const ucstring& arg3=empty, const ucstring& arg4=empty, const ucstring& arg5=empty,const ucstring& arg6=empty)
{
	ucstring ret = format;
	NLMISC::strFindReplace(ret, "{0}", arg1);
	NLMISC::strFindReplace(ret, "{1}", arg2);
	NLMISC::strFindReplace(ret, "{2}", arg3);
	NLMISC::strFindReplace(ret, "{3}", arg4);
	NLMISC::strFindReplace(ret, "{4}", arg5);
	NLMISC::strFindReplace(ret, "{5}", arg5);
	NLMISC::strFindReplace(ret, "{6}", arg6);
	return ret;
}


void CGameDownloader::updateStartProcessText()
{
		// update the Start download install text in function of the mode (install, repaire), the version(full, normal), the protocol(torrent, normal)
	ucstring title = "";
	ucstring content = "";
	
	
	if (getFirstInstall())		
	{
		// text install
		title = getString("uiBGD_TitleStartProcessInstall");
		content = format( getString("uiBGD_ContentStartProcessInstall"), sizeToHumanStd( getRyzomInstallSize()));			
	}
	else
	{
		// text repair
		title = getString("uiBGD_RyzomUpdateRepair");
		content = format( getString("uiBGD_ContentStartProcessRepair"), sizeToHumanStd( getRyzomInstallSize()));
	}

	if ( !getUseFullVersion())
	{		
		// text explaining full version
		content += "\n\n";
		content +=  getString("uiBGD_ContentStartProcessFullVersion");
	}

	if (getUseTorrent())
	{
		// text explaining torrent protocol
		content += "\n\n";
		content += getString("uiBGD_ContentStartProcessTorrent");

	}

	NLMISC::strFindReplace(content, "\\n", "\n");
	if (_ChoosePackageDlg)
	{
		_ChoosePackageDlg->setInstallText(title, content);
	}
	
}

void CGameDownloader::registerChoosePackageDlg(CChoosePackageDlg* dlg){
	_ChoosePackageDlg = dlg;
}

void CGameDownloader::setLabelTimeLeftVisible(bool visible)
{
}


void CGameDownloader::doAction(const char* actionName, bool state)
{



	nldebug("doAction '%s' %d", actionName, state);


	

	std::string action(actionName);
	

	if (this->getState() != IGameDownloader::InstallFinished && action == "download_cancel")
	{	
		
		// display a "Are you sure to quit" popup message
		ucstring title = getString("uiBGD_QuitInstallTitle" );
		ucstring content = getString( "uiBGD_QuitInstallContent" );
		if (getFirstInstall())
		{
			content += "\n\n";
			content += getString( "uiBGD_QuitInstallRestart" );
			content = wrapText(content);
		}
	
		MSGBOXPARAMSW params = {0};
		params.cbSize = sizeof MSGBOXPARAMSW;
		params.hInstance = NULL;
		params.lpszText = (LPCWSTR)content.c_str();
		params.lpszCaption = (LPCWSTR)title.c_str();
		params.dwStyle = MB_OKCANCEL | MB_ICONEXCLAMATION;
		params.dwLanguageId = MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL);
		bool previousState = _UpdateDownloadIcone;
		_UpdateDownloadIcone = false;
		UINT result = MessageBoxIndirectW (&params);
		_UpdateDownloadIcone = previousState;


		if (result == IDOK)
		{
			setState(IGameDownloader::InstallFinished);	
		}
		else
		{	// the user click on cancel so go back to where we were			
		}
	}

	// Start of application
	if (this->getState() == IGameDownloader::Init)
	{
		// Update label in function fo the mode
		if (action == "welcome_continue")
		{
			this->setStarted(true);
			
			if ( this->getFirstInstall() )
			{
				this->setLabelContentText( getString( "uiBGD_LoadVersionInfo" ) );
				this->setState(IGameDownloader::InitFinished);
			}
			else
			{
				this->setLabelContentText( getString("uiBGD_CheckData" ) );
				this->setState(IGameDownloader::InitFinished);
			}
			
			
			return;
		}
	//	return;
	}

	// Download/Install confirmation form
	if (this->getState() == IGameDownloader::CheckFinished)
	{
		// click on Full version check box -> update the text displayed
		if (action == "download_full_version")
		{
			this->setUseFullVersion(state);
			this->updateStartProcessText();
			return;
		}

		// click on Torrent protocol box -> update the text displayed
		if (action == "download_use_torrent")
		{
			this->setUseTorrent(state);
			this->updateStartProcessText();
			return;
		}

		// click on continue button at the download asking -> update the text displayed
		if (action == "download_continue")
		{			
			//form->closePopup();
			this->setBrowserActive(true);
			this->setLabelTimeLeftVisible(false);			
			this->setLabelContentText(  getString( "uiBGD_LoadDownloadList" ) );			
			this->selectPackage(this->getUseFullVersion());
			this->setUseTorrent(this->getUseTorrent());
			this->getDownloadList();
			return;

		}
	//	return;
	}

	// Ask install form (no more displayed)
	if (this->getState() == IGameDownloader::AskInstalling)
	{
		// click on continue button
		if (action ==  "install_continue")
		{
			// continue install
			this->setState(IGameDownloader::InstallOk);
		}
		return;
	}
	
// Form telling that the installation is over and asking the player if he want to lauch ryzom
//	if (this->getState() == IGameDownloader::InstallFinishing)
	{
		// if click on continue, continue the install process (with the "Start ryzom At End" setted by the check box)
		if (action ==  "launch_continue")
		{	
		//	form->closePopup();			
		//	this->setProgressBarVisible( false);				
			return;
		}
		// click on Ryzom "Start at End" checkbox
		if (action ==  "launch_start_at_end")
		{			
			this->setStartRyzomAtEnd( state );		
		}
	//	return;
	}

}

void CGameDownloader::packageSelectionUpdated(ICkeckInfoViewer* viewer)
{
	nldebug("Package list is updated. Open ChoosePackage Dlg.");
	CChoosePackageDlg choose;
	bool previousState = _UpdateDownloadIcone;
	_UpdateDownloadIcone = false;
	UINT result = choose.DoModal();		
	_UpdateDownloadIcone = previousState;

	if (result == IDOK)
	{
		doAction("download_continue", true);
	}
	else
	{		
		doAction("download_cancel", true);
	}
	
	
}

void CGameDownloader::setInstallTaskDlg(CInstallTaskDlg* dlg)
{
	this->_InstallTaskDlg = dlg;
}

//! Translate duration in seconde to string readable by humain
ucstring CGameDownloader::timeToHuman(double t)
{
	
	if (t==0)	
	{
		return  ucstring(std::string(""));
	}	
	double days = 60*60*24;
	double hours = 60*60;
	double minutes = 60;		

	uint32 d, h, m, s;

	// comput duration in day hours month secod
	d = static_cast<uint32>(t / days);
	t = t - d * days;
	h = static_cast<uint32>(t / hours);
	t = t - h * hours;
	m = static_cast<uint32>(t / minutes);
	t = t - m * minutes;
	s = static_cast<uint32>(t);
	
	ucstring ret;
	// if bigger thant a day : time is too big to be meaningfull
	if ( d > 0)
	{
		ret = std::string("-");
		return ret;
	}
	
	if ( h > 0)
	{	// use translatiion to display hours
		ucstring formatTxt = getString(h==1?"uiBGD_Hour":"uiBGD_Hours");	
		ret =  ret + ucstring(std::string(" ")) + format(formatTxt, NLMISC::toString("%d", h));
	
	}
	
	if ( m > 0 && h == 0)
	{		
		// use translatiion to display minutes (if more thant one hours do not display minute because its not meanginfull)
		ucstring formatTxt = getString(m==1?"uiBGD_Minute":"uiBGD_Minutes");	
		ret =  ret + ucstring(std::string(" ")) + format(formatTxt, NLMISC::toString("%d", m));
	}

	if ( s >= 0 && h ==0 && m < 3)
	{
		// use translatiion to display seconds (if more than 3minutes its not meaningfull to display seconds)
		ucstring formatTxt = getString(s==1?"uiBGD_Seconde":"uiBGD_Secondes");	
		ret =  ret + ucstring(std::string(" ")) + format(formatTxt, NLMISC::toString("%d", s));
	}
	// display something link "(3 minute restante)"
	ret = format(getString("uiBGD_TimeLeft"), ret);			
	return ret;

}
void CGameDownloader::onFileUpdate(const char * trad, const char * filename, uint32 rate, uint32 index, uint32 filecount,  uint32 firstValue, uint32 lastValue, uint64 size, uint64 fullSize, const char * category  )
{
	//function used by install and download process

	ucstring tr = getString(trad);
	ucstring f = "";
	ucstring r = "";
	ucstring s = "";
	ucstring i = "";
	
	//if filename exist add "of filename" to sentence
	if (filename)
	{		
		ucstring  filenameFormat = getString("uiBGD_FilenameFormat");		
		f = format(filenameFormat, ucstring(std::string(filename)));			
	}

	if (index > filecount) { index = filecount; }
	if (size > fullSize) { size = fullSize; }
	//if rate exist add "at 18KB/S" to sentence
	if (rate)
	{		
		ucstring  rateFormat = getString("uiBGD_RateFormat");
		r = format(rateFormat, sizeToHumanStd(rate));						
	}

	//if filecount exist add "(14/15)" to sentence
	if (filecount)
	{
		i = format(std::string(" ({0}/{1})"), NLMISC::toString("%d", index), NLMISC::toString("%d", filecount));		
	}
	//if filecount exist add " - (16MB/205MB)" to sentence
	if (fullSize)
	{
		s = format(std::string(" - {0}/{1}"), sizeToHumanStd(size), sizeToHumanStd(fullSize));				
	}
	
	//display sentence like "download of toto.bnp at 14MB/S (1/4) -( 200MB/1.6GB)
	
	sint64 newProgressBarValue = -1;


	if (fullSize) { newProgressBarValue = firstValue + ( (lastValue-firstValue) * size) / fullSize; }
	else if (filecount) { newProgressBarValue = firstValue + ( (lastValue-firstValue) * index) / filecount; }

	// if progress bar has invalide value do not update it
	if (newProgressBarValue != -1 && ( 0 <= newProgressBarValue  && newProgressBarValue <= 100))
	{
		setProgressBarValue(static_cast<int>(newProgressBarValue));
	}
	else
	{
		std::string WTF=NLMISC::toString("%d", newProgressBarValue);
		WTF += "d";
	}
	ucstring toDisplay = format(std::string("{0}{1}{2}{3}{4}"), tr, f, r, i, s);
	// if category can be equaldownload or upload
	if (category)
	{
		IGameDownloader* gd = IGameDownloader::getInstance();
		static std::string previous("");
		static sint64 rate2 = 0; 

		// smooth a litle bit the rate
		if (previous == category)
		{
			rate2 = (3*rate2 + rate)/4;  
		}
		else
		{
			previous = std::string(category);
			rate2 = rate;
		}

		if (category)
		{
			ucstring  timeleft = "";//getString(category);
			if (rate2)
			{
				
				double t = double(fullSize - size) /  rate2;
				// when downloading we add the time "estimate" the install process will tack
				if (std::string(category) == std::string("uiBGD_CategoryDownload"))
				{
					t +=  double (gd->getUseFullVersion() ?	gd->getRyzomInstallSize() : gd->getRosInstallSize() ) / _InstallRate;
				}
				
				timeleft = timeToHuman(t);
				toDisplay +=  "\n";
				toDisplay +=  timeleft; // Hack to put on the same label the time left
				ucstring  cat = getString(category);
				cat += " ";
				cat += timeleft;
				setLabelTimeLeftText( cat );
			}

			//dispaly at "14hours left" to the "Size required label"
			
		}
		
	}

	setLabelContentText( toDisplay.toString() );	
}


void CGameDownloader::displayMsgClientUpdated(bool startClient)
{
	setLabelContentText(getString("uiBGD_InstallFinished"));
	setProgressBarValue(100);

	CInstallSuccessDlg installSuccess;

	bool previousState = _UpdateDownloadIcone;
	_UpdateDownloadIcone = false;
	UINT result = installSuccess.DoModal();
	_UpdateDownloadIcone = false;
	
	if (result != IDOK)
	{
		// We are in the case the user has clicke on quit button then ok on the popup
		if (getState() == IGameDownloader::InstallFinished)
		{	// we do not launch the install (because he have quit)
			startClient = false;
			setStartRyzomAtEnd(false);
		}

	}

	// the batch has the right to continue
	this->_WaitForInstallLock.leave();

	
	if (startClient && getStartRyzomAtEnd())
	{
		if (NLMISC::CFile::fileExists("client_ryzom_rd.exe") )
		{
			NLMISC::launchProgram("client_ryzom_rd.exe", "");			; 
		}
	}

	if (_InstallTaskDlg->IsWindowVisible()) { 
		_InstallTaskDlg->ShowWindow(SW_HIDE); 
	}

	NLMISC::nlSleep(10000);	
	setState(IGameDownloader::InstallFinished);

}


ucstring CGameDownloader::getString(const std::string& msg)
{
	std::string msgCpy = msg;

	ucstring ret = NLMISC::CI18N::get(msgCpy);
	return ret;
}


void CGameDownloader::closeApplication()
{
	PostQuitMessage(0);
}


void CGameDownloader::fatalError(const char* msg, const char* param1, const char* param2)
{


	//do not display fatal Error 2 time
	IGameDownloader* gd = IGameDownloader::getInstance();
	if (gd->getState() == IGameDownloader::InstallFinished)
	{
		return;
	}

	// display Error message
	ucstring str =  getString(msg);
	ucstring p1, p2;
	p1.fromUtf8(param1);
	p2.fromUtf8(param2);
	ucstring content = format(str, p1, p2);
	
	//Display a message asking to check support formum (Support Url is different between language)
	ucstring supportUrl = getString("uiBGD_ErrorSupportUrl");
	ucstring supportUrlFormat = getString("uiBGD_ErrorSupportFormat");
	
	content += "\n\n";
	content += format(supportUrlFormat, supportUrl );

	ucstring restart;
	// Display information how to restart launcher by clicking on ryzom icon
	// only if ryzom has not been already installed
	if ( gd->getFirstInstall())
	{		
		restart =  getString("uiBGD_QuitInstallRestart" );
	}

	// Help button launch the url
	

	
	CErrorBoxDlg error(content, restart);
	bool previousState = _UpdateDownloadIcone;
	_UpdateDownloadIcone = false;
	error.DoModal();
	_UpdateDownloadIcone = previousState;

	
	
	// stop the program
	gd->setState(IGameDownloader::InstallFinished);
	
}

std::string CGameDownloader::logOnServer(const std::string& msg)
{	
	std::string msg2 = getLogUrl(msg);
	uint32 state = LoginStep_Unknown;
	bool forceSyncMsg = false;

	if (msg2.find("start_download") != std::string::npos)
	{ 
		nldebug("Init connection with stats.php");

		CLoginProgressPostThread::getInstance().init(theApp.ConfigFile);	
		//nldebug("Reset the login step");
		//CLoginRegistry::setLoginStep(0);//reset the login step because new download/installation process		
		state = InstallStep_StartDownload;  forceSyncMsg = true; 
	}
	else if (msg2.find("update_download") != std::string::npos) { state = InstallStep_UpdateDownload; }
	else if (msg2.find("stop_download") != std::string::npos) { state = InstallStep_StopDownload; }
	else if (msg2.find("start_install") != std::string::npos) { state = InstallStep_StartInstall; }
	else if (msg2.find("update_install") != std::string::npos) { state = InstallStep_UpdateInstall; }
	else if (msg2.find("stop_install") != std::string::npos) { state = InstallStep_StopInstall; }


	
	// Give 10 second for the logOnServerToBeInitialized
	if (forceSyncMsg)
	{
		int nbSec = 0;
		std::string ret;
		bool msgSend = false;
		int it = 0; // the number of iteration before give up

		CLoginProgressPostThread::getInstance().step(CLoginStep(state, msg2.c_str(),&ret,&msgSend));
		// msg are supposed to leave queue every second
		while (!msgSend && it < 30)
		{
			NLMISC::nlSleep(100);
			++it;
		}
		nldebug("Update stats (and wait result): %s", msg.c_str());
		return ret;
	}
	else
	{
		//if ( state != InstallStep_UpdateDownload && state != InstallStep_UpdateInstall)
		{
			nldebug("Update stats: %s", msg.c_str());
		}		
		CLoginProgressPostThread::getInstance().step(CLoginStep(state, msg2.c_str()));
	}
	return "";	
}

std::string CGameDownloader::getInstallHttpBackground() const
{  
	std::string in(_RyzomControl->getInstallHttpBackground());
	std::string::size_type it = in.find("_wk.");

	if (it == std::string::npos)
	{
		return in;
	}
	// there is now wk web page so use en instead
	std::string out;
	out = in.substr(0, it);
	out += "_en.";
	out += in.substr(it+4);
	return out;
}




//-----------------------------------------------------------------------------------------
IGameDownloader* IGameDownloader::_Instance = 0;

IGameDownloader* IGameDownloader::getInstance()
{
	if (!_Instance)
	{
		_Instance = new CGameDownloader(  );
	}
	return _Instance;
};


