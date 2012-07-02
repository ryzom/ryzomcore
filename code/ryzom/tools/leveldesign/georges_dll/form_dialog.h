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

#if !defined(FORM_TYPE_H_INCLUDED)
#define FORM_TYPE_H_INCLUDED

#include "base_dialog.h"
#include "edit_list_ctrl.h"
#include "color_wnd.h"
#include "icon_wnd.h"

#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_dfn.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// form_dialog.h : header file
//

class CGeorgesEditView;

namespace NLGEORGES
{
class CFormElmStruct;
class CFormElmVirtualStruct;
class CFormElmAtom;
class CFormElmArray;
class CFormElm;
};

class CFormDialog;

/**
  * Base class of form dialog widgets.
  */
class IFormWidget
{
public:

	// Type of sub parameter to edit
	enum TTypeSrc
	{
		TypeForm = 0,
		TypeFormParent,
		TypeArray,
		TypeType,
		TypeVirtualDfn,
	};
	
	// Constructor
	IFormWidget (CFormDialog *dialog, uint structId, const char *formName, TTypeSrc typeSrc, uint slot);

	// Destructor
	virtual ~IFormWidget () {}

	// Update the text of the base label
	void updateLabel ();

	// Does this id is in this widget ?
	bool isDialog (uint id) const;

	// Get the structure ID of this widget
	uint getStructId () const;

	// Get the type edited with this widget
	TTypeSrc getSrcType () const;

	// Get the form name of the widget
	const std::string &getFormName () const;

	// Get the slot number for this widget
	uint getSlot () const;

	// This widget is expendable in height (big edit for exemple)
	virtual bool extendableHeight () const;

	// Ok / cancel hit
	virtual void onOk () {}
	virtual void onCancel () {}

	// Update widget date
	virtual void updateData (bool update = true) = 0;

	// Does the widget has the focus
	virtual bool haveFocus () = 0;

	// Give the focus
	virtual void setFocus () = 0;

	// Does the window is in the widget
	virtual bool isWnd (const CWnd *wnd) const=0;

	// Resize callback
	virtual void resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize) = 0;

	// Get from document, update rightview UI
	virtual void getFromDocument (NLGEORGES::CForm &form) = 0;

	// Get the widget value
	virtual void getValue (std::string &result);

	// Get the widget array value count
	virtual uint getNumValue ();

	// Get the widget array value
	virtual void getValue (std::string &result, uint value);

	// Get the rect
	virtual bool getWindowRect (RECT &rect) const;

	// Open the selected text
	virtual void onOpenSelected ();

	// Get the sub object node using current sub object
	bool getNode (const NLGEORGES::CFormDfn **parentDfn, uint &indexDfn, const NLGEORGES::CFormDfn **nodeDfn, 
		const NLGEORGES::CType **nodeType, NLGEORGES::CFormElm **node, NLGEORGES::UFormDfn::TEntryType &type, 
		bool &array) const;

	// Get the sub object as CFormElm
	NLGEORGES::CFormElm *getFormElmNode () const;

	// Get the sub object as CFormElmStruct
	NLGEORGES::CFormElmStruct *getFormElmStructNode () const;

	// Get the sub object as CFormElmVirtualStruct
	NLGEORGES::CFormElmVirtualStruct *getFormElmVirtualStructNode () const;

	// Get the sub object as CFormElmArray
	NLGEORGES::CFormElmArray *getFormElmArrayNode () const;

	// Get the sub object as CFormElmAtom
	NLGEORGES::CFormElmAtom *getFormElmAtomNode () const;

protected:

	// Dialog Id
	uint		FirstId;

	// Dialog Id
	uint		LastId;

	// Struct id of the element
	uint		StructId;

	// Slot id
	uint		Slot;

	// String atom name
	std::string FormName;

	// The label
	CStatic		Label;

	// String atom name
	std::string SavedLabel;

	// Source edit type
	TTypeSrc		SrcType;

	// Parent dialog
	CFormDialog		*Dialog;
};

/**
  * Widgets implementation for memory combo.
  */
