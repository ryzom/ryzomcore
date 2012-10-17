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

// game share
#include "game_share/utils.h"

// local
#include "gus_utils.h"
#include "saves_unit.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace GUS;


//-----------------------------------------------------------------------------
// SAVES namespace
//-----------------------------------------------------------------------------

namespace SAVES
{
	//-----------------------------------------------------------------------------
	// class CSavesUnitFileList
	//-----------------------------------------------------------------------------

	class CSavesUnitFileList: public ISavesUnitElement
	{
	public:
		CSavesUnitFileList(const CSString& parentPath,const CSString& directoryName,const CSString& fileSpec);
		bool update(ISavesCallbackHandler* parent);
		void appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const;

	protected:
		CSString _Path;
		CSString _Name;
		CSString _FileSpec;
		typedef map<CSString,CFileDescription> TFileDescriptions;
		TFileDescriptions _FileDescriptions;
	};


	//-----------------------------------------------------------------------------
	// class CShardRootDirectory
	//-----------------------------------------------------------------------------

	class CShardRootDirectory: public ISavesUnitElement
	{
	public:
		CShardRootDirectory(const CSString& path);
		bool update(ISavesCallbackHandler* parent);
		void appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const;

	private:
		CSString _Path;

		typedef vector<CSString> TRequiredFiles;
		TRequiredFiles _RequiredFiles;

		typedef map<CSString,TSavesUnitElementPtr> TRequiredDirectories;
		TRequiredDirectories _RequiredDirectories;

		typedef map<CSString,CFileDescription> TFiles;
		TFiles _Files;
	};


	//-----------------------------------------------------------------------------
	// class CShardCharacterDirectory
	//-----------------------------------------------------------------------------

	class CShardCharacterDirectory: public CSavesUnitFileList
	{
	public:
		CShardCharacterDirectory(const CSString& parentPath,const CSString& directoryName);
	};


	//-----------------------------------------------------------------------------
	// class CShardOfflineCommandsDirectory
	//-----------------------------------------------------------------------------

	class CShardOfflineCommandsDirectory: public CSavesUnitFileList
	{
	public:
		CShardOfflineCommandsDirectory(const CSString& parentPath,const CSString& directoryName);
	};


	//-----------------------------------------------------------------------------
	// class CShardGuildDirectory
	//-----------------------------------------------------------------------------

	class CShardGuildDirectory: public CSavesUnitFileList, public ISavesCallbackHandler
	{
	public:
		CShardGuildDirectory(const CSString& parentPath,const CSString& directoryName);
		bool update(ISavesCallbackHandler* parent);
		void appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const;

		void addNew(const NLMISC::CSString& fileName,uint32 timeStamp,uint32 size);
		void addChange(const NLMISC::CSString& fileName,uint32 timeStamp,uint32 size);
		void addDeleted(const NLMISC::CSString& fileName);
		void addElement(ISavesUnitElement* newChild);

	private:
		ISavesCallbackHandler* _Parent;
	};


	//-----------------------------------------------------------------------------
	// class CShardGuildFile
	//-----------------------------------------------------------------------------

	class CShardGuildFile: public ISavesUnitElement
	{
	public:
		CShardGuildFile(const CSString& path);
		bool update(ISavesCallbackHandler* parent);
		void appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const;

	private:
		CSString _Path;
		uint64	 _Checksum;
		uint32	 _FileSize;
		uint32	 _TimeStamp;
	};


	//-----------------------------------------------------------------------------
	// class CBakRootDirectory
	//-----------------------------------------------------------------------------

	class CBakRootDirectory: public ISavesUnitElement
	{
	public:
		CBakRootDirectory(const CSString& path);
		bool update(ISavesCallbackHandler* parent);
		void appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const;

	private:
		CSString _Path;
		typedef set<CSString> TChildren;
		TChildren _Children;
	};


	//-----------------------------------------------------------------------------
	// class CWwwRootDirectory
	//-----------------------------------------------------------------------------

	class CWwwRootDirectory: public ISavesUnitElement
	{
	public:
		CWwwRootDirectory(const CSString& path);
		bool update(ISavesCallbackHandler* parent);
		void appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const;

