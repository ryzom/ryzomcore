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

#include "stdafx.h"

#include "editor_primitive.h"
#include "world_editor.h"
#include "tools_logic.h"
#include "dialog_properties.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NL3D;
using namespace NLGEORGES;

// ***************************************************************************

/*

Implementation notes:
---------------------

- Quad tree channels

	- Thoses actions must update the primitives quad grid iterator :
		+ Load project file (loadDocument)
		+ Import primitive file (CActionImportPrimitive)
		+ Load primitive file (CActionLoadPrimitive)
		+ New primitive file (CActionNewPrimitive)
		+ Clear primitive files (CActionClearPrimitives)
		+ Delete primitive file (CActionDeleteDatabaseElement)
		+ New primitive (CActionAddPrimitiveByClass)
		+ Delete prim (CActionDelete)
		+ Modify a primitive
			+ Move (CActionMove)
			+ Rotate (CActionRotate)
			+ Scale (CActionScale)
			+ Move points (CActionMove)
			+ Rotate points (CActionRotate)
			+ Scale points (CActionScale)
			+ Add points (CActionAddVertex)
			+ Delete points (CActionDeleteSub)
		+ Generate primitive (CActionAddPrimitive)
		+ Paste primitives (CActionAddPrimitive)
		+ Move primitive (CActionAddPrimitive)
		- Show primitive
		- Hide primitive

	- So, thoses actions classes must update the primitives quad grid iterator in there undo / redo methods :

		+ CActionMove
			InvalidatePrimitive / InvalidatePrimitive
		+ CActionRotate
			InvalidatePrimitive / ModifiedPrimitive
		+ CActionScale
			ModifiedPrimitive / ModifiedPrimitive
		+ CActionAddVertex
			ModifiedPrimitive / ModifiedPrimitive
		+ CActionDeleteSub
			ModifiedPrimitive / ModifiedPrimitive
		+ CActionDelete
			~IPrimitiveEditor / ModifiedPrimitive
		+ CActionAddPrimitiveByClass
			ModifiedPrimitive / ~IPrimitiveEditor 
		+ CActionAddPrimitive
			ModifiedPrimitive / ~IPrimitiveEditor 

		+ CActionDeleteDatabaseElement
			~IPrimitiveEditor  / InvalidatePrimitiveQuadGridRec
		+ CActionClearPrimitives
			~IPrimitiveEditor  / InvalidatePrimitiveQuadGridRec
		+ CActionNewPrimitive
			InvalidatePrimitiveQuadGridRec / ~IPrimitiveEditor 
		+ CActionImportPrimitive
			InvalidatePrimitiveQuadGridRec / ~IPrimitiveEditor 
		+ CActionLoadPrimitive
			InvalidatePrimitiveQuadGridRec / ~IPrimitiveEditor 

	- Those methods must invalidate primitive quad grid too

		- loadDocument
			InvalidatePrimitiveQuadGridRec 

	- Those methods use the quad grid to select primitives

		- CDisplay::OnDraw
		- CDisplay::pickPoint
		- CDisplay::pickRect

- Logic tree structure channel

	- Thoses actions must update the primitives logic tree structure :
		- Load project file (loadDocument)
		- Import primitive file (CActionImportPrimitive)
		- Load primitive file (CActionLoadPrimitive)
		- New primitive file (CActionNewPrimitive)
		- Clear primitive files (CActionClearPrimitives)
		- Delete primitive file (CActionDeleteDatabaseElement)
		- New primitive (CActionAddPrimitiveByClass)
		- Delete prim (CActionDelete)
		- Generate primitive (CActionAddPrimitive)
		- Paste primitives (CActionAddPrimitive)
		- Move primitive (CActionAddPrimitive)

- Logic tree parameters channel

  	- Thoses actions must update the primitives logic tree parameters :
		- Modify a primitive
			- Modify properties ()
		- Show primitive
		- Hide primitive


A modified primitive linked list is used to invalid primitive quad grid iterator only once.
In the global update, a call to UpdateQuadGrid () is done to update primitive in the linked list.
*/

// ***************************************************************************
// The primitive quad grid
// ***************************************************************************

TPrimQuadGrid PrimitiveQuadGrid;

// ***************************************************************************

