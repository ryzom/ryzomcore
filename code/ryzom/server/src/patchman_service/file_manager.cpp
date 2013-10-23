// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// nel
#include "nel/misc/variable.h"
#include "nel/misc/file.h"

// game share
#include "game_share/utils.h"

// local
#include "file_manager.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
//using namespace NLNET;
using namespace PATCHMAN;


//-----------------------------------------------------------------------------
// some NLMISC Variable
//-----------------------------------------------------------------------------

NLMISC::CVariable<uint32> FileCacheSize("patchman","FileCacheSize","Minimum size for re file cache",100*1024*1024,0,true);


namespace PATCHMAN
{		
	//-----------------------------------------------------------------------------
	// handy utils
	//-----------------------------------------------------------------------------

	static NLMISC::CSString getTempFileName(const NLMISC::CSString& fileName)
	{
		return fileName+".download.tmp";
	}

	static NLMISC::CSString getIndexFileName(const NLMISC::CSString& rootDirectory)
	{
		return rootDirectory+".patchman.file_index";
	}


	//-----------------------------------------------------------------------------
	// methods SFileInfo
	//-----------------------------------------------------------------------------

	bool SFileInfo::updateFileInfo(const NLMISC::CSString& fileName,const NLMISC::CSString& fullFileName, SFileInfo::TUpdateMethod updateMethod, IFileInfoUpdateListener* updateListener)
	{
		// if file doesn't exist then just drop out
		if (!NLMISC::CFile::fileExists(fullFileName))
		{
			clear();
			return false;
		}

		// give ourselves 5 attempts in case the file is being accessed...
		for(uint32 i=0;i<5;++i)
		{
			try
			{
				uint32 newFileSize= NLMISC::CFile::getFileSize(fullFileName);
				uint32 newFileTime= NLMISC::CFile::getFileModificationDate(fullFileName);

				// work out whether the record has changed (based on size and time stamp)
				bool changed= ( (FileTime!=newFileTime) || (FileSize!=newFileSize) );

				// note: it is possible to hit an exception in the checksum calculation if the file is being accessed
				// by someone else
				if (updateMethod==FORCE_RECALCULATE || (updateMethod==RECALCULATE_IF_CHANGED  && (newFileSize!=FileSize || newFileTime!=FileTime)))
				{
					// call the updateListener object's callback so that they can write a log, etc
					if (updateListener!=NULL)
					{
						updateListener->cbFileInfoRescanning(fileName,newFileSize);
					}

					// workout the new checksum and update the 'changed' flag accordingly
					CHashKeyMD5 newChecksum= NLMISC::getMD5(fullFileName);
					changed|= (Checksum!=newChecksum);
					Checksum= newChecksum;
				}

				// update the fields in our new record
				FileName= fileName;
				FileSize= newFileSize;
				FileTime= newFileTime;

				// if there's an update listener object, then let them know about the file update
				if (changed && updateListener!=NULL)
				{
					updateListener->cbFileInfoUpdate(*this);
				}

				// return true if we are unchanged, otherwise false
				return !changed;
			}
			catch(...)
			{
				nlwarning("Exception thrown in getMD5(\"%s\") ... will try again in a few seconds",fullFileName.c_str());
				nlSleep(5);
			}
		}

		nlwarning("Failed to get info on file: %s",fullFileName.c_str());
		clear();
		return false;
	}


	//-----------------------------------------------------------------------------
	// methods CFileSpec
	//-----------------------------------------------------------------------------

	CFileSpec::CFileSpec()
	{
	}

	CFileSpec::CFileSpec(const NLMISC::CSString& fileSpec)
	{
		_NameSpec= NLMISC::CFile::getFilename(fileSpec);
		_PathSpec= NLMISC::CFile::getPath(fileSpec);

		_NameIsWild= _NameSpec.contains('*') || _NameSpec.contains('?');
		_PathIsWild= _PathSpec.contains('*') || _PathSpec.contains('?');

		_AcceptAllNames= (_NameSpec=="*");
		_AcceptAllPaths= (_PathSpec=="*/");
	}

	bool CFileSpec::matches(const NLMISC::CSString& fullFileName) const
	{
		CSString name= NLMISC::CFile::getFilename(fullFileName);
		CSString path= NLMISC::CFile::getPath(fullFileName);

		return (_AcceptAllPaths || pathMatches(path))  && (_AcceptAllNames || nameMatches(name));
	}

