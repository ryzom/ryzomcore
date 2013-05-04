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

// world_editor_doc.cpp : implementation of the CWorldEditorDoc class
//

#include "stdafx.h"

#include "action.h"
#include "world_editor.h"
#include "main_frm.h"
#include "world_editor_doc.h"
#include "display.h"
#include "dialog_properties.h"
#include "editor_primitive.h"
#include "file_dialog_ex.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NL3D;

#define WORLD_EDITOR_FILE_VERSION 2
#define WORLD_EDITOR_DATABASE_SIZE 100

// ***************************************************************************
// CWorldEditorDoc

IMPLEMENT_DYNCREATE(CWorldEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(CWorldEditorDoc, CDocument)
	//{{AFX_MSG_MAP(CWorldEditorDoc)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CWorldEditorDoc construction/destruction

CWorldEditorDoc::CWorldEditorDoc()
{
	_ModificationMode = false;
	clearModifications ();
}

CWorldEditorDoc::~CWorldEditorDoc()
{
	clearModifications ();
}

// ***************************************************************************

BOOL CWorldEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Main frame ?
	CMainFrame *mainFrame = getMainFrame ();
	if (mainFrame)
		mainFrame->updateData ();

	return newDocument () ? TRUE : FALSE;
}

// ***************************************************************************

bool CWorldEditorDoc::newDocument ()
{
	/*if (IsWindow(PropertyDialog))
		PropertyDialog.ShowWindow (SW_HIDE);*/

	std::list<CDialogProperties*>::iterator it = PropertiesDialogs.begin();

	while ( it != PropertiesDialogs.end() )
	{
		(*it)->ShowWindow( SW_HIDE );
		delete (*it);
		it++;
	}

	PropertiesDialogs.clear();

	_DataDir = "";
	_Context = "";
	
	// Erase all editable root primitive
	CDatabaseList::iterator ite = _DataHierarchy.begin();
	while (ite != _DataHierarchy.end())
	{
		CDatabaseList::iterator next = ite;
		next++;
		if (ite->Editable)
			_DataHierarchy.erase (ite);
		ite = next;
	}
	
	_DataHierarchy.recomputePointerArray ();

	// Init the landscape
	CMainFrame *mainFrame = getMainFrame ();
	if (mainFrame)
		mainFrame->initLandscapeData ();

	clearModifications ();

	// Invalidate all
	if (getMainFrame ())
	{
		getMainFrame ()->invalidateTools ();
	}
	invalidateLeftView ();

	// Invalidate pointers
	InvalidateAllPrimitives ();
	if (getMainFrame())
	{
		// we verify the structures of the primitives, and mark the incorrect nodes
		VerifyPrimitivesStructures();
	}

	return true;
}

// ***************************************************************************
// CWorldEditorDoc diagnostics

#ifdef _DEBUG
void CWorldEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWorldEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// ***************************************************************************
// CWorldEditorDoc commands

CWorldEditorDoc	*getDocument ()
{
	if (getMainFrame ())
 		return (CWorldEditorDoc	*)(getMainFrame ()->GetActiveDocument ());
	else
		return NULL;
}

// ***************************************************************************

BOOL CWorldEditorDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	// Timer enabled ?
	getMainFrame ()->TimerEnabled = false;

	// Result 
	newDocument ();
	bool res = loadDocument (lpszPathName) ? TRUE : FALSE;

	// Timer enabled ?
	getMainFrame ()->TimerEnabled = true;

	return res;
}

// ***************************************************************************

bool CWorldEditorDoc::loadDocument (const char *filename)
{
	bool result = true;

	// Get path name
	string path = NLMISC::CFile::getPath (filename);

	// Backup current path
	string backupPath = CPath::getCurrentPath ();

	// Set current path
	CPath::setCurrentPath (path.c_str ());
	
	// Load the document
	CIFile file;
	if (file.open (filename))
	{
		try
		{
			// Load the document in XML
			CIXml xml;
			xml.init (file);

			// Get root node
			bool success = false;
			xmlNodePtr rootNode = xml.getRootNode ();
			if (rootNode)
			{
				// Good header ?
				if (strcmp ((const char *)(rootNode->name), "NEL_WORLD_EDITOR_PROJECT") ==0)
				{
					// Read the version
					int version = -1;

					// Read the parameters
					xmlNodePtr node = CIXml::getFirstChildNode (rootNode, "VERSION");
					if (node)
					{
						string versionString;
						if (CIXml::getContentString (versionString, node))
							version = atoi (versionString.c_str ());
					}
					
					if (version == -1)
					{
						// Error
						theApp.syntaxError (filename, rootNode, "No version node");
					}
					else
					{
						// Old format, serial it
						if (version <= 1)
						{
							file.close ();
							CIFile file2;
							nlverify (file2.open (filename));
							CIXml xml2;
							xml2.init (file2);
							serial (xml2);

							// Done 
							success = true;
						}
						else
						{
							// Read it
							if (version > WORLD_EDITOR_FILE_VERSION)
							{
								theApp.syntaxError (filename, node, "Unknown file version");
							}
							else
							{
								// Read data directory
								node = CIXml::getFirstChildNode (rootNode, "DATA_DIRECTORY");
								if (node)
								{
									CIXml::getPropertyString (_DataDir, node, "VALUE");
								}

								// Read data directory
								node = CIXml::getFirstChildNode (rootNode, "CONTEXT");
								if (node)
								{
									CIXml::getPropertyString (_Context, node, "VALUE");
								}

								// Read the database element
								node = CIXml::getFirstChildNode (rootNode, "DATABASE_ELEMENT");
								if (node)
								{
									do
									{
										// Get the type
										string type;
										if (theApp.getPropertyString (type, filename, node, "TYPE"))
										{
											// Read the filename
											string filenameChild;
											if (theApp.getPropertyString (filenameChild, filename, node, "FILENAME"))
											{
												// Is it a landscape ?
												if (type == "landscape")
												{
													_DataHierarchy.push_back (CDatabaseElement (CDatabaseElement::Landscape));
													_DataHierarchy.recomputePointerArray ();

													// Get the primitives
													xmlNodePtr primitives = CIXml::getFirstChildNode (node, "PRIMITIVES");
													if (primitives)
													{
														// Read it
														_DataHierarchy.back ().Primitives.read (primitives, filename, theApp.Config);

														// Set the filename
														_DataHierarchy.back ().Filename = filenameChild;
													}
												}
												else
												{
													_DataHierarchy.push_back (CDatabaseElement (CDatabaseElement::Primitive));
													_DataHierarchy.recomputePointerArray ();

													// Set the filename
													_DataHierarchy.back ().Filename = filenameChild;
												}

											}
										}
									}
									while (node = CIXml::getNextChildNode (node, "DATABASE_ELEMENT"));
								}

								// Done 
								success = true;
							}
						}
					}
				}
				else
				{
					// Error
					theApp.syntaxError (filename, rootNode, "Unknown file header : %s", rootNode->name);
				}
			}
			
			if (!success)
			{
				result = false;
				newDocument ();
			}
			else
			{
				// Last time
				_LastModifedTime = NLMISC::CFile::getFileModificationDate (filename);

				// Load the files
				uint i;
				for (i=0; i<_DataHierarchy.size (); i++)
				{
					if (_DataHierarchy[i].Editable)
					{
						// Not modified
						_DataHierarchy[i].Modified = false;

						getMainFrame()->launchLoadingDialog(toString("loading %s", _DataHierarchy[i].Filename.c_str()).c_str());
						getMainFrame()->progressLoadingDialog(float(i+0.0001f)/_DataHierarchy.size());
						// Landscape ?
						if (_DataHierarchy[i].Type == CDatabaseElement::Landscape)
						{
							// Load the landscape
							try
							{
								CIFile fileIn;
								if (fileIn.open (_DataHierarchy[i].Filename))
								{
									CIXml xml(true);
									xml.init (fileIn);
									_DataHierarchy[i].ZoneRegion.serial (xml);

									// Last time
									_DataHierarchy[i].LastModifedTime = NLMISC::CFile::getFileModificationDate (_DataHierarchy[i].Filename);
								}
								else
								{
									if (!theApp.yesNoMessage ("Can't open file %s for reading. Continue reading ?", _DataHierarchy[i].Filename.c_str ()))
									{
										result = false;
									}
								}
							}
							catch (Exception& e)
							{
								theApp.errorMessage ("Error reading file %s : %s", _DataHierarchy[i].Filename.c_str (), e.what ());
							}
						}
						else
						{
							// Checks
							nlassert (_DataHierarchy[i].Type == CDatabaseElement::Primitive);

							// Load the primitive
							try
							{
								CIFile fileIn;
								if (fileIn.open (_DataHierarchy[i].Filename))
								{
									// Xml stream
									CIXml xmlIn;
									xmlIn.init (fileIn);

									// set the primitive context
									CPrimitiveContext::instance().CurrentPrimitive = &_DataHierarchy[i].Primitives;

									// Read it
									if (!_DataHierarchy[i].Primitives.read (xmlIn.getRootNode (), _DataHierarchy[i].Filename.c_str(), theApp.Config))
									{
										theApp.errorMessage ("Error reading file %s", _DataHierarchy[i].Filename.c_str ());
									}

									// cleanup the primitive context
									CPrimitiveContext::instance().CurrentPrimitive = NULL;

									// Last time
									_DataHierarchy[i].LastModifedTime = NLMISC::CFile::getFileModificationDate (_DataHierarchy[i].Filename);
								}
								else
								{
									if (!theApp.yesNoMessage ("Can't open file %s for reading. Continue reading ?", _DataHierarchy[i].Filename.c_str ()))
									{
										result = false;
									}
								}
							}
							catch (Exception& e)
							{
								theApp.errorMessage ("Error reading file %s : %s", _DataHierarchy[i].Filename.c_str (), e.what ());
							}
						}

						getMainFrame()->terminateLoadingDialog();
						/* // Not modified
						_DataHierarchy[i].Modified = false;*/
					}
				}

				// Init the landscape
				CMainFrame *mainFrame = getMainFrame ();
				if (mainFrame)
					mainFrame->initLandscapeData ();

				// Reset document changes
				clearModifications ();
			}
		}
		catch (Exception &e)
		{
			theApp.errorMessage ("Error reading file %s : %s", filename, e.what ());
		
			result = false;
		}
	}
	else
	{
		theApp.errorMessage ("Can't open the file %s for reading.", filename);

		result = false;
	}

	// Invalidate some stuff
	invalidateLeftView ();
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();
	VerifyPrimitivesStructures();
		
	// Restaure current path
	CPath::setCurrentPath (backupPath.c_str ());

	// Initialize default values
	for (uint j=0; j<_DataHierarchy.size(); ++j)
	{
		if (_DataHierarchy[j].Type == CDatabaseElement::Primitive)
		{
			updateDefaultValues (j);
		}
	}

	// Check property types
	for (uint j=0; j<_DataHierarchy.size(); ++j)
	{
		if (_DataHierarchy[j].Type == CDatabaseElement::Primitive)
		{
			updateDefaultValues (j);
		}
	}

	// Warn the plugins
	uint i;
	for (i=0; i<theApp.Plugins.size(); ++i)
	{
		for (uint j=0; j<_DataHierarchy.size(); ++j)
		{
			if (_DataHierarchy[j].Editable)
			{
				if (_DataHierarchy[j].Type == CDatabaseElement::Primitive)
					theApp.Plugins[i]->primitiveChanged(_DataHierarchy[j].Primitives.RootNode);
			}
		}
	}


	return result;
}