class CFormMemCombo : public IFormWidget
{
public:
	
	enum
	{
		CmdBrowse = 0,
	};

	CFormMemCombo (CFormDialog *dialog, uint structId, const char *atomName, TTypeSrc typeSrc, uint slot);
	virtual ~CFormMemCombo ();

	// The combo
	CMemoryComboBox	Combo;

	// The Spin
	CSpinButtonCtrl	Spin;

	// The Spin
	CButton			Browse;

	// Use the sipnner
	bool			UseSpinner;

	// Is a file browser
	bool			FileBrowser;

	// Create the widget
	void create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label, const char *reg, bool spinner, bool fileBrowser, const char *filenameExt);

	// From IFormWidget
	void onOk ();
	void updateData (bool update = true);
	bool haveFocus ();
	void setFocus ();
	void getFromDocument (NLGEORGES::CForm &form);
	void getValue (std::string &result);
	bool getWindowRect (RECT &rect) const;
	bool isWnd (const CWnd *wnd) const;
	void resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize);
};

/**
  * Widgets implementation for combo.
  */
class CFormCombo : public IFormWidget
{
public:
	
	enum
	{
		CmdBrowse,
	};

	CFormCombo (CFormDialog *dialog, uint structId, const char *atomName, IFormWidget::TTypeSrc typeForm, uint slot);
	virtual ~CFormCombo ();

	// The combo
	CComboBox	Combo;

	// Create the widget
	void create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label);

	// From IFormWidget
	void onOk ();
	void updateData (bool update = true);
	bool haveFocus ();
	void setFocus ();
	void getFromDocument (NLGEORGES::CForm &form);
	void getValue (std::string &result);
	bool getWindowRect (RECT &rect) const;
	bool isWnd (const CWnd *wnd) const;
	void resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize);
};

/**
  * Widgets implementation for combo.
  */
class CFormBigEdit : public IFormWidget
{
public:
	
	enum
	{
		CmdBrowse,
	};

	CFormBigEdit (CFormDialog *dialog, uint structId, const char *atomName, IFormWidget::TTypeSrc typeForm, uint slot);
	virtual ~CFormBigEdit ();

	// The combo
	CEdit		Edit;

	// Create the widget
	void create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label);

	// From IFormWidget
	void onOk ();
	void updateData (bool update = true);
	bool haveFocus ();
	void setFocus ();
	void getFromDocument (NLGEORGES::CForm &form);
	void getValue (std::string &result);
	bool getWindowRect (RECT &rect) const;
	bool isWnd (const CWnd *wnd) const;
	void resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize);
	bool extendableHeight () const;
};

/**
  * Widgets implementation for color dialog.
  */
class CColorEdit : public IFormWidget
{
public:
	
	enum
	{
		CmdBrowse,
	};

	CColorEdit (CFormDialog *dialog, uint structId, const char *atomName, IFormWidget::TTypeSrc typeForm, uint slot);
	virtual ~CColorEdit ();

	// The combo
	CColorWnd	Color;
	CButton		Reset;
	CEdit		Edit;
	bool		Empty;

	// Create the widget
	void create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label);

	// From IFormWidget
	void onOk ();
	void updateData (bool update = true);
	bool haveFocus ();
	void setFocus ();
	void getFromDocument (NLGEORGES::CForm &form);
	void getValue (std::string &result);
	bool getWindowRect (RECT &rect) const;
	bool isWnd (const CWnd *wnd) const;
	void resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize);
};

/**
  * Widgets implementation for list.
  */
class CListWidget : public IFormWidget
{
public:
	
	CListWidget (CFormDialog *dialog, uint structId, const char *atomName, IFormWidget::TTypeSrc typeForm, uint slot);
	virtual ~CListWidget ();

private:

