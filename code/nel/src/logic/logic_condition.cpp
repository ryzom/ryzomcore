// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#include "stdlogic.h"
#include "nel/logic/logic_condition.h"

#include "nel/logic/logic_variable.h"
#include "nel/logic/logic_state_machine.h"

#include "nel/misc/o_xml.h"

using namespace NLMISC;
using namespace std;

namespace NLLOGIC
{

//-------------------------------------------------
// setLogicStateMachine :
//
//-------------------------------------------------
void CLogicComparisonBlock::setLogicStateMachine( CLogicStateMachine * logicStateMachine )
{
	if( logicStateMachine == 0 )
	{
		nlwarning("(LOGIC)<CLogicComparisonBlock::setLogicStateMachine> The state machine is null");
	}
	else
	{
		_LogicStateMachine = logicStateMachine;
	}

} // setLogicStateMachine //


//-------------------------------------------------
// testLogic :
//
//-------------------------------------------------
bool CLogicComparisonBlock::testLogic()
{
	CLogicVariable var;
	if( _LogicStateMachine->getVariable( VariableName, var ) == false )
	{
		nlwarning("(LOGIC)<CLogicComparisonBlock::testLogic> The variable %s is unknown in the state machine",VariableName.c_str());
		return false;
	}

	if( Operator == "<"  )	return ( var.getValue() <  Comparand );
	if( Operator == "<=" )	return ( var.getValue() <= Comparand );
	if( Operator == ">"  )	return ( var.getValue() >  Comparand );
	if( Operator == ">=" )	return ( var.getValue() >= Comparand );
	if( Operator == "==" )	return ( var.getValue() == Comparand );
	if( Operator == "!=" )	return ( var.getValue() != Comparand );

	nlwarning("(LOGIC)<CLogicComparisonBlock::testLogic> The comparison block operator %s is unknown",Operator.c_str());
	return false;

} // testLogic //


//-------------------------------------------------
// serial :
//
//-------------------------------------------------
/*void CLogicComparisonBlock::serial( IStream &f )
{
	f.xmlPush("COMPARISON_BLOCK");

	f.serial( VariableName );
	f.serial( Operator );
	f.serial( Comparand );

	f.xmlPop();

} // serial //*/

void CLogicComparisonBlock::write (xmlNodePtr node) const
{
	xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)"COMPARISON_BLOCK", NULL);
	xmlSetProp (elmPtr, (const xmlChar*)"VariableName", (const xmlChar*)VariableName.c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Operator", (const xmlChar*)Operator.c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Comparand", (const xmlChar*)toString(Comparand).c_str());
}

void CLogicComparisonBlock::read (xmlNodePtr node)
{
	xmlCheckNodeName (node, "COMPARISON_BLOCK");

	VariableName = getXMLProp (node, "VariableName");
	Operator = getXMLProp (node, "Operator");
	Comparand = atoiInt64(getXMLProp (node, "Comparand").c_str());
}







//-------------------------------------------------
// setLogicStateMachine :
//
//-------------------------------------------------
void CLogicConditionLogicBlock::setLogicStateMachine( CLogicStateMachine * logicStateMachine )
{
	if( logicStateMachine == 0 )
	{
		nlwarning("(LOGIC)<CCLogicConditionLogicBlock::setLogicStateMachine> The state machine is null");
	}
	else
	{
		// set the state machine of this node
		_LogicStateMachine = logicStateMachine;

		// set the state machine of the logic block
		ComparisonBlock.setLogicStateMachine( logicStateMachine );
	}

} // setLogicStateMachine //


//-------------------------------------------------
// testLogic :
//
//-------------------------------------------------
bool CLogicConditionLogicBlock::testLogic()
{
	switch( Type )
	{
		case NOT :
		{
			return true;
		}
		break;

		case COMPARISON :
		{
			return ComparisonBlock.testLogic();
		}
		break;

		case SUB_CONDITION :
		{
			CLogicCondition condition;
			if( _LogicStateMachine->getCondition(SubCondition,condition) )
			{
				return condition.testLogic();
			}
			else
			{
				nlwarning("(LOGIC)<CLogicConditionLogicBlock::testLogic> The subcondition \"%s\" is unknown in the state machine \"%s\"",
					SubCondition.c_str(),_LogicStateMachine->getName().c_str());
			}

		}

		default :
			nlerror("(LOGIC)<CLogicConditionLogicBlock::testLogic> logic block type %d is unknown",Type);
	}

	return false;

} // testLogic //



//-------------------------------------------------
// fillVarSet :
//
//-------------------------------------------------
void CLogicConditionLogicBlock::fillVarSet( set<string>& condVars )
{
	if( Type == COMPARISON )
	{
		condVars.insert( ComparisonBlock.VariableName );
	}
	else
	{
		if( Type == SUB_CONDITION )
		{
			CLogicCondition condition;
			if( _LogicStateMachine->getCondition(SubCondition,condition) )
			{
				condition.fillVarSet( condVars );
			}
		}
	}

} // fillVarSet //