// ***************************************************************************

BOOL CWorldEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	BOOL result = TRUE;

	// Get path name
	string path = NLMISC::CFile::getPath (lpszPathName);

	// Backup current path
	string backupPath = CPath::getCurrentPath ();

	// Set current path
	CPath::setCurrentPath (path.c_str ());
	
	// Timer enabled ?
	getMainFrame ()->TimerEnabled = false;

	// Save land files
	for (uint i=0; i<_DataHierarchy.size (); i++)
	{
		if (_DataHierarchy[i].Editable)
		{
			// Make path relative
			string relativeFileName = _DataHierarchy[i].Filename;
			CPath::makePathRelative (NLMISC::CFile::getPath (lpszPathName).c_str (), relativeFileName);
			if (relativeFileName != _DataHierarchy[i].Filename)
			{
				_DataHierarchy[i].Filename = relativeFileName;
				modifyProject ();
			}

			// Modified
			if (_DataHierarchy[i].Modified)
			{
				// Landscape ?
				if (_DataHierarchy[i].Type == CDatabaseElement::Landscape)
				{
					// Save the landscape
					try
					{
						// Got a filename ?
						if (_DataHierarchy[i].Filename.empty ())
						{
							CFileDialogEx dialog (BASE_REGISTRY_KEY, "land", FALSE, "land", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "NeL Ligo Landscape Files (*.land)|*.land|All Files (*.*)|*.*||", getMainFrame ());
							if (dialog.DoModal() == IDOK)
							{
								_DataHierarchy[i].Filename = dialog.GetPathName();
							}
							else
							{
								continue;
							}
						}

						// Open the primitive file for writing
						COFile fileOut;
						if (fileOut.open (_DataHierarchy[i].Filename, false, false, true))
						{
							// Be careful with the flushing of the COXml object
							{
								COXml xmlOut;
								xmlOut.init (&fileOut);
								_DataHierarchy[i].ZoneRegion.serial (xmlOut);

								// Done
								_DataHierarchy[i].Modified = false;
							}

							fileOut.close();
						}
						else
						{
							if (!theApp.yesNoMessage ("Can't open file %s for writing. Continue saving ?", _DataHierarchy[i].Filename.c_str ()))
							{
								result = FALSE;
							}
						}

						_DataHierarchy[i].LastModifedTime = NLMISC::CFile::getFileModificationDate (_DataHierarchy[i].Filename);
					}
					catch (Exception& e)
					{
						theApp.errorMessage ("Error writing file %s : %s", _DataHierarchy[i].Filename.c_str (), e.what ());
					}
				}
				else
				{
					// Checks
					nlassert (_DataHierarchy[i].Type == CDatabaseElement::Primitive);

					// Save the primitive
					try
					{
						// Got a filename ?
						if (_DataHierarchy[i].Filename.empty ())
						{
							CFileDialogEx dialog (BASE_REGISTRY_KEY, "primitive", FALSE, "primitive", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "NeL Ligo Primitive Files (*.primitive)|*.primitive|All Files (*.*)|*.*||", getMainFrame ());
							if (dialog.DoModal() == IDOK)
							{
								string pathFromDialog = dialog.GetPathName();

								// Remove uppercase in filename
								string::size_type pos = pathFromDialog.rfind ("\\", 0);
								for (uint j=pos+1 ; j<pathFromDialog.size() ; j++)
									pathFromDialog[j] = toLower(pathFromDialog[j]);

								_DataHierarchy[i].Filename = pathFromDialog;

								getMainFrame ()->invalidateTools ();
							}					
							else
							{
								continue;
							}
						}

						// Open the primitive file for writing
						COFile fileOut;
						if (fileOut.open (_DataHierarchy[i].Filename, false, false, true))
						{
							COXml xmlOut;
							xmlOut.init (&fileOut);

							// Write the document
							_DataHierarchy[i].Primitives.write (xmlOut.getDocument (), _DataHierarchy[i].Filename.c_str ());

							// Flush it
							xmlOut.flush ();

							// Done
							_DataHierarchy[i].Modified = false;

							// Invalidate tools
							getMainFrame ()->invalidateToolsParam ();

							// Last time
							fileOut.close ();
						}
						else
						{
							if (!theApp.yesNoMessage ("Can't open file %s for writing. Continue saving ?", _DataHierarchy[i].Filename.c_str ()))
							{
								result = FALSE;
							}
						}

						_DataHierarchy[i].LastModifedTime = NLMISC::CFile::getFileModificationDate (_DataHierarchy[i].Filename);
					}
					catch (Exception& e)
					{
						theApp.errorMessage ("Error writing file %s : %s", _DataHierarchy[i].Filename.c_str (), e.what ());
					}
				}

				// Invalidate title
				CDatabaseLocatorPointer locator;
				locator.getRoot (i);
				InvalidatePrimitive (locator, LogicTreeParam);
			}
		}
	}

	// Document modified ?
	if (_Modified)
	{
		// Save the document
		COFile file;
		if (file.open (lpszPathName, false, false, true))
		{
			try
			{
				{
					// Save the document in XML
					COXml xml;
					xml.init (&file);

					// Create the document
					xmlNodePtr rootNode = xmlNewDocNode (xml.getDocument (), NULL, (const xmlChar*)"NEL_WORLD_EDITOR_PROJECT", NULL);
					xmlDocSetRootElement (xml.getDocument (), rootNode);

					// Version
					xmlNodePtr node = xmlNewChild ( rootNode, NULL, (const xmlChar*)"VERSION", NULL);
					xmlNodePtr text = xmlNewText ((const xmlChar *)toString (WORLD_EDITOR_FILE_VERSION).c_str ());
					xmlAddChild (node, text);

					// DATA_DIRECTORY
					node = xmlNewChild ( rootNode, NULL, (const xmlChar*)"DATA_DIRECTORY", NULL);
					xmlSetProp (node, (const xmlChar*)"VALUE", (const xmlChar*)_DataDir.c_str ());

					// DATA_DIRECTORY
					node = xmlNewChild ( rootNode, NULL, (const xmlChar*)"CONTEXT", NULL);
					xmlSetProp (node, (const xmlChar*)"VALUE", (const xmlChar*)_Context.c_str ());

					// Database
					for (uint i=0; i<_DataHierarchy.size (); i++)
					{
						if (_DataHierarchy[i].Editable)
						{
							// New element
							node = xmlNewChild ( rootNode, NULL, (const xmlChar*)"DATABASE_ELEMENT", NULL);
							xmlSetProp (node, (const xmlChar*)"FILENAME", (const xmlChar*)_DataHierarchy[i].Filename.c_str ());

							// Type
							bool landscape = isLandscape (i);
							xmlSetProp (node, (const xmlChar*)"TYPE", (const xmlChar*)(landscape?"landscape":"primitive"));

							// Write properties ?
							if (landscape)
							{
								node = xmlNewChild ( node, NULL, (const xmlChar*)"PRIMITIVES", NULL);
								_DataHierarchy[i].Primitives.write (node, lpszPathName);
							}
						}
					}

					// Not modified
					_Modified = false;
				}

				// Last time
				file.close ();
			}
			catch (Exception &e)
			{
				theApp.errorMessage ("Error writing file %s : %s", lpszPathName, e.what ());

				result = FALSE;
			}
		}
		else
		{
			result = FALSE;
		}

		_LastModifedTime = NLMISC::CFile::getFileModificationDate (lpszPathName);
	}

	// No modification is for this level of undo
	_LastSaveUndo = _Undo;
	_NoModificationUndo = false;

	updateModifiedState ();

	// Timer enabled
	getMainFrame ()->TimerEnabled = true;

	// Restaure current path
	CPath::setCurrentPath (backupPath.c_str ());

	return result;
}

