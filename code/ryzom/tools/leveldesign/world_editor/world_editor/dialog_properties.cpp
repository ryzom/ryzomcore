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

// dialog_properties.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "main_frm.h"
#include "dialog_properties.h"
#include "nel/misc/path.h"
#include "action.h"
#include <string>
#include "nel/ligo/primitive.h"
#include "tools_logic.h"
#include "external_editor.h"
#include "file_dialog_ex.h"

using namespace std;
using namespace NLLIGO;
using namespace NLMISC;

// ***************************************************************************

#define SCROLLING_STEPS	2	// parameter to finetune the scroller

#define CHECKBOX_HEIGHT 20
#define COMBO_REAL_HEIGHT 300
#define COMBO_HEIGHT 20
#define LABEL_HEIGHT 15
#define SPACE_HEIGHT (widgetPos.left)
#define EDIT_HEIGHT 18
#define FIRST_WIDGET 10
#define LAST_WIDGET 100
#define FILE_BUTTON_WIDTH 50

#define STRING_SELECT_COMBOBOX_ID 9

#define DIFFERENT_VALUE_STRING "<different values>"
#define DIFFERENT_VALUE_MULTI_STRING "<diff>"

//CDialogProperties PropertyDialog;
std::list<CDialogProperties*> PropertiesDialogs;
CPoint CDialogProperties::s_lastPosition = CPoint( -1, -1 );


BOOL CScrollPane::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// forward control command to dialog.
	return m_dlgProperty->OnCommand(wParam, lParam);
}

LRESULT CScrollPane::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case LBN_CHANGE:
		m_dlgProperty->WindowProc(message, wParam, lParam);
		break;
	}
	
	return CStatic::WindowProc(message, wParam, lParam);
}

// ***************************************************************************
// CDialogProperties dialog
// ***************************************************************************

CDialogProperties::CDialogProperties()
{
	m_PropertyCont.init(this);
	_Modal = false;
	_Cancel = true;
	_ScrollBar = false;
}

// ***************************************************************************

CDialogProperties::CDialogProperties(std::list<NLLIGO::IPrimitive*> &locators, CWnd* pParent /*=NULL*/)
	: CDialog(CDialogProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogProperties)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	m_PropertyCont.init(this);

	// Reserve some memory
	_PropDlgLocators.reserve (locators.size ());

	CWorldEditorDoc *doc = getDocument ();
	std::list<NLLIGO::IPrimitive*>::iterator ite = locators.begin ();
	while (ite != locators.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);
		_PropDlgLocators.push_back (locator);
		ite++;
	}

	_Modal = true;
	_Cancel = false;
}

// ***************************************************************************

void CDialogProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogProperties)
	DDX_Control(pDX, IDUPDATE, m_updateButton);
	DDX_Control(pDX, IDOK, m_OKButton);
	DDX_Control(pDX, IDC_FIRST_PROP, m_FirstProp);
	DDX_Control(pDX, IDC_PROPERTY_FRAME, m_PropertyFrame);
	DDX_Control(pDX, IDC_SCROLLBAR_PROP, m_ScrollBar);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CDialogProperties, CDialog)
	//ON_NOTIFY_RANGE( LVN_ITEMACTIVATE, 0, -1, onListCtrlItemActivate )
	//{{AFX_MSG_MAP(CDialogProperties)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDUPDATE, OnUpdate)
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_SETFOCUS()
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CDialogProperties message handlers
// ***************************************************************************

void CDialogProperties::OnOK() 
{

	OnUpdate();

	::CRect rect;
	GetWindowRect( &rect );
	
	s_lastPosition = CPoint( rect.left, rect.top );
	PropertiesDialogs.remove( this );

	CDialog::OnOK();
}

// ***************************************************************************

void CDialogProperties::OnCancel() 
{
	::CRect rect;
	GetWindowRect( &rect );
	
	s_lastPosition = CPoint( rect.left, rect.top );
	PropertiesDialogs.remove( this );

	CDialog::OnCancel();
}

// ***************************************************************************

void CDialogProperties::removeWidgets ()
{
	std::list<CWidget>::iterator ite = Widgets.begin ();
	while (ite != Widgets.end ())
	{
		/*
		CWidget &widget = *ite;
		// Create a label ?
		if ( (widget.Parameter.Type == CPrimitiveClass::CParameter::ConstString) || 
			 (widget.Parameter.Type == CPrimitiveClass::CParameter::String) || 
			 (widget.Parameter.Type == CPrimitiveClass::CParameter::StringArray) ||
			 (widget.Parameter.Type == CPrimitiveClass::CParameter::ConstStringArray))
		{
			// Create the label
			widget.Static.DestroyWindow ();
		}

		// What kind of primitive ?
		if (widget.Parameter.Type == CPrimitiveClass::CParameter::Boolean)
		{
			widget.CheckBox.DestroyWindow ();

		}
		else if (widget.Parameter.Type == CPrimitiveClass::CParameter::ConstString)
		{
			widget.ComboBox.DestroyWindow ();
		}
		else if (widget.Parameter.Type == CPrimitiveClass::CParameter::String)
		{
			widget.EditBox.DestroyWindow ();
			if (widget.Parameter.FileExtension != "")
			{
				widget.CheckBox.DestroyWindow ();
			}
		}
		else if (widget.Parameter.Type == CPrimitiveClass::CParameter::StringArray)
		{
			widget.MultiLineEditBox.DestroyWindow ();
			if (!widget.Parameter.Folder.empty() || !widget.Parameter.FileExtension.empty())
			{
				widget.CheckBox.DestroyWindow ();
			}
		}
		else if (widget.Parameter.Type == CPrimitiveClass::CParameter::ConstStringArray)
		{
			widget.ListEditBox.DestroyWindow ();
		}

		// File ?
		if (widget.Parameter.Filename)
		{
			widget.CheckBox.DestroyWindow ();
		}
		*/
		CWidget &widget = *ite;
		CWnd *windows[] = { &widget.Static, &widget.ComboBox, &widget.CheckBox, &widget.EditBox, &widget.MultiLineEditBox, &widget.ListEditBox};
		int i;
		for (i=0; i<sizeof(windows)/sizeof(CWnd*); i++)
		{
			if (IsWindow (*(windows[i])))
				windows[i]->DestroyWindow ();
		}

		ite++;
	}

	Widgets.clear ();

/*	RECT rect;
	GetWindowRect (&rect);
	rect.bottom = rect.top + WindowHeight;
	MoveWindow (&rect);
	GetDlgItem (IDOK)->GetWindowRect (&rect);
	ScreenToClient (&rect);
	rect.top = WidgetPos.top;
	rect.bottom = WidgetPos.bottom;
	GetDlgItem (IDOK)->MoveWindow (&rect);
	GetDlgItem (IDCANCEL)->GetWindowRect (&rect);
	ScreenToClient (&rect);
	rect.top = WidgetPos.top;
	rect.bottom = WidgetPos.bottom;
	GetDlgItem (IDCANCEL)->MoveWindow (&rect);
	GetDlgItem (IDUPDATE)->GetWindowRect (&rect);
	ScreenToClient (&rect);
	rect.top = WidgetPos.top;
	rect.bottom = WidgetPos.bottom;
	GetDlgItem (IDUPDATE)->MoveWindow (&rect);
*/

}

// ***************************************************************************

void CDialogProperties::addWidget (const CPrimitiveClass::CParameter &parameter, RECT &widgetPos, bool frozen)
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Get client rect
	RECT clientRect;
	m_PropertyCont.GetClientRect(&clientRect);
//	GetClientRect (&clientRect);
	RECT firstProp;
	m_FirstProp.GetWindowRect(&firstProp);

	// Backup top
	uint oldTop = widgetPos.top;
	// backup right
	uint oldRight = widgetPos.right;
		
	// Set the right limit
