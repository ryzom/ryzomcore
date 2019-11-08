/*
Create a .torrent from the repository
example of use map O: on \\dailyclient\nevrax\output launch the application from the O:/ directory
with as parametre the number of version you wish 
and the url of the tracker
*/
#include "ryzom_control/ryzom_control.h"
#include "torrent_control/torrent_maker.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

std::vector<std::string> _Warning;
// add the warning to a String container that is displayed at the end of the process
void logError(const char* msg)
{
	_Warning.push_back(std::string(msg));
}

int main(int argc, char* argv[])
{	//execute from 0: (map on  \\dailyclient\nevrax\output)
	if (argc < 3) 
	{
		std::cerr<<"Need two paramerer first the version path 47 \n"
			"second the tracker http://oe28.nevrax.com:7969/announce"<<std::endl;
		return 1;
	}
	IRyzomVersionMaker* ryzom = IRyzomVersionMaker::getInstance();
	// Set the log Error for ryzom
	ryzom->setLog(logError);
	char buffer[1024];
	int version = atoi(argv[1]);
	sprintf(buffer, "patch/%05d/ryzom_%05d.idx", version, version);
	std::string idx = buffer;
	sprintf(buffer, "patch/%05d/ryzom_%05d.torrent", version, version);
	std::string output = buffer;	

	IArchiveMakerListener* listener = CTorrentMaker::newTorrentMaker(argv[2]);

	// Set the log Error for torrent
	listener->setLog(logError);

	// create a new torrent with annoucer as argv2
	ryzom->create(idx.c_str(), output.c_str(), listener);
	
	// delete stuf
	CTorrentMaker::deleteTorrentMaker(listener);
	IRyzomVersionMaker::releaseInstance();

	//display warinng if exist
	if (!_Warning.empty())
	{		
		std::string::size_type path = output.rfind("/");
		std::ofstream out( (output + ".txt").c_str());
		size_t first=0, last = _Warning.size();
		for (; first != last; ++first)
		{
			out <<_Warning[first] << std::endl;

		}
	}
	return 0;
}

