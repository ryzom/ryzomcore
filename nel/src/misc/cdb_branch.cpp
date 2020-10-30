// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2013-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

//#define TRACE_READ_DELTA
//#define TRACE_WRITE_DELTA
//#define TRACE_SET_VALUE




//////////////
// Includes //
//////////////
#include "nel/misc/cdb_branch.h"
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/xml_auto_ptr.h"
//#include <iostream.h>

////////////////
// Namespaces //
////////////////
using namespace std;


#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/progress_callback.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/bit_set.h"
#include "nel/misc/cdb_bank_handler.h"

#include <libxml/parser.h>
//#include <io.h>
#include <fcntl.h>
#include <string.h>

#include <string>


using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC{

//-----------------------------------------------
//	init
//
//-----------------------------------------------
static /*inline*/ void addNode( ICDBNode *newNode, std::string newName, CCDBNodeBranch* parent,
						    std::vector<ICDBNode *> &nodes, std::vector<ICDBNode *> &nodesSorted,
							xmlNodePtr &child, const string& bankName,
							bool atomBranch, bool clientOnly,
							IProgressCallback &progressCallBack,
							bool mapBanks, CCDBBankHandler *bankHandler = NULL )
{
	nodesSorted.push_back(newNode);
	nodes.push_back(newNode);
	nodes.back()->setParent(parent);
	nodes.back()->setAtomic( parent->isAtomic() || atomBranch );
	nodes.back()->init(child, progressCallBack);

	// Setup bank mapping for first-level node
	if ( mapBanks && (parent->getParent() == NULL) )
	{
		if ( ! bankName.empty() )
		{
			bankHandler->mapNodeByBank( bankName );
			//nldebug( "CDB: Mapping %s for %s (node %u)", newName.c_str(), bankName.c_str(), nodes.size()-1 );
		}
		else
		{
			nlerror( "Missing bank for first-level node %s", newName.c_str() );
		}
	}
}

void CCDBNodeBranch::init( xmlNodePtr node, IProgressCallback &progressCallBack, bool mapBanks, CCDBBankHandler *bankHandler )
{
	xmlNodePtr child;

	_Sorted = false;
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
		if ( atom ) sAtom = (const char*)atom;
		if ( clientonly ) sClientonly = clientonly.getDatas();
		nlassert((const char *) name != NULL);
		if ((const char *) count != NULL)
		{
			// dealing with an array of entries
			uint countAsInt;
			fromString((const char*) count, countAsInt);

			for (uint i=0;i<countAsInt;i++)
			{
				// Progress bar
				progressCallBack.progress ((float)i/(float)countAsInt);
				progressCallBack.pushCropedValues ((float)i/(float)countAsInt, (float)(i+1)/(float)countAsInt);

//				nlinfo("+ %s%d",name,i);
				string newName = string(name.getDatas())+toString(i);
				addNode( new CCDBNodeBranch(newName), newName, this, _Nodes, _NodesByName, child, sBank, sAtom=="1", sClientonly=="1", progressCallBack, mapBanks, bankHandler );
//				nlinfo("-");

				// Progress bar
				progressCallBack.popCropedValues ();
			}
		}
		else
		{
			// dealing with a single entry
//			nlinfo("+ %s",name);
			string newName = string(name.getDatas());
			addNode( new CCDBNodeBranch(newName), newName, this, _Nodes, _NodesByName, child, sBank, sAtom=="1", sClientonly=="1", progressCallBack, mapBanks, bankHandler );
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
			fromString((const char *) count, countAsInt);

			for (uint i=0;i<countAsInt;i++)
			{
				// Progress bar
				progressCallBack.progress ((float)i/(float)countAsInt);
				progressCallBack.pushCropedValues ((float)i/(float)countAsInt, (float)(i+1)/(float)countAsInt);

//				nlinfo("  %s%d",name,i);
				string newName = string(name.getDatas())+toString(i);
				addNode( new CCDBNodeLeaf(newName), newName, this, _Nodes, _NodesByName, child, sBank, false, false, progressCallBack, mapBanks, bankHandler );

				// Progress bar
				progressCallBack.popCropedValues ();
			}
		}
		else
		{
//			nlinfo("  %s",name);
			string newName = string(name.getDatas());
			addNode( new CCDBNodeLeaf(newName), newName, this, _Nodes, _NodesByName, child, sBank, false, false, progressCallBack, mapBanks, bankHandler );
		}

		// Progress bar
		progressCallBack.popCropedValues ();
		nodeId++;
	}

	// count number of bits required to store the id
	if ( (mapBanks) && (getParent() == NULL) )
	{		
		nlassert( bankHandler != NULL );
		nlassertex( bankHandler->getUnifiedIndexToBankSize() == countNode, ("Mapped: %u Nodes: %u", bankHandler->getUnifiedIndexToBankSize(), countNode) );
		bankHandler->calcIdBitsByBank();
		_IdBits = 0;
	}
	else
	{
		if (!_Nodes.empty())
			for ( _IdBits=1; _Nodes.size() > ((size_t)1 <<_IdBits) ; _IdBits++ ) {}
		else
			_IdBits = 0;
	}

	find(""); // Sort !
}


