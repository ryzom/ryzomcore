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
// include
//-----------------------------------------------------------------------------

// pre compiled headers
#include "stdpch.h"

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/singleton.h"
#include "nel/misc/command.h"
#include "nel/misc/file.h"

// game share
#include "utils.h"
#include "deployment_configuration.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-----------------------------------------------------------------------------
// namespace DEPCFG
//-----------------------------------------------------------------------------

namespace DEPCFG
{
	//-----------------------------------------------------------------------------
	// forward class declarations
	//-----------------------------------------------------------------------------

	struct SExeRecord;
	class CInfoBlock;
	class CInfoContainer;
	class CDeploymentConfigurationImplementation;


	//-----------------------------------------------------------------------------
	// struct SExeRecord
	//-----------------------------------------------------------------------------

	struct SExeRecord
	{
		typedef std::set<NLMISC::CSString>		TDataEntries;
		typedef std::vector<NLMISC::CSString>	TCfgEntries;

		NLMISC::CSString	FullName;

		NLMISC::CSString	DomainName;
		NLMISC::CSString	ShardName;
		NLMISC::CSString	UniqueName;
		NLMISC::CSString	CmdLine;
		NLMISC::CSString	Host;
		NLMISC::CSString	StartOrder;
		TDataEntries		DataEntries;
		TCfgEntries			CfgEntries;
		TCfgEntries			CfgEntriesPost;

		// serial method
		void serial(NLMISC::IStream& stream)
		{
			stream.serial(FullName);
			stream.serial(DomainName);
			stream.serial(ShardName);
			stream.serial(UniqueName);
			stream.serial(CmdLine);
			stream.serial(Host);
			stream.serialCont(DataEntries);
			stream.serialCont(CfgEntries);
			stream.serialCont(CfgEntriesPost);
		}
	};


	//-----------------------------------------------------------------------------
	// class CInfoBlock
	//-----------------------------------------------------------------------------

	class CInfoBlock: public NLMISC::CRefCount
	{
	public:
		// ctors
		CInfoBlock(const NLMISC::CSString& name="");

		// write accessors
		void setDomainName(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors);
		void setShardName(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors);
		void setUniqueName(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors);
		void setCmdLine(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors);
		void setHost(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors);
		void setStartOrder(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors);

		void addUseEntry(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors);
		void addDataEntry(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors);

		void addCfgEntry(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors);
		void addCfgEntryPost(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors);

		void addCfgFile(const NLMISC::CSString& fileName,const NLMISC::CSString& context,uint32& errors);
		void addCfgFilePost(const NLMISC::CSString& fileName,const NLMISC::CSString& context,uint32& errors);

		// direct read accessors
		uint32 getNumParents() const;
		const NLMISC::CSString& getName() const;
		const NLMISC::CSString& getDomainName() const;

		// indirect read accessors
		bool isDomain() const;
		bool isShard() const;

		// setup the _Children vector from the _UseEntries vector
		void setupChildren(CInfoContainer* container,uint32& errors);

		// traverse the tree of children, cumulating data on exes, and instantiating the exe records in 'container'
		void buildExeSet(CInfoContainer* container,uint32& errors,const SExeRecord& parentExeRecord=SExeRecord());

		// serial
		void serial(NLMISC::IStream& stream);

		// display the contents of the container
		void dump(NLMISC::CLog& log) const;

	private:
		// data types
		typedef std::set<NLMISC::CSString>				TUseEntries;
		typedef std::set<NLMISC::CSString>				TDataEntries;
		typedef std::vector<NLMISC::CSString>			TCfgEntries;

		typedef std::vector<CInfoBlock*>				TChildren;

		// private methods
		bool _haveCircularRef(CInfoBlock* other) const;

		// data (basics)
		NLMISC::CSString	_Name;
		uint32				_NumParents;
		TChildren			_Children;

		// data extracted from input file
		NLMISC::CSString	_DomainName;
		NLMISC::CSString	_ShardName;
		NLMISC::CSString	_UniqueName;
		NLMISC::CSString	_CmdLine;
		NLMISC::CSString	_Host;
		NLMISC::CSString	_StartOrder;
		TUseEntries			_UseEntries;
		TDataEntries		_DataEntries;
		TCfgEntries			_CfgEntries;
		TCfgEntries			_CfgEntriesPost;
	};


	//-----------------------------------------------------------------------------
	// class CInfoContainer
	//-----------------------------------------------------------------------------

	class CInfoContainer
	{
	public:
		// public interface
		void clear();
		bool empty() const;

		bool read(const NLMISC::CSString& fileName);
		void serial(NLMISC::IStream& stream);

		void getHostNames(THostNames& result) const;
		void getDomainNames(TDomainNames& result) const;
		void getShardNames(const TDomainName& domainName,TShardNames& result) const;
		void getAppNames(const THostName& hostName,const TDomainName& domainName,TAppNames& result) const;
		void getAppNames(const THostName& hostName,const TDomainName& domainName,const TShardName& shardName,TAppNames& result) const;

		void getHost(const THostName& hostName,SHostDescription& result) const;
		void getDomain(const TDomainName& domainName, SDomainDescription& result) const;
		void getShard(const TDomainName& domainName,const TShardName& shardName,SShardDescription& result) const;
		void getApp(const TDomainName& domainName,const TAppName& appName,SAppDescription& result) const;

		void dumpInfoBlocks(NLMISC::CLog& log) const;
		void dumpDomains(NLMISC::CLog& log) const;

		// interface used by CInfoBlock methods
		CInfoBlock* getInfoBlock(const NLMISC::CSString& name);
		void addExe(const SExeRecord& exeRecord,uint32& errors);

	private:
		// data types
		typedef NLMISC::CSmartPtr<CInfoBlock>				TInfoBlockPtr;
		typedef std::map< NLMISC::CSString,TInfoBlockPtr >	TInfoBlocks;
		typedef std::set< NLMISC::CSString >				TFileNameSet;
		typedef std::vector< SExeRecord >					TExeRecords;
		typedef std::vector<uint32>							TExeIdx;
		typedef std::map<NLMISC::CSString,TExeIdx>			TShardExes;
		typedef std::map<NLMISC::CSString,TShardExes>		TDomainExes;

		// private methods
		void _readFile(const NLMISC::CSString& fileName,uint32& errors,TFileNameSet& fileNameStack);
		void _buildDomainTree(uint32& errors);
		void _buildExeSet(uint32& errors);

		// data
		TInfoBlockPtr	_CurrentInfoBlock;
		TInfoBlocks		_InfoBlocks;
		TExeRecords		_ExeRecords;
		TDomainExes		_DomainExes;
	};


	//-----------------------------------------------------------------------------
	// class CDeploymentConfigurationImplementation
	//-----------------------------------------------------------------------------

