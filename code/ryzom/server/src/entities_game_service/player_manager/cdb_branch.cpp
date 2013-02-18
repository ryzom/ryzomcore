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



#include "stdpch.h"

//#define TRACE_READ_DELTA
//#define TRACE_WRITE_DELTA
//#define TRACE_SET_VALUE




//////////////
// Includes //
//////////////
#include "player_manager/cdb_branch.h"
#include "player_manager/cdb_leaf.h"
#include "nel/misc/xml_auto_ptr.h"
#include <libxml/parser.h>

////////////////
// Namespaces //
////////////////
using namespace NLMISC;
using namespace std;


#include "nel/misc/i_xml.h"
#include "nel/misc/progress_callback.h"

#include <libxml/parser.h>
//#include <io.h>
#include <fcntl.h>
#include <string.h>

#include <string>


using namespace std;
using namespace NLMISC;


NL_INSTANCE_COUNTER_IMPL(CCDBStructNodeBranch);
/*
 * Label the tree branches with the matching bank name
 *
 * Precondition: the tree was constructed with init() and some _BankLabels contain one label
 * (no children of these them can have a label).
 *
 * Postcondition: the ancestors of each node that had one label have this label (shifted to
 * recognize propagated labels).
 */
void	ICDBStructNode::labelBranch( TCDBBank bank )
{
	if ( _BankLabels && (_BankLabels->size() == 1) && (*(_BankLabels->begin()) == bank) )
	{
		// Propagate label to ancestors, until root
		nldebug( "CDB: Bank %s: retropropagating from node %s to root", CCDBStructBanks::getBankName(bank), getParent()?getName()->c_str():"(root)" );
		ICDBStructNode *node = this;
		while ( (node = node->getParent()) != NULL )
		{
			if ( ! node->_BankLabels )
				node->_BankLabels = new set<sint>;
			node->_BankLabels->insert( (bank+1) << CDB_BANK_SHIFT ); // shifted to differenciate labels set at loading and labels propagated back
		}
		// Don't continue the recursion lower in the siblings
	}
	else
	{
		// Recurse
		labelSiblings( bank );
	}
}


/*
 * Move the siblings that match the bank name to the destination tree
 *
 * Precondition: the tree was constructed and labelled backward until the root (see labelBranch()).
 * Consequently, one branch can have several labels.
 *
 * When a branch is found with a single original matching label (not shifted), it is moved to the
 * specified bank tree (i.e. it is removed from the source tree).
 * For each branch with a shifted matching label, recreate it in the bank tree (we can't remove it
 * from the source tree because it can be used in several bank trees).
 *
 * Obsolete (not true anymore):
 * If a branch does not match the current bank, create an empty node to keep the same indices.
 */
void	CCDBStructNodeBranch::moveBranchesToBank( CCDBStructNodeBranch *destRoot, TCDBBank bank )
{
	if ( (!_BankLabels) || _BankLabels->empty() )
	{
		nlwarning( "CDB: Found a branch (%s) with no bank label", getParent()?getName()->c_str():"root" );
		if ( ! _BankLabels )
			nlerror( "Malformed database.xml" );
	}

	//const string *n = getName(); nldebug( "CDB: Node %p %s (%s): %u labels", this, n?n->c_str():"", getParent()?"branch":"root", _BankLabels?_BankLabels->size():0 );
	if ( _BankLabels->find( (bank+1) << CDB_BANK_SHIFT ) != _BankLabels->end() )
	{
		// The node contains a propagated matching label
		CCDBStructNodeBranch *newBranch = destRoot;
		if ( getParent() ) // not the root
		{
			nldebug( "CDB: Bank %s: Building branch %s", CCDBStructBanks::getBankName(bank), getName()->c_str() );

			// Recreate the branch in the bank tree (without the nodes and labels)
			newBranch = new CCDBStructNodeBranch();
			newBranch->setAtomic( isAtomic() );
			destRoot->attachChild( newBranch, *getName() );
		}

		// Continue the recursion to move the matching nodes to the new branch
		vector<ICDBStructNode*>::iterator in;
		for ( in=_Nodes.begin(); in!=_Nodes.end(); ++in )
		{
			if ( *in )
				(*in)->moveBranchesToBank( newBranch, bank );
		}
	}
	else if ( _BankLabels->find( bank ) != _BankLabels->end() ) // the originally labelled node
	{
		// Move the entire branch to the bank (deleting the label) and stop recursion
		delete _BankLabels;
		_BankLabels = NULL;
		if ( getParent() ) // not the root
		{
			string name = *getName();
			nldebug( "CDB: Bank %s: Attaching branch %s", CCDBStructBanks::getBankName(bank), name.c_str() );
			getParent()->detachChild( this );
			destRoot->attachChild( this, name );
		}
	}
	/*else
	{
		if ( getParent() )
		{
			// Stop recursion, but add an empty node to keep the same indices as on the client
			nldebug( "CDB: Bank %s: Building empty node %s", CCDBStructBanks::getBankName(bank), getName()->c_str() );
			CCDBStructNodeBranch *newEmptyBranch = new CCDBStructNodeBranch();
			destRoot->attachChild( newEmptyBranch, *getName() );
		}
	}*/
}