// Linked list of modified primitive
std::list<IPrimitive*>	ModifiedPrimitive;

// ***************************************************************************

// Linked list of selected primitive
std::list<IPrimitive*> Selection;

// ***************************************************************************

void InitQuadGrid ()
{
	// todo : Put those hard coded quadtree values in parameters
	PrimitiveQuadGrid.create (256, 100.f);
}

// ***************************************************************************

//inline IPrimitiveEditor *getPrimitiveEditor (const IPrimitive *primitive)
//{
////#ifdef NL_DEBUG
//		return dynamic_cast<IPrimitiveEditor*> (const_cast<IPrimitive*> (primitive));
////#else
////		return (IPrimitiveEditor*)(CPrimNodeEditor*)primitive;
////#endif
//}

// ***************************************************************************

void AddPrimitivesLogicTree (const CDatabaseLocatorPointer &locator, IPrimitiveEditor *primitiveEditor, CToolsLogic *toolWnd)
{
	if (toolWnd)
	{
		// My child id
		HTREEITEM parentItem = TVI_ROOT;
		HTREEITEM lastBrother = TVI_LAST;

		// Get its parent
		const IPrimitive *parent = locator.Primitive->getParent ();
		if (parent)
		{
			// Parent locator
			IPrimitiveEditor *parentEditor = getPrimitiveEditor (const_cast<IPrimitive*>(parent));

			// Parent must be in the tree
			parentItem = parentEditor->_TreeItem;
			nlassert (parentEditor->_TreeItem);

			// Insert brothers
			uint childId;
			nlverify (parent->getChildId (childId, locator.Primitive));

			// *** Insert after my first previous & visible brother

			lastBrother = TVI_FIRST;
			while (childId > 0)
			{
				// Previous brother
				const IPrimitive *brother;
				nlverify (parent->getChild (brother, childId-1));

				// Get the brother primitive editor
				const IPrimitiveEditor *brotherEditor = getPrimitiveEditor (brother);

				// Last brother
				if (brotherEditor->_TreeItem)
				{
					lastBrother = brotherEditor->_TreeItem;
					break;
				}
				childId--;
			}
		}
		else
		{
			uint childId = locator.getDatabaseIndex ();

			// *** Insert first my previous brothers

			if (childId == 0)
				lastBrother = TVI_FIRST;
			else if (childId > 0)
			{
				// Get the brother
				CDatabaseLocatorPointer brotherLocator;
				brotherLocator.getRoot (childId-1);

				// Get the brother primitive editor
				const IPrimitiveEditor *brotherEditor = getPrimitiveEditor (brotherLocator.Primitive);

				// Last brother
				lastBrother = brotherEditor->_TreeItem;
				nlassert (lastBrother);
			}
		}

		// Add it
		nlassert (primitiveEditor->_TreeItem == NULL);
		const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*locator.Primitive);
		if (!primClass || primClass->Visible)
			primitiveEditor->_TreeItem = toolWnd->addPrimitive (parentItem, lastBrother, locator);

		// Update tree item parameters
		toolWnd->updatePrimitive (primitiveEditor->_TreeItem, locator);
	}

	// Validate
	primitiveEditor->_Channels &= ~(LogicTreeStruct|LogicTreeParam);
}

// ***************************************************************************

void UpdatePrimitiveSelection (IPrimitiveEditor *primitiveEditor, CDatabaseLocatorPointer &locator, bool &modified)
{
	// Is selected ?
//	IProperty *prop;
	if (getPrimitiveEditor(locator.Primitive)->getSelected())
//	if (locator.Primitive->getPropertyByName ("selected", prop))
	{
		// In the list ?
		if (primitiveEditor->_SelectionIterator == Selection.end ())
		{
			// Add it to the selection list
			Selection.push_front (const_cast<IPrimitive*> (locator.Primitive));

			// Get the iterator
			primitiveEditor->_SelectionIterator = Selection.begin ();
			modified = true;
		}
	}
	else
	{
		// In the list ?
		if (primitiveEditor->_SelectionIterator != Selection.end ())
		{
			// Remove it
			primitiveEditor->removeFromSelection ();
			modified = true;
		}
	}
}

// ***************************************************************************

