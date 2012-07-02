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

// world_editor_doc.h : interface of the CWorldEditorDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORLD_EDITOR_DOC_H__79C92BF7_274F_4B2C_A546_91B7E5218C49__INCLUDED_)
#define AFX_WORLD_EDITOR_DOC_H__79C92BF7_274F_4B2C_A546_91B7E5218C49__INCLUDED_

#include <nel/ligo/primitive_class.h>
#include <nel/ligo/zone_region.h>
#include <hash_set>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Data
struct CLigoData
{
	uint8			PosX;
	uint8			PosY;
	uint8			Rot;
	uint8			Flip;
	std::string		ZoneName;
	std::string		SharingMatNames[4];
	uint8			SharingCutEdges[4];
	bool operator!= (const CLigoData& other) const
	{
		return (PosX != other.PosX) ||
			(PosY != other.PosY) ||
			(Rot != other.Rot) ||
			(Flip != other.Flip) ||
			(ZoneName != other.ZoneName) ||
			(SharingMatNames[0] != other.SharingMatNames[0]) ||
			(SharingMatNames[1] != other.SharingMatNames[1]) ||
			(SharingMatNames[2] != other.SharingMatNames[2]) ||
			(SharingMatNames[3] != other.SharingMatNames[3]) ||
			(SharingCutEdges[0] != other.SharingCutEdges[0]) ||
			(SharingCutEdges[1] != other.SharingCutEdges[1]) ||
			(SharingCutEdges[2] != other.SharingCutEdges[2]) ||
			(SharingCutEdges[3] != other.SharingCutEdges[3]);
	}
};

// Primitive locator
class CDatabaseLocator
{
	friend class CWorldEditorDoc;
	friend class CDatabaseLocatorPointer;
public:
	// Default cstor, reserve some room
	CDatabaseLocator ()
	{
		_LocateStack.reserve (10);
		_LocateStack.resize (1);
		XSubPrim = 0xffffffff;
		Y = 0xffffffff;
	}
	CDatabaseLocator (uint region, sint32 x, sint32 y);

	bool						operator== (const CDatabaseLocator &other) const;
	bool						operator< (const CDatabaseLocator &other) const
	{
		const uint size = (uint)_LocateStack.size ();
		const uint otherSize = (uint)other._LocateStack.size ();
		const uint minSize = std::min (size, otherSize);
		uint i;
		for (i=0; i<minSize; i++)
		{
			if (_LocateStack[i] < other._LocateStack[i])
				return true;
			if (_LocateStack[i] > other._LocateStack[i])
				return false;
		}
		return (size < otherSize);
	}

	uint						getDatabaseIndex () const;
	virtual void				getParent ();

	std::string					getPathName () const;

public:
	// Region X, Y
	sint32						XSubPrim, Y;

protected:
	// The locate stack
	std::vector<uint>	_LocateStack;
};

// Primitive pointer
class CDatabaseLocatorPointer : public CDatabaseLocator
{
public:

	bool						next ();
	void						getRoot (uint i);
	void						getRoot (const std::string& rootFileName);
	std::string&				getRootFileName(NLLIGO::IPrimitive* rootNode);
	bool						firstChild ();
	bool						nextChild ();
	bool						previousChild ();
	virtual void				getParent ();
	void						appendChild (CDatabaseLocator &dest) const;
	bool						operator== (const CDatabaseLocatorPointer &other) const;

	const NLLIGO::IPrimitive	*Primitive;
};

class CWorldEditorDoc : public CDocument
{
	friend class CDatabaseLocator;
	friend class CDatabaseLocatorPointer;
	friend class CActionLigoTile;
	friend class CActionLigoMove;
	friend class CActionLigoResize;
	friend class CActionImportPrimitive;
	friend class CActionLoadPrimitive;
	friend class CActionNewPrimitive;
	friend class CActionClearPrimitives;
	friend class CActionDelete;
	friend class CActionAddPrimitiveByClass;
	friend class CActionAddLandscape;
	friend class CActionDeleteDatabaseElement;
	friend class CActionAddPrimitive;
	friend class CToolsLogicTree;
	friend class CMainFrame;
	friend class CActionNewLandscape;
	friend class CActionXChgDatabaseElement;
protected: // create from serialization only
	CWorldEditorDoc();
	DECLARE_DYNCREATE(CWorldEditorDoc)

// Attributes
public:

// Methods

