// Eer.cpp : fichier projet principal.



#include "game_downloader.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include <exception>

#using <System.dll>
#include <vcclr.h>

#include "CClientInstallForm.h"
#include "ryzom_control/ryzom_control.h"
#include "torrent_control/torrent_control.h"
#include <cstring>

// transform an int to a string
static std::string toString(int value)
{
	static char buffer[256];
	std::sprintf(buffer, "%d", (int)value);	
	return std::string(buffer);
}

//transform an uint64 to a Hex string
static std::string uint64ToHex(unsigned long long size)
{
	char data[256];
	sprintf(data, "%llX", size);
	return std::string(data);
}

//transform a size to a Human readable size ex "1.83GB"
static std::string sizeToHumanStd(unsigned long long size)
{
		double total = size;		
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
		std::sprintf(buffer, "%0.3f", (double)total);
		std::string ret(buffer);
		ret+=unit;

		return ret;
}


//! Store patch info (from version description file given By Ryzom module)
class CEntry
{
public:
	CEntry(){ Timestamp = 0; Size = unsigned int(-1); }

	CEntry(const char* patchName, const char* sourceName, unsigned int timestamp, const char* extractPath, unsigned int sevenZipFileSize,  unsigned int size)
		:PatchName(patchName), SourceName(sourceName), Timestamp(timestamp),ExtractPath(extractPath), SevenZipFileSize(sevenZipFileSize), Size(size){}

	std::string PatchName; //!< The name of the patch "0422/exedll.bnp"
	std::string SourceName; //!< The name of the source "data/exedll.bnp"
	unsigned int Timestamp; //!< The timestamp of the unpacked file
	std::string ExtractPath;  //!< the directory whe data can be unpack eg "data/" 
	unsigned int Size;//!< The size of the bnp file after depacking the lzma
	unsigned int SevenZipFileSize; //!The size of the lzma
};

//! Use to feed IRyzomControl to specify the list of file we want to download/intall
class CStringContainer : public IStringContainer
{
public:
	CStringContainer(std::vector<std::string>* data):_Data(data){}
	virtual unsigned int getCount() const { return _Data->size(); }
	virtual const char * getName(unsigned int index) const { return (*_Data)[index].c_str(); }
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
	virtual unsigned int getCount() const { return _Data->size(); }

	//! \seed ITorrentStringContainer::getName
	virtual const char * getName(unsigned int index) const 
	{ 
		static std::string ret;
		ret = _Prefix;
		ret += (*_Data)[index].PatchName;
		return ret.c_str(); 
	}

	//! \seed ITorrentStringContainer::setSize
	virtual void setSize(unsigned int index, unsigned int size)
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
	virtual unsigned int getCount() const { return _Data->size(); }

	//! \see IInstallEntryViewer::getPatchName
	virtual const char * getPatchName(unsigned int index) const { return (*_Data)[index].PatchName.c_str(); } 
	
	//! \see IInstallEntryViewer::getSourceName
	virtual const char * getSourceName(unsigned int index) const { return (*_Data)[index].SourceName.c_str(); } 

	//! \see IInstallEntryViewer::getTimestamp
	virtual unsigned int getTimestamp(unsigned int index) const { return (*_Data)[index].Timestamp; } 

	//! \see IInstallEntryViewer::getExtractPath
	virtual const char * getExtractPath(unsigned int index) const { return (*_Data)[index].ExtractPath.c_str(); } 

	//! \see IInstallEntryViewer::getSize
	virtual unsigned int getSize(unsigned int index) const { return (*_Data)[index].Size; } 

	//! \see IInstallEntryViewer::getSZipFileSize
	virtual unsigned int getSZipFileSize(unsigned int index) const { return (*_Data)[index].SevenZipFileSize; } 
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