/*
 * Set a label while loading from XML
 *
 * Precondition: _BankLabels == NULL;
 */
void	ICDBStructNode::setLabel( const std::string& bankName )
{
	if ( ! bankName.empty() )
	{
		nlassert( _BankLabels == NULL );
		nldebug( "CDB: Read bank name %s at node %s", bankName.c_str(), getParent()?getName()->c_str():"(root)" );
		TCDBBank bank = CCDBStructBanks::readBankName( bankName );
		if ( bank != INVALID_CDB_BANK )
		{
			_BankLabels = new set<sint>;
			_BankLabels->insert( bank );
		}
	}
}

/*
 * Set the data index into the leaves. The passed index is incremented for each leaf leaf or atomic branch.
 * The "returned" value of index is the number of indices set.
 */
void	CCDBStructNodeBranch::initDataIndex( TCDBDataIndex& index )
{
	if ( _AtomicFlag )
	{
		_DataIndex = index;
		checkIfNotMaxIndex();
		++index;
	}

	for ( uint i=0; i!=_Nodes.size(); ++i )
	{
		_Nodes[i]->initDataIndex( index );
	}
}


/*
 * Set the data index into the leaves and atomic branches.
 * The passed index is incremented for each leaf or atomic branch.
 * When setting an index, call the provided callback.
 */
void	CCDBStructNodeBranch::initIdAndCallForEachIndex( CBinId& id, void (*callback)(ICDBStructNode*, void*), void *arg )
{
	if ( _AtomicFlag )
	{
		callback( this, arg );
	}

	//nldebug( "CDB: Initializing branch %s %p (%u subnodes)", _Parent?getName()->c_str():"(root)", this, _Nodes.size() );
	for ( uint i=0; i!=_Nodes.size(); ++i )
	{
		id.push( i, _IdBits );
		_Nodes[i]->initIdAndCallForEachIndex( id, callback, arg );
		id.pop();
	}
}


//-----------------------------------------------
//	init
//
//-----------------------------------------------
static inline void addNode( ICDBStructNode *newNode,
						    std::string newName,
						    CCDBStructNodeBranch* parent,
							std::vector<std::string> &names,
							std::vector<ICDBStructNode *> &nodes,
							std::map<std::string,uint32> &index,
							xmlNodePtr &child,
							const char *bankName,
							bool atomBranch,
							bool clientOnly,
							NLMISC::IProgressCallback &progressCallBack )
{
	names.push_back(newName);
	index.insert(make_pair(newName,(NLMISC::TSStringId)nodes.size()));
	nodes.push_back(newNode);
	nodes.back()->setParent(parent);
	nodes.back()->setLabel(bankName);

	// ClientOnly nodes have just an empty branch (no children) to have the same indices as the client
	if ( ! clientOnly )
	{
		nodes.back()->setAtomic( parent->isAtomic() || atomBranch );
		nodes.back()->init(child, progressCallBack);
	}
}


/*
 *
 */