//	widgetPos.right = clientRect.right - widgetPos.left;
// 	widgetPos.right = firstProp.right;

	// File ?
	if (parameter.Filename
		|| (parameter.Type==CPrimitiveClass::CParameter::String && parameter.FileExtension != "")
		|| (parameter.Type==CPrimitiveClass::CParameter::StringArray && parameter.Folder != "") )
		widgetPos.right -= FILE_BUTTON_WIDTH;

	// Widget id
	uint id = Widgets.size ()+FIRST_WIDGET;

	// Add a widget
	Widgets.push_back (CWidget (this));
	CWidget &widget = Widgets.back ();

	// Copy the parameter
	widget.Parameter = parameter;

	// Widget enabled ?
	bool enabled = !parameter.ReadOnly && !frozen;

	// Create a label ?
	if ( (widget.Parameter.Type == CPrimitiveClass::CParameter::ConstString) || 
		 (widget.Parameter.Type == CPrimitiveClass::CParameter::String) || 
		 (widget.Parameter.Type == CPrimitiveClass::CParameter::StringArray) ||
		 (widget.Parameter.Type == CPrimitiveClass::CParameter::ConstStringArray) )
	{
		// Resize the widgetPos
		widgetPos.bottom = widgetPos.top + LABEL_HEIGHT;
		
		if (widget.Parameter.Type == CPrimitiveClass::CParameter::StringArray && widget.Parameter.Folder != "")
		{
			RECT buttonRect = widgetPos;
			buttonRect.left = buttonRect.right;
			buttonRect.right = buttonRect.left + FILE_BUTTON_WIDTH;
			
			// Create an edit box
			nlverify (widget.CheckBox.Create ("Select", BS_PUSHBUTTON|WS_VISIBLE|WS_TABSTOP|(enabled?0:WS_DISABLED), buttonRect, &m_PropertyCont, id));
			widget.CheckBox.SetFont (GetFont ());
		}

		// Create the label
		widget.Static.Create ("", WS_VISIBLE, widgetPos, &m_PropertyCont);
		widget.Static.SetFont (GetFont ());

		// Next position
		widgetPos.top = widgetPos.bottom;
	}

	// What kind of primitive ?
	if (widget.Parameter.Type == CPrimitiveClass::CParameter::Boolean)
	{
		// Resize the widgetPos
		widgetPos.bottom = widgetPos.top + CHECKBOX_HEIGHT;

		string Name = widget.Parameter.Name;

		// Create a check box
		nlverify (widget.CheckBox.Create (Name.c_str (), BS_3STATE|WS_VISIBLE|WS_TABSTOP|(enabled?0:WS_DISABLED), widgetPos, &m_PropertyCont, id));
		widget.CheckBox.SetFont (GetFont ());
	}
	else if (widget.Parameter.Type == CPrimitiveClass::CParameter::ConstString)
	{
		// Resize the widgetPos
		RECT comboPos = widgetPos;
		widgetPos.bottom = widgetPos.top + COMBO_HEIGHT;
		comboPos.bottom = widgetPos.top + COMBO_REAL_HEIGHT;

		// Create a combo box
		//nlverify (widget.ComboBox.Create (CBS_DISABLENOSCROLL|WS_VSCROLL|CBS_DROPDOWNLIST|(widget.Parameter.SortEntries?CBS_SORT:0)|WS_VISIBLE|WS_TABSTOP|(enabled?0:WS_DISABLED), comboPos, &m_PropertyCont, id));
		nlverify (widget.ComboBox.Create (CBS_DISABLENOSCROLL|WS_VSCROLL|(widget.Parameter.Editable?CBS_DROPDOWN:CBS_DROPDOWNLIST)|(widget.Parameter.SortEntries?CBS_SORT:0)|WS_VISIBLE|WS_TABSTOP|(enabled?0:WS_DISABLED), comboPos, &m_PropertyCont, id));
		widget.ComboBox.loaded = false;
		widget.ComboBox.Widget = &widget;
		widget.ComboBox.SetFont (GetFont ());

		// Get the good list
		widget.ComboBox.ResetContent ();
		string	ctx = doc->getContext ();
		std::map<std::string, CPrimitiveClass::CParameter::CConstStringValue>::iterator ite = widget.Parameter.ComboValues.find (doc->getContext ().c_str());
		// we insert an empty string in case of a default value
		if (!widget.Parameter.SortEntries)
		{
			widget.ComboBox.InsertString( -1, "");
		}
		else
		{
			widget.ComboBox.AddString("");
		}
		if (ite != widget.Parameter.ComboValues.end ())
		{
			vector<string>	PathList;
			{
				ite->second.appendFilePath(PathList);
				
				vector<const IPrimitive*>	relativePrimPaths;
				{
					vector<const IPrimitive*>	startPrimPath;
					for (uint locIndex=0;locIndex<_PropDlgLocators.size();locIndex++)
						startPrimPath.push_back(_PropDlgLocators[locIndex].Primitive);
					ite->second.getPrimitivesForPrimPath(relativePrimPaths, startPrimPath);
				}
				
				ite->second.appendPrimPath(PathList, relativePrimPaths);
			}
			if (widget.Parameter.SortEntries) 
					std::sort(PathList.begin(), PathList.end());
			widget.ComboBox.setData(PathList);		
		}

		// Add default values
		if (doc->getContext () != "default")
		{
			ite = widget.Parameter.ComboValues.find ("default");
			if (ite != widget.Parameter.ComboValues.end ())
			{
				vector<string>	PathList;
				{
					ite->second.appendFilePath(PathList);
					
					vector<const IPrimitive*>	relativePrimPaths;
					{
						vector<const IPrimitive*>	startPrimPath;
						for (uint locIndex=0;locIndex<_PropDlgLocators.size();locIndex++)
							startPrimPath.push_back(_PropDlgLocators[locIndex].Primitive);
						ite->second.getPrimitivesForPrimPath(relativePrimPaths, startPrimPath);
					}
					
					ite->second.appendPrimPath(PathList, relativePrimPaths);
				}
				if (widget.Parameter.SortEntries)
					std::sort(PathList.begin(), PathList.end());
				widget.ComboBox.setData(PathList);
			}
		}
	}
	else if (widget.Parameter.Type == CPrimitiveClass::CParameter::String)
	{
		// Resize the widgetPos
		widgetPos.bottom = widgetPos.top + EDIT_HEIGHT;

		// Create an edit box
		nlverify (widget.EditBox.CreateEx (WS_EX_CLIENTEDGE, _T("EDIT"), "", WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|(enabled?0:ES_READONLY), widgetPos, &m_PropertyCont, id));
		widget.EditBox.SetFont (GetFont ());
		if (widget.Parameter.FileExtension != "")
		{
			RECT buttonRect = widgetPos;
			buttonRect.left = buttonRect.right;
			buttonRect.right = buttonRect.left + FILE_BUTTON_WIDTH;
			
			// Create an edit box
			nlverify (widget.CheckBox.Create ("Open...", BS_PUSHBUTTON|WS_VISIBLE|WS_TABSTOP|(enabled?0:WS_DISABLED), buttonRect, &m_PropertyCont, id));
			widget.CheckBox.SetFont (GetFont ());
		}
	}
	else if (widget.Parameter.Type == CPrimitiveClass::CParameter::StringArray)
	{
		// Resize the widgetPos
		widgetPos.bottom = widgetPos.top + widget.Parameter.WidgetHeight;

		// Create an edit box
		if (widget.Parameter.DisplayHS)
		{
			nlverify (widget.MultiLineEditBox.CreateEx (WS_EX_CLIENTEDGE, _T("EDIT"), "", 
				WS_VSCROLL|WS_HSCROLL|ES_MULTILINE|ES_WANTRETURN|WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|ES_AUTOVSCROLL|(enabled?0:ES_READONLY), widgetPos, &m_PropertyCont, id));
		}
		else
		{
			nlverify (widget.MultiLineEditBox.CreateEx (WS_EX_CLIENTEDGE, _T("EDIT"), "", 
				WS_VSCROLL|ES_MULTILINE|ES_WANTRETURN|WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|ES_AUTOVSCROLL|(enabled?0:ES_READONLY), widgetPos, &m_PropertyCont, id));
		}


		// Resize the column
		RECT listRect;
		widget.MultiLineEditBox.GetClientRect (&listRect);

		CFont font;
		font.CreateStockObject (ANSI_FIXED_FONT);
		widget.MultiLineEditBox.SetFont (&font);

		// Create an "EDIT" button if the text is editable (FileExtension != "")
		if (widget.Parameter.FileExtension != "")
		{
			widgetPos.top = widgetPos.bottom;
			widgetPos.bottom += LABEL_HEIGHT;
			RECT buttonRect = widgetPos;
			
			// Create an edit box
			nlverify (widget.CheckBox.Create ("Edit...", BS_PUSHBUTTON|WS_VISIBLE|WS_TABSTOP|(enabled?0:WS_DISABLED), buttonRect, &m_PropertyCont, id));
			widget.CheckBox.SetFont (GetFont ());
		}
	}
	else if (widget.Parameter.Type == CPrimitiveClass::CParameter::ConstStringArray)
	{
		// Resize the widgetPos
		widgetPos.bottom = widgetPos.top + widget.Parameter.WidgetHeight;

		// Create an edit box
		nlverify (widget.ListEditBox.CreateEx (WS_EX_CLIENTEDGE, _T("LISTBOX"), "", WS_VSCROLL|WS_CHILD|WS_VISIBLE|WS_TABSTOP|LBS_NOTIFY|(enabled?0:WS_DISABLED), widgetPos, &m_PropertyCont, id));

		// Resize the column
		RECT listRect;
		widget.ListEditBox.GetClientRect (&listRect);

		// Get the good list
		widget.ListEditBox.StringSelectComboBox.ResetContent ();
		std::map<std::string, CPrimitiveClass::CParameter::CConstStringValue>::iterator ite = widget.Parameter.ComboValues.find (doc->getContext ().c_str());
		// we insert an empty string in case of a default value
		widget.ListEditBox.StringSelectComboBox.InsertString( -1, "");
		if (ite != widget.Parameter.ComboValues.end ())
		{
			vector<string>	PathList;
			{
				ite->second.appendFilePath(PathList);
				
				vector<const IPrimitive*>	relativePrimPaths;
				{
					vector<const IPrimitive*>	startPrimPath;
					for (uint locIndex=0;locIndex<_PropDlgLocators.size();locIndex++)
						startPrimPath.push_back(_PropDlgLocators[locIndex].Primitive);
					ite->second.getPrimitivesForPrimPath(relativePrimPaths, startPrimPath);
				}
				
				ite->second.appendPrimPath(PathList, relativePrimPaths);
			}
			
			for (vector<string>::iterator	it=PathList.begin(), itEnd=PathList.end(); it!=itEnd; ++it)
			{
				widget.ListEditBox.StringSelectComboBox.InsertString( -1, it->c_str ());
			}
		}

		// Add default values
		if (doc->getContext () != "default")
		{
			ite = widget.Parameter.ComboValues.find ("default");
			if (ite != widget.Parameter.ComboValues.end ())
			{
				vector<string>	PathList;
				{
					ite->second.appendFilePath(PathList);
					
					vector<const IPrimitive*>	relativePrimPaths;
					{
						vector<const IPrimitive*>	startPrimPath;
						for (uint locIndex=0;locIndex<_PropDlgLocators.size();locIndex++)
							startPrimPath.push_back(_PropDlgLocators[locIndex].Primitive);
						ite->second.getPrimitivesForPrimPath(relativePrimPaths, startPrimPath);
					}
					
					ite->second.appendPrimPath(PathList, relativePrimPaths);
				}
				
				for (vector<string>::iterator	it=PathList.begin(), itEnd=PathList.end(); it!=itEnd; ++it)
				{
					widget.ListEditBox.StringSelectComboBox.InsertString( -1, it->c_str ());
				}
			}
		}

		widget.ListEditBox.SetFont (GetFont ());
		widget.ListEditBox.StringSelectComboBox.SetFont (GetFont ());
	}

	// File ?
	// if ((parameter.Filename) && (widget.Parameter.Type != CPrimitiveClass::CParameter::ConstStringArray))
	if (parameter.Filename && (parameter.FileExtension.empty() || widget.Parameter.Type != CPrimitiveClass::CParameter::StringArray))
	{
		// Resize the widgetPos
		RECT buttonRect = widgetPos;
		buttonRect.left = buttonRect.right;
		buttonRect.right = buttonRect.left + FILE_BUTTON_WIDTH;

		// Create an edit box
		//nlverify (widget.CheckBox.Create ("Open...", BS_PUSHBUTTON|WS_VISIBLE|WS_TABSTOP, buttonRect, this, id));
		nlverify (widget.CheckBox.Create ("View...", BS_PUSHBUTTON|WS_VISIBLE|WS_TABSTOP|(enabled?0:WS_DISABLED), buttonRect, &m_PropertyCont, id));
		widget.CheckBox.SetFont (GetFont ());
	}

	// Next position
	widgetPos.top = widgetPos.bottom + SPACE_HEIGHT;

	// Move ok and cancel button
//	uint heightGrow = widgetPos.top - oldTop;
/*	RECT okRect;
	GetDlgItem (IDOK)->GetWindowRect (&okRect);
	ScreenToClient (&okRect);
	okRect.top += heightGrow;
	okRect.bottom += heightGrow;
	GetDlgItem (IDOK)->MoveWindow (&okRect);
	GetDlgItem (IDCANCEL)->GetWindowRect (&okRect);
	ScreenToClient (&okRect);
	okRect.top += heightGrow;
	okRect.bottom += heightGrow;
	GetDlgItem (IDCANCEL)->MoveWindow (&okRect);
	GetDlgItem (IDUPDATE)->GetWindowRect (&okRect);
	ScreenToClient (&okRect);
	okRect.top += heightGrow;
	okRect.bottom += heightGrow;
	GetDlgItem (IDUPDATE)->MoveWindow (&okRect);

	// Resize the dialog box
	RECT windowRect;
	GetWindowRect (&windowRect);
	
	// Center only modal window
	if (_Modal)
	{
		sint width = windowRect.right - windowRect.left;
		sint height = windowRect.bottom - windowRect.top;
		RECT desktop;
		GetDesktopWindow ()->GetClientRect (&desktop);
		windowRect.left = (desktop.right - desktop.left - width) / 2;
		windowRect.right = windowRect.left + width;
		windowRect.top = (desktop.bottom - desktop.top - height) / 2;
		windowRect.top -= heightGrow/2;
		windowRect.bottom = windowRect.top + height + heightGrow;
	}
	else
	{
		windowRect.bottom += heightGrow;
	}
*/
	// resize the property container
	RECT contRect;
	m_PropertyCont.GetClientRect(&contRect);
	contRect.bottom = widgetPos.top;
	m_PropertyCont.CalcWindowRect(&contRect, 0);
	m_PropertyCont.MoveWindow(&contRect, TRUE);

	// Not set
	widget.Set = false;

	// restore righ
	widgetPos.right = oldRight;
}

// ***************************************************************************

BOOL CDialogProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Load accel table
	nlverify (_AccelTable = LoadAccelerators (theApp.m_hInstance, MAKEINTRESOURCE(IDR_DIALOG)));
 
	// Get the OK Button position
//	GetDlgItem (IDOK)->GetWindowRect (&WidgetPos);
	RECT winRect;
	GetWindowRect (&winRect);
	WindowWidth = winRect.right - winRect.left;
	WindowHeight = winRect.bottom - winRect.top;

	// Center only modal window
	RECT desktop;
	GetDesktopWindow ()->GetClientRect (&desktop);
	GetWindowRect (&winRect);
	uint width = winRect.right - winRect.left;
	uint height = winRect.bottom - winRect.top;
	winRect.top = (desktop.bottom - desktop.top - height) / 2;
	winRect.left = (desktop.right - desktop.left - width) / 2;
	winRect.right = winRect.left + width;
	winRect.bottom = winRect.top + height;
	MoveWindow (&winRect);

	RECT contRect;
	m_PropertyFrame.GetClientRect(&contRect);
	m_ContMinHeight = contRect.bottom;
//	m_PropertyFrame.ClientToScreen(&contRect);
	// leave 16 px for the scroll bar
	contRect.right-=16;
	m_PropertyCont.Create("", 0, contRect, &m_PropertyFrame);
//	m_PropertyCont.SetCursor()

	m_PropertyCont.ShowWindow(SW_SHOW);

	// store the relative starting position for widget
	m_FirstProp.GetWindowRect (&WidgetPos);
	m_PropertyCont.ScreenToClient (&WidgetPos);

	// Store the relative button position
	RECT buttonRect, dialogRect;
	m_OKButton.GetWindowRect(&buttonRect);
	ScreenToClient(&buttonRect);
	GetClientRect(&dialogRect);
	m_ButtonPos = dialogRect.bottom - buttonRect.top;

	// Store the property frame position
	m_PropertyFrame.GetWindowRect(&PropertyFrameRect);
	ScreenToClient(&PropertyFrameRect);

	// Store the relative frame position
	GetWindowRect(&dialogRect);
	ScreenToClient(&dialogRect);
	m_FrameTopSpace = PropertyFrameRect.top - dialogRect.top;
	m_FrameBottomSpace = dialogRect.bottom - PropertyFrameRect.bottom;

//	RECT desktop;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &desktop, 0);
	m_MaxFrameHeight = desktop.bottom - desktop.top - m_FrameTopSpace - m_FrameBottomSpace;
	

	// Rebuild the dialog
	rebuildDialog ();
	
	// Show / hide cancel
	GetDlgItem (IDCANCEL)->ShowWindow ( SW_SHOW );//_Cancel?SW_SHOW:SW_HIDE);
	GetDlgItem (IDUPDATE)->ShowWindow ( SW_SHOW );//_Cancel?SW_SHOW:SW_HIDE);

	GetDlgItem(IDUPDATE)->GetWindowRect(&buttonRect);
	ScreenToClient(&buttonRect);
	m_buttonWidth = buttonRect.right;
	GetDlgItem(IDOK)->GetWindowRect(&buttonRect);
	ScreenToClient(&buttonRect);
	m_buttonWidth -= buttonRect.left;


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************
// CDialogProperties::CWidget
// ***************************************************************************