	// My list ctrl
	class CMyEditListCtrl : public CEditListCtrl
	{
		friend CListWidget;
		CEditListCtrl::TItemEdit getItemEditMode (uint item, uint subItem);
		void getMemComboBoxProp (uint item, uint subItem, std::string &regAdr, bool &browse);
		void getNewItemText (uint item, uint subItem, std::string &ret);
		void getBrowseInfo (uint item, uint subItem, std::string &defExt, std::string &defFilename, std::string &defDir, std::string &filter);
		CListWidget	*Ctrl;
	};
	friend class CListWidget::CMyEditListCtrl;

	// The list
	std::string		RegAdr;
	uint			Divid;
	CMyEditListCtrl	ListCtrl;
	CButton			Button;

public:
	// Create the widget
	void create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label, const char *reg, uint Divid=1);

	// Add column
	void addColumn (const char *name);

	// From IFormWidget
	void onOk ();
	void updateData (bool update = true);
	bool haveFocus ();
	void setFocus ();
	void getFromDocument (NLGEORGES::CForm &form);
	void getValue (std::string &result, uint value);
	bool getWindowRect (RECT &rect) const;
	uint getNumValue ();
	bool isWnd (const CWnd *wnd) const;
	void resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize);
	bool extendableHeight () const;
	void onOpenSelected ();
};

/**
  * Widgets implementation for color dialog.
  */
class CIconWidget : public IFormWidget
{
public:

	CIconWidget (CFormDialog *dialog, uint structId, const char *atomName, IFormWidget::TTypeSrc typeForm, uint slot);
	virtual ~CIconWidget ();

	// The combo
	CIconWnd	Icon;

	// Create the widget
	void create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label);

	// From IFormWidget
	void onOk ();
	void updateData (bool update = true);
	bool haveFocus ();
	void setFocus ();
	void getFromDocument (NLGEORGES::CForm &form);
	void getValue (std::string &result);
	bool getWindowRect (RECT &rect) const;
	bool isWnd (const CWnd *wnd) const;
	void resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize);
};


/**
  * The dialog object for FORM
  */
class CFormDialog : public CBaseDialog
{
// Construction
public:
	CFormDialog ();   // standard constructor
	~CFormDialog ();   // standard constructor

	enum
	{
		TypeCombo= 100,
	};

	enum
	{
		LtParents,
		LtStruct,
	};

	enum
	{
		ParentHeight = 80,
		DfnHeight = 230,
	};

	// Widget index count
	uint WidgetIndexCount;
	uint WidgetFocused;

	// Array of widget
	std::vector<IFormWidget*>	Widgets;

	// From CDialog
	virtual void OnOK ();
	virtual void OnCancel ();
	
	// From CBaseDialog
	void onOpenSelected ();
	void onFirstFocus ();
	void onLastFocus ();

	// Get from document, update rightview UI
	void getFromDocument ();

	// Get the dfn name of the form
	void getDfnName (std::string &result) const;

	// On sub focus changed
	void onGetSubFocus (uint id);

private:
	// Sub update functions
	void getVirtualDfnFromDocument (const NLGEORGES::CFormDfn *_dfn, const char *structName, uint slot);
	void getDfnFromDocument (const NLGEORGES::CFormDfn &_dfn, const char *structName, uint slot);
	void getArrayFromDocument (const char *arrayName, uint structId, uint slot);
	void getTypeFromDocument (const NLGEORGES::CType &_type, const char *name, const char *typeFilename, const char *structName, uint slot);
public:

	// Set to document, update document with rightview UI
	void setToDocument (uint widget);

	void resizeWidgets ();

	void updateLabels ();

	void updateValues ();

	// Clear the dialog
	void clear ();

private:
	// Get the widget id in widgets array with a dialog Id.
	int getWidget (uint dialogId) const;

	// Add a widget
	CWnd* addTypeWidget (const NLGEORGES::CType &type, uint elmIndex, const char *title, const char *atomName, 
		const char *typeFilename, RECT &currentPos, NLGEORGES::CForm &form, IFormWidget::TTypeSrc typeWidget, const char *filenameExt, uint slot);


	CWnd* GetNextDlgTabItem( CWnd* pWndCtl, BOOL bPrevious = FALSE ) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFormDialog)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pNewWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(FORM_TYPE_H_INCLUDED)
