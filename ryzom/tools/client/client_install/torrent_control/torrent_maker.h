#ifndef __TORRENT_MAKER_H__
#define __TORRENT_MAKER_H__

#include "libtorrent/config.hpp"
#include "ryzom_control/ryzom_control.h"

class IArchiveMakerListener;

/*
The aim of this module is to create new torrent files from data.
Data are sorted by type and version number. 
The aim is that all chunck of data needed by a partial download to be contigous.
*/
class   TORRENT_EXPORT CTorrentMaker
{
public:
	/*create a new IArchiveMakerListener (see make_torrent.cpp to example)
	\param annouce The url of the tracker eg "http://oe28.nevrax.com:7969/announce"
	\see IArchiveMakerListener
	*/
	
	static IArchiveMakerListener* newTorrentMaker( const char* annouce );
	/*
	Delete a IArchiveMakerListener created by newTorrentMakder()
	The delete operator is not used because an object from a dll (torroent_dll) must not be deleted by an other dll (exe)
	*/
	static void deleteTorrentMaker(IArchiveMakerListener* toDelete);
};


#endif

