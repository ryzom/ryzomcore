#include "ryzom_control.h"

#include <string>
#include "login_patch2.h"
#include "nel/misc/i18n.h"
 
#include "nel/misc/app_context.h"
#include "nel/misc/system_info.h"

std::string TheTmpInstallDirectory;	
std::string VersionName;
std::string R2ServerVersion;

std::string CfgApplication;
std::string ClientLauncherUrl;



 NLMISC::CApplicationContext* ApplicationContext = 0;

using namespace NLMISC;
 
//-------------------------------------------
//! see ICkeckInfoViewer
class CCheckInfoViewer : public ICkeckInfoViewer
{
public:
	CCheckInfoViewer(CPatchManager::SPatchInfo* patchInfo);

	//! \see ICkeckInfoViewer::getCount
	virtual unsigned int getCount(ECat cat) const;

	//! \see ICkeckInfoViewer::getCount
	virtual const char * getName(ECat cat, unsigned int index) const;

	//! \see ICkeckInfoViewer::getCount
	virtual unsigned int getSZipFileSize(ECat cat, unsigned int index) const;

	//! \see ICkeckInfoViewer::getCount
	virtual unsigned int getFileSize(ECat cat, unsigned int index) const;

private:
	CPatchManager::SPatchInfo *_PatchInfo;
};

CCheckInfoViewer::CCheckInfoViewer(CPatchManager::SPatchInfo* patchInfo):_PatchInfo(patchInfo){}

unsigned int CCheckInfoViewer::getCount(ECat cat) const
{
	switch(cat)
	{
		case NonOptCat:	return _PatchInfo->NonOptCat.size();
		case ReqCat:	return _PatchInfo->ReqCat.size();
		case OptCat:	return _PatchInfo->OptCat.size();
	}
	return 0;
}

const char * CCheckInfoViewer::getName(ECat cat, unsigned int index) const
{
	switch(cat)
	{
		case NonOptCat:	return _PatchInfo->NonOptCat[index].Name.c_str();
		case ReqCat:	return _PatchInfo->ReqCat[index].Name.c_str();
		case OptCat:	return _PatchInfo->OptCat[index].Name.c_str();
	}
	return 0;
}

unsigned int CCheckInfoViewer::getSZipFileSize(ECat cat, unsigned int index) const
{
	
	switch(cat)
	{	
		case NonOptCat:	return _PatchInfo->NonOptCat[index].SZipSize;
		case ReqCat: return _PatchInfo->ReqCat[index].SZipSize;
		case OptCat: return _PatchInfo->OptCat[index].SZipSize;
	}
	return 0;
}

unsigned int CCheckInfoViewer::getFileSize(ECat cat, unsigned int index) const
{
	switch(cat)
	{	
		case NonOptCat:	return _PatchInfo->NonOptCat[index].FinalFileSize;
		case ReqCat: return _PatchInfo->ReqCat[index].FinalFileSize;
		case OptCat: return _PatchInfo->OptCat[index].FinalFileSize;
	}
	return 0;
}
//-------------------------------------------------
//! Function that enble to "load" a i18n file from the Data member instread of harddisk
class CInstallLoadProxy : public NLMISC::CI18N::ILoadProxy
{
public:
	//! \see  NLMISC::CI18N::ILoadProxy::loadStringFile
	virtual void loadStringFile(const std::string &filename, ucstring &text);
public:
	std::map<std::string, ucstring> Data;

};

void CInstallLoadProxy::loadStringFile(const std::string &filename, ucstring &text)
{
	// if info exist reaturns data from memner Data
	std::map<std::string, ucstring>::const_iterator found ( Data.find(filename) );
	if ( found == Data.end())
	{
		text = ucstring("");
		return;
	}
	text = found->second;
}

/// Used to forward log info From patchmanager to IRyzomControlListener (the gui)
class CRyzomInstallStateListener : public IPatchManagerStateListener
{
public:
	CRyzomInstallStateListener(IRyzomControlListener* listener):_Listener(listener){}

	void setState (bool bOutputToLog, const ucstring &ucsNewState);

private:
	IRyzomControlListener* _Listener;
};

