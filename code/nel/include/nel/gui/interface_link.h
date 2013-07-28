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



#ifndef CL_INTERFACE_LINK_H
#define CL_INTERFACE_LINK_H

#include "nel/misc/cdb_branch.h"
#include "nel/misc/cdb_branch_observing_handler.h"

namespace NLGUI
{
	class CReflectedProperty;
	class CInterfaceExprValue;
	class CInterfaceExprNode;
	class CInterfaceElement;
	class CInterfaceGroup;

	/** A link in an interface.
	  * A link is an object that can read one or several values from the database, that can evaluate an expression
	  * on these database entries (simple computation, using the CInterfaceExpr class), and that can affect the result to
	  * an interface property that has been exported by an interface element (the export system uses reflect.h).
	  * The first time it is created, it places observers on the database entries that are needed by the expression, so each
	  * time a database value changes, the link is marked as 'triggered'
	  * When updateTrigeredLinks() is called, all links are effectively updated.
	  *
	  * Example of use : connecting a change in the db tree to the 'active' state of a window
	  *
	  * NB : an additionnal action handler can be provided
	  * NB : The links are owned by the interface element (using a smart pointer)
	  * NB : Several targets may be used.
	  *
	  * \author Nicolas Vizerie
	  * \author Nevrax France
	  * \date 2002
	  */
	class CInterfaceLink : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		#ifdef NL_DEBUG
			// for debugging purposes : if this link is 'named' e.g is owner by CInterfaceManager
			// and was created by calling CInterfaceManager::addLink, contains the name of this link
			std::string LinkName;
		#endif
	public:
		struct CTargetInfo
		{
			CInterfaceElement *Elem;
			std::string        PropertyName;
			/** Affect a value to this target.
			  * \return true if the affectation could be made
			  */
			bool affect(const CInterfaceExprValue &value);
		};
		struct CCDBTargetInfo
		{
			NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> Leaf;
			std::string LeafName;
		};


		/// Updates triggered interface links when triggered by the observed branch
		class CInterfaceLinkUpdater : public NLMISC::CCDBBranchObservingHandler::IBranchObserverCallFlushObserver
		{
		public:
			CInterfaceLinkUpdater();
			~CInterfaceLinkUpdater();
			void onObserverCallFlush();
		};

	public:
		CInterfaceLink();
		~CInterfaceLink(); // this object should only be destroyed by a CInterfaceElement
		/** Make a link between the given interface element properties and a value that depends on database entries.
		  * The link is automatically added in the link list of the targets element (it calls CInterfaceElement::addLink), so when all target elements are removed, the link is.
		  * If there are no target element, the link is permanent (removed at exit)
		  * NB : The target is not updated during this call.
		  */
		bool				init(const std::vector<CTargetInfo> &targets, const std::vector<CCDBTargetInfo> &cdbTargets, const std::string &expr, const std::string &actionHandler, const std::string &ahParams, const std::string &ahCond, CInterfaceGroup *parent);
		// force all the links that have been created to update their targets. This can be called when the interface has been loaded, and when the databse entries have been retrieved.
		static void			updateAllLinks();
		// force all trigered links to be updated
		static void			updateTrigeredLinks();
		// remove from the _LinksWithNoTarget list if the link has no target
		void				uninit();

		// Force an update of the target of this link
		void				update();
		/** Remove a target element. It won't be updated anymore by that link
		  * NB : this don't call removeLink() on the target
		  */
		void				removeTarget(CInterfaceElement *elem);
		// Get the number of targets of this link
		uint				getNumTargets() const { return (uint)_Targets.size(); }
		// Get the i-th target
		CInterfaceElement	*getTarget(uint index) const { return _Targets[index]._InterfaceElement; }

		static void			removeAllLinks();

		static void			setTargetProperty (const std::string & Target, const CInterfaceExprValue &val);

		static bool				isUpdatingAllLinks() { return _UpdateAllLinks; }

		/** From a target name of a link, retrieve the target element and its target target property
		  * \return true if the target is valid
		  */
		static bool splitLinkTarget(const std::string &target, CInterfaceGroup *parentGroup, std::string &propertyName, CInterfaceElement *&targetElm);

		/** From several target names of a link (seprated by ','), retrieve the target elements and their target properties
		  * \return true if all targets are valid
		  */
		static bool splitLinkTargets(const std::string &targets, CInterfaceGroup *parentGroup, std::vector<CInterfaceLink::CTargetInfo> &targetsVect);
		static bool splitLinkTargetsExt(const std::string &targets, CInterfaceGroup *parentGroup, std::vector<CInterfaceLink::CTargetInfo> &targetsVect, std::vector<CInterfaceLink::CCDBTargetInfo> &cdbTargetsVect);
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	private:
		friend struct CRemoveTargetPred;
		// a target property
		struct CTarget
		{
			CInterfaceElement	        *_InterfaceElement;
			const CReflectedProperty    *_Property;
		};
	private:
		typedef std::list<CInterfaceLink *> TLinkList;
		typedef NLMISC::CSmartPtr<CInterfaceLink> TLinkSmartPtr;
		typedef std::vector<TLinkSmartPtr> TLinkVect;
		typedef std::vector<NLMISC::ICDBNode *> TNodeVect;
	private:
		std::vector<CTarget>         _Targets;
		std::vector<CCDBTargetInfo>	 _CDBTargets;
		TNodeVect					 _ObservedNodes;
		std::string					 _Expr;
		CInterfaceExprNode			 *_ParseTree;
		std::string					 _ActionHandler;
		std::string					 _AHParams;
		std::string					 _AHCond;
		CInterfaceExprNode			*_AHCondParsed;
		CInterfaceGroup				*_AHParent;
		static TLinkList             _LinkList;
		TLinkList::iterator			 _ListEntry;
		bool						 _On;
		static TLinkVect             _LinksWithNoTarget; // there should be an owner for links with no targets
		static bool					 _UpdateAllLinks;
		///\ name triggered link mgt
		//@{
			// next/previous link that was trigered. NULL means end or start of list
			// each ptr is duplicated because with manage 2 lists : one list in which links are added, and one list in which we update links.
			// This way one link can trigger another with no prb
			CInterfaceLink				*_PrevTriggeredLink[2];
			CInterfaceLink				*_NextTriggeredLink[2];
			bool						 _Triggered[2];
			// global lists
			static CInterfaceLink       *_FirstTriggeredLink[2];
			static CInterfaceLink       *_LastTriggeredLink[2];
			// iterators in current list being updated : they're global so that deleting a CInterfaceLink instance prevent them from becoming dangling pointers
			static CInterfaceLink		*_CurrUpdatedLink;
			static CInterfaceLink		*_NextUpdatedLink;
			// Index of the list in which triggered link must be inserted
			static uint					 _CurrentTriggeredLinkList;

			//
			void	linkInTriggerList(uint list);
			void	unlinkFromTriggerList(uint list);
		//@}

	private:
		/** Inherited from ICDBNode::IPropertyObserver
		  * This doesn't update the node directly, but mark it as 'triggered'
		  * The node is really updated during the call to 'updateTrigeredLinks()'
		  */
		virtual void update(NLMISC::ICDBNode *node);
		void    createObservers(const TNodeVect &nodes);
		void    removeObservers(const TNodeVect &nodes);
		// debug : check that there are as many targets as reference to a link
		void    checkNbRefs();
	};

}

#endif