	bool CFileSpec::nameMatches(const NLMISC::CSString& fileName) const
	{
		return _AcceptAllNames || ( _NameIsWild ? testWildCard(fileName,_NameSpec) : (fileName==_NameSpec) );
	}

	bool CFileSpec::pathMatches(const NLMISC::CSString& path) const
	{
		// treat the special case where the file that we're set to accept all paths
		if (_AcceptAllPaths)
			return true;

		// treat the special case where the file that we're testing has no path and we
		// are looking for files in the root directory only
		if (path.empty() && (_PathSpec=="./") )
			return true;

		return _PathIsWild ? testWildCard(path,_PathSpec) : (path==_PathSpec);
	}

	NLMISC::CSString CFileSpec::toString() const
	{
		return _PathSpec+_NameSpec;
	}

	const CSString& CFileSpec::nameSpec() const
	{
		return _NameSpec;
	}

	const CSString& CFileSpec::pathSpec() const
	{
		return _PathSpec;
	}

	bool CFileSpec::isWild() const
	{
		return _NameIsWild || _PathIsWild;
	}

	bool CFileSpec::nameIsWild() const
	{
		return _NameIsWild;
	}

	bool CFileSpec::pathIsWild() const
	{
		return _PathIsWild;
	}



	//-----------------------------------------------------------------------------
	// methods CRepositoryDirectory
	//-----------------------------------------------------------------------------

	CRepositoryDirectory::CRepositoryDirectory(const NLMISC::CSString& path)
	{
		_Root= NLMISC::CPath::getFullPath(path);
		_IndexFileIsUpToDate= false;
	}

	void CRepositoryDirectory::clear()
	{
		// clear out our directories map
		_DirectoryTree.clear();
	}

	void CRepositoryDirectory::rescanFull(IFileInfoUpdateListener* updateListener)
	{
		// rescan our directories recursively, starting at the root
		_rescanDirectory("",true,updateListener);

		// update the index file as required
		if (!_IndexFileIsUpToDate)
			writeIndex();
	}

	void CRepositoryDirectory::rescanPartial(IFileInfoUpdateListener* updateListener)
	{
		// if the directory tree is empty then start with the rooot directory...
		if (_DirectoryTree.empty())
		{
			_rescanDirectory("",false,updateListener);
			return;
		}

		// try to get hold of a ref to the last directory that we rescanned
		TDirectoryTree::iterator it= _DirectoryTree.find(_LastRescan);
		if (it==_DirectoryTree.end())
		{
			// we failed to get a ref to the last directory so wrap back round to the start
			it= _DirectoryTree.begin();
		}
		else
		{
			// increement our iterator to get hold of a ref to the next directory in the map
			++it;
			// if we reach end of map then wrap back to the start
			if (it==_DirectoryTree.end())
			{
				it= _DirectoryTree.begin();
			}
		}

		// scan the next directory in our map (this is not recursive
		_rescanDirectory(it->first,false,updateListener);
		_LastRescan= it->first;

		// update the index file as required
		if (!_IndexFileIsUpToDate)
			writeIndex();
	}

	void CRepositoryDirectory::updateFile(const NLMISC::CSString& fileName,SFileInfo::TUpdateMethod updateMethod, IFileInfoUpdateListener* updateListener)
	{
		// if the file doesn't exist then give up
		if (!NLMISC::CFile::fileExists(_Root+fileName))
			return;

		// split the file name into path and fileName
		const NLMISC::CSString path= NLMISC::CFile::getPath(fileName);

		// get hold of the directory object for this file (or create a new one if need be)
		TFileInfoMap& directory= _DirectoryTree[path];

		// get hold of the file info object for this (or create a new one if need be)
		SFileInfo& fileInfo= directory[fileName];

		// update the file info for the given file
		_IndexFileIsUpToDate&= fileInfo.updateFileInfo(fileName,_Root+fileName,updateMethod,updateListener);
	}