bool CDialogProperties::CWidget::fromParameter (const IProperty *property, const IPrimitive &primitive, const CPrimitiveClass *primitiveClass, const CPrimitiveClass::CParameter &parameter)
{
	// Good one ?
	if (parameter == Parameter)
	{
		// Get pointers
		const CPropertyString *propString = dynamic_cast<const CPropertyString *> (property);
		const CPropertyStringArray *propStringString = dynamic_cast<const CPropertyStringArray *> (property);

		// Default value ?
		if	(	(property == NULL)
			||	(!Set && property->Default))
			Default = true;

		// Check box ?
		if (Parameter.Type == CPrimitiveClass::CParameter::StringArray)
		{
			if (propStringString)
			{
				// Get the value
				vector<string> currentValue;
				getValue (currentValue);

				// Already sets ?
				if (Set && (((currentValue != propStringString->StringArray) && (!Default || !propStringString->Default)) || (Default != propStringString->Default)))
				{
					// Not by default
					Default = false;

					// Set as uninitialized
					setEditTextMultiLine (MultiLineEditBox, DIFFERENT_VALUE_STRING);
					updateMultiline ();
					MultipleValues = true;
				}
				else
				{
					// Set as filled
					Set = true;

					// Add the string
					setEditTextMultiLine (MultiLineEditBox, propStringString->StringArray);
					updateMultiline ();
				}
			}
			else
			{
				// Add the strings
				string temp;
				vector<std::string> result;
				Parameter.getDefaultValue (result, primitive, *primitiveClass, &FromWhere);

				uint i;
				for (i=0; i<result.size(); i++)
				{
					temp += result[i];
					if (i != (result.size()-1))
						temp += '\n';
				}
				setEditTextMultiLine (MultiLineEditBox, temp.c_str());
				updateMultiline ();

				// Set as filled
				Set= true;
			}
		}
		else if (Parameter.Type == CPrimitiveClass::CParameter::ConstStringArray)
		{
			if (propStringString)
			{
				// Get the value
				vector<string> currentValue;
				getValue (currentValue);

				// Already sets ?
				if (Set && (((currentValue != propStringString->StringArray) && (!Default || !propStringString->Default)) || (Default != propStringString->Default)))
				{
					// Not by default
					Default = false;

					// Set as uninitialized
					vector<std::string> tmp;
					{
						tmp=currentValue;
						const	vector<string>	&temp=propStringString->StringArray;

						//	Check if we have all previous values.
						for (uint tmpInd=0;tmpInd<tmp.size();tmpInd++)
						{
							if (tmp[tmpInd].find(DIFFERENT_VALUE_MULTI_STRING)!=string::npos)	//	Already tagged as diff.
								continue;

							uint tempInd;
							for (tempInd=0;tempInd<temp.size();tempInd++)
							{
								if (tmp[tmpInd].find(temp[tempInd])!=string::npos)	//	Match.
									break;
							}
							if (tempInd>=temp.size())	//	Not Found ?
								tmp[tmpInd]=DIFFERENT_VALUE_MULTI_STRING+tmp[tmpInd];	//	Set as different.
						}
						
						for (uint tempInd=0;tempInd<temp.size();tempInd++)
						{
							uint tmpInd;
							for (tmpInd=0;tmpInd<tmp.size();tmpInd++)
							{
								if (tmp[tmpInd].find(temp[tempInd])!=string::npos)	//	Match.
									break;
							}
							if (tmpInd>=tmp.size())	//	Not Found ?
								tmp.push_back(DIFFERENT_VALUE_MULTI_STRING+temp[tempInd]);	//	Set as different.
						}
						
					}
					
					setEditTextMultiLine (ListEditBox, tmp);
					updateList ();
					MultipleValues = true;
				}
				else
				{
					// Set as filled
					Set = true;

					// Add the string
					setEditTextMultiLine (ListEditBox, propStringString->StringArray);
					updateList ();
				}
			}
			else
			{
				// Add the strings
				vector<std::string> result;
			 	Parameter.getDefaultValue (result, primitive, *primitiveClass, &FromWhere);
					
				setEditTextMultiLine (ListEditBox, result);
				updateList ();

				// Set as filled
				Set= true;
			}
		}
		else
		{
			// Already sets ?
			if (propString)
			{
				// Get the value
				string currentValue;
				getValue (currentValue);

				// Multi value ?
				if (Set && (((currentValue != propString->String) && (!Default || !propString->Default)) || (Default != propString->Default)))
				{
					// Not by default
					Default = false;

					// Boolean ?
					switch (Parameter.Type)
					{
					case  CPrimitiveClass::CParameter::Boolean:
						// Set as uninitialized
						CheckBox.SetCheck (2);
						updateBoolean ();
						break;
					case  CPrimitiveClass::CParameter::ConstString:
						// Set as uninitialized
						if (!Parameter.SortEntries)
							ComboBox.InsertString (-1, DIFFERENT_VALUE_STRING);
						else
							ComboBox.AddString( DIFFERENT_VALUE_STRING);
						ComboBox.SelectString(-1 ,DIFFERENT_VALUE_STRING);
						OriginalString = DIFFERENT_VALUE_STRING;
						updateCombo ();
						break;
					case  CPrimitiveClass::CParameter::String:
						// Set as uninitialized
						EditBox.SetWindowText (DIFFERENT_VALUE_STRING);
						break;
					}
					MultipleValues = true;
				}
				else
				{
					// Set as filled
					Set= true;

					// Boolean ?
					switch (Parameter.Type)
					{
					case  CPrimitiveClass::CParameter::Boolean:
						// Check the box
						CheckBox.SetCheck (Default?2:(propString->String == "true")?1:0);
						updateBoolean ();
						break;
					case  CPrimitiveClass::CParameter::ConstString:
						if (Parameter.Editable || ComboBox.SelectString(-1 ,propString->String.c_str ()) == CB_ERR)
						{
							ComboBox.SetWindowText(propString->String.c_str ());
							ComboBox.InsertString( -1, propString->String.c_str());
							ComboBox.SelectString(-1 ,propString->String.c_str ());
						}
						OriginalString = propString->String.c_str();
						updateCombo ();
						break;
					case  CPrimitiveClass::CParameter::String:
						{
							bool backupDefault = Default;
							Initializing = true;
							setWindowTextUTF8 (EditBox, propString->String.c_str ());
							Default = backupDefault;
							Initializing = false;
						}
						break;
					}
				}
			}
			else
			{
				if (primitiveClass)
				{
					// Boolean ?
					std::string result;
					Parameter.getDefaultValue (result, primitive, *primitiveClass, &FromWhere);
					if (!result.empty())
					{
						switch (Parameter.Type)
						{
						case  CPrimitiveClass::CParameter::Boolean:
							// Check the box
							CheckBox.SetCheck (2);
							updateBoolean ();
							break;
						case  CPrimitiveClass::CParameter::ConstString:
							ComboBox.SelectString(-1 ,result.c_str ());
							OriginalString = result.c_str();
							updateCombo ();
							break;
						case  CPrimitiveClass::CParameter::String:
							{
								setWindowTextUTF8 (EditBox, result.c_str ());
								Default = true;
							}
							break;
						}
					}
				}

				// Set as filled
				Set= true;
			}
		}

		// Done
		setStaticName();
		return true;
	}
	return false;
}

// ***************************************************************************

bool CDialogProperties::CWidget::toParameter (const CDatabaseLocatorPointer &locator, const CPrimitiveClass::CParameter &parameter)
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Set the parameter
	if (Parameter == parameter)
	{
		// Widget ok ?
		if (Set && (!MultipleValues))
		{
			// What type ?
			if ( (parameter.Type == CPrimitiveClass::CParameter::StringArray) || (parameter.Type == CPrimitiveClass::CParameter::ConstStringArray) 
				||	(parameter.Type == CPrimitiveClass::CParameter::ConstStringArray) )
			{
				// Get the value
				vector<string> value;
				/* todo hulud remove if (Default)
					value.clear ();
				else*/
					getValue (value);

				if (parameter.Type == CPrimitiveClass::CParameter::ConstStringArray)
				{
					//	Get Current Value.
					const IProperty	*prop;
					vector<string>	newValues;
					bool	haveToUpdate=false;
					
					if	(locator.Primitive->getPropertyByName (parameter.Name.c_str (), prop))
					{
						// Dynamic cast
						const CPropertyStringArray *const	propStringArray = dynamic_cast<const CPropertyStringArray *> (prop);
						nlassert (propStringArray);
						newValues=propStringArray->StringArray;
					}

					//	have to remove some string ?
					//	on parse las valeur de la primitive
					for (int newValueInd=0;newValueInd<(int)newValues.size();newValueInd++)
					{
						uint valueInd;
						// On check si elles existent dans la dialog
						for (valueInd=0;valueInd<value.size();valueInd++)
						{
							if (value[valueInd].find(DIFFERENT_VALUE_MULTI_STRING)==string::npos)	//	not diff.
							{
								if (value[valueInd]==newValues[newValueInd])	//	found !
									break;
							}
							else
							{
								if (value[valueInd].find(newValues[newValueInd])!=string::npos)	//	found !
									break;
							}

						}
						//	si elle n'existent pas, on les remove.
						if (valueInd>=value.size())	//	not found -> remove it
						{
							haveToUpdate=true;
							newValues.erase(newValues.begin()+newValueInd);
							newValueInd--;							
						}
												
					}
					
					//	have to add some string ?
					//	on parse les valeurs de la dialog
					std::vector<std::string>::iterator	insertIt=newValues.begin();
					for (uint valueInd=0;valueInd<value.size();valueInd++)
					{
						//	si elle n'existe pas on la rajoute.
						if (value[valueInd].find(DIFFERENT_VALUE_MULTI_STRING)==string::npos)	//	not diff.
						{
							std::vector<std::string>::iterator	foundIt=find(newValues.begin(), newValues.end(), value[valueInd]);
							if	(foundIt==newValues.end())
							{
								haveToUpdate=true;
								if (insertIt==newValues.end())
								{
									newValues.push_back(value[valueInd]);
									insertIt=newValues.begin();
								}
								else
								{
									insertIt=newValues.insert(insertIt+1, value[valueInd]);	// .push_back(value[valueInd]);
								}

							}
							else
							{
								insertIt=foundIt;
							}

						}

					}
					if (haveToUpdate)
						doc->addModification (new CActionSetPrimitivePropertyStringArray (locator, parameter.Name.c_str (), newValues, Default));
				}

				if (parameter.Type == CPrimitiveClass::CParameter::StringArray)
				{
					// Set the value
					if	(	value.empty()
						||	(value[0] != DIFFERENT_VALUE_STRING))
					{
						doc->addModification (new CActionSetPrimitivePropertyStringArray (locator, parameter.Name.c_str (), value, Default));
					}
					else
					{
						if (value.size()>1)	//	More than DIFFERENT_VALUE_STRING
						{
							//	Get Current Value.
							const IProperty *prop;
							vector<string> newValues;

							if	(locator.Primitive->getPropertyByName (parameter.Name.c_str (), prop))
							{
								// Dynamic cast
								const CPropertyStringArray *const	propStringArray = dynamic_cast<const CPropertyStringArray *> (prop);
								nlassert (propStringArray);
								newValues=propStringArray->StringArray;
							}

							for (size_t valIndex=1;valIndex<value.size();valIndex++)	//	ALL Except DIFFERENT_VALUE_STRING
							{
								if (std::find(newValues.begin(), newValues.end(), value[valIndex])==newValues.end())
									newValues.push_back(value[valIndex]);
							}
							doc->addModification (new CActionSetPrimitivePropertyStringArray (locator, parameter.Name.c_str (), newValues, Default));
						}

					}

				}

			}
			else
			{
				// Get the value
				string value;
				/* todo hulud remove if (Default)
					value = "";
				else*/
					getValue (value);

				// Set the value
				if (value != DIFFERENT_VALUE_STRING)
					doc->addModification (new CActionSetPrimitivePropertyString (locator, parameter.Name.c_str (), value.c_str (), Default));
			}
		}
		return true;
	}
	return false;
}

// ***************************************************************************

void CDialogProperties::CWidget::getValue (std::string &result) const
{
	nlassert (Parameter.Type != CPrimitiveClass::CParameter::StringArray);

	// Check box ?
	if (Parameter.Type == CPrimitiveClass::CParameter::Boolean)
	{
		// Get the value
		if (CheckBox.GetCheck() == 1)
			result = "true";
		else if (CheckBox.GetCheck() == 0)
			result = "false";
		else
			result = "";
	}
	else if (Parameter.Type == CPrimitiveClass::CParameter::ConstString)
	{
		// Get the text
		CString str;
		getWindowTextUTF8 (ComboBox, str);
		result = (const char*)str;
	}
	else
	{
		nlassert (Parameter.Type == CPrimitiveClass::CParameter::String);

		// Get the text
		CString str;
		getWindowTextUTF8 (EditBox, str);
		result = (const char*)str;
	}
}

// ***************************************************************************

void CDialogProperties::CWidget::getValue (std::vector<std::string> &result) const
{
	nlassert (	(Parameter.Type == CPrimitiveClass::CParameter::StringArray) ||
				(Parameter.Type == CPrimitiveClass::CParameter::ConstStringArray) );

	// Get the string
	result.clear ();
	CString str;
	if (Parameter.Type == CPrimitiveClass::CParameter::StringArray)
	{
		getWindowTextUTF8 (MultiLineEditBox, str);
//		MultiLineEditBox.UnloadText(str);
		const char *strP = str;
		string dst;
		while (*strP)
		{
			// New line ?
			if (*strP == '\n')
			{
				result.push_back (dst);
				dst = "";
			}
			else if (*strP != '\r')
			{
				dst += *strP;
			}
			strP++;
		}
		if (!dst.empty())
			result.push_back (dst);
	}
	else
	{
		uint i;
		const uint size = ListEditBox.GetCount();
		result.resize (size);
		for (i=0; i<size; i++)
		{
			CString str;
			ListEditBox.GetText( i, str);
			result[i] = (const char*)str;
		}
	}
}

// ***************************************************************************

void CDialogProperties::CWidget::setFocus()
{
	if (IsWindow (Static.m_hWnd))
	{
		Static.SetFocus();
	}
	if (IsWindow (ComboBox.m_hWnd))
	{
		ComboBox.SetFocus();
	}
	if (IsWindow (CheckBox.m_hWnd))
	{
		CheckBox.SetFocus();
	}
	if (IsWindow (EditBox.m_hWnd))
	{
		EditBox.SetFocus();
	}
	if (IsWindow (MultiLineEditBox.m_hWnd))
	{
		MultiLineEditBox.SetFocus();
	}
	if (IsWindow (ListEditBox.m_hWnd))
	{
		ListEditBox.SetFocus();
	}
}

// ***************************************************************************

void CDialogProperties::CWidget::updateBoolean ()
{
	// No multiple value
	MultipleValues = false;
	Set = true;
	
	// Return to default ?
	/* todo hulud remove
	int oldValue = CheckBox.GetCheck();*/

	// Default value ?
	if (Default)
	{
		string value;
		DialogProperties->setDefaultValue (this, value);
		CheckBox.SetCheck(((!value.empty())&&(value=="true"))?1:0);
	}

/* todo hulud remove
	if (oldValue != CheckBox.GetCheck())
		ApplyAutoname(widget);*/
	setStaticName();
}

// ***************************************************************************