void UpdatePrimitives ()
{
	// Get the tools window
	CMainFrame *mainWnd = getMainFrame ();
	if (mainWnd)
	{
		CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(getMainFrame ()->m_wndSplitter.GetPane(0,1));

		// Sort the list
		static vector<CDatabaseLocatorPointer>	toSort;
		toSort.clear ();

		CWorldEditorDoc *doc = getDocument ();
		std::list<NLLIGO::IPrimitive*>::iterator ite = ModifiedPrimitive.begin ();
		while (ite != ModifiedPrimitive.end ())
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, *ite);
			toSort.push_back (locator);
			ite++;
		}
		sort (toSort.begin (), toSort.end ());

		// For each modified primitive 
		sint i;
		sint count = (sint)toSort.size ();
		for (i=count-1; i>=0; i--)
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, toSort[i].Primitive);
			IPrimitiveEditor *primitiveEditor = getPrimitiveEditor (const_cast<IPrimitive*>(toSort[i].Primitive));

			// Logic tree structure modified ?
			if (primitiveEditor->_Channels & LogicTreeStruct)
			{
				// Remove from the tree
				primitiveEditor->removeFromLogicTree ();
			}
		}

		// Selection is changed ?
		bool selectionChanged = false;

		// For each modified primitive 
		for (i=0; i<count; i++)
		{
			const IPrimitive *primitive = toSort[i].Primitive;
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, primitive);
			IPrimitiveEditor *primitiveEditor = getPrimitiveEditor (const_cast<IPrimitive*>(primitive));

			// Quad grid ?
			if (primitiveEditor->_Channels & QuadTree)
			{
				// Remove from the container
				primitiveEditor->removeFromQuadGrid ();

				// Num points
				uint pointCount = (primitive)->getNumVector ();
				if (pointCount > 0)
				{
					// Point pointer
					const CPrimVector *points = (primitive)->getPrimVector ();

					// BBox
					CAABBox	bbox;
					bbox.setCenter (points[0]);

					// Extend the bbox
					uint j;
					for (j=1; j<pointCount; j++)
					{
						bbox.extend (points[j]);
					}

					// Insert in the quadtree
					primitiveEditor->_QuadIterator = PrimitiveQuadGrid.insert (bbox.getMin (), bbox.getMax (), const_cast<IPrimitive*> (primitive));

					// Get the linked primitives
					const IPrimitive* linkedPrimitive = theApp.Config.getLinkedPrimitive (*primitive);

					// Is this primitive linked with another one ?
					if (linkedPrimitive)
					{
						IPrimitiveEditor *primitiveEditorLinked = getPrimitiveEditor (const_cast<IPrimitive*>(linkedPrimitive));
						if (linkedPrimitive->getNumVector () > 0)
						{
							bbox.setCenter (points[0]);
							bbox.extend (linkedPrimitive->getPrimVector ()[0]);

							// Insert in the quadtree
							primitiveEditor->_QuadIteratorLink = PrimitiveQuadGrid.insert (bbox.getMin (), bbox.getMax (), CQuadGridEntry 
								(const_cast<IPrimitive*> (primitive), const_cast<IPrimitive*> (linkedPrimitive)));
						}
					}
				}

				// Validate
				primitiveEditor->_Channels &= ~QuadTree;
			}

			// Logic tree structure ?
			if (primitiveEditor->_Channels & LogicTreeStruct)
			{
				// Add the primitive
				AddPrimitivesLogicTree (locator, primitiveEditor, toolWnd);

				// The flag is validated by AddPrimitivesLogicTree
			}

			// Logic tree parameters ?
			if (primitiveEditor->_Channels & LogicTreeParam)
			{
				// Update tree item parameters
				if (toolWnd)
					toolWnd->updatePrimitive (primitiveEditor->_TreeItem, locator);

				// Validate
				primitiveEditor->_Channels &= ~LogicTreeParam;
			}

			// Selection ?
			if (primitiveEditor->_Channels & _SelectionSelectState)
			{
				// Update the selection
				UpdatePrimitiveSelection (primitiveEditor, locator, selectionChanged);

				// Validate
				primitiveEditor->_Channels &= ~_SelectionSelectState;
			}

			// Remove from the modified list
			nlassert (primitiveEditor->_Channels == 0);
			ModifiedPrimitive.erase (primitiveEditor->_ModifiedIterator);
			primitiveEditor->_ModifiedIterator = ModifiedPrimitive.end ();
		}

		/*// Change dialog selection ?
		if (selectionChanged)
		{
			if ( pDlg )
				pDlg->changeSelection (Selection);
		}
*/
		nlassert (ModifiedPrimitive.size ()==0);
	}
}