// ***************************************************************************

void CWorldEditorDoc::serial (NLMISC::IStream &s)
{
	s.xmlPush ("NEL_WORLD_EDITOR_PROJECT");
		
	// Serial
	int version = s.serialVersion (2);

	s.xmlSerial (_DataDir, "DATA_DIRECTORY");
	
	if (version>0)
	{
		s.xmlSerial (_Context, "CONTEXT");
	}
	
	// Old structures
	vector<CLandscapeDeprecated> landscapes;
	vector<CPrimitiveDeprecated> primitives;

	s.xmlPush ("LANDSCAPE");
		s.serialCont (landscapes);
	s.xmlPop ();

	s.xmlPush ("PRIMITIVES");
		s.serialCont (primitives);
	s.xmlPop ();

	// * Convert in new structures

	// Resvere
	// _DataHierarchy.reserve (landscapes.size () + primitives.size ());

	// Copy landscapes
	uint i;
	for (i=0; i<landscapes.size (); i++)
	{
		// Add an element
		_DataHierarchy.push_back (CDatabaseElement (CDatabaseElement::Landscape));
		_DataHierarchy.recomputePointerArray ();

		// Copy
		_DataHierarchy.back ().Filename = landscapes[i].Filename;
	}

	// Copy primitives
	for (i=0; i<primitives.size (); i++)
	{
		// Add an element
		_DataHierarchy.push_back (CDatabaseElement (CDatabaseElement::Primitive));
		_DataHierarchy.recomputePointerArray ();

		// Copy
		_DataHierarchy.back ().Filename = primitives[i].Filename;
	}

	s.xmlPop ();
}

// ***************************************************************************

void CWorldEditorDoc::updateModifiedState ()
{
	// Modification flag
	bool modified = _NoModificationUndo | (_Undo != _LastSaveUndo);

	SetModifiedFlag (modified?TRUE:FALSE);

	if (modified)
	{
		CString title = GetTitle ();
		if ( (title.GetLength()<2) || (title[title.GetLength()-1] != '*') || (title[title.GetLength()-2] != ' ') )
			SetTitle (title+" *");
	}
	else
	{
		string title = (const char*)GetTitle ();
		if ( (title.size ()>=2) && (title[title.size()-1] == '*') && (title[title.size()-2] == ' ') )
		{
			title.resize (title.size () - 2);
			SetTitle (title.c_str());
		}
	}
}

// ***************************************************************************

void CWorldEditorDoc::noUndoModification ()
{
	_NoModificationUndo = true;
	
	updateModifiedState ();
}

// ***************************************************************************

void CWorldEditorDoc::undo ()
{
	// Should not be in modification mode
	nlassertex (_ModificationMode == false, ("In modification mode !"));

	if (_Undo > 0)
	{
		_Undo--;

		// Undo actions
		for (uint i=0; i<_Actions[_Undo].size (); i++)
			_Actions[_Undo][_Actions[_Undo].size ()-i-1]->undo ();

		updateModifiedState ();
	}
}

// ***************************************************************************

void CWorldEditorDoc::redo ()
{
	// Should not be in modification mode
	nlassertex (_ModificationMode == false, ("In modification mode !"));

	if (_Undo < _Actions.size ())
	{
		// Undo actions
		for (uint i=0; i<_Actions[_Undo].size (); i++)
			_Actions[_Undo][i]->redo ();

		_Undo++;

		updateModifiedState ();
	}
}

// ***************************************************************************

void CWorldEditorDoc::clearModifications ()
{
	for (uint i=0; i<_Actions.size (); i++)
	for (uint j=0; j<_Actions[i].size (); j++)
	{
		delete _Actions[i][j];
	}

	_Actions.clear ();
	_NoModificationUndo = false;
	_Undo = 0;
	_LastSaveUndo = 0;
	_Modified = false;
}

// ***************************************************************************

const string &CWorldEditorDoc::getDataDir () const
{
	return _DataDir;
}

// ***************************************************************************


void CWorldEditorDoc::getFilePath(uint primIndex,string & relativeFileName)
{	
	relativeFileName=_DataHierarchy[primIndex].Filename;
	CPath::makePathRelative ((NLMISC::CFile::getPath ((LPCTSTR)CWorldEditorDoc::GetPathName())).c_str(), relativeFileName);
}

// ***************************************************************************

void CWorldEditorDoc::setDataDir (const char *dir)
{
	_DataDir = dir;
	
	modifyProject ();

	noUndoModification ();
}

// ***************************************************************************

const string& CWorldEditorDoc::getPathOfSelectedPrimitive() const
{
	return _PathOfSelectedPrimitive;
}

// ***************************************************************************

void CWorldEditorDoc::setPathOfSelectedPrimitive(const std::string& s)
{
	_PathOfSelectedPrimitive = s;
}

// ***************************************************************************

bool CWorldEditorDoc::getZoneAmongRegions (CDatabaseLocator &locator, class CBuilderZoneRegion*& pBZRfrom, sint32 x, sint32 y)
{
	uint32 nNbRegion = _DataHierarchy.size ();
	int regionId = -1;
	for (uint32 i = 0; i < nNbRegion; ++i)
	{
		// Landscape ?
		if (_DataHierarchy[i].Type == CDatabaseElement::Landscape)
		{
			regionId++;
			const NLLIGO::CZoneRegion &region = getZoneRegionAbsolute (i);
			if ((x < region.getMinX ())||(x > region.getMaxX ())||(y < region.getMinY ())||(y > region.getMaxY ()))
				continue;
			if (region.getName (x, y) != STRING_UNUSED)
			{
				pBZRfrom = getMainFrame ()->_ZoneBuilder->_ZoneRegions[regionId];
				locator = CDatabaseLocator (regionId, x, y);
				return true;
			}
		}
	}

	// The zone is not present in other region so it is an empty or oob zone of the current region
	const NLLIGO::CZoneRegion &region = getZoneRegion(pBZRfrom->RegionId);
	if ((x < region.getMinX ())||(x > region.getMaxX ())||(y < region.getMinY ())||(y > region.getMaxY ()))
		return false; // Out Of Bound

	locator = CDatabaseLocator (pBZRfrom->RegionId, x, y);

	return true;
}

// ***************************************************************************

const NLLIGO::CZoneRegion &CWorldEditorDoc::getZoneRegion (uint landscape) const
{
	uint index = regionIDToDatabaseElementID (landscape);
	return _DataHierarchy[index].ZoneRegion;
}

// ***************************************************************************

uint CWorldEditorDoc::regionIDToDatabaseElementID (uint landscape) const
{
	uint32 nNbRegion = _DataHierarchy.size ();
	uint32 i;
	for (i = 0; i < nNbRegion; ++i)
	{
		// Landscape ?
		if (_DataHierarchy[i].Type == CDatabaseElement::Landscape)
		{
			if (landscape == 0)
				return i;
			landscape --;
		}
	}
	// Should be found
	nlstop;		
	return i;
}

// ***************************************************************************

const NLLIGO::CZoneRegion &CWorldEditorDoc::getZoneRegionAbsolute (uint landscape) const
{
	nlassert (isLandscape (landscape));
	return _DataHierarchy[landscape].ZoneRegion;
}

// ***************************************************************************

bool CWorldEditorDoc::addModification (class IAction *action)
{
	// Should be in modification mode
	nlassertex (_ModificationMode == true, ("Not in modification mode !"));

	// Push first the action
	uint actionId = _CurrentAction.size ();
	_CurrentAction.push_back (action);

	// Do the action
	if (!action->redo ())
	{
		// Delete all new actions
		nlassert (_CurrentAction.size ()>0);
		for (uint a=_CurrentAction.size ()-1; a>actionId; a--)
		{
			// Undo
			_CurrentAction[a]->undo ();
			delete _CurrentAction[a];
		}
		delete action;
		_CurrentAction.pop_back ();

		return false;
	}

	
	return true;
}

