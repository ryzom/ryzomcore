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

// main_frm.h : interface of the CMainFrame class
//
// ***************************************************************************


#if !defined(AFX_MAIN_FRM_H__1647027B_EE0B_4857_B290_B87FB521A91C__INCLUDED_)
#define AFX_MAIN_FRM_H__1647027B_EE0B_4857_B290_B87FB521A91C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "builder_zone.h"
#include "plugin_interface.h"
#include "world_editor_doc.h"
#include "find_primitive_dlg.h"
#include "goto_dialog.h"

#define DELTA_POS_ADD_PRIMITIVE 20

// ***************************************************************************

class IMasterCB;
class CLoadingDialog;
class CDisplay;

// ***************************************************************************

struct SType 
{
	std::string		Name;
	NLMISC::CRGBA	Color;
	// -----------------------------
	void serial (NLMISC::IStream&f);
};

// ***************************************************************************

struct SEnvironnement
{
	std::vector<SType>	Types;
	NLMISC::CRGBA		BackgroundColor;
	// SExportOptions		ExportOptions;
	std::string			DataDir;
	// -----------------------------
	SEnvironnement();
	void serial (NLMISC::IStream&f);
};

// ***************************************************************************

class CMainFrame : public CFrameWnd, public IPluginAccess 
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

	CSplitterWnd m_wndSplitter;

	// Transformation mode
	enum TTransformMode
	{
		Select = 0,
		Move,
		Rotate,
		Turn,
		Scale,
		Radius,
		AddPoint
	};

private:
	TTransformMode				_TransformModes[2];

	// Width of the tool panel
	LONG						_ToolColumnSize;

	// Show landscape
	bool						_ShowLandscape;

	// Show layers
	bool						_ShowLayers;

	// Show grid
	bool						_ShowGrid;

	// Show points
	bool						_ShowPoints;

	// Show PACS
	bool						_ShowPACS;

	// Show primitives
	bool						_ShowPrimitives;

	// Show details
	bool						_ShowDetails;

	// Show collisions
	bool						_ShowCollisions;

public:

	// Timer enabled
	bool						TimerEnabled;

// Methods

	// Show landscape ?
	bool						showLandscape () const
	{
		return _ShowLandscape;
	}

	// Show layers ?
	bool						showLayers () const
	{
		return _ShowLayers;
	}

	// Show grid ?
	bool						showGrid () const
	{
		return _ShowGrid;
	}

	// Show points ?
	bool						showPoints () const
	{
		return _ShowPoints;
	}

	// Show PACS ?
	bool						showPACS () const
	{
		return _ShowPACS;
	}

	// Show primitives ?
	bool						showPrimitives () const
	{
		return _ShowPrimitives;
	}

	// Show details ?
	bool						showDetails () const
	{
		return _ShowDetails;
	}

	// Show collisions ?
	bool						showCollisions () const
	{
		return _ShowCollisions;
	}

	// Transform methods
	inline TTransformMode		getTransformMode () const
	{
		return _TransformModes[(uint)_SelectionLocked];
	}
	void						setTransformMode (TTransformMode mode);

	// Init the landscape data
	bool						initLandscapeData ();
	class CWorldEditorDoc		*getDocument ();

	// Flush current messages
	void						flushMessages ();

private:
	// Selection data
	bool						_ValidSelection;
	bool						_SelectionLocked;

	// Landscape validate
	bool						_ValidLandscape;
public:

	// Selection locked ?
	bool						isSelectionLocked () const 
	{
		return _SelectionLocked;
	}

	// Selection locked ?
	void						setSelectionLocked (bool lock);

	// Delete selected primitives
	void						deletePrimitive (bool subDelete, const char *actionName);

	// Create a context menu for selection
	void						createContextMenu (CWnd *parent, const CPoint &point, bool transformMode);

	// Display info in the status bar
	void						displayInfo (const char *info);

	// Disactive / enable interaction
	void						interaction (bool enable);

	// is interaction enabled, disactived ?
	bool						isInteraction () const;

	// Copy, cut, paste
private:
	std::vector<NLLIGO::IPrimitive*>	_PrimitiveClipboard;

	// Delete the primitive clipboard
	void						deletePrimitiveClipboard ();

	// Get the filenames openable in a primitive
	void						buildFilenameVector (const NLLIGO::IPrimitive &primitive, std::vector<std::string> &dest);