// ***************************************************************************

void UpdateSelection ()
{
	// Get the tools window
	CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(getMainFrame ()->m_wndSplitter.GetPane(0,1));

	// Selection is changed ?
	bool selectionChanged = false;

	// For each modified primitive 
	CWorldEditorDoc *doc = getDocument ();
	std::list<NLLIGO::IPrimitive*>::iterator ite = ModifiedPrimitive.begin ();
	while (ite != ModifiedPrimitive.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);
		IPrimitiveEditor *primitiveEditor = getPrimitiveEditor (*ite);

		// Selection ?
		if (primitiveEditor->_Channels & _SelectionSelectState)
		{
			// Update the selection
			UpdatePrimitiveSelection (primitiveEditor, locator, selectionChanged);

			// Validate
			primitiveEditor->_Channels &= ~_SelectionSelectState;
		}

		// Next primitive to update
		ite++;
	}

	/*// Change dialog selection ?
	if (selectionChanged)
	{
		PropertyDialog.changeSelection (Selection);
	}*/
}

// ***************************************************************************

void InvalidatePrimitive (const CDatabaseLocatorPointer &locator, uint channels)
{
	if (locator.Primitive)
	{
		IPrimitiveEditor *primitiveEditor = getPrimitiveEditor (const_cast<IPrimitive*>(locator.Primitive));

		// No, link it
		if (primitiveEditor->_ModifiedIterator == ModifiedPrimitive.end ())
		{
			ModifiedPrimitive.push_front (const_cast<IPrimitive*> (locator.Primitive));
			primitiveEditor->_ModifiedIterator = ModifiedPrimitive.begin ();
		}

		// Add the channel flags
		primitiveEditor->_Channels |= channels;

		// If invalidate QuadTree, check if the primitive is linked and if so, invalidate the linked primitives
		if (channels & QuadTree)
		{
			// Next primitive
			const IPrimitive* linkedPrimitive = theApp.Config.getLinkedPrimitive (*locator.Primitive);
			if (linkedPrimitive)
			{
				IPrimitiveEditor *linkedPrimitiveEditor = getPrimitiveEditor (const_cast<IPrimitive*>(linkedPrimitive));
				if (linkedPrimitiveEditor->_ModifiedIterator == ModifiedPrimitive.end ())
				{
					ModifiedPrimitive.push_front (const_cast<IPrimitive*> (linkedPrimitive));
					linkedPrimitiveEditor->_ModifiedIterator = ModifiedPrimitive.begin ();
				}
				linkedPrimitiveEditor->_Channels |= QuadTree;
			}

			// Previous primitive
			linkedPrimitive = theApp.Config.getPreviousLinkedPrimitive (*locator.Primitive);
			if (linkedPrimitive)
			{
				IPrimitiveEditor *linkedPrimitiveEditor = getPrimitiveEditor (const_cast<IPrimitive*>(linkedPrimitive));
				if (linkedPrimitiveEditor->_ModifiedIterator == ModifiedPrimitive.end ())
				{
					ModifiedPrimitive.push_front (const_cast<IPrimitive*> (linkedPrimitive));
					linkedPrimitiveEditor->_ModifiedIterator = ModifiedPrimitive.begin ();
				}
				linkedPrimitiveEditor->_Channels |= QuadTree;
			}
		}
	}
}

// ***************************************************************************

void InvalidatePrimitiveRec (const CDatabaseLocatorPointer &locator, uint channels)
{
	// Invalidate this one
	InvalidatePrimitive (locator, channels);

	// For each child
	CDatabaseLocatorPointer myLocator = locator;
	if (myLocator.firstChild ())
	{
		do
		{
			InvalidatePrimitiveRec (myLocator, channels);
		}
		while (myLocator.nextChild ());
	}
}

// ***************************************************************************

