// Condition.cpp: implementation of the CCondition class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "logic_editor.h"
#include "condition.h"
#include "nel/logic/logic_condition.h"

#include <vector>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLLOGIC;

//////////////////////////////////////////////////////////////////////
// CConditionNode implementation
//////////////////////////////////////////////////////////////////////

CConditionNode::CConditionNode(const CConditionNode &node)
{
	this->m_dComparand = node.m_dComparand;
	this->m_pParentNode = node.m_pParentNode;
	this->m_sConditionName = node.m_sConditionName;
	this->m_sOperator = node.m_sOperator;
	this->m_sVariableName = node.m_sVariableName;
	this->m_type = node.m_type;

	// copy sub condition node tree
	CConditionNode *pNode, *newNode;
	POSITION pos = node.m_ctSubTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctSubTree.GetNext( pos );

		if (pNode != NULL)
		{
			newNode = new CConditionNode( *pNode );
			this->m_ctSubTree.AddTail( newNode );
			newNode->m_pParentNode = this;
		}
	}

}



CConditionNode::~CConditionNode()
{
	CConditionNode *pNode = NULL;

	// delete all sub nodes
	POSITION pos = m_ctSubTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctSubTree.GetNext( pos );

		if (pNode != NULL)
		{
			delete pNode;
			pNode = NULL;
		}
	}
}

const CString & CConditionNode::getNodeAsString() const
{
	m_sNodeString.Empty();

	if (m_type == NOT)
		m_sNodeString = "NOT";
	else if (m_type == TERMINATOR)
		m_sNodeString = "term";
	else if (m_type == SUB_CONDITION)
		m_sNodeString = m_sConditionName;
	else // comparison
	{
		m_sNodeString.Format(_T("%s %s %g"),LPCTSTR(m_sVariableName),LPCTSTR(m_sOperator), m_dComparand );
	}

	return m_sNodeString;
}



void CConditionNode::changeConditionName( const CString &old, const CString &newName)
{
	CConditionNode *pNode = NULL;

	POSITION pos = m_ctSubTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctSubTree.GetNext( pos );

		if (pNode != NULL)
		{
			pNode->changeConditionName(old, newName);
		}
	}

	//
	if ( m_sConditionName == old)
		m_sConditionName = newName;
}

void CConditionNode::conditionDeleted( const CString &name)
{
	CConditionNode *pNode = NULL;

	POSITION oldpos;
	POSITION pos = m_ctSubTree.GetHeadPosition();
	while (pos != NULL)
	{
		oldpos = pos;
		pNode = m_ctSubTree.GetNext( pos );

		if (pNode != NULL)
		{
			if ( pNode->m_sConditionName != name)
				pNode->conditionDeleted(name);
			else
			{
				this->m_ctSubTree.RemoveAt( oldpos );
				delete pNode;
				pNode = NULL;
			}
		}
	}
}





//////////////////////////////////////////////////////////////////////
// CCondition implementation
//////////////////////////////////////////////////////////////////////
CCondition::CCondition()
{
}


CCondition::CCondition( const CCondition &cond)
{
	this->m_sName = cond.m_sName;

	// copy sub condition node tree
	CConditionNode *pNode, *newNode;
	POSITION pos = cond.m_ctConditionTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctConditionTree.GetNext( pos );

		if (pNode != NULL)
		{
			newNode = new CConditionNode( *pNode );
			this->m_ctConditionTree.AddTail( newNode );
		}
	}	

}


CCondition::~CCondition()
{
	CConditionNode *pNode = NULL;

	// delete all sub nodes
	POSITION pos = m_ctConditionTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctConditionTree.GetNext( pos );

		if (pNode != NULL)
		{
			delete pNode;
			pNode = NULL;
		}
	}	
}



void CCondition::changeConditionName( CString old, const CString &newName) const
{
	CConditionNode *pNode = NULL;
	
	POSITION pos = m_ctConditionTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctConditionTree.GetNext( pos );

		if (pNode != NULL)
		{
			pNode->changeConditionName( old, newName);
		}
	}	
}


void CCondition::conditionDeleted( CString name)
{
	CConditionNode *pNode = NULL;

	POSITION oldpos;
	POSITION pos = m_ctConditionTree.GetHeadPosition();
	while (pos != NULL)
	{
		oldpos = pos;
		pNode = m_ctConditionTree.GetNext( pos );

		if (pNode != NULL)
		{
			if ( pNode->m_sConditionName != name)
				pNode->conditionDeleted(name);
			else
			{
				this->m_ctConditionTree.RemoveAt( oldpos );
				delete pNode;
				pNode = NULL;
			}
		}
	}
}



