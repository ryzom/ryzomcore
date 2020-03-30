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


#include "stdmisc.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/async_file_manager.h"


using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

//CAsyncFileManager *CAsyncFileManager::_Singleton = NULL;
NLMISC_SAFE_SINGLETON_IMPL(CAsyncFileManager);


// ***************************************************************************

/*CAsyncFileManager::CAsyncFileManager()
{
}
*/
// ***************************************************************************

/*CAsyncFileManager &CAsyncFileManager::getInstance()
{
	if (_Singleton == NULL)
	{
		_Singleton = new CAsyncFileManager();
	}
	return *_Singleton;
}
*/
// ***************************************************************************

void CAsyncFileManager::terminate ()
{
	if (_Instance != NULL)
	{
		INelContext::getInstance().releaseSingletonPointer("CAsyncFileManager", _Instance);
		delete _Instance;
		_Instance = NULL;
	}
}


void CAsyncFileManager::addLoadTask(IRunnable *ploadTask)
{
	addTask(ploadTask);
}

bool CAsyncFileManager::cancelLoadTask(const CAsyncFileManager::ICancelCallback &callback)
{
	CSynchronized<list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
	list<CWaitingTask> &rTaskQueue = acces.value ();
	list<CWaitingTask>::iterator it = rTaskQueue.begin();

	while (it != rTaskQueue.end())
	{
		IRunnable *pR = it->Task;

		// check the task with the cancel callback.
		if (callback.callback(pR))
		{
			// Delete the load task
			delete pR;
			rTaskQueue.erase (it);
			return true;
		}
		++it;
	}

	// If not found, the current running task may be the one we want to cancel. Must wait it.
	// Beware that this code works because of the CSynchronized access we made above (ensure that the
	// taskmanager will end just the current task async (if any) and won't start an other one.
	waitCurrentTaskToComplete ();

	return false;
}

// ***************************************************************************
/*
void CAsyncFileManager::loadMesh(const std::string& meshName, IShape **ppShp, IDriver *pDriver)
{
	addTask (new CMeshLoad(meshName, ppShp, pDriver));
}
*/
// ***************************************************************************
/*
bool CAsyncFileManager::cancelLoadMesh(const std::string& sMeshName)
{
	CSynchronized<list<IRunnable *> >::CAccessor acces(&_TaskQueue);
	list<IRunnable*> &rTaskQueue = acces.value ();
	list<IRunnable*>::iterator it = rTaskQueue.begin();

	while (it != rTaskQueue.end())
	{
		IRunnable *pR = *it;
		CMeshLoad *pML = dynamic_cast<CMeshLoad*>(pR);
		if (pML != NULL)
		{
			if (pML->MeshName == sMeshName)
			{
				// Delete mesh load task
				delete pML;
				rTaskQueue.erase (it);
				return true;
			}
		}
		++it;
	}
	return false;
}
*/
// ***************************************************************************
/*
void CAsyncFileManager::loadIG (const std::string& IGName, CInstanceGroup **ppIG)
{
	addTask (new CIGLoad(IGName, ppIG));
}

// ***************************************************************************

void CAsyncFileManager::loadIGUser (const std::string& IGName, UInstanceGroup **ppIG)
{
	addTask (new CIGLoadUser(IGName, ppIG));
}
*/
// ***************************************************************************

void CAsyncFileManager::loadFile (const std::string& sFileName, uint8 **ppFile)
{
	addTask (new CFileLoad (sFileName, ppFile));
}

// ***************************************************************************

void CAsyncFileManager::loadFiles (const std::vector<std::string> &vFileNames, const std::vector<uint8**> &vPtrs)
{
	addTask (new CMultipleFileLoad (vFileNames, vPtrs));
}

// ***************************************************************************

void CAsyncFileManager::signal (bool *pSgn)
{
	addTask (new CSignal (pSgn));
}

// ***************************************************************************

void CAsyncFileManager::cancelSignal (bool *pSgn)
{
	CSynchronized<list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
	list<CWaitingTask> &rTaskQueue = acces.value ();
	list<CWaitingTask>::iterator it = rTaskQueue.begin();

	while (it != rTaskQueue.end())
	{
		IRunnable *pR = it->Task;
		CSignal *pS = dynamic_cast<CSignal*>(pR);
		if (pS != NULL)
		{
			if (pS->Sgn == pSgn)
			{
				// Delete signal task
				delete pS;
				rTaskQueue.erase (it);
				return;
			}
		}
		++it;
	}
}

// ***************************************************************************
// FileLoad
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager::CFileLoad::CFileLoad (const std::string& sFileName, uint8 **ppFile)
{
	_FileName = sFileName;
	_ppFile = ppFile;
}

// ***************************************************************************
void CAsyncFileManager::CFileLoad::run (void)
{
	FILE *f = nlfopen (_FileName, "rb");
	if (f != NULL)
	{
		uint32 filesize=CFile::getFileSize (f);
		uint8 *ptr = new uint8[filesize];
		if (fread (ptr, filesize, 1, f) != 1)
			nlwarning("AFM: Couldn't read '%s'", _FileName.c_str());
		fclose (f);

		*_ppFile = ptr;
	}
	else
	{
		nlwarning ("AFM: Couldn't load '%s'", _FileName.c_str());
		*_ppFile = (uint8*)-1;
	}
}

// ***************************************************************************
void CAsyncFileManager::CFileLoad::getName (std::string &result) const
{
	result = "FileLoad (" + _FileName + ")";
}

// ***************************************************************************
// MultipleFileLoad
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager::CMultipleFileLoad::CMultipleFileLoad (const std::vector<std::string> &vFileNames,
														 const std::vector<uint8**> &vPtrs)
{
	_FileNames = vFileNames;
	_Ptrs = vPtrs;
}

// ***************************************************************************
void CAsyncFileManager::CMultipleFileLoad::run (void)
{
	for (uint32 i = 0; i < _FileNames.size(); ++i)
	{
		FILE *f = nlfopen (_FileNames[i], "rb");
		if (f != NULL)
		{
			uint32 filesize=CFile::getFileSize (f);
			uint8 *ptr = new uint8[filesize];
			if (fread (ptr, filesize, 1, f) != 1)
				nlwarning("AFM: Couldn't read '%s'", _FileNames[i].c_str());
			fclose (f);

			*_Ptrs[i] = ptr;
		}
		else
		{
			nlwarning ("AFM: Couldn't load '%s'", _FileNames[i].c_str());
			*_Ptrs[i] = (uint8*)-1;
		}
	}

}

// ***************************************************************************
void CAsyncFileManager::CMultipleFileLoad::getName (std::string &result) const
{
	result = "MultipleFileLoad (";
	uint i;
	for (i=0; i<_FileNames.size (); i++)
	{
		if (i)
			result += ", ";
		result += _FileNames[i];
	}
	result += ")";
}
// ***************************************************************************
// Signal
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager::CSignal::CSignal (bool *pSgn)
{
	Sgn = pSgn;
	*Sgn = false;
}

// ***************************************************************************
void CAsyncFileManager::CSignal::run (void)
{
	*Sgn = true;
}

// ***************************************************************************
void CAsyncFileManager::CSignal::getName (std::string &result) const
{
	result = "Signal";
}

} // NLMISC

