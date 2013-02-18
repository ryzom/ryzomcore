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


#ifndef CDB_MANAGER_H
#define CDB_MANAGER_H

#include "nel/misc/cdb_branch.h"
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/cdb_bank_handler.h"
#include "nel/misc/cdb_branch_observing_handler.h"

namespace NLMISC{

	/// Class that encapsulates the separate CDB components
	class CCDBManager{
	
	public:
		/**
		 The constructor
		 @param maxBanks  -  The maximum number of banks to be used

		 */
		CCDBManager( const char *rootNodeName, uint maxBanks );

		~CCDBManager();


		/**
		 Returns the specified leaf node from the database.
		 @param name The name of the leaf node.
		 @param create Specifies if the node should be created if it doesn't exist yet.

		 */
		CCDBNodeLeaf* getDbLeaf( const std::string &name, bool create = true );



		/**
		 Returns the specified branch node from the database.
		 @param name The name of the branch.

		 */
		CCDBNodeBranch* getDbBranch( const std::string &name );


		/**
		 Deletes the specified database node.
		 @param name The name of the database node.

		 */
		void delDbNode( const std::string &name );
		
		/**
		 Adds an observer to a branch of the database.
		 @param branchName The name of the branch we want to observe
		 @param observer   The observer we want to add
		 @param positiveLeafNameFilter A vector of strings containing the names of the leaves we want to observe

		 */
		void addBranchObserver( const char *branchName, ICDBNode::IPropertyObserver *observer, const std::vector< std::string >& positiveLeafNameFilter = std::vector< std::string >() );

		/**
		 Adds an observer to a branch of the database.
		 @param branch  The branch we want to observe
		 @param observer   The observer we want to add
		 @param positiveLeafNameFilter A vector of strings containing the names of the leaves we want to observe

		 */
		void addBranchObserver( CCDBNodeBranch *branch, ICDBNode::IPropertyObserver *observer, const std::vector< std::string >& positiveLeafNameFilter = std::vector< std::string >() );


		/**
		 Adds an observer to a branch of the database.
		 @param branchName The name of the branch we start from
		 @param dbPathFromThisNode The path to the branch we want to observe
		 @param observer   The observer we want to add
		 @param positiveLeafNameFilter An array of strings containing the names of the leaves we want to observe
		 @param positiveLeafNameFilterSize The size of the array

		 */
		void addBranchObserver( const char *branchName, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer, const char **positiveLeafNameFilter = NULL, uint positiveLeafNameFilterSize = 0 );

		
		/**
		 Adds an observer to a branch of the database.
		 @param branch The branch we start from
		 @param dbPathFromThisNode The path to the branch we want to observe
		 @param observer   The observer we want to add
		 @param positiveLeafNameFilter An array of strings containing the names of the leaves we want to observe
		 @param positiveLeafNameFilterSize The size of the array

		 */
		void addBranchObserver( CCDBNodeBranch *branch, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer, const char **positiveLeafNameFilter, uint positiveLeafNameFilterSize );


		/**
		 Removes an observer from a branch in the database.
		 @param branchName The name of the branch
		 @param observer The observer we want to remove

		 */
		void removeBranchObserver( const char *branchName, ICDBNode::IPropertyObserver* observer );


		/**
		 Removes an observer from a branch in the database.
		 @param branch The branch
		 @param observer The observer we want to remove

		 */
		void removeBranchObserver( CCDBNodeBranch *branch, ICDBNode::IPropertyObserver* observer );


		/**
		 Removes an observer from a branch in the database.
		 @param branchName The name of the branch we start from
		 @param dbPathFromThisNode The path to the branch we want to observe from the starting branch
		 @param observer The observer we want to remove

		 */
		void removeBranchObserver( const char *branchName, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer );


		/**
		 Removes an observer from a branch in the database.
		 @param branchName The name of the branch we start from
		 @param dbPathFromThisNode The path to the branch we want to observe from the starting branch
		 @param observer The observer we want to remove

		 */
		void removeBranchObserver( CCDBNodeBranch *branch, const char *dbPathFromThisNode, ICDBNode::IPropertyObserver &observer );


		/**
		 Adds a branch observer call flush observer. ( These are notified after the branch observers are notified )
		 @param observer The observer

		 */
		void addFlushObserver( CCDBBranchObservingHandler::IBranchObserverCallFlushObserver *observer );


		/**
		 Removes a branch observer call flush observer.
		 @param observer The observer
		 */
		void removeFlushObserver( CCDBBranchObservingHandler::IBranchObserverCallFlushObserver *observer );

		/**
		 Notifies the observers whose observed branches were updated.
		 */
		void flushObserverCalls();

		/**
		 Resets the specified bank.
		 @param gc GameCycle ( no idea what it is exactly, probably some time value )
		 @param bank The banks we want to reset

		 */
		void resetBank( uint gc, uint bank );
		
		/**
		 @brief Resizes the bank holders. WARNING: Resets data contained.
		 @param newSize - The new maximum number of banks.
		 */
		void resizeBanks( uint newSize );
	
	protected:
		CCDBBankHandler bankHandler;
		CCDBBranchObservingHandler branchObservingHandler;
		CRefPtr< CCDBNodeBranch > _Database;
	};

}

#endif

