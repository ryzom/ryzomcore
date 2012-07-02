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
#ifndef DEPLOYMENT_CONFIGURATION_H
#define DEPLOYMENT_CONFIGURATION_H

//-----------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/sstring.h"


//-----------------------------------------------------------------------------
// namespace DEPCFG
//-----------------------------------------------------------------------------

namespace DEPCFG
{
	//-----------------------------------------------------------------------------
	// some handy constants
	//-----------------------------------------------------------------------------

	#define defaultDeploymentConfigurationFileName "server_park_database.txt"


	//-----------------------------------------------------------------------------
	// some handy typedefs for identifiers, map keys, etc
	//-----------------------------------------------------------------------------

	typedef NLMISC::CSString TDomainName;
	typedef NLMISC::CSString TShardName;
	typedef NLMISC::CSString THostName;
	typedef NLMISC::CSString TAppName;

	typedef NLMISC::CVectorSString TDomainNames;
	typedef NLMISC::CVectorSString TShardNames;
	typedef NLMISC::CVectorSString THostNames;
	typedef NLMISC::CVectorSString TAppNames;

	typedef NLMISC::CSString TCmdLine;
	typedef NLMISC::CSString TCfgFile;
	typedef NLMISC::CVectorSString TDataPacks;


	//-----------------------------------------------------------------------------
	// struct SDomainDescription
	//-----------------------------------------------------------------------------

	struct SDomainDescription
	{
		//-------------------------------------------------------------------------
		// identifying the domain
		TDomainName		DomainName;
		//-------------------------------------------------------------------------
		// the set of shards and apps belonging to this domain and their hosts
		TShardNames		Shards;
		THostNames		Hosts;
		TAppNames		Apps;

		//-------------------------------------------------------------------------
		// a handy 'clear()' method
		void clear()
		{
			DomainName.clear();
			Shards.clear();
			Hosts.clear();
			Apps.clear();
		}
	};


	//-----------------------------------------------------------------------------
	// struct SShardDescription
	//-----------------------------------------------------------------------------

	struct SShardDescription
	{
		//-------------------------------------------------------------------------
		// identifying the shard by name and domain
		TDomainName		DomainName;
		TShardName		ShardName;
		//-------------------------------------------------------------------------
		// the set of hosts and apps used by this shard
		THostNames		Hosts;
		TAppNames		Apps;

		//-------------------------------------------------------------------------
		// a handy 'clear()' method
		void clear()
		{
			DomainName.clear();
			ShardName.clear();
			Hosts.clear();
			Apps.clear();
		}
	};


	//-----------------------------------------------------------------------------
	// struct SHostDescription
	//-----------------------------------------------------------------------------

	struct SHostDescription
	{
		//-------------------------------------------------------------------------
		// identifying the host (machine's network name)
		THostName		HostName;
		//-------------------------------------------------------------------------
		// the set of domains that refference this host
		TDomainNames	Domains;

		//-------------------------------------------------------------------------
		// a handy 'clear()' method
		void clear()
		{
			HostName.clear();
			Domains.clear();
		}
	};


	//-----------------------------------------------------------------------------
	// struct SAppDescription
	//-----------------------------------------------------------------------------

	struct SAppDescription
	{
		//-------------------------------------------------------------------------
		// identifying the app, the shard it belongs to, machine it runs on, etc
		TDomainName		DomainName;
		TShardName		ShardName;
		THostName		HostName;
		TAppName		AppName;
		NLMISC::CSString		StartOrder;

		//-------------------------------------------------------------------------
		// some real data
		TCmdLine		CmdLine;	// the command line to execute the app
		TCfgFile		CfgFile;	// the cfg file contents for the app
		TDataPacks		DataPacks;	// the set of data packs for the app

		//-------------------------------------------------------------------------
		// a handy 'clear()' method
		void clear()
		{
			DomainName.clear();
			ShardName.clear();
			HostName.clear();
			AppName.clear();
			StartOrder.clear();
			CmdLine.clear();
			CfgFile.clear();
			DataPacks.clear();
		}
	};


	//-----------------------------------------------------------------------------
	// class CDeploymentConfiguration
	//-----------------------------------------------------------------------------

	class CDeploymentConfiguration
	{
	public :
		//-------------------------------------------------------------------------
		// this is a singleton so we have a getInstance() method
		// remaining methods are pure virtuals...

		// get hold of the instance of the singleton that derives from CDeploymentConfiguration
		static CDeploymentConfiguration& getInstance();


		//-------------------------------------------------------------------------
		// methods for reading / serialising deployment configuration

		// read the deployment configuration from a specified file
		//
		// file format is line base with the following format:
		// '//'... comment to end of line
		// 'define' <block_name>		// define a new block
		//		'domain' <domain_name>	// flag this block as a domain and give it a name
		//		'shard' <domain_name>	// flag this block as a shard and give it a name
		//		'use' <block_name>		// add a child block to this block
		//		'name' <block_name>		// set the unique name to use for the executables derived from this block
		//		'cmdLine' <cmd_line>	// setup the command line to use for exes derived from this block
		//		'host' <host_name>		// assign the exes derived from this block to a given host
		//		'cfg' <cfg_line>		// add a line to be included in the cfg file of executables derived from this block
		//		'cfgAfter' <cfg_line>	// as above but cfg lines appended at end of cfg file (not at start)
		//		'data' <data_name>		// add dependency on a given data block to exes derived from this block
		//
		virtual bool read(const NLMISC::CSString& fileName) =0;

		// write the info blocks out to a file (in no particular order)
		// the output file can be re-read via the 'read()' method
		virtual void write(const NLMISC::CSString& fileName) =0;

		// serialise the deployment configuration, for dispatch to / reception from other apps
		virtual void serial(NLMISC::IStream& stream) =0;


		//-------------------------------------------------------------------------
		// read accessors - getting sets of names

		// get the complete set of hosts (machines on which we have apps running)
		virtual void getHostNames(THostNames& result) const =0;
		
		// get the complete set of domains
		virtual void getDomainNames(TDomainNames& result) const =0;

		// get the shards for a given domain
		virtual void getShardNames(const TDomainName& domainName,TShardNames& result) const =0;
		
		// get the apps for a given host and domain
		virtual void getAppNames(const THostName& hostName,const TDomainName& domainName,TAppNames& result) const =0;

		// get the apps for a given host, domain and shard
		virtual void getAppNames(const THostName& hostName,const TDomainName& domainName,const TShardName& shardName,TAppNames& result) const =0;

		
		//-------------------------------------------------------------------------
		// read accessors - getting info structure for a named object

		// get a named host
		virtual void getHost(const THostName& hostName,SHostDescription& result) const =0;
		
		// get a named domains
		virtual void getDomain(const TDomainName& domainName, SDomainDescription& result) const =0;

		// get a named shard for a given domain
		virtual void getShard(const TDomainName& domainName,const TShardName& shardName,SShardDescription& result) const =0;
		
		// get a named app for a given domain
		virtual void getApp(const TDomainName& domainName,const TAppName& appName,SAppDescription& result) const =0;


		//-------------------------------------------------------------------------
		// methods for dumping info to a given log

		// dump raw information (organised by info block, not by domain or host)
		virtual void dumpInfoBlocks(NLMISC::CLog& log) const =0;
		
		// dump info organised by domain
		virtual void dumpDomains(NLMISC::CLog& log) const =0;
	};

}	// end of namespace

//-----------------------------------------------------------------------------
#endif
