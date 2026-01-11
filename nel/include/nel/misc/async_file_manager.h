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


#ifndef NL_ASYNC_FILE_MANAGER_H
#define NL_ASYNC_FILE_MANAGER_H

#include "types_nl.h"
#include "task_manager.h"


namespace NLMISC
{

/**
 * CAsyncFileManager is a class that manage file loading in a seperate thread
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2002
 */
class CAsyncFileManager : public CTaskManager
{
	NLMISC_SAFE_SINGLETON_DECL(CAsyncFileManager);
	CAsyncFileManager() {}
public:

	// Must be called instead of constructing the object
//	static CAsyncFileManager &getInstance ();
	// NB: release the singleton, but assert if there is any pending loading tasks.
	// Each system that use the async file manager should ensure it has no more pending task in it
	static void terminate ();

	// Do not use these methods with the bigfile manager
	void loadFile (const std::string &fileName, uint8 **pPtr);
	void loadFiles (const std::vector<std::string> &vFileNames, const std::vector<uint8**> &vPtrs);


	void signal (bool *pSgn); // Signal a end of loading for a group of "mesh or file" added
	void cancelSignal (bool *pSgn);

	/**
	 *	CCancelCallback is an interface that is used in call to CAsyncFileManager::cancelLoad.
	 *	This class contains one method that is called for each task in the async file loader.
	 *	If the method returns true, then the task is removed and cancelLoad return.
	 *	\author Boris Boucher
	 *	\author Nevrax France
	 *	\date 10/2002
	 */
	class ICancelCallback
	{
	public:
		virtual ~ICancelCallback() {}
		virtual bool callback(const IRunnable *prunnable) const =0;
	};

	/// Add a load task in the CAsyncFileManager task list.
	void addLoadTask(IRunnable *ploadTask);
	/** Call this method to cancel a programmed load.
	 *	\return False if the task either already ended or running.
	 */
	bool cancelLoadTask(const ICancelCallback &cancelCallBack);

private:

//	CAsyncFileManager (); // Singleton mode -> access it with the getInstance function

//	static CAsyncFileManager *_Singleton;

	// All the tasks
	// -------------

	// Load a file
	class CFileLoad : public IRunnable
	{
		std::string _FileName;
		uint8 **_ppFile;
	public:
		CFileLoad (const std::string& sFileName, uint8 **ppFile);
		void run (void);
		void getName (std::string &result) const;
	};

	// Load multiple files
	class CMultipleFileLoad : public IRunnable
	{
		std::vector<std::string> _FileNames;
		std::vector<uint8**> _Ptrs;
	public:
		CMultipleFileLoad (const std::vector<std::string> &vFileNames, const std::vector<uint8**> &vPtrs);
		void run (void);
		void getName (std::string &result) const;
	};

	// Signal
	class CSignal  : public IRunnable
	{
	public:
		bool *Sgn;
	public:
		CSignal (bool *pSgn);
		void run (void);
		void getName (std::string &result) const;
	};

};


} // NLMISC


#endif // NL_ASYNC_FILE_MANAGER_H

/* End of async_file_manager.h */