void InvalidateAllPrimitives ()
{
	// Erase all caches
/*	Selection.clear ();
	PrimitiveQuadGrid.clear ();
	ModifiedPrimitive.clear ();*/
	
	// Invalidate quad grid
	CWorldEditorDoc *doc = getDocument ();
	if (doc)
	{
		uint i;
		uint count = doc->getNumDatabaseElement ();
		for (i=0; i<count; i++)
		{
			// Is a primtive ?
			CDatabaseLocatorPointer locator;
			locator.getRoot (i);
			InvalidatePrimitiveRec (locator, QuadTree|LogicTreeStruct|SelectionState);
		}
	}
}

// ***************************************************************************

void OnModifyPrimitive (CPrimitives &primitives)
{
	// Look for the good database element using the primitives pointer

	CWorldEditorDoc *doc = getDocument ();
	uint i;
	const uint count = doc->getNumDatabaseElement ();
	for (i=0; i<count; i++)
	{
		if (&doc->getDatabaseElements (i) == &primitives)
			break;
	}

	// If found, modify it

	if (i < count)
	{
		doc->modifyDatabase (i);
	}
}

// ***************************************************************************
// IPrimitiveEditor
// ***************************************************************************

IPrimitiveEditor::~IPrimitiveEditor ()
{
	removeFromQuadGrid ();
	removeFromSelection ();
	removeFromModified ();
}

// ***************************************************************************

void IPrimitiveEditor::postReadCallback(const NLLIGO::IPrimitive *thePrim)
{
	// hidden / selected / expanded flags are not readed anymore
	_Hidden   = false;
	_Selected = false;
	_Expanded = false;

	/*
	// code to read flags from file
	const std::string props = thePrim->getUnparsedProperties();

	if (props.find("@selected") != string::npos)
		_Selected = true;
	else
		_Selected = false;

	if (props.find("@expanded") != string::npos)
		_Expanded = true;
	else
		_Expanded = false;

	if (props.find("@hidden") != string::npos)
		_Hidden = true;
	else
		_Hidden = false;
	*/
}

// ***************************************************************************

void IPrimitiveEditor::preWriteCallback(const NLLIGO::IPrimitive *thePrim) const
{
	/*
	// code to write flags from file
	string props;
	if (_Selected)
		props += "@selected";
	if (_Expanded)
		props += "@expanded";
	if (_Hidden)
		props += "@hidden";

	thePrim->setUnparsedProperties(props);
	*/
}


// ***************************************************************************

void IPrimitiveEditor::removeFromQuadGrid ()
{
	// Already inserted ?
	if (_QuadIterator != PrimitiveQuadGrid.end ())
	{
		// Remove from the container
		PrimitiveQuadGrid.erase (_QuadIterator);

		// Clear it
		_QuadIterator = PrimitiveQuadGrid.end ();
	}

	// Link already inserted ?
	if (_QuadIteratorLink != PrimitiveQuadGrid.end ())
	{
		// Remove from the container
		PrimitiveQuadGrid.erase (_QuadIteratorLink);

		// Clear it
		_QuadIteratorLink = PrimitiveQuadGrid.end ();
	}
}

// ***************************************************************************

void IPrimitiveEditor::resetTreeItemRec ()
{
	_TreeItem = NULL;
}

// ***************************************************************************

void IPrimitiveEditor::removeFromLogicTree ()
{
	// Already inserted ?
	if (_TreeItem)
	{
		// Remove from the tree
		CMainFrame *mainFrame = getMainFrame ();
		if (mainFrame)
		{
			CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(mainFrame->m_wndSplitter.GetPane(0,1));
			if (toolWnd)
				toolWnd->GetTreeCtrl ()->DeleteItem (_TreeItem);
		}

		// Null
		resetTreeItemRec ();
	}
}

// ***************************************************************************

void IPrimitiveEditor::removeFromSelection ()
{
	// Already inserted ?
	if (_SelectionIterator != Selection.end ())
	{
		// Remove from the container
		Selection.erase (_SelectionIterator);

		// Clear it
		_SelectionIterator = Selection.end ();
	}
}

// ***************************************************************************

void IPrimitiveEditor::removeFromModified ()
{
	// Already inserted ?
	if (_ModifiedIterator != ModifiedPrimitive.end ())
	{
		// Remove from the container
		ModifiedPrimitive.erase (_ModifiedIterator);

		// Clear it
		_ModifiedIterator = ModifiedPrimitive.end ();
	}
}