	private:
		CSString _Path;
		typedef set<CSString> TChildren;
		TChildren _Children;
	};


	//-----------------------------------------------------------------------------
	// class CWwwGroupDirectory
	//-----------------------------------------------------------------------------

	class CWwwGroupDirectory: public ISavesUnitElement
	{
	public:
		CWwwGroupDirectory(const CSString& parentPath,const CSString& directoryName);
		bool update(ISavesCallbackHandler* parent);
		void appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const;

	private:
		CSString _Path;
		CSString _Name;
		typedef set<CSString> TChildren;
		TChildren _Children;
	};


	//-----------------------------------------------------------------------------
	// class CWwwChildDirectory
	//-----------------------------------------------------------------------------

	class CWwwChildDirectory: public CSavesUnitFileList
	{
	public:
		CWwwChildDirectory(const CSString& parentPath,const CSString& directoryName);
	};


	//-----------------------------------------------------------------------------
	// methods CSavesUnitFileList
	//-----------------------------------------------------------------------------

	CSavesUnitFileList::CSavesUnitFileList(const CSString& parentPath,const CSString& directoryName,const CSString& fileSpec)
	{
		_Path= parentPath+directoryName+"/";
		_Name= directoryName;
		_FileSpec= fileSpec;
	}

	void CSavesUnitFileList::appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const
	{
		// iterate over our file index, appending file descriptions to the fdc
		for (TFileDescriptions::const_iterator it=_FileDescriptions.begin();it!=_FileDescriptions.end();++it)
		{
			BOMB_IF(path!=it->second.FileName.left(path.size()),"Skipping file because path doesn't match ("+path+"): "+it->second.FileName,continue);
			fdc.addFile(it->second.FileName.leftCrop(path.size()),it->second.FileTimeStamp,it->second.FileSize);
		}
	}

	bool CSavesUnitFileList::update(ISavesCallbackHandler* parent)
	{
		// if the directory no longer exists then signal all of its children as deleted
		if (!NLMISC::CFile::isDirectory(_Path))
		{
			for (TFileDescriptions::iterator it=_FileDescriptions.begin();it!=_FileDescriptions.end();++it)
			{
				parent->addDeleted(_Path+it->first);
			}
			return false;
		}

		// scan the directory for files
		CFileDescriptionContainer fdc;
		fdc.addFileSpec(_Path+_FileSpec);

		// a little set that we'll use to check for deleted files at the end...
		set<CSString> recentFiles;

		// run through the files looking for entries that don't match the previous scan
		for (uint32 i=0;i<fdc.size();++i)
		{
			// get hold of the file name
			const CFileDescription& fd= fdc[i];
			const CSString& fileName= fd.FileName;

			// mark this file as existing in the most recent scan
			recentFiles.insert(fileName);

			// lookup the file name in our file index...
			TFileDescriptions::iterator it= _FileDescriptions.find(fileName);

			// do we have a new file?
			if (it==_FileDescriptions.end())
			{
				parent->addNew(fileName,fd.FileTimeStamp,fd.FileSize);
				_FileDescriptions[fileName]=fdc[i];
				continue;
			}

			// has the file changed?
			if (it->second.FileTimeStamp!=fd.FileTimeStamp || it->second.FileSize!=fd.FileSize)
			{
				parent->addChange(fileName,fd.FileTimeStamp,fd.FileSize);
				it->second=fdc[i];
				continue;
			}
		}

		// look for deleted files
		vector<CSString> deadFiles;
		for (TFileDescriptions::iterator it=_FileDescriptions.begin();it!=_FileDescriptions.end();++it)
		{
			if (recentFiles.find(it->first)==recentFiles.end())
			{
				deadFiles.push_back(it->first);
			}
		}

		// deal with the dead files that we found
		for (uint32 i=deadFiles.size();i--;)
		{
			parent->addDeleted(deadFiles[i]);
			_FileDescriptions.erase(deadFiles[i]);
		}

		return true;
	}


	//-----------------------------------------------------------------------------
	// methods CShardRootDirectory
	//-----------------------------------------------------------------------------