	//----------------- IGameDownloader --------------------
	//! \see IGameDownloader::update
	virtual void update() ;	
	//! \see IGameDownloader::getPackageSelection
	virtual void getPackageSelection(bool mustRepair);
	//! \see IGameDownloader::setForm
	virtual void setForm(System::Windows::Forms::Form^ form);
	//! \see IGameDownloader::getState
	virtual EState getState() const { return _State; }
	//! \see IGameDownloader::setState
	virtual void setState(EState state){ _State = state; }
	//! \see IGameDownloader::updateLauncher
	virtual bool updateLauncher();
	//! \see IGameDownloader::updateTorrent
	virtual bool updateTorrent();
	//! \see IGameDownloader::setTorrentInfo
	virtual void setTorrentInfo(const std::string & torrentPatchFilename,
		const std::string & torrentFilename,
		unsigned int  torrentPatchTimestamp);
	//! \see IGameDownloader::setInstallInfo
	virtual void setInstallInfo(const std::string & installPatchFilename,
		const std::string & installFilename,
		const std::string & installPatchExtract, unsigned int installPatchTimestamp) ;
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
	//! \see IGameDownloader::getInstallHttpBackground
	virtual std::string getInstallHttpBackground() const {  return std::string(_RyzomControl->getInstallHttpBackground());}
	//! \see IGameDownloader::getLogUrl
	virtual std::string getLogUrl(const std::string& action) const ;
	//! \see IGameDownloader::setFirstInstall
	virtual void setFirstInstall(bool firstInstall) { _FirstTimeInstall = firstInstall; }
	//! \see IGameDownloader::getFirstInstall
	virtual bool getFirstInstall() const { return _FirstTimeInstall; }
	//! \see IGameDownloader::getRosInstallSize
	virtual  unsigned long long getRosInstallSize() const {	return _RosInstallSize;	}
	//! \see IGameDownloader::getRyzomInstallSize
	virtual unsigned long long getRyzomInstallSize() const {return _RyzomInstallSize; }
	//! \see IGameDownloader::getRosPatchSize
	virtual  unsigned long long getRosPatchSize() const { return _RosPatchSize;	}
	//! \see IGameDownloader::getRyzomPatchSize
	virtual unsigned long long getRyzomPatchSize() const { return _RyzomPatchSize; }
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
	virtual void addToDownloadList(const char* patchName, const char* sourceName, unsigned int timestamp, const char* extractPath, unsigned int sZipFileSize, unsigned int size);
	//! \see IRyzomControlListener::selectPackage
	virtual void selectPackage(bool mainland);
	//! \see IRyzomControlListener::setUseTorrent
	virtual void setUseTorrent(bool useTorrent);
	//! \see IRyzomControlListener::setVersionInfo
	virtual void setVersionInfo(const std::vector<std::string>& patchUris, const std::string& serverPath) ;
	//! \see IRyzomControlListener::onFileInstallFinished
	virtual void onFileInstallFinished();	
	//! \see IRyzomControlListener::onFileDownloading
	virtual void onFileDownloading(const char* sourceName, unsigned int  rate, unsigned int  fileIndex, unsigned int  fileCount, unsigned long long  fileSize, unsigned long long fullSize);	
	//! \see IRyzomControlListener::onFileInstalling
	virtual void onFileInstalling(const char* sourceName, unsigned int  rate, unsigned int  fileIndex, unsigned int  fileCount, unsigned long long  fileSize, unsigned long long fullSize);		
	//! \see IRyzomControlListener::onFileDownloadFinished
	virtual void onFileDownloadFinished();
	//! \see IRyzomControlListener::getVersion
	virtual const char * getVersion() const { return _Version.c_str();; }
	//! \see IRyzomControlListener::getPatchUriCount
	virtual unsigned int getPatchUriCount() const { return _PatchUris.size(); }
	//! \see IRyzomControlListener::getPatchUriValue
	virtual const char * getPatchUriValue(unsigned int index) const { return _PatchUris[index].c_str(); }
	//! \see IRyzomControlListener::getServerPath
	virtual const char * getServerPath() const { return _ServerPath.c_str(); }
	//! \see IRyzomControlListener::getLauncherName
	virtual  const char * getLauncherName() const { return "install.bat"; }
	//! \see IRyzomControlListener::requestStop
	virtual void requestStop();
	//! \see IRyzomControlListener::initI18n
	virtual void initI18n(const char* lang, unsigned char *data, unsigned int length);
	//! \see IRyzomControlListener::updateFuntion
	void updateFuntion(TUpdateType updateType, unsigned int index, unsigned int max);
	//! \see IRyzomControlListener::fatalError
	virtual void fatalError(const char* msg, const char* param1, const char* param2) { _Form->fatalError(msg, param1, param2); 	}
	


private:
	IRyzomControl* _RyzomControl; //!< Control the ryzom patch protocol
	ITorrentControl* _TorrentControl; //!< Control the torrent patch protocol
	gcroot<client_install::CClientInstallForm ^> _Form; //!< The Gui form
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
	unsigned int _TorrentPatchTimestamp;//!< The timestamp of the torrent file
	std::string _InstallPatchFilename; //!< obsolete
	std::string _InstallFilename;//!< obsolete
	unsigned int  _InstallPatchTimestamp;//!< obsolete
	std::string _InstallPatchExtract;//!< obsolete
	bool _UseTorrent; //!< Is torrent protocol used
	bool _TorrentExist; //!< Torrent file exist on server
	bool _FirstTimeInstall; //!< is it the first install (or repair)
	bool _StartRyzomAtEnd; //!< Must strat ryzom at end of install
	unsigned long long _RosPatchSize; //!< Size of the patch when not in full version
	unsigned long long _RyzomPatchSize; //!< Size of the patch in full version
	unsigned long long _RosInstallSize; //!< Size of data use on dique after depack on initial version
	unsigned long long _RyzomInstallSize; //!< Size of data use on dique after depack on full version
	unsigned long long 	_PreviousDownloaded; //!< Size of data downloaded on previous session
	std::string _SessionId; //!< number of session (given by stats.php)
	std::string _InstallId; //!< install id (given by registy)
	std::string _UserId; //!<user id (given by stats.php)
	bool _AutoInstall; //!< obsolete
	bool _Started;
	bool _DownloadOnly; //!< obsolete
	bool _UseFullVersion; //same as _Mainland but only for GUI
	
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
	_FirstTimeInstall = true;