//-------------------------------------------------
// serial :
//
//-------------------------------------------------
/*void CLogicConditionLogicBlock::serial( IStream &f )
{
	f.xmlPush("CONDITION_LOGIC_BLOCK");

	f.serialEnum( Type );
	switch( Type )
	{
		case NOT : break;

		case COMPARISON :
		{
			f.serial( ComparisonBlock );
		}
		break;

		case SUB_CONDITION :
		{
			f.serial( SubCondition );
		}
		break;
	};

	f.xmlPop();
};*/

void CLogicConditionLogicBlock::write (xmlNodePtr node) const
{
	xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)"CONDITION_LOGIC_NODE", NULL);
	xmlSetProp (elmPtr, (const xmlChar*)"Type", (const xmlChar*)toString((uint32)Type).c_str());
	switch( Type )
	{
		case NOT : break;

		case COMPARISON :
		{
			ComparisonBlock.write(elmPtr);
		}
		break;

		case SUB_CONDITION :
		{
			xmlSetProp (elmPtr, (const xmlChar*)"SubCondition", (const xmlChar*)SubCondition.c_str());
		}
		break;
	};
}

void CLogicConditionLogicBlock::read (xmlNodePtr node)
{
	xmlCheckNodeName (node, "CONDITION_LOGIC_NODE");
	uint32 uType;
	NLMISC::fromString(getXMLProp(node, "Type"), uType);
	Type = (TLogicConditionLogicBlockType)uType;
	switch( Type )
	{
		case NOT : break;

		case COMPARISON :
		{
			ComparisonBlock.read (node);
		}
		break;

		case SUB_CONDITION :
		{
			SubCondition = getXMLProp (node, "SubCondition");
		}
		break;
	};
}

//-----------------------------------------



//-------------------------------------------------
// setLogicStateMachine :
//
//-------------------------------------------------
void CLogicConditionNode::setLogicStateMachine( CLogicStateMachine * logicStateMachine )
{
	if( logicStateMachine == 0 )
	{
		nlwarning("(LOGIC)<CLogicConditionNode::setLogicStateMachine> The state machine is null");
	}
	else
	{
		// set the state machine of this node
		_LogicStateMachine = logicStateMachine;

		// set the state machine of the logic block
		LogicBlock.setLogicStateMachine( logicStateMachine );

		// set the state machine for the sub nodes
		vector<CLogicConditionNode *>::iterator itNodes;
		for( itNodes = _Nodes.begin(); itNodes != _Nodes.end(); ++itNodes )
		{
			(*itNodes)->setLogicStateMachine( logicStateMachine );
		}
	}

} // setLogicStateMachine //


//-------------------------------------------------
// addNode :
//
//-------------------------------------------------
void CLogicConditionNode::addNode( CLogicConditionNode * node )
{
	node->setLogicStateMachine( _LogicStateMachine );
	_Nodes.push_back( node );

} // addToSubNodes //


//-------------------------------------------------
// testLogic :
//
//-------------------------------------------------
bool CLogicConditionNode::testLogic()
{
	// test the logic block
	if( LogicBlock.testLogic() == false )
	{
		return false;
	}

	// if there's no subtree we assess the subtree is true
	if( _Nodes.empty() )
	{
		return true;
	}

	// test the subtree
	if( LogicBlock.isNotBlock() )
	{
		// the subtree is false if at least one node is true
		vector<CLogicConditionNode *>::iterator itNodes;
		for( itNodes = _Nodes.begin(); itNodes != _Nodes.end(); ++itNodes )
		{
			if( (*itNodes)->testLogic() == true )
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		// the subtree is true if at least one node is true
		vector<CLogicConditionNode *>::iterator itNodes;
		for( itNodes = _Nodes.begin(); itNodes != _Nodes.end(); ++itNodes )
		{
			if( (*itNodes)->testLogic() == true )
			{
				return true;
			}
		}

		return false;
	}

} // testLogic //


//-------------------------------------------------
// fillVarSet :
//
//-------------------------------------------------
void CLogicConditionNode::fillVarSet( set<string>& condVars )
{
	if( Type == LOGIC_NODE )
	{
		LogicBlock.fillVarSet( condVars );
	}

	vector<CLogicConditionNode *>::iterator itNode;
	for( itNode = _Nodes.begin(); itNode != _Nodes.end(); ++itNode )
	{
		(*itNode)->fillVarSet( condVars );
	}

} // fillVarSet //


//-------------------------------------------------
// serial :
//
//-------------------------------------------------
/*void CLogicConditionNode::serial( IStream &f )
{
	f.xmlPush("CONDITION_NODE");

	f.serialEnum( Type );
	switch( Type )
	{
		case TERMINATOR : break;
		case LOGIC_NODE :
		{
			f.serial( LogicBlock );
			if( f.isReading() )
			{
				uint32 sz;
				f.serial( sz );
				uint i;
				for( i = 0; i < sz; i++ )
				{
					CLogicConditionNode * node = new CLogicConditionNode();
					f.serial( *node );
					_Nodes.push_back( node );
				}
			}
			else
			{
				uint32 sz = _Nodes.size();
				f.serial( sz );
				vector<CLogicConditionNode *>::iterator itNode;
				for( itNode = _Nodes.begin(); itNode != _Nodes.end(); ++itNode )
				{
					f.serial( **itNode );
				}
			}
		}
		break;
	};

	f.xmlPop();

} // serial //*/

void CLogicConditionNode::write (xmlNodePtr node) const
{
	xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)"CONDITION_NODE", NULL);
	xmlSetProp (elmPtr, (const xmlChar*)"Type", (const xmlChar*)toString((uint32)Type).c_str());

	switch( Type )
	{
		case TERMINATOR : break;
		case LOGIC_NODE :
		{
			LogicBlock.write(elmPtr);
			vector<CLogicConditionNode *>::const_iterator itNode = _Nodes.begin();
			for( ; itNode != _Nodes.end(); ++itNode )
			{
				(*itNode)->write(elmPtr);
			}
		}
		break;
	};
}