	CShardRootDirectory::CShardRootDirectory(const CSString& path)
	{
		_Path= cleanPath(path,true);

		_RequiredFiles.push_back("account_names.txt");
		_RequiredFiles.push_back("character_names.txt");

		_RequiredDirectories["characters"]=						new CShardCharacterDirectory(_Path,"characters");
		_RequiredDirectories["characters_offline_commands"]=	new CShardOfflineCommandsDirectory(_Path,"characters_offline_commands");
		_RequiredDirectories["guilds"]=							new CShardGuildDirectory(_Path,"guilds");
	}

	void CShardRootDirectory::appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const
	{
		for (TFiles::const_iterator it=_Files.begin();it!=_Files.end();++it)
		{
			fdc.addFile(CFile::getFilename(it->second.FileName),it->second.FileTimeStamp,it->second.FileSize);
		}
	}

	bool CShardRootDirectory::update(ISavesCallbackHandler* parent)
	{
		// make sure that our own directory exists
		if (!CFile::isDirectory(_Path))
			return false;

		// scan for the required files...
		CFileDescriptionContainer fdc;
		for (uint32 i=0;i<_RequiredFiles.size();++i)
		{
			// is the file missing ?
			if (!CFile::fileExists(_Path+_RequiredFiles[i]))
			{
				// lookup the file name in our '_Files' map
				TFiles::iterator it= _Files.find(fdc[i].FileName);

				// check whether the file existed previously
				if (it!=_Files.end())
				{
					parent->addDeleted(it->first);
					_Files.erase(it);
				}
				continue;
			}

			// reestablish the file time and size info
			fdc.addFile(_Path+_RequiredFiles[i]);
		}

		// run through the found files checking whether they're new or have changed
		for (uint32 i=0;i<fdc.size();++i)
		{
			// lookup the file name in our '_Files' map
			TFiles::iterator it= _Files.find(fdc[i].FileName);

			// is the file new?
			if (it==_Files.end())
			{
				parent->addNew(fdc[i].FileName,fdc[i].FileTimeStamp,fdc[i].FileSize);
				_Files[fdc[i].FileName]= fdc[i];
				continue;
			}

			// has the file changed ?
			if (fdc[i].FileSize!=it->second.FileSize || fdc[i].FileTimeStamp!=it->second.FileTimeStamp)
			{
				parent->addChange(fdc[i].FileName,fdc[i].FileTimeStamp,fdc[i].FileSize);
				it->second= fdc[i];
			}
		}

		// run through the required directories to check that they're all active and OK
		for (TRequiredDirectories::iterator it= _RequiredDirectories.begin(); it!=_RequiredDirectories.end();++it)
		{
			// if the required directory unit isn't active and the directory physically exists
			// then activate it and add it to the parent object
			if (!it->second->isActive() && CFile::isDirectory(_Path+it->first))
			{
				parent->addElement(it->second);
			}
		}

		return true;
	}


	//-----------------------------------------------------------------------------
	// methods CShardCharacterDirectory
	//-----------------------------------------------------------------------------

	CShardCharacterDirectory::CShardCharacterDirectory(const CSString& parentPath,const CSString& directoryName):
		CSavesUnitFileList(parentPath,directoryName,"*_pdr.bin")
	{
	}


	//-----------------------------------------------------------------------------
	// methods CShardOfflineCommandsDirectory
	//-----------------------------------------------------------------------------

	CShardOfflineCommandsDirectory::CShardOfflineCommandsDirectory(const CSString& parentPath,const CSString& directoryName):
		CSavesUnitFileList(parentPath,directoryName,"*.offline_commands")
	{
	}


	//-----------------------------------------------------------------------------
	// methods CShardGuildDirectory
	//-----------------------------------------------------------------------------

	CShardGuildDirectory::CShardGuildDirectory(const CSString& parentPath,const CSString& directoryName):
		CSavesUnitFileList(parentPath,directoryName,"guild_*.bin")
	{
		_Parent=NULL;
	}

	void CShardGuildDirectory::appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const
	{
		// nothing to do... this element only contains sub dirctories and not files
	}

	bool CShardGuildDirectory::update(ISavesCallbackHandler* parent)
	{
		// take a copy of the _Parent and assign it a new value (basically performs a stacking operation)
		ISavesCallbackHandler* hold=_Parent;
		_Parent= parent;

		// transfer to inheritted update() method
		bool result=CSavesUnitFileList::update(this);

		// return parent to previous value and return the result
		_Parent=hold;
		return result;
	}