	// query methods
	void CRepositoryDirectory::getFileInfo(const NLMISC::CSString& fileSpec,TFileInfoVector& result,IFileRequestValidator* validator,const NLNET::IModuleProxy *sender) const
	{
		// split into directory and file name
		const CFileSpec spec(fileSpec);

		// setup a set of paths to scan
		std::vector<TDirectoryTree::const_iterator> paths;

		// if the path has wirlcards in it then do a search for wildcard matches otherwise just do a lookup
		if (spec.pathIsWild())
		{
			for (TDirectoryTree::const_iterator it=_DirectoryTree.begin();  it!=_DirectoryTree.end(); ++it)
			{
				if (spec.pathMatches(it->first))
					paths.push_back(it);
			}
		}
		else
		{
			TDirectoryTree::const_iterator it= _DirectoryTree.find(spec.pathSpec());
			if (it != _DirectoryTree.end())
				paths.push_back(it);
		}

		// run through the selected paths looking for files
		for (uint32 i=0;i<paths.size();++i)
		{
			// compose the file name that we're supposed to match
			NLMISC::CSString fullFilePattern= paths[i]->first+spec.nameSpec();

			// if the filename doesn't have wildcards then just do a straight lookup...
			if (!spec.nameIsWild())
			{
				// see whether we have a match...
				TFileInfoMap::const_iterator fit= paths[i]->second.find(fullFilePattern);
				if (fit!=paths[i]->second.end())
				{
					// call the overloadable validation callback before adding the file info record to the result container
					if (validator==NULL || validator->cbValidateFileInfoRequest(sender,fit->second.FileName))
					{
						result.push_back(fit->second);
					}
				}
				continue;
			}

			// run through the files in the given path
			for (TFileInfoMap::const_iterator fit= paths[i]->second.begin(); fit!= paths[i]->second.end(); ++fit)
			{
				// get hold of the file name for this map entry
				const NLMISC::CSString& name=fit->second.FileName;

				// do either a wildcard compare or a quick and dirty string compare
				if (testWildCard(name,fullFilePattern))
				{
					// call the overloadable validation callback before adding the file info record to the result container
					if (validator==NULL || validator->cbValidateFileInfoRequest(sender,name))
					{
						result.push_back(fit->second);
					}
				}
			}
		}
	}

	void CRepositoryDirectory::getFile(const NLMISC::CSString& fileName,NLMISC::CSString& resultData,IFileRequestValidator* validator,const NLNET::IModuleProxy *sender) const
	{
		// start by clearing out the result container...
		resultData.clear();

		// allow the overloadable validation callback a chance to prohibit read
		if (validator!=NULL && !validator->cbValidateDownloadRequest(sender,fileName))
			return;

		// if the file exists then go ahead and read it
		NLMISC::CSString fullFileName= _Root+fileName;
		if (NLMISC::CFile::fileExists(fullFileName))
		{
			resultData.readFromFile(fullFileName);
		}
	}

	const NLMISC::CSString &CRepositoryDirectory::getRootDirectory() const
	{
		return _Root;
	}

	bool CRepositoryDirectory::readIndex()
	{
		// start by clearing out our containers
		clear();

		// make sure the file exists (return false if not found)
		NLMISC::CSString indexFileName= getIndexFileName(_Root);
		if (!NLMISC::CFile::fileExists(indexFileName))
			return false;

		// read the file contents
		NLMISC::CSString index;
		index.readFromFile(indexFileName);
		DROP_IF(index.empty(),"Failed to read data from index file: "+indexFileName,return false);

		nlinfo("CRepositoryDirectory_Reading index file: %s",indexFileName.c_str());
		NLMISC::CVectorSString lines;
		index.splitLines(lines);
		for (uint32 i=0;i<lines.size();++i)
		{
			// get hold of the line and strip off comments and spurious blanks
			NLMISC::CSString line= lines[i].splitTo("//").strip();
			if (line.empty()) continue;

			// break the line down into constituent parts
			SFileInfo fileInfo;
			fileInfo.FileSize= line.strtok(",").strip().atoi();
			fileInfo.FileTime= line.strtok(",").strip().atoi();
			fileInfo.Checksum.fromString(line.strtok(",").strip());
			fileInfo.FileName= line.strip();

			// make sure that the text in the line was valid and that the file on disk looks like the one in our record
			DROP_IF(fileInfo.FileName.empty(),"Skipping line due to parse error: "+lines[i],continue);

			// add the result to one of our maps of files
			NLMISC::CSString path= NLMISC::CFile::getPath(fileInfo.FileName);
			_DirectoryTree[path][fileInfo.FileName]=fileInfo;
		}
		nlinfo("%d entries read from index file",lines.size());
		_IndexFileIsUpToDate= true;
		return true;
	}