void CRyzomInstallStateListener::setState(bool bOutputToLog, const ucstring &ucsNewState)
{
	_Listener->setState(bOutputToLog, ucsNewState.toString().c_str());
}

//-------------------------------------------------

/// Implementation of the IRyzomControlImpl
class CRyzomControlImpl : public IRyzomControl, public IAsyncDownloader
{
public:
	
	CRyzomControlImpl(IRyzomControlListener* listener);
	~CRyzomControlImpl();
	//! load info from cfg file(call by ctor)
	void loadCfg();

	//from IRyzomControl
	//! \see IRyzomControl::destroy
	virtual void destroy(){}
	//! \see IRyzomControl::check
	virtual void check();
	//! \see IRyzomControl::update
	virtual void update();
	// called by PatchManager
	virtual void getPackageSelection(bool mustRepairFirst);
	//! \see IRyzomControl::getDownloadList
	virtual void getDownloadList(IStringContainer* categoryList);	
	//! \see IRyzomControl::install
	virtual void install(IInstallEntryViewer* installList, bool download);
	//! \see IRyzomControl::getServerVersion
	virtual const char* getServerVersion() const;	
	//! \see IRyzomControl::getServerVersion
	virtual const char* getLanguage() const;
	//! \see IRyzomControl::getServerVersion
	virtual const char* getStartupHost() const;
	//! \see IRyzomControl::getServerVersion
	virtual const char* getApplication() const;
	//! \see IRyzomControl::getServerVersion
	virtual bool download(const char* patchPath, const char* sourcePath,
		const char* tmpDirectory, uint32 timestamp);
	//! \see IRyzomControl::requestStop
	virtual void requestStop();
	//! \see IRyzomControl::extract
	virtual bool extract(char* sourceFilename [],
		char* extractPath [],
		unsigned int size);	
	//! \see IRyzomControl::extract
	virtual void initPatchManager();
	//! \see IRyzomControl::extract
	virtual const char* getPatchUrl() const;
	//! \see IRyzomControl::extract
	virtual const char* getLogUrl(const char* action) const;
	//! \see IRyzomControl::getInstallHttpBackground
	virtual const char* getInstallHttpBackground() const;
	//! \see IRyzomControl::setStartRyzomAtEnd
	virtual void setStartRyzomAtEnd(bool ok) { _PatchManager->setStartRyzomAtEnd( ok); }

	//from IAsyncDownloader
	//! \see IRyzomControlListener::addToDownloadList
	void addToDownloadList(const std::string &patchName, const std::string &sourceName, uint32 timestamp, const std::string& extractPath, unsigned int sevenZipSize, unsigned int size);
	//! \see IRyzomControlListener::onFileInstallFinished
	virtual void onFileInstallFinished();
	//! \see IRyzomControlListener::onFileDownloadFinished
	virtual void onFileDownloadFinished();
	//! \see IRyzomControlListener::onFileDownloading
	virtual void onFileDownloading(const std::string& sourceName, unsigned int rate, unsigned int fileIndex, unsigned int fileCount, unsigned long long fileSize, unsigned long long fullSize);
	//! \see IRyzomControlListener::onFileInstalling
	virtual void onFileInstalling(const std::string& sourceName, unsigned int rate, unsigned int fileIndex, unsigned int fileCount, unsigned long long fileSize, unsigned long long fullSize);	
	//! \see IRyzomControlListener::initI18n
	void  initI18n(const char* lang, unsigned char *data, unsigned int length);
	//! \see IRyzomControlListener::fatalError
	virtual void fatalError(const std::string& errorId, const std::string& param1, const std::string& param2);

private:
	enum EUpdaterState  {idle, init, scanData, checkData, patch, finish}; //!< different state of the install process
	EUpdaterState _CurrentState;	//!< The current state of the pach process
	CPatchManager* _PatchManager; //!< The Patch Manager that realy does the pach process
	IRyzomControlListener* _Listener; //!< The listener received controller feed back (the GUI must inherit from this class)
	CInstallLoadProxy*	_InstallLoadProxy;//!< Contains i18n file (needed to "load the translation"
	CRyzomInstallStateListener* _StateListener; //!< The Stats listener receive the log ask from patch manager
	bool _InitPatchManager; //!< true if initPatchManager has been called
	std::string _Language; //!< The language of the install program (from cfg file)
	std::string _StartupHost;//!< The startup host used to have pacth url (from cfg file)
	std::string _StartupPage;//!< The startup host used to have pacth url (from cfg file)
	std::string _InstallStartupHost;//!< used for debug (if we want to use alternate host for client_install.php) (from cfg file)
	std::string _InstallStartupPage;//!< used for debug (if we want to use alternate host for client_install.php) (from cfg file)
	std::string _Application;//!< The Application name of Ryzom(from cfg file)
	std::string _InstallStatsUrl; // the URL of the stats tools eg http://r2linux03/stats/stats.php
};

