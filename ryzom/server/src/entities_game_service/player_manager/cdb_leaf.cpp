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


#define TRACE_READ_DELTA
//#define TRACE_WRITE_DELTA
//#define TRACE_SET_VALUE



//////////////
// Includes //
//////////////
#include "player_manager/cdb_leaf.h"
#include "nel/misc/xml_auto_ptr.h"
#include <libxml/parser.h>


////////////////
// Namespaces //
////////////////
using namespace NLMISC;
using namespace std;


//-----------------------------------------------
//	init
//-----------------------------------------------
void CCDBStructNodeLeaf::init( xmlNodePtr node, NLMISC::IProgressCallback &progressCallBack ) 
{
	CXMLAutoPtr type((const char*)xmlGetProp (node, (xmlChar*)"type"));
	nlassert((const char *) type != NULL);

	// IF type is an INT with n bits [1,64].
	if ((type.getDatas()[0] == 'I') || (type.getDatas()[0] == 'U'))
	{
		uint nbBit;
		NLMISC::fromString(type.getDatas() + 1, nbBit);
		if(nbBit>=1 && nbBit<=64)
			_Type=(ICDBStructNode::EPropType)nbBit;
		else
		{
			nlwarning("CCDBNodeLeaf::init : property is an INT and should be between [1,64] but it is %d bit(s).", nbBit);
			_Type = ICDBStructNode::UNKNOWN;
		}
	}
	else if (type.getDatas()[0] == 'S')
	{
		uint nbBit;
		NLMISC::fromString(type.getDatas() + 1, nbBit);
		if(nbBit>=1 && nbBit<=64)
			_Type = (ICDBStructNode::EPropType)nbBit; // all is I on the server (unlike the client)
		else
		{
			nlwarning("CCDBNodeLeaf::init : property is an SINT and should be between [1,64] but it is %d bit(s).", nbBit);
			_Type = ICDBStructNode::UNKNOWN;
		}
	}
	// ELSE
	else
	{
		// IF it is a TEXT.
		if(!strcmp(type, "TEXT"))
			_Type = ICDBStructNode::TEXT;
		// ELSE type unknown.
		else
		{
			nlwarning("CCDBNodeLeaf::init : type '%s' is unknown.", type.getDatas());
			_Type = ICDBStructNode::UNKNOWN;
		}
	}

} // init //



//-----------------------------------------------
//	getNode
//
//-----------------------------------------------
ICDBStructNode * CCDBStructNodeLeaf::getNode( std::vector<uint16>& ids, uint idx )
{
	// assert that there are no indexes left in the index list
	nlassert( idx==ids.size());
	return this;
} // getNode //



//-----------------------------------------------
//	getNode
//
//-----------------------------------------------
ICDBStructNode * CCDBStructNodeLeaf::getNode( uint16 idx )
{
	return this;
} // getNode //


//-----------------------------------------------
//	getNode
//
//-----------------------------------------------
ICDBStructNode * CCDBStructNodeLeaf::getNode( const CTextId& id, bool bCreate )
{
	return this;
} // getNode //


/*
 * Move the siblings that match the bank name to the destination tree
 *
 * Precondition: the tree was constructed and labelled backward until the root.
 */
void	CCDBStructNodeLeaf::moveBranchesToBank( CCDBStructNodeBranch *destRoot, TCDBBank bank )
{
	if ( (!_BankLabels) || _BankLabels->empty() )
	{
		nlwarning( "CDB: moveBranchesToBank() should not come to a leaf (%s) with no label", getParent()?getName()->c_str():"root" );
	}
	else if ( _BankLabels && (_BankLabels->find(bank) != _BankLabels->end()) )
	{
		// Test if the branch has several different labels
		if ( _BankLabels->size() > 1 )
		{
			nlwarning( "CDB: moveBranchesToBank: a leaf should not have several labels" );
		}
		else // only one label (and matching)
		{
			// Move the entire branch to the bank (deleting the label)
			delete _BankLabels;
			_BankLabels = NULL;
			if ( getParent() )
			{
				string name = *getName();
				nldebug( "CDB: Moving leaf %s to bank %s", name.c_str(), CCDBStructBanks::getBankName(bank) );
				getParent()->detachChild( this );
				destRoot->attachChild( this, name );
			}
		}
	}
}


/*
 * Build a textid corresponding to the leaf
 */
ICDBStructNode::CTextId			CCDBStructNodeLeaf::buildTextId() const
{
	const CCDBStructNodeBranch *node = getParent();
	if ( ! node )
		return CTextId();

	// Get the root
	while ( (node->getParent()) != NULL )
		node = node->getParent();

	// Browse the tree, using the leaf bin id, and build the text id
	CTextId textId;
	string binStr;
	std::vector< std::pair <uint32, int> >::const_iterator ibi;
	for ( ibi=_LeafId.Ids.begin(); ibi!=_LeafId.Ids.end(); ++ibi )
	{
		binStr += toString( " %u/%d", (*ibi).first, (*ibi).second );
		const string *name = node->getNodeName( (uint16)((*ibi).first) );
		if ( ! name )
		{
			textId.push( toString( "<idx %u not found>", (*ibi).first ) );
			return textId;
		}
		textId.push( *name );
		if ( ! (node = dynamic_cast<const CCDBStructNodeBranch*>(node->getNode( (uint16)((*ibi).first) )) ) )
		{
			++ibi;
			while ( ibi<_LeafId.Ids.end() )
			{
				binStr += toString( " -%u/%d", (*ibi).first, (*ibi).second );
				++ibi;
			}
			textId.push( binStr );
			break;
		}
		
	}
	return textId;
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
//#############################################################################################