	bool CRepositoryDirectory::writeIndex() const
	{
		// get hold of the file name for the index file
		NLMISC::CSString indexFileName= getIndexFileName(_Root);
//		nldebug("Flushing changes to index file: %s",indexFileName.c_str());

		// setup a text buffer to build our output file in
		NLMISC::CSString outputText= "// Index file for patchman service\n// *FORMAT*: size, time, checksum, filename\n\n";

		// iterate over our directories and their files, building entries
		for (TDirectoryTree::const_iterator dit= _DirectoryTree.begin(); dit!= _DirectoryTree.end(); ++dit)
		{
			const TFileInfoMap& theDirectory= dit->second;
			for (TFileInfoMap::const_iterator fit= theDirectory.begin(); fit!=theDirectory.end(); ++fit)
			{
				const SFileInfo& theInfo= fit->second;
				outputText+= NLMISC::toString("%10u,%12u, %s, %s\n",theInfo.FileSize,theInfo.FileTime,theInfo.Checksum.toString().c_str(),theInfo.FileName.c_str());
			}

		}

		// write the resulting buffer to disk
		_IndexFileIsUpToDate= outputText.writeToFile(indexFileName);
		return _IndexFileIsUpToDate;
	}

	void CRepositoryDirectory::_rescanDirectory(const NLMISC::CSString& directoryName, bool recurse, IFileInfoUpdateListener* updateListener)
	{
//		nldebug("VERBOSE_Scanning directory: root=%s directory=%s",_Root.c_str(),directoryName.c_str());

		// make sure we exist in the '_DirectoryTree' map (and get a handle to it)
		TFileInfoMap& theDirectory= _DirectoryTree[directoryName];

		// first scan for directories
		std::vector<std::string> pathContents;
		NLMISC::CPath::getPathContent(_Root+directoryName,false,true,false,pathContents);

		// run through the directories we found...
		for (uint32 i=(uint32)pathContents.size();i--;)
		{
			NLMISC::CSString childDirectoryName= NLMISC::CSString(pathContents[i]).leftCrop((uint32)_Root.size());

			// make sure they exist in the '_DirectoryTree' map
			_DirectoryTree[childDirectoryName];

			// if we're recursing then go for it
			if (recurse)
			{
				_rescanDirectory(childDirectoryName,recurse,updateListener);
			}
		}

		// run through all of the files in our map flagging them as 'not updated'
		for (TFileInfoMap::iterator fit= theDirectory.begin(); fit!= theDirectory.end(); ++fit)
		{
			fit->second.FileName.clear();
		}
		
		// now scan for files
		pathContents.clear();
		NLMISC::CPath::getPathContent(_Root+directoryName,false,false,true,pathContents);

		// run through the files adding them to ourself
		for (uint32 i=(uint32)pathContents.size();i--;)
		{
			// if the file is system file then skip it
			if (pathContents[i].find("/.")!=std::string::npos)
				continue;

			// construct the file name
			NLMISC::CSString fileName= NLMISC::CSString(pathContents[i]).leftCrop((uint32)_Root.size()); 
			// get hold of the directory entry for this file (or create a new one if not exist) and update it
			_IndexFileIsUpToDate&= _DirectoryTree[directoryName][fileName].updateFileInfo(fileName,pathContents[i],SFileInfo::RECALCULATE_IF_CHANGED,updateListener);
		}

		// run through all of the files in our map looking for files that are not updated and that need erasing
		TFileInfoMap::iterator fit= theDirectory.begin();
		while (fit!=theDirectory.end())
		{
			TFileInfoMap::iterator thisIt= fit;
			++fit;
			if (thisIt->second.FileName.empty())
			{
				// if there's an update listener object, then let them know about the file update
				if (updateListener!=NULL)
				{
					updateListener->cbFileInfoErased(thisIt->first);
				}

				// erase the entry in our files map and flag the index file as out of date
				theDirectory.erase(thisIt);
				_IndexFileIsUpToDate= false;
			}
		}
	}


