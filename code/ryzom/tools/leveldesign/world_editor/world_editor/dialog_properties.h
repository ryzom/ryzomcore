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

#if !defined(AFX_DIALOG_PROPERTIES_H__0F911E98_1C9F_489C_B898_7D8E4455FADE__INCLUDED_)
#define AFX_DIALOG_PROPERTIES_H__0F911E98_1C9F_489C_B898_7D8E4455FADE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dialog_properties.h : header file
//

#include "my_list_box.h"
//#include "color_edit_wnd.h"


class CScrollPane : public CStatic
{
	class CDialogProperties	*m_dlgProperty;
public:

	void init(class CDialogProperties *dlgProp)
	{
		m_dlgProperty = dlgProp;
	}

	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};

/////////////////////////////////////////////////////////////////////////////
// CDialogProperties dialog

#define COMBO_STROKE_DELAI 500

class CMyComboBox : public CComboBox
{
public:
	CMyComboBox ()
	{
		_LastStrokeTime = 0;
		Widget = NULL;
		loaded = false;
	}
	void*						Widget;
	void						setData(const std::vector<std::string>& data) { _data = data; }
	bool						loaded;

private:
	std::string					_LastString;
	sint64						_LastStrokeTime;
	std::vector<std::string>	_data;
	void						reloadData();
	virtual BOOL				PreTranslateMessage( MSG* pMsg );
};


class CDialogProperties : public CDialog
{
	friend class CScrollPane;
	friend class CMyComboBox;
// Construction
public:
	CDialogProperties();
	CDialogProperties(std::list<NLLIGO::IPrimitive*> &locators, CWnd* pParent = NULL);   // standard constructor

	// Change the selection
	void changeSelection (std::list<NLLIGO::IPrimitive*> &locators);

	bool containsSelection( std::list<NLLIGO::IPrimitive*> &locators );
	bool containsSelection( std::vector<CDatabaseLocatorPointer> &locators );
	bool equalsSelection( std::list<NLLIGO::IPrimitive*> &locators );
	bool removePrimitives( std::list<NLLIGO::IPrimitive*> &locators );

	inline int getOriginalHeight() { return m_originalHeight; }

private:
	
	// Cancel button ?
	bool										_Cancel;

	// Modal window ?
	bool										_Modal;

	// Scrollbar ?
	bool										_ScrollBar;

	// locators
	std::vector<CDatabaseLocatorPointer>		_PropDlgLocators;

	// Class for a widget
	class CWidget
	{
	public:
		CWidget (CDialogProperties *dlg)
//			: MultiLineEditBox(CString("ui/ryzom.color"), true)
		{
			Set = false;
			Default = false;
			MultipleValues = false;
			Modified = false;
			Initializing = false;
			DialogProperties = dlg;
		};
		CWidget (const CWidget &other)
//			: MultiLineEditBox(CString("ui/ryzom.color"), true)
		{
			Static.Attach (other.Static.m_hWnd);
			ComboBox.Attach (other.ComboBox.m_hWnd);
			CheckBox.Attach (other.CheckBox.m_hWnd);
			EditBox.Attach (other.EditBox.m_hWnd);
			MultiLineEditBox.Attach (other.MultiLineEditBox.m_hWnd);
			ListEditBox.Attach (other.ListEditBox.m_hWnd);
			Set = other.Set;
			Default = other.Default;
			MultipleValues = other.MultipleValues;
			Modified = other.Modified;
			Initializing = other.Initializing;
			Parameter = other.Parameter;
			DialogProperties = other.DialogProperties;
		}
		~CWidget ()
		{
			if (IsWindow (Static.m_hWnd))
			{
				Static.Detach ();
			}
			if (IsWindow (ComboBox.m_hWnd))
			{
				ComboBox.Detach ();
			}
			if (IsWindow (CheckBox.m_hWnd))
			{
				CheckBox.Detach ();
			}
			if (IsWindow (EditBox.m_hWnd))
			{
				EditBox.Detach ();
			}
			if (IsWindow (MultiLineEditBox.m_hWnd))
			{
				MultiLineEditBox.Detach ();
			}
			if (IsWindow (ListEditBox.m_hWnd))
			{
				ListEditBox.Detach ();
			}
		}

		void setFocus();
		// Set from the parameter
		bool fromParameter (const NLLIGO::IProperty *property, const NLLIGO::IPrimitive &primitive, const NLLIGO::CPrimitiveClass *primitiveClass, const NLLIGO::CPrimitiveClass::CParameter &parameter);

