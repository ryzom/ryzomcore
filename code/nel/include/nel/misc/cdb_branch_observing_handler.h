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

#ifndef CDB_BRANCH_OBS_HNDLR
#define CDB_BRANCH_OBS_HNDLR

#include "nel/misc/cdb_branch.h"

namespace NLMISC{

	/**
	 @brief Manages the CDB branch observers.

	 When a leaf's data changes, it notifies the branch, which then marks the observers as notifiable.
	 The marked observers can then be notified and flushed on request.

	 */
	class CCDBBranchObservingHandler{

		enum{
			MAX_OBS_LST  = 2
		};

	public:
		CCDBBranchObservingHandler();

		~CCDBBranchObservingHandler();

		/// Notifies the observers, and flushes the list
		void flushObserverCalls();

		void reset();

		void addBranchObserver( CCDBNodeBranch *branch, ICDBNode::IPropertyObserver *observer, const std::vector< std::string >& positiveLeafNameFilter );

		void addBranchObserver( CCDBNodeBranch *branch, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer, const char **positiveLeafNameFilter, uint positiveLeafNameFilterSize );

		void removeBranchObserver( CCDBNodeBranch *branch, ICDBNode::IPropertyObserver* observer );

		void removeBranchObserver( CCDBNodeBranch *branch, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer );
		
		
		///Observer for branch observer flush events.
		class IBranchObserverCallFlushObserver : public CRefCount{
		public:
			virtual ~IBranchObserverCallFlushObserver(){}
			virtual void onObserverCallFlush() = 0;
		};
	
	private:
		void triggerFlushObservers();
	
	public:
		void addFlushObserver( IBranchObserverCallFlushObserver *observer );
		void removeFlushObserver( IBranchObserverCallFlushObserver *observer );
	
	private:

		/**
		 @brief Handle to a branch observer.

		 The handle stores the owner branch, the observer and remembers if it's marked for notifying the observer.
		 Also it manages adding/removing itself to/from the marked observer handles list, which is handled by CCDBBranchObservingHandler.

		 */
		class CCDBDBBranchObserverHandle : public CCDBNodeBranch::ICDBDBBranchObserverHandle{

		public:

			CCDBDBBranchObserverHandle( ICDBNode::IPropertyObserver *observer, CCDBNodeBranch *owner, CCDBBranchObservingHandler *handler );

			~CCDBDBBranchObserverHandle();

			ICDBNode* owner(){ return _owner; }

			ICDBNode::IPropertyObserver* observer(){ return _observer; }

			bool observesLeaf( const std::string &leafName );

			bool inList( uint list );

			void addToFlushableList();

			void removeFromFlushableList( uint list );

			void removeFromFlushableList();

		private:

			bool _inList[ MAX_OBS_LST ];

			std::vector< std::string > _observedLeaves;

			CCDBNodeBranch *_owner;

			NLMISC::CRefPtr< ICDBNode::IPropertyObserver > _observer;

			CCDBBranchObservingHandler *_handler;

		};

		std::list< CCDBNodeBranch::ICDBDBBranchObserverHandle* > flushableObservers[ MAX_OBS_LST ];

		CCDBNodeBranch::ICDBDBBranchObserverHandle *currentHandle;

		uint currentList;

		std::vector< IBranchObserverCallFlushObserver* > flushObservers;

	};
}

#endif


