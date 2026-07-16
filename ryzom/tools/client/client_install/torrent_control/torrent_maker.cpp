

#include "torrent_maker.h"
#include "ryzom_control/ryzom_control.h"

#include <string>
#include <vector>
#include <map>
#include <set>

#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include <exception>
#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/file.hpp"
#include "libtorrent/storage.hpp"
#include "libtorrent/hasher.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/format.hpp>

using namespace boost::filesystem;
using namespace libtorrent;



/** "private" implementation of IArchiveMakerListener
The aim of this class is to create a torrent (create an archive, add elements, save the archive)
*/
class CTorrentMakerListener : public IArchiveMakerListener
{
public:
	/** Constructor
	\param tracker A tracker url (eg http://oe28.nevrax.com:7969/announce)
	*/
	CTorrentMakerListener(const char* tracker): _Tracker(tracker)
	{

	}

	/** Begin a trorrent achive
	The function addArchiveElement must be called between beginArchive  and endArchive
	\param
	*/
	virtual void beginArchive(const char* outputfile) 
	{
		// destroy previous
		_TorrentInfo = libtorrent::torrent_info();
		_OutputName = outputfile;
	}

	/** Add an element to the archive Ryzom specific parametres are added in order to sort file in the hope that all
	chunck needed for a partial download are contigus.
	\param filename The name of a file to add
	\param version The version of the added file
	\param optional Must be true if the category of the file is optional
	\param timestamp The timestamp of the file
	
	The function addArchiveElement must be called between beginArchive  and endArchive
	Warnning: all params after version seems to no be used but are still present in order to be able to log infos.
	*/
	virtual void addArchiveElement(const char* filename, unsigned int version, bool optional, unsigned int timestamp) 
	{

		/*
		if lzma file( from patch repository) exist add to torrent archive instead display warning
		All file must be present. But if a file is not present simply display a warning: client standard 
		patch system must still be working.
		*/
		path path2 ((boost::format("patch/%05d/%s.lzma")% version % filename).str());

		try 
		{
			if (exists(path2))
			{
				_TorrentInfo.add_file( path2, file_size( path2));
			}
			else
			{				
				std::string tmp = (boost::format("\"%s\" is describe in the idx file but bnp do not exist ") %  path2.string().c_str()).str();					
				if (_Log) {	_Log(tmp.c_str()); }
			}
		}
		catch (const std::exception& err)
		{			
			std::string tmp = err.what();
			if (_Log) {	_Log(tmp.c_str()); }
		}
	}

	/*Create a bittorent file
	The function beginArchive and addArchiveElement must be called before.
	Compute all md5 of file to add. Save the torrent file.

	*/
	virtual void endArchive()
	{
		std::string::size_type sep = _OutputName.rfind("/") + 1;
		std::string tmpFilename = _OutputName.substr(sep);

		// create a tempory torrent at the root of the patch directory
		{
			//if a .torrent file already exist remove it
			path full_path = complete(path(tmpFilename));
			if (boost::filesystem::exists(full_path))
			{ 
				boost::filesystem::remove(full_path);
			}

			ofstream out(full_path, std::ios_base::binary);

			// specify the size of the chunck (smaller chunck need more md5)
			int piece_size = 256 * 1024;
			// specify the name of the creator (can be see in meta_file info)
			char const* creator_str = "libtorrent";

			
			_TorrentInfo.set_piece_size(piece_size);

			storage st(_TorrentInfo, full_path.branch_path());
			_TorrentInfo.add_tracker(_Tracker.c_str()); // specify the tracker

			// calculate the hash for all pieces
			int num =_TorrentInfo.num_pieces();
			std::vector<char> buf(piece_size);
			for (int i = 0; i < num; ++i)
			{
				try {
					// calckculate the hash of the piece
					st.read(&buf[0], i, 0, _TorrentInfo.piece_size(i));
					hasher h(&buf[0], _TorrentInfo.piece_size(i));
					_TorrentInfo.set_hash(i, h.final());
					std::cout << "Chunk " <<(i+1)<< "  " << ((i+1) / 4) << "M /" << (num/4) << "M\r"<<std::endl;
				}
				catch ( const std::exception& e)
				{
					std::string tmp = e.what();
					if (_Log) {	_Log(tmp.c_str()); }
				}
			}

			_TorrentInfo.set_creator(creator_str);

			// create the torrent and print it to out
			entry e = _TorrentInfo.create_torrent();
			libtorrent::bencode(std::ostream_iterator<char>(out), e);
		}
		// move the file to its output directory

		if (boost::filesystem::exists(_OutputName))
		{ 
			boost::filesystem::remove(_OutputName);
		}
		boost::filesystem::rename(tmpFilename, _OutputName);		
		
	}
	// set the log function
	virtual void setLog( void (*log)(const char*)) 
	{
		_Log = log;
	}

private:
	libtorrent::torrent_info _TorrentInfo; //libtorrent struct to create .torrent file
	std::string _OutputName;	 //name of the .torrent file
	std::string _Tracker;	// url of the tracker
	void (*_Log)(const char*); // function ptr to a log function
};

//---------------------------------------------------------------------------
IArchiveMakerListener* CTorrentMaker::newTorrentMaker(const char* tracker)
{
	return new CTorrentMakerListener( tracker);
}

void CTorrentMaker::deleteTorrentMaker(IArchiveMakerListener* toDelete)
{
	delete toDelete;
}