void CDialogProperties::CWidget::updateCombo ()
{
	// No multiple value
	MultipleValues = false;
	Set = true;

	// Get the default value
	if (Default)
	{
		string value;
		DialogProperties->setDefaultValue (this, value);
		if (value != "")
		{
			int index = ComboBox.FindString (-1, value.c_str());
			if (index != CB_ERR)
				ComboBox.SetCurSel (index);
		}
	}

	/* todo hulud remove
	if (curSel != ComboBox.GetCurSel())
		ApplyAutoname(widget);*/
	setStaticName();
}

// ***************************************************************************

void CDialogProperties::CWidget::updateMultiline ()
{
	// No multiple value
	MultipleValues = false;
	Set = true;

	// Default ?
	if (Default)
	{
		vector<string> vectString;
		DialogProperties->setDefaultValue (this, vectString);
		if (!vectString.empty())
		{
			// Add the string
			string temp;
			uint i;
			for (i=0; i<vectString.size(); i++)
			{
				temp += vectString[i];
				if (i != (vectString.size()-1))
					temp += '\n';
			}
			setEditTextMultiLine (MultiLineEditBox, temp.c_str());
		}
	}
	setStaticName();
}

// ***************************************************************************

void CDialogProperties::CWidget::updateList ()
{
	// No multiple value
	MultipleValues = false;
	Set = true;

	// Default ?
	if (Default)
	{
		vector<string> vectString;
		DialogProperties->setDefaultValue (this, vectString);
		if (!vectString.empty())
		{
			setEditTextMultiLine (ListEditBox, vectString);
		}
	}
	setStaticName();
}

// ***************************************************************************

bool CDialogProperties::isModified()
{
	list<CWidget>::iterator iteWid;

	for (iteWid=Widgets.begin();iteWid!=Widgets.end();iteWid++)
	{
		if ((*iteWid).Modified)
		{
			return true;
		}
		// special case for editable combo box
		if ((*iteWid).Parameter.Type == CPrimitiveClass::CParameter::ConstString)
		{
			CString	text;
			getWindowTextUTF8 ((*iteWid).ComboBox, text);
			if ((*iteWid).OriginalString.c_str() != text)
			{
				return true;
			}
		}
	}
	return false;
}

// ***************************************************************************



void CDialogProperties::CWidget::getFilename (string &result)
{
	if (Parameter.Type == CPrimitiveClass::CParameter::ConstStringArray)
	{
		int sel = ListEditBox.GetCurSel();
		if (sel != LB_ERR)
		{
			CString cstr;
			ListEditBox.GetText (sel, cstr);
			result = (const char*)cstr;
		}
		else
			result = "";
	}
	else
	{
		getValue (result);
	}
}

// ***************************************************************************

// return true if the given hwnd is one of this widget
bool CDialogProperties::CWidget::isHwndMatch(HWND hWnd)
{
	if (IsWindow (Static.m_hWnd) && Static.m_hWnd == hWnd)
		return true;
	if (IsWindow (ComboBox.m_hWnd) && ComboBox.m_hWnd == hWnd)
		return true;
	if (IsWindow (CheckBox.m_hWnd) && CheckBox.m_hWnd == hWnd)
		return true;
	if (IsWindow (EditBox.m_hWnd) && EditBox.m_hWnd == hWnd)
		return true;
	if (IsWindow (MultiLineEditBox.m_hWnd) && MultiLineEditBox.m_hWnd == hWnd)
		return true;
	if (IsWindow (ListEditBox.m_hWnd) && ListEditBox.m_hWnd == hWnd)
		return true;

	return false;
}

// ***************************************************************************

// get the next window
HWND CDialogProperties::CWidget::getNextWindow(HWND hWnd)
{
	if (hWnd == 0)
	{
		if (IsWindow (ComboBox.m_hWnd))
			return ComboBox.m_hWnd;
		if (IsWindow (EditBox.m_hWnd))
			return EditBox.m_hWnd;
		if (IsWindow (MultiLineEditBox.m_hWnd))
			return MultiLineEditBox.m_hWnd;
		if (IsWindow (ListEditBox.m_hWnd))
			return ListEditBox.m_hWnd;
		if (IsWindow (CheckBox.m_hWnd))
			return CheckBox.m_hWnd;
	}
	else
	{
		if (IsWindow (ComboBox.m_hWnd) && ComboBox.m_hWnd == hWnd && IsWindow (CheckBox.m_hWnd))
			return CheckBox.m_hWnd;
		if (IsWindow (EditBox.m_hWnd) && EditBox.m_hWnd == hWnd && IsWindow (CheckBox.m_hWnd))
			return CheckBox.m_hWnd;
		if (IsWindow (MultiLineEditBox.m_hWnd) && MultiLineEditBox.m_hWnd == hWnd && IsWindow (CheckBox.m_hWnd))
			return CheckBox.m_hWnd;
		if (IsWindow (ListEditBox.m_hWnd) && ListEditBox.m_hWnd == hWnd && IsWindow (CheckBox.m_hWnd))
			return CheckBox.m_hWnd;
	}

	return 0;
}

// ***************************************************************************

// get the previous window
HWND CDialogProperties::CWidget::getPreviousWindow(HWND hWnd)
{
	if (hWnd == 0)
	{
		if (IsWindow (CheckBox.m_hWnd))
			return CheckBox.m_hWnd;
		if (IsWindow (ListEditBox.m_hWnd))
			return ListEditBox.m_hWnd;
		if (IsWindow (MultiLineEditBox.m_hWnd))
			return MultiLineEditBox.m_hWnd;
		if (IsWindow (EditBox.m_hWnd))
			return EditBox.m_hWnd;
		if (IsWindow (ComboBox.m_hWnd))
			return ComboBox.m_hWnd;
	}
	else
	{
		if (IsWindow (CheckBox.m_hWnd) && CheckBox.m_hWnd == hWnd)
		{
			if (IsWindow (ComboBox.m_hWnd))
				return ComboBox.m_hWnd;
			if (IsWindow (EditBox.m_hWnd))
				return EditBox.m_hWnd;
			if (IsWindow (MultiLineEditBox.m_hWnd))
				return MultiLineEditBox.m_hWnd;
			if (IsWindow (ListEditBox.m_hWnd))
				return ListEditBox.m_hWnd;
		}
	}

	return 0;
}


// ***************************************************************************

bool CDialogProperties::CWidget::OnSize( int cx, int cy, int decY )
{
	RECT rect;
	bool sizeY = false;
	
	if ( Static.m_hWnd )
	{
		Static.GetWindowRect( &rect );
		DialogProperties->m_PropertyFrame.ScreenToClient( &rect );
		rect.top += decY;
		rect.bottom += decY;
		Static.MoveWindow( &rect );
	}
	
	if ( ComboBox.m_hWnd && IsWindow( ComboBox.m_hWnd ) )
	{
		ComboBox.GetWindowRect( &rect );
		DialogProperties->m_PropertyFrame.ScreenToClient( &rect );
		rect.top += decY;
		rect.bottom += decY;
		rect.right += cx;
		ComboBox.MoveWindow( &rect );
	}
	
	if ( EditBox.m_hWnd && IsWindow( EditBox.m_hWnd ) )
	{
		EditBox.GetWindowRect( &rect );
		DialogProperties->m_PropertyFrame.ScreenToClient( &rect );
		rect.top += decY;
		rect.bottom += decY;
		rect.right += cx;
		EditBox.MoveWindow( &rect );
	}
	
	if ( MultiLineEditBox.m_hWnd && IsWindow( MultiLineEditBox.m_hWnd ) )
	{
		MultiLineEditBox.GetWindowRect( &rect );
		DialogProperties->m_PropertyFrame.ScreenToClient( &rect );
		rect.right += cx;
		rect.top += decY;
		rect.bottom += decY;
		if ( rect.bottom + cy >= rect.top + 100 ) 
		{
			::CRect dlgRect;
			DialogProperties->GetWindowRect( &dlgRect );

			if ( ( cy <= 0 ) || ( dlgRect.Height() > DialogProperties->getOriginalHeight() ) )
			{
				rect.bottom += cy; 
				sizeY = true;
				decY += cy;
			}
		}
		MultiLineEditBox.MoveWindow( &rect );
	}
	
	if ( ListEditBox.m_hWnd && IsWindow( ListEditBox.m_hWnd ) )
	{
		ListEditBox.GetWindowRect( &rect );
		DialogProperties->m_PropertyFrame.ScreenToClient( &rect );
		rect.right += cx;
		rect.top += decY;
		rect.bottom += decY;
		ListEditBox.MoveWindow( &rect );
	}

	if ( CheckBox.m_hWnd )
	{
		CheckBox.GetWindowRect( &rect );
		DialogProperties->m_PropertyFrame.ScreenToClient( &rect );
		if ( rect.left > 30 )
		{
			rect.left += cx;
			rect.right += cx;
		}
		
		CString text;
		CheckBox.GetWindowText( text );
		if (  text == "Edit..." )
		{
			rect.right += cx;
		}
		
		rect.top += decY;
		rect.bottom += decY;
		CheckBox.MoveWindow( &rect );
	}

	return sizeY;
}

// ***************************************************************************

// ***************************************************************************

BOOL CDialogProperties::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD wNotifyCode = HIWORD(wParam); // notification code 

	switch (wNotifyCode)
	{
	case BN_CLICKED:
		{
			// Button id
			int idButton = (int) LOWORD(wParam);    // identifier of button 
			if (idButton >= FIRST_WIDGET)
			{
				// Get the widget
				CWidget *widget = getWidget (idButton-FIRST_WIDGET);
				
				// Check box ?
				if (widget->Parameter.Type == CPrimitiveClass::CParameter::Boolean)
				{
					if (widget->Default)
						widget->CheckBox.SetCheck(0);
					else
						widget->CheckBox.SetCheck((widget->CheckBox.GetCheck()+1)%3);

					// If 2 set as default
					widget->Default = widget->CheckBox.GetCheck() == 2;

					widget->updateBoolean ();
					widget->Modified = true;
				}
				else if (widget->Parameter.Type == CPrimitiveClass::CParameter::String)
				{
					// we open a file selector
					nlassert (widget->Parameter.FileExtension != "");

					// Create a dialog file, starting at the specified Folder
					/* todo hulud remove
					CString oldValue;
					widget->EditBox.GetWindowText (oldValue);*/
					CFileDialogEx dialog (BASE_REGISTRY_KEY, "default", TRUE, widget->Parameter.FileExtension.c_str (), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 
						(widget->Parameter.FileExtension+" (*."+widget->Parameter.FileExtension+")|*."+widget->Parameter.FileExtension+"|All Files (*.*)|*.*||").c_str (), getMainFrame ());
					if (widget->Parameter.Folder != "")
					{
						dialog.m_ofn.lpstrInitialDir = widget->Parameter.Folder.c_str();
					}
					if (dialog.DoModal() == IDOK)
					{
						CString str;
						str = dialog.GetFileTitle();
						setWindowTextUTF8 (widget->EditBox, str);

			/* todo hulud remove
						if ((const char*)oldValue != str)
							ApplyAutoname(widget);*/
						widget->setStaticName();
						widget->Modified = true;
					}
				}
				else if (widget->Parameter.Type == CPrimitiveClass::CParameter::StringArray)
				{
					if (widget->Parameter.FileExtension.empty())
					{
						nlassert (widget->Parameter.Folder != "");
						SelectFolder(widget);
					}
					else
					{
						// Edit with an external editor
						std::vector<std::string> result;
						widget->getValue (result);
						uint i;
						string text = "";
						for (i=0; i<result.size(); i++)
						{
							text += result[i];
							if (i+1<result.size())
								text += "\n";
						}
						CConfigFile::CVar *var = getMainFrame ()->getConfigFile().getVarPtr ("TextEditor");
						char windows[512];
						GetWindowsDirectory (windows, sizeof (windows));
						if (EditExternalText (var?var->asString():windows+string ("/notepad.exe"), text, widget->Parameter.FileExtension.c_str ()))
						{
							widget->Default = false;
							widget->Modified = true;
							widget->updateMultiline();
							setEditTextMultiLine (widget->MultiLineEditBox, text.c_str());
						}
					}
				}
				else
				{
					// Should be a filename
					nlassert (widget->Parameter.Filename);

					// No multiple values ?
					if (!widget->MultipleValues)
					{
						// Get the string
						string filename;
						widget->getFilename (filename);

						// Not empty ?
						if (!filename.empty ())
						{
							// Open the file
							if (!widget->Parameter.FileExtension.empty ())
								filename += "."+widget->Parameter.FileExtension;

							// Lookup
							if (widget->Parameter.Lookup)
								filename = CPath::lookup (filename, false, false, false);
							if (!filename.empty ())
							{
								// Open the file
								if (!openFile (filename.c_str ()))
									// Error
									theApp.errorMessage ("Can't open the file %s", filename.c_str ());
							}
							else
							{
								// Error
								widget->getFilename (filename);
								theApp.errorMessage ("Can't find the file %s", filename.c_str ());
							}
						}
					}
				}
			}
		}
		break;
	case CBN_SELCHANGE:
		{
			int idComboBox = (int) LOWORD(wParam);  // identifier of combo box 
			nlassert ((idComboBox >= FIRST_WIDGET) || (idComboBox==STRING_SELECT_COMBOBOX_ID));

			CWidget *widget = getWidget (idComboBox-FIRST_WIDGET);
			if (widget->Parameter.Type == CPrimitiveClass::CParameter::ConstString)
			{
				// Get the widget
				int curSel = widget->ComboBox.GetCurSel();

				// Check box ?
				nlassert (widget->Parameter.Type == CPrimitiveClass::CParameter::ConstString);

				// Remove
				int index = widget->ComboBox.FindString (-1, DIFFERENT_VALUE_STRING);
				if (index != CB_ERR)
					widget->ComboBox.DeleteString(index);

				// Default value ?
				widget->Default = (widget->ComboBox.GetCurSel () == 0);
				widget->updateCombo ();
				widget->Modified = true;
			}
		}
		break;
	case EN_CHANGE:
		{
			int idEditCtrl = (int) LOWORD(wParam); // identifier of edit control 
			if ((idEditCtrl >= FIRST_WIDGET) && (idEditCtrl <= LAST_WIDGET))
			{
				// Get the widget
				CWidget *widget = getWidget (idEditCtrl-FIRST_WIDGET);

				// Check box ?
				if (widget->Parameter.Type == CPrimitiveClass::CParameter::String)
				{
					// No multiple value
					widget->MultipleValues = false;
					if (!widget->Initializing)
						widget->Modified = true;
					widget->Set = true;

					/* todo hulud remove
					CString oldValue;
					widget->EditBox.GetWindowText (oldValue);*/

					// String NULL ?
					CString text;
					getWindowTextUTF8 (widget->EditBox, text);
					widget->Default = (text == "");

					// Default ?
					if (widget->Default)
					{
						string value;
						setDefaultValue (widget, value);
						if (!value.empty())
						{
							setWindowTextUTF8 (widget->EditBox, value.c_str());
							widget->Default = true;
						}
					}
					else if ( text == DIFFERENT_VALUE_STRING )
						widget->Modified = false;

			/* todo hulud remove
					CString newValue;
					widget->EditBox.GetWindowText (newValue);
					if (newValue != oldValue)
						ApplyAutoname(widget);*/

					// No more multiple value
					widget->setStaticName();
				}
				else if (widget->Parameter.Type == CPrimitiveClass::CParameter::StringArray)
				{
					nlassert (widget->Parameter.Type == CPrimitiveClass::CParameter::StringArray);

					// String NULL ?
					std::vector<std::string> result;
					widget->getValue (result);
					widget->Default = result.empty();
					widget->updateMultiline ();
					widget->Modified = true;
				}
				/*else
				{
					nlassert (widget->Parameter.Type == CPrimitiveClass::CParameter::ConstStringArray);

					// String NULL ?
					std::vector<std::string> result;
					widget->getValue (result);
					widget->Default = result.empty();
					widget->updateList ();
				}*/
			}
		}
		break;
 	}
	
	return CDialog::OnCommand(wParam, lParam);
}

