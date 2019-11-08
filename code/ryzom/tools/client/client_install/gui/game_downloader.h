#ifndef GameDownloader_H
#define GameDownloader_H

#using <System.dll>

#include <string>
#include <vector>

/*
Core of the install program  Exchanges messages with Ryzom Interface, Torrent Interface and Gui
Use get getInstance to have a usefull instance
*/
class IGameDownloader
{
public:
	//! The differente state Software
	enum EState {Init, InitFinished, UpdateVersion, UpdateVersionFinished, UpdateLauncher, UpdateLauncherFinished, UpdateTorrent, UpdateTorrentFinished,
		UpdatePackage, UpdatePackageFinished, 
		CheckFinished, RepositoryUpdated,  Downloading,  DownloadFinished, AskInstalling, InstallOk, Installing, AskInstallFinishing, InstallFinishing, InstallFinished};

public:

	virtual ~IGameDownloader(){}

	//Get the instance of the manager
	static IGameDownloader* getInstance();

	// Ask the Ryzom Dll to look at wich package are available for download
	// if must Repare is true do a scan_data before a check
	//IRyzomUpdateListener::onCheckFinished indicates which package are available
	//IRyzomUpdateListener::onScanDataFinished indicates that the scan is finished
	virtual void getPackageSelection(bool mustRepair) = 0;

	// Ask the Ryzom Dll the list of file that must be downloaded
	// selectPackage must be called before the answser is done via IRyzomUpdateListener::addToDownloadList and IRyzomUpdateListener::onDownloadListFinished 
	virtual void getDownloadList() = 0;

	//! must be called at each tick (update gui on exectue asynchronous message)
	virtual void update() = 0;

	//! set the form the core is updating
	virtual void setForm(System::Windows::Forms::Form^ form) = 0;

	//! set the current Version option selected by the user (full Version or initial version)
	virtual void selectPackage(bool fullvesion) = 0;
	
	// set Infos fore where to look for data (this infos are read from a version.xml file generate by php/db)
	// versionId is an optional value mean to download a specifc value
	virtual void setVersionInfo(const std::vector<std::string>& patchUris, const std::string& serverPath) = 0;	

	//! Used to update the launcher (depack the .bnp) *NIY*
	virtual bool updateLauncher() = 0;

	//! load a .torrent from the server
	virtual bool updateTorrent() = 0;

	//! set the name of the torrentFile to read, the tempory patch directory, the timestamp of the torrent file	
	virtual void setTorrentInfo(const std::string & torrentPatchFilename,
		const std::string & torrentFilename,
		unsigned int  torrentPatchTimestamp) = 0;

	//! set the name of the torrentFile to read, the tempory patch directory (usefull for the updating of the .torrent)
	virtual void setInstallInfo(const std::string & installPatchFilename,
		const std::string & installFilename, const std::string & installPatchExtract, 
		unsigned int torrentPatchTimestamp) = 0;
	
	//! forward message to IRyzomControl for initializing the i18n of the ryzom dll
	virtual void initI18n(const char* lang, unsigned char *data, unsigned int length) = 0;
		
	//! Returns the current language from cfg file (forward to IRyzomControl)
	virtual std::string getLanguage() const = 0;

	//! Returns the patch Url from patch serveur (forward to IRyzomControl)
	virtual std::string getPatchUrl() const = 0;

	//! see IRyzomControl::getLogUrl
	virtual std::string getLogUrl(const std::string& action) const = 0;

	//! Set if it's the first install or not (by looking if the unpack/exedll.bnp file exist)
	virtual void setFirstInstall(bool firstInstall)  = 0;
	//! Get the first install value (was set by setFirstInstall)
	virtual bool getFirstInstall() const = 0;

	//! return the size of the Ros version patch		
	virtual unsigned long long  getRosPatchSize() const = 0;

	//! return the size of the Full version patch
	virtual unsigned long long  getRyzomPatchSize() const = 0;

	//! return the size of the Ros version when depacked
	virtual unsigned long long  getRosInstallSize() const = 0;

	//! return the size of the full version when depacked
	virtual unsigned long long  getRyzomInstallSize() const = 0;

	//! \see IRzyomControl::getInstallHttpBackground
	virtual std::string getInstallHttpBackground() const = 0;
	
	//! return the current state
	virtual EState getState() const = 0;
	//! update the current state(change are taken at the update function)
	virtual void setState(EState state) = 0;
	

	//! Set the session id (from stats.php)
	virtual void setSessionId(const std::string& sessionId) = 0;

	//! Set the install id (from registree)
	virtual void setInstallId(const std::string& productId) = 0;

	//! Set the user id (from stats.php)
	virtual void setUserId(const std::string& clientId) = 0;	

	//! obsolet
	virtual void setAutoInstall(bool autoInstall) = 0;
	//! obsolete
	virtual bool getAutoInstall() const = 0;
	
	//! indicate if welcom message has been displayed
	//!{
	virtual bool getStarted() const = 0;
	virtual void setStarted(bool started) = 0;
	//!}

	/* indicates if Ryzom must be start at end
	It store the gui state. And send the result to Ryzom module
	{
	*/
	virtual bool getStartRyzomAtEnd() const = 0;
	virtual void setStartRyzomAtEnd(bool ok) = 0;
	//!}

	//! obsolete {
	virtual void setDownloadOnly(bool downloadOnly) = 0;
	virtual bool getDownloadOnly() const = 0;
	//!}

	//! indicates if the full version is needed (store gui update, then send to ryzom/torrent module {
	virtual void setUseFullVersion(bool downloadOnly) = 0;
	virtual bool getUseFullVersion() const = 0;
	//! }

	//! inidicates if a torrent exist on server. If it exist a combo box must be displayed. {
	virtual void setTorrentExist(bool exist) = 0;
	virtual bool getTorrentExist() const = 0;
	//! }

	//! inidicates if a torrent is used (the torrent exist and the user has selected it) {
	virtual void setUseTorrent(bool useTorrent) = 0;
	virtual bool getUseTorrent() const = 0;
	//!}
	
	
private:
	static IGameDownloader* _Instance; //!instance of the Gui
};

#endif