void CLogicConditionNode::read (xmlNodePtr node)
{
	xmlCheckNodeName (node, "CONDITION_NODE");
	uint32 uType;
	NLMISC::fromString(getXMLProp(node, "Type"), uType);
	Type = (TConditionNodeType )uType;
	switch( Type )
	{
		case TERMINATOR : break;
		case LOGIC_NODE :
		{
			LogicBlock.read (node);

			{
				// Count the parent
				uint nb = CIXml::countChildren (node, "CONDITION_NODE");
				uint i = 0;
				xmlNodePtr parent = CIXml::getFirstChildNode (node, "CONDITION_NODE");
				while (i<nb)
				{
					CLogicConditionNode *v = new CLogicConditionNode();
					v->read(parent);
					_Nodes.push_back (v);

					// Next parent
					parent = CIXml::getNextChildNode (parent, "CONDITION_NODE");
					i++;
				}
			}
		}
		break;
	};
}

//-------------------------------------------------
// ~CLogicConditionNode :
//
//-------------------------------------------------
CLogicConditionNode::~CLogicConditionNode()
{
	vector<CLogicConditionNode *>::iterator itNodes;
	for( itNodes = _Nodes.begin(); itNodes != _Nodes.end(); ++itNodes )
	{
		delete (*itNodes);
	}

} // ~CLogicConditionNode //







//-------------------------------------------------
// setLogicStateMachine :
//
//-------------------------------------------------
void CLogicCondition::setLogicStateMachine( CLogicStateMachine * logicStateMachine )
{
	if( logicStateMachine == 0 )
	{
		nlwarning("(LOGIC)<CLogicCondition::setLogicStateMachine> The state machine is null");
	}
	else
	{
		// init the logic state machine for each node
		vector<CLogicConditionNode>::iterator itNodes;
		for( itNodes = Nodes.begin(); itNodes != Nodes.end(); ++itNodes )
		{
			(*itNodes).setLogicStateMachine( logicStateMachine );
		}
	}

} // setLogicStateMachine //


//-------------------------------------------------
// testLogic :
//
//-------------------------------------------------
bool CLogicCondition::testLogic()
{
	vector<CLogicConditionNode>::iterator itNodes;
	for( itNodes = Nodes.begin(); itNodes != Nodes.end(); ++itNodes )
	{
		if( (*itNodes).testLogic() == false )
		{
			return false;
		}
	}

	return true;

} // testLogic //



//-------------------------------------------------
// fillVarSet :
//
//-------------------------------------------------
void CLogicCondition::fillVarSet( set<string>& condVars )
{
	vector<CLogicConditionNode>::iterator itNode;
	for( itNode = Nodes.begin(); itNode != Nodes.end(); ++itNode )
	{
		(*itNode).fillVarSet( condVars );
	}

} // fillVarSet //



//-------------------------------------------------
// serial :
//
//-------------------------------------------------
/*void CLogicCondition::serial( IStream &f )
{
	f.xmlPush("CONDITION");

	f.serial( _ConditionName );
	f.serialCont( Nodes );

	f.xmlPop();

} // serial //*/

void CLogicCondition::write (xmlNodePtr node) const
{
	xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)"CONDITION", NULL);
	xmlSetProp (elmPtr, (const xmlChar*)"Name", (const xmlChar*)_ConditionName.c_str());

	uint i;
	for (i = 0; i < Nodes.size(); i++)
	{
		Nodes[i].write(elmPtr);
	}
}

void CLogicCondition::read (xmlNodePtr node)
{
	xmlCheckNodeName (node, "CONDITION");

	_ConditionName = getXMLProp (node, "Name");

	{
		// Count the parent
		uint nb = CIXml::countChildren (node, "CONDITION_NODE");
		uint i = 0;
		xmlNodePtr parent = CIXml::getFirstChildNode (node, "CONDITION_NODE");
		while (i<nb)
		{
			CLogicConditionNode v;
			v.read(parent);
			Nodes.push_back (v);

			// Next parent
			parent = CIXml::getNextChildNode (parent, "CONDITION_NODE");
			i++;
		}
	}
}


} // NLLOGIC