void CRyzomControlImpl::fatalError(const std::string& errorId, const std::string& param1, const std::string& param2)
{
	//forward message to gui via IRyzomControlListener
	_Listener->fatalError(errorId.c_str(), param1.c_str(), param2.c_str());
}

const char* CRyzomControlImpl::getServerVersion() const
{
	//get infos on version from manager
	return _PatchManager->getServerVersion().c_str();
}

CRyzomControlImpl::CRyzomControlImpl(IRyzomControlListener* listener)
{
	_PatchManager = CPatchManager::getInstance();
	// the name of the file to launch at end of install
	_PatchManager->setRyzomFilename( ".\\client_ryzom_rd.exe" );
	_CurrentState = idle;
	_Listener = listener;
	_InstallLoadProxy = new CInstallLoadProxy();
	_StateListener = new CRyzomInstallStateListener(listener);
	_PatchManager->setStateListener(_StateListener);
	_InitPatchManager = false;
	TheTmpInstallDirectory = _Listener->getTmpInstallDirectory();
	loadCfg();
		
}


CRyzomControlImpl::~CRyzomControlImpl()
{
	delete _InstallLoadProxy;
}


void CRyzomControlImpl::check()
{

	
}


void  CRyzomControlImpl::getPackageSelection(bool mustRepairFirst)
{
	if (_CurrentState == idle)
	{	
		if (mustRepairFirst)
		{
			// launch MD5 integrity check
			_CurrentState  = scanData;
			_PatchManager->startScanDataThread();
		
		}
		else
		{
			// launch size timestamp integrity check
			_CurrentState = checkData;
			_PatchManager->startCheckThread();					
		}
	}
}


void CRyzomControlImpl::getDownloadList(IStringContainer* categoryList)
{
	if (_CurrentState == idle)
	{
		
		_CurrentState = patch;
		std::vector<std::string> categories;
		unsigned int first = 0, last = categoryList->getCount() ;
		for (; first != last; ++first)
		{
			categories.push_back(categoryList->getName(first));
		}
		// Ask the PatchManager to use inform use of progression
		_PatchManager->setAsyncDownloader( this );
		// we add all non optional categories to download
		_PatchManager->startPatchThread(categories);
		
	}
}


void CRyzomControlImpl::update()
{

	if (_CurrentState == idle) { return; }
	
	
	if (_CurrentState == scanData)
	{
		bool ok, end;
		end = _PatchManager->isScanDataThreadEnded(ok);
		// update Gui
		_Listener->updateFuntion(IRyzomControlListener::ScanData, _PatchManager->getCurrentFilesToGet()+1, _PatchManager->getTotalFilesToGet());					
		// if scan Data thread is finished launch Check Data Thread
		if (!end)	{ return; }
		_CurrentState = idle;
		_Listener->onScanDataFinished();
		_CurrentState = checkData;
		_PatchManager->startCheckThread();		
		
		return;
	}
	

	if (_CurrentState == checkData)
	{
		bool ok, end;
		end = _PatchManager->isCheckThreadEnded(ok);
		//update GUI
		_Listener->updateFuntion(IRyzomControlListener::CheckData, _PatchManager->getCurrentFilesToGet()+1, _PatchManager->getTotalFilesToGet());
		// if check Data thread is finished give package list to to Listener
		if (!end)	{ return; }
		CPatchManager::SPatchInfo InfoOnPatch;
		_PatchManager->getInfoToDisp(InfoOnPatch);
		CCheckInfoViewer viewer(&InfoOnPatch);
		_CurrentState = idle;
		_Listener->onCheckFinished(&viewer);
		return;
	}


	if (_CurrentState == patch)
	{
		bool ok, end;
		end = _PatchManager->isPatchThreadEnded(ok);
		//update GUI
		_Listener->updateFuntion(IRyzomControlListener::Patch, _PatchManager->getCurrentFilesToGet()+1, _PatchManager->getTotalFilesToGet());
		// if download is finished infor listener
		if (!end)	{	return;	}
		_CurrentState = idle;
		_Listener->onDownloadListFinished();

	}


}


