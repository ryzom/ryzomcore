// Condition.h: interface for the CCondition class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONDITION_H__160ED1FB_6106_4D5D_B298_0C66ED094B4B__INCLUDED_)
#define AFX_CONDITION_H__160ED1FB_6106_4D5D_B298_0C66ED094B4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

class CConditionNode;

namespace NLLOGIC
{
class CLogicConditionNode;
class CLogicCondition;
}

typedef CList< CConditionNode*, CConditionNode* > TPtrConditionList;


/**
* class CConditionNode
*/
class CConditionNode
{
public:	
	enum TConditionType
	{
		NOT =0,
		COMPARISON,
		SUB_CONDITION,

		TERMINATOR,
	};

	/// constructor
	inline CConditionNode( CConditionNode *parent = NULL) : m_pParentNode(parent)
	{
		m_type = TERMINATOR;
		m_dComparand = 0;
	}

	/// copy constructor, also copy all sub nodes (and allocate new nodes for them)
	CConditionNode( const CConditionNode &node);


	///destructor
	~CConditionNode();


	/// get the node as a string for display
	const CString &getNodeAsString() const;

	inline CConditionNode *getParentNode() { return m_pParentNode; }

	/// change all the occurences of the 'old' condition into 'newName' (recursive)
	void changeConditionName( const CString &old, const CString &newName);

	/// change all the occurences of the 'old' condition into 'newName' (recursive)
	void conditionDeleted( const CString &name);

	
// attributes
public:
	/// \name condition logic block
	//@{
	/// type of the block
	TConditionType	m_type;

	/// if sub condition block
	CString			m_sConditionName;
	
	/// if Comparison :
	CString			m_sVariableName;
	CString			m_sOperator; //in set '<' '>' '<=' >=' '=' '!='
	double			m_dComparand;
	//@}

	/// the sub conditions tree
	TPtrConditionList	m_ctSubTree;

	/// pointer on the 'parent' node, or NULL if this node is a first level node
	CConditionNode	*	m_pParentNode;

private:
	mutable CString		m_sNodeString;
};


/**
* class CCondition
*/
class CCondition  
{
public:
	/// constructor
	CCondition();

	/// copy constructor
	CCondition( const CCondition &cond);

	/// change all the occurences of the 'old' condition into 'newName' (for the nodes)
	void changeConditionName( CString old, const CString &newName) const;

	// the specified condition has been deleted, delete all references to it
	void conditionDeleted( CString name);

	virtual ~CCondition();


// attributes:
public:
	CString				m_sName;

	TPtrConditionList	m_ctConditionTree;
};



/**
 *	cConditionToCLogicCondition
 */
void cConditionToCLogicCondition( CCondition& condition, NLLOGIC::CLogicCondition& logicCondition );

/**
 *	cLogicConditionToCCondition
 */
void cLogicConditionToCCondition( const NLLOGIC::CLogicCondition& logicCondition, CCondition& condition );


#endif // !defined(AFX_CONDITION_H__160ED1FB_6106_4D5D_B298_0C66ED094B4B__INCLUDED_)