	// Get the data directory
	const std::string	&getDataDir () const;

	// Get the default save path
	void				getFilePath(uint primIndex,std::string&);

	// Set the data directory
	void				setDataDir (const char *dir);

	// Get the path of selected primitive
	const std::string	&getPathOfSelectedPrimitive() const;

	// Set the path of selected primitive
	void				setPathOfSelectedPrimitive(const std::string&);

	// Serial
	void				serial (NLMISC::IStream &s);

	// Modify the document
	void				beginModification ();

	// Add a modification at the document in a undo/redo session. Return false if the redo method has failed and the action is deleted.
	bool				addModification (class IAction *action);

	void				endModification ();

	bool				inModificationMode() const {return _ModificationMode;}

	// Notify the document it as been modified without possible undo 
	void				noUndoModification ();

	// Notify an undo / redo on in ligo mode
	void				redo ();
	void				undo ();

	// Is undo / redo available ?
	bool				undoAvailable () const;
	bool				redoAvailable () const;

	// Notify modification

	void				modifyDatabase (uint dbIndex);
	void				modifyProject ();

	// *** Acces Zone region

	uint	regionIDToDatabaseElementID (uint landscape) const;
	const NLLIGO::CZoneRegion &getZoneRegion (uint landscape) const;
	const NLLIGO::CZoneRegion &getZoneRegionAbsolute (uint landscape) const;

	// Get ligo data at a specific locator
	void				getLigoData (CLigoData &data, const CDatabaseLocator &locator);

	// Get a specific zone among all zones
	bool				getZoneAmongRegions (CDatabaseLocator &locator, class CBuilderZoneRegion*& pBZRfrom, sint32 x, sint32 y);

	// *** Acces Prim region

public:

	// Get primitives count
	uint				getNumDatabaseElement () const;

	// Get a primitive filename
	const std::string	&getDatabaseElement (uint primitive) const;

	// Get a primitive
	const NLLIGO::CPrimitives &getDatabaseElements (uint primitive) const;

	// Get a primitive formated name
	void				getPrimitiveDisplayName (std::string &result, uint primitive) const;

	// Is a primitive already loaded in the document ?
	bool				isPrimitiveLoaded(const std::string &primPath);

	// Is a landscape ?
	bool				isLandscape (uint dbIndex) const;

	// Is a primitive ?
	bool				isPrimitive (uint dbIndex) const;

	// Is an editable primtive ?
	bool				isEditable (uint dbIndex) const;

	// Update default values
	void				updateDefaultValues (uint dnIndex);

	// Reset default values. onlyZero flag reset id only iof id is 0
	void				resetUniqueID (const NLLIGO::IPrimitive &primitive, bool onlyZero = false);

	struct TPropertyNonUnique
	{
		const NLLIGO::IPrimitive	*Primitive;
		std::string					PropertyName;
	};

	// Regenerate any ID that is not unique (i.e. any id that is found in the received hash_set). 
	// All the valid or regenerated ID are then inserted in the hash_set
	void				forceIDUniqueness(const NLLIGO::IPrimitive &primitive, CHashSet<std::string> &ids, std::vector<TPropertyNonUnique> &nonUnique);

private:
	bool				updateDefaultValuesInternal (NLLIGO::IPrimitive &primitive);

private:

	// Delete a primitive, for actions only
	void deletePrimitive (const CDatabaseLocator &locator);

	// Insert a primitive, for actions only
	void insertPrimitive (const CDatabaseLocator &locator, NLLIGO::IPrimitive *primitive);

	// Create a primitive, for actions only
	const NLLIGO::IPrimitive *createPrimitive (const CDatabaseLocator &locator, const char *className, const char *primName, 
		const NLMISC::CVector &initPos, float deltaPos, const std::vector<NLLIGO::CPrimitiveClass::CInitParameters> &initParameters);

	// Init a primitive parameters
	void initPrimitiveParameters (const NLLIGO::CPrimitiveClass &primClass, NLLIGO::IPrimitive &primitive,
									const std::vector<NLLIGO::CPrimitiveClass::CInitParameters> &initParameters);