	class CDeploymentConfigurationImplementation: public CSingleton<CDeploymentConfigurationImplementation>, public CDeploymentConfiguration
	{
	public:
		bool read(const NLMISC::CSString& fileName);
		void write(const NLMISC::CSString& fileName);
		void serial(NLMISC::IStream& stream);

		void getHostNames(THostNames& result) const;
		void getDomainNames(TDomainNames& result) const;
		void getShardNames(const TDomainName& domainName,TShardNames& result) const;
		void getAppNames(const THostName& hostName,const TDomainName& domainName,TAppNames& result) const;
		void getAppNames(const THostName& hostName,const TDomainName& domainName,const TShardName& shardName,TAppNames& result) const;

		void getHost(const THostName& hostName,SHostDescription& result) const;
		void getDomain(const TDomainName& domainName, SDomainDescription& result) const;
		void getShard(const TDomainName& domainName,const TShardName& shardName,SShardDescription& result) const;
		void getApp(const TDomainName& domainName,const TAppName& appName,SAppDescription& result) const;

		void dumpInfoBlocks(NLMISC::CLog& log) const;
		void dumpDomains(NLMISC::CLog& log) const;

	private:
		CInfoContainer _InfoContainer;
	};


	//-----------------------------------------------------------------------------
	// methods CInfoBlock
	//-----------------------------------------------------------------------------

	CInfoBlock::CInfoBlock(const NLMISC::CSString& name)
	{
		_Name= name;
		_NumParents= 0;
	}

	void CInfoBlock::setDomainName(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors)
	{
		DROP_IF(!_DomainName.empty(),context+"Attempting to set a domain name more than once for the same info block",++errors; return);
		DROP_IF(entry.empty(),context+"Attempting to set an empty domain name",++errors; return);
		_DomainName=entry;
	}

	void CInfoBlock::setShardName(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors)
	{
		DROP_IF(!_ShardName.empty(),context+"Attempting to set a shard name more than once for the same info block",++errors; return);
		DROP_IF(entry.empty(),context+"Attempting to set an empty shard name",++errors; return);
		_ShardName=entry;
	}

	void CInfoBlock::setUniqueName(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors)
	{
		DROP_IF(!_UniqueName.empty(),context+"Attempting to set a name more than once for the same info block",++errors; return);
		DROP_IF(entry.empty(),context+"Attempting to set an empty name",++errors; return);
		_UniqueName=entry;
	}

	void CInfoBlock::setCmdLine(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors)
	{
		DROP_IF(!_CmdLine.empty(),context+"Attempting to set a cmdLine more than once for the same info block",++errors; return);
		DROP_IF(entry.empty(),context+"Attempting to set an empty cmdLine",++errors; return);
		_CmdLine=entry;
	}

	void CInfoBlock::setHost(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors)
	{
		DROP_IF(!_Host.empty(),context+"Attempting to set a host more than once for the same info block",++errors; return);
		DROP_IF(entry.empty(),context+"Attempting to set an empty host",++errors; return);
		_Host=entry;
	}

	void CInfoBlock::setStartOrder(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors)
	{
		DROP_IF(!_StartOrder.empty(),context+"Attempting to set a startOrder more than once for the same info block",++errors; return);
		DROP_IF(entry.empty(),context+"Attempting to set an empty startOrder",++errors; return);
		_StartOrder=entry;
	}

	void CInfoBlock::addUseEntry(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors)
	{
		DROP_IF(_UseEntries.find(entry) != _UseEntries.end(), context + "Ignoring duplicate refference to 'use' clause: " + entry.c_str(), return);
		_UseEntries.insert(entry);
	}

	void CInfoBlock::addDataEntry(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors)
	{
		DROP_IF(_DataEntries.find(entry) != _DataEntries.end(), context + "Ignoring duplicate refference to 'data' clause: " + entry.c_str(), return);
		_DataEntries.insert(entry);
	}

	void CInfoBlock::addCfgEntry(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors)
	{
		_CfgEntries.push_back(entry);
	}

	void CInfoBlock::addCfgEntryPost(const NLMISC::CSString& entry,const NLMISC::CSString& context,uint32& errors)
	{
		_CfgEntriesPost.push_back(entry);
	}

	void CInfoBlock::addCfgFile(const NLMISC::CSString& fileName,const NLMISC::CSString& context,uint32& errors)
	{
		// make sure a file name is supplied
		DROP_IF(fileName.empty(),context+"No file name found following 'cfgFile'", ++errors;return);

		// read in the src file
		NLMISC::CSString fileContents;
		fileContents.readFromFile(fileName);
		DROP_IF(fileContents.empty(),"File not found: "+fileName, ++errors;return);

		// split the file contents into lines
		NLMISC::CVectorSString lines;
		fileContents.splitLines(lines);

		// append the lines to the '_CfgEntries' container
		_CfgEntries.insert(_CfgEntries.end(),lines.begin(),lines.end());
	}

	void CInfoBlock::addCfgFilePost(const NLMISC::CSString& fileName,const NLMISC::CSString& context,uint32& errors)
	{
		// make sure a file name is supplied
		DROP_IF(fileName.empty(),context+"No file name found following 'cfgFilePost'", ++errors;return);

		// read in the src file
		NLMISC::CSString fileContents;
		fileContents.readFromFile(fileName);
		DROP_IF(fileContents.empty(),"File not found: "+fileName, ++errors;return);

		// split the file contents into lines
		NLMISC::CVectorSString lines;
		fileContents.splitLines(lines);

		// prepend the lines to the '_CfgEntriesPost' container
		_CfgEntriesPost.insert(_CfgEntriesPost.begin(),lines.begin(),lines.end());
	}

	uint32 CInfoBlock::getNumParents() const
	{
		return _NumParents;
	}

	const NLMISC::CSString& CInfoBlock::getName() const
	{
		return _Name;
	}

	const NLMISC::CSString& CInfoBlock::getDomainName() const
	{
		return _DomainName;
	}

	bool CInfoBlock::isDomain() const
	{
		return !_DomainName.empty();
	}

	bool CInfoBlock::isShard() const
	{
		return !_ShardName.empty();
	}

	void CInfoBlock::setupChildren(CInfoContainer* container,uint32& errors)
	{
		// start by clearing out the child vector that we're going to fill
		_Children.clear();

		// iterate over the the 'use' clauses
		for (TUseEntries::iterator it= _UseEntries.begin(); it!= _UseEntries.end(); ++it)
		{
			const NLMISC::CSString& theEntry= *it;
			
			// try to get a pointer to the refferenced info block...
			CInfoBlock* infoBlockPtr= container->getInfoBlock(theEntry);
			DROP_IF(infoBlockPtr == NULL, "Failed to find block named '" + theEntry + "' while fixing up children of block: " + _Name.c_str(), ++errors; continue);

			// make sure that this block doesn't figure amongst the children of the refferenced info block (to avoid circular refs)
			DROP_IF(_haveCircularRef(infoBlockPtr), "Circular dependency found between definitions of '" + _Name + "' and '" + theEntry.c_str() + "'", ++errors; continue);

			// add the info block to our children
			_Children.push_back(infoBlockPtr);
			++(infoBlockPtr->_NumParents);
		}
	}