void CCDBStructNodeBranch::init( xmlNodePtr node, NLMISC::IProgressCallback &progressCallBack ) 
{
	xmlNodePtr child;

	// look for other branches within this branch
	uint countNode = CIXml::countChildren (node, "branch") + CIXml::countChildren (node, "leaf");
	uint nodeId = 0;
	for (child = CIXml::getFirstChildNode (node, "branch"); child; 	child = CIXml::getNextChildNode (child, "branch"))
	{
		// Progress bar
		progressCallBack.progress ((float)nodeId/(float)countNode);
		progressCallBack.pushCropedValues ((float)nodeId/(float)countNode, (float)(nodeId+1)/(float)countNode);

		CXMLAutoPtr name((const char*)xmlGetProp (child, (xmlChar*)"name"));
		CXMLAutoPtr count((const char*)xmlGetProp (child, (xmlChar*)"count"));
		CXMLAutoPtr bank((const char*)xmlGetProp (child, (xmlChar*)"bank"));
		CXMLAutoPtr atom((const char*)xmlGetProp (child, (xmlChar*)"atom"));
		CXMLAutoPtr clientonly((const char*)xmlGetProp (child, (xmlChar*)"clientonly"));

		string sBank, sAtom, sClientonly;
		if ( bank ) sBank = bank.getDatas();
		if ( atom ) sAtom = atom.getDatas();
		if ( clientonly ) sClientonly = clientonly.getDatas();
		nlassert((const char *) name != NULL);
		if ((const char *) count != NULL)
		{
			// dealing with an array of entries
			uint countAsInt;
			NLMISC::fromString(count, countAsInt);
			nlassert((const char *) count != NULL);

			for (uint i=0;i<countAsInt;i++)
			{
				// Progress bar
				progressCallBack.progress ((float)i/(float)countAsInt);
				progressCallBack.pushCropedValues ((float)i/(float)countAsInt, (float)(i+1)/(float)countAsInt);
				
//				nlinfo("+ %s%d",name,i);
				addNode( new CCDBStructNodeBranch, string(name.getDatas())+toString(i), this, _Names, _Nodes, _Index, child, sBank.c_str(), sAtom=="1", sClientonly=="1", progressCallBack );

/*
				_Names.push_back(string(name)+toString(i));
				_Index.insert(make_pair(string(name)+toString(i),_Nodes.size()));
				_Nodes.push_back(new CCDBStructNodeBranch);
				_Nodes.back()->setParent(this);
				_Nodes.back()->init(child);
*/
//				nlinfo("-");

				// Progress bar
				progressCallBack.popCropedValues ();
			}			
		}
		else
		{
			// dealing with a single entry
//			nlinfo("+ %s",name);
			addNode( new CCDBStructNodeBranch, string(name.getDatas()), this, _Names, _Nodes, _Index, child, sBank.c_str(), sAtom=="1", sClientonly=="1", progressCallBack );
/*
			_Names.push_back(name);
			_Index.insert(make_pair(name,_Nodes.size()));
			_Nodes.push_back(new CCDBStructNodeBranch);
			_Nodes.back()->setParent(this);
			_Nodes.back()->init(child);
*/
//			nlinfo("-");
		}
			
		// Progress bar
		progressCallBack.popCropedValues ();
		nodeId++;
	}

	// look for leaves of this branch
	for (child = CIXml::getFirstChildNode (node, "leaf"); child; 	child = CIXml::getNextChildNode (child, "leaf"))
	{
		// Progress bar
		progressCallBack.progress ((float)nodeId/(float)countNode);
		progressCallBack.pushCropedValues ((float)nodeId/(float)countNode, (float)(nodeId+1)/(float)countNode);

		CXMLAutoPtr name((const char*)xmlGetProp (child, (xmlChar*)"name"));
		CXMLAutoPtr count((const char*)xmlGetProp (child, (xmlChar*)"count"));
		CXMLAutoPtr bank((const char*)xmlGetProp (child, (xmlChar*)"bank"));

		string sBank;
		if ( bank ) sBank = bank.getDatas();
		nlassert((const char *) name != NULL);
		if ((const char *) count != NULL)
		{
			// dealing with an array of entries
			uint countAsInt;
			NLMISC::fromString(count, countAsInt);
			nlassert((const char *) count != NULL);

			for (uint i=0;i<countAsInt;i++)
			{
				// Progress bar
				progressCallBack.progress ((float)i/(float)countAsInt);
				progressCallBack.pushCropedValues ((float)i/(float)countAsInt, (float)(i+1)/(float)countAsInt);

//				nlinfo("  %s%d",name,i);
				addNode( new CCDBStructNodeLeaf, string(name.getDatas())+toString(i), this, _Names, _Nodes, _Index, child, sBank.c_str(), false, false,  progressCallBack );
/*
				_Names.push_back(string(name)+toString(i));
				_Index.insert(make_pair(string(name)+toString(i),_Nodes.size()));
				_Nodes.push_back(new CCDBStructNodeLeaf);
				_Nodes.back()->setParent(this);
				_Nodes.back()->init(child);
*/

				// Progress bar
				progressCallBack.popCropedValues ();
			}			
		}
		else
		{
//			nlinfo("  %s",name);

			addNode( new CCDBStructNodeLeaf, string(name.getDatas()), this, _Names, _Nodes, _Index, child, sBank.c_str(), false, false, progressCallBack );
/*
			_Names.push_back(name);
			_Index.insert(make_pair(name,_Nodes.size()));
			_Nodes.push_back(new CCDBStructNodeLeaf);
			_Nodes.back()->setParent(this);
			_Nodes.back()->init(child);
*/
		}
			
		// Progress bar
		progressCallBack.popCropedValues ();
		nodeId++;
	}

	calcIdBits();
}