//-----------------------------------------------
//	attachChild
//
//-----------------------------------------------
void CCDBNodeBranch::attachChild( ICDBNode * node, string nodeName )
{
	nlassert(_Parent==NULL);

	if (node)
	{
		node->setParent(this);
		_Nodes.push_back( node );
		//nldebug ( "CDB: Attaching node" );
		_NodesByName.push_back( node );
		_Sorted = false;
#if NL_CDB_OPTIMIZE_PREDICT
		_PredictNode = node;
#endif
	}

} // attachChild //

//-----------------------------------------------
//	getLeaf
//
//-----------------------------------------------
CCDBNodeLeaf *CCDBNodeBranch::getLeaf( const char *id, bool bCreate )
{
	// get the last name piece
	const char *last = strrchr( id, ':' );
	if( !last )
		return NULL;
	ICDBNode *pNode = find( &last[1] );
	if( !pNode && bCreate )
	{
		pNode = new CCDBNodeLeaf( id );
		_Nodes.push_back( pNode );
		_NodesByName.push_back( pNode );
		_Sorted = false;
		pNode->setParent(this);
	}
	return dynamic_cast<CCDBNodeLeaf *>(pNode);
}

//-----------------------------------------------
//	getNode
//
//-----------------------------------------------
ICDBNode * CCDBNodeBranch::getNode (const CTextId& id, bool bCreate)
{
	// lookup next element from textid in my index => idx
	const string &str = id.readNext();

	ICDBNode *pNode = find(str);
	// If the node do not exists
	if ( pNode == NULL )
	{
		if (bCreate)
		{
			// Yoyo: must not be SERVER or LOCAL, cause definied through xml.
			// This may cause some important crash error
			//nlassert(!id.empty());
			//nlassert(id.getElement(0)!="SERVER");
			//nlassert(id.getElement(0)!="LOCAL");
			ICDBNode *newNode;
			if (id.getCurrentIndex() == id.size() )
				newNode= new CCDBNodeLeaf (str);
			else
				newNode= new CCDBNodeBranch (str);

			_Nodes.push_back( newNode );
			_NodesByName.push_back( newNode );
			_Sorted = false;
			newNode->setParent(this);
			pNode = newNode;
		}
		else
		{
			return NULL;
		}
	}

	// get property from child
	if (!id.hasElements())
		return pNode;

	return pNode->getNode( id, bCreate );

} // getNode //


//-----------------------------------------------
//	getNode
//
//-----------------------------------------------
ICDBNode * CCDBNodeBranch::getNode( uint16 idx )
{
	if ( idx < _Nodes.size() )
		return _Nodes[idx];
	else
		return NULL;

} // getNode //


//-----------------------------------------------
//	write
//
//-----------------------------------------------
void CCDBNodeBranch::write( CTextId& id, FILE * f)
{
	uint i;
	for( i = 0; i < _Nodes.size(); i++ )
	{
		id.push (*_Nodes[i]->getName());
		_Nodes[i]->write(id,f);
		id.pop();
	}

} // write //

//-----------------------------------------------
//	getProp
//
//-----------------------------------------------
sint64 CCDBNodeBranch::getProp( CTextId& id )
{
	// lookup next element from textid in my index => idx
	const string &str = id.readNext();
	ICDBNode *pNode = find( str );
	nlassert( pNode != NULL );

	// get property from child
	return pNode->getProp( id );

} // getProp //



//-----------------------------------------------
// setProp :
// Set the value of a property (the update flag is set to true)
// \param id is the text id of the property/grp
// \param name is the name of the property
// \param value is the value of the property
// \return bool : 'true' if property found.
//-----------------------------------------------
bool CCDBNodeBranch::setProp( CTextId& id, sint64 value )
{

	// lookup next element from textid in my index => idx
	const string &str = id.readNext();
	ICDBNode *pNode = find( str );

	// Property not found.
	if(pNode == NULL)
	{
		nlwarning("Property %s not found in %s", str.c_str(), id.toString().c_str());
		return false;
	}

	// set property in child
	pNode->setProp(id,value);
	// Done
	return true;
}// setProp //


