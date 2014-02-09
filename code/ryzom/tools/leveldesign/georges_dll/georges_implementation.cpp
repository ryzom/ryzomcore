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

// Interface Implementation

#include "stdafx.h"

#undef GEORGES_EXPORT
#define GEORGES_EXPORT __declspec( dllexport ) 

#include "georges_interface.h"
#include "georges_edit.h"
#include "main_frm.h"
#include "left_view.h"
#include "georges_edit_doc.h"
#include "georges_edit_view.h"
#include "child_frm.h"
#include "action.h"

#include "nel/georges/form_elm.h"
#include "nel/georges/type.h"
#include "nel/georges/form.h"
#include "nel/georges/form_elm.h"

#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"

#include <string>

using namespace NLGEORGES;
using namespace NLMISC;

/////////////////////////////////////////////////////////////////////////////
// The one and only CGeorgesEditApp object

// CGeorgesEditApp TheGlobalApp;


// The interface implemented through this class
class CGeorgesImpl : public IGeorges
{

public:

	CGeorgesImpl();

	virtual ~CGeorgesImpl();

	// Init the UI
	virtual void initUI (int nCmdShow, bool exeStandalone, HWND parent=NULL);

	// Init the UI Light version
	virtual void initUILight (int nCmdShow, int x, int y, int cx, int cy);

	// Go
	virtual void go ();	

	// Release the UI
	virtual void releaseUI (); 

	// Get the main frame
	virtual void* getMainFrame (); 
	
	// Get instance
	static GEORGES_EXPORT IGeorges* getInterface (int version = GEORGES_VERSION); 

	// Release instance
	static GEORGES_EXPORT void releaseInterface (IGeorges* pGeorges);

	virtual void createInstanceFile (const std::string &_sxFullnameWithoutExt, const std::string &_dfnname);

//	virtual void NewDocument();

	virtual void NewDocument( const std::string& _sxdfnname);

	virtual void LoadDocument( const std::string& _sxfullname );

/*	virtual void SaveDocument( const std::string& _sxfullname );

	virtual void CloseDocument();
*/
	// Directories settings
	virtual void SetDirDfnTyp		(const std::string& _sxworkdirectory);
	virtual void SetDirPrototype	(const std::string& _sxrootdirectory);
	virtual void SetDirLevel		(const std::string& _sxrootdirectory);

	virtual std::string GetDirDfnTyp ();
/*	virtual std::string GetDirPrototype	();
	virtual std::string GetDirLevel ();*/

	// Put a text in the right cell
	virtual void PutGroupText (const std::vector<std::string>& _vText, bool append);
	virtual void PutText (const std::string& _sText);
	virtual void LineUp ();
	virtual void LineDown ();

	virtual BOOL PreTranslateMessage (MSG *pMsg);
/*
	virtual void SaveAllDocument();

	virtual void CloseAllDocument();

	virtual void SetTypPredef( const std::string& _sxfilename, const std::vector< std::string >& _pvs );

	virtual void MakeDfn( const std::string& _sxfullname, const std::vector< std::pair< std::string, std::string > >* const _pvdefine = 0 );*/

	virtual void MakeTyp( const std::string& filename, TType type, TUI ui, const std::string& _min, const std::string& _max, const std::string& _default, const std::vector< std::pair< std::string, std::string > >* const _pvpredef );
};

// ---------------------------------------------------------------------------
CGeorgesImpl::CGeorgesImpl()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}