	void CInfoBlock::buildExeSet(CInfoContainer* container,uint32& errors,const SExeRecord& parentExeRecord)
	{
		// setup a record to accumulate data into as we traverse the tree, starting
		// with a copy of the data passed in from parents
		SExeRecord theExe= parentExeRecord;

		// add a chunk to the exe record's 'FullName' property
		theExe.FullName+= (theExe.FullName.empty()?"":".")+ _Name;

		// make sure we don't have any field duplication...
		DROP_IF(!_DomainName.empty() && !theExe.DomainName.empty(),	"more than one domain found in: "+theExe.FullName,	++errors );
		DROP_IF(!_ShardName.empty() && !theExe.ShardName.empty(),	"more than one shard found in: "+theExe.FullName,	++errors );
		DROP_IF(!_CmdLine.empty() && !theExe.CmdLine.empty(),		"more than one cmdLine found in: "+theExe.FullName,		++errors );
		DROP_IF(!_Host.empty() && !theExe.Host.empty(),				"more than one host found in: "+theExe.FullName,	++errors );
		WARN_IF(!_UniqueName.empty() && !theExe.UniqueName.empty(), "replacing name '" + theExe.UniqueName + "' with '" + _UniqueName.c_str() + "' in: " + theExe.FullName.c_str());

		// fill our own data into the exe record
		if (!_DomainName.empty())	theExe.DomainName =	_DomainName;
		if (!_ShardName.empty())	theExe.ShardName =	_ShardName;
		if (!_UniqueName.empty())	theExe.UniqueName =	_UniqueName;
		if (!_CmdLine.empty())		theExe.CmdLine =	_CmdLine;
		if (!_Host.empty())			theExe.Host =		_Host;
		if (!_StartOrder.empty())	theExe.StartOrder =	_StartOrder;
		// merge contents of 2 sets
		theExe.DataEntries.insert(	_DataEntries.begin(),	_DataEntries.end()	);
		// append or pre-pend contents of one vector to another
		theExe.CfgEntries.insert( theExe.CfgEntries.end(), _CfgEntries.begin(), _CfgEntries.end() );
		theExe.CfgEntriesPost.insert( theExe.CfgEntriesPost.begin(), _CfgEntriesPost.begin(), _CfgEntriesPost.end() );

		// if this is the node with the cmdLine then think about updating the unique name...
		if (!_CmdLine.empty() && theExe.UniqueName.empty())
		{
			theExe.UniqueName= _Name;
		}

		// do something with the exe record depending on whetherwe're a tree branch or leaf
		if (_Children.empty())
		{
			// merge the cfg entries together to make a single block
			theExe.CfgEntries.insert(theExe.CfgEntries.end(),theExe.CfgEntriesPost.begin(),theExe.CfgEntriesPost.end());
			theExe.CfgEntriesPost.clear();

			// this is a leaf node (it has no children) so it must describe an executable
			container->addExe(theExe,errors);
		}
		else
		{
			// this is a branch node so recurse into children
			for (TChildren::iterator it=_Children.begin();it!=_Children.end();++it)
			{
				(*it)->buildExeSet(container,errors,theExe);
			}
		}
	}

	void CInfoBlock::serial(NLMISC::IStream& stream)
	{
		if (stream.isReading())
		{
			// if we're reading then we clear out the children vector - it'll be rebuilt at the end of the serial
			_Children.clear();
		}

		stream.serial(_Name);
		stream.serial(_NumParents);
		stream.serial(_DomainName);
		stream.serial(_ShardName);
		stream.serial(_UniqueName);
		stream.serial(_CmdLine);
		stream.serial(_Host);

		stream.serialCont(_UseEntries);
		stream.serialCont(_DataEntries);
		stream.serialCont(_CfgEntries);
		stream.serialCont(_CfgEntriesPost);
	}

	void CInfoBlock::dump(NLMISC::CLog& log) const
	{
		//*****************************
		// NOTE:
		// This method is called to
		// create a text save of the
		// data that we contain - it
		// must output ALL data that
		// read() requires and in a 
		// read()-compatible format
		//*****************************

		log.displayNL("define %s // refferenced by %u other defines",_Name.c_str(),_NumParents);

		if (!_DomainName.empty())
		{
			log.displayNL("\tdomain\t%s",_DomainName.c_str());
		}

		if (!_ShardName.empty())
		{
			log.displayNL("\tshard\t%s",_ShardName.c_str());
		}

		if (!_UniqueName.empty())
		{
			log.displayNL("\tname\t%s",_UniqueName.c_str());
		}

		if (!_CmdLine.empty())
		{
			log.displayNL("\tcmdLine\t%s",_CmdLine.c_str());
		}

		if (!_Host.empty())
		{
			log.displayNL("\thost\t%s",_Host.c_str());
		}

		for (TUseEntries::const_iterator it=_UseEntries.begin(); it!=_UseEntries.end(); ++it)
		{	
			log.displayNL("\tuse\t%s",it->c_str());
		}

		for (TDataEntries::const_iterator it=_DataEntries.begin(); it!=_DataEntries.end(); ++it)
		{
			log.displayNL("\tdata\t%s",it->c_str());
		}

		for (TCfgEntries::const_iterator it=_CfgEntries.begin(); it!=_CfgEntries.end(); ++it)
		{
			log.displayNL("\tcfg\t%s",it->c_str());
		}

		for (TCfgEntries::const_iterator it=_CfgEntriesPost.begin(); it!=_CfgEntriesPost.end(); ++it)
		{
			log.displayNL("\tcfgAfter\t%s",it->c_str());
		}

		log.displayNL("");
	}

	bool CInfoBlock::_haveCircularRef(CInfoBlock* other) const
	{
		// in the case of a circular refference we end up with the 'other'=='this'
		if (this==other)
			return true;

		// recurse into children looking for a deep circular ref
		for (TChildren::const_iterator it= other->_Children.begin();it!=other->_Children.end();++it)
		{
			CInfoBlock* child= *it;
			if (_haveCircularRef(child))
				return true;
		}

		// no circular ref found so return false
		return false;
	}

	//-----------------------------------------------------------------------------
	// methods CInfoContainer
	//-----------------------------------------------------------------------------

	void CInfoContainer::clear()
	{
		_InfoBlocks.clear();
		_CurrentInfoBlock= NULL;
		_ExeRecords.clear();
		_DomainExes.clear();
	}