	_RyzomControl =  IRyzomControl::init(this);
	_TorrentControl = ITorrentControl::getInstance();
	_TorrentControl->setListener(this);

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

}


void CGameDownloader::setVersionInfo(const std::vector<std::string>& patchUris, const std::string& serverPath)
{
	_PatchUris = patchUris;
	_ServerPath = serverPath;
	// save Version info
	std::string versionInfo = std::string(_RyzomControl->getApplication()) + ".version";
	std::string finalPath = _TmpDir + versionInfo;
	
	_RyzomControl->initPatchManager(); //update ServerVersion

	// creation of download directory if not exist
	{
		System::String^ tmpDirName = gcnew System::String( std::string( _InstallDir).c_str());
		if (!System::IO::Directory::Exists(tmpDirName))
		{
			System::IO::Directory::CreateDirectory( tmpDirName );
		}
	}
	// creation of tmp download directory if not exist
	{
		System::String^ tmpDirName = gcnew System::String( std::string( _TmpDir).c_str());
		if (!System::IO::Directory::Exists(tmpDirName))
		{
			System::IO::Directory::CreateDirectory( tmpDirName );
		}
	}

	// save version 
	{
		std::string version = _InstallDir + "version.txt";
		FILE*  pFile = fopen(version.c_str(), "wb");
		fprintf(pFile, "%s", versionInfo.c_str() );
		fclose(pFile);
	}
	_Version = _RyzomControl->getServerVersion();
	{
		unsigned int versionId = atoi(_Version.c_str());
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


	_ToDownloadCategory.clear();
	// Add all optional catgory to Category to download 
	{
		unsigned int i = 0;
		unsigned int count = viewer->getCount(ICkeckInfoViewer::OptCat);
		for (; i != count; ++i)
		{
			_ToDownloadCategory.push_back(std::string(viewer->getName(ICkeckInfoViewer::OptCat, i) ) );
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
		unsigned int i = 0;
		unsigned int count = viewer->getCount(ICkeckInfoViewer::NonOptCat);
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
		unsigned int i = 0;
		unsigned int count = viewer->getCount(ICkeckInfoViewer::OptCat);
		for (; i != count; ++i)
		{
			_RyzomPatchSize+= viewer->getSZipFileSize(ICkeckInfoViewer::OptCat, i);
			_RyzomInstallSize+= viewer->getFileSize(ICkeckInfoViewer::OptCat, i);
		}
	}
	// Calculate patch and install size of ReqCat Category
	{
		unsigned int i = 0;
		unsigned int count = viewer->getCount(ICkeckInfoViewer::ReqCat);
		for (; i != count; ++i)
		{
			_RyzomPatchSize += viewer->getSZipFileSize(ICkeckInfoViewer::ReqCat, i);
			_RyzomInstallSize +=  viewer->getFileSize(ICkeckInfoViewer::ReqCat, i);
		}
	}
	// if there is no category to download then the client is up to date display the message
	if (viewer->getCount(ICkeckInfoViewer::OptCat) == 0 && viewer->getCount(ICkeckInfoViewer::NonOptCat ) == 0 && viewer->getCount(ICkeckInfoViewer::ReqCat ) == 0 )
	{
		setState(IGameDownloader::InstallFinishing);	  
		_Form->displayMsgClientUpdated(true);		
		return;
	}

	setState(CheckFinished);
	// display message you're about to downlaod ryzom ...
	_Form->packageSelectionUpdated(viewer);	
}


void CGameDownloader::onDownloadListFinished()
{

	// compute the size of date of file downloaded in the previous sessions
	unsigned int first(0), last(_Entries.size());
	for (; first != last; ++first)
	{
		std::string tmpName = this->getTorrentTmpDir() + _PatchRootDir + _Entries[first].PatchName;	
		System::String^ tmpStr = gcnew System::String(tmpName.c_str());
		if ( System::IO::File::Exists(tmpStr ))
		{
			
			System::IO::FileInfo^ fi = gcnew System::IO::FileInfo(tmpStr);
			_PreviousDownloaded += fi->Length;		
		}
	}

	// As sone as the list of file to download is given lauch the download process
	_Form->onDownloadListFinished();
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


void CGameDownloader::addToDownloadList(const char* patchName, const char* sourceName, unsigned int timestamp, const char* extractPath, unsigned int sevenZipFileSize, unsigned int size)
{
	//creating the list of file to download
	std::string newPatchName =  patchName;
	_Entries.push_back( CEntry(newPatchName.c_str(), sourceName, timestamp, extractPath, sevenZipFileSize, size) );	
	//display msg to GUI
	_Form->addToDownloadList(newPatchName.c_str(), sourceName, timestamp);
}


void CGameDownloader::update()
{
	_TorrentControl->update();
	_RyzomControl->update();
	if (_State == Downloading)
	{
		if (getUseTorrent())
		{
			// In case of torrent downloading we have to pull to have info on donload stat
			if ( _TorrentControl->isCurrentDownloadValid())
			{
				_Form->onFileUpdate(
					_TorrentControl->getCurrentDownloadStateLabel(),
					_TorrentControl->getCurrentDownloadName(), 
					_TorrentControl->getCurrentDownloadDownloadRate(), 
					0, 
					0,  
					0, 
					50, 
					_TorrentControl->getCurrentDownloadTotalDone(), 
					_TorrentControl->getCurrentDownloadTotalWanted(),
					"uiCategoryDownload"
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
		unsigned int first = 0, last= _Entries.size();
		long long fullSize = 0;
		for ( ; first != last; ++first)
		{
			fullSize += _Entries[first].Size;
		}
		_Form->askForInstall(fullSize);	//message is not displayed anymore (at end of download launch install)
		return;
	}

	if (_State == InstallOk)
	{
		_State = Installing;
		CInstallEntryViewer viewer(&_Entries);
		_RyzomControl->install(&viewer, false);
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
	System::Diagnostics::Debug::WriteLine(gcnew System::String(msg));
}

void CGameDownloader::onFileDownloading(const char* sourceName, unsigned int  rate, unsigned int  fileIndex, unsigned int  fileCount, unsigned long long  fileSize, unsigned long long fullSize)
{	// from ryzom protocol to gui
	if (fullSize == 0)
	{
		fullSize = 100;
		fileSize = 0;
	}
	unsigned int max = 50;
	unsigned int index = unsigned int( double(max) * fileSize / fullSize);
	//display the download bar from 0 to 50 percent

	_Form->onFileUpdate("uiDownload", sourceName, rate, fileIndex, fileCount, 0, 50, fileSize, fullSize, "uiCategoryDownload");
}


void CGameDownloader::onFileInstalling(const char* sourceName, unsigned int  rate, unsigned int  fileIndex, unsigned int  fileCount, unsigned long long  fileSize, unsigned long long fullSize)
{	// from ryzom protocol to gui
	if (fullSize == 0)
	{
		fullSize = 100;
		fileSize = 0;
	}
	//display the isntall  bar from 50 to 100 percent
	unsigned int max = 50;
	unsigned int index = 50 + unsigned int( double(max) * fileSize / fullSize);
	
	_Form->onFileUpdate("uiInstalling", sourceName, rate, fileIndex, fileCount, 50, 100, fileSize, fullSize, "uiCategoryInstall");
}

void CGameDownloader::onFileDownloadFinished()
{
	_State = DownloadFinished;
}


void CGameDownloader::onFileInstallFinished()
{
	setState (AskInstallFinishing);
	if ( _Form->WaitMutex )
	{
		 _Form->WaitMutex->WaitOne();
	}
//	_Form->onFileInstallFinished();

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

void CGameDownloader::initI18n(const char* lang, unsigned char *data, unsigned int length)
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
		_Form->setState(newState);
	}
}

void CGameDownloader::updateFuntion(IRyzomControlListener::TUpdateType updateType, unsigned int index, unsigned int max)
{
	// from ryzom module to gui
	switch (updateType)
	{
		case IRyzomControlListener::ScanData: _Form->onFileUpdate("uiScanData", 0, 0, index, max, 0, 100, 0, 0, 0); break;
		case IRyzomControlListener::CheckData: _Form->onFileUpdate("uiCheckData", 0, 0, index, max, 0, 0, 0, 0, 0); break;
		case IRyzomControlListener::Patch: _Form->onFileUpdate("uiPatch", 0, 0, index, max, 0, 0, 0, 0, 0); break;
		default:;
	}
}

// obosolete
void CGameDownloader::setTorrentInfo(const std::string & torrentPatchFilename,
	const std::string & torrentFilename,
	unsigned int  torrentPatchTimestamp)
{
	_TorrentPatchFilename = torrentPatchFilename;
	_TorrentFilename = torrentFilename;
	_TorrentPatchTimestamp = torrentPatchTimestamp;
}

// obosolete
void CGameDownloader::setInstallInfo(const std::string & installPatchFilename,
	const std::string & installFilename,
	const std::string & installPatchExtract, unsigned int installPatchTimestamp) 
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
		copy += std::string("&install_id=") + _InstallId;
		copy += std::string("&os=") + std::string(INelControl::getOS ());
		copy += std::string("&proc=") + std::string(INelControl::getProc ());
		copy += std::string("&memory=") + std::string(sizeToHumanStd(INelControl::availablePhysicalMemory ()));
		//copy += std::string("&hd=") + std::string(INelControl::availableHDSpace ());
		copy += std::string("&video_card=") + std::string(INelControl::getVideoInfoDeviceName ());
		copy += std::string("&driver_version=") + std::string(uint64ToHex(INelControl::driverVersion ()));			
	}
	else
	{


		if (action.find("start_download") != std::string::npos)
		{
			long long installSize = _Mainland ? getRyzomInstallSize() : getRosInstallSize();
			long long downloadSize = _Mainland ? getRyzomPatchSize() : getRosPatchSize();			
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
		copy += std::string("&user_id=") + _UserId;			
	}


	return std::string(_RyzomControl->getLogUrl(copy.c_str()));
}

void CGameDownloader::setStartRyzomAtEnd(bool ok) 
{
	_StartRyzomAtEnd = ok;
	_RyzomControl->setStartRyzomAtEnd(ok);
}

void CGameDownloader::setForm(System::Windows::Forms::Form^ form)
{
	client_install::CClientInstallForm^ tmp = static_cast<client_install::CClientInstallForm^>(form);
	_Form = tmp;
}

//-----------------------------------------------------------------------------------------
IGameDownloader* IGameDownloader::_Instance;

IGameDownloader* IGameDownloader::getInstance()
{
	if (!_Instance)
	{
		_Instance = new CGameDownloader(  );
	}
	return _Instance;
};