void CRyzomControlImpl::addToDownloadList(const std::string &patchName, const std::string &sourceName, uint32 timestamp, const std::string& extratPath, unsigned int sevenZipFile, unsigned int size)
{	
	// forward to listener (gui)
	_Listener->addToDownloadList(patchName.c_str(), sourceName.c_str(), timestamp, extratPath.c_str(), sevenZipFile, size);
}


void CRyzomControlImpl::install(IInstallEntryViewer* installList, bool download)
{
	// create list of fil to download / install
	std::vector<CInstallThreadEntry> entries;
	uint32 first =0, last =  installList->getCount();
	for (; first != last; ++first)
	{
		
		entries.push_back( CInstallThreadEntry(
			installList->getPatchName(first),
			installList->getSourceName(first),
			installList->getTimestamp(first),
			installList->getExtractPath(first),
			installList->getSize(first),
			installList->getSZipFileSize(first)
			)
			);
	}

	// Use this list to feed Patch / Install Thread
	if (download)
	{
		_PatchManager->startDownloadThread(entries);
	}
	else
	{
		_PatchManager->startInstallThread(entries);
	}
}


void CRyzomControlImpl::onFileInstallFinished()
{
	// forward from PatchManager to listener (gui)
	_Listener->onFileInstallFinished();
}

void CRyzomControlImpl::onFileDownloading(const std::string& sourceName, uint32 rate, uint32 fileIndex, uint32 fileCount, uint64 fileSize, uint64 fullSize)
{
	// forward from PatchManager to listener (gui)
	_Listener->onFileDownloading(sourceName.c_str(),  rate,  fileIndex,  fileCount,  fileSize,  fullSize);
}

void CRyzomControlImpl::onFileInstalling(const std::string& sourceName, uint32 rate, uint32 fileIndex, uint32 fileCount, uint64 fileSize, uint64 fullSize)
{
	// forward from PatchManager to listener (gui)
	_Listener->onFileInstalling(sourceName.c_str(),  rate,  fileIndex,  fileCount,  fileSize,  fullSize);
}


void CRyzomControlImpl::onFileDownloadFinished()
{
	// forward from PatchManager to listener (gui)
	_Listener->onFileDownloadFinished();
}


// C Callback 
static void requestStopFun()
{	
	CRyzomControlImpl* impl = dynamic_cast<CRyzomControlImpl*>(IRyzomControl::getInstance());	
	impl->requestStop();
}


void CRyzomControlImpl::initPatchManager()
{
	if (_InitPatchManager) { return; }
	// Ask to listener the PatchURI
	std::vector<std::string> patchURIs;
	for ( unsigned int first = 0, last = _Listener->getPatchUriCount(); first != last; ++first)
	{
		patchURIs.push_back(std::string( _Listener->getPatchUriValue(first) ) );
	}

	// Init patch manager with url from login.php
	_PatchManager->init(patchURIs, _Listener->getServerPath(), _Listener->getVersion());
	_InitPatchManager= true;
}
bool  CRyzomControlImpl::download(const char* patchPath, const char* sourcePath,
								  const char* tmpDirectory, uint32 timestamp)
{
	// forward from listener (GUI) to Patchmanager
	if (!_InitPatchManager)
	{
		initPatchManager();
	}
	return _PatchManager->download(std::string(patchPath), std::string(sourcePath),
		std::string(tmpDirectory), timestamp);
}