// ***************************************************************************

void CWorldEditorDoc::getLigoData (CLigoData &data, const CDatabaseLocator &locator)
{
	NLLIGO::CZoneRegion &region = _DataHierarchy[locator.getDatabaseIndex ()].ZoneRegion;
	nlassert ((locator.XSubPrim >= region.getMinX ()) && (locator.XSubPrim <= region.getMaxX ()) && (locator.Y >= region.getMinY ()) && (locator.Y <= region.getMaxY ()));

	data.PosX = region.getPosX (locator.XSubPrim, locator.Y);
	data.PosY = region.getPosY (locator.XSubPrim, locator.Y);
	data.ZoneName = region.getName (locator.XSubPrim, locator.Y);
	data.Rot = region.getRot (locator.XSubPrim, locator.Y);
	data.Flip = region.getFlip (locator.XSubPrim, locator.Y);
	data.SharingMatNames[0] = region.getSharingMatNames (locator.XSubPrim, locator.Y, 0);
	data.SharingMatNames[1] = region.getSharingMatNames (locator.XSubPrim, locator.Y, 1);
	data.SharingMatNames[2] = region.getSharingMatNames (locator.XSubPrim, locator.Y, 2);
	data.SharingMatNames[3] = region.getSharingMatNames (locator.XSubPrim, locator.Y, 3);
	data.SharingCutEdges[0] = region.getSharingCutEdges (locator.XSubPrim, locator.Y, 0);
	data.SharingCutEdges[1] = region.getSharingCutEdges (locator.XSubPrim, locator.Y, 1);
	data.SharingCutEdges[2] = region.getSharingCutEdges (locator.XSubPrim, locator.Y, 2);
	data.SharingCutEdges[3] = region.getSharingCutEdges (locator.XSubPrim, locator.Y, 3);
}

// ***************************************************************************

void CWorldEditorDoc::CLandscapeDeprecated::serial (NLMISC::IStream &s)
{
	s.serialVersion (0);
	s.serial (Filename);
}

// ***************************************************************************

void CWorldEditorDoc::CPrimitiveDeprecated::serial (NLMISC::IStream &s)
{
	s.serialVersion (0);
	s.serial (Filename);
}

// ***************************************************************************

CWorldEditorDoc::CDatabaseElement::CDatabaseElement ()
{
	Type = Undefined;
	Editable = true;
	Modified = false;
}

// ***************************************************************************

CWorldEditorDoc::CDatabaseElement::CDatabaseElement (TType type)
{
	Type = type;
	if (Type == Landscape)
		Primitives.RootNode->addPropertyByName ("class", new CPropertyString ("landscape"));
	Editable = true;
	Modified = false;
}

// ***************************************************************************

void CWorldEditorDoc::beginModification ()
{
	nlassertex (_ModificationMode == false, ("Already in modification mode !"));
	_ModificationMode = true;

	// Erase current action
	_CurrentAction.clear ();
}

// ***************************************************************************

void CWorldEditorDoc::endModification ()
{
	nlassertex (_ModificationMode == true, ("Not in modification mode !"));
	_ModificationMode = false;

	// Add a undo entry if some actions have been performed
	if (!_CurrentAction.empty ())
	{
		// Remove next actions
		for (uint i=_Undo; i<_Actions.size (); i++)
		for (uint j=0; j<_Actions[i].size (); j++)
		{
			delete _Actions[i][j];
		}

		// Place the current action
		_Actions.resize (_Undo+1);
		_Actions[_Undo] = _CurrentAction;

		// Invalidate save position
		if (_Undo < _LastSaveUndo)
			_LastSaveUndo = -1;

		_Undo++;

		updateModifiedState ();

		// check if we need to send an update to the plugins
		bool needUpdate = false;
		std::vector<IAction*>::iterator first(_CurrentAction.begin()), last(_CurrentAction.end());
		for (; first != last; ++first)
		{
			needUpdate |= (*first)->isAffectingContent();
		}
		if (needUpdate)
		{
			for (uint i=0; i<theApp.Plugins.size(); ++i)
			{
				for (uint j=0; j<_DataHierarchy.size(); ++j)
				{
					if (_DataHierarchy[j].Type == CDatabaseElement::Primitive)
						theApp.Plugins[i]->primitiveChanged(_DataHierarchy[j].Primitives.RootNode);
				}
			}
		}
	}
}

// ***************************************************************************

uint CWorldEditorDoc::getNumDatabaseElement () const
{
	return _DataHierarchy.size ();
}

// ***************************************************************************

const std::string &CWorldEditorDoc::getDatabaseElement (uint primitive) const
{
	return _DataHierarchy[primitive].Filename;
}

// ***************************************************************************

const NLLIGO::CPrimitives &CWorldEditorDoc::getDatabaseElements (uint primitive) const
{
	return _DataHierarchy[primitive].Primitives;
}

// ***************************************************************************

void CWorldEditorDoc::getLocator (CDatabaseLocatorPointer &locator, const IPrimitive *primitive) const
{
	static std::vector<uint> cache;
	cache.clear ();

	// Get the first
	const IPrimitive *current = primitive;
	const IPrimitive *parent = current->getParent ();
	while (parent)
	{
		// Get the child id
		uint childId;
		nlverify (parent->getChildId (childId, current));
		cache.push_back (childId);
		current = parent;
		parent = current->getParent ();
	}

	// Root node
	uint i;
	CWorldEditorDoc *doc = getDocument ();
	uint count = doc->getNumDatabaseElement ();
	for (i=0; i<count; i++)
	{
		if (doc->getDatabaseElements (i).RootNode == current)
		{
			break;
		}
	}

	nlassert (i<count);
	cache.push_back (i);

	// Cash size
	count = cache.size ();
	locator._LocateStack.resize (count);
	for (i=0; i<count; i++)
		locator._LocateStack[i] = cache[count-i-1];

	// Set the pointer
	locator.Primitive = primitive;
}

// ***************************************************************************

void CWorldEditorDoc::getLocator (CDatabaseLocatorPointer &locatorDest, const CDatabaseLocator &locator) const
{
	nlassert (locator._LocateStack.size ()>0);

	// Copy the stack
	locatorDest._LocateStack = locator._LocateStack;

	// Get the first primitive
	locatorDest.Primitive = _DataHierarchy[locator._LocateStack[0]].Primitives.RootNode;

	// Get into the stack
	uint stackSize = locator._LocateStack.size ();
	for (uint i=1; i<stackSize; i++)
	{
		// Get the child		
		nlverify (locatorDest.Primitive->getChild (locatorDest.Primitive, locator._LocateStack[i]));
	}
}

// ***************************************************************************

void CWorldEditorDoc::getFirstLocator (CDatabaseLocatorPointer &locator) const
{
	// Set as end
	locator._LocateStack[0] = 0xffffffff;
	locator.Primitive = NULL;

	// Is a first primitive ?
	if (_DataHierarchy.size ())
	{
		locator._LocateStack[0] = 0;
	}
}

// ***************************************************************************

void CWorldEditorDoc::deletePrimitive (const CDatabaseLocator &locator)
{
	// Locate it
	CDatabaseLocatorPointer locatorDest;
	getLocator (locatorDest, locator);

	// Get the parent
	IPrimitive *parent = const_cast<IPrimitive*>(locatorDest.Primitive)->getParent ();
	nlassert (parent);

	// Get the child id
	uint childId;
	nlverify (parent->getChildId (childId, locatorDest.Primitive));

	// Delete the child
	nlverify (parent->removeChild (childId));
}

// ***************************************************************************

void CWorldEditorDoc::insertPrimitive (const CDatabaseLocator &locator, IPrimitive *primitive)
{
	nlassert (locator._LocateStack.size ()>0);

	// Get the first primitive
	IPrimitive *prim = _DataHierarchy[locator._LocateStack[0]].Primitives.RootNode;

	// Get into the stack
	uint stackSize = locator._LocateStack.size ();
	uint i;
	for (i=1; i<stackSize-1; i++)
	{
		// Get the child		
		nlverify (prim->getChild (prim, locator._LocateStack[i]));
	}

	// Set the context
	CPrimitiveContext::instance().CurrentPrimitive = &_DataHierarchy[locator._LocateStack[0]].Primitives;

	// Insert the primitive in the parent
	nlverify (prim->insertChild (primitive, locator._LocateStack[i]));

	// unset the context
	CPrimitiveContext::instance().CurrentPrimitive = NULL;
}

// ***************************************************************************

uint32 getUniqueId ()
{
	// Wait 1 ms
	sint64 time = NLMISC::CTime::getLocalTime ();
	sint64 time2;
	while ((time2 = NLMISC::CTime::getLocalTime ()) == time)
	{
	}

	return (uint32)time2;
}

// ***************************************************************************

