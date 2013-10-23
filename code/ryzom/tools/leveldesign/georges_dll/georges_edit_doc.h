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

#if !defined(AFX_GEORGES_EDITDOC_H__266E6547_220E_4A6E_9285_5F91BCD53E5B__INCLUDED_)
#define AFX_GEORGES_EDITDOC_H__266E6547_220E_4A6E_9285_5F91BCD53E5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "nel/misc/types_nl.h"

#include "nel/georges/type.h"
#include "nel/georges/form.h"
#include "nel/georges/form_dfn.h"
#include "nel/georges/form_loader.h"

#include "plugin_interface.h"

class CLeftView;
class CGeorgesEditView;
class CGeorgesEditDoc;

/**
  * This class is the sub object in the leftview tree.
  *
  * The CGeorgesEditDocSub pointers are stored in the CTreeCtrl user data. The current CGeorgesEditDocSub edited in the right view
  * is the current ledfview CTreeCtrl selection.
  *
  * To get the node or the DFN entry from a CGeorgesEditDocSub, use the form name stored in the CGeorgesEditDocSub and make a
  * getNodeByName / getValueByName.
  */
class CGeorgesEditDocSub
{
public:
	// What is the sub object ?
	enum TSub
	{
		Null,		// Nothing in this node (root ?)
		Header,		// Header node
		Type,		// This node is a type
		Dfn,		// This node is a dfn
		Form,		// This node is a form
	};

	CGeorgesEditDocSub ();

	// Delete sub objects
	~CGeorgesEditDocSub ();

	// Get the type of the sub object
	bool	isEditable () const;

	// Is Subobject editable
	TSub	getType () const;

	// Get the my id in the parent children array
	uint	getIdInParent () const;

	// Get the sub object name
	const std::string& getName () const;

	// Get the form name
	const std::string& getFormName () const;

	// Get the sub object children
	uint		getChildrenCount ();
	void		removeChildren (uint child);
	CGeorgesEditDocSub	*getChild (uint child);
	CGeorgesEditDocSub	*getParent ();

	// Get the slot number
	uint	getSlot () const;

	// Create a sub object
	void		create (TSub type, const char *name, uint structId, const char *formName, uint slot);

	// Add a sub object
	CGeorgesEditDocSub	*add (TSub type, const char *name, uint structId, const char *formName, uint slot);

	// Clean the struct
	void	clean ();

	// Get item image
	int		getItemImage (CGeorgesEditDoc *doc) const;

private:

	uint								_StructId;
	std::string							_Name;
	std::string							_FormName;
	TSub								_Type;
	std::vector<CGeorgesEditDocSub*>	_Children;
	CGeorgesEditDocSub					*_Parent;
	uint								_Slot;
};

/**
  * To get the current sub object, use getSelectedObject.
  *
  * When the structure of the document changes (and the left tree too), CGeorgesEditDoc::updateDocumentStructure must be called.
  * This method destroy and construct the CGeorgesEditDocSub tree and the leftView CTreeCtrl.
  *
  * To change of sub selection, call changeSubSelection.
  *
  * When the left / right view need update, (document changes, sub object selection change..), call the right view getXXXFromDocument() method.
  * When the document need update from the left / right view, (right view widget edition), call the left / right view setXXXToDocument () method.
  */
class CGeorgesEditDoc : public CDocument, public NLGEORGES::IEditDocument
{
	friend class IAction;
protected: // create from serialization only
	CGeorgesEditDoc();
	// DECLARE_DYNCREATE(CGeorgesEditDoc)

public:
	/**
	  * Call this method when the document have been modified (not the structure but the document values).
	  *
	  * After this call, any CGeorgesEditDocSub pointers in this document are invalid.
	  */
	bool modify (class IAction *action, bool modified = true, bool setHeaderStateToModified = true);

protected:
	// Set the modified state of the document
	void setModifiedState (bool modified);

// Attributes
public:

	// Return the root node for a slot
	NLGEORGES::CFormElm *getRootNode (uint slot);

	// Trick, when this flag is true, no modification (made by modified()) can be made.
	bool NoModification;


	// A form loader
	NLGEORGES::CFormLoader	FormLoader;

	// The root sub object 
	CGeorgesEditDocSub		RootObject;

	/** 
	  * Return the current sub object
	  * 
	  * Warning, it you call modify or any method that call updateDocumentStructure (), this pointer is not valid anymore.
	  */
	CGeorgesEditDocSub		*getSelectedObject ();

	// Change the sub selection. If subSelection == NULL, select the content
	void					changeSubSelection (CGeorgesEditDocSub *subSelection, CView *view);
	void					changeSubSelection (uint subId, CView *view);

	// Switch focus to the given view
	CView					*switchToView(CView* pNewView);

	// Get the type pointer
	NLGEORGES::CType		*getTypePtr ();

	// Get the DFN pointer
	NLGEORGES::CFormDfn		*getDfnPtr ();

	// Get the form document pointer
	NLGEORGES::CForm		*getFormPtr ();

	// Get the header pointer
	NLGEORGES::CFileHeader	*getHeaderPtr ();