/*
 * Count and store the number of bits required to store the id
 */
void CCDBStructNodeBranch::calcIdBits()
{
	if ( _Names.size() > 0)
		for ( _IdBits=1; _Names.size() > unsigned(1<<_IdBits) ; _IdBits++ ) {}
	else
		_IdBits = 0;
}


//-----------------------------------------------
//	attachChild
//
//-----------------------------------------------
void CCDBStructNodeBranch::attachChild( ICDBStructNode * node, const string& nodeName )
{
	//nlassert(_Parent==NULL); // ??? node->_Parent?

	//nldebug( "CDB: Attaching child %p to node %p (%s)", node, this, nodeName.c_str() );
	node->setParent(this);
	_Nodes.push_back( node );
	_Names.push_back( nodeName );
	_Index.insert( make_pair(nodeName,(NLMISC::TSStringId)_Nodes.size() -1) );

	calcIdBits();
} // attachChild //


/*
 * Set a node pt to NULL (not deleting)
 */
void CCDBStructNodeBranch::detachChild( ICDBStructNode *node )
{
	vector<ICDBStructNode*>::iterator in;
	for ( in=_Nodes.begin(); in!=_Nodes.end(); ++in )
	{
		if ( (*in) == node )
		{
			//nldebug( "CDB: Detaching child %p from node %p", node, this );
			(*in) = NULL;
			break;
		}
	}
}


//-----------------------------------------------
//	getNode
//
//-----------------------------------------------
ICDBStructNode * CCDBStructNodeBranch::getNode( std::vector<uint16>& ids, uint idx )
{
	if( idx == ids.size() )
	{
		return this;
	}
	else
	{
		return _Nodes[ids[idx]]->getNode(ids,idx+1);
	}

} // getNode //



//-----------------------------------------------
//	getNode
//
//-----------------------------------------------
ICDBStructNode * CCDBStructNodeBranch::getNode (const CTextId& id , bool bCreate) 
{
	// lookup next element from textid in my index => idx
	string str = id.readNext();
	map<string,uint32>::iterator itIdx = _Index.find( str );
	// If the node does not exist
	if ( itIdx == _Index.end() )
	{
		if ( bCreate )
		{
			ICDBStructNode *newNode;
			if (id.getCurrentIndex() == id.size() )
				newNode= new CCDBStructNodeLeaf;
			else
				newNode= new CCDBStructNodeBranch;

			_Nodes.push_back( newNode );
			_Names.push_back( str );
			_Index.insert( make_pair(str,(NLMISC::TSStringId)_Nodes.size()-1) );
			newNode->setParent(this);
			itIdx = _Index.find(str);
		}
		else
		{
			return NULL;
		}
	}
	// Get property from child
	if (!id.hasElements())
	{
		return _Nodes[(*itIdx).second];
	}
	return _Nodes[(*itIdx).second]->getNode( id, bCreate );

} // getNode //



//-----------------------------------------------
//	getNode
//
//-----------------------------------------------
ICDBStructNode * CCDBStructNodeBranch::getNode( uint16 idx )
{
	nlassert( idx < _Nodes.size() );
	return _Nodes[idx];

} // getNode //


/*
 * Return the data index corresponding to a text id (from the root)
 */
TCDBDataIndex	CCDBStructNodeBranch::findDataIndex( ICDBStructNode::CTextId& id ) const
{
	// Lookup next element from textid in my index => idx
	string str = id.readNext();
	map<string,uint32>::const_iterator itIdx = _Index.find( str );

	// Get property from child
	if ( itIdx != _Index.end() )
		return _Nodes[(*itIdx).second]->findDataIndex( id );
	else
		return CDB_INVALID_DATA_INDEX;
}


/*
 * Browse the tree, and for each atom branch encountered, call the callback passing the argument
 * provided and the index of the atom branch.
 */
void			CCDBStructNodeBranch::foreachAtomBranchCall( void (*callback)(void*,TCDBDataIndex), void *arg ) const
{
	if ( _AtomicFlag )
	{
		callback( arg, _DataIndex );
	}

	// Recurse
	vector<ICDBStructNode *>::const_iterator itNode;
	for( itNode = _Nodes.begin(); itNode != _Nodes.end(); ++itNode )
	{
		(*itNode)->foreachAtomBranchCall( callback, arg );
	}
}


/*
 * Browse the tree, building the text id, and for each leaf encountered, call the callback
 * passing the argument provided, the index and the text id.
 */