	void CShardGuildDirectory::addNew(const NLMISC::CSString& fileName,uint32 timeStamp,uint32 size)
	{
		nlassert(_Parent!=NULL);

		// a new file added ... need to add a new GuildFile element to the parent to represent it
		_Parent->addElement(new CShardGuildFile(fileName));

		// nothing to do - we let the guild file manage it's own existance
	}

	void CShardGuildDirectory::addChange(const NLMISC::CSString& fileName,uint32 timeStamp,uint32 size)
	{
		nlassert(_Parent!=NULL);

		// nothing to do... guild files change all the time for no good reason
	}

	void CShardGuildDirectory::addDeleted(const NLMISC::CSString& fileName)
	{
		nlassert(_Parent!=NULL);

		// nothing to do - we let the guild file manage it's own existance
	}

	void CShardGuildDirectory::addElement(ISavesUnitElement* newChild)
	{
		// pass the 'new element' up to the parent
		_Parent->addElement(newChild);
	}


	//-----------------------------------------------------------------------------
	// methods CShardGuildFile
	//-----------------------------------------------------------------------------

	CShardGuildFile::CShardGuildFile(const CSString& path)
	{
		_Path= path;
		_Checksum= 0;
		_FileSize= 0;
		_TimeStamp= 0;
	}

	bool CShardGuildFile::update(ISavesCallbackHandler* parent)
	{
		// check whether the file still exists
		if (!CFile::fileExists(_Path))
		{
			parent->addDeleted(_Path);
			_Checksum= 0;
			_FileSize= 0;
			_TimeStamp= 0;
			return false;
		}

		// get the up to date file size
		uint32 fileSize= CFile::getFileSize(_Path);

		// calculate the current checksum...

		// setup a buffer and read the file data into it
		CSString fileBody;
		fileBody.readFromFile(_Path);

		// pad the buffer to a multiple of 8 characters
		fileBody+=CSString("01234567").left(8-(fileBody.size()&7));
		nlassert( (fileBody.size()&7)==0 && fileBody.size()>7 );

		// run through the buffer performing a very simple shift and xor checksum (good enough for our purposes)
		// note that we could have used an MD5 but his is much much much faster (less strain on the CPU)
		uint64 checksum;
		for (uint32 i=fileBody.size()/8;i--;)
		{
			checksum= ((checksum<<1)|(checksum>>63))^((uint64*)&fileBody[0])[i];
		}

		// see whether we have a new file
		if (_Checksum==0 && _FileSize==0)
		{
			_TimeStamp= CFile::getFileModificationDate(_Path);
			parent->addNew(_Path,_TimeStamp,fileSize);
		}
		else
		{
			// see if we have a change to our file
			if (_Checksum!=checksum || _FileSize!=fileSize)
			{
				_TimeStamp= CFile::getFileModificationDate(_Path);
				parent->addChange(_Path,_TimeStamp,fileSize);
			}
		}

		// record our new values for next time round
		_Checksum= checksum;
		_FileSize= fileSize;

		return true;
	}

	void CShardGuildFile::appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const
	{
		if (_Checksum!=0 || _FileSize!=0)
		{
			BOMB_IF(path!=_Path.left(path.size()),"Skipping file because path doesn't match ("+path+"): "+_Path,return);
			fdc.addFile(_Path.leftCrop(path.size()),_TimeStamp,_FileSize);
		}
	}


	//-----------------------------------------------------------------------------
	// methods CBakRootDirectory
	//-----------------------------------------------------------------------------

	CBakRootDirectory::CBakRootDirectory(const CSString& path)
	{
		_Path= cleanPath(path,true);
	}

	void CBakRootDirectory::appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const
	{
		// nothing to do... this element only contains sub dirctories and not files
	}

