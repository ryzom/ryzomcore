#ifndef __TORRENT_CONTROL_H__
#define __TORRENT_CONTROL_H__

#include "libtorrent/config.hpp"

//This DLL is use for bittorent protocl management (from install software)

/** This abstract class defines interface to access to a list of file to patch and to set the size of a file that must be patched
A simple stl struct can not be used because it can have differente implementation from two different dll
- torrent_control.dll <=> is not using stl_port (because does not works with torrent header)
- ryzom_control.dll <=> is using stl_port
*/
class   TORRENT_EXPORT ITorrentStringContainer
{
public:
	virtual ~ITorrentStringContainer(){}
	// get the size of the container
	virtual unsigned int getCount() const = 0;
	//get an element of the container (the function is named getName (because it store patch filename list
	virtual const char * getName(unsigned int index) const = 0;	
	//set The size of the file that is stored at position index
	virtual void setSize(unsigned int index, unsigned int size) = 0;
};


/*
This class must be inherited by a GUI class to display the progress info of a torrent download
This class must be inherited from a Controler class to change the default port and upload bittrate limit
*/
class TORRENT_EXPORT ITorrentListener
{
public:
	virtual ~ITorrentListener(){}	
	/// defines the first port to listen (torrent specific protocol)
	virtual int getFirstPort() const {return 6881; }
	/// defines the last port to listen (torrent specific protocol)
	virtual int getLastPort() const { return 6889; }
	/// defines the upload rate limit (0 means no limit)
	virtual int getUploadRateLimit() const { return 20*1024; }	
	/// periodicaly called to inform of download progress
	virtual void progressInfo(const char* msg){}
	/// called when download is finished (file is complete)
	virtual void torrentFinished() {}
	/// should be defined by inherited class to set the directory where torrent tmp file are save (eg patch/tmp)
	virtual const char * getTorrentTmpDir() const =0;

};

/**
This class is used to control torrent protocol.
An implementation of this interface is given by getInstance method
This singleton is used to control the bittorrent protoocol.

*/
class   TORRENT_EXPORT ITorrentControl
{
public:
	virtual ~ITorrentControl(){}

	//! get the instance of the singleton
	static ITorrentControl* getInstance();
	/** start the download of a torrent file. Do not download from the torrent file that are not in allewedFile
		\param torrentFilename The name of the torrent file
		\param allowedFile The list of file from the torrent file that we want to download
	*/
	virtual void startDownloading(const char* torrentFilename, ITorrentStringContainer* allowedFiles) = 0;
	
	/**
	Returns true if we are downloading a file
	*/
	virtual bool isCurrentDownloadValid() const = 0;

	/**
	Returns the name of the file we are currently downloading (isCurrentDownloadValid must be true) 
	*/
	virtual const char * getCurrentDownloadName() const = 0;

	/**
	Returns the state of the current download.
	Values can be "uiQueued", "uiChecking", "uiConnecting", "uiDownloadingMetadata"
		, "uiDownload", "uiFinished", "uiSeeding", "uiAllocating".
	*/
	virtual const char* getCurrentDownloadStateLabel() const = 0;

	/** Returns the progress percent of the current action (between 0 and 100)
	*/
	virtual unsigned __int64 getCurrentDownloadProgress() const = 0;

	/** Returns the Download Rate of the current download (octet by second)
	File must be valid.
	*/
	virtual float getCurrentDownloadDownloadRate() const = 0;

	/** Returns the quantity of data that is already downloaded
	*/
	virtual unsigned __int64 getCurrentDownloadTotalDone() const = 0;
	
	/** Returns the quantity of data that is needed
	*/
	virtual unsigned __int64 getCurrentDownloadTotalWanted() const = 0;

	/** Must be called at each tick (for udating gui)
	*/
	virtual void update()= 0;	

	/** Set the listener that is callback by this module to display the download progression
	*/
	virtual void setListener(ITorrentListener* listener) = 0;

	/** obsolete
	*/
	virtual void updateEntriesSize(const char* torrentFilename, ITorrentStringContainer* allowedFiles) = 0;
};

#endif