void CRyzomControlImpl::requestStop()
{
	// forward from Patchmanage to (GUI)
	_Listener->requestStop();
}

bool  CRyzomControlImpl::extract(char* sourceFilename [],
								char* extractPath [],
								unsigned int size) 
{
	// forward from listener (GUI) to patchmanager

	std::vector<std::string> source;
	std::vector<std::string> extract;
	for ( uint32 first = 0 ; first != size; ++ first)
	{
		source.push_back(sourceFilename[first]);
		extract.push_back(extractPath[first]);
	}
	return _PatchManager->extract("tmp", source, extract, "updt_launcher.bat", _Listener->getLauncherName(), requestStopFun);
}


void  CRyzomControlImpl::initI18n(const char* lang, unsigned char *data, unsigned int length)
{
	using namespace NLMISC;
	ucstring result;
	CI18N::readTextBuffer(data, length,  result, false);
	
	_InstallLoadProxy->Data[ std::string(lang) + std::string(".uxt") ] = result;
	// load of a simulated "lang.uxt" trad file passd as argument
	CI18N::setLoadProxy(_InstallLoadProxy);
	CI18N::load(lang);
}


const char * CRyzomControlImpl::getLogUrl(const char* action) const
{
	// create url to stats.php with get argument 
	static std::string buffer;
	buffer = _InstallStatsUrl+ std::string("?cmd=log&msg=") + action;
	return buffer.c_str();
}


const char *CRyzomControlImpl::getPatchUrl() const
{
	// create patch url (login php)
	static std::string buffer;
	buffer = std::string("http://")+_InstallStartupHost + _InstallStartupPage  + std::string("?cmd=get_patch_url&lang=") + _Language + std::string("&domain=") + _Application;
	return buffer.c_str();
}

const char *CRyzomControlImpl::getInstallHttpBackground() const
{
	// create smal background web site url
	static std::string buffer;
	buffer = ClientLauncherUrl + std::string("client_install_")+ _Language + std::string(".htm");
	return buffer.c_str();
}

void CRyzomControlImpl::loadCfg()
{
	NLMISC::CConfigFile		ConfigFile;
	bool ok = true;

	// if already launch destroy cfg file (exedell has been unpack)
	// and create another cleaner

	if (CFile::fileExists("unpack/exedll.bnp"))
	{
		try 
		{
			if (CFile::fileExists("./client.cfg"))
			{
				ConfigFile.load("./client.cfg");
			}
			else
			{
				ok = false;
			}
		}
		catch(...)
		{
			ok = false;
		}
		
		if (ok &&  ConfigFile.getVarPtr("LanguageCode")  )
		{
			_Language = ConfigFile.getVar("LanguageCode").asString(0);
		}
		else
		{
			_Language = "en";
		}
		//Use .cfg value to write to the new clean client.cfg
		{
			std::string cfgContent = std::string("RootConfigFilename   = \"client_default.cfg\";\n") +
			std::string("LanguageCode = \"")+_Language + std::string("\";\n");
			if (ConfigFile.getVarPtr("StartupHost"))
			{
				_StartupHost = ConfigFile.getVar("StartupHost").asString(0);
				cfgContent += std::string("StartupHost = \"")+_StartupHost + std::string("\";\n");
			}

			COFile cfg("client.cfg");
			cfg.serialBuffer((uint8*)cfgContent.c_str(), cfgContent.size());

		}	
		// delete data/*.string_cache files
		{
			std::vector<std::string> vFiles;
			CPath::getPathContent("data", false, false, true, vFiles);				
			for (uint32 i = 0; i < vFiles.size(); ++i)
			{
				if (NLMISC::CFile::getExtension(vFiles[i]) == "string_cache")
				{
					NLMISC::CFile::deleteFile(vFiles[i]);
				}
			}
		}
	}
		
	// normal case load the client.cfg
	try 
	{
		ConfigFile.load("./client.cfg");
	}
	catch(...)
	{
		_Listener->fatalError("uiCfgError" , "", "");
	}

	// load calues from client.cfg
	if (!ConfigFile.getVarPtr("StartupHost") 
		|| !ConfigFile.getVarPtr("Application") 
		|| !ConfigFile.getVarPtr("LanguageCode") 
		|| !ConfigFile.getVarPtr("StartupPage") 
		)
	{
		_Listener->fatalError("uiCfgError" , "", "");
		return;
	}
	_Application = ConfigFile.getVar("Application").asString(0);
	CfgApplication = _Application;
	_StartupHost = ConfigFile.getVar("StartupHost").asString(0);
	_Language = ConfigFile.getVar("LanguageCode").asString(0);	
	_StartupPage = ConfigFile.getVar("StartupPage").asString(0);	

	if ( ConfigFile.getVarPtr("InstallStartupPage") )
	{
		//dev can use specific url for debug
		_InstallStartupPage = ConfigFile.getVarPtr("InstallStartupPage")->asString(0); 
	}
	else
	{
		_InstallStartupPage = "/login2/client_install.php";		
		std::string::size_type pos = _StartupPage.rfind("/");
		if (pos != std::string::npos)
		{
			_InstallStartupPage = _StartupPage.substr(0, pos);
			_InstallStartupPage += "/client_install.php";
		}	
	}
	
	//dev can use specific url for debug
	if ( ConfigFile.getVarPtr("InstallStartupHost") )
	{
		_InstallStartupHost = ConfigFile.getVarPtr("InstallStartupHost")->asString(0); 
	}
	else
	{
		_InstallStartupHost = _StartupHost;
	}

	// Url of the stas.php
	if ( ConfigFile.getVarPtr("InstallStatsUrl") )
	{
		_InstallStatsUrl = ConfigFile.getVarPtr("InstallStatsUrl")->asString(0);
	}
	
	ClientLauncherUrl =std::string("http://")+ _InstallStartupHost; // if not def user same server
}