	bool CInfoContainer::empty() const
	{
		return _InfoBlocks.empty();
	}

	bool CInfoContainer::read(const NLMISC::CSString& fileName)
	{
		// start by clearing out our contents...
		clear();

		// setup some basics
		uint32 errors= 0;
		TFileNameSet fileNameSet;

		// read in the src file
		_readFile(fileName,errors,fileNameSet);

		// build the blocks into a tree
		_buildDomainTree(errors);

		// build the set of executables from the tree
		_buildExeSet(errors);

		// make sure that no errors were encountered...
		DROP_IF(errors!=0,NLMISC::toString("%s: Parse Failed: %u errors found",fileName.c_str(),errors),clear(); return false);

		return true;
	}

	void CInfoContainer::_readFile(const NLMISC::CSString& fileName,uint32& errors,TFileNameSet& fileNameSet)
	{
		// read in the src file
		NLMISC::CSString fileContents;
		fileContents.readFromFile(fileName);
		DROP_IF(fileContents.empty(),"File not found: "+fileName, ++errors;return);

		// split the file into lines
		NLMISC::CVectorSString lines;
		fileContents.splitLines(lines);

		// process the lines one by one
		for (uint32 i=0;i<lines.size();++i)
		{
			// setup a context string to pre-pend to error messages
			NLMISC::CSString context= NLMISC::toString("%s:%u: ",fileName.c_str(),i);

			// remove comments and encapsulating blanks
			NLMISC::CSString line= lines[i].splitToLineComment().strip();
			if (line.empty()) continue;

			// split the line into keyword and args
			NLMISC::CSString args= line;
			NLMISC::CSString keyword= args.strtok(" \t");
			NLMISC::CSString rawArgs= lines[i].strip();
			rawArgs.strtok(" \t");

			// try to treat the keyword
			if (keyword=="include")
			{
				DROP_IF(args.empty(), context + "No file name found following 'include': " + line.c_str(), ++errors; continue);
				DROP_IF(fileNameSet.find(args) != fileNameSet.end(), context + "Warning: Duplicate 'include' block ignored: " + line.c_str(), continue);
				fileNameSet.insert(args);
				_readFile(args.unquoteIfQuoted(),errors,fileNameSet);
			}
			else if (keyword=="define")
			{
				DROP_IF(args.empty(), context + "No block name found following 'define': " + line.c_str(), ++errors; continue);
				DROP_IF(_InfoBlocks.find(args) != _InfoBlocks.end(), context + "Duplicate 'define' block found: " + line.c_str(), ++errors; continue);
				// create a new info block and push it into our infoblock set
				_CurrentInfoBlock= new CInfoBlock(args);
				_InfoBlocks[args]= _CurrentInfoBlock;
			}
			else
			{
				DROP_IF(_CurrentInfoBlock == NULL, context + "Expecting 'define <block_name>' but found: " + line.c_str(), ++errors; continue);

				if		(keyword=="domain")			{ _CurrentInfoBlock->setDomainName(args,context,errors); }
				else if	(keyword=="shard")			{ _CurrentInfoBlock->setShardName(args,context,errors); }
				else if (keyword=="name")			{ _CurrentInfoBlock->setUniqueName(args,context,errors); }
				else if (keyword=="cmdLine")		{ _CurrentInfoBlock->setCmdLine(args,context,errors); }
				else if (keyword=="host")			{ _CurrentInfoBlock->setHost(args,context,errors); }
				else if (keyword=="startOrder")		{ _CurrentInfoBlock->setStartOrder(args,context,errors); }
				else if (keyword=="use")			{ _CurrentInfoBlock->addUseEntry(args,context,errors); }
				else if (keyword=="data")			{ _CurrentInfoBlock->addDataEntry(args,context,errors); }
				else if	(keyword=="cfg")			{ _CurrentInfoBlock->addCfgEntry(rawArgs,context,errors); }
				else if	(keyword=="cfgAfter")		{ _CurrentInfoBlock->addCfgEntryPost(rawArgs,context,errors); }
				else if	(keyword=="cfgFile")		{ _CurrentInfoBlock->addCfgFile(args,context,errors); }
				else if	(keyword=="cfgFileAfter")	{ _CurrentInfoBlock->addCfgFilePost(args,context,errors); }
				else								{ DROP(context + "Unrecognised keyword: " + line.c_str(), ++errors; continue); }
			}
		}
	}

	void CInfoContainer::_buildDomainTree(uint32& errors)
	{
		// iterate over the info block container, setting up 'child' vectors
		for (TInfoBlocks::iterator it= _InfoBlocks.begin(); it!= _InfoBlocks.end(); ++it)
		{
			CInfoBlock& theInfoBlock= *it->second;
			theInfoBlock.setupChildren(this,errors);
		}

		// display the list of orphans
		for (TInfoBlocks::iterator it= _InfoBlocks.begin(); it!= _InfoBlocks.end(); ++it)
		{
			CInfoBlock& theInfoBlock= *it->second;
			WARN_IF(!theInfoBlock.isDomain() && theInfoBlock.getNumParents()==0,"Found unrefferenced info block: "+theInfoBlock.getName());
		}
	}

	void CInfoContainer::_buildExeSet(uint32& errors)
	{
		// iterate over the info block container, looking for domains
		for (TInfoBlocks::iterator it= _InfoBlocks.begin(); it!= _InfoBlocks.end(); ++it)
		{
			CInfoBlock& theInfoBlock= *it->second;
			if (theInfoBlock.isDomain())
			{
				const NLMISC::CSString& domainName= theInfoBlock.getDomainName();
				DROP_IF(_DomainExes.find(domainName)!=_DomainExes.end(),"Duplicate domain name found: "+domainName,++errors;continue);
				nldebug("Building executable set for domain: %s",domainName.c_str());
				theInfoBlock.buildExeSet(this,errors);
			}

			// fixup the names in the domain to make them unique

			// run through the exes a first time to determine which unique names are unique and which are not
			std::map<NLMISC::CSString, uint32> nameCounts;
			for (TExeRecords::iterator it2= _ExeRecords.begin(); it2!=_ExeRecords.end(); ++it2)
			{
				// skip anything that's not from our domain
				if (it2->DomainName!=theInfoBlock.getDomainName())
					continue;

				// get hold of the name
				NLMISC::CSString& name= it2->UniqueName;
				// yell if the name already looks like a 'fixed up' name
				DROP_IF(name.right(3).left(1)=="_" && (name.right(2)=="00" || name.right(2).atoui()!=0),"Appending '_' to name ending in '_00' style format as this can clash with auto renumbering => "+name+'_',name+='_')
				// compose a second version of the name with the shard name added
				NLMISC::CSString name_with_shard = name + '_' + it2->ShardName.c_str();
				// insert both versions of the name into the unique name counter
				++nameCounts[name];
				++nameCounts[name_with_shard];
			}

			// run through the exes a second time to fix names that are not unique
			std::map<NLMISC::CSString, uint32> nameIdx;
			for (TExeRecords::iterator it2= _ExeRecords.begin(); it2!=_ExeRecords.end(); ++it2)
			{
				// skip anything that's not from our domain
				if (it2->DomainName!=theInfoBlock.getDomainName())
					continue;

				// get hold of the name
				NLMISC::CSString& name= it2->UniqueName;
				// if the name is unique then continue
				if (nameCounts[name]==1)
					continue;

				// compose a second version of the name with the shard name added
				name+='_';
				name+=it2->ShardName;
				// if the name is unique within the shard then continue
				if (nameCounts[name]==1)
					continue;

				// make the name name unique by appending a number to it
				uint32 idx= ++nameIdx[name];
				name+=NLMISC::toString("_%02u",idx);
			}
		}
	}