	bool CBakRootDirectory::update(ISavesCallbackHandler* parent)
	{
		// scan the root directory for -inc, -day and refference sub directories
		std::vector<std::string> subDirectories;
		NLMISC::CPath::getPathContent(_Path,false,true,false,subDirectories);
		for (uint32 i=0;i<subDirectories.size();++i)
		{
			CSString name= subDirectories[i];

			// if directory name isn't one of the one's we're after then skip it
			if (name.right(5)!="-inc/" && name.right(5)!="-day/" && name!="refference/")
				continue;

			// if the sub directory didn't previously exist then create it
			if (_Children.find(name)==_Children.end())
			{
				_Children.insert(name);
				parent->addElement(new CShardRootDirectory(name));
			}
		}

		return true;
	}


	//-----------------------------------------------------------------------------
	// methods CWwwRootDirectory
	//-----------------------------------------------------------------------------

	CWwwRootDirectory::CWwwRootDirectory(const CSString& path)
	{
		_Path= cleanPath(path,true);
	}

	void CWwwRootDirectory::appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const
	{
		// nothing to do... this element only contains sub dirctories and not files
	}

	bool CWwwRootDirectory::update(ISavesCallbackHandler* parent)
	{
		// scan the root directory for 2 letter sub directories
		std::vector<std::string> subDirectories;
		NLMISC::CPath::getPathContent(_Path,false,true,false,subDirectories);
		for (uint32 i=0;i<subDirectories.size();++i)
		{
			// if directory name is not 2 letters then skip it
			if (subDirectories[i].size()!=2)
				continue;

			// if the sub directory didn't previously exist then create it
			if (_Children.find(subDirectories[i])==_Children.end())
			{
				_Children.insert(subDirectories[i]);
				parent->addElement(new CWwwGroupDirectory(_Path,subDirectories[i]));
			}
		}

		return true;
	}


	//-----------------------------------------------------------------------------
	// methods CWwwGroupDirectory
	//-----------------------------------------------------------------------------

	CWwwGroupDirectory::CWwwGroupDirectory(const CSString& parentPath,const CSString& directoryName)
	{
		_Path= parentPath+directoryName+"/";
		_Name= directoryName;
	}

	void CWwwGroupDirectory::appendFileListToFdc(const NLMISC::CSString& path,CFileDescriptionContainer &fdc) const
	{
		// nothing to do... this element only contains sub dirctories and not files
	}

	bool CWwwGroupDirectory::update(ISavesCallbackHandler* parent)
	{
		// make sure the directory hasn't been deleted
		if (!NLMISC::CFile::isDirectory(_Path))
			return false;

		// scan the directory for sub directories starting with same first 2 letters
		std::vector<std::string> subDirectories;
		NLMISC::CPath::getPathContent(_Path,false,true,false,subDirectories);
		for (uint32 i=0;i<subDirectories.size();++i)
		{
			// if directory name is not 2 letters then skip it
			if (CSString(subDirectories[i]).left(2)!=_Name)
				continue;

			// if the sub directory didn't previously exist then create it
			if (_Children.find(subDirectories[i])==_Children.end())
			{
				_Children.insert(subDirectories[i]);
				parent->addElement(new CWwwChildDirectory(_Path,subDirectories[i]));
			}
		}

		return true;
	}


	//-----------------------------------------------------------------------------
	// methods CWwwChildDirectory
	//-----------------------------------------------------------------------------

	CWwwChildDirectory::CWwwChildDirectory(const CSString& parentPath,const CSString& directoryName):
		CSavesUnitFileList(parentPath,directoryName,"*")
	{
	}


	//-----------------------------------------------------------------------------
	// methods CSavesUnit
	//-----------------------------------------------------------------------------

	CSavesUnit::CSavesUnit()
	{
		_IsInitialised= false;
		_IsFirstScan= true;
		_ChangeMsg= new CMsgRSUpdate;
	}

	void CSavesUnit::init(const CSString& directoryName,TType type)
	{
		// setup our properties
		_IsFirstScan= true;
		_IsInitialised= true;
		_Children.clear();
		_Path= cleanPath(directoryName,true);

		// add the root elements to the children container
		switch(type)
		{
		case SHARD:
			_Children.push_back(new CShardRootDirectory(_Path));
			_Children.back()->setActive(true);
			break;

		case BAK:
			_Children.push_back(new CBakRootDirectory(_Path));
			_Children.back()->setActive(true);
			break;

		case WWW:
			_Children.push_back(new CWwwRootDirectory(_Path));
			_Children.back()->setActive(true);
			break;

		default:
			nlerror("Invalid saves module type");
		}

		// setup the children iterator to start at the first element
		_IterationIndex= 0;
	}

