// logic_editorDoc.h : interface of the CLogic_editorDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGIC_EDITORDOC_H__7E37E572_7B2F_4A62_BDC2_F1F4751381AD__INCLUDED_)
#define AFX_LOGIC_EDITORDOC_H__7E37E572_7B2F_4A62_BDC2_F1F4751381AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Counter.h"
#include "Condition.h"
#include "State.h"
#include "ChildFrm.h"


namespace NLLOGIC
{
class CLogicStateMachine;
}

class CLogic_editorDoc : public CDocument
{
protected: // create from serialization only
	CLogic_editorDoc();
	DECLARE_DYNCREATE(CLogic_editorDoc)

// Attributes
public:
	
	/// the variables
	CStringList			m_variables;

	/// the counters, map counter name to counter object
	CMapStringToPtr		m_counters;

	/// the states, map name to object
	CMapStringToPtr		m_states;

	/// the conditions, map name to object
	CMapStringToPtr		m_conditions;

	BOOL InitCounterPage;
	BOOL InitConditionPage;
	BOOL InitStatePage;


// Operations
public:

	/**
	 * change a variable name (all it's occurence)
	 * \param CString old the old name of the var (not a reference because we may delete the string when deleting the variable !!)
	 * \param CString &newName the new name of the var
	 * \return BOOL TRUE if the change has been made, FALSE if an error occurred
	 */
	BOOL changeVarName( CString old, const CString &newName);

	/**
	 * Delete a variable
	 * \param CString name the name of the variable to delete
	 */
	void deleteVar( CString name);

	/**
	 * change a counter name (all it's occurence)
	 * \param CString old the old name of the counter
	 * \param CString &newName the new name of the counter
	 * \return BOOL TRUE if the change has been made, FALSE if an error occurred
	 */
	BOOL changeCounterName( CString old, const CString &newName);

	/**
	 * Delete a counter
	 * \param CString name the name of the counter to delete
	 */
	void deleteCounter( CString name);


	/**
	 * change a condition name (all it's occurence)
	 * \param CString old the old name of the condition
	 * \param CString &newName the new name of the condition
	 * \return BOOL TRUE if the change has been made, FALSE if an error occurred
	 */
	BOOL changeConditionName( CString old, const CString &newName);

	/**
	 * Delete a condition
	 * \param CString &name the name of the condition to delete
	 */
	void deleteCondition( CString name);

	
	/**
	 * change a state name (all it's occurence)
	 * \param CString old the old name of the state
	 * \param CString &newName the new name of the state
	 * \return BOOL TRUE if the change has been made, FALSE if an error occurred
	 */
	BOOL changeStateName( CString old, const CString &newName);

	/**
	 * Delete a state
	 * \param CString name the name of the state to delete
	 */
	void deleteState( CString name);

	/**
	 *	Load
	 */
	BOOL load( LPCTSTR fileName );

	/**
	 *	Create a new doc instance
	 */
	static CLogic_editorDoc * getNewDoc() { return new CLogic_editorDoc(); }
	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogic_editorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLogic_editorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CLogic_editorDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGIC_EDITORDOC_H__7E37E572_7B2F_4A62_BDC2_F1F4751381AD__INCLUDED_)
