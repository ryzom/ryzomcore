/** \file backup_service.h
 * Temporary backup service for player character save file
 *
 */



#ifndef BACKUP_SERVICE_H
#define BACKUP_SERVICE_H


// net
#include "nel/misc/types_nl.h"
#include "nel/misc/log.h"
#include "nel/net/service.h"
#include "nel/misc/variable.h"

// stl
#include <string>

//
#include "backup_file_access.h"


/**
 * CBackupService
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2004
 */
class CBackupService : public NLNET::IService
{
public :
	/** 
	 * init the service
	 */
	void init();

	/**
	 * main loop
	 */
	bool update();
	
	/**
	 * release
	 */
	void release();

	// stall shard
	void stallShard(const std::string& fileName);

	/**
	 * get Instance
	 */
	static CBackupService * getInstance() { return (CBackupService *) NLNET::IService::getInstance(); }

	/**
	 * get stall, return true if we must not try to save file because previous pb are occurs
	 */
	bool getStall() { return _SaveStall; }

	/**
	 * set stall mode
	 */
	void setStall( bool stall ) { _SaveStall = stall; }

	NLNET::IModule		*getBackupModule()	{ return _BackupModule; };

	void				setBackupModule(NLNET::IModule *module)	{ _BackupModule = module; }

	void		onModuleDown(NLNET::IModuleProxy *proxy);

public:


	CFileAccessManager	FileManager;

private:

	bool		_SaveStall;

	NLNET::TModulePtr		_BackupModule;

	// A layer 3 server to serve synchronous file load
	NLNET::CCallbackServer	*_CallbackServer;
}; 


extern NLMISC::CVariable<std::string>	StatDirFilter;


class CDirectoryRateStat
{
public:

	CDirectoryRateStat()
	{
		clear();
	}

	void	clear();

	void	readFile(const std::string& filename, uint32 filesize);
	
	void	writeFile(const std::string& filename, uint32 filesize);

	uint	getMeanReadRate();

	uint	getMeanWriteRate();

	void	display(NLMISC::CLog& log);

private:

	struct CDirectory
	{
		struct CFileQueueEntry
		{
			CFileQueueEntry(NLMISC::TTime time, uint32 size, const std::string& filename = "", uint32 duration = 0) : ActionTime(time), Size(size), Filename(filename), Duration(duration)	{ }
			NLMISC::TTime	ActionTime;
			uint32			Size;
			std::string		Filename;
			uint32			Duration;
		};

		typedef std::deque<CFileQueueEntry>	TFileQueue;

		// read stats
		TFileQueue		ReadQueue;
		uint64			ReadBytes;
		uint32			ReadFiles;

		// write stats
		TFileQueue		WrittenQueue;
		uint64			WrittenBytes;
		uint32			WrittenFiles;

		CDirectory()
		{
			clear();
		}

		void	clear()
		{
			ReadBytes = 0;
			ReadFiles = 0;

			WrittenBytes = 0;
			WrittenFiles = 0;
		}

		void	read(NLMISC::TTime now, uint32 filesize)
		{
			updateTime(now - 60*1000);
			ReadBytes += filesize;
			++ReadFiles;
			ReadQueue.push_back(CFileQueueEntry(now, filesize));
		}

		void	write(NLMISC::TTime now, uint32 filesize)
		{
			updateTime(now - 60*1000);
			WrittenBytes += filesize;
			++WrittenFiles;
			WrittenQueue.push_back(CFileQueueEntry(now, filesize));
		}

		void	updateTime(NLMISC::TTime limit)
		{
			while (!WrittenQueue.empty() && WrittenQueue.front().ActionTime < limit)
			{
				WrittenBytes -= WrittenQueue.front().Size;
				--WrittenFiles;
				WrittenQueue.pop_front();
			}
			while (!ReadQueue.empty() && ReadQueue.front().ActionTime < limit)
			{
				ReadBytes -= ReadQueue.front().Size;
				--ReadFiles;
				ReadQueue.pop_front();
			}
		}
	};


	// mapping to filtered directories
	typedef std::map<std::string, CDirectory>	TDirectoryMap;
	TDirectoryMap						_DirectoryMap;

};



#endif // BACKUP_SERVICE_H

/* End of backup_service.h */