// ***************************************************************************

CDialogProperties::CWidget *CDialogProperties::getWidget (uint widgetId)
{
	std::list<CWidget>::iterator ite = Widgets.begin ();
	while (widgetId > 0)
	{
		widgetId--;
		ite++;
	}
	return &(*ite);
}

// ***************************************************************************

void CDialogProperties::changeSelection (std::list<NLLIGO::IPrimitive*> &locators)
{
	// Print a message if changes will be lost
	if (isModified() && !getMainFrame()->yesNoMessage("All changes will be lost ! Are you sure ?"))
		return;

	_PropDlgLocators.clear ();
	CWorldEditorDoc *doc = getDocument ();
	std::list<NLLIGO::IPrimitive*>::iterator ite = locators.begin ();
	while (ite != locators.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);
		_PropDlgLocators.push_back (locator);
		ite++;
	}

	// Shown ?
	if (IsWindowVisible ())
	{
		rebuildDialog ();
	}
}

// ***************************************************************************
bool CDialogProperties::containsSelection( std::list<NLLIGO::IPrimitive*> &locators )
{
	std::list<NLLIGO::IPrimitive*>::iterator ite = locators.begin ();
	while (ite != locators.end ())
	{
		std::vector<CDatabaseLocatorPointer>::iterator itDB = _PropDlgLocators.begin();

		bool ok = false;
		while ( !ok && ( itDB != _PropDlgLocators.end() ) )
		{
			ok = ( (*itDB).Primitive == (*ite) );
			itDB++;
		}

		if ( !ok )
			return false;

		ite++;
	}

	return true;
}

// ***************************************************************************
bool CDialogProperties::containsSelection( std::vector<CDatabaseLocatorPointer> &locators )
{
	std::vector<CDatabaseLocatorPointer>::iterator ite = locators.begin ();
	while (ite != locators.end ())
	{
		std::vector<CDatabaseLocatorPointer>::iterator itDB = _PropDlgLocators.begin();

		bool ok = false;
		while ( !ok && ( itDB != _PropDlgLocators.end() ) )
		{
			ok = ( (*itDB) == (*ite) );
			itDB++;
		}

		if ( !ok )
			return false;

		ite++;
	}

	return true;
}

// ***************************************************************************

bool CDialogProperties::equalsSelection( std::list<NLLIGO::IPrimitive*> &locators )
{
	if ( locators.size() != _PropDlgLocators.size() )
		return false;

	return containsSelection( locators );
}

// ***************************************************************************

void CDialogProperties::rebuildDialog ()
{
	// Remove widgets
	removeWidgets ();

	// Something selected ?
	const uint selSize = _PropDlgLocators.size ();

	// The parameter list
	set<CPrimitiveClass::CParameter>	parameterList;

	// Does at least one of the selected primitive is a static child ?
	bool staticChildSelected = false;

	// For each selected primitive
	uint i;
	for (i=0; i<selSize; i++)
	{
		// Primitive ?
		if(const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->getParent()==NULL)
		{
			
			/*const CPrimitiveClass::CParameter *tac=&primClass->Parameters[0];
			string sou=tac->DefaultValue[0].Name;
			sou="test";*/
			const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->removePropertyByName("name");
			const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->removePropertyByName("path");
			//TODO  faire une fonction dans CWorldDoc pour recup m_strPathName
			string name;
			getDocument()->getPrimitiveDisplayName(name,_PropDlgLocators[i].getDatabaseIndex());
			string path;
			getDocument()->getFilePath(_PropDlgLocators[i].getDatabaseIndex(),path);

			const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->addPropertyByName("name",new CPropertyString (name));
			const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->addPropertyByName("path",new CPropertyString (path));
		}

		const IPrimitive * primitive= _PropDlgLocators[i].Primitive;

		if (primitive)
		{
			staticChildSelected |= theApp.Config.isStaticChild (*primitive);

			// Get the class name
			const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*primitive);

			// Use the class or not ?
			if (primClass)
			{
				// For each properties of the class
				for (uint p=0; p<primClass->Parameters.size (); p++)
				{
					// Is the parameter visible ?
					if (primClass->Parameters[p].Visible)
					{
						parameterList.insert (primClass->Parameters[p]);
					}
				}
			}
			else
			{
				// For each primitive property
				uint numProp = primitive->getNumProperty ();
				for (uint p=0; p<numProp; p++)
				{
					// Get the property
					std::string		propertyName;
					const IProperty *prop;
					nlverify (primitive->getProperty (p, propertyName, prop));

					// Add a default property
					CPrimitiveClass::CParameter defProp (*prop, propertyName.c_str ());
					parameterList.insert (defProp);
				}
			}
		}
	}
	
	// Remove property Name, Hidden and Selected
	set<CPrimitiveClass::CParameter>::iterator ite = parameterList.begin ();
	while (ite != parameterList.end ())
	{
		// Next iterator
		set<CPrimitiveClass::CParameter>::iterator next = ite;
		next++;

		// Property name ?
		if (/*(ite->Name == "name") || *//*(ite->Name == "hidden")*/ /*|| (ite->Name == "selected") ||*/ (ite->Name == "class"))
		{
			// Remove it
			parameterList.erase (ite);
		}

		ite = next;
	}

	// Add the default parameter
	CPrimitiveClass::CParameter defaultParameter;
	defaultParameter.Visible = true;
	defaultParameter.Filename = false;

	// The name
	RECT widgetPos = WidgetPos;


	// Add the other widgets
	//for (i=0;i<3;++i) //JC: a small debug trick to artifically simulate a huge DialogBox
	{
		// Add first the name
		ite = parameterList.begin ();
		while (ite != parameterList.end ())
		{
			if (ite->Name == "name")
			{
				addWidget (*ite, widgetPos, staticChildSelected);
				break;
			}
			ite++;
		}

		// Then add the other one
		ite = parameterList.begin ();
		while (ite != parameterList.end ())
		{
			if (ite->Name != "name")
				addWidget (*ite, widgetPos, false);
			
			ite++;
		}
	}

	std::string windowName;

	// For each selected primitive
	for (i=0; i<selSize; i++)
	{
		// Primitive ?
		const IPrimitive *primitive = _PropDlgLocators[i].Primitive;
		if (primitive)
		{
			// Get the class name
			const  CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*primitive);

			// Use the class or not ?
			if (primClass)
			{
				if (windowName.empty())
					windowName = primClass->Name;
				else if (primClass->Name != windowName)
					windowName = "<different class>";
				
				// Get the property for name
				/*const IProperty *propName;
				if (primitive->getPropertyByName ("name", propName))
				{
					// Create a default property
					CPrimitiveClass::CParameter defProp (*propName, "name");
			
					// Add the property in the good widget
					std::list<CWidget>::iterator ite = Widgets.begin ();
					while (ite != Widgets.end ())
					{
						// Set the parameter
						if (ite->fromParameter (propName, *primitive, primClass, defProp))
							break;

						ite++;
					}
				}*/

				// For each properties of the class
				for (uint p=0; p<primClass->Parameters.size (); p++)
				{
					// Is the parameter visible ?
					if (primClass->Parameters[p].Visible)
					{
						// Get the property
						const IProperty *prop;
						if (primitive->getPropertyByName (primClass->Parameters[p].Name.c_str(), prop))
						{
							// Add the property in the good widget
							std::list<CWidget>::iterator ite = Widgets.begin ();
							while (ite != Widgets.end ())
							{
								// Set the parameter
								if (ite->fromParameter (prop, *primitive, primClass, primClass->Parameters[p]))
									break;

								ite++;
							}
						}
						else
						{
							// Add the property in the good widget
							std::list<CWidget>::iterator ite = Widgets.begin ();
							while (ite != Widgets.end ())
							{
								// Set the parameter
								if (ite->fromParameter (NULL, *primitive, primClass, primClass->Parameters[p]))
									break;

								ite++;
							}
						}
					}
				}
			}
			else
			{
				// For each primitive property
				uint numProp = primitive->getNumProperty ();
				for (uint p=0; p<numProp; p++)
				{
					// Get the property
					std::string		propertyName;
					const IProperty *prop;
					nlverify (primitive->getProperty (p, propertyName, prop));

					// Create a default property
					CPrimitiveClass::CParameter defProp (*prop, propertyName.c_str ());
					
					// Add the property in the good widget
					std::list<CWidget>::iterator ite = Widgets.begin ();
					while (ite != Widgets.end ())
					{
						// Set the parameter
						if (ite->fromParameter (prop, *primitive, NULL, defProp))
							break;

						ite++;
					}
				}
			}
		}
	}
 
	// resize the dialog and sub control to the max or min extend possible
	{
		// force the cont to it's minimum size before other resizing code
		{
			RECT contRect;
			m_PropertyCont.GetWindowRect(&contRect);
			m_PropertyFrame.ScreenToClient(&contRect);
			contRect.bottom = max(int(contRect.bottom), m_ContMinHeight);
			contRect.right = contRect.left + 425;
			m_PropertyCont.MoveWindow(&contRect);
		}
		// first size to ideal size
		RECT desktop;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &desktop, 0);

		// resize frame cont
		::CRect contRect;
		m_PropertyCont.GetWindowRect(&contRect);
		m_PropertyFrame.ScreenToClient(&contRect);
		contRect.bottom += -contRect.top;
		contRect.bottom = min(int(contRect.bottom), m_MaxFrameHeight);
		contRect.top = 0;
		AdjustWindowRectEx(&contRect,
			m_PropertyFrame.GetStyle(), 
			false,
			m_PropertyFrame.GetExStyle());
//		m_PropertyFrame.CalcWindowRect(&contRect);
		int height = contRect.Height();
		m_PropertyFrame.MoveWindow(PropertyFrameRect.top, 
									PropertyFrameRect.left, 
									445,//PropertyFrameRect.right-PropertyFrameRect.left,
									height);
		m_PropertyFrame.GetClientRect(&contRect);

		// resize dialog
		RECT dialogRect;
		GetWindowRect(&dialogRect);
		ScreenToClient(&dialogRect);
		m_PropertyFrame.GetWindowRect(&contRect);
		ScreenToClient(&contRect);
		dialogRect.bottom = contRect.bottom + m_FrameBottomSpace;
		ClientToScreen(&dialogRect);
		dialogRect.right = dialogRect.left + 455;
		m_curDlgRect = dialogRect;
		m_originalHeight = m_curDlgRect.Height();
		MoveWindow(&dialogRect);

		// replace the dialog in available window place
		GetWindowRect(&dialogRect);
		if (dialogRect.top < desktop.top)
		{
			dialogRect.bottom = desktop.top + (dialogRect.bottom - dialogRect.top);
			dialogRect.top = desktop.top;
		}
		if (dialogRect.bottom > desktop.bottom)
		{
			dialogRect.top = desktop.bottom - (dialogRect.bottom - dialogRect.top);
			dialogRect.bottom = desktop.bottom;
		}
		

		MoveWindow(&dialogRect);
		m_DialogWidth = dialogRect.right - dialogRect.left;

