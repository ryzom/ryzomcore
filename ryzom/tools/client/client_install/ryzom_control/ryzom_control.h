#ifndef __RYZOM_CONTROL_H__
#define __RYZOM_CONTROL_H__


#ifdef RYZOM_CONTROL_BUILD
	#define RYZOM_CONTROL_EXPORT __declspec(dllexport)
#else
	#define RYZOM_CONTROL_EXPORT __declspec(dllimport)
#endif

/** This class must be use by module outside the dll to access Ryzom Specific info on patch.
Class with stl object is not used. Becaus stl can have different definition between 2 dll
*/
class   RYZOM_CONTROL_EXPORT ICkeckInfoViewer
{
public:
	virtual ~ICkeckInfoViewer(){}	
	enum ECat { NonOptCat, ReqCat, OptCat };

public:
	/** Returns the number of item of a category
	eg	getCount(ICkeckInfoViewer::ReqCat) return the number of element for the category required
	*/
	virtual unsigned int getCount(ECat cat) const = 0;

	/**	Returns the name (by example exe_dll.bnp) of the patch element
	\param cat is the category (can be option, non optional or required)
	\index is the index of the element (starting at zero finishing at this->getCount(cat))
	*/
	virtual const char * getName(ECat cat, unsigned int index) const = 0;

	/**	Returns the seven_zip size of the pach element (by example exe_dll.bnp.lzma)
	\param cat is the category (can be option, non optional or required)
	\index is the index of the element (starting at zero finishing at this->getCount(cat))
	*/
	virtual unsigned int getSZipFileSize(ECat cat, unsigned int index) const = 0;

	/**	Returns the size of the pach element after depack (size of exe_dll.bnp)
	\param cat is the category (can be option, non optional or required)
	\index is the index of the element (starting at zero finishing at this->getCount(cat))
	*/
	virtual unsigned int getFileSize(ECat cat, unsigned int index) const = 0;
};

/** Module outside of the dll must inherit of this class in order to knwo the current state of the download.*/
class   RYZOM_CONTROL_EXPORT IRyzomControlListener
{
public:

	/* Different stats of the download process (ScanData is the md5 integrity test,
		CheckData is the size + timestamp integrity testpach is downloading
		*/
	enum TUpdateType { ScanData, CheckData, Patch, Install };

public:
	virtual ~IRyzomControlListener(){}	
	/** Called when the check thread finished the viewer param enable the inherited class to know the different package that need to be patch
	\param viewer The list of package that must be patched
	*/
	virtual void onCheckFinished(ICkeckInfoViewer* viewer){}

	/** Called when scan data is finished (scan data is the md5 integrity test
	*/
	virtual void onScanDataFinished() {}

	/** Called when all the file that need to be downloaded are finished
	*/
	virtual void onDownloadListFinished(){}

	/** A file has been added to the download list (will be downlaod via patch system)
		\param patchName The name of the patch "/0012/exe_dll.bnp.lzma"
		\param sourceName The name of the sourceName "data/exe_dll.bnp"
		\param timestamp The timestamp to apply to the file after install
		\param extractPath if present the directory to extract data (eg "unpack/")
		\param sZipFileSize The size of the lzma file
		\param size The size of the bnp file		
	*/
	virtual void addToDownloadList(const char* patchName, const char* sourceName, unsigned int timestamp, const char* extractPath, unsigned int sZipFileSize, unsigned int size){}

	/** Called when the all file have been installed
	*/
	virtual void onFileInstallFinished(){}

	/** Called periodicaly (at each new download) to indicate the state of the downlaod progress
	\param sourceName the name of the file that is downloading
	\param rate The amount of byte by second of the process
	\param fileIndex The index of the file (0 to fileCount)
	\param fileCount The number of file to process
	\param fileSize The size of data processed
	\param fullSize The total amount of data to process
	*/
	virtual void onFileDownloading(const char* sourceName, unsigned int  rate, unsigned int  fileIndex, unsigned int  fileCount, uint64  fileSize, uint64 fullSize){}

	/** Called periodicaly (at each new install) to indicate the state of the install progress
	\param sourceName the name of the file that is installing
	\param rate The amount of byte by second of the process
	\param fileIndex The index of the file (0 to fileCount)
	\param fileCount The number of file to process
	\param fileSize The size of data processed
	\param fullSize The total amount of data to process
	*/
	virtual void onFileInstalling(const char* sourceName, unsigned int  rate, unsigned int  fileIndex, unsigned int  fileCount, uint64  fileSize, uint64 fullSize){}
	
	/** Called at the end of the downlaod process
	*/
	virtual void onFileDownloadFinished(){}

	/** Returns the quantity of Uri given by the login process
	*/