	// Get a locator with a primitive
public:
	void				getLocator (CDatabaseLocatorPointer &locator, const NLLIGO::IPrimitive *primitive) const;
	void				getLocator (CDatabaseLocatorPointer &locatorDest, const CDatabaseLocator &locator) const;
	void				getFirstLocator (CDatabaseLocatorPointer &locator) const;

	// *** File modification
	void				updateFiles ();
	static bool			checkFileDate (const char *filename, uint32 date);

/*	// Get ligo data at a specific locator
	void				getLigoData (CLigoData &data, const CDatabaseLocator &locator);

	// Get a specific zone among all zones
	bool				getZoneAmongRegions (CDatabaseLocator &locator, class CBuilderZoneRegion*& pBZRfrom, sint32 x, sint32 y);*/

	// Get document context
	const std::string	&getContext () const 
	{
		return _Context;
	}

	// Get document context
	void setContext (const char *context)
	{
		_Context = context;
	}


	void modifyPropertyDlg();

private:

	// Data directory
	std::string			_DataDir;

	// Document context
	std::string			_Context;

	// Path of selected primitive
	std::string			_PathOfSelectedPrimitive;

	// Set the modified mode for the document
	void				updateModifiedState ();

	// Clear changes
	void				clearModifications ();

	// List of landscape, deprecated, for compatibility
	struct CLandscapeDeprecated
	{
		CLandscapeDeprecated ()
		{
			Modified = false;
		}
		std::string				Filename;
		NLLIGO::CZoneRegion		ZoneRegion;
		void					serial (NLMISC::IStream &s);
		bool					Modified;
		uint32					LastModifedTime;
	};

	// List of primitive, deprecated, for compatibility
	struct CPrimitiveDeprecated
	{
		CPrimitiveDeprecated ()
		{
			Modified = false;
		}
		std::string				Filename;
		NLLIGO::CPrimitives		Primitives;
		void					serial (NLMISC::IStream &s);
		bool					Modified;
		uint32					LastModifedTime;
	};
	
	// Database
	class CDatabaseElement
	{
	public:
		// Landscape or primitive ?
		enum TType
		{
			Landscape,
			Primitive,
			Undefined
		};

		// Constructor
		CDatabaseElement ();
		CDatabaseElement (TType type);

		// Landscape or primitive ?
		TType					Type;

		// Global
		std::string				Filename;
		void					serial (NLMISC::IStream &s);
		bool					Modified;
		bool					Editable;
		uint32					LastModifedTime;

		// Landscape
		NLLIGO::CZoneRegion		ZoneRegion;

		// Primitives
		NLLIGO::CPrimitives		Primitives;

		// set of used alias in the primitives
		std::set<uint32>		UsedAliases;
	};

	// The database
	class CDatabaseList : public std::list<CDatabaseElement>
	{
	public:
		const CDatabaseElement& operator[] (uint index) const
		{
			return *(_PointerArray[index]);
		}
		CDatabaseElement& operator[] (uint index)
		{
			return *(_PointerArray[index]);
		}

		// Recompute pointer array
		void recomputePointerArray ()
		{
			_PointerArray.clear ();
			std::list<CDatabaseElement>::iterator ite = begin ();
			while (ite != end ())
			{
				_PointerArray.push_back (&(*ite));
				ite++;
			}
		}

	private:
		// The pointer array
		std::vector<CDatabaseElement*>	_PointerArray;
	};

	CDatabaseList				_DataHierarchy;

	// Undo / redo / modification flags
	std::vector< std::vector<IAction*> >		_Actions;
	std::vector<IAction*>		_CurrentAction;
	bool						_NoModificationUndo;
	uint						_Undo;
	uint						_LastSaveUndo;
	bool						_ModificationMode;
	bool						_Modified;
	uint32						_LastModifedTime;


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldEditorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

private:
	bool	newDocument ();
	bool	loadDocument (const char *filename);

// Implementation
public:
	virtual ~CWorldEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWorldEditorDoc)
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CWorldEditorDoc *getDocument ();

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORLD_EDITOR_DOC_H__79C92BF7_274F_4B2C_A546_91B7E5218C49__INCLUDED_)