//-----------------------------------------------------
//	cConditionNodeToCLogicConditionNode (Editor --> Service)
//
//-----------------------------------------------------
void cConditionNodeToCLogicConditionNode(CConditionNode * conditionNode, CLogicConditionNode * logicConditionNode )
{
	// if this node is a terminator node
	if( conditionNode->m_type == CConditionNode::TERMINATOR )
	{
		logicConditionNode->Type = CLogicConditionNode::TERMINATOR; 		
	}
	else
	// this node is a logic node
	{
		logicConditionNode->Type = CLogicConditionNode::LOGIC_NODE;
				
		// part 1 : a logic block(not/comparison/subcondition)
		switch( conditionNode->m_type )
		{
			case CConditionNode::NOT :
			{
				logicConditionNode->LogicBlock.Type = CLogicConditionLogicBlock::NOT;
				
			}
			break;

			case CConditionNode::COMPARISON :
			{
				logicConditionNode->LogicBlock.Type = CLogicConditionLogicBlock::COMPARISON;
				
				logicConditionNode->LogicBlock.ComparisonBlock.VariableName = NLMISC::tStrToUtf8(conditionNode->m_sVariableName);
			    logicConditionNode->LogicBlock.ComparisonBlock.Operator = NLMISC::tStrToUtf8(conditionNode->m_sOperator);
				logicConditionNode->LogicBlock.ComparisonBlock.Comparand = (sint64)conditionNode->m_dComparand;
			}
			break;
			
			case CConditionNode::SUB_CONDITION :
			{
				logicConditionNode->LogicBlock.Type = CLogicConditionLogicBlock::SUB_CONDITION;
				
				logicConditionNode->LogicBlock.SubCondition = NLMISC::tStrToUtf8(conditionNode->m_sConditionName);
			}
			break;
		}

		// part 2 : a condition sub tree
		POSITION pos;
		for( pos = conditionNode->m_ctSubTree.GetHeadPosition(); pos != NULL; )
		{
			CConditionNode * pConditionNode = conditionNode->m_ctSubTree.GetNext( pos );
			CLogicConditionNode * logicConditionNodeTmp = new CLogicConditionNode();
			cConditionNodeToCLogicConditionNode( pConditionNode, logicConditionNodeTmp );
			logicConditionNode->addNode( logicConditionNodeTmp );
		}
	}

} // cConditionNodeToCLogicConditionNode //




//-----------------------------------------------------
//	cConditionToCLogicCondition (Editor --> Service)
//
//-----------------------------------------------------
void cConditionToCLogicCondition( CCondition& condition, CLogicCondition& logicCondition )
{
	// condition name
	logicCondition.setName(NLMISC::tStrToUtf8(condition.m_sName));

	// nodes
	POSITION pos;
	for( pos = condition.m_ctConditionTree.GetHeadPosition(); pos != NULL; )
	{
		// get the node
		CConditionNode * pConditionNode = condition.m_ctConditionTree.GetNext( pos );
		
		// convert the node
		CLogicConditionNode * logicConditionNode = new CLogicConditionNode();
		cConditionNodeToCLogicConditionNode( pConditionNode, logicConditionNode );

		// add the node
		logicCondition.addNode( *logicConditionNode );
	}

} // cConditionToCLogicCondition //






//-----------------------------------------------------
//	cLogicConditionNodeToCConditionNode (Service --> Editor)
//
//-----------------------------------------------------
void cLogicConditionNodeToCConditionNode( const CLogicConditionNode * logicConditionNode, CConditionNode * node )
{
	// terminator node
	if(logicConditionNode->Type == CLogicConditionNode::TERMINATOR)
	{
		node->m_type = CConditionNode::TERMINATOR;
	}
	// logic block with condition sub tree
	else
	{
		// part 1 : a logic block(not/comparison/subcondition)
		switch( logicConditionNode->LogicBlock.Type )
		{
			case CLogicConditionLogicBlock::NOT :
			{
				node->m_type = CConditionNode::NOT;
			};
			break;

			case CLogicConditionLogicBlock::COMPARISON :
			{
				node->m_type = CConditionNode::COMPARISON;
				
				node->m_sVariableName = CString(logicConditionNode->LogicBlock.ComparisonBlock.VariableName.c_str());
				node->m_sOperator = CString(logicConditionNode->LogicBlock.ComparisonBlock.Operator.c_str());
				node->m_dComparand = (double)logicConditionNode->LogicBlock.ComparisonBlock.Comparand;

			};
			break;

			case CLogicConditionLogicBlock::SUB_CONDITION :
			{
				node->m_type = CConditionNode::SUB_CONDITION;
				node->m_sConditionName = CString(logicConditionNode->LogicBlock.SubCondition.c_str());
			};
			break;

			default :
			{
				node->m_type = CConditionNode::TERMINATOR;
			}

		}
		
		// part 2 : a condition sub tree
		vector<CLogicConditionNode *>::const_iterator itNode;
		for( itNode = logicConditionNode->_Nodes.begin(); itNode != logicConditionNode->_Nodes.end(); ++itNode )
		{
			CConditionNode * nodeTmp = new CConditionNode();
			cLogicConditionNodeToCConditionNode( *itNode, nodeTmp );
			nodeTmp->m_pParentNode = node;
			node->m_ctSubTree.AddTail( nodeTmp );
		}
		
	}

} // cLogicConditionNodeToCConditionNode //




//-----------------------------------------------
//	cLogicConditionToCCondition 
//
//-----------------------------------------------
void cLogicConditionToCCondition( const CLogicCondition& logicCondition, CCondition& condition )
{
	// condition name
	condition.m_sName = CString( logicCondition.getName().c_str() );

	// condition tree
	vector<CLogicConditionNode>::const_iterator itNode;
	for( itNode = logicCondition.Nodes.begin(); itNode != logicCondition.Nodes.end(); ++itNode )
	{
		// convert the node
		CConditionNode * node = new CConditionNode();
		cLogicConditionNodeToCConditionNode( &(*itNode), node );
	
		// add the node
		condition.m_ctConditionTree.AddTail( node );
	}

} // cLogicConditionToCCondition //