	void CInfoContainer::serial(NLMISC::IStream& stream)
	{
		if (stream.isReading())
		{
			// start by clearing out our contents...
			clear();

			// get the number of info blocks from the stream
			uint32 count;
			stream.serial(count);

			// get the info blocks from the stream one by one
			for (uint32 i=0;i<count;++i)
			{
				_CurrentInfoBlock= new CInfoBlock;
				stream.serial(*_CurrentInfoBlock);
				_InfoBlocks[_CurrentInfoBlock->getName()]= _CurrentInfoBlock;
			}

			// setup an error accumulator
			uint32 errors= 0;

			// build the blocks into a tree
			_buildDomainTree(errors);

			// build the set of executables from the tree
			_buildExeSet(errors);

			// make sure that no errors were encountered...
			DROP_IF(errors!=0,NLMISC::toString("Serial Failed: %u errors found",errors), clear();return);

			// note - on exit, _CurrentInfoBlock refferences the last info block read
		}
		else
		{
			// put the number of info blocks to the stream
			uint32 count= (uint32)_InfoBlocks.size();
			stream.serial(count);
			// put the info blocks to the stream one by one
			for (TInfoBlocks::iterator it= _InfoBlocks.begin(); it!=_InfoBlocks.end(); ++it)
			{
				stream.serial(*(it->second));
			}
		}
	}

	void CInfoContainer::getHostNames(THostNames& result) const
	{
		// clear out the result before we begin work...
		result.clear();

		// use a little set to avoid adding host names more than once
		std::set<NLMISC::CSString> namesFound;

		// fill the result in from our internal data
		bool found= false;
		for (TExeRecords::const_iterator it= _ExeRecords.begin(); it!=_ExeRecords.end(); ++it)
		{
			const SExeRecord& theApp= *it;

			// ignore exes that are on hosts that we've already dealt with
			if (namesFound.find(theApp.Host)!=namesFound.end()) continue;

			// we've found a new host so add it to our result container
			result.push_back(theApp.Host);
			namesFound.insert(theApp.Host);

			found=true;
		}

		if (!found)
		{
			// as a note - if we get here it is because the request has failed - no hosts were identified with the given domain
			// this is not an error case so we just add a debug for info
			nldebug("getHostNames failed - no hosts found");
		}
	}

	void CInfoContainer::getDomainNames(TDomainNames& result) const
	{
		// clear out the result before we begin work...
		result.clear();

		// fill the result in from our internal data
		for (TDomainExes::const_iterator dit= _DomainExes.begin(); dit!=_DomainExes.end(); ++dit)
		{
			const NLMISC::CSString& domainName= dit->first;
			result.push_back(domainName);
		}

		if (result.empty())
		{
			// as a note - if we get here it is because the request has failed - no domains were found
			// this is not an error case so we just add a debug for info
			nldebug("getDomainNames failed - no domains found");
		}
	}

	void CInfoContainer::getShardNames(const TDomainName& domainName,TShardNames& result) const
	{
		// clear out the result before we begin work...
		result.clear();

		// fill the result in from our internal data
		bool found= false;
		for (TDomainExes::const_iterator dit= _DomainExes.begin(); dit!=_DomainExes.end(); ++dit)
		{
			// ignore shards that aren't in the requested domain
			const NLMISC::CSString& domName= dit->first;
			if (domName!=domainName) continue;

			// run throught he shards for our chosen domain...
			const TShardExes& shards= dit->second;
			for (TShardExes::const_iterator sit= shards.begin(); sit!=shards.end(); ++sit)
			{
				const NLMISC::CSString shardName= sit->first;
				result.push_back(shardName);
			}

			found=true;
		}

		if (!found)
		{
			// as a note - if we get here it is because the request has failed - no shards were identified with the given domain
			// this is not an error case so we just add a debug for info
			nldebug("getShardNames failed for domainName('%s')",domainName.c_str());
		}
	}

	void CInfoContainer::getAppNames(const THostName& hostName,const TDomainName& domainName,TAppNames& result) const
	{
		// clear out the result before we begin work...
		result.clear();

		// fill the result in from our internal data
		bool found= false;
		for (TExeRecords::const_iterator it= _ExeRecords.begin(); it!=_ExeRecords.end(); ++it)
		{
			const SExeRecord& theApp= *it;

			// ignore exes that aren't in the requested domain
			if (theApp.DomainName!=domainName) continue;

			// ignore exes that are on the wrong host
			if (theApp.Host!=hostName) continue;

			// we've found a new host so add it to our result container
			result.push_back(theApp.UniqueName);

			found=true;
		}

		if (!found)
		{
			// as a note - if we get here it is because the request has failed - no apps were identified with the given host and domain
			// this is not an error case so we just add a debug for info
			nldebug("getAppNames failed for hostName('%s'), domainName('%s')",hostName.c_str(),domainName.c_str());
		}
	}

	void CInfoContainer::getAppNames(const THostName& hostName,const TDomainName& domainName,const TShardName& shardName,TAppNames& result) const
	{
		// clear out the result before we begin work...
		result.clear();

		// fill the result in from our internal data
		bool found= false;
		for (TExeRecords::const_iterator it= _ExeRecords.begin(); it!=_ExeRecords.end(); ++it)
		{
			const SExeRecord& theApp= *it;

			// ignore exes that aren't in the requested domain
			if (theApp.DomainName!=domainName) continue;

			// ignore exes that are on the wrong host
			if (theApp.Host!=hostName) continue;

			// ignore exes that are on the shard host
			if (theApp.ShardName!=shardName) continue;

			// we've found a new host so add it to our result container
			result.push_back(theApp.UniqueName);

			found=true;
		}

		if (!found)
		{
			// as a note - if we get here it is because the request has failed - no apps were identified with the given host, shard and domain
			// this is not an error case so we just add a debug for info
			nldebug("getAppNames failed for hostName('%s'), domainName('%s'), shardName('%s')",hostName.c_str(),domainName.c_str(),shardName.c_str());
		}
	}