	//-----------------------------------------------------------------------------
	// methods CFileManager
	//-----------------------------------------------------------------------------

	bool CFileManager::load(const NLMISC::CSString& fileName, uint32 startOffset, uint32 numBytes, NLMISC::CSString& result)
	{
		// clear out the return value before we begin
		result.clear();

		// make sure the file exists
		if (!NLMISC::CFile::fileExists(fileName))
			return false;

//		nldebug("Loading file data for: %s",fileName.c_str());

		// get the file's vital statistics from disk
		uint32 fileSize= NLMISC::CFile::getFileSize(fileName);
		uint32 fileTime= NLMISC::CFile::getFileModificationDate(fileName);

		// run through the files to see if the one we're after is here
		TCacheFiles::iterator it= _CacheFiles.begin();
		for (; it!=_CacheFiles.end();++it)
		{
			// if we've found the file we're after then break out here
			if (it->FileName==fileName && fileSize==it->FileSize && fileTime==it->FileTime)
			{
//				nldebug("- Found data in Ram @ offset: %d",it->StartOffset);
				break;
			}
		}

		// if we didn't find the file then we have to load it
		if (it==_CacheFiles.end())
		{
//			nldebug("- Found data NOT already in Ram");

			// setup a data block for this file
			SCacheFileEntry newFileEntry;
			newFileEntry.FileName= fileName;
			newFileEntry.FileSize= fileSize;
			newFileEntry.FileTime= fileTime;
			newFileEntry.StartOffset= ~0u;

			// if the buffer is too small to load this file then reallocate it and clear out the _CacheFiles vector
			if (_CacheBuffer.size()<fileSize || _CacheFiles.empty())
			{
				_CacheFiles.clear();
				_CacheBuffer.clear();
				_CacheBuffer.resize(max(uint32(fileSize*2),uint32(FileCacheSize)));
				newFileEntry.StartOffset= 0;
//				nldebug("- Grew buffer to %d bytes",_CacheBuffer.size());
			}
			else
			{
				newFileEntry.StartOffset= _CacheFiles.back().StartOffset+_CacheFiles.back().FileSize;
				uint32 requiredEndOffset= newFileEntry.StartOffset+ fileSize;
				// see whether we'll fit in between the 'back file and end of buffer
				if (requiredEndOffset>_CacheBuffer.size())
				{
					// clear out all remaining files between us and the end of buffer
					while (!_CacheFiles.empty() && _CacheFiles.front().StartOffset>=newFileEntry.StartOffset)
					{
//						nldebug("- Ditching cache entry: %s",_CacheFiles.front().FileName.c_str());
						_CacheFiles.pop_front();
						nlassert(!_CacheFiles.empty());
					}
					// not enough space at end of file so we'll need to spin round to the start
					newFileEntry.StartOffset= 0;
					requiredEndOffset= fileSize;
				}
				// our start offset is now OK, so make a bit of room for our data as required
				while (!_CacheFiles.empty() && _CacheFiles.front().StartOffset>=newFileEntry.StartOffset && _CacheFiles.front().StartOffset<requiredEndOffset)
				{
//					nldebug("- Ditching cache entry: %s",_CacheFiles.front().FileName.c_str());
					_CacheFiles.pop_front();
				}
			}

//			nldebug("- Reading file data @offset: %d (%d bytes)",newFileEntry.StartOffset,fileSize);

			// read in the file
			FILE* inf= fopen(fileName.c_str(),"rb");
			BOMB_IF(inf==NULL,"Failed to open input file for reading: "+fileName,return false);
			uint32 bytesRead=(uint32)fread(&_CacheBuffer[newFileEntry.StartOffset],1,fileSize,inf);
			fclose(inf);
			BOMB_IF(bytesRead!=fileSize,"Failed to read data from input file: "+fileName,return false);

			// add our new file descriptioon block to the _CacheFiles container
			_CacheFiles.push_back(newFileEntry);
			it=_CacheFiles.end();
			it--;
		}

		// make sure the requested file segment is valid
		uint32 endOffset= startOffset+numBytes;
		DROP_IF(endOffset>fileSize,"Ignoring request for data where end offset > file size: "+fileName,return false);

//		nldebug("- Retrieving data from buffer @offset: %d (%d bytes)",it->StartOffset+startOffset,numBytes);

		// copy out the data chunk that we're after
		result.resize(numBytes);
		memcpy(&result[0],&_CacheBuffer[it->StartOffset+startOffset],numBytes);

		// we succeeded so return true
		return true;
	}