/*		// now, size to physical available size and minimum size
		int height = dialogRect.bottom - dialogRect.top;
		int deskHeight = desktop.bottom - desktop.top;
		height = min(height, deskHeight);
		height = max(height, int(PropertyFrameRect.bottom-PropertyFrameRect.top));

		dialogRect.bottom = dialogRect.top + height;

		if (dialogRect.bottom > desktop.bottom)
		{
			dialogRect.top -= dialogRect.bottom-desktop.bottom;
			dialogRect.bottom = desktop.bottom;
		}

		MoveWindow(&dialogRect);
		ScreenToClient(&dialogRect);
		
		// resize frame to desktop size	
		m_PropertyFrame.GetWindowRect(&contRect);
		ScreenToClient(&contRect);
		contRect.bottom = dialogRect.bottom-m_FramePos;
		m_PropertyFrame.MoveWindow(&contRect);
		// resize container window
		m_PropertyCont.GetWindowRect(&contRect);
		m_PropertyFrame.ScreenToClient(&contRect);



		if (contRect.bottom-contRect.top < m_ContMinHeight)
			contRect.bottom = contRect.top + m_ContMinHeight;

		m_PropertyCont.MoveWindow(&contRect);
*/
	}
	
	// replace the 3 button
	RECT dialogRect;
	GetClientRect(&dialogRect);
	RECT buttonRect;
	// OK
	GetDlgItem(IDOK)->GetWindowRect(&buttonRect);
	ScreenToClient(&buttonRect);
	buttonRect.bottom = (dialogRect.bottom - m_ButtonPos) + (buttonRect.bottom-buttonRect.top);
	buttonRect.top = dialogRect.bottom - m_ButtonPos;
	GetDlgItem(IDOK)->MoveWindow(&buttonRect);
	// cancel
	GetDlgItem(IDCANCEL)->GetWindowRect(&buttonRect);
	ScreenToClient(&buttonRect);
	buttonRect.bottom = (dialogRect.bottom - m_ButtonPos) + (buttonRect.bottom-buttonRect.top);
	buttonRect.top = dialogRect.bottom - m_ButtonPos;
	GetDlgItem(IDCANCEL)->MoveWindow(&buttonRect);
	//IDUPDATE
	GetDlgItem(IDUPDATE)->GetWindowRect(&buttonRect);
	ScreenToClient(&buttonRect);
	buttonRect.bottom = (dialogRect.bottom - m_ButtonPos) + (buttonRect.bottom-buttonRect.top);
	buttonRect.top = dialogRect.bottom - m_ButtonPos;
	GetDlgItem(IDUPDATE)->MoveWindow(&buttonRect);

	RECT contRect;
	m_PropertyCont.GetWindowRect(&contRect);
	RECT frameRect;
	m_PropertyFrame.GetClientRect(&frameRect);
	// Set the scroll bar value
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE/*SIF_ALL*/|SIF_DISABLENOSCROLL; // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nMin = 0; 
	si.nMax = contRect.bottom - contRect.top;
	si.nPage = frameRect.bottom; 
	int pos = std::min(LONG(m_ScrollBar.GetScrollPos()), std::max(LONG(0), LONG(si.nMax - si.nPage)));
	m_ScrollBar.SetScrollInfo(&si, FALSE);
	// foul MFC by sending a pseudo message
	OnVScroll(SB_THUMBPOSITION, pos, &m_ScrollBar);

	// replace the scroll bar control
	RECT scrollRect;
	m_PropertyFrame.GetClientRect(&scrollRect);
	m_PropertyFrame.ClientToScreen(&scrollRect);
	scrollRect.left = scrollRect.right-16;
	ScreenToClient(&scrollRect);
	m_ScrollBar.MoveWindow(&scrollRect, TRUE);

	// set the name of the dlg according to displayed class
	SetWindowText( ( std::string( "Properties for : " ) + windowName ).c_str() );

//	// JC: added scrolling properties
//	::CRect clientRect;
//	GetClientRect (clientRect);
//	GetWindowRect (m_rect);
//	
//	m_nScrollPos = 0;
//	
//	HWND hWndEntireScreen = ::GetDesktopWindow(); 
//	BOOL bSuccess = ::GetWindowRect(hWndEntireScreen,&rectEntireScreen); 
//	_ScrollBar = false;
//	if (bSuccess != FALSE)
//	{ 
//		RECT	desktop;
//		GetDesktopWindow()->GetClientRect(&desktop);
////		GetDesktopWindow()->ClientToScreen(&desktop);
//
//		int nHeightOfMyDlg = m_rect.Height(); 
//		int nMaxDesiredHeight = desktop.bottom;
//		if (nHeightOfMyDlg >= nMaxDesiredHeight) 
//		{ 
//			// resize to 90% of the entire screen, and vertically center it on the screen
//			MoveWindow(m_rect.left,(int)(0.05*rectEntireScreen.Height()), m_rect.Width(),nMaxDesiredHeight);
//		}
//
//		GetWindowRect (m_rect2);
//			
//		int nScrollMax = clientRect.Height();
//		GetClientRect (clientRect);
//		int nScrollPage = clientRect.Height()+1; // clientRect.Height();
//
//		SCROLLINFO si;
//		si.cbSize = sizeof(SCROLLINFO);
//		si.fMask = SIF_ALL|SIF_DISABLENOSCROLL; // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS;
//		si.nMin = 0;
//		si.nMax = nScrollMax;
//		si.nPage = nScrollPage; // si.nMax/SCROLLING_STEPS;
//		si.nPos = 0;
//		m_ScrollBar.SetScrollInfo(/*(SB_VERT/*|SB_CTL*/ &si, TRUE); 
//		_ScrollBar = true;
//	}

	ApplyAutoname();

	for (i=0; i<selSize; i++)
	{
		if(const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->getParent()==NULL)
		{
			const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->removePropertyByName("name");
			const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->removePropertyByName("path");
			const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->addPropertyByName("name",new CPropertyString (""));
		}
	}

	// focus the first control
	if (!Widgets.empty())
	{
		HWND toFocus = Widgets.begin()->getNextWindow(0);
		//(*Widgets.begin()).setFocus();
		::SetFocus(toFocus);
		::RedrawWindow(toFocus, NULL, NULL, RDW_INVALIDATE|RDW_INTERNALPAINT);
		// It's "name", so select content
		Widgets.begin()->EditBox.SetSel(0, -1, true);
//		if (focusName.empty())
//		{
//			toFocus = Widgets.begin()->getNextWindow(0);
//		}
//		else
//		{
//			// try to find a widget with the same parameter name as previous
//			list<CWidget>::iterator first(Widgets.begin()), last(Widgets.end());
//			for (; first != last; ++first)
//			{
//				if (first->Parameter.Name == focusName)
//				{
//					toFocus = first->getNextWindow(0);
//					break;
//				}
//			}
//			// no match, focus the first
//			if (first == last)
//			{
//				toFocus = Widgets.begin()->getNextWindow(0);
//			}
//		}

//		// keep the focused control in slider visibility
//		{
//			int move = 0;
//
//			RECT focusPos;
//			::GetWindowRect(toFocus, &focusPos);
//
//			m_PropertyFrame.ScreenToClient(&focusPos);
//			RECT framePos;
//			m_PropertyFrame.GetClientRect(&framePos);
//			if (focusPos.top < framePos.top)
//				move = focusPos.top - framePos.top;
//			else if (focusPos.bottom > framePos.bottom)
//				move = focusPos.bottom - framePos.bottom;
//
//			if (move != 0)
//			{
//				move = m_ScrollBar.GetScrollPos()+move;
//				// full MFC by sending a pseudo message
//				OnVScroll(SB_THUMBPOSITION, move, &m_ScrollBar);
//			}
//
//		}

	}

	SetActiveWindow();
	// trigger a global redraw of the dialog
	Invalidate();
}

// ***************************************************************************

void CDialogProperties::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	if (bShow)
		rebuildDialog ();

	CDialog::OnShowWindow(bShow, nStatus);
}

// ***************************************************************************

void CDialogProperties::OnUpdate() 
{
	// we update the current dialog
	updateModification ();
	updateModification ();

	// then, we look for other dialogs to be updated
	std::list<CDialogProperties*>::iterator it = PropertiesDialogs.begin();

	while ( it != PropertiesDialogs.end() )
	{
		if ( (*it)->containsSelection( _PropDlgLocators ) ||
			 containsSelection( (*it)->_PropDlgLocators ) )
			(*it)->rebuildDialog();
		it++;
	}
}

// ***************************************************************************

void CDialogProperties::updateModification ()
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Something selected ?
	const uint selSize = _PropDlgLocators.size ();

	// Auto name
	ApplyAutoname ();

	// Modification
	bool inModificationMode = doc->inModificationMode();
	if (/*!_Modal &&*/ !inModificationMode)
		doc->beginModification ();

	// For each selected primitive
	for (uint i=0; i<selSize; i++)
	{
		// Primitive ?
		const IPrimitive *primitive = _PropDlgLocators[i].Primitive;
		if (primitive)
		{
			// Get the primitive class
			const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*primitive);

			// Use the class or not ?
			if (primClass)
			{
				// Get the property for name
				const IProperty *propName;
				if (primitive->getPropertyByName ("name", propName))
				{
					// Create a default property
					CPrimitiveClass::CParameter defProp (*propName, "name");
			
					// Add the property in the good widget
					std::list<CWidget>::iterator ite = Widgets.begin ();
					while (ite != Widgets.end ())
					{
						// Set the parameter
						if (ite->toParameter (_PropDlgLocators[i], defProp))
							break;
						(*ite).Modified=false;

						ite++;
					}
				}

				// For each properties of the class
				for (uint p=0; p<primClass->Parameters.size (); p++)
				{
					// Is the parameter visible ?
					if (primClass->Parameters[p].Visible)
					{
						// Add the property in the good widget
						std::list<CWidget>::iterator ite = Widgets.begin ();
						while (ite != Widgets.end ())
						{
							(*ite).Modified=false;
							if (ite->toParameter (_PropDlgLocators[i], primClass->Parameters[p]))
								break;


							ite++;
						}
					}
				}
			}
			else
			{
				// For each primitive property
				uint numProp = primitive->getNumProperty ();
				for (uint p=0; p<numProp; p++)
				{
					// Get the property
					std::string		propertyName;
					const IProperty *prop;
					nlverify (primitive->getProperty (p, propertyName, prop));

					// Create a default property
					CPrimitiveClass::CParameter defProp (*prop, propertyName.c_str ());
					
					// Add the property in the good widget
					std::list<CWidget>::iterator ite = Widgets.begin ();
					while (ite != Widgets.end ())
					{
						(*ite).Modified=false;
						if (ite->toParameter (_PropDlgLocators[i], defProp))
							break;

						ite++;
					}
				}
			}
		}
	}

	// Modification
	if (/*!_Modal &&*/ !inModificationMode)
		doc->endModification ();

	getMainFrame ()->updateData ();
}

// ***************************************************************************

void CDialogProperties::updateModifiedState ()
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Something selected ?
	const uint selSize = _PropDlgLocators.size ();



	// Modification
	bool inModificationMode = doc->inModificationMode();
	if (!_Modal && !inModificationMode)
		doc->beginModification ();

	// For each selected primitive
	for (uint i=0; i<selSize; i++)
	{
		// Primitive ?
		const IPrimitive *primitive = _PropDlgLocators[i].Primitive;
		if (primitive)
		{
			// Get the primitive class
			const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*primitive);

			// Use the class or not ?
			if (primClass)
			{
				// Get the property for name
				const IProperty *propName;
				if (primitive->getPropertyByName ("name", propName))
				{
					// Create a default property
					CPrimitiveClass::CParameter defProp (*propName, "name");
			
					// Add the property in the good widget
					std::list<CWidget>::iterator ite = Widgets.begin ();
					while (ite != Widgets.end ())
					{
						
						(*ite).Modified=false;

						ite++;
					}
				}

				// For each properties of the class
				for (uint p=0; p<primClass->Parameters.size (); p++)
				{
					// Is the parameter visible ?
					if (primClass->Parameters[p].Visible)
					{
						// Add the property in the good widget
						std::list<CWidget>::iterator ite = Widgets.begin ();
						while (ite != Widgets.end ())
						{
							(*ite).Modified=false;
							


							ite++;
						}
					}
				}
			}
			else
			{
				// For each primitive property
				uint numProp = primitive->getNumProperty ();
				for (uint p=0; p<numProp; p++)
				{
					// Get the property
					std::string		propertyName;
					const IProperty *prop;
					nlverify (primitive->getProperty (p, propertyName, prop));

					// Create a default property
					CPrimitiveClass::CParameter defProp (*prop, propertyName.c_str ());
					
					// Add the property in the good widget
					std::list<CWidget>::iterator ite = Widgets.begin ();
					while (ite != Widgets.end ())
					{
						(*ite).Modified=false;

						ite++;
					}
				}
			}
		}
	}

	// Modification
	if (!_Modal && !inModificationMode)
		doc->endModification ();
}

// ***************************************************************************

void CDialogProperties::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	
//	m_ScrollBar.OnVScroll(nSBCode, nPos, pScrollBar);

	if ( !IsWindowVisible() )
		return;
	
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
	
	RECT frameRect;
	m_PropertyFrame.GetClientRect(&frameRect);
//	m_PropertyFrame.ScreenToClient(&contRect);
//	contRect.top = -m_ScrollBar.GetScrollPos();
//	contRect.bottom = -m_ScrollBar.GetScrollPos();
//	m_PropertyCont.s&contRect, TRUE);

//
//	int nDelta;
//	int nMaxPos = m_rect.Height() - m_rect2.Height();
//	
	int newPos;
	int limit = pScrollBar->GetScrollLimit();
	switch (nSBCode)
	{
	case SB_LINEDOWN:
		newPos = std::min(limit, int(pScrollBar->GetScrollPos()+16));
		break;
		
	case SB_LINEUP:
		newPos = std::max(0, int(pScrollBar->GetScrollPos()-16));
		break;
		
	case SB_PAGEDOWN:
		newPos = std::min(limit, int(pScrollBar->GetScrollPos()+frameRect.bottom));
		break;
		
	case SB_PAGEUP:
		newPos = std::max(0, int(pScrollBar->GetScrollPos()-frameRect.bottom));
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		newPos = nPos;
		break;
	default:
		return;
	}
//	m_nScrollPos += nDelta;
//	m_ScrollBar.SetScrollPos(SB_VERT,m_nScrollPos,TRUE);
	pScrollBar->SetScrollPos(newPos, TRUE);
	RECT contRect;
	m_PropertyCont.GetWindowRect(&contRect);
	m_PropertyFrame.ScreenToClient(&contRect);
//	int delta = -contRect.top - newPos;
	int height = contRect.bottom - contRect.top;
	contRect.top = -newPos;
	contRect.bottom = contRect.top + height;