void CWorldEditorDoc::initPrimitiveParameters (const CPrimitiveClass &primClass, IPrimitive &primitive,
												const std::vector<CPrimitiveClass::CInitParameters> &initParameters)
{
	// Other parameters
	for (uint p=0; p<initParameters.size (); p++)
	{
		// The property
		const CPrimitiveClass::CInitParameters &parameter = initParameters[p];

		// Look for it in the class
		uint cp;
		for (cp=0; cp<primClass.Parameters.size (); cp++)
		{
			// Good one ?
			if (primClass.Parameters[cp].Name == initParameters[p].Name)
				break;
		}

		// The primitive type
		CPrimitiveClass::CParameter::TType type;

		// Found ?
		if (cp<primClass.Parameters.size ())
			type = primClass.Parameters[cp].Type;

		// Name ?
		if (initParameters[p].Name == "name")
			type = CPrimitiveClass::CParameter::String;

		// Continue ?
		if (cp<primClass.Parameters.size () || (initParameters[p].Name == "name"))
		{
			// Default value ?
			if (!parameter.DefaultValue.empty ())
			{
				// Type of property
				switch (type)
				{
				case CPrimitiveClass::CParameter::Boolean:
				case CPrimitiveClass::CParameter::ConstString:
				case CPrimitiveClass::CParameter::String:
					{
						// Some feedback
						if (parameter.DefaultValue.size () > 1)
							theApp.errorMessage ("Warning: parameter (%s) in class name (%s) has more than 1 default value (%d).", 
							parameter.Name.c_str (), primClass.Name.c_str (), parameter.DefaultValue.size ());

						if (
							(cp<primClass.Parameters.size () && !primClass.Parameters[cp].Visible)
							|| parameter.DefaultValue[0].GenID)
						{
							// Remove this property
							primitive.removePropertyByName (parameter.Name.c_str ());

							// Add this property
							primitive.addPropertyByName (parameter.Name.c_str (), 
								new CPropertyString ((parameter.DefaultValue[0].GenID ? toString (getUniqueId ()) : 
									"").c_str ()));
						}
						break;
					}
				case CPrimitiveClass::CParameter::ConstStringArray:
				case CPrimitiveClass::CParameter::StringArray:
					{
						bool Visible = false;
						uint i;
						if (cp<primClass.Parameters.size () && !primClass.Parameters[cp].Visible)
						{
							Visible = true;
						}
						for (i=0; i<parameter.DefaultValue.size (); i++)
						{
							// Generate a unique id ?
							if (parameter.DefaultValue[i].GenID)
							{
								Visible = true;
							}
						}
						if (Visible)
						{
							// Remove this property
							primitive.removePropertyByName (parameter.Name.c_str ());

							// Add this property
							CPropertyStringArray *str = new CPropertyStringArray ();
							str->StringArray.resize (parameter.DefaultValue.size ());
							for (i=0; i<parameter.DefaultValue.size (); i++)
							{
								// Generate a unique id ?
								if (parameter.DefaultValue[i].GenID)
								{
									str->StringArray[i] = toString (getUniqueId ());
								}
								else
								{
									str->StringArray[i] = "";
								}
							}
							primitive.addPropertyByName (parameter.Name.c_str (), str);
						}
						break;
					}
				}
			}
		}
		else
		{
			// Some feedback
			theApp.errorMessage ("Warning: parameter (%s) doesn't exist in class (%s).", 
				initParameters[p].Name.c_str (), primClass.Name.c_str ());
		}
	}
}

// ***************************************************************************

const NLLIGO::IPrimitive *CWorldEditorDoc::createPrimitive (const CDatabaseLocator &locator, const char *className, const char *primName, 
															const CVector &initPos, float deltaPos, 
															const std::vector<CPrimitiveClass::CInitParameters> &initParameters)
{
	// Get the prim class
	const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (className);
	if (primClass)
	{
		// Create the base primitive
		IPrimitive *primitive = NULL;
		switch (primClass->Type)
		{
		case CPrimitiveClass::Node:
			primitive = new CPrimNodeEditor;
			break;
		case CPrimitiveClass::Point:
			{
				CPrimPointEditor *point = new CPrimPointEditor;
				primitive = point;
				point->Point.CVector::operator= (initPos);
			}
			break;
		case CPrimitiveClass::Path:
			primitive = new CPrimPathEditor;
			break;
		case CPrimitiveClass::Zone:
			primitive = new CPrimZoneEditor;
			break;
		case CPrimitiveClass::Alias:
			primitive = new CPrimAliasEditor;
			break;
		case CPrimitiveClass::Bitmap:
			primitive = new CPrimBitmap;
			break;
		}
		nlassert (primitive);

		// Add properties
		primitive->addPropertyByName ("class", new CPropertyString (className));
		primitive->addPropertyByName ("name", new CPropertyString (primName, primName[0] == 0));

		// Init with default parameters
		std::vector<CPrimitiveClass::CInitParameters> tempParam;
		tempParam.reserve (primClass->Parameters.size ());
		for (uint i=0; i<primClass->Parameters.size (); i++)
			tempParam.push_back (primClass->Parameters[i]);
		initPrimitiveParameters (*primClass, *primitive, tempParam);

		// Init with option parameters
		initPrimitiveParameters (*primClass, *primitive, initParameters);

		// Insert the primitive
		insertPrimitive (locator, primitive);

		// The new pos
		CVector newPos = initPos;
		newPos.x += deltaPos;

		// Create static children
		uint c;
		for (c=0; c<primClass->StaticChildren.size (); c++)
		{
			// The child ref
			const CPrimitiveClass::CChild &child = primClass->StaticChildren[c];

			// The new locator
			CDatabaseLocator childLocator = locator;
			childLocator._LocateStack.push_back (0);

			// Create the child
			const NLLIGO::IPrimitive *childPrim = createPrimitive (childLocator, child.ClassName.c_str (), 
				child.Name.c_str (), newPos, deltaPos, primClass->StaticChildren[c].Parameters);

			// The new pos
			newPos.y += deltaPos;
		}

		// Canceled ?
		if (c<primClass->StaticChildren.size ())
		{
			deletePrimitive (locator);
			return NULL;
		}

		// Prim file ?
		if (primClass->Type == CPrimitiveClass::Bitmap)
		{
			// Create a dialog file
			CFileDialogEx dialog (BASE_REGISTRY_KEY, "image", TRUE, primClass->FileExtension.c_str (), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 
				(primClass->FileType+" (*."+primClass->FileExtension+")|*."+primClass->FileExtension+"|All Files (*.*)|*.*||").c_str (), getMainFrame ());
			if (dialog.DoModal() == IDOK)
			{
				// Save filename
				static_cast<CPrimBitmap*>(primitive)->init (dialog.GetPathName ());
			}
		}

		// Continue ?
		if (primitive)
		{
			// Auto init ?
			if (!primClass->AutoInit)
			{
				// Make a vector of locator
				CDatabaseLocatorPointer locatorPtr;
				getLocator (locatorPtr, locator);
				std::list<NLLIGO::IPrimitive*> locators;
				locators.push_back (const_cast<IPrimitive*> (locatorPtr.Primitive));

				// Yes, go
				CDialogProperties dialogProperty (locators, getMainFrame ());
				dialogProperty.DoModal ();
			}

			// Eval the default name property
			string name;
			if (!primitive->getPropertyByName ("name", name) || name.empty())
			{
				const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*primitive);
				if (primClass)
				{
					uint i;
					for (i=0; i<primClass->Parameters.size(); i++)
					{
						if (primClass->Parameters[i].Name == "name")
						{
							std::string result;
							primClass->Parameters[i].getDefaultValue (result, *primitive, *primClass, NULL);
							if (!result.empty())
							{
								primitive->removePropertyByName ("name");
								primitive->addPropertyByName ("name", new CPropertyString (result.c_str(), true));
							}
						}
					}
				}
			}
		
			// Init primitive default values
			primitive->initDefaultValues (theApp.Config);
		}

		// Done
		return primitive;
	}
	else
	{
		theApp.errorMessage ("Unknown primitive class name : %s", className);
	}
	return NULL;
}
				
// ***************************************************************************

void CWorldEditorDoc::getPrimitiveDisplayName (std::string &result, uint primitive) const
{
	// Return a formated string
	result = formatString (NLMISC::CFile::getFilename (getDatabaseElement (primitive)).c_str ());
	if (result.empty ())
		result = "Untitled";
	if (_DataHierarchy[primitive].Modified && _DataHierarchy[primitive].Editable)
		result += " *";
}


// ***************************************************************************

void CWorldEditorDoc::modifyDatabase (uint dbIndex)
{
	if (_DataHierarchy[dbIndex].Editable)
	{
		_DataHierarchy[dbIndex].Modified = true;
		if (isLandscape (dbIndex))
			modifyProject ();
	}

	// Invalidate title
	CDatabaseLocatorPointer locator;
	locator.getRoot (dbIndex);
	InvalidatePrimitive (locator, LogicTreeParam);

	// plugin callback
//	for (uint i=0; i<theApp.Plugins.size(); ++i)
//		theApp.Plugins[i]->primitiveChanged(locator.Primitive);
}