	virtual unsigned int getPatchUriCount() const= 0;
	/** Must return the a patch Uri by its index.
	*/
	virtual const char * getPatchUriValue(unsigned int index) const = 0;
	
	/** Must return the pas path to the patch backup server
	*/
	virtual const char* getServerPath() const = 0;

	/** Must return the version number as  string (eg "187")
	*/
	virtual  const char * getVersion() const = 0;

	/** Must return the name of the laucher application "client_install.exe"
	*/
	virtual  const char * getLauncherName() const = 0;

	/** Must return the name of the laucher application "client_install.exe"
	*/
	virtual  const char * getTmpInstallDirectory() const = 0;

	/** The download/install process has been stoped
	*/
	virtual void requestStop() = 0;

	/** Display a warning / info message (from the patch system)
	\param bOutputToLog True if the string must be display (warning), false if not (infod / debug)
	*/
	virtual void setState (bool bOutputToLog, const char* newState) = 0;

	/** Called by the ScanData / CheckData process to indicate the state of progress
		\param updateType  can be ScanData, CheckData
		\param index The index of the file processed
		\param max The number of file to process
	*/
	virtual void updateFuntion(TUpdateType updateType, unsigned int index, unsigned int max) = 0;

	
	/** Called to indicate an error
	\param  msg is a i18n error msg
	\param param1 A error specific msg (empty or the argument to the format string given by the translation of msg)
	\param param1 same as param1		
	*/
	virtual void fatalError(const char* msg, const char* param1, const char* param2) = 0;

};
	
/** Interface to access to patch infos.
A concret class with stl struct could create issues if used between dll compiled with different options
*/
class   RYZOM_CONTROL_EXPORT IInstallEntryViewer
{
public:
	virtual ~IInstallEntryViewer(){}	
	//! Returns the number of entry in the patch list
	virtual unsigned int getCount() const = 0;
	//! Returns the name of a patch (.lzma)entry (from the patch list)
	virtual const char * getPatchName(unsigned int index) const = 0;	
	//! Returns the name of a source (.bnp) (from the patch list)
	virtual const char * getSourceName(unsigned int index) const = 0;	
	//! Returns the timestamp of the file
	virtual unsigned int getTimestamp(unsigned int index) const = 0;	
	//! Returns the path where the file must be extract b.e. "data/" (or empty string)
	virtual const char * getExtractPath(unsigned int index) const = 0;	
	//! Returns the size of the bnp file
	virtual unsigned int getSize(unsigned int index) const = 0;	
	//! Returns the size of the lzma file
	virtual unsigned int getSZipFileSize(unsigned int index) const = 0;	

};
/** Must be inherited to implement "string" container
A concret class with stl struct could not be usedbecause it can create issues if used between dll compiled with different options
*/

class   RYZOM_CONTROL_EXPORT IStringContainer
{
public:
	virtual ~IStringContainer(){}
	//! Returns the size of the container
	virtual unsigned int getCount() const = 0;

	/** Return the value of the string conainer at specified index
	The function is called getName and not getValue because its only used is to hold a list of filename
	\param the index to get the file (between 0 and this->getCount())
	*/
	virtual const char * getName(unsigned int index) const = 0;	
};

/** This class is the entry point to control the Ryzom Patching process
At the init function a listener is passed to have feedback from the controller
*/
class   RYZOM_CONTROL_EXPORT IRyzomControl
{


public:
	virtual ~IRyzomControl(){}	
	/**	Initialize the controller with a feedback class (listener) and return the instance
	*/
	static IRyzomControl* init(IRyzomControlListener* listener);

	/** Return the instance of the singleton (must be initialized with init becfore)
	*/
	static IRyzomControl* getInstance() { return _Instance; }

	//! obsolete?
	virtual void destroy() = 0;

	//!obsolete?
	virtual void check() = 0;

	//! Must be called at each tick (called back the listener  to indicate the state of the progression)
	virtual void update() = 0;

	/** Ask the instance to call the listener back with the package list in function of the data present on the disk ("matis_island",...)
	\param mustRepair True if a md5 check must be done on present file
	*/
	virtual void getPackageSelection(bool mustRepair) = 0;

	/** Ask the instance to call the listener back with the list of file to download
	\pram categoryList The list of category we wants to download  (given via getPackageSelection)
	*/
	virtual void getDownloadList(IStringContainer* categoryList) = 0;

	/** Indicates if we want to start ryzom at the end of install.
	A .bat file is created to unpack data. The module want to know if he must add a start client_ryzom_rd.exe at the end of the .bat file
	\pram ok True if we want to start Ryzom client at the end of the install process.
	*/ 
	virtual void setStartRyzomAtEnd(bool ok) = 0;