const char* CRyzomControlImpl::getApplication() const
{
	return _Application.c_str();
}

const char* CRyzomControlImpl::getLanguage() const
{
	return _Language.c_str();
}

const char* CRyzomControlImpl::getStartupHost() const
{
	return _StartupHost.c_str();
}
//---------------------------------------------------------

//!see IRyzomVersionMaker
class CRyzomVersionMaker : public IRyzomVersionMaker
{
private:
	//! Store pach element infos
	class CArchiveFile
	{
	public:
		CArchiveFile(const std::string& filename, uint32 version, bool optional, uint32 timestamp)
			:BaseName(filename), Version(version), Optional(optional), Timestamp(timestamp){}			

		std::string BaseName;
		uint32 Version;
		uint32 Timestamp;
		bool Optional;

	};

	//! Sort patch element by category (optional and by timestamp)
	class CArchiveFileSortFunc
	{
	public:
		bool operator ()(const CArchiveFile* lf, const CArchiveFile* rh) const
		{
			if ( lf->Optional != rh->Optional)
			{
				return lf->Optional;
			}
			return lf->Timestamp < rh->Timestamp;
		}
	};

public:
	static CRyzomVersionMaker* getInstance();

	static void releaseInstance();

	//! \see RyzomVersionMaker::setLog
	virtual void setLog( void (*log)(const char*)) 
	{
		_Log = log;
	}

	//! \see RyzomVersionMaker::create
	virtual void create(const char* idxFilename, const char* archivefilename, IArchiveMakerListener* listener);		
	
private:
	static CRyzomVersionMaker* _Instance;
	void (*_Log)(const char*);
};

CRyzomVersionMaker* CRyzomVersionMaker::_Instance = 0;