// ---------------------------------------------------------------------------
CGeorgesImpl::~CGeorgesImpl()
{
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::initUI( int nCmdShow, bool exeStandalone,  HWND parent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	theApp.initInstance(nCmdShow, exeStandalone, -1, -1, -1, -1);
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::initUILight (int nCmdShow, int x, int y, int cx, int cy)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	theApp.initInstance (nCmdShow, false, x, y, cx, cy);
	((CMainFrame*)(theApp.m_pMainWnd))->m_bDontClose = true;
	theApp.OnViewRefresh();
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::go()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	theApp.Run ();
	// theApp.m_pMainWnd->ActivateFrame;

/*
	MSG	msg;
	while (GetMessage( &msg, *theApp.m_pMainWnd, 0, 0) == TRUE)
	{
		if (!theApp.m_pMainWnd->PreTranslateMessage(&msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		if (!IsWindow (*theApp.m_pMainWnd))
			break;
	}	*/
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::releaseUI()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (IsWindow (*theApp.m_pMainWnd))
		theApp.m_pMainWnd->DestroyWindow();
	theApp.m_pMainWnd = NULL;
	theApp.releasePlugins ();
}

// ---------------------------------------------------------------------------
void * CGeorgesImpl::getMainFrame ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	

	return theApp.m_pMainWnd;
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::SetDirDfnTyp		(const std::string& _sxDirectory)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesEditApp* theApp = dynamic_cast<CGeorgesEditApp*>(AfxGetApp());
	
	//theApp.SetDirDfnTyp (_sxDirectory);
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::SetDirPrototype	(const std::string& _sxDirectory)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	//theApp.SetDirPrototype (_sxDirectory);
	theApp.RootSearchPath = _sxDirectory;
	theApp.initCfg ();
	theApp.OnViewRefresh ();
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::SetDirLevel		(const std::string& _sxDirectory)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	//theApp.SetDirLevel (_sxDirectory);
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::PutGroupText (const std::vector<std::string>& _vText, bool append)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMainFrame *pWnd = dynamic_cast< CMainFrame* >( theApp.m_pMainWnd );
	CChildFrame *pChild = (CChildFrame*)pWnd->MDIGetActive ();
	if (pChild == NULL) return;
	// Get active document
	CGeorgesEditDoc *doc = (CGeorgesEditDoc *)pChild->GetActiveDocument();
	if (doc)
	{
		// Get the left view
		CLeftView* pView = doc->getLeftView ();

		// Check type
		CGeorgesEditDocSub *subDoc = doc->getSelectedObject ();
		if (subDoc)
		{
			// Get the node
			const CFormDfn *parentDfn;
			uint indexDfn;
			const CFormDfn *nodeDfn;
			const CType *nodeType;
			CFormElm *node;
			UFormDfn::TEntryType type;
			bool array;
			bool parentVDfnArray;
			CForm *form=doc->getFormPtr ();
			CFormElm *elm = doc->getRootNode (subDoc->getSlot ());
			nlverify ( elm->getNodeByName (subDoc->getFormName ().c_str (), &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

			// Is a type entry ?
			if ((type == UFormDfn::EntryType) && array)
			{
				std::string formName = subDoc->getFormName ();
				uint slot = subDoc->getSlot ();

				// Current node size
				uint size = 0;
				if (node)
				{
					CFormElmArray *arrayPtr = safe_cast<CFormElmArray*>(node);
					nlverify (arrayPtr->getArraySize (size));
				}

				// Modify the size of the array
				char value[512];
				smprintf (value, 512, "%d", _vText.size () + ((append)?size:0));
				doc->modify (new CActionBuffer (IAction::FormArraySize, NULL, 0, *doc, formName.c_str (), 
					value, doc->getLeftView ()->getCurrentSelectionId (), slot));

				uint i;
				for (i=0; i<_vText.size (); i++)
				{
					uint index =  i + ((append)?size:0);
					std::string formNameAtom = formName + "[" + toString (index) + "]";
					doc->modify (new CActionString (IAction::FormTypeValue, _vText[i].c_str (), *doc, formNameAtom.c_str (),  "",
						doc->getLeftView ()->getCurrentSelectionId (), slot));
					doc->modify (new CActionString (IAction::FormArrayRename, _vText[i].c_str(), *doc, formNameAtom.c_str (), 
						toString (index).c_str (), doc->getLeftView ()->getCurrentSelectionId (), slot));
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::PutText (const std::string& _sText)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMainFrame *pWnd = dynamic_cast< CMainFrame* >( theApp.m_pMainWnd );
	CChildFrame *pChild = (CChildFrame*)pWnd->MDIGetActive ();
	if (pChild == NULL) return;
	// Get active document
	CGeorgesEditDoc *doc = (CGeorgesEditDoc *)pChild->GetActiveDocument();
	if (doc)
	{
		// Get the left view
		CLeftView* pView = doc->getLeftView ();

		// Check type
		CGeorgesEditDocSub *subDoc = doc->getSelectedObject ();
		if (subDoc)
		{
			// Get the node
			const CFormDfn *parentDfn;
			uint indexDfn;
			const CFormDfn *nodeDfn;
			const CType *nodeType;
			CFormElm *node;
			UFormDfn::TEntryType type;
			bool array;
			bool parentVDfnArray;
			CForm *form=doc->getFormPtr ();
			CFormElm *elm = doc->getRootNode (subDoc->getSlot ());
			nlverify ( elm->getNodeByName (subDoc->getFormName ().c_str (), &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

			// It is an array ?
			if (array&&(type == UFormDfn::EntryType))
			{
				// Modify the node
				doc->modify (new CActionString (IAction::FormTypeValue, _sText.c_str(), *doc, subDoc->getFormName ().c_str (),  "",
					doc->getLeftView ()->getCurrentSelectionId (), subDoc->getSlot ()));
				doc->modify (new CActionString (IAction::FormArrayRename, _sText.c_str(), *doc, subDoc->getFormName ().c_str (), 
					toString (subDoc->getIdInParent ()).c_str (), doc->getLeftView ()->getCurrentSelectionId (), subDoc->getSlot ()));
				doc->updateDocumentStructure ();
				doc->UpdateAllViews (pView);
			}
			else if ((UFormDfn::EntryDfn)&&(!array))
			{
				// Get the right view
				CGeorgesEditView* view = doc->getRightView ();
				if (view->FormDialog.WidgetFocused != 0xffffffff)
				{
					// Set the string
					doc->modify (new CActionString (IAction::FormValue, _sText.c_str(), *doc, 
						view->FormDialog.Widgets[view->FormDialog.WidgetFocused]->getFormName ().c_str (),  "",
						doc->getLeftView ()->getCurrentSelectionId (), subDoc->getSlot ()));
					doc->updateDocumentStructure ();
					doc->UpdateAllViews (pView);
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::LineUp ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMainFrame* pWnd = dynamic_cast< CMainFrame* >( theApp.m_pMainWnd );
	CMDIChildWnd *pChild = pWnd->MDIGetActive ();
	if (pChild == NULL) return;
	// Get active document
	CGeorgesEditDoc *doc = (CGeorgesEditDoc *)pChild->GetActiveDocument ();
	if (doc)
	{
		// Get the left view
		CLeftView* pView = doc->getLeftView ();

		// Check type
		CGeorgesEditDocSub *subDoc = doc->getSelectedObject ();
		if (subDoc)
		{
			// Get the node
			const CFormDfn *parentDfn;
			uint indexDfn;
			const CFormDfn *nodeDfn;
			const CType *nodeType;
			CFormElm *node;
			UFormDfn::TEntryType type;
			bool array;
			bool parentVDfnArray;
			CForm *form=doc->getFormPtr ();
			CFormElm *elm = doc->getRootNode (subDoc->getSlot ());
			nlverify ( elm->getNodeByName (subDoc->getFormName ().c_str (), &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

			// Is a type entry ?
			if ( (type == UFormDfn::EntryType) && !array )
			{
				// Select next 
				if ((subDoc->getIdInParent ()) > 0)
					doc->changeSubSelection (subDoc->getParent ()->getChild (subDoc->getIdInParent ()-1), pView);
			}
		}
	}
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::LineDown ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMainFrame* pWnd = dynamic_cast< CMainFrame* >( theApp.m_pMainWnd );
	CMDIChildWnd *pChild = pWnd->MDIGetActive ();
	if (pChild == NULL) return;

	// Get active document
	CGeorgesEditDoc *doc = (CGeorgesEditDoc *)pChild->GetActiveDocument ();
	if (doc)
	{
		// Get the left view
		CLeftView* pView = doc->getLeftView ();

		// Check type
		CGeorgesEditDocSub *subDoc = doc->getSelectedObject ();
		if (subDoc)
		{
			// Get the node
			const CFormDfn *parentDfn;
			uint indexDfn;
			const CFormDfn *nodeDfn;
			const CType *nodeType;
			CFormElm *node;
			UFormDfn::TEntryType type;
			bool array;
			bool parentVDfnArray;
			CForm *form=doc->getFormPtr ();
			CFormElm *elm = doc->getRootNode (subDoc->getSlot ());
			nlverify ( elm->getNodeByName (subDoc->getFormName ().c_str (), &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

			// Is a type entry ?
			if ( (type == UFormDfn::EntryType) && !array )
			{
				// Select next 
				if ((subDoc->getIdInParent ()+1) < subDoc->getParent ()->getChildrenCount ())
					doc->changeSubSelection (subDoc->getParent ()->getChild (subDoc->getIdInParent ()+1), pView);
			}
		}
	}
}

// ---------------------------------------------------------------------------
BOOL CGeorgesImpl::PreTranslateMessage (MSG *pMsg)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMainFrame* pWnd = dynamic_cast< CMainFrame* >( theApp.m_pMainWnd );
	if (pWnd == NULL)
		return FALSE;
	return theApp.PreTranslateMessage (pMsg);
}

// ---------------------------------------------------------------------------
/*void CGeorgesImpl::SetTypPredef( const std::string& _sxfilename, const std::vector< std::string >& _pvs )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesEditApp* pApp = dynamic_cast< CGeorgesEditApp* >( AfxGetApp() );
	CLoader* ploader = theApp.GetLoader();
	std::vector< CStringEx > vsx;
	for( std::vector< std::string >::const_iterator it = _pvs.begin(); it != _pvs.end(); ++it )
		vsx.push_back( *it );

	try
	{
		ploader->SetTypPredef( _sxfilename, vsx );
	}
	catch (NLMISC::Exception &e)
	{
		std::string tmp = std::string(e.what()) + "(" + _sxfilename + ")";
		theApp.m_pMainWnd->MessageBox(tmp.c_str(), "Georges_Lib", MB_ICONERROR | MB_OK);
	}
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::MakeDfn( const std::string& _sxfullname, const std::vector< std::pair< std::string, std::string > >* const _pvdefine  )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesEditApp* papp = dynamic_cast< CGeorgesEditApp* >( AfxGetApp() );
	CLoader* ploader = theApp.GetLoader();
	std::vector< std::pair< CStringEx, CStringEx > >* vsx = 0;
	if( _pvdefine )
	{
		std::vector< std::pair< CStringEx, CStringEx > > v;
		vsx = &v;
		for( std::vector< std::pair< std::string, std::string > >::const_iterator it = _pvdefine->begin(); it != _pvdefine->end(); ++it )
			vsx->push_back( std::make_pair( it->first , it->second  ) );
	}
	ploader->MakeDfn( _sxfullname, vsx );
}
*/
// ---------------------------------------------------------------------------
void CGeorgesImpl::MakeTyp( const std::string& filename, TType type, TUI ui, const std::string& _min, const std::string& _max, const std::string& _default, const std::vector< std::pair< std::string, std::string > >* const _pvpredef )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Create a type
	CType t;
	t.Type = (CType::TType)type;
	t.UIType = (CType::TUI)ui;
	t.Min= _min;
	t.Max = _max;
	t.Default = _default;

	if (_pvpredef)
	{
		t.Definitions.resize (_pvpredef->size ());
		uint i;
		for (i=0; i<_pvpredef->size (); i++)
		{
			t.Definitions[i].Label = (*_pvpredef)[i].first;
			t.Definitions[i].Value = (*_pvpredef)[i].second;
		}
	}

	// Save the type
	COFile output;
	if (output.open (filename))
	{
		try
		{
			// XML stream
			COXml outputXml;
			outputXml.init (&output);

			// Write
			t.write (outputXml.getDocument (), theApp.Georges4CVS);
		}
		catch (Exception &e)
		{
			nlwarning ("Error during writing file '%s' : ", filename.c_str (), e.what ());
		}
	}
	else
	{
		nlwarning ("Can't open the file %s for writing", filename.c_str ());
	}
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::createInstanceFile (const std::string &_sxFullnameWithoutExt, const std::string &_dfnname)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CFormLoader formLoader;
	CFormDfn *dfn = formLoader.loadFormDfn (_dfnname.c_str(), false);
	if (!dfn)
	{
		char msg[512];
		smprintf (msg, 512, "Can't load DFN '%s'", _dfnname);
		theApp.outputError (msg);
		return;
	}

	NLMISC::CSmartPtr<NLGEORGES::UForm> Form = new CForm;

	std::string fullName;
	fullName = _sxFullnameWithoutExt + ".";

	int i = 0;
	if (_dfnname[i] == '.') ++i;
	for (; i < (int)_dfnname.size(); ++i)
	{
		if (_dfnname[i] == '.') break;
		fullName += _dfnname[i];
	}

	((CFormElmStruct*)&Form->getRootNode ())->build (dfn);
	COFile f;
	COXml ox;
	if (f.open (fullName))
	{
		ox.init(&f);
		((NLGEORGES::CForm*)((UForm*)Form))->write (ox.getDocument(), _sxFullnameWithoutExt.c_str (), theApp.Georges4CVS);
		ox.flush();
		f.close();
	}
	else
	{
		char msg[512];
		smprintf (msg, 512, "Can't write '%s'", fullName);
		theApp.outputError (msg);
		return;
	}
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::LoadDocument( const std::string& _sxfullname )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	try
	{
		theApp.OpenDocumentFile(_sxfullname.c_str());
	}
	catch (NLMISC::Exception &e)
	{
		std::string tmp = std::string(e.what()) + "(" + _sxfullname + ")";
		theApp.m_pMainWnd->MessageBox(tmp.c_str(), "Georges_Lib", MB_ICONERROR | MB_OK);
	}
}

// ---------------------------------------------------------------------------
/*void CGeorgesImpl::SaveDocument( const std::string& _sxfullname )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesEditApp* pApp = dynamic_cast< CGeorgesEditApp* >( AfxGetApp() );
	CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( ( (CMainFrame*)theApp.m_pMainWnd )->GetActiveDocument() );
	pgdoc->OnSaveDocument( _sxfullname.c_str() );
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::SaveAllDocument()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesEditApp* papp = dynamic_cast< CGeorgesEditApp* >( AfxGetApp() );
	theApp.SaveAllDocument();
}*/

// ---------------------------------------------------------------------------
void CGeorgesImpl::NewDocument( const std::string& _sxdfnname)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	/*CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( ( (CMainFrame*)theApp.m_pMainWnd )->GetActiveDocument() );
	pgdoc->NewDocument( _sxdfnname );*/
}

// ---------------------------------------------------------------------------
/*void CGeorgesImpl::NewDocument()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesEditApp* pApp = dynamic_cast< CGeorgesEditApp* >( AfxGetApp() );
	CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( ( (CMainFrame*)theApp.m_pMainWnd )->GetActiveDocument() );
	pgdoc->OnNewDocument();
}*/

// ---------------------------------------------------------------------------
/*void CGeorgesImpl::CloseDocument()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesEditApp* pApp = dynamic_cast< CGeorgesEditApp* >( AfxGetApp() );
	CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( ( (CMainFrame*)theApp.m_pMainWnd )->GetActiveDocument() );
	pgdoc->OnCloseDocument();
}

// ---------------------------------------------------------------------------
void CGeorgesImpl::CloseAllDocument()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesEditApp* pApp = dynamic_cast< CGeorgesEditApp* >( AfxGetApp() );
	theApp.CloseAllDocument();
}*/

// ---------------------------------------------------------------------------
std::string CGeorgesImpl::GetDirDfnTyp ()
{
	return theApp.RootSearchPath;
}

// *******
// STATICS
// *******

// ---------------------------------------------------------------------------
void IGeorges::releaseInterface (IGeorges* pGeorges)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	delete pGeorges;

}

// ---------------------------------------------------------------------------
IGeorges* IGeorges::getInterface (int version)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Check version number
	if (version != GEORGES_VERSION)
	{
		MessageBox (NULL, "Bad version of georges.dll.", "Georges", MB_ICONEXCLAMATION|MB_OK);
		return NULL;
	}
	else
		return new CGeorgesImpl;

}

// ---------------------------------------------------------------------------
IGeorges* IGeorgesGetInterface (int version)
{
	return IGeorges::getInterface (version);

}

// ---------------------------------------------------------------------------
void IGeorgesReleaseInterface (IGeorges* pGeorges)
{
	IGeorges::releaseInterface (pGeorges);

}