	void CInfoContainer::getHost(const THostName& hostName,SHostDescription& result) const
	{
		// clear out the result before we begin work...
		result.clear();
		result.HostName= hostName;

		// use a set to buildup lists of unique host names
		typedef std::set<NLMISC::CSString> TNameSet;
		TNameSet domainNames;

		// fill the result in from our internal data
		bool found= false;
		for (TExeRecords::const_iterator it= _ExeRecords.begin(); it!=_ExeRecords.end(); ++it)
		{
			const SExeRecord& theApp= *it;

			// ignore exes that aren't on the requested host
			if (theApp.Host!=hostName) continue;

			// add this exe's domain to the domains set
			domainNames.insert(theApp.DomainName);

			found=true;
		}

		// copy the hosts set to the result record
		for (TNameSet::const_iterator it= domainNames.begin(); it!=domainNames.end(); ++it)
		{
			result.Domains.push_back(*it);
		}

		if (!found)
		{
			// as a note - if we get here it is because the request has failed - no host was identified with the given name
			// this is not an error case so we just add a debug for info
			nldebug("getHost failed for hostName('%s')",hostName.c_str());
		}
	}

	void CInfoContainer::getDomain(const TDomainName& domainName, SDomainDescription& result) const
	{
		// clear out the result before we begin work...
		result.clear();
		result.DomainName= domainName;

		// use a couple of sets to buildup lists of unique host and shard names
		typedef std::set<NLMISC::CSString> TNameSet;
		TNameSet hostNames;
		TNameSet shardNames;

		// fill the result in from our internal data
		bool found= false;
		for (TExeRecords::const_iterator it= _ExeRecords.begin(); it!=_ExeRecords.end(); ++it)
		{
			const SExeRecord& theApp= *it;

			// ignore exes that aren't in the requested domain
			if (theApp.DomainName!=domainName) continue;

			// add this exe's shard to the shards set
			shardNames.insert(theApp.ShardName);

			// add this exe's host to the hosts set
			hostNames.insert(theApp.Host);

			// add this exe's unique name to the result apps record
			result.Apps.push_back(theApp.UniqueName);

			found=true;
		}

		// copy the hosts set to the result record
		for (TNameSet::const_iterator it= hostNames.begin(); it!=hostNames.end(); ++it)
		{
			result.Hosts.push_back(*it);
		}

		// copy the shards set to the result record
		for (TNameSet::const_iterator it= shardNames.begin(); it!=shardNames.end(); ++it)
		{
			result.Shards.push_back(*it);
		}

		if (!found)
		{
			// as a note - if we get here it is because the request has failed - no domain was identified with the given name
			// this is not an error case so we just add a debug for info
			nldebug("getDomain failed for domainName('%s')",domainName.c_str());
		}
	}

	void CInfoContainer::getShard(const TDomainName& domainName,const TShardName& shardName,SShardDescription& result) const
	{
		// clear out the result before we begin work...
		result.clear();
		result.DomainName= domainName;
		result.ShardName= shardName;

		// use a set to buildup lists of unique host names
		typedef std::set<NLMISC::CSString> TNameSet;
		TNameSet hostNames;

		// fill the result in from our internal data
		bool found= false;
		for (TExeRecords::const_iterator it= _ExeRecords.begin(); it!=_ExeRecords.end(); ++it)
		{
			const SExeRecord& theApp= *it;

			// ignore exes that aren't in the requested domain
			if (theApp.DomainName!=domainName) continue;

			// ignore exes that aren't in the requested shard
			if (theApp.ShardName!=shardName) continue;

			// add this exe's host to the hosts set
			hostNames.insert(theApp.Host);

			// add this exe's unique name to the result apps record
			result.Apps.push_back(theApp.UniqueName);

			found=true;
		}

		// copy the hosts set to the result record
		for (TNameSet::const_iterator it= hostNames.begin(); it!=hostNames.end(); ++it)
		{
			result.Hosts.push_back(*it);
		}

		if (!found)
		{
			// as a note - if we get here it is because the request has failed - no shard was identified with the given name and domain
			// this is not an error case so we just add a debug for info
			nldebug("getShard failed for domainName('%s'), shardName('%s')",domainName.c_str(),shardName.c_str());
		}
	}

	void CInfoContainer::getApp(const TDomainName& domainName,const TAppName& appName,SAppDescription& result) const
	{
		// clear out the result before we begin work...
		result.clear();
		result.DomainName= domainName;
		result.AppName= appName;

		// use a set to buildup lists of unique host names
		typedef std::set<NLMISC::CSString> TNameSet;
		TNameSet domainNames;

		// fill the result in from our internal data
		for (TExeRecords::const_iterator it= _ExeRecords.begin(); it!=_ExeRecords.end(); ++it)
		{
			const SExeRecord& theApp= *it;

			// skip exes that aren't in the requested domain
			if (theApp.DomainName!=domainName) continue;

			// skip exes that don't have the correct name
			if (theApp.UniqueName!=appName) continue;

			// we've found the exe so fill in the result record...
			result.ShardName= theApp.ShardName;
			result.HostName=  theApp.Host;
			result.StartOrder= theApp.StartOrder;
			result.CmdLine=	  theApp.CmdLine;

			// setup the config file to start initialised with the app name for this app
			result.CfgFile= 
				"// Auto generated config file\n"
				"// Use with commandline: "+theApp.CmdLine+"\n"
				"AESAliasName= \"" + appName.c_str() + "\";\n"
				"\n";

			// copy the cfg set to the result record (the cfgAfter set should have been merged in already)
			for (SExeRecord::TCfgEntries::const_iterator cit= theApp.CfgEntries.begin(); cit!=theApp.CfgEntries.end(); ++cit)
			{
				result.CfgFile+=*cit;
				result.CfgFile+='\n';
			}

			// copy the dataEntries set to the result record
			for (SExeRecord::TDataEntries::const_iterator dit= theApp.DataEntries.begin(); dit!=theApp.DataEntries.end(); ++dit)
			{
				result.DataPacks.push_back(*dit);
			}

			// we're all done so we can return merrily
			return;
		}

		// as a note - if we get here it is because the request has failed - no app was identified with the given name and domain
		// this is not an error case so we just add a debug for info
		nldebug("getApp failed for domainName('%s'), appName('%s')",domainName.c_str(),appName.c_str());
	}