// ***************************************************************************

void CWorldEditorDoc::modifyProject ()
{
	_Modified = true;
}

// ***************************************************************************

void CWorldEditorDoc::updateFiles ()
{
	// Landscape
	uint i;
	for (i=0; i<_DataHierarchy.size (); i++)
	{
		// Check date
		if (!checkFileDate (_DataHierarchy[i].Filename.c_str (), _DataHierarchy[i].LastModifedTime))
		{
			// Ask for reloading
			if (theApp.yesNoMessage ("The file \"%s\" has been modified.\nReload it ?", _DataHierarchy[i].Filename.c_str ()))
			{
				// Clear modification
				clearModifications ();

				// Landscape ?
				if (_DataHierarchy[i].Type == CDatabaseElement::Landscape)
				{
					try
					{
						CIFile fileIn;
						if (fileIn.open (_DataHierarchy[i].Filename))
						{
							CIXml xml(true);
							xml.init (fileIn);
							_DataHierarchy[i].ZoneRegion.serial (xml);
						}
						else
						{
							theApp.errorMessage ("Can't open file %s for reading.", _DataHierarchy[i].Filename.c_str ());
						}
					
						// Last time
						_DataHierarchy[i].LastModifedTime = NLMISC::CFile::getFileModificationDate (_DataHierarchy[i].Filename);
					}
					catch (Exception& e)
					{
						theApp.errorMessage ("Error reading file %s : %s", _DataHierarchy[i].Filename.c_str (), e.what ());
					}
				}
				else
				{
					try
					{
						CIFile fileIn;
						if (fileIn.open (_DataHierarchy[i].Filename))
						{
							// Xml stream
							CIXml xmlIn;
							xmlIn.init (fileIn);

							// Read it
							CPrimitiveContext::instance().CurrentPrimitive = &(_DataHierarchy[i].Primitives);
							if (!_DataHierarchy[i].Primitives.read (xmlIn.getRootNode (), _DataHierarchy[i].Filename.c_str(), theApp.Config))
							{
								theApp.errorMessage ("Error reading file %s", _DataHierarchy[i].Filename.c_str ());
								CPrimitiveContext::instance().CurrentPrimitive = NULL;
							}
							CPrimitiveContext::instance().CurrentPrimitive = NULL;

							// Last time
							_DataHierarchy[i].LastModifedTime = NLMISC::CFile::getFileModificationDate (_DataHierarchy[i].Filename);

							InvalidateAllPrimitives();

						}
						else
						{
							theApp.errorMessage ("Can't open file %s for reading.", _DataHierarchy[i].Filename.c_str ());
						}
					}
					catch (Exception& e)
					{
						theApp.errorMessage ("Error reading file %s : %s", _DataHierarchy[i].Filename.c_str (), e.what ());
					}
				}

				// Invalidate views
				invalidateLeftView ();
			}
			else
			{
				// Get the new date
				_DataHierarchy[i].LastModifedTime = NLMISC::CFile::getFileModificationDate (_DataHierarchy[i].Filename);
			}
		}
	}
	
	// Check date
	if (!checkFileDate (GetPathName (), _LastModifedTime))
	{
		// Ask for reloading
		if (theApp.yesNoMessage ("The file \"%s\" has been modified.\nReload it ?", (const char*)GetPathName ()))
		{
			newDocument ();
			loadDocument (GetPathName ());
		}
		else
		{
			// Get the new date
			_LastModifedTime = NLMISC::CFile::getFileModificationDate ((const char*)GetPathName ());
		}
	}
}

// ***************************************************************************

bool CWorldEditorDoc::checkFileDate (const char *filename, uint32 date)
{
	// File exist ?
	if (!NLMISC::CFile::fileExists (filename))
		return true;

	// Check each file in the project
	return NLMISC::CFile::getFileModificationDate (filename) == date;
}

// ***************************************************************************

bool CWorldEditorDoc::undoAvailable () const
{
	return _Undo > 0;
}

// ***************************************************************************

bool CWorldEditorDoc::redoAvailable () const
{
	return _Undo < _Actions.size ();
}

// ***************************************************************************

bool CWorldEditorDoc::isPrimitiveLoaded(const std::string &primPath)
{
	uint i = 0, count = getNumDatabaseElement();
	string curPath;

	while (i < count)
	{
		getFilePath(i, curPath);
		if (primPath == curPath)
			break;
		else
			i++;
	}

	if (i < count)
		return true;
	else
		return false;
}

// ***************************************************************************

bool CWorldEditorDoc::isLandscape (uint dbIndex) const
{
	return _DataHierarchy[dbIndex].Type == CDatabaseElement::Landscape;
}

// ***************************************************************************

bool CWorldEditorDoc::isPrimitive (uint dbIndex) const
{
	return _DataHierarchy[dbIndex].Type == CDatabaseElement::Primitive;
}

// ***************************************************************************

bool CWorldEditorDoc::isEditable  (uint dbIndex) const
{
	return _DataHierarchy[dbIndex].Editable;
}

// ***************************************************************************

void CWorldEditorDoc::resetUniqueID (const NLLIGO::IPrimitive &primitive, bool onlyZero)
{
	// Got a primitive class ?
	const CPrimitiveClass *_class = theApp.Config.getPrimitiveClass (primitive);
	if (_class)
	{
		// For each parameters
		uint i;
		for (i=0; i<_class->Parameters.size (); i++)
		{
			// String or string array ?
			if (_class->Parameters[i].Type == CPrimitiveClass::CParameter::String)
			{
				// Default value available ?
				if (!_class->Parameters[i].DefaultValue.empty ())
				{
					// Unique Id ?
					if (_class->Parameters[i].DefaultValue[0].GenID)
					{
						string propVal;
						primitive.getPropertyByName(_class->Parameters[i].Name.c_str(), propVal);
						if (!onlyZero || propVal.empty() || propVal == "0")
						{
							// Remove it
							CDatabaseLocatorPointer locator;
							getLocator (locator, &primitive);
							
							addModification (new CActionSetPrimitivePropertyString (locator, _class->Parameters[i].Name.c_str (), toString (getUniqueId ()).c_str (), false));
						}
					}
				}
			}
			else if (_class->Parameters[i].Type == CPrimitiveClass::CParameter::StringArray)
			{
				uint j;
				for (j=0; j<_class->Parameters[i].DefaultValue.size (); j++)
				{
					// Unique Id ?
					if (_class->Parameters[i].DefaultValue[j].GenID)
					{
						string propVal;
						primitive.getPropertyByName(_class->Parameters[j].Name.c_str(), propVal);
						if (!onlyZero || propVal.empty() || propVal ==  "0")
						{
							// The doesn't exist ?
							std::vector<string> result;
							std::vector<string> *resultPtr = NULL;
							primitive.getPropertyByName (_class->Parameters[i].Name.c_str (), resultPtr);

							// Copy
							if (resultPtr)
								result = *resultPtr;

							// Resize
							if (result.size ()<=j)
								result.resize (j+1);

							// Set the value
							result[j] = toString (getUniqueId ());

							CDatabaseLocatorPointer locator;
							getLocator (locator, &primitive);
							addModification (new CActionSetPrimitivePropertyStringArray (locator, _class->Parameters[i].Name.c_str (), result, false));
						}
					}
				}
			}
		}
	}

	// Recurcive call
	uint i;
	const uint count = primitive.getNumChildren ();
	for (i=0; i<count; i++)
	{
		// Get the child
		const IPrimitive *child;
		nlverify (primitive.getChild (child, i));
		resetUniqueID (*child, onlyZero);
	}
}