void			CCDBStructNodeBranch::foreachLeafCall( void (*callback)(void*,TCDBDataIndex,CTextId*), CTextId& id, void *arg )
{
	// Recurse
	for ( uint i=0; i!=_Nodes.size(); ++i )
	{
		id.push( _Names[i] );
		_Nodes[i]->foreachLeafCall( callback, id, arg );
		id.pop();
	}
}


/*
 * Browse the tree, and for each leaf encountered, call the callback
 * passing the argument provided and the leaf, and post-incrementing the counter
 */
void			CCDBStructNodeBranch::foreachLeafCall( void (*callback)(void*,CCDBStructNodeLeaf*,uint&), uint& counter, void *arg )
{
	// Recurse
	for ( uint i=0; i!=_Nodes.size(); ++i )
	{
		_Nodes[i]->foreachLeafCall( callback, counter, arg );
	}
}


/*
 * Build the binary id corresponding to the branch
 */
/*void			CCDBStructNodeBranch::buildBinId( CBinId& binId ) const
{
	// Build a vector of the indexes in the tree, going back from the node to the root
	vector<uint> ids;
	uint index;
	const CCDBStructNodeBranch *node = this, *parentNode;
	while ( (parentNode = node->getParent()) )
	{
		parentNode->getNodeIndex( (ICDBStructNode*)node, index );
		ids.push_back( index );
		node = parentNode;
	}

	// Browse the vector backwards to build the binary id
	vector<uint>::reverse_iterator it;
	for ( it=ids.rbegin(); it!=ids.rend(); ++it )
	{
		binId.push( *it, node->idBits() );
		node = static_cast<const CCDBStructNodeBranch*>(node->getNode( *it )); // surely a branch because we get back to where we were
	}
}*/


/*
 * Build the binary id corresponding to the branch (it must have at least one leaf)
 */
void			CCDBStructNodeBranch::buildBinIdFromLeaf( CBinId& binId ) const
{
	uint siblingLevel = 0;
	const CCDBStructNodeLeaf *leaf = findFirstLeaf( siblingLevel );
	if ( leaf )
	{
		binId = leaf->binLeafId();
		for ( uint i=0; i!=siblingLevel; ++i )
			binId.pop();
	}
}


/*
 * Return the first leaf found, and set the number of indirections in siblingLevel
 */
const CCDBStructNodeLeaf *CCDBStructNodeBranch::findFirstLeaf( uint& siblingLevel ) const
{
	++siblingLevel;
	const CCDBStructNodeLeaf *leaf;
	vector<ICDBStructNode*>::const_iterator itNode;
	for( itNode = _Nodes.begin(); itNode != _Nodes.end(); ++itNode )
	{
		leaf = (*itNode)->findFirstLeaf( siblingLevel );
		if ( leaf )
			return leaf;
	}
	--siblingLevel;
	return NULL;
}


/*
 * Return a pointer to a node corresponding to a bank and a property name (from the root)
 */
ICDBStructNode *CCDBStructNodeBranch::getICDBStructNodeFromNameFromRoot( const std::string& name )
{
	ICDBStructNode::CTextId txtId( name );
	return getNode( txtId, false );
}


//-----------------------------------------------
//	Destructor (clear)
//
//-----------------------------------------------
CCDBStructNodeBranch::~CCDBStructNodeBranch()
{
	vector<ICDBStructNode*>::iterator itNode;
	for( itNode = _Nodes.begin(); itNode != _Nodes.end(); ++itNode )
	{
		if ( *itNode )
		{
			delete (*itNode);
		}
	}
	_Nodes.clear();
	_Index.clear();
	_Names.clear();

} // clear //



/*
 *
 */
std::string ICDBStructNode::CBinId::toString() const
{
	string s1 = "Bits: ", s2 = "Idx: ";
	sint nbbits = 0;
	for ( uint i=0; i<Ids.size(); i++ )
	{
		s1 += NLMISC::toString( "%s%d+", s1.c_str(), Ids[i].second );
		s2 += NLMISC::toString( "%s%d ", s2.c_str(), Ids[i].first );
		nbbits += Ids[i].second;
	}
	s1 += NLMISC::toString( "%s = %d bits. %s", s1.c_str(), nbbits, s2.c_str() );
	return s1;
}


#ifdef TRACE_READ_DELTA
#undef TRACE_READ_DELTA
#endif

#ifdef TRACE_WRITE_DELTA
#undef TRACE_WRITE_DELTA
#endif

#ifdef TRACE_SET_VALUE
#undef TRACE_SET_VALUE
#endif