//	m_PropertyFrame.ClientToScreen(&contRect);
	m_PropertyCont.MoveWindow(&contRect, TRUE);

	m_PropertyFrame.GetWindowRect(&contRect);
	ScreenToClient(&contRect);

//	m_PropertyCont.ScrollWindow(0, delta);
	
} 

// ***************************************************************************

void CDialogProperties::ApplyAutoname()
{
	// Update default parameters
	std::list<CWidget>::iterator ite = Widgets.begin();
	while (ite != Widgets.end())
	{
		if (ite->Default)
		{
			if (ite->Parameter.Type == CPrimitiveClass::CParameter::Boolean)
			{
				ite->CheckBox.SetCheck (2);
				ite->updateBoolean ();
			}
			else if (ite->Parameter.Type == CPrimitiveClass::CParameter::ConstString)
			{
				ite->ComboBox.SelectString(-1 , "");
				ite->updateCombo ();
			}
			else if (ite->Parameter.Type == CPrimitiveClass::CParameter::String)
			{
				ite->Initializing = true;
				ite->EditBox.SetWindowText("");
				ite->Initializing = false;
			}
			else if (ite->Parameter.Type == CPrimitiveClass::CParameter::StringArray)
			{
				ite->Initializing = true;
				ite->MultiLineEditBox.SetWindowText("");
				ite->Initializing = false;
//				ite->MultiLineEditBox.LoadText(CString());
				ite->updateMultiline ();
			}
			else if (ite->Parameter.Type == CPrimitiveClass::CParameter::ConstStringArray)
			{
				ite->Initializing = true;
				ite->ListEditBox.SetWindowText("");
				ite->Initializing = false;
				ite->updateList ();
			}
		}

		ite++;
	}

/*	// is the widget Autonamable ?
	string b = widget->Parameter.Autoname;
	if (!b.empty())
	{
		// we get the widget's name to propagate it to the linked name
		CString result;
		if (widget->Parameter.Type == CPrimitiveClass::CParameter::Boolean)
		{
			if (widget->CheckBox.GetCheck() == 1)
				result = "true";
			else if (widget->CheckBox.GetCheck() == 0)
				result = "false";
		}
		else if (widget->Parameter.Type == CPrimitiveClass::CParameter::ConstString)
		{
			// and we rename the correct field accordingly
			widget->ComboBox.GetLBText(widget->ComboBox.GetCurSel(), result);
		}
		else if (widget->Parameter.Type == CPrimitiveClass::CParameter::String)
		{
			widget->EditBox.GetWindowText(result);
		}
		else if (widget->Parameter.Type == CPrimitiveClass::CParameter::StringArray)
		{
			// we just get the first element
			std::vector<std::string> tempArray;
			widget->getValue (tempArray);
			if (!tempArray.empty())
			{
				result = tempArray[0].c_str();
			}
		}

		std::list<CWidget>::iterator ite;
		// if the widget has autoname, we simply copy its name to the linked widget
		ite = Widgets.begin ();
		while (ite != Widgets.end ())
		{
			CWidget &widget2 = *ite;
			if (widget2.Parameter.Type == CPrimitiveClass::CParameter::String)
			{
				string a = widget2.Parameter.Name;
				if (a == b)
				{
					// propagate the name to the autonamed field
					CString str2;
					widget2.EditBox.GetWindowText(str2);
					int i = str2.GetLength();
					while(--i >= 0)
					{
						if (str2[i] <'0' || str2[i]>'9')
						{
							break;
						}
					}
					// copy the name, followed by the existing number
					widget2.EditBox.SetWindowText(result+"_"+str2.Mid(i+1));
				}
			}
			ite++;
		}
	}
	else
	{
		const uint selSize = _Locators.size ();
		
		// The parameter list
		set<CPrimitiveClass::CParameter>	parameterList;
		
		// For each selected primitive
		uint i;
		for (i=0; i<selSize; i++)
		{
			// Primitive ?
			const IPrimitive *primitive = _Locators[i].Primitive;
			if (primitive)
			{
				// Get the class name
				const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*primitive);
				// Use the class or not ?
				if (primClass)
				{
					if (primClass->Autoname != "")
					{
						// we rename the string in one pass
						CString Name = primClass->Autoname.c_str();
						// is the widget useful for building the name ? No->let's continue
						CString fnd = widget->Parameter.Name.c_str();
						fnd = "$"+fnd+"$";
						if (Name.Find(fnd)<0) continue;

						std::list<CWidget>::iterator ite;
						// if the widget has autoname, we simply copy its name to the linked widget
						ite = Widgets.begin ();
						while (ite != Widgets.end ())
						{
							CWidget &widget2 = *ite;
							// we get the widget's name to propagate it to the linked name
							CString result;
							CString old = widget2.Parameter.Name.c_str();
							old = "$"+old+"$";
							if (widget2.Parameter.Type == CPrimitiveClass::CParameter::Boolean)
							{
								if (widget2.CheckBox.GetCheck() == 1)
									result = "true";
								else if (widget2.CheckBox.GetCheck() == 0)
									result = "false";
								Name.Replace(old, result);
							}
							else if (widget2.Parameter.Type == CPrimitiveClass::CParameter::ConstString)
							{
								// and we rename the correct field accordingly
								int sel=widget2.ComboBox.GetCurSel();
								if (sel != -1)
								{
									widget2.ComboBox.GetLBText(sel, result);
								}
								Name.Replace(old, result);
							}
							else if (widget2.Parameter.Type == CPrimitiveClass::CParameter::String)
							{
								if (widget2.Parameter.Name != "name")
								{
									widget2.EditBox.GetWindowText(result);
									Name.Replace(old, result);
								}
							}
							else if (widget2.Parameter.Type == CPrimitiveClass::CParameter::StringArray)
							{
								// we just get the first element
								std::vector<std::string> tempArray;
								widget2.getValue (tempArray);
								if (!tempArray.empty())
								{
									result = tempArray[0].c_str();
								}
							}
							ite++;
						}
						
						// then, we set the name of the "name" editbox
						ite = Widgets.begin ();
						while (ite != Widgets.end ())
						{
							CWidget &widget2 = *ite;
							
							if (widget2.Parameter.Type == CPrimitiveClass::CParameter::String)
							{
								if (widget2.Parameter.Name == "name")
								{
									// propagate the name to the autonamed field
									CString str2;
									if (Name != str2)
									{
										widget2.EditBox.SetWindowText(Name);
									}
								}
							}
							ite++;
						}
						
					}

				}
			}
		}
	}*/
}

// ***************************************************************************

void CDialogProperties::CWidget::setStaticName()
{
	string Name = Parameter.Name;
	if (Default)
	{
		Name += " (default value)";
	}
	if (Parameter.Type == CPrimitiveClass::CParameter::Boolean)
	{
		setWindowTextUTF8 (CheckBox, Name.c_str());
	}
	else
	{
		setWindowTextUTF8 (Static, Name.c_str());
	}
}

// ***************************************************************************

void CDialogProperties::SelectFolder(CWidget *widget)
{
	// select a directory in Win95 and NT4 (ugly code taken from Microsoft)
	BROWSEINFO bi;
	memset((LPVOID)&bi, 0, sizeof(bi));
	TCHAR szDisplayName[_MAX_PATH];
	szDisplayName[0] = '\0';
	bi.hwndOwner = GetSafeHwnd();
	bi.pszDisplayName = szDisplayName;
	bi.lpszTitle = _T("Pick a folder, any folder:");
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = 0;

	LPITEMIDLIST pidlRoot = NULL;
	LPSHELLFOLDER desktop;
	OLECHAR olePath[_MAX_PATH + 1];
	ULONG ulDummy;
	
	SHGetDesktopFolder (&desktop);
	
	if (widget->Parameter.Folder != "")
	{
		LPTSTR szPath = (char *)widget->Parameter.Folder.c_str();
		MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, szPath, -1, olePath, _MAX_PATH);
		desktop->ParseDisplayName (NULL, NULL, olePath, &ulDummy, &pidlRoot, &ulDummy);
	}
	bi.pidlRoot = pidlRoot;
	
	LPMALLOC pMalloc;
	HRESULT hr = SHGetMalloc(&pMalloc);

	LPITEMIDLIST pIIL = ::SHBrowseForFolder(&bi);

	if (pIIL != NULL)
	{
		TCHAR szInitialDir[_MAX_PATH];
		BOOL bRet = ::SHGetPathFromIDList(pIIL, (char*)&szInitialDir);
		if (bRet)
		{
			int s = strlen(szInitialDir);
			while (s)
			{
				--s;
				if (szInitialDir[s] == '\\' || szInitialDir[s] == ':')
				{
					++s;
					break;
				}
			}

			std::vector<std::string> tempArray;
			widget->getValue (tempArray);
			tempArray.push_back (szInitialDir+s);
			setEditTextMultiLine (widget->MultiLineEditBox, tempArray);
			widget->updateMultiline ();

			widget->MultipleValues = false;
			widget->Modified = true;
			widget->Set = true;
			widget->Default = false;
			/* todo hulud remove
			ApplyAutoname(widget);*/
			widget->setStaticName();
			
			pMalloc->Free(pIIL);
		}
	}

	desktop->Release ();
	if (pidlRoot) pMalloc->Free (pidlRoot);
	pMalloc->Release ();
}

// ***************************************************************************

BOOL CDialogProperties::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		UINT &message = pMsg->message;
		// handle 'tab' for widgets
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
		{
			bool shift = ::GetAsyncKeyState(VK_SHIFT) != 0;
			bool widgetWindow = true;
			HWND tabTo = 0;

			// yep, we got a tab, check the wnd
			std::list<CWidget>::iterator first(Widgets.begin()), last(Widgets.end());
			for (; first != last; ++first)
			{
				if (first->isHwndMatch(pMsg->hwnd))
				{
					// ok, we got the good one, focus the next widget or wrap
					if (!shift)
					{
						tabTo = first->getNextWindow(pMsg->hwnd);
						if (!tabTo)
						{
							// advance to next widget
							++first;
							if (first == last)
							{
								tabTo = m_OKButton.m_hWnd;
								widgetWindow = false;
							}
							else
								tabTo = first->getNextWindow(0);
						}
					}
					else
					{
						tabTo = first->getPreviousWindow(pMsg->hwnd);
						if (!tabTo)
						{
							// advance to previous widget
							if (first == Widgets.begin())
							{
								tabTo = m_updateButton.m_hWnd;
								widgetWindow = false;
							}
							else
							{
								--first;
								tabTo = first->getPreviousWindow(0);
							}
						}
					}

					// stop to parse the widget
					break;

				}
			}

			// if no tab found, try the main button
			if (!tabTo && !Widgets.empty())
			{
				// not a widget window, check Ok and Update buttons
				if (pMsg->hwnd == m_OKButton.m_hWnd && shift)
				{
					// select the last widget
					CWidget &wg = Widgets.back();
					tabTo = wg.getPreviousWindow(0);
					widgetWindow = false;
				}
				else if (pMsg->hwnd == m_updateButton.m_hWnd && !shift)
				{
					// select the first widget
					CWidget &wg = Widgets.front();
					tabTo = wg.getNextWindow(0);
					widgetWindow = true;
				}
			}

			// some focus to set ?
			if (tabTo)
			{
				// focus and redraw the control (to showup focus rectagne if needed)
				::SetFocus(tabTo);
				::RedrawWindow(tabTo, NULL, NULL, RDW_INVALIDATE|RDW_INTERNALPAINT);
				// keep the focused control in slider visibility
				if (widgetWindow)
				{
					int move = 0;

					RECT focusPos;
					::GetWindowRect(tabTo, &focusPos);

					m_PropertyFrame.ScreenToClient(&focusPos);
					// add the additionnal space for label (we like to see the label of current control !)
					focusPos.top -= LABEL_HEIGHT + 5;
					
					RECT framePos;
					m_PropertyFrame.GetClientRect(&framePos);
					if (focusPos.top < framePos.top)
						move = focusPos.top - framePos.top;
					else if (focusPos.bottom > framePos.bottom)
						move = focusPos.bottom - framePos.bottom;

					if (move != 0)
					{
						move = m_ScrollBar.GetScrollPos()+move;
						// full MFC by sending a pseudo message
						OnVScroll(SB_THUMBPOSITION, move, &m_ScrollBar);
					}

				}
				return TRUE;
			}
		}
		// handle ALT+arrow to change selection
		else if (pMsg->message == WM_SYSKEYDOWN 
			&& (pMsg->wParam == VK_UP 
				|| pMsg->wParam == VK_DOWN
				|| pMsg->wParam == VK_LEFT
				|| pMsg->wParam == VK_RIGHT))
		{
			
			CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(getMainFrame ()->m_wndSplitter.GetPane(0,1));

			toolWnd->GetTreeCtrl()->SendMessage(WM_KEYDOWN, pMsg->wParam, 0);

			return TRUE;
		}

		// finally, translate the message
		if (::TranslateAccelerator(theApp.m_pMainWnd->m_hWnd, _AccelTable, pMsg))
			return TRUE;

	}

	return CDialog::PreTranslateMessage(pMsg);
}

// ***************************************************************************

void CDialogProperties::setDefaultValue (CWidget *widget, string &value)
{
	value = "";
	bool foundOne = false;
	widget->MultipleValues = false;
	string fromWhere;
		
	// Something selected ?
	const uint selSize = _PropDlgLocators.size ();

	// For each selected primitive
	for (uint i=0; i<selSize; i++)
	{
		// Primitive ?
		const IPrimitive *primitive = _PropDlgLocators[i].Primitive;
		if (primitive)
		{
			// Get the primitive class
			const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*primitive);

			// Use the class or not ?
			if (primClass)
			{
				// Look for the good property
				uint p;
				for (p=0; p<primClass->Parameters.size(); p++)
				{
					if (primClass->Parameters[p].Name == widget->Parameter.Name)
					{
						string temp;
						primClass->Parameters[p].getDefaultValue (temp, *primitive, *primClass, &fromWhere);

						if (foundOne)
						{
							if (value != temp)
							{
								value = DIFFERENT_VALUE_STRING;
								widget->MultipleValues = true;
							}
							if (widget->FromWhere != fromWhere)
							{
								widget->FromWhere = "";
							}
						}
						else
						{
							value = temp;
							foundOne = true;
							widget->FromWhere = fromWhere;
						}

						break;
					}
				}
			}
		}
	}
}