public:

	/* Old trap Start */

	CLoadingDialog		*LoadingDialog;
	CDisplay			*DispWnd;

	int					CreateX, CreateY, CreateCX, CreateCY;

	bool				_Exit;
	bool				_SplitterCreated;
	CBuilderZone		*_ZoneBuilder;
	sint32				_Mode;	// 0-Mode Zone, 1-Mode Logic 2-Mode Trans, 3-Plugin position control
	/// Remenber mode when plugin take control.
	sint32				_LastMode;
	std::string			_ExeDir; // WorldEditor directory
	IMasterCB			*_MasterCB;

	//std::vector<CType>	_Types;
	SEnvironnement		_Environnement;

	/// The plugin that control the position editor (NULL is none)
	IPluginCallback		*_CurrentPlugin;

	/// The controled position
	NLMISC::CVector		_PositionControl;

	void setExeDir (const char* str);
	void setDataDir (const char* str);
	void uninitTools();
	void initTools();
	void invalidateLandscape ();
	void invalidateTools();
	void invalidateToolsParam();
	void invalidateModification ();
	void updateData ();

	void primZoneModified();

	// Initialize the main frame (must be done before init of the tools)
	bool init (bool bMakeAZone = true);
	void uninit ();

	void displayStatusBarInfo ();
	void adjustSplitter();

	void launchLoadingDialog (const std::string &sText);
	void progressLoadingDialog (float progress, bool flushMessages = true);
	void terminateLoadingDialog ();

	// void viewLand(bool withIgs);

	NLMISC::CConfigFile &getConfigFile();
	CWnd *getMainWindow()					{ return this; }

	// from IPluginAccess
	bool yesNoMessage (const char *format, ... );
	void errorMessage (const char *format, ... );
	void infoMessage (const char *format, ... );
	void startPositionControl(IPluginCallback *plugin, const NLMISC::CVector &initPos);
	void stopPositionControl(IPluginCallback *plugin);
	// functions to create/remove the Root Primitive for server actions
	virtual NLLIGO::IPrimitive *createRootPluginPrimitive (const char *name);
	virtual void deleteRootPluginPrimitive (void);
	virtual void getAllRootPluginPrimitive (std::vector<NLLIGO::IPrimitive*> &prims);
	/*
	 *	The players or information coming from the server are considered as primitives by the WorldEditor
	 *   so we need to create/delete/modify these primitives
	 */
	// Create a plugin primitive
	const NLLIGO::IPrimitive *createPluginPrimitive (
		const char *className, 
		const char *primName, 
		const NLMISC::CVector &initPos, 
		float deltaPos, 
		const std::vector<NLLIGO::CPrimitiveClass::CInitParameters> &initParameters,
		NLLIGO::IPrimitive *parent
		);

	// delete a plugin primitive
	void deletePluginPrimitive (const NLLIGO::IPrimitive *primitive);

	// indicates to the WorldEditor that the primitive has changed
	void invalidatePluginPrimitive (const NLLIGO::IPrimitive *primitive, uint channels);

	// invalidate the left view completely
	virtual void invalidateLeftView();


	// get the display window coordinates
	void	getWindowCoordinates(NLMISC::CVector &vmin, NLMISC::CVector &vmax);

	virtual const std::list<NLLIGO::IPrimitive*>	&getCurrentSelection();

	virtual bool isSelected(const NLLIGO::IPrimitive &prim) const;

	void setCurrentSelection(std::vector<NLLIGO::IPrimitive*>& );

	virtual const NLLIGO::IPrimitive* getRootNode(const std::string&);

	virtual std::string& getRootFileName(NLLIGO::IPrimitive*);

	virtual void registerPrimitiveDisplayer(IPrimitiveDisplayer *displayer, const std::vector<std::string> &primClassNames);

	virtual const std::list<NLLIGO::IPrimitive*> &getCurrentSelection() const;

	virtual void refreshPropertyDialog() { _MustRefreshPropertyDialog = true; }


	virtual std::string sheetIdToSheetName(NLMISC::CSheetId sheetID) const;

	virtual void setPrimitiveHideFlag(NLLIGO::IPrimitive &prim, bool hidden);

	virtual CPrimTexture *createTexture();

	// Delete a texture object
	virtual void deleteTexture(CPrimTexture *tex);

	// from IPluginAccess
	virtual bool buildNLBitmapFromTGARsc(HRSRC bm,HMODULE hm,NLMISC::CBitmap &dest);

	bool CanDrop();


	/* Old Trap End */ 

// Operations
public:

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CReBar      m_wndReBar;
	CDialogBar  m_wndDlgBar;
	CFindPrimitiveDlg *m_FindPrimitiveDlg;

	CGotoDialog	m_GotoDlg;

	bool _MustRefreshPropertyDialog;

// Generated message map functions
protected:

	void onLogicChanged(const std::vector<NLLIGO::CPrimRegion*> &regions);