	TRepositoryDirectoryPtr CFileManager::getRepositoryDirectory(const NLMISC::CSString& path)
	{
		// get hold of a ref to the map entry pointing at the directory we want (create a new map entry if need be)
		TRepositoryDirectoryRefPtr& thePtr=_RepositoryDirectories[path];

		// if the map entry is null then create a new one
		if (thePtr==NULL)
		{
			thePtr= new CRepositoryDirectory(path);
			thePtr->readIndex();
		}

		return &*thePtr;
	}

	bool CFileManager::save(const NLMISC::CSString& fileName, const NLMISC::CMemStream& data)
	{
		NLMISC::CSString tmpFileName= fileName+"__patchman__.sav";

		// make sure the destination file is deleted before we begin
		if (NLMISC::CFile::fileExists(fileName))
		{
			NLMISC::CFile::deleteFile(fileName);
		}

		// try to write the tmp file
		try
		{
			// make sure that the directory structure exists foe the file
			NLMISC::CSString path= NLMISC::CFile::getPath(fileName);
			if (!path.empty())
			{
				NLMISC::CFile::createDirectoryTree(path);
			}

			// go ahead and write the data to disk...
			COFile outputFile(tmpFileName);
			outputFile.serialBuffer(const_cast<uint8*>(data.buffer()),data.size());
		}
		catch(...)
		{
		}

		// make sure that the file write succeeded
		if (NLMISC::CFile::getFileSize(tmpFileName)!=data.size())
		{
			nlwarning("Failed to save file '%s' because failed to save tmp file: %s",fileName.c_str(),tmpFileName.c_str());
			NLMISC::CFile::deleteFile(tmpFileName);
			return false;
		}

		// write succeeded so rename the tmp file to the correct file name
		bool ok= NLMISC::CFile::moveFile(fileName.c_str(),tmpFileName.c_str());
		DROP_IF(!ok,"Failed to save file '"+fileName+"' because failed to rename tmp file: '"+tmpFileName+"'",return false);

		return true;
	}

	uint32 CFileManager::getFileSize(const NLMISC::CSString& fileName)
	{
		try
		{
			return NLMISC::CFile::getFileSize(fileName);
		}
		catch(...)
		{
			return 0;
		}
	}

} // end of namespace


//-----------------------------------------------------------------------------
// NLMISC_COMMANDS for testing the singleton interface
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(patchman,fileManagerLoad,"Load a file segment via the file manager","<file name> <start offset> <num bytes>")
{
	if (args.size()!=3)
		return false;

	CSString fileName= args[0];
	CSString startOffset=args[1];
	CSString numBytes=args[2];

	CSString data;
	bool ok= CFileManager::getInstance().load(fileName,startOffset.atoui(),numBytes.atoui(),data);
	if (ok)
	{
		log.displayNL("Loaded %d bytes from file: %s[%d] (requested %d) starting: %s",data.size(),fileName.c_str(),startOffset.atoui(),numBytes.atoui(),data.left(20).quote().c_str());
	}
	else
	{
		log.displayNL("Load failed for file: %s (from offset %d to %d)",fileName.c_str(),startOffset.atoui(),startOffset.atoui()+numBytes.atoui()-1);
	}

	return true;
}							

NLMISC_CATEGORISED_COMMAND(patchman,fileManagerSave,"Save a file via the file manager","<file name> <text to save>")
{
	if (args.size()!=2)
		return false;

	CSString fileName= args[0];
	CMemStream data;
	data.serialBuffer((uint8*)(&args[1][0]),(uint32)args[1].size());
	CFileManager::getInstance().save(fileName,data);

	return true;
}