// ***************************************************************************
// CPrimNodeEditor
// ***************************************************************************

IPrimitive *CPrimNodeEditor::copy () const
{
	return new CPrimNodeEditor (*this);
}

bool CPrimNodeEditor::read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config)
{
	bool ret = CPrimNode::read(xmlNode, filename, version, config);
	IPrimitiveEditor::postReadCallback(this);
	return ret;
}

void CPrimNodeEditor::write (xmlNodePtr xmlNode, const char *filename) const
{
	IPrimitiveEditor::preWriteCallback(this);
	CPrimNode::write(xmlNode, filename);
}

void CPrimNodeEditor::onModifyPrimitive (CPrimitives &primitives) const
{
	OnModifyPrimitive (primitives);
}


// ***************************************************************************
// CPrimPointEditor
// ***************************************************************************

IPrimitive *CPrimPointEditor::copy () const
{
	return new CPrimPointEditor (*this);
}

bool CPrimPointEditor::read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config)
{
	bool ret = CPrimPoint::read(xmlNode, filename, version, config);
	IPrimitiveEditor::postReadCallback(this);
	return ret;
}

void CPrimPointEditor::write (xmlNodePtr xmlNode, const char *filename) const
{
	IPrimitiveEditor::preWriteCallback(this);
	CPrimPoint::write(xmlNode, filename);
}

void CPrimPointEditor::onModifyPrimitive (CPrimitives &primitives) const
{
	OnModifyPrimitive (primitives);
}

// ***************************************************************************
// CPrimPathEditor
// ***************************************************************************

IPrimitive *CPrimPathEditor::copy () const
{
	return new CPrimPathEditor (*this);
}

bool CPrimPathEditor::read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config)
{
	bool ret = CPrimPath::read(xmlNode, filename, version, config);
	IPrimitiveEditor::postReadCallback(this);
	return ret;
}

void CPrimPathEditor::write (xmlNodePtr xmlNode, const char *filename) const
{
	IPrimitiveEditor::preWriteCallback(this);
	CPrimPath::write(xmlNode, filename);
}

void CPrimPathEditor::onModifyPrimitive (CPrimitives &primitives) const
{
	OnModifyPrimitive (primitives);
}


// ***************************************************************************
// CPrimZoneEditor
// ***************************************************************************

IPrimitive *CPrimZoneEditor::copy () const
{
	return new CPrimZoneEditor (*this);
}

bool CPrimZoneEditor::read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config)
{
	bool ret = CPrimZone::read(xmlNode, filename, version, config);
	IPrimitiveEditor::postReadCallback(this);
	return ret;
}

void CPrimZoneEditor::write (xmlNodePtr xmlNode, const char *filename) const
{
	IPrimitiveEditor::preWriteCallback(this);
	CPrimZone::write(xmlNode, filename);
}

void CPrimZoneEditor::onModifyPrimitive (CPrimitives &primitives) const
{
	OnModifyPrimitive (primitives);
}

// ***************************************************************************
// CPrimAliasEditor
// ***************************************************************************

NLMISC::IClassable	*CPrimAliasEditor::creator() 
{
	return new CPrimAliasEditor;
}

NLLIGO::IPrimitive *CPrimAliasEditor::copy () const
{
	return new CPrimAliasEditor(*this);
}

bool CPrimAliasEditor::read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config)
{
	bool ret = CPrimAlias::read(xmlNode, filename, version, config);
	IPrimitiveEditor::postReadCallback(this);
	return ret;
}

void CPrimAliasEditor::write (xmlNodePtr xmlNode, const char *filename) const
{
	IPrimitiveEditor::preWriteCallback(this);
	CPrimAlias::write(xmlNode, filename);
}

void CPrimAliasEditor::onModifyPrimitive (CPrimitives &primitives) const
{
	OnModifyPrimitive (primitives);
}

// ***************************************************************************
// CPrimBitmap
// ***************************************************************************

CPrimBitmap::CPrimBitmap ()
{
	_Loaded = false;
}

// ***************************************************************************