public:

	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose ();
	afx_msg void OnSize (UINT nType, int cx, int cy);
	afx_msg void onMenuFileExit ();
	afx_msg void onMenuFileNewLogic ();
	afx_msg void onMenuFileUnloadLogic ();
	afx_msg void onMenuFileOpenLogic ();
	afx_msg void onMenuFileNewLandscape ();
	afx_msg void onMenuFileUnloadLandscape ();
	afx_msg void onMenuModeLogic ();
	afx_msg void onMenuViewBackground ();
	afx_msg void onMenuModeZone ();
	afx_msg void onMenuModeTransition ();
	afx_msg void onMenuViewGrid ();
	afx_msg void onMenuModeUndo ();
	afx_msg void onMenuModeRedo ();
	afx_msg void onUpdateModeUndo (CCmdUI* pCmdUI);
	afx_msg void onUpdateModeRedo (CCmdUI* pCmdUI);
	afx_msg void onProjectNewlandscape();
	afx_msg void onProjectAddlandscape();
	afx_msg void onProjectSettings();
	afx_msg void OnUpdateEditTransition(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditZone(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditLogic(CCmdUI* pCmdUI);
	afx_msg void OnProjectImportPrim();
	afx_msg void OnProjectAddPrimitive();
	afx_msg void OnProjectClearallprimitive();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditLock();
	afx_msg void OnEditSelect();
	afx_msg void OnEditDetails();
	afx_msg void OnViewLandscape();
	afx_msg void OnViewLayers();
	afx_msg void OnViewGrid();
	afx_msg void OnViewPoints();
	afx_msg void OnViewPACS();
	afx_msg void OnViewPrimitives();
	afx_msg void OnEditTranslate();
	afx_msg void OnEditRotate();
	afx_msg void OnEditTurn();
	afx_msg void OnEditRadius();
	afx_msg void OnEditScale();
	afx_msg void OnEditAddPoint();
	afx_msg void OnEditDelete();
	afx_msg void OnProjectNewPrimitive();
	afx_msg void OnEditProperties();
	afx_msg void OnAddPrimitive (UINT nID);
	afx_msg void OnGeneratePrimitive (UINT nID);
	afx_msg void OnUpdateEditAddPoint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditLock(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditProperties(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRotate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditScale(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditTranslate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditTurn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRadius(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditSelect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDetails(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewLandscape(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewHide(CCmdUI* pCmdUI);
	afx_msg void OnViewHide();
	afx_msg void OnViewShow();
	afx_msg void OnUpdateViewShow(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditExpand(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCollapse(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectChildren();
	afx_msg void OnUpdateEditSelectChildren(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEditExpand ();
	afx_msg void OnEditCollapse ();
	afx_msg void OnHelpFinder();
	afx_msg void OnOpenFile (UINT nID);
	afx_msg void OnViewLocateselectedprimitives();
	afx_msg void OnUpdateViewLocateselectedprimitives(CCmdUI* pCmdUI);
	afx_msg void OnViewLocateselectedprimitivesTree();
	afx_msg void OnUpdateViewLocateselectedprimitivesTree(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectByLocation();
	afx_msg void OnUpdateEditSelectByLocation(CCmdUI* pCmdUI);
	afx_msg void OnProjectResetuniqueid();
	afx_msg void OnProjectGeneratenullid();
	afx_msg void OnProjectForceiduniqueness();
	afx_msg void OnEditFind();
	afx_msg void OnEditGoto();
	afx_msg void OnUpdateFind(CCmdUI* pCmdUI);
	afx_msg void OnViewCollisions();
	afx_msg void OnHelpHistory();
	afx_msg void OnExportSnapshot();
	afx_msg void OnWindowsPrimitiveconfiguration();
	afx_msg void OnUpdateWindowsPrimitiveconfiguration(CCmdUI* pCmdUI);
	afx_msg void OnProjectResetPrimitiveConfiguration();
	afx_msg void OnDestroy();
	afx_msg void OnSavePosition();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnMissionCompiler();
	afx_msg void OnNameDlg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ***************************************************************************

class CLoadingDialog : public CDialog
{

public:

	CLoadingDialog (CWnd*pParent);

	void setText (const std::string &text);
	void setProgress (float progress);
};

// ***************************************************************************

// Standard callback
class CWorldEditorProgressCallback : public NLMISC::IProgressCallback
{
public:
	CWorldEditorProgressCallback ();
	~CWorldEditorProgressCallback ();
	void progress (float value);
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAIN_FRM_H__1647027B_EE0B_4857_B290_B87FB521A91C__INCLUDED_)