/*
 * Update the database from the delta, but map the first level with the bank mapping (see _CDBBankToUnifiedIndexMapping)
 */
void CCDBNodeBranch::readAndMapDelta( TGameCycle gc, CBitMemStream& s, uint bank, CCDBBankHandler *bankHandler )
{
	nlassert( ! isAtomic() ); // root node mustn't be atomic

	// Read index
	uint32 idx;
	s.serial( idx, bankHandler->getFirstLevelIdBits( bank ) );

	// Translate bank index -> unified index
	idx = bankHandler->getServerToClientUIDMapping( bank, idx );
	if (idx >= _Nodes.size())
	{
		throw Exception ("idx %d > _Nodes.size() %d ", idx, _Nodes.size());
	}

	// Display the Name if we are in verbose mode
	if ( verboseDatabase )
	{
		string displayStr = string("Reading: ") +  *(_Nodes[idx]->getName());
		//CInterfaceManager::getInstance()->getChatOutput()->addTextChild( ucstring( displayStr ),CRGBA(255,255,255,255));
		nlinfo( "CDB: %s%s %u/%d", (!getParent())?"[root] ":"-", displayStr.c_str(), idx, _IdBits );
	}

	// Apply delta to children nodes
	_Nodes[idx]->readDelta( gc, s );
}


//-----------------------------------------------
//	readDelta
//
//-----------------------------------------------
void CCDBNodeBranch::readDelta( TGameCycle gc, CBitMemStream & f )
{
	if ( isAtomic() )
	{
		// Read the atom bitfield
		uint nbAtomElements = countLeaves();
		if(verboseDatabase)
			nlinfo( "CDB/ATOM: %u leaves", nbAtomElements );
		CBitSet bitfield( nbAtomElements );
		f.readBits( bitfield );
		if ( ! bitfield.getVector().empty() )
		{
			if(verboseDatabase)
			{
				nldebug( "CDB/ATOM: Bitfield: %s LastBits:", bitfield.toString().c_str() );
				f.displayLastBits( bitfield.size() );
			}
		}

		// Set each modified property
		uint atomIndex;
		for ( uint i=0; i!=bitfield.size(); ++i )
		{
			if ( bitfield[i] )
			{
				if(verboseDatabase)
				{
					nldebug( "CDB/ATOM: Reading prop[%u] of atom", i );
				}

				atomIndex = i;
				CCDBNodeLeaf *leaf = findLeafAtCount( atomIndex );
				if ( leaf )
					leaf->readDelta( gc, f );
				else
					nlwarning( "CDB: Can't find leaf with index %u in atom branch %s", i, getParent()?getName()->c_str():"(root)" );
			}
		}
	}
	else
	{
		uint32 idx;

		f.serial(idx,_IdBits);

		if (idx >= _Nodes.size())
		{
			throw Exception ("idx %d > _Nodes.size() %d ", idx, _Nodes.size());
		}

		// Display the Name if we are in verbose mode
		if ( verboseDatabase )
		{
			string displayStr = string("Reading: ") +  *(_Nodes[idx]->getName());
			//CInterfaceManager::getInstance()->getChatOutput()->addTextChild( ucstring( displayStr ),CRGBA(255,255,255,255));
			nlinfo( "CDB: %s%s %u/%d", (!getParent())?"[root] ":"-", displayStr.c_str(), idx, _IdBits );
		}

		_Nodes[idx]->readDelta(gc, f);
	}
}// readDelta //



//-----------------------------------------------
//	clear
//
//-----------------------------------------------
// For old debug of random crash (let it in case it come back)
//static bool	AllowTestYoyoWarning= true;
void CCDBNodeBranch::clear()
{
	// TestYoyo. Track the random crash at exit
	/*if(AllowTestYoyoWarning)
	{
		std::string	name= getFullName();
		nlinfo("** clear: %s", name.c_str());
	}*/

	vector<ICDBNode *>::iterator itNode;
	for( itNode = _Nodes.begin(); itNode != _Nodes.end(); ++itNode )
	{
		(*itNode)->clear();
		// TestYoyo
		//AllowTestYoyoWarning= false;
		delete (*itNode);
		//AllowTestYoyoWarning= true;
	}
	_Nodes.clear();
	_NodesByName.clear();
	// must remove all branch observers, to avoid any problem in subsequent flushObserversCalls()
	removeAllBranchObserver();
} // clear //


/*
 * Find the leaf which count is specified (if found, the returned value is non-null and count is 0)
 */
