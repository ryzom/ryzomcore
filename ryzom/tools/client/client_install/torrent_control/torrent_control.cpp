
#include "torrent_control.h"

#include <string>
#include <vector>
#include <map>
#include <set>

#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>



#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/identify_client.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/ip_filter.hpp"

#include "ryzom_control/ryzom_control.h"
//-------------------------------------------
/**
Implementation of ITorrentControl
\see ITorrentControl
*/
class CTorrentControl : public ITorrentControl
{
public:
	CTorrentControl()
	{
		_Listener = 0;
		_Session = 0;
	}

	/// \see ITorrentControl::updateEntriesSize (obsolete)
	void updateEntriesSize(const char* torrentFilename, ITorrentStringContainer* allowedFiles)
	{
		std::map<std::string, unsigned int> entries;

		unsigned int first = 0, last = allowedFiles->getCount();
		for (; first != last; ++first)
		{
			std::string name  = (allowedFiles->getName(first));				
			entries.insert( std::make_pair(name, first) );
		}
		
		std::ifstream in(torrentFilename, std::ios_base::binary);
		std::vector<char> data;

		while (!in.eof())
		{
			data.push_back(in.get());
		}
		libtorrent::entry e = libtorrent::bdecode( data.begin(), data.end());
		libtorrent::torrent_info ti(e);
		// read file size from meta info and set allowed file
		for (libtorrent::torrent_info::file_iterator i = ti.begin_files();
				i != ti.end_files(); ++i)
		{
			std::map<std::string, unsigned int>::iterator found = entries.find(std::string(_Listener->getTorrentTmpDir()) + i->path.string());
			if ( found != entries.end())
			{
				allowedFiles->setSize( found->second, i->size);
			}
		}					
	}

	/// \see ITorrentControl::startDownloading
	void startDownloading(const char* torrentFilename, ITorrentStringContainer* allowedFiles)
	{
		assert(_Listener);
		assert(torrentFilename);	
		// if allowedFiles is true we filter files otherwise all file are accepted
		if (allowedFiles)
		{
			_Filter = true;
			unsigned int first = 0, last = allowedFiles->getCount();
			for (; first != last; ++first)
			{
				std::string name  = (allowedFiles->getName(first));				
				_AllowedFileSet.insert(name);
			}			
		}
		else
		{
			_Filter = false;
		}

		// allowedFiles == 0 means all files are allowed
		try {
			// start a new session
			_Session = new libtorrent::session();
			// listen port 6881 to 6889
			_Session->listen_on(std::make_pair(_Listener->getFirstPort(), _Listener->getLastPort()));

			//ses.set_max_half_open_connections(-1);
			//ses.set_download_rate_limit(-1);
			//_Session->set_upload_rate_limit(_Listener->getUploadRateLimit()); // <=> 20 Kb of upload

			//libtorrent::session_settings settings;
			//_Session->set_settings(settings);
			//_Session->set_severity_level(libtorrent::alert::debug);
			_Session->set_severity_level(libtorrent::alert::info);
			

			// read the torrent file
			std::ifstream in(torrentFilename, std::ios_base::binary);

			if (!in.good())
			{
				_Listener->progressInfo("Torrent File not found");				
				return;
			}
			std::vector<char> data;

			while (!in.eof())
			{
				data.push_back(in.get());
			}
			// decode the torrent file
			libtorrent::entry e = libtorrent::bdecode( data.begin(), data.end());
			libtorrent::torrent_info ti(e);

			std::vector<bool> filtered;
			/* if _Filter is true for each file in the torrent index we test if the file is accepted
			and we set the value to a bool vector used by the filter_files functions
			*/
  			if (_Filter)
			{
				std::set<std::string> torrentDownloading;
				std::set<std::string> torrentIgnoring;

				unsigned int index = 0;
				filtered.resize( unsigned int(ti.num_files()) );
				for (libtorrent::torrent_info::file_iterator i = ti.begin_files();
						i != ti.end_files(); ++i)
				{				
					// The file is not accepted (optional)
					if ( _AllowedFileSet.find( std::string(_Listener->getTorrentTmpDir()) + i->path.string()) == _AllowedFileSet.end())
					{
						torrentIgnoring.insert(std::string(_Listener->getTorrentTmpDir()) + i->path.string());
						std::string str = "Ignoring from torrent: ";					
						str += i->path.string();
						_Listener->progressInfo(str.c_str());
						filtered[index] = true;	

				
					}
					else
					{
						// The file is accepted (not optional)
						torrentDownloading.insert(std::string(_Listener->getTorrentTmpDir()) + i->path.string());
						filtered[index] = false;	

						std::string str = "Downloading from torrent: ";					
						str += i->path.string();
						_Listener->progressInfo(str.c_str());


					}					
					++index;
				}
				std::set<std::string>::iterator first(_AllowedFileSet.begin()), last(_AllowedFileSet.end());				
				for ( ; first != last; ++first)
				{
					if ( torrentDownloading.find(*first) == torrentDownloading.end()
						&&torrentIgnoring.find(*first) == torrentIgnoring.end())
					{
						std::string str = "File not present in torrent (but present in downlaod list): ";					
						str += *first;
 						_Listener->progressInfo(str.c_str());
					}
				}
			}

			
			_CurrentDownload = _Session->add_torrent(ti, boost::filesystem::path(std::string(_Listener->getTorrentTmpDir())), libtorrent::entry(), false);
			// to the filtering (must be called after add_torrent)
			if (_Filter) { _CurrentDownload.filter_files(filtered); }

		}
		catch (const std::exception& e)
		{			
			// display warning?
		}

	}