	void CInfoContainer::dumpDomains(NLMISC::CLog& log) const
	{
		for (TDomainExes::const_iterator dit= _DomainExes.begin(); dit!=_DomainExes.end(); ++dit)
		{
			const NLMISC::CSString& domainName= dit->first;
			const TShardExes& shards= dit->second;
			log.displayNL("-- domain: %s",domainName.c_str());

			for (TShardExes::const_iterator sit= shards.begin(); sit!=shards.end(); ++sit)
			{
				const NLMISC::CSString& shardName= sit->first;
				const TExeIdx& exeIdx= sit->second;
				log.displayNL("   -- shard: %s",shardName.c_str());

				for (TExeIdx::const_iterator eit= exeIdx.begin(); eit!=exeIdx.end(); ++eit)
				{
					uint32 idx= *eit;
					nlassert(idx<_ExeRecords.size());
					const SExeRecord& theExe= _ExeRecords[idx];
					log.displayNL("      -- %s:%s (%s)",theExe.Host.c_str(),theExe.CmdLine.c_str(),theExe.UniqueName.c_str());
				}
			}
		}
	}

	void CInfoContainer::dumpInfoBlocks(NLMISC::CLog& log) const
	{
		log.displayNL("//------------------------------------------------------------------------------");
		log.displayNL("// Dump of cfg database file contents");
		log.displayNL("//------------------------------------------------------------------------------");
		for (TInfoBlocks::const_iterator it=_InfoBlocks.begin(); it!=_InfoBlocks.end(); ++it)
		{
			it->second->dump(log);
		}
		log.displayNL("//------------------------------------------------------------------------------");
	}

	CInfoBlock* CInfoContainer::getInfoBlock(const NLMISC::CSString& name)
	{
		TInfoBlocks::iterator it= _InfoBlocks.find(name);
		return (it==_InfoBlocks.end())? NULL: it->second;
	}

	void CInfoContainer::addExe(const SExeRecord& exeRecord,uint32& errors)
	{
	//	nldebug("Adding CmdLine: %s",exeRecord.FullName.c_str());

		// note: if we hit errors then we continue anyway to make sure we display a complete set of error messages
		DROP_IF(exeRecord.DomainName.empty(),	"No 'domain' property found in: "+exeRecord.FullName,	++errors );
		DROP_IF(exeRecord.ShardName.empty(),	"No 'shard' property found in: "+exeRecord.FullName,	++errors );
		DROP_IF(exeRecord.CmdLine.empty(),		"No 'cmdLine' property found in: "+exeRecord.FullName,	++errors );
		DROP_IF(exeRecord.Host.empty(),			"No 'host' property found in: "+exeRecord.FullName,		++errors );
		DROP_IF(exeRecord.CfgEntries.empty(),	"No 'cfg' entriesfound in: "+exeRecord.FullName,		++errors );

		// add a refference from the domains' shard map to the exe...
		_DomainExes[exeRecord.DomainName][exeRecord.ShardName].push_back((uint32)_ExeRecords.size());

		// we may have hit errors but we go ahead anyway as in the case of errors the whole thing will be cleared out anyway
		_ExeRecords.push_back(exeRecord);
	}

	//-----------------------------------------------------------------------------
	// methods CDeploymentConfigurationImplementation
	//-----------------------------------------------------------------------------

	bool CDeploymentConfigurationImplementation::read(const NLMISC::CSString& fileName)
	{
		// setup a temp container to hold the version of the file that we're reading
		CInfoContainer container;

		// do the reading and make sure we catch any possible execeptions (like for read in progress)
		try
		{
			container.read(fileName);
		}
		catch(...)
		{
			container.clear();
		}

		// if the read failed for whatever reason then giveup
		DROP_IF(container.empty(),"Failed to update deployment configuration from file: "+fileName,return false);

		// copy the temp container into our internal object
		_InfoContainer= container;

		// display a funky victory message
		nlinfo("Deployment configuration successfully updated from file: %s",fileName.c_str());

		return true;
	}

	void CDeploymentConfigurationImplementation::write(const NLMISC::CSString& fileName)
	{
		// create a displayer to gather the output of the command
		class CStringDisplayer: public IDisplayer
		{
		public:
			NLMISC::CSString Data;
			void doDisplay( const CLog::TDisplayInfo& args, const char *message)
			{
				Data += message;
			}
		};

		// instantiate the displayer and a log object and assign one to the other
		CStringDisplayer stringDisplayer;
		NLMISC::CLog myLog;
		myLog.addDisplayer(&stringDisplayer);

		// dump the info blocks to our log object (accumulating the result as a string)
		dumpInfoBlocks(myLog);

		// write the text accumulated in the log object to a text file
		stringDisplayer.Data.writeToFile(fileName);
	}

	void CDeploymentConfigurationImplementation::serial(NLMISC::IStream& stream)
	{
		// setup a temp container to hold the version of the file that we're reading (if we're reading)
		// and fill the container in with our internal object just in case we're writing
		CInfoContainer container= _InfoContainer;

		// do the serial and make sure we catch any possible execeptions (like for read in progress)
		try
		{
			stream.serial(container);
		}
		catch(...)
		{
			container.clear();
		}

		// if the serial failed for whatever reason then giveup
		DROP_IF(container.empty(),"Failed to serial deployment configuration: ",return);

		// copy the temp container into our internal object (incase this was a read operation)
		_InfoContainer= container;

		// display a funky victory message
		nlinfo("Deployment configuration successfully serialised");
	}

	void CDeploymentConfigurationImplementation::getDomainNames(TDomainNames& result) const
	{
		_InfoContainer.getDomainNames(result);
	}

	void CDeploymentConfigurationImplementation::getShardNames(const TDomainName& domainName,TShardNames& result) const
	{
		_InfoContainer.getShardNames(domainName,result);
	}

	void CDeploymentConfigurationImplementation::getHostNames(THostNames& result) const
	{
		_InfoContainer.getHostNames(result);
	}

	void CDeploymentConfigurationImplementation::getAppNames(const THostName& hostName,const TDomainName& domainName,TAppNames& result) const
	{
		_InfoContainer.getAppNames(hostName,domainName,result);
	}

	void CDeploymentConfigurationImplementation::getAppNames(const THostName& hostName,const TDomainName& domainName,const TShardName& shardName,TAppNames& result) const
	{
		_InfoContainer.getAppNames(hostName,domainName,shardName,result);
	}

	void CDeploymentConfigurationImplementation::getHost(const THostName& hostName,SHostDescription& result) const
	{
		_InfoContainer.getHost(hostName,result);
	}

	void CDeploymentConfigurationImplementation::getDomain(const TDomainName& domainName, SDomainDescription& result) const
	{
		_InfoContainer.getDomain(domainName,result);
	}

	void CDeploymentConfigurationImplementation::getShard(const TDomainName& domainName,const TShardName& shardName,SShardDescription& result) const
	{
		_InfoContainer.getShard(domainName,shardName,result);
	}

	void CDeploymentConfigurationImplementation::getApp(const TDomainName& domainName,const TAppName& appName,SAppDescription& result) const
	{
		_InfoContainer.getApp(domainName,appName,result);
	}