		// Get to the parameter
		bool toParameter (const CDatabaseLocatorPointer &locator, const NLLIGO::CPrimitiveClass::CParameter &parameter);

		// Get the parameter
		void getValue (std::string &result) const;

		// Get the parameter
		void getValue (std::vector<std::string> &result) const;

		// Set the label name
		void setStaticName();

		// A value is set ?
		bool		Set;

		// Default value or not ?
		bool		Default;

		// Multiple value ?
		bool		MultipleValues;
		// The user has modified this property
		bool		Modified;

		// If true, EN_CHANGE is called because initialization process has made a setWindowText
		// If false, EN_CHANGE is called because the user has modified the text in the window text
		bool		Initializing;

		// Different MFC Widget
		CStatic		Static;
		CMyComboBox	ComboBox;
		CButton		CheckBox;
		CEdit		EditBox;
		CEdit		MultiLineEditBox;
//		ColorEditWnd	MultiLineEditBox;
		CMyListBox	ListEditBox;
		std::string	FromWhere;
		// CListCtrl	ListBox;

		/// For combo box only
		std::string	OriginalString;

		// The dialog property
		CDialogProperties	*DialogProperties;

		// The parameter
		NLLIGO::CPrimitiveClass::CParameter		Parameter;

		// Update boolean value
		void updateBoolean ();
		void updateCombo ();
		void updateMultiline ();
		void updateList ();

		// Get the filename to open, return "" if nothing to open.
		void getFilename (std::string &result);


		// return true if the given hwnd is one of this widget
		bool isHwndMatch(HWND hWnd);

		// get the next window
		HWND getNextWindow(HWND hwnd);
		// get the previous window
		HWND getPreviousWindow(HWND hwnd);

		bool OnSize( int cx, int cy, int decY );
	};

public:
	// Get the default value for this widget, return true if multiple value has been found
	void setDefaultValue (CWidget *widget, std::string &value);
	void setDefaultValue (CWidget *widget, std::vector<std::string> &value);


private:
	// Array of widget
	std::list<CWidget>	Widgets;

	// The default widget position
	RECT	WidgetPos;
	// minium dialog size
	uint	WindowWidth;
	uint	WindowHeight;

	static CPoint s_lastPosition;
	
private:

	// Accelerator table
	HACCEL	_AccelTable;

	CWidget *getWidget (uint widgetId);

	// Rebuild the dialog
	void rebuildDialog ();
	void ApplyAutoname();
	void SelectFolder(CWidget *widget);
		
public:
	// Add some widget
	void addWidget (const NLLIGO::CPrimitiveClass::CParameter &parameter, RECT &widgetPos, bool frozen);

	// Clear the widgets
	void removeWidgets ();
	void updateModification ();

	void updateModifiedState();

	//test whether any widget has been modified
	bool isModified();

	static const CPoint& getLastPosition() { return s_lastPosition; }

// Dialog Data
	//{{AFX_DATA(CDialogProperties)
	enum { IDD = IDD_PROPERTIES };
	CButton	m_updateButton;
	CButton	m_OKButton;
	CStatic	m_FirstProp;
	CStatic	m_PropertyFrame;
	CScrollBar	m_ScrollBar;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogProperties)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void OnSetFocus(CWnd*);
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_nScrollPos;
	//CRect m_rect, m_rect2;
	//CRect rectEntireScreen; 

	// Dynamicaly create control to contains the scrollable part
	CScrollPane	m_PropertyCont;
//	CStatic	m_PropertyCont;
	// Relative top position of button from the dialog bottom client rect
	int		m_ButtonPos;
	// default property frame position
	RECT	PropertyFrameRect;
	/// Relative bottom position of property frame from the dialog bottom client rect.
	int		m_FrameTopSpace;
	int		m_FrameBottomSpace;
	int		m_MaxFrameHeight;
	int		m_DialogWidth;
	/// Minimum container size
	int		m_ContMinHeight;

	CRect	m_curDlgRect;
	int		m_originalHeight;
	int		m_buttonWidth;
	
	// Generated message map functions
	//{{AFX_MSG(CDialogProperties)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnUpdate();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI );
	afx_msg void OnMove(int x, int y);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//extern CDialogProperties PropertyDialog;
extern std::list<CDialogProperties*> PropertiesDialogs;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOG_PROPERTIES_H__0F911E98_1C9F_489C_B898_7D8E4455FADE__INCLUDED_)