void CPrimBitmap::init (const char *filename)
{
	// Set the name
	removePropertyByName ("filename");
	addPropertyByName ("filename", new CPropertyString (filename));
}

// ***************************************************************************

NL3D::CTextureBlank *CPrimBitmap::getTexture () const
{
	// Not loaded
	if (!_Loaded)
	{	
		// Get the filename
		string filename;
		if (getPropertyByName ("filename", filename))
		{
			_Texture = new CTextureBlank ();
			_Texture->setWrapS (ITexture::Clamp);
			_Texture->setWrapT (ITexture::Clamp);
			_Texture->setReleasable (false);
			try
			{
				CIFile iFile;
				if (iFile.open (filename))
				{
					// Load the texture
					_Texture->load (iFile);

					// Power of 2 texture
					uint width = raiseToNextPowerOf2 (_Texture->getWidth ());
					uint height = raiseToNextPowerOf2 (_Texture->getHeight());
					if ((width != _Texture->getWidth ()) || (height != _Texture->getHeight ()))
					{
						_Texture->resample (width, height);
					}
				}
				else
				{
					theApp.errorMessage ("Can't read bitmap %s", filename.c_str ());
				}
			}
			catch (Exception &e)
			{
				theApp.errorMessage ("Error reading bitmap %s : %s", filename.c_str (), e.what ());
			}
		}
	}
	_Loaded = true;

	return _Texture;
}

// ***************************************************************************

void VerifyPrimitivesStructures()
{
	getMainFrame()->launchLoadingDialog("checking primitives structures");
	// Invalidate quad grid
	CWorldEditorDoc *doc = getDocument ();
	if (doc)
	{
		// Begin modifications
		doc->beginModification ();

		string path;
		string errors;
		uint i;
		uint count = doc->getNumDatabaseElement ();
		IPrimitiveEditor::StructureError error = IPrimitiveEditor::NoError;
		for (i=0; i<count; i++)
		{
			getMainFrame()->progressLoadingDialog(float(i+0.0001f)/count);
			if (doc->isEditable (i))
			{
				// Is a primtive ?
				CDatabaseLocatorPointer locator;
				locator.getRoot (i);
				error = (IPrimitiveEditor::StructureError)((sint)VerifyPrimitiveRec (locator, path, errors) | (sint)error);
				//VerifyPrimitiveRec (locator);

				// auto generate missing ID
				doc->resetUniqueID(*(doc->getDatabaseElements(i).RootNode), true);
			}
		}
		// End modifications
		doc->endModification ();

		if (error != IPrimitiveEditor::NoError)
		{
			errors = "Found errors in the tree structure:\n"+errors;
			int first = errors.find_first_not_of ("\n");
			int end = errors.find_first_of ("\n");
			while (first != string::npos)
			{
				nlwarning (errors.substr(first, end-first).c_str());
				first = end;
				end = errors.find_first_of ("\n", end+1);
			}

			theApp.errorMessage (errors.c_str());
		}
	}
	getMainFrame()->terminateLoadingDialog();
}