// ***************************************************************************

void CDialogProperties::setDefaultValue (CWidget *widget, std::vector<std::string> &value)
{
	value.clear ();
	bool foundOne = false;
	widget->MultipleValues = false;
	string fromWhere;
		
	// Something selected ?
	const uint selSize = _PropDlgLocators.size ();

	// For each selected primitive
	for (uint i=0; i<selSize; i++)
	{
		// Primitive ?
		const IPrimitive *primitive = _PropDlgLocators[i].Primitive;
		if (primitive)
		{
			// Get the primitive class
			const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*primitive);

			// Use the class or not ?
			if (primClass)
			{
				// Look for the good property
				uint p;
				for (p=0; p<primClass->Parameters.size(); p++)
				{
					if (primClass->Parameters[p].Name == widget->Parameter.Name)
					{
						vector<string> temp;
						primClass->Parameters[p].getDefaultValue (temp, *primitive, *primClass, &fromWhere);

						if (foundOne)
						{
							if (value != temp)
							{
								value.resize (1);
								value[0] = DIFFERENT_VALUE_STRING;
								widget->MultipleValues = true;
							}
							if (widget->FromWhere != fromWhere)
							{
								widget->FromWhere = "";
							}
						}
						else
						{
							value = temp;
							foundOne = true;
							widget->FromWhere = fromWhere;
						}

						break;
					}
				}
			}
		}
	}
}

// ***************************************************************************

LRESULT CDialogProperties::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case LBN_CHANGE:
		// Button id
		int idButton = wParam;    // identifier of button 
		if (idButton >= FIRST_WIDGET)
		{
			// Get the widget
			CWidget *widget = getWidget (idButton-FIRST_WIDGET);
			nlassert (widget->Parameter.Type == CPrimitiveClass::CParameter::ConstStringArray);

			// String NULL ?
			std::vector<std::string> result;
			widget->getValue (result);
			widget->Default = result.empty();
			widget->updateList ();
			widget->Modified = true;
		}

		break;
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

// ***************************************************************************

void CDialogProperties::OnSetFocus(CWnd*)
{
	if(!Widgets.empty())
	{
		list<CWidget>::iterator ite=Widgets.begin();
		(*ite).setFocus();
	}
}

// ***************************************************************************

BOOL CDialogProperties::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	int pos = m_ScrollBar.GetScrollPos();
	pos -= zDelta;
	m_ScrollBar.SetScrollPos(pos, FALSE);
	pos = m_ScrollBar.GetScrollPos();
	OnVScroll(SB_THUMBPOSITION, pos, &m_ScrollBar);
//	m_ScrollBar.OnMouseWheel(nFlags, zDelta, pt);
/*	if (_ScrollBar)
	{
		int newPos = m_nScrollPos - zDelta;
		clamp(newPos, 0, m_rect.Height() - m_rect2.Height());
		int delta = newPos - m_nScrollPos;
		if (delta)
		{
			m_nScrollPos += delta;
			SetScrollPos(SB_VERT,m_nScrollPos,TRUE);
			ScrollWindow(0,-delta);
		}
	}
*/	
	return CDialog::OnMouseWheel(nFlags, zDelta, pt);

}

// ***************************************************************************

bool dataToClipboard (CWnd *wnd, UINT format, void *data, uint len)
{
	// Open the clipboard
	if (wnd->OpenClipboard ())
	{
		nlverify (EmptyClipboard ());
		if (data)
		{
			// Alloc a global memory
			HGLOBAL hData = GlobalAlloc (GMEM_MOVEABLE|GMEM_DDESHARE, len);
			if (hData)
			{
				// Copy the string
				LPVOID dataPtr = GlobalLock (hData);
				nlverify (dataPtr);
				memcpy (dataPtr, data, len);

				// Release the pointer
				GlobalUnlock (hData);

				// Set the clipboard
				nlverify (SetClipboardData (format, hData));

				// Close the clipboard
				CloseClipboard ();

				// Ok
				return true;
			}
		}

		// Close the clipboard
		CloseClipboard ();
	}
	return false;
}

// ***************************************************************************

bool dataFromClipboard (CWnd *wnd, UINT format, void *data, uint lenMax)
{
	// Open the clipboard
	if (wnd->OpenClipboard ())
	{
		// Get the clipboard data
		HANDLE hData = GetClipboardData (format);
		if (hData)
		{
			DWORD len = GlobalSize (hData);
 			
			// Get the string
			LPVOID dataPtr = GlobalLock (hData);
			nlverify (dataPtr);

			// Copy the string
			memcpy (data, dataPtr, std::min(len, (DWORD)lenMax));

			// Close the clipboard
			CloseClipboard ();

			// Ok
			return true;
		}

		// Close the clipboard
		CloseClipboard ();
	}
	return false;
}

// ***************************************************************************

void CMyComboBox::reloadData()
{
	if (!loaded)
	{
		int n = GetCurSel();
		CString s;
		if(n != CB_ERR) GetLBText(n, s);
		ResetContent();
		SetRedraw(FALSE);
		InsertString(-1, "");
		for (vector<string>::iterator it=_data.begin(), itEnd=_data.end(); it!=itEnd; ++it)
			InsertString(-1, it->c_str());
		loaded = true;
		SetRedraw(TRUE);
		if(n != CB_ERR) SelectString(-1, s);
	}
}

// ***************************************************************************

BOOL CMyComboBox::PreTranslateMessage( MSG* pMsg )
{
	if (Widget != NULL && ((CDialogProperties::CWidget*)Widget)->Parameter.Editable)
	{
		if (pMsg->message == WM_CHAR)
			GetParent()->SendMessage(WM_COMMAND, MAKELPARAM(GetDlgCtrlID(), CBN_SELCHANGE), (LPARAM)m_hWnd);
		else if (pMsg->message == WM_LBUTTONDOWN)
			reloadData();
		return CComboBox::PreTranslateMessage(pMsg);
	}

	bool search = false;
	if (pMsg->message == WM_CHAR)
	{
		// Erase key buffer ?
		sint64 time = NLMISC::CTime::getLocalTime ();
		if (time - _LastStrokeTime > COMBO_STROKE_DELAI)
			_LastString = "";

		// Add this char
		_LastString.push_back (tolower((TCHAR) pMsg->wParam));
		search = true;

		// New stroke time
		_LastStrokeTime = time;
	}
	// Copy ?
	else if ((pMsg->message == WM_KEYDOWN) && ('C' == (int) pMsg->wParam) && (GetAsyncKeyState (VK_CONTROL) & 0x8000))
	{
		int curSel = GetCurSel ();
		if (curSel != CB_ERR)
		{
			CString rString;
			GetLBText (curSel, rString);
			dataToClipboard (this, CF_TEXT, (void*)(const char*)rString, rString.GetLength()+1);
		}
	}
	// Paste ?
	else if ((pMsg->message == WM_KEYDOWN) && ('V' == (int) pMsg->wParam) && (GetAsyncKeyState (VK_CONTROL) & 0x8000))
	{
		char text[512];
		if (dataFromClipboard (this, CF_TEXT, text, 511))
		{
			// 0 final
			text[511] = 0;
			_LastString = text;
			search = true;
		}
	}
	// Button down ?
	else if (pMsg->message == WM_LBUTTONDOWN)
		reloadData();

	// Process string search ?
	if (search)
	{
		// Find a string with this name
		const uint count = GetCount();
		uint i;
		uint matchSize = 0;
		sint matchItem = -1;
		for (i=0; i<count; i++)
		{
			CString rString;
			GetLBText (i, rString);
			string tmp = strlwr ((const char*)rString);
			uint size = std::min (_LastString.size(), tmp.size());
			if (size > matchSize)
			{
				if (strncmp(tmp.c_str(), _LastString.c_str(), size) == 0)
				{
					matchItem = i;
					matchSize = size;
				}
			}
		}

		// Found something ?
		if (matchItem != -1)
		{
			SetCurSel (matchItem);
			GetParent()->SendMessage(WM_COMMAND, MAKELPARAM(GetDlgCtrlID(), CBN_SELCHANGE), (LPARAM)m_hWnd);
		}

		return TRUE;
	}

	return FALSE;
}

// ***************************************************************************

void CDialogProperties::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize( nType, cx, cy );
	
	if ( !IsWindowVisible() )
		return;


	::CRect newDlgRect;
	int deltaX, deltaY;
	GetWindowRect( &newDlgRect );
	deltaX = newDlgRect.Width() - m_curDlgRect.Width();
	deltaY = newDlgRect.Height() - m_curDlgRect.Height();
	m_curDlgRect = newDlgRect;

	OnVScroll(SB_THUMBPOSITION, 0, &m_ScrollBar);

	{
		RECT frameRect;
		m_PropertyFrame.GetWindowRect( &frameRect );
		ScreenToClient( &frameRect );
		frameRect.right += deltaX;
		frameRect.bottom += deltaY;
		m_PropertyFrame.MoveWindow( &frameRect );
	}

	list<CWidget>::iterator ite=Widgets.begin();
	int decPosY = 0;

	while ( ite != Widgets.end() )
	{
		if ( (*ite).OnSize( deltaX, deltaY, decPosY ) == true )
			decPosY += deltaY;
		ite++;
	}
	
	{
		RECT contRect;
		m_PropertyCont.GetWindowRect( &contRect );
		m_PropertyFrame.ScreenToClient( &contRect );
		contRect.right += deltaX;
		contRect.bottom += decPosY;
		//contRect.bottom = contRect.top + cy - m_FrameBottomSpace;
		m_PropertyCont.MoveWindow ( &contRect );
	}

	// replace the 3 button
	RECT buttonRect, dialogRect;
	int space = ( newDlgRect.Width() - m_buttonWidth ) / 2;
	int decalage;
	GetClientRect(&dialogRect);
	
	// OK
	GetDlgItem(IDOK)->GetWindowRect(&buttonRect);
	ScreenToClient(&buttonRect);
	buttonRect.bottom = (dialogRect.bottom - m_ButtonPos) + (buttonRect.bottom-buttonRect.top);
	buttonRect.top = dialogRect.bottom - m_ButtonPos;
	decalage = space - buttonRect.left;
	buttonRect.left += decalage;
	buttonRect.right += decalage;
	GetDlgItem(IDOK)->MoveWindow(&buttonRect);
	// cancel
	GetDlgItem(IDCANCEL)->GetWindowRect(&buttonRect);
	ScreenToClient(&buttonRect);
	buttonRect.bottom = (dialogRect.bottom - m_ButtonPos) + (buttonRect.bottom-buttonRect.top);
	buttonRect.top = dialogRect.bottom - m_ButtonPos;
	buttonRect.left += decalage;
	buttonRect.right += decalage;
	GetDlgItem(IDCANCEL)->MoveWindow(&buttonRect);
	//IDUPDATE
	GetDlgItem(IDUPDATE)->GetWindowRect(&buttonRect);
	ScreenToClient(&buttonRect);
	buttonRect.bottom = (dialogRect.bottom - m_ButtonPos) + (buttonRect.bottom-buttonRect.top);
	buttonRect.top = dialogRect.bottom - m_ButtonPos;
	buttonRect.left += decalage;
	buttonRect.right += decalage;
	GetDlgItem(IDUPDATE)->MoveWindow(&buttonRect);

	RECT contRect;
	m_PropertyCont.GetWindowRect(&contRect);
	RECT frameRect;
	m_PropertyFrame.GetClientRect(&frameRect);
	// Set the scroll bar value
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE /*SIF_ALL*/|SIF_DISABLENOSCROLL; // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nMin = 0;
	si.nMax = contRect.bottom - contRect.top;
	si.nPage = frameRect.bottom; 
	int pos = std::min(LONG(m_ScrollBar.GetScrollPos()), std::max(LONG(0), LONG(si.nMax - si.nPage)));
	m_ScrollBar.SetScrollInfo(&si, FALSE);
	// foul MFC by sending a pseudo message
	OnVScroll(SB_THUMBPOSITION, pos, &m_ScrollBar);
	
	// replace the scroll bar control
	RECT scrollRect;
	m_PropertyFrame.GetClientRect(&scrollRect);
	m_PropertyFrame.ClientToScreen(&scrollRect);
	scrollRect.left = scrollRect.right-16;
	ScreenToClient(&scrollRect);
	m_ScrollBar.MoveWindow(&scrollRect, TRUE);
	
	
	Invalidate();
}

// ***************************************************************************

void CDialogProperties::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI ) 
{
	CDialog::OnGetMinMaxInfo( lpMMI );
	
	lpMMI->ptMinTrackSize.x = 455;
	lpMMI->ptMinTrackSize.y = 50;
}

// ***************************************************************************

bool CDialogProperties::removePrimitives( list<NLLIGO::IPrimitive*> &locators )
{
	for(list<IPrimitive*>::iterator itP = locators.begin(); itP != locators.end(); itP++)
	{
		NLLIGO::IPrimitive* pPrim = *itP;

		for(vector<CDatabaseLocatorPointer>::iterator it = _PropDlgLocators.begin(); it != _PropDlgLocators.end(); it++)
		{
			if ( (*it).Primitive == pPrim )
			{
				_PropDlgLocators.erase( it );
				break;
			}
		}
	}

	if(_PropDlgLocators.empty())
	{
		// if all primitives were removed, the dialog is closed
		OnCancel();
		return true;
	}
	else
	{
		OnUpdate();
		return false;
	}
}


void CDialogProperties::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
}