	void CSavesUnit::update()
	{
		nlassert(_IsInitialised);
		if (_IterationIndex>=_Children.size())
		{
			// reset the update iterator
			_IterationIndex= 0;

			// clear the 'first scan' flag meaning that we've now scanned the entire directory tree at least once
			_IsFirstScan= false;
		}
		else
		{
			// update the next directory
			if (_Children[_IterationIndex]->update(this)==false)
			{
				// the update returned false meaning the directory doesn't exist any more...

				// mark the child object as inactive
				_Children[_IterationIndex]->setActive(false);

				// drop the object from our chilren container
				_Children[_IterationIndex]= _Children.back();
				_Children.pop_back();
			}
			else
			{
				// move the iterator on for the next update
				++_IterationIndex;
			}
		}
	}

	bool CSavesUnit::ready() const
	{
		nlassert(_IsInitialised);
		return !_IsFirstScan;
	}

	void CSavesUnit::getFileList(CFileDescriptionContainer &result) const
	{
		// clear out the result container before we begin
		result.clear();

		// iterate over children getting each off them to add their files to the result...
		for (uint32 i=0;i<_Children.size();++i)
		{
			_Children[i]->appendFileListToFdc(_Path,result);
		}
	}

	void CSavesUnit::addElement(ISavesUnitElement* newChild)
	{
		// add the element to out children container
		_Children.push_back(newChild);

		// mark the element as 'active'
		newChild->setActive(true);
	}

	TMsgRSUpdatePtr CSavesUnit::popNextChangeSet()
	{
		// setup a smart pointer to avoid premature destruction of our return object
		TMsgRSUpdatePtr result= _ChangeMsg;

		// create a new message to hold future changes
		_ChangeMsg= new CMsgRSUpdate;

		// return the current change set
		return result;
	}

	void CSavesUnit::addNew(const NLMISC::CSString& fileName,uint32 timeStamp,uint32 size)
	{
		// if there's an active '_ChangeMsg' message object then add our new entry to it
		if (_ChangeMsg!=NULL)
		{
			BOMB_IF(_Path!=fileName.left(_Path.size()),"addNew() FAILED: Skipping file because path doesn't match ("+_Path+"): "+fileName,return);
			_ChangeMsg->addNew(fileName.leftCrop(_Path.size()),timeStamp,size);
		}
	}

	void CSavesUnit::addChange(const NLMISC::CSString& fileName,uint32 timeStamp,uint32 size)
	{
		// if there's an active '_ChangeMsg' message object then add our new entry to it
		if (_ChangeMsg!=NULL)
		{
			BOMB_IF(_Path!=fileName.left(_Path.size()),"addChange() FAILED: Skipping file because path doesn't match ("+_Path+"): "+fileName,return);
			_ChangeMsg->addChange(fileName.leftCrop(_Path.size()),timeStamp,size);
		}
	}

	void CSavesUnit::addDeleted(const NLMISC::CSString& fileName)
	{
		// if there's an active '_ChangeMsg' message object then add our new entry to it
		if (_ChangeMsg!=NULL)
		{
			BOMB_IF(_Path!=fileName.left(_Path.size()),"addDeleted() FAILED: Skipping file because path doesn't match ("+_Path+"): "+fileName,return);
			_ChangeMsg->addDeleted(fileName.leftCrop(_Path.size()));
		}
	}


	//-----------------------------------------------------------------------------
	// methods ISavesUnitElement
	//-----------------------------------------------------------------------------

	ISavesUnitElement::ISavesUnitElement()
	{
		_ActivationCounter=0;
	}

	bool ISavesUnitElement::isActive() const
	{
		return (_ActivationCounter!=0);
	}

	void ISavesUnitElement::setActive(bool value)
	{
		if (value)
		{
			++_ActivationCounter;
		}
		else
		{
			nlassert(_ActivationCounter>0);
			--_ActivationCounter;
		}
	}
}
