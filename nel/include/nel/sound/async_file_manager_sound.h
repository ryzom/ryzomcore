// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "nel/misc/thread.h"

namespace NLSOUND
{

class IBuffer;


/**
 * CAsyncFileManagerSound is a class that manage sound file loading in a seperate thread
 * This class mostly depend on the CAsyncFileManager class for serializing async file
 * loading request.
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2002
 */
class CAsyncFileManagerSound
{
	NLMISC_SAFE_SINGLETON_DECL(CAsyncFileManagerSound);
public:
//	static	CAsyncFileManagerSound &getInstance();
	static  void	terminate();


	void	loadWavFile(IBuffer *pdestBuffer, const std::string &filename);
	void	cancelLoadWaveFile(const std::string &filename);

	// Do not use these methods with the bigfile manager
	void loadFile (const std::string &fileName, uint8 **pPtr);
	void loadFiles (const std::vector<std::string> &vFileNames, const std::vector<uint8**> &vPtrs);

	void signal (bool *pSgn); // Signal a end of loading for a group of "mesh or file" added
	void cancelSignal (bool *pSgn);

private:
	/// Constructor
	CAsyncFileManagerSound() {}

	/// Singleton instance.
//	static CAsyncFileManagerSound	*_Singleton;


	/// A non exported class for load canceling purpose.
	friend class CCancelLoadWavFile;

	// Load task.
	class CLoadWavFile : public NLMISC::IRunnable
	{
		IBuffer		*_pDestbuffer;

	public:
		std::string	_Filename;

		CLoadWavFile (IBuffer *pdestBuffer, const std::string &filename);
		void run (void);
	};

};

} // NLSOUND