void CRyzomVersionMaker::create(const char* idxFilename, const char* archivefilename, IArchiveMakerListener* listener)
{

	typedef std::vector<CArchiveFile*> TArchives;
	TArchives Archives;
	CProductDescriptionForClient client;
	//load version description file
	client.load(idxFilename);

	{
		const CBNPFileSet &fileset = client.getFiles();
		//for each category
		const CBNPCategorySet & categorySet = client.getCategories();
		uint32 first = 0, last = categorySet.categoryCount();
		for (; first != last; ++first)
		{
			// for each file of a category
			const CBNPCategory& category = categorySet.getCategory(first);
			uint32 first2=0, last2=category.fileCount();
			for (;first2 != last2; ++first2)
			{
				// get the last VersionInfo of the file
				const std::string& filename = category.getFile(first2);
				const CBNPFile* filePtr = fileset.getFileByName(filename);
				if (filePtr)
				{
					uint32 versionCount = filePtr->versionCount();	
					if ( versionCount != 0)
					{
						const CBNPFileVersion& version = filePtr->getVersion(versionCount - 1);
					// add file info to the Archive list
						Archives.push_back( new CRyzomVersionMaker::CArchiveFile(filename, version.getVersionNumber(), category.isOptional(), version.getTimeStamp()));					
					}
					else
					{
						if (_Log)
						{
							std::string msg = NLMISC::toString("\"%s\" has no version but is member of categories \"%s\"...", filename.c_str(), category.getName().c_str());
							_Log(msg.c_str());
						}
					}
				}
				else
				{					
					if (_Log)
					{
						std::string msg = NLMISC::toString("\"%s\" is in category but not in fileset", filename.c_str() );
						_Log(msg.c_str());
					}
				}
			}
		}
	}
	// sort the Archive list by category and timestamp (Archive is download by chunk so we try that the player has linear need)
	std::stable_sort(Archives.begin(), Archives.end(), CRyzomVersionMaker::CArchiveFileSortFunc());
	listener->beginArchive(archivefilename);
	{
		
		TArchives::const_iterator first=Archives.begin(), last = Archives.end();
		for (; first != last; ++first)
		{
			listener->addArchiveElement((*first)->BaseName.c_str(), (*first)->Version, (*first)->Optional, (*first)->Timestamp);
		}
	}
	{
		TArchives::iterator first=Archives.begin(), last = Archives.end();
		for (; first != last; ++first)
		{
			delete *first;
		}
	}
	listener->endArchive();

}

	
CRyzomVersionMaker* CRyzomVersionMaker::getInstance()
{	//singleton
	if (!_Instance){ _Instance = new CRyzomVersionMaker(); }
	return _Instance;
}

 void CRyzomVersionMaker::releaseInstance()
{
	// release singleton
	if (_Instance)
	{
		delete _Instance;
		_Instance = 0;
		delete ApplicationContext;
	}
}

IRyzomVersionMaker* IRyzomVersionMaker::getInstance()
{
	return CRyzomVersionMaker::getInstance();
}

void IRyzomVersionMaker::releaseInstance()
{
	CRyzomVersionMaker::releaseInstance();
}

IArchiveMakerListener::~IArchiveMakerListener(){}
 //-----------------------------------------------------------
//!Forward CSystemInfo:: function
//!{
const char* INelControl::getOS ()
{
	static std::string ret;
	ret =	CSystemInfo::getOS();
	return ret.c_str();
}

const char* INelControl::getProc ()
{
	static std::string ret;
	ret =	CSystemInfo::getProc();
	return ret.c_str();
}

const char* INelControl::availableHDSpace ()
{
	static std::string ret;
	ret =	CSystemInfo::availableHDSpace(".");
	return ret.c_str();
}

unsigned int INelControl::availablePhysicalMemory ()
{
	return CSystemInfo::availablePhysicalMemory();
}

const char*  INelControl::getVideoInfoDeviceName ()
{	
	static std::string ret;
	unsigned long long version;
	ret = "";
	bool ok = CSystemInfo::getVideoInfo(ret, version);
	if (ok)
	{
		return ret.c_str();
	}
	return "";
}

unsigned long long INelControl::driverVersion()
{
	static std::string ret;
	unsigned long long version=0;
	ret = "";
	bool ok = CSystemInfo::getVideoInfo(ret, version);
	if (ok)
	{
		return version;
	}
	return 0;
}
//!}
//-----------------------------------------------------------
IRyzomControl* IRyzomControl::init(IRyzomControlListener* listener)
{
	ApplicationContext = new NLMISC::CApplicationContext();
	_Instance = new CRyzomControlImpl(listener);
	return _Instance;
}


IRyzomControl* IRyzomControl::_Instance = 0;