IPrimitiveEditor::StructureError VerifyPrimitiveRec (const CDatabaseLocatorPointer &locator, const string &path, string &errors)
{
	IPrimitiveEditor::StructureError error = IPrimitiveEditor::NoError;
	// For every child, we check the errors
	CDatabaseLocatorPointer myLocator = locator;

	const IPrimitive *primitive = locator.Primitive;
	string newPath = "?";
	if (primitive)
	{
		if (primitive->getParent())
		{
			primitive->getPropertyByName ("name", newPath);
			newPath = ":"+newPath;
		}
		else
		{
			CWorldEditorDoc *doc = getDocument ();
			uint databaseIndex = locator.getDatabaseIndex ();			
			newPath = doc->getDatabaseElement (databaseIndex);
		}
	}
	newPath = path + newPath;

	// we parse the tree recursively, collection errors, and propagating to the parent
	if (myLocator.firstChild ())
	{
		do
		{
			// not very elegant, but we need to set bit fields
			error = (IPrimitiveEditor::StructureError)((sint)VerifyPrimitiveRec (myLocator, newPath, errors) | (sint)error);
		}
		while (myLocator.nextChild ());
	}
	// when we finished to recurse the tree, we simply check if the StaticChildren are all here
	// all other children MUST be either Dynamic or Generated

	if (primitive)
	{
		string className;
		bool Success;
		Success = primitive->getPropertyByName ("class", className);

		if (!Success)
		{
			// is this the root node ?
			const IPrimitive *parent = primitive->getParent();
			if (!parent)
			{
				// if no parent, we have a root node
				className = "root";
				Success = true;
			}
		}
		if (Success)
		{
			const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (className.c_str ());
			if (primClass)
			{
				uint CountStaticChildren = 0;
				uint MissingChildren = 0;
				myLocator = locator;
				// we parse the tree recursively, collection errors, and propagating to the parent
				if (myLocator.firstChild ())
				{
					do
					{
						const IPrimitive *primitive2 = myLocator.Primitive;
						if (primitive2)
						{
							string ClassName2;
							nlverify (primitive2->getPropertyByName ("class", ClassName2));
							string name;
							if (primitive2->getPropertyByName ("name", name))
							{
								uint c;
								bool Found = false;
								for (c=0; c<primClass->StaticChildren.size (); c++)
								{
									if (primClass->StaticChildren[c].ClassName == ClassName2 &&
										primClass->StaticChildren[c].Name == name)
									{
										Found = true;
										++CountStaticChildren;
										break;
									}
								}

								for (c=0; c<primClass->GeneratedChildren.size (); c++)
								{
									if (primClass->GeneratedChildren[c].ClassName == ClassName2)
									{
										Found = true;
										break;
									}
								}
								
								for (c=0; c<primClass->DynamicChildren.size (); c++)
								{
									if (primClass->DynamicChildren[c].ClassName == ClassName2)
									{
										Found = true;
										break;
									}
								}
								if (!Found)
								{
									++MissingChildren;

									// Mark the child as error
									IPrimitiveEditor *childPrimitiveEditor = getPrimitiveEditor (const_cast<IPrimitive*>(primitive2));
									childPrimitiveEditor->_Error = IPrimitiveEditor::InExcessField;
									InvalidatePrimitive (myLocator, LogicTreeParam);

									string childName;
									primitive2->getPropertyByName ("name", childName);
									errors += newPath + " : " + "The child \"" + childName + "\" (class \"" + ClassName2 + "\") is not declared in class \"" + className + "\".\n";
								}
							}
						}
					}
					while (myLocator.nextChild ());
				}
				if (MissingChildren)
				{
					error = IPrimitiveEditor::InExcessField;
				}
				if (CountStaticChildren != primClass->StaticChildren.size())
				{
					error = IPrimitiveEditor::InExcessField;
					errors += newPath+" : "+"Missing static children.\n";
				}
			}
		}
	}	
			
	if (locator.Primitive)
	{
		IPrimitiveEditor *primitiveEditor = getPrimitiveEditor (const_cast<IPrimitive*>(locator.Primitive));
		
		// set the error message
		primitiveEditor->_Error = error;
		InvalidatePrimitive (locator, LogicTreeParam);
	}
	return error;
}


const IPrimitiveEditor *getPrimitiveEditor(const NLLIGO::IPrimitive *primitive)
{
	if (primitive == NULL)
		return NULL;

	const CPrimNodeEditor *pne = dynamic_cast<const CPrimNodeEditor *>(primitive);
	if (pne)
	{
		return pne;
	}
	const CPrimPathEditor *ppe = dynamic_cast<const CPrimPathEditor *>(primitive);
	if (ppe)
	{
		return ppe;
	}
	const CPrimZoneEditor *pze = dynamic_cast<const CPrimZoneEditor *>(primitive);
	if (pze)
	{
		return pze;
	}
	const CPrimPointEditor *ppte = dynamic_cast<const CPrimPointEditor *>(primitive);
	if (ppte)
	{
		return ppte;
	}
	const CPrimAliasEditor *pae = dynamic_cast<const CPrimAliasEditor *>(primitive);
	if (pae)
	{
		return pae;
	}

	nlassert(false);
	return NULL;

}

IPrimitiveEditor *getPrimitiveEditor(NLLIGO::IPrimitive *primitive)
{
	return const_cast<IPrimitiveEditor*>(getPrimitiveEditor(const_cast<const IPrimitive *>(primitive)));
}
