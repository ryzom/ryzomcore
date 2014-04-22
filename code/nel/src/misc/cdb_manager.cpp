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

#include "stdmisc.h"
#include "nel/misc/cdb_manager.h"

namespace NLMISC{

	CCDBManager::CCDBManager( const char *rootNodeName, uint maxBanks ) : bankHandler( maxBanks )
	{
		_Database = new CCDBNodeBranch( std::string( rootNodeName ) );
	}

	CCDBManager::~CCDBManager()
	{
		if( _Database != NULL )
		{
			_Database->clear();
			delete _Database;
			_Database = NULL;
		}
	}

	CCDBNodeLeaf* CCDBManager::getDbLeaf( const std::string &name, bool create )
	{
		if( name.empty() )
			return NULL;

		CCDBNodeLeaf *leaf = NULL;
		leaf = dynamic_cast< CCDBNodeLeaf* >( _Database->getNode( ICDBNode::CTextId( name ), create ) );
		return leaf;
	}

	CCDBNodeBranch* CCDBManager::getDbBranch( const std::string &name )
	{
		if( name.empty() )
			return NULL;
		
		CCDBNodeBranch	*branch = NULL;
		branch = dynamic_cast< CCDBNodeBranch* >( _Database->getNode( ICDBNode::CTextId( name ), false ) );
		return branch;
	}


	void CCDBManager::delDbNode( const std::string &name )
	{
		if( name.empty() )
			return;
		
		_Database->removeNode( ICDBNode::CTextId( name ) );
	}
	
	void CCDBManager::addBranchObserver( const char *branchName, ICDBNode::IPropertyObserver *observer, const std::vector< std::string >& positiveLeafNameFilter )
	{
		CCDBNodeBranch *b = dynamic_cast< CCDBNodeBranch* >( _Database->getNode( ICDBNode::CTextId( std::string( branchName ) ), false ) );
		if( b == NULL )
			return;
		branchObservingHandler.addBranchObserver( b, observer, positiveLeafNameFilter );
	}
	
	void CCDBManager::addBranchObserver( CCDBNodeBranch *branch, ICDBNode::IPropertyObserver *observer, const std::vector< std::string >& positiveLeafNameFilter )
	{
		if( branch == NULL )
			return;
		branchObservingHandler.addBranchObserver( branch, observer, positiveLeafNameFilter );
	}
	
	void CCDBManager::addBranchObserver( const char *branchName, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer, const char **positiveLeafNameFilter, uint positiveLeafNameFilterSize )
	{
		CCDBNodeBranch *b = dynamic_cast< CCDBNodeBranch* >( _Database->getNode( ICDBNode::CTextId( std::string( branchName ) ), false ) );
		if( b == NULL )
			return;
		branchObservingHandler.addBranchObserver( b, dbPathFromThisNode, observer, positiveLeafNameFilter, positiveLeafNameFilterSize );
	}
	
	void CCDBManager::addBranchObserver( CCDBNodeBranch *branch, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer, const char **positiveLeafNameFilter, uint positiveLeafNameFilterSize )
	{
		if( branch == NULL )
			return;
		branchObservingHandler.addBranchObserver( branch, dbPathFromThisNode, observer, positiveLeafNameFilter, positiveLeafNameFilterSize );
	}
	
	void CCDBManager::removeBranchObserver( const char *branchName, ICDBNode::IPropertyObserver* observer )
	{
		CCDBNodeBranch *b = dynamic_cast< CCDBNodeBranch* >( _Database->getNode( ICDBNode::CTextId( std::string( branchName ) ), false ) );
		if( b == NULL )
			return;
		branchObservingHandler.removeBranchObserver( b, observer );
	}
	
	void CCDBManager::removeBranchObserver( CCDBNodeBranch *branch, ICDBNode::IPropertyObserver* observer )
	{
		if( branch == NULL )
			return;
		branchObservingHandler.removeBranchObserver( branch, observer );
	}
	
	void CCDBManager::removeBranchObserver( const char *branchName, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer )
	{
		CCDBNodeBranch *b = dynamic_cast< CCDBNodeBranch* >( _Database->getNode( ICDBNode::CTextId( std::string( branchName ) ), false ) );
		if( b == NULL )
			return;
		branchObservingHandler.removeBranchObserver( b, dbPathFromThisNode, observer );
	}
	
	void CCDBManager::removeBranchObserver( CCDBNodeBranch *branch, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer )
	{
		if( branch == NULL )
			return;
		branchObservingHandler.removeBranchObserver( branch, dbPathFromThisNode, observer );
	}
	
	void CCDBManager::addFlushObserver( CCDBBranchObservingHandler::IBranchObserverCallFlushObserver *observer )
	{
		if( observer == NULL )
			return;
		branchObservingHandler.addFlushObserver( observer );
	}
	
	void CCDBManager::removeFlushObserver( CCDBBranchObservingHandler::IBranchObserverCallFlushObserver *observer )
	{
		if( observer == NULL )
			return;
		branchObservingHandler.removeFlushObserver( observer );
	}
	
	void CCDBManager::flushObserverCalls()
	{
		branchObservingHandler.flushObserverCalls();
	}

	void CCDBManager::resetBank( uint gc, uint bank )
	{
		_Database->resetNode( gc, bankHandler.getUIDForBank( bank ) );
	}

	void CCDBManager::resizeBanks( uint newSize )
	{
		bankHandler.resize( newSize );
	}

}