CCDBNodeLeaf *CCDBNodeBranch::findLeafAtCount( uint& count )
{
	vector<ICDBNode *>::const_iterator itNode;
	for ( itNode = _Nodes.begin(); itNode != _Nodes.end(); ++itNode )
	{
		CCDBNodeLeaf *leaf = (*itNode)->findLeafAtCount( count );
		if ( leaf )
			return leaf;
	}
	return NULL;
}

/*
 * Count the leaves
 */
uint CCDBNodeBranch::countLeaves() const
{
	uint n = 0;
	vector<ICDBNode *>::const_iterator itNode;
	for ( itNode = _Nodes.begin(); itNode != _Nodes.end(); ++itNode )
	{
		n += (*itNode)->countLeaves();
	}
	return n;
}


void CCDBNodeBranch::display (const std::string &prefix)
{
	nlinfo("%sB %s", prefix.c_str(), _DBSM->localUnmap(_Name).c_str());
	string newPrefix = " " + prefix;
	vector<ICDBNode *>::const_iterator itNode;
	for ( itNode = _Nodes.begin(); itNode != _Nodes.end(); ++itNode )
	{
		(*itNode)->display(newPrefix);
	}
}

void CCDBNodeBranch::removeNode (const CTextId& id)
{
	// Look for the node
	CCDBNodeBranch *pNode = dynamic_cast<CCDBNodeBranch*>(getNode(id,false));
	if (pNode == NULL)
	{
		nlwarning("node %s not found", id.toString().c_str());
		return;
	}
	CCDBNodeBranch *pParent = pNode->_Parent;
	if (pParent== NULL)
	{
		nlwarning("parent node not found");
		return;
	}
	// search index node unsorted
	uint	indexNode;
	for (indexNode = 0; indexNode < pParent->_Nodes.size(); ++indexNode)
		if (pParent->_Nodes[indexNode] == pNode)
			break;
	if (indexNode == pParent->_Nodes.size())
	{
		nlwarning("node not found");
		return;
	}
	// search index node sorted
	uint	indexSorted;
	for (indexSorted = 0; indexSorted < pParent->_NodesByName.size(); ++indexSorted)
		if (pParent->_NodesByName[indexSorted] == pNode)
			break;
	if (indexSorted == pParent->_NodesByName.size())
	{
		nlwarning("node not found");
		return;
	}

	// Remove node from parent
	pParent->_Nodes.erase (pParent->_Nodes.begin()+indexNode);
	pParent->_NodesByName.erase (pParent->_NodesByName.begin()+indexSorted);
	pParent->_Sorted = false;

	// Delete the node
	pNode->clear();
	delete pNode;
}

void CCDBNodeBranch::onLeafChanged( NLMISC::TStringId leafName )
{
	for( TObserverHandleList::iterator itr = observerHandles.begin(); itr != observerHandles.end(); ++itr )
		if( (*itr)->observesLeaf( *leafName ) )
			(*itr)->addToFlushableList();

	if( _Parent != NULL )
		_Parent->onLeafChanged( leafName );
}


//-----------------------------------------------
//	addObserver
//
//-----------------------------------------------
bool CCDBNodeBranch::addObserver(IPropertyObserver* observer,CTextId& id)
{
	//test if this node is the desired one, if yes, add the observer to all the children nodes
	if ( id.getCurrentIndex() == id.size() )
	{
		for (uint i = 0; i < _Nodes.size(); ++i)
		{
			if (!_Nodes[i]->addObserver(observer,id))
				return false;
		}
		return true;
	}

	// lookup next element from textid in my index => idx
	const string &str = id.readNext();
	ICDBNode *pNode = find( str );
	// Property not found.
	if(pNode == NULL)
	{
		nlwarning(" Property %s not found", id.toString().c_str());
		return false;
	}

	// set property in child
	pNode->addObserver(observer,id);
	return true;

} // addObserver //

//-----------------------------------------------
//	removeObserver
//
//-----------------------------------------------
bool CCDBNodeBranch::removeObserver(IPropertyObserver* observer, CTextId& id)
{
	//test if this node is the desired one, if yes, remove the observer to all the children nodes
	if ( id.getCurrentIndex() == id.size() )
	{
		for (uint i = 0; i < _Nodes.size(); ++i)
		{
			if (!_Nodes[i]->removeObserver(observer, id))
				return false;
		}
		return true;
	}

	// lookup next element from textid in my index => idx
	const string &str = id.readNext();
	ICDBNode *pNode = find( str );
	// Property not found.
	if(pNode == NULL)
	{
		nlwarning(" Property %s not found", id.toString().c_str());
		return false;
	}

	// remove observer in child
	pNode->removeObserver(observer,id);
	return true;

} // removeObserver //