NLMISC_CATEGORISED_COMMAND(patchman,testCFileSpec,"test the CFileSpec class","")
{
	if (args.size()!=0)
		return false;

	{
		nlinfo("test a");
		CFileSpec fsa("foo/bar");
		nlassert(fsa.matches("foo/bar"));
		nlassert(!fsa.matches("foo/bard"));
		nlassert(!fsa.matches("food/bar"));
		nlassert(!fsa.matches("foo/d/bar"));
		nlassert(!fsa.nameIsWild());
		nlassert(!fsa.pathIsWild());
		nlassert(fsa.nameMatches("bar"));
		nlassert(fsa.pathMatches("foo/"));
		nlassert(!fsa.nameMatches("bard"));
		nlassert(!fsa.pathMatches("food/"));
		nlassert(!fsa.pathMatches("foo/d/"));
		nlassert(fsa.nameSpec()=="bar");
		nlassert(fsa.pathSpec()=="foo/");
		nlassert(fsa.toString()=="foo/bar");
	}

	{
		nlinfo("test b");
		CFileSpec fsb("foo/bar*");
		nlassert(fsb.matches("foo/bar"));
		nlassert(fsb.matches("foo/bard"));
		nlassert(!fsb.matches("food/bar"));
		nlassert(!fsb.matches("foo/d/bar"));
		nlassert(fsb.nameIsWild());
		nlassert(!fsb.pathIsWild());
		nlassert(fsb.nameMatches("bard"));
		nlassert(fsb.pathMatches("foo/"));
		nlassert(fsb.nameMatches("bard"));
		nlassert(!fsb.pathMatches("food/"));
		nlassert(!fsb.pathMatches("foo/d/"));
		nlassert(fsb.nameSpec()=="bar*");
		nlassert(fsb.pathSpec()=="foo/");
		nlassert(fsb.toString()=="foo/bar*");
	}

	{
		nlinfo("test c");
		CFileSpec fsc("foo*/bar");
		nlassert(fsc.matches("foo/bar"));
		nlassert(!fsc.matches("foo/bard"));
		nlassert(fsc.matches("food/bar"));
		nlassert(fsc.matches("foo/d/bar"));
		nlassert(!fsc.nameIsWild());
		nlassert(fsc.pathIsWild());
		nlassert(fsc.nameMatches("bar"));
		nlassert(fsc.pathMatches("foo/"));
		nlassert(!fsc.nameMatches("bard"));
		nlassert(fsc.pathMatches("food/"));
		nlassert(fsc.pathMatches("foo/d/"));
		nlassert(fsc.nameSpec()=="bar");
		nlassert(fsc.pathSpec()=="foo*/");
		nlassert(fsc.toString()=="foo*/bar");
	}

	{
		nlinfo("test d");
		CFileSpec fsd("foo*/bar*");
		nlassert(fsd.matches("foo/bar"));
		nlassert(fsd.matches("foo/bard"));
		nlassert(fsd.matches("food/bar"));
		nlassert(fsd.matches("foo/d/bar"));
		nlassert(fsd.nameIsWild());
		nlassert(fsd.pathIsWild());
		nlassert(fsd.nameMatches("bar"));
		nlassert(fsd.pathMatches("foo/"));
		nlassert(fsd.nameMatches("bard"));
		nlassert(fsd.pathMatches("food/"));
		nlassert(fsd.pathMatches("foo/d/"));
		nlassert(fsd.nameSpec()=="bar*");
		nlassert(fsd.pathSpec()=="foo*/");
		nlassert(fsd.toString()=="foo*/bar*");
	}

	return true;
}

	class CFileSpec
	{
	public:
		// ctors
		CFileSpec();
		CFileSpec(const NLMISC::CSString& fileSpec);

		// test a complete match (filename and path)
		bool matches(const NLMISC::CSString& fullFileName) const;

		// test whether the given file name matches the filename part of the filespec
		// note - the supplied filename should already have been stripped of its path
		bool nameMatches(const NLMISC::CSString& fileName) const;

		// test whether the given path matches the path part of the filespec
		// note - the supplied path should not have an attached file name
		bool pathMatches(const NLMISC::CSString& path) const;

		// retrieve the filespec as a single string (for serialising, etc)
		NLMISC::CSString toString() const;

		// accessors
		const NLMISC::CSString& nameSpec() const;
		const NLMISC::CSString& pathSpec() const;
		bool nameIsWild() const;
		bool pathIsWild() const;

	private:
		NLMISC::CSString _NameSpec;
		NLMISC::CSString _PathSpec;
		bool _NameIsWild;			// true if _NameSpec contains wildcards	('*' or '?')
		bool _PathIsWild;			// true if _PathSpec contains wildcards ('*' or '?')
	};