	/** Ask the instance to downlaod or start the installation of a patch file list
	\parm isntallList The list of file to install/download
	\pram download if false indicates that the download is also needed
	*/
	virtual void install(IInstallEntryViewer* installList, bool download = false) = 0;


	/** Called by patch Thread to indicate the state of the downlaod process 
	It will call  back the listener function.
	\see CRyzomListener::onDownloadProgress
	*/
	virtual bool download(const char* patchPath, const char* sourcePath,
		const char* tmpDirectory, unsigned int timestamp) = 0;

	/** Called by install Thread to apply the unpack of data 
	\param sourceFilename The list of file name of file to unpaack
	\parma extractPath A list of extract path (or empty string)
	\pram siez the number of entry of *BOOTH* lists
	*/
	virtual bool extract(char* sourceFilename [], char* extractPath [],	unsigned int size) = 0;

	/**	Called by Install dll to indicates to the listener that the install lise finished
	*/
	virtual void requestStop() = 0;
	
	/** Init the i18n subsystem using Ryzom .uxt file content
	\param language is the langage to read data eg "fr"
	\param data is the content of a .uxt i18n file
	\param length Is the size of data of the i18n file
	*/
	virtual void initI18n(const char* language, unsigned char *data, unsigned int length) = 0;

	/** When called ask the system to read i18n file and PatchUri path
	*/
	virtual void initPatchManager() = 0;

	/** Returns the StartupHost (from the config file)
	*/
	virtual const char* getStartupHost() const = 0;

	/** Returns the Application (from the config file)
	*/
	virtual const char* getApplication() const = 0;
	
	/** Returns the language code (from the config file)
	*/
	virtual const char* getLanguage() const = 0;

	/** Return the version of the server as string eg "188"
	*/
	virtual const char* getServerVersion() const = 0;	

	/** Return the backup patch url
	*/
	virtual const char* getPatchUrl() const = 0;

	/** Return the url of the php stat website (eg http://r2linux03/stats/starts.php?action=log&...
	\param action is the action to execute on the website
	*/
	virtual const char* getLogUrl(const char* action) const = 0;
	
	/** Gets the url of the small website display while the download process (eg http://dl1.gfsrv.net:40916/installer/client_install_fr.htm)
	*/
	virtual const char* getInstallHttpBackground() const = 0;
	
private:
	/** The instance of the module 	
	*/
	static IRyzomControl* _Instance; 
};


/** Used by the archive creation process to control the creation of a new archive
The only use of this class for the moment is CTorrentMakerListener that enable the creation of .torrent file
*/
class RYZOM_CONTROL_EXPORT IArchiveMakerListener
{
public:
	//! \see CTorrentMakerListener::beginArchive
	virtual void beginArchive( const char* outputfile) = 0;

	//! \see CTorrentMakerListener::addArchiveElement
	virtual void addArchiveElement(const char* filename, unsigned int version, bool optional, unsigned int timestamp) = 0;
	
	//! \see CTorrentMakerListener::endArchive
	virtual void endArchive() = 0;	
	
	//! \see CTorrentMakerListener::setLog
	virtual void setLog( void (*log)(const char*)) = 0;
	
	virtual ~IArchiveMakerListener();
};


/** Used to create new archive with file sorted by category and timestamp to let the possibilit to be
able to download only chunck of data need for a patch (in case of using the torrent protocol)
*/
class RYZOM_CONTROL_EXPORT IRyzomVersionMaker
{
public:
	virtual ~IRyzomVersionMaker(){}

	//! Gets the instance of the archive creator
	static IRyzomVersionMaker* getInstance();
	//! Release the isntance of the archive creator

	static void releaseInstance();
	

	/** creat a new archive (using idx file to knwo the list of file of the archive)
	\param idxFilename The name of the idx file used to have the list.
	\param archiveName The name of the archive to create
	\param listener The file that realy create the archive
	*/
	virtual void create(const char* idxFilename, const char* archivefilename, IArchiveMakerListener* listener)  = 0;
	
	//! set a log function
	virtual void setLog( void (*log)(const char*)) = 0;

};

/** Get the access to nel info on os, proc, memory and others
Used for class that do not realy want to link with nel (eg the installer)
*/
class RYZOM_CONTROL_EXPORT INelControl
{
public:
	//! see INelControl::getOS
	static const char* getOS ();
	//! see INelControl::getProc
	static const char* getProc ();
	//! see INelControl::availableHDSpace
	static const char*  availableHDSpace ();
	//! see INelControl::availablePhysicalMemory
	static unsigned int availablePhysicalMemory ();
	//! see INelControl::getVideoInfoDeviceName
	static const char*  getVideoInfoDeviceName ();
	//! see INelControl::driverVersion
	static uint64 driverVersion();

};
#endif