//-----------------------------------------------
void CCDBNodeBranch::addBranchObserver( ICDBDBBranchObserverHandle *handle, const std::vector<std::string>& positiveLeafNameFilter)
{
	CCDBNodeBranch::TObserverHandleList::iterator itr
		= std::find( observerHandles.begin(), observerHandles.end(), handle );

	if( itr != observerHandles.end() ){
		delete handle;
		return;
	}

	observerHandles.push_back( handle );
}

//-----------------------------------------------
void CCDBNodeBranch::addBranchObserver( ICDBDBBranchObserverHandle *handle, const char *dbPathFromThisNode, const char **positiveLeafNameFilter, uint positiveLeafNameFilterSize)
{
	CCDBNodeBranch *branchNode;
	if (dbPathFromThisNode[0] == '\0') // empty string
	{
		branchNode = this;
	}
	else
	{
		branchNode = safe_cast<CCDBNodeBranch*>(getNode(ICDBNode::CTextId(dbPathFromThisNode), false));
		if( branchNode == NULL ){
			std::string msg = *getName();
			msg += ":";
			msg += dbPathFromThisNode;
			msg += " branch missing in DB";

			nlerror( msg.c_str() );
			delete handle;
			return;
		}
	}
	std::vector<std::string> leavesToMonitor(positiveLeafNameFilterSize);
	for (uint i=0; i!=positiveLeafNameFilterSize; ++i)
	{
		leavesToMonitor[i] = string(positiveLeafNameFilter[i]);
	}
	branchNode->addBranchObserver(handle, leavesToMonitor);
}

//-----------------------------------------------
void CCDBNodeBranch::removeBranchObserver(const char *dbPathFromThisNode, ICDBNode::IPropertyObserver& observer)
{
	CCDBNodeBranch *branchNode = safe_cast<CCDBNodeBranch*>(getNode(ICDBNode::CTextId(dbPathFromThisNode), false));
	if( branchNode == NULL ){
		std::string msg = *getName();
		msg += ":";
		msg += dbPathFromThisNode;
		msg += " branch missing in DB";
		nlerror( msg.c_str() );
		return;
	}
	branchNode->removeBranchObserver(&observer);
}


//-----------------------------------------------
bool CCDBNodeBranch::removeBranchObserver(IPropertyObserver* observer)
{
	bool found = false;

	TObserverHandleList::iterator itr = observerHandles.begin();
	while( itr != observerHandles.end() )
	{
		if( (*itr)->observer() == observer )
		{
			(*itr)->removeFromFlushableList();
			delete *itr;
			itr = observerHandles.erase( itr );
			found = true;
		}
		else
		{
			++itr;
		}
	}

	return found;
}

//-----------------------------------------------
void CCDBNodeBranch::removeAllBranchObserver()
{
	for( TObserverHandleList::iterator itr = observerHandles.begin();
		itr != observerHandles.end(); ++itr ){
		(*itr)->removeFromFlushableList();
		delete *itr;
	}

	observerHandles.clear();
}

//-----------------------------------------------
// Useful for find
//-----------------------------------------------
class CCDBNodeBranchComp : public std::binary_function<ICDBNode *, ICDBNode *, bool>
{
public:
	bool operator()(const ICDBNode * x, const ICDBNode * y) const
	{
		return *(x->getName()) < *(y->getName());
	}
};

class CCDBNodeBranchComp2 : public std::binary_function<ICDBNode *, const string &, bool>
{
public:
	bool operator()(const ICDBNode * x, const string & y) const
	{
		return *(x->getName()) < y;
	}
};


//-----------------------------------------------
ICDBNode *CCDBNodeBranch::find(const std::string &nodeName)
{
#if NL_CDB_OPTIMIZE_PREDICT
	ICDBNode *predictNode = _PredictNode;
	if (predictNode)
	{
		if (predictNode->getParent() == this 
			&& *predictNode->getName() == nodeName)
		{
			return predictNode;
		}
	}
#endif

	if (!_Sorted)
	{
		_Sorted = true;
		sort(_NodesByName.begin(), _NodesByName.end(), CCDBNodeBranchComp());
	}

	CCDBNodeLeaf tmp(nodeName);
	vector<ICDBNode*>::iterator it = lower_bound(_NodesByName.begin(), _NodesByName.end(), &tmp, CCDBNodeBranchComp());
	if (it == _NodesByName.end())
		return NULL;
	else
	{
		if (*(*it)->getName() == nodeName)
		{
#if NL_CDB_OPTIMIZE_PREDICT
			ICDBNode *node = *it;
			_PredictNode = node;
			return node;
#else
			return *it;
#endif
		}
		else
			return NULL;
	}
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

}