	void CDeploymentConfigurationImplementation::dumpInfoBlocks(NLMISC::CLog& log) const
	{
		_InfoContainer.dumpInfoBlocks(log);
	}

	void CDeploymentConfigurationImplementation::dumpDomains(NLMISC::CLog& log) const
	{
		_InfoContainer.dumpDomains(log);
	}


	//-----------------------------------------------------------------------------
	// methods CDeploymentConfiguration
	//-----------------------------------------------------------------------------

	CDeploymentConfiguration& CDeploymentConfiguration::getInstance()
	{
		return CSingleton<CDeploymentConfigurationImplementation>::getInstance();
	}

} // end of namespace

//NLMISC_CATEGORISED_COMMAND(depcfg,readDepCfgFile,"(re)read the deployment cfg file","[<file name>=\"server_park_database.txt\"]")
//{
//	NLMISC::CSString fileName= defaultDeploymentConfigurationFileName;
//
//	switch (args.size())
//	{
//	case 1:
//		fileName=args[0];
//		break;
//
//	case 0:
//		break;
//
//	default:
//		return false;
//	}
//
//	DEPCFG::CDeploymentConfiguration::getInstance().read(fileName);
//
//	return true;
//}
//
//NLMISC_CATEGORISED_COMMAND(depcfg,writeDepCfgFile,"write the deployment cfg file","[<file name>=\"saved_server_park_database.txt\"]")
//{
//	NLMISC::CSString fileName= CSString("saved_") + defaultDeploymentConfigurationFileName;
//
//	switch (args.size())
//	{
//	case 1:
//		fileName=args[0];
//		break;
//
//	case 0:
//		break;
//
//	default:
//		return false;
//	}
//
//	DEPCFG::CDeploymentConfiguration::getInstance().read(fileName);
//
//	return true;
//}
//
//NLMISC_CATEGORISED_COMMAND(depcfg,saveDepCfgBinary,"write a binary version of the deployment file to disk","<file name>")
//{
//	if (args.size()!=1)
//		return false;
//
//	NLMISC::COFile outf(args[0]);
//	outf.serial(DEPCFG::CDeploymentConfiguration::getInstance());
//
//	return true;
//}
//
//NLMISC_CATEGORISED_COMMAND(depcfg,loadDepCfgBinary,"read a binary version of the deployment file from disk","<file name>")
//{
//	if (args.size()!=1)
//		return false;
//
//	NLMISC::CIFile inf(args[0]);
//	inf.serial(DEPCFG::CDeploymentConfiguration::getInstance());
//
//	return true;
//}

NLMISC_CATEGORISED_COMMAND(depcfg,dumpDepCfgInfoBlocks,"dump the raw info blocks for the deployment config singleton","")
{
	if (args.size()!=0)
		return false;

	DEPCFG::CDeploymentConfiguration::getInstance().dumpInfoBlocks(log);

	return true;
}

NLMISC_CATEGORISED_COMMAND(depcfg,dumpDepCfgDomains,"dump the domain set for the deployment config singleton","")
{
	if (args.size()!=0)
		return false;

	DEPCFG::CDeploymentConfiguration::getInstance().dumpDomains(log);

	return true;
}

NLMISC_CATEGORISED_COMMAND(depcfg,dumpDepCfgHosts,"dump the host set for the deployment config singleton","")
{
	if (args.size()!=0)
		return false;

	log.displayNL("--------------------------------------------");
	log.displayNL("Hosts");
	log.displayNL("--------------------------------------------");

	DEPCFG::THostNames hostNames;
	DEPCFG::CDeploymentConfiguration::getInstance().getHostNames(hostNames);
	sort(hostNames.begin(),hostNames.end());
	for (DEPCFG::THostNames::iterator hit= hostNames.begin(); hit!=hostNames.end(); ++hit)
	{
		DEPCFG::SHostDescription host;
		DEPCFG::CDeploymentConfiguration::getInstance().getHost(*hit,host);
		log.displayNL("Host %s (%d domains)",host.HostName.c_str(),host.Domains.size());

		for (DEPCFG::TDomainNames::iterator dit= host.Domains.begin(); dit!=host.Domains.end(); ++dit)
		{
			log.displayNL("-- Domain %s",dit->c_str());
		}
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(depcfg,dumpDepCfgShards,"dump the shard set for the deployment config singleton","")
{
	if (args.size()!=0)
		return false;

	log.displayNL("--------------------------------------------");
	log.displayNL("Shards");
	log.displayNL("--------------------------------------------");

	DEPCFG::TDomainNames domainNames;
	DEPCFG::CDeploymentConfiguration::getInstance().getDomainNames(domainNames);
	sort(domainNames.begin(),domainNames.end());
	for (DEPCFG::TDomainNames::iterator dit= domainNames.begin(); dit!=domainNames.end(); ++dit)
	{
		DEPCFG::SDomainDescription domain;
		DEPCFG::CDeploymentConfiguration::getInstance().getDomain(*dit,domain);
		log.displayNL("Domain %s (%d shards with %d apps on %d hosts)",domain.DomainName.c_str(),domain.Shards.size(),domain.Apps.size(),domain.Hosts.size());

		DEPCFG::TShardNames shardNames;
		DEPCFG::CDeploymentConfiguration::getInstance().getShardNames(*dit,shardNames);
		nlassert(shardNames==domain.Shards);
		for (DEPCFG::TShardNames::iterator sit= shardNames.begin(); sit!=shardNames.end(); ++sit)
		{
			DEPCFG::SShardDescription shard;
			DEPCFG::CDeploymentConfiguration::getInstance().getShard(*dit,*sit,shard);
			log.displayNL("-- Shard %s/%s (%d apps on %d hosts)",shard.DomainName.c_str(),shard.ShardName.c_str(),shard.Apps.size(),shard.Hosts.size());

			for (DEPCFG::THostNames::iterator hit= shard.Hosts.begin(); hit!=shard.Hosts.end(); ++hit)
			{
				log.displayNL("   -- Host %s",hit->c_str());
				DEPCFG::TAppNames appNames;
				DEPCFG::CDeploymentConfiguration::getInstance().getAppNames(*hit,*dit,*sit,appNames);
				for (DEPCFG::TAppNames::iterator ait= appNames.begin(); ait!=appNames.end(); ++ait)
				{
					DEPCFG::SAppDescription app;
					DEPCFG::CDeploymentConfiguration::getInstance().getApp(*dit,*ait,app);
					uint32 cfgFileLines=app.CfgFile.countLines();
					uint32 numDataPacks= (uint32)app.DataPacks.size();
					log.displayNL("      -- App: %-20s: %s (cfg file length: %d lines, data packs used: %d)",app.AppName.c_str(),app.CmdLine.c_str(),cfgFileLines,numDataPacks);
				}
			}
		}
	}

	return true;
}