// ***************************************************************************
void CWorldEditorDoc::forceIDUniqueness(const NLLIGO::IPrimitive &primitive, CHashSet<std::string> &ids, std::vector<TPropertyNonUnique> &nonUnique)
{
	// Got a primitive class ?
	const CPrimitiveClass *_class = theApp.Config.getPrimitiveClass (primitive);
	if (_class)
	{
		// For each parameters
		uint i;
		for (i=0; i<_class->Parameters.size (); i++)
		{
			// String or string array ?
			if (_class->Parameters[i].Type == CPrimitiveClass::CParameter::String)
			{
				// Default value available ?
				if (!_class->Parameters[i].DefaultValue.empty ())
				{
					// Unique Id ?
					if (_class->Parameters[i].DefaultValue[0].GenID)
					{
						string propVal;
						primitive.getPropertyByName(_class->Parameters[i].Name.c_str(), propVal);
						if (ids.find(propVal) != ids.end())
						{
							// store it in the list of non unique id
							TPropertyNonUnique nu;
							nu.Primitive = &primitive;
							nu.PropertyName = _class->Parameters[i].Name;

							nonUnique.push_back(nu);
//							// regenerate it
//							CDatabaseLocatorPointer locator;
//							getLocator (locator, &primitive);
//							
//							addModification (new CActionSetPrimitivePropertyString (locator, _class->Parameters[i].Name.c_str (), toString (getUniqueId ()).c_str (), false));
//
//							regenCount++;
						}
						primitive.getPropertyByName(_class->Parameters[i].Name.c_str(), propVal);
						ids.insert(propVal);
					}
				}
			}
			else if (_class->Parameters[i].Type == CPrimitiveClass::CParameter::StringArray)
			{
				uint j;
				for (j=0; j<_class->Parameters[i].DefaultValue.size (); j++)
				{
					// Unique Id ?
					if (_class->Parameters[i].DefaultValue[j].GenID)
					{
						string propVal;
						primitive.getPropertyByName(_class->Parameters[j].Name.c_str(), propVal);
						if (ids.find(propVal) != ids.end())
						{
							// store it in the list of non unique id
							TPropertyNonUnique nu;
							nu.Primitive = &primitive;
							nu.PropertyName = _class->Parameters[j].Name;

							nonUnique.push_back(nu);
//							// The doesn't exist ?
//							std::vector<string> result;
//							std::vector<string> *resultPtr = NULL;
//							primitive.getPropertyByName (_class->Parameters[i].Name.c_str (), resultPtr);
//
//							// Copy
//							if (resultPtr)
//								result = *resultPtr;
//
//							// Resize
//							if (result.size ()<=j)
//								result.resize (j+1);
//
//							// Set the value
//							result[j] = toString (getUniqueId ());
//
//							CDatabaseLocatorPointer locator;
//							getLocator (locator, &primitive);
//							addModification (new CActionSetPrimitivePropertyStringArray (locator, _class->Parameters[i].Name.c_str (), result, false));
//
//							regenCount++;
						}
						primitive.getPropertyByName(_class->Parameters[j].Name.c_str(), propVal);
						ids.insert(propVal);
					}
				}
			}
		}
	}

	// Recursive call
	uint i;
	const uint count = primitive.getNumChildren ();
	for (i=0; i<count; i++)
	{
		// Get the child
		const IPrimitive *child;
		nlverify (primitive.getChild (child, i));
		forceIDUniqueness (*child, ids, nonUnique);
	}
}

// ***************************************************************************

void CWorldEditorDoc::modifyPropertyDlg()
{
	
	/*if (PropertyDialog.IsWindowVisible() &&PropertyDialog.isModified())
	{
		if(theApp.yesNoMessage("Some entries seem to be modified.\nWould you like to save the last primitive?"))
			PropertyDialog.updateModification ();
		else
			PropertyDialog.updateModifiedState();
	}*/
}

// ***************************************************************************

bool CWorldEditorDoc::updateDefaultValuesInternal (NLLIGO::IPrimitive &primitive)
{
	// Modified
	bool modified = false;

	// Got a primitive class ?
	const CPrimitiveClass *_class = theApp.Config.getPrimitiveClass (primitive);
	if (_class)
	{
		// For each parameters
		uint i;
		for (i=0; i<_class->Parameters.size (); i++)
		{
			// First check the primitive property has to good type
			IProperty *prop;
			if (primitive.getPropertyByName (_class->Parameters[i].Name.c_str (), prop))
			{
				// String to array ?
				CPropertyString *propString = dynamic_cast<CPropertyString *> (prop);;
				const bool classStringArray = _class->Parameters[i].Type == CPrimitiveClass::CParameter::StringArray || 
					_class->Parameters[i].Type == CPrimitiveClass::CParameter::ConstStringArray;
				if (propString && classStringArray)
				{
					// Build an array string
					vector<string> array;
					if (!propString->String.empty())
						array.push_back (propString->String);
					prop = new CPropertyStringArray (array);
					primitive.removePropertyByName (_class->Parameters[i].Name.c_str ());
					primitive.addPropertyByName (_class->Parameters[i].Name.c_str (), prop);
					modified = true;
				}

				// Array to string ?
				CPropertyStringArray *propStringArray = dynamic_cast<CPropertyStringArray *> (prop);
				if (propStringArray && !classStringArray)
				{
					// Build an array string
					string str;
					if (!propStringArray->StringArray.empty())
						str = propStringArray->StringArray[0];
					prop = new CPropertyString (str);
					primitive.removePropertyByName (_class->Parameters[i].Name.c_str ());
					primitive.addPropertyByName (_class->Parameters[i].Name.c_str (), prop);
					modified = true;
				}
			}

			// String or string array ?
			if (_class->Parameters[i].Type == CPrimitiveClass::CParameter::String)
			{
				// Default value available ?
				if (!_class->Parameters[i].DefaultValue.empty ())
				{
					// Unique Id ?
					if (_class->Parameters[i].DefaultValue[0].GenID)
					{
						// The doesn't exist ?
						string result;
						if (!primitive.getPropertyByName (_class->Parameters[i].Name.c_str (), result))
						{
							// Add it !
							primitive.addPropertyByName (_class->Parameters[i].Name.c_str (), new CPropertyString (toString (getUniqueId ()).c_str ()));
							modified = true;
						}
					}
					// Hidden ?
					else if (!_class->Parameters[i].Visible)
					{
						// The doesn't exist ?
						string result;
						if (!primitive.getPropertyByName (_class->Parameters[i].Name.c_str (), result))
						{
							// Add it !
							primitive.addPropertyByName (_class->Parameters[i].Name.c_str (), new CPropertyString (""));
							modified = true;
						}
					}
				}
			}
			else if ( (_class->Parameters[i].Type == CPrimitiveClass::CParameter::StringArray) ||
						(_class->Parameters[i].Type == CPrimitiveClass::CParameter::ConstStringArray) )
			{
				uint j;
				for (j=0; j<_class->Parameters[i].DefaultValue.size (); j++)
				{
					// Unique Id ?
					if (_class->Parameters[i].DefaultValue[j].GenID)
					{
						// The doesn't exist ?
						std::vector<string> result;
						std::vector<string> *resultPtr = NULL;
						if (!primitive.getPropertyByName (_class->Parameters[i].Name.c_str (), resultPtr) || (resultPtr->size ()<=j))
						{
							// Copy
							if (resultPtr)
								result = *resultPtr;

							// Resize
							if (result.size ()<=j)
								result.resize (j+1);

							// Resize to it
							primitive.removePropertyByName (_class->Parameters[i].Name.c_str ());

							// Set the value
							result[j] = toString (getUniqueId ());

							// Add the new property array
							primitive.addPropertyByName (_class->Parameters[i].Name.c_str (), new CPropertyStringArray (result));
							modified = true;
						}
					}
					// Hidden ?
					else if (!_class->Parameters[i].Visible)
					{
						// The doesn't exist ?
						std::vector<string> result;
						std::vector<string> *resultPtr = NULL;
						if (!primitive.getPropertyByName (_class->Parameters[i].Name.c_str (), resultPtr) || (resultPtr->size ()<=j))
						{
							// Copy
							if (resultPtr)
								result = *resultPtr;

							// Resize
							if (result.size ()<=j)
								result.resize (j+1);

							// Resize to it
							primitive.removePropertyByName (_class->Parameters[i].Name.c_str ());

							// Set the value
							result[j] = "";

							// Add the new property array
							primitive.addPropertyByName (_class->Parameters[i].Name.c_str (), new CPropertyStringArray (result));
							modified = true;
						}
					}
				}
			}
			else
			{
				// Default value available ?
				if (!_class->Parameters[i].DefaultValue.empty ())
				{
					// Hidden ?
					if (!_class->Parameters[i].Visible)
					{
						// The doesn't exist ?
						string result;
						if (!primitive.getPropertyByName (_class->Parameters[i].Name.c_str (), result))
						{
							// Add it !
							primitive.addPropertyByName (_class->Parameters[i].Name.c_str (), new CPropertyString (""));
							modified = true;
						}
					}
				}
			}
		}
	}

	// Recurcive call
	uint i;
	const uint count = primitive.getNumChildren ();
	for (i=0; i<count; i++)
	{
		// Get the child
		IPrimitive *child;
		nlverify (primitive.getChild (child, i));
		modified |= updateDefaultValuesInternal (*child);
	}

	return modified;
}

// ***************************************************************************

void CWorldEditorDoc::updateDefaultValues (uint dbIndex)
{
	// Ref on the db
	CDatabaseElement &dbElm = _DataHierarchy[dbIndex];
	
	// Primitive is valid 
	if (dbElm.Primitives.RootNode)
	{
		// Modify something ?
		if (updateDefaultValuesInternal (*dbElm.Primitives.RootNode))
		{
			// Information 
			theApp.infoMessage ("In file (%s) : Some primitives have been modified to initialise their default values\nor to change their properties type.", dbElm.Filename.c_str ());

			// Invalidate the document
			modifyDatabase (dbIndex);
		}
	}
}

// ***************************************************************************