	// Get the left view
	CLeftView				*getLeftView ();

	// Get the right view
	CGeorgesEditView		*getRightView ();

	/** 
	  * Update the document structure. Call this when the document structure have been modified.
	  * It erases the sub object structure, rebuild it and rebuild the leftview tree.
	  *
	  * After this call, any CGeorgesEditDocSub pointers in this document are invalid.
	  */
	void					updateDocumentStructure ();

	/**
	  * Call when the view is actived / disactivated
	  */
	void onActivateView (bool activate);

	// Notify the plugins that this value has changed
	void notifyPlugins (const char *valueName);

	// Load a form file
	bool loadFormFile (const char *filename);

	// Add a parent form to the file
	bool addParent (const char *filename);

	// Document is a type ?
	virtual bool isType () const;

	// Document is a dfn ?
	virtual bool isDfn () const;

	// Docuemnt is a form ?
	virtual bool isForm () const;

	// overload setPathName to update plugin
	void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU);

protected:
	// Internal methods to construct the sub object structre
	CGeorgesEditDocSub		*addStruct (CGeorgesEditDocSub *parent, NLGEORGES::CFormElmStruct *_struct, NLGEORGES::CFormDfn *rootDfn, const char *name, uint structId, const char *formName, uint slot);

	// Internal methods to construct the sub object structre
	CGeorgesEditDocSub		*addArray (CGeorgesEditDocSub *parent, NLGEORGES::CFormElmArray *array, NLGEORGES::CFormDfn *rootDfn, const char *name, uint structId, const char *formName, uint slot);

protected:
	// Log system
	void									logValueChange (const char *valueName, const char *newValue, bool present);
	void									flushValueChange ();
	std::map<std::string, std::string>		_LastLogs;

	// Undo / redo system
	uint									_UndoModify;
	std::vector<IAction*>					_UndoBuffer;
	std::vector<IAction*>					_RedoBuffer;
	void									clearUndo ();
	void									clearRedo ();

protected:


	// The value
	NLMISC::CSmartPtr<NLGEORGES::CType>		Type;
	NLMISC::CSmartPtr<NLGEORGES::CFormDfn>	Dfn;
	NLMISC::CSmartPtr<NLGEORGES::UForm>		Form;

	// From IEditDocument
	NLGEORGES::UForm *getForm ();
	void getDfnFilename (std::string &dfnName);
	void bind (NLGEORGES::IEditPlugin *plugin, NLGEORGES::IEditDocumentPlugin *document);
	bool getActiveNode (std::string &dfnName);
	void refreshView ();
	void getFilename (std::string &pathname);
	void getTitle (std::string &pathname);
	void setValue (const char *valueName, const char *value, uint slot);

// Operations
public:

	// Plugin information
	class CPlugin
	{
	public:
		// Constructor
		CPlugin (NLGEORGES::IEditPlugin *plugin, NLGEORGES::IEditDocumentPlugin *pluginInterface);

		// The plugin document interface
		NLGEORGES::IEditDocumentPlugin		*PluginInterface;
		NLGEORGES::IEditPlugin				*Plugin;
	};

	// Array of plugin information
	std::vector<CPlugin>	PluginArray;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesEditDoc)
	public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGeorgesEditDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGeorgesEditDoc)
	afx_msg void OnUpdateFileSaveAll(CCmdUI* pCmdUI);
public:
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CGeorgesEditDocType : public CGeorgesEditDoc
{
protected: // create from serialization only
	CGeorgesEditDocType() {}
	DECLARE_DYNCREATE(CGeorgesEditDocType)

	// Document is a type ?
	bool	isType () const;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesEditDocType)
	public:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGeorgesEditDocType)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CGeorgesEditDocDfn : public CGeorgesEditDoc
{
protected: // create from serialization only
	CGeorgesEditDocDfn() {}
	DECLARE_DYNCREATE(CGeorgesEditDocDfn)

	// Document is a dfn ?
	bool	isDfn () const;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesEditDocDfn)
	public:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGeorgesEditDocDfn)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CGeorgesEditDocForm : public CGeorgesEditDoc
{
protected: // create from serialization only
	CGeorgesEditDocForm() {}
	DECLARE_DYNCREATE(CGeorgesEditDocForm)

	// Docuemnt is a form ?
	bool	isForm () const;

	// Init the document
	bool	initDocument (const char *dfnName, bool newElement);

	// Fetch a backup buffer
	void fetch (uint buffer);

	// Fetch a backup buffer
	void hold (uint buffer);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesEditDocForm)
	public:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGeorgesEditDocForm)
public:
	afx_msg void OnEditFetch1();
	afx_msg void OnEditFetch3();
	afx_msg void OnEditFetch4();
	afx_msg void OnEditFetch2();
	afx_msg void OnEditHold1();
	afx_msg void OnEditHold2();
	afx_msg void OnEditHold3();
	afx_msg void OnEditHold4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEORGES_EDITDOC_H__266E6547_220E_4A6E_9285_5F91BCD53E5B__INCLUDED_)