	/// \see ITorrentControl::getCurrentDownloadStateLabel
	virtual const char* getCurrentDownloadStateLabel() const
	{
		if (!_CurrentDownload.is_valid())
		{
			return "uiFinished";
		}

		libtorrent::torrent_status s = _CurrentDownload.status();
		static char const* state_str[] =
			{"uiQueued", "uiChecking", "uiConnecting", "uiDownloadingMetadata"
			, "uiDownload", "uiFinished", "uiSeeding", "uiAllocating"};

		static std::string str;
		str = std::string(state_str[s.state]);
		return str.c_str();
	}

	/// \see ITorrentControl::isCurrentDownloadValid
	virtual bool isCurrentDownloadValid() const
	{
		return _CurrentDownload.is_valid() && _CurrentDownload.has_metadata();
	}

	/// \see ITorrentControl::getCurrentDownloadName
	virtual const char* getCurrentDownloadName() const
	{
		return _CurrentDownload.get_torrent_info().name().c_str();
	}

	/// \see ITorrentControl::getCurrentDownloadProgress
	virtual unsigned long long getCurrentDownloadProgress() const
	{
		if (!isCurrentDownloadValid())
		{
			return 0;
		}
		// s.progress is beween 0 and 1
		libtorrent::torrent_status s = _CurrentDownload.status();
		return s.progress * 100;
	}

	/// \see ITorrentControl::getCurrentDownloadDownloadRate
	virtual float getCurrentDownloadDownloadRate() const
	{
		libtorrent::torrent_status s = _CurrentDownload.status();
		return s.download_rate;
	}

	/// \see ITorrentControl::getCurrentDownloadTotalDone
	virtual unsigned long long getCurrentDownloadTotalDone() const
	{
		libtorrent::torrent_status s = _CurrentDownload.status();
		return s.total_done;
	}

	
	/// \see ITorrentControl::getCurrentDownloadTotalWanted
	virtual unsigned long long getCurrentDownloadTotalWanted() const
	{
		libtorrent::torrent_status s = _CurrentDownload.status();
		return s.total_wanted;
	}

	/// \see ITorrentControl::update
	virtual void update()
	{
		if (!_Listener || !_Session)
		{
			return;
		}
		std::auto_ptr<libtorrent::alert> a;
		a = _Session->pop_alert();
		//As finished if an alert happends => alert the listener
		while (a.get())
		{
			if (libtorrent::torrent_finished_alert* p = dynamic_cast<libtorrent::torrent_finished_alert*>(a.get()))
			{
				if (_CurrentDownload.is_valid())
				{
					_CurrentDownload = libtorrent::torrent_handle();
					_Listener->torrentFinished();
				}
			}
			const char * severity[] = { "debug", "info", "warning", "critical", "fatal", "none" };
			std::string str;
			str += std::string( severity[a->severity()] );
			str += " ";
			str +=  a->msg();
			_Listener->progressInfo(str.c_str());
		
			a = _Session->pop_alert();
		}

		//Data is correct (download previously) => alert the listener
		if (_CurrentDownload.is_valid() && _CurrentDownload.status().state == libtorrent::torrent_status::finished )
		{
			_CurrentDownload = libtorrent::torrent_handle();
			_Listener->torrentFinished();
		}
		
	}
	//: \see ITorrentControl::getInstance
	static CTorrentControl* getInstance()
	{
		if (!_Instance)
		{
			_Instance = new CTorrentControl();
		}
		return _Instance;
	}
	
	//: \see ITorrentControl::setListener
	virtual void setListener(ITorrentListener* listener) 
	{
		_Listener = listener;
	}
private:
	libtorrent::session* _Session;	//! < a libtorrent session
	libtorrent::torrent_handle _CurrentDownload; //!< The current file downloaded
	std::set<std::string>	_AllowedFileSet; //!< The list of file that are allowed to download
	bool					_Filter;  //!< is a filter used
	ITorrentListener*		_Listener; //!< The listener that will received messages when download is finished
	static CTorrentControl* _Instance; //!< The instance of the concret class
};

ITorrentControl* ITorrentControl::getInstance()
{
	return CTorrentControl::getInstance();
}


CTorrentControl* CTorrentControl::_Instance;