BOOL CWorldEditorDoc::OnCmdMsg(UINT nID, int nCode, void* pExtra,
						  AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// If pHandlerInfo is NULL, then handle the message
	if (pHandlerInfo == NULL)
	{
		CCmdUI* pCmdUI=(CCmdUI*)pExtra;
		// Filter the commands sent to a text color menu option
		for (uint i = ID_WINDOWS_PLUGINS+1; i < ID_WINDOWS_PLUGINS+1+theApp.Plugins.size(); i++)
		{
			if (nID ==(UINT) (i))
			{
				if (nCode == CN_COMMAND)
				{
					if(theApp.Plugins.at(i-ID_WINDOWS_PLUGINS-1)->isActive())
						theApp.Plugins.at(i-ID_WINDOWS_PLUGINS-1)->closePlugin();
					else
						theApp.Plugins.at(i-ID_WINDOWS_PLUGINS-1)->activatePlugin();

				}
				else if (nCode == CN_UPDATE_COMMAND_UI)
				{
					// Update UI element state
					
					if(theApp.Plugins.at(i-ID_WINDOWS_PLUGINS-1)->isActive())
						pCmdUI->SetCheck();
					else
						pCmdUI->SetCheck(0);
					pCmdUI->Enable();
					
				}
				return TRUE;
			}
		}
	}

	// If we didn't process the command, call the base class
	// version of OnCmdMsg so the message-map can handle the message
	return CDocument::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// ***************************************************************************





// ***************************************************************************
// CDatabaseLocator
// ***************************************************************************

uint CDatabaseLocator::getDatabaseIndex () const
{
	return _LocateStack[0];
}

// ***************************************************************************

void CDatabaseLocator::getParent ()
{
	_LocateStack.resize (_LocateStack.size ()-1);
}

// ***************************************************************************
// CDatabaseLocatorPointer
// ***************************************************************************

void CDatabaseLocatorPointer::getParent ()
{
	nlassert (Primitive);
	CDatabaseLocator::getParent ();

	// Get the parent pointer
	Primitive = Primitive->getParent ();
}

// ***************************************************************************

bool CDatabaseLocatorPointer::operator== (const CDatabaseLocatorPointer &other) const
{
	return CDatabaseLocator::operator== (other);
}

// ***************************************************************************

void CDatabaseLocatorPointer::appendChild (CDatabaseLocator &dest) const
{
	nlassert (Primitive);
	dest._LocateStack = _LocateStack;
	dest._LocateStack.push_back (Primitive->getNumChildren ());
	dest.XSubPrim = 0xffffffff;
}

// ***************************************************************************

bool CDatabaseLocatorPointer::next ()
{
	// Should not be end
	if (_LocateStack[0] == 0xffffffff)
		return false;

	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// We are in a primitive ?
	if (Primitive)
	{
		// Some children to visite ?
		if (Primitive->getNumChildren () > 0)
		{
			// Get the first child
			nlverify (Primitive->getChild (Primitive, 0));

			// Push
			_LocateStack.push_back (0);

			// Done
			return true;
		}

		// Try to visit a brother
		const IPrimitive *parent = Primitive->getParent ();

		// Have a parent ?
		while (parent)
		{
			// The child Id
			uint childId;
			
			nlverify (parent->getChildId (childId, Primitive));
			childId++;
			if (childId < parent->getNumChildren ())
			{
				nlassert (_LocateStack.back () == childId-1);
				_LocateStack.back () = childId;
				nlverify (parent->getChild (Primitive, childId));
				return true;
			}
			else
			{
				// Get next parent 
				Primitive = parent;
				parent = parent->getParent ();

				// Unstack
				_LocateStack.pop_back ();
			}
		}

		// Next primitive Id
		_LocateStack[0]++;
		nlassert (_LocateStack.size () == 1);
		Primitive = NULL;
		if (_LocateStack[0] < doc->_DataHierarchy.size ())
		{
			return true;
		}

		// End
		_LocateStack[0] = 0xffffffff;
		return false;
	}
	else
	{
		Primitive = doc->_DataHierarchy[_LocateStack[0]].Primitives.RootNode;
		return true;
	}
}

// ***************************************************************************

void CDatabaseLocatorPointer::getRoot (uint i)
{
	XSubPrim = 0xffffffff;
	_LocateStack.clear ();
	_LocateStack.push_back (i);

	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	Primitive = doc->_DataHierarchy[i].Primitives.RootNode;
}

// ***************************************************************************

void CDatabaseLocatorPointer::getRoot (const string& rootFileName)
{
	XSubPrim = 0xffffffff;
	_LocateStack.clear ();
	

	// Get the document
	CWorldEditorDoc *doc = getDocument ();
	uint count = doc->getNumDatabaseElement ();
	uint i=0;
	for(i=0;i<count;i++)
		if(rootFileName.compare(doc->_DataHierarchy[i].Filename)==0)
			break; 
	if(i<count)
	{
		Primitive=doc->_DataHierarchy[i].Primitives.RootNode;
		_LocateStack.push_back (i);
	}
	else
	{
		exception e("Bad primitive File Name. Please refresh.");
		throw(e);
	}
}

// ***************************************************************************

string& CDatabaseLocatorPointer::getRootFileName (IPrimitive* rootPrim)
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();
	uint count = doc->getNumDatabaseElement ();
	uint i=0;
	for(i=0;i<count;i++)
		if(doc->_DataHierarchy[i].Primitives.RootNode ==rootPrim)
			break;

	if(i==count)
	{
		exception e("Bad primitive File Name. Please refresh.");
		throw(e);
	}	

	return doc->_DataHierarchy[i].Filename;

}

// ***************************************************************************

bool CDatabaseLocatorPointer::firstChild ()
{
	XSubPrim = 0xffffffff;
	// Should not be end
	if (_LocateStack[0] == 0xffffffff)
		return false;

	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Some children to visite ?
	if (Primitive->getNumChildren () > 0)
	{
		// Get the first child
		nlverify (Primitive->getChild (Primitive, 0));

		// Push
		_LocateStack.push_back (0);

		// Done
		return true;
	}
	else
	{
		// Primitive doesn't exist
		Primitive = NULL;

		// Push
		_LocateStack.push_back (0);

		// Done
		return false;
	}
}

// ***************************************************************************

bool CDatabaseLocatorPointer::nextChild ()
{
	// Try to visit a brother
	const IPrimitive *parent = Primitive->getParent ();

	// Have a parent ?
	nlassert (parent);

	// The child Id
	uint childId;
	
	nlverify (parent->getChildId (childId, Primitive));
	childId++;
	if (childId < parent->getNumChildren ())
	{
		nlassert (_LocateStack.back () == childId-1);
		_LocateStack.back () = childId;
		nlverify (parent->getChild (Primitive, childId));
		return true;
	}
	else
	{
		// Get next parent 
		Primitive = parent;
		parent = parent->getParent ();

		// Unstack
		_LocateStack.pop_back ();
		return false;
	}
}

// ***************************************************************************

bool CDatabaseLocatorPointer::previousChild ()
{
	// Try to visit a brother
	const IPrimitive *parent = Primitive->getParent ();

	// Have a parent ?
	nlassert (parent);

	// The child Id
	uint childId;
	
	nlverify (parent->getChildId (childId, Primitive));
	if (childId > 0)
	{
		childId--;
		nlassert (_LocateStack.back () == childId+1);
		_LocateStack.back () = childId;
		nlverify (parent->getChild (Primitive, childId));
		return true;
	}
	else
	{
		// Get next parent 
		Primitive = parent;
		parent = parent->getParent ();

		// Unstack
		_LocateStack.pop_back ();
		return false;
	}
}

// ***************************************************************************

bool CDatabaseLocator::operator== (const CDatabaseLocator &other) const
{
	return _LocateStack == other._LocateStack;
}

// ***************************************************************************

std::string CDatabaseLocator::getPathName () const
{
	// Get the doc
	CWorldEditorDoc *doc = getDocument ();

	// First string
	string path;
	doc->getPrimitiveDisplayName (path, _LocateStack[0]);
	
	// Next
	const IPrimitive *prim = doc->_DataHierarchy[_LocateStack[0]].Primitives.RootNode;
	for (uint i=1; i<_LocateStack.size (); i++)
	{
		// Add the child
		prim->getChild (prim, _LocateStack[i]);

		// Get its name
		path += ".";
		string name;
		if (prim->getPropertyByName ("name", name))
			path += name;
	}

	return path;
}

// ***************************************************************************

CDatabaseLocator::CDatabaseLocator (uint region, sint32 x, sint32 y)
{
	_LocateStack.push_back (getDocument ()->regionIDToDatabaseElementID(region));
	XSubPrim = x;
	Y = y;
}

// ***************************************************************************

void CWorldEditorDoc::OnFileOpen() 
{
	theApp.OnFileOpen ();	
}

// ***************************************************************************

void CWorldEditorDoc::OnFileSave() 
{
	theApp.OnFileSave ();	
}

// ***************************************************************************

void CWorldEditorDoc::OnFileSaveAs() 
{
	theApp.OnFileSaveAs ();	
}

// ***************************************************************************

