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
#include "nel/misc/cdb_branch_observing_handler.h"

namespace NLMISC{
	CCDBBranchObservingHandler::CCDBBranchObservingHandler()
	{
		reset();
	}

	CCDBBranchObservingHandler::~CCDBBranchObservingHandler()
	{
		reset();
	}

	void CCDBBranchObservingHandler::flushObserverCalls()
	{
		bool flushed = false;

		do
		{
			flushed = false;
			uint oldList = currentList;
			currentList = 1 - currentList;
			
			std::list< CCDBNodeBranch::ICDBDBBranchObserverHandle* >::iterator itr
				= flushableObservers[ oldList ].begin();
			
			while( itr != flushableObservers[ oldList ].end() )
			{
				currentHandle = *itr;
				++itr;
				
				if( currentHandle->observer() != NULL )
					currentHandle->observer()->update( currentHandle->owner() );
				
				// Update might have removed it
				if( currentHandle != NULL )
					currentHandle->removeFromFlushableList( oldList );

				currentHandle = NULL;
				flushed = true;
			}
			triggerFlushObservers();

		}while( flushed );

		triggerFlushObservers();
	}

	void CCDBBranchObservingHandler::reset()
	{
		currentList = 0;
		currentHandle = NULL;

		for( uint i = 0; i < MAX_OBS_LST; i++ )
			flushableObservers[ i ].clear();
	}

	void CCDBBranchObservingHandler::addBranchObserver( CCDBNodeBranch *branch, ICDBNode::IPropertyObserver *observer, const std::vector< std::string >& positiveLeafNameFilter )
	{
		if( branch == NULL )
			return;

		CCDBDBBranchObserverHandle *handle = new CCDBDBBranchObserverHandle( observer, branch, this );
		branch->addBranchObserver( handle, positiveLeafNameFilter );
	}

	void CCDBBranchObservingHandler::addBranchObserver( CCDBNodeBranch *branch, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer, const char **positiveLeafNameFilter, uint positiveLeafNameFilterSize )
	{
		if( branch == NULL )
			return;

		CCDBDBBranchObserverHandle *handle = new CCDBDBBranchObserverHandle( &observer, branch, this );
		branch->addBranchObserver( handle, dbPathFromThisNode, positiveLeafNameFilter, positiveLeafNameFilterSize );
	}

	void CCDBBranchObservingHandler::removeBranchObserver( CCDBNodeBranch *branch, ICDBNode::IPropertyObserver* observer )
	{
		branch->removeBranchObserver( observer );
	}

	void CCDBBranchObservingHandler::removeBranchObserver( CCDBNodeBranch *branch, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer )
	{
		branch->removeBranchObserver( dbPathFromThisNode, observer );
	}
	
	void CCDBBranchObservingHandler::triggerFlushObservers()
	{
		for( std::vector< IBranchObserverCallFlushObserver* >::iterator itr = flushObservers.begin();
			 itr != flushObservers.end(); itr++ )
			 (*itr)->onObserverCallFlush();
	}
	
	void CCDBBranchObservingHandler::addFlushObserver( IBranchObserverCallFlushObserver *observer )
	{
		std::vector< IBranchObserverCallFlushObserver* >::iterator itr
			= std::find( flushObservers.begin(), flushObservers.end(), observer );
		
		if( itr != flushObservers.end() )
			return;
		
		flushObservers.push_back( observer );
	}
	
	void CCDBBranchObservingHandler::removeFlushObserver( IBranchObserverCallFlushObserver *observer )
	{
		std::vector< IBranchObserverCallFlushObserver* >::iterator itr
			= std::find( flushObservers.begin(), flushObservers.end(), observer );
		
		if( itr == flushObservers.end() )
			return;
		
		flushObservers.erase( itr );
	}

	CCDBBranchObservingHandler::CCDBDBBranchObserverHandle::CCDBDBBranchObserverHandle( NLMISC::ICDBNode::IPropertyObserver *observer, NLMISC::CCDBNodeBranch *owner, CCDBBranchObservingHandler *handler )
	{
		std::fill( _inList, _inList + MAX_OBS_LST, false );
		_observer = observer;
		_owner = owner;
		_handler = handler;
	}

	CCDBBranchObservingHandler::CCDBDBBranchObserverHandle::~CCDBDBBranchObserverHandle()
	{
		_observer = NULL;
	}

	bool CCDBBranchObservingHandler::CCDBDBBranchObserverHandle::observesLeaf( const std::string &leafName )
	{
		if( !_observedLeaves.empty() ){
			std::vector< std::string >::iterator itr
				= std::find( _observedLeaves.begin(), _observedLeaves.end(), leafName );
			
			if( itr == _observedLeaves.end() )
				return false;
			else
				return true;
		}

		return true;
	}

	bool CCDBBranchObservingHandler::CCDBDBBranchObserverHandle::inList( uint list )
	{
		return _inList[ list ];
	}

	void CCDBBranchObservingHandler::CCDBDBBranchObserverHandle::addToFlushableList()
	{
		uint list = _handler->currentList;

		if( _inList[ list ] )
			return;

		_handler->flushableObservers[ list ].push_back( this );
		_inList[ list ] = true;
	}

	void CCDBBranchObservingHandler::CCDBDBBranchObserverHandle::removeFromFlushableList( uint list )
	{
		if( !_inList[ list ] )
			return;

		std::list< CCDBNodeBranch::ICDBDBBranchObserverHandle* >::iterator itr
			= std::find( _handler->flushableObservers[ list ].begin(),
			             _handler->flushableObservers[ list ].end(), this );

		if( itr == _handler->flushableObservers[ list ].end() )
			return;

		if( _handler->currentHandle == this )
			_handler->currentHandle = NULL;

		_handler->flushableObservers[ list ].erase( itr );
		_inList[ list ] = false;
		
	}

	void CCDBBranchObservingHandler::CCDBDBBranchObserverHandle::removeFromFlushableList()
	{
		for( uint i = 0; i < MAX_OBS_LST; i++ )
			removeFromFlushableList( i );
	}
}

