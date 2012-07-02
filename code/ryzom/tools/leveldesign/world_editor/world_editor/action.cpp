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

#include "action.h"
#include "world_editor.h"
#include "main_frm.h"
#include "world_editor_doc.h"
#include "editor_primitive.h"
#include "nel/sound/background_sound_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

// ***************************************************************************

CActionLigoTile::CActionLigoTile (const CLigoData &data, const CDatabaseLocator &locator)
{
	// Backup new data
	_New = data;

	// Backup old data
	CWorldEditorDoc *doc = getDocument ();
	doc->getLigoData (_Old, locator);

	// Backup locator
	_Locator = locator;
}

// ***************************************************************************

void CActionLigoTile::undo ()
{
	set (_Old);
	invalidateLeftView ();
	CWorldEditorDoc *doc = getDocument ();
	doc->modifyDatabase (_Locator.getDatabaseIndex ());
}

// ***************************************************************************

bool CActionLigoTile::redo ()
{
	set (_New);
	invalidateLeftView ();
	CWorldEditorDoc *doc = getDocument ();
	doc->modifyDatabase (_Locator.getDatabaseIndex ());
	return true;
}

// ***************************************************************************

void CActionLigoTile::set (const CLigoData &data)
{
	CWorldEditorDoc *doc = getDocument ();
	NLLIGO::CZoneRegion &region = doc->_DataHierarchy[_Locator.getDatabaseIndex ()].ZoneRegion;
	nlassert ((_Locator.XSubPrim >= region.getMinX ()) && (_Locator.XSubPrim <= region.getMaxX ()) && (_Locator.Y >= region.getMinY ()) && (_Locator.Y <= region.getMaxY ()));

	region.setPosX (_Locator.XSubPrim, _Locator.Y, data.PosX);
	region.setPosY (_Locator.XSubPrim, _Locator.Y, data.PosY);
	region.setName (_Locator.XSubPrim, _Locator.Y, data.ZoneName);
	region.setRot (_Locator.XSubPrim, _Locator.Y, data.Rot);
	region.setFlip (_Locator.XSubPrim, _Locator.Y, data.Flip);
	region.setSharingMatNames (_Locator.XSubPrim, _Locator.Y, 0, data.SharingMatNames[0]);
	region.setSharingMatNames (_Locator.XSubPrim, _Locator.Y, 1, data.SharingMatNames[1]);
	region.setSharingMatNames (_Locator.XSubPrim, _Locator.Y, 2, data.SharingMatNames[2]);
	region.setSharingMatNames (_Locator.XSubPrim, _Locator.Y, 3, data.SharingMatNames[3]);
	region.setSharingCutEdges (_Locator.XSubPrim, _Locator.Y, 0, data.SharingCutEdges[0]);
	region.setSharingCutEdges (_Locator.XSubPrim, _Locator.Y, 1, data.SharingCutEdges[1]);
	region.setSharingCutEdges (_Locator.XSubPrim, _Locator.Y, 2, data.SharingCutEdges[2]);
	region.setSharingCutEdges (_Locator.XSubPrim, _Locator.Y, 3, data.SharingCutEdges[3]);
}

// ***************************************************************************

CActionLigoMove::CActionLigoMove (uint index, sint32 deltaX, sint32 deltaY)
{
	_Index = index;
	_DeltaX = deltaX;
	_DeltaY = deltaY;
}

// ***************************************************************************

void CActionLigoMove::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	NLLIGO::CZoneRegion &region = doc->_DataHierarchy[_Index].ZoneRegion;
	region.setMaxX (region.getMaxX () - _DeltaX);
	region.setMinX (region.getMinX () - _DeltaX);
	region.setMaxY (region.getMaxY () - _DeltaY);
	region.setMinY (region.getMinY () - _DeltaY);
	doc->modifyDatabase (_Index);
}

// ***************************************************************************

bool CActionLigoMove::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	NLLIGO::CZoneRegion &region = doc->_DataHierarchy[_Index].ZoneRegion;
	region.setMaxX (region.getMaxX () + _DeltaX);
	region.setMinX (region.getMinX () + _DeltaX);
	region.setMaxY (region.getMaxY () + _DeltaY);
	region.setMinY (region.getMinY () + _DeltaY);
	doc->modifyDatabase (_Index);
	return true;
}

// ***************************************************************************

CActionLigoResize::CActionLigoResize (uint index, sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY)
{
	_Index = index;
	_NewMinX = newMinX;
	_NewMaxX = newMaxX;
	_NewMinY = newMinY;
	_NewMaxY = newMaxY;

	// Backup old region zone
	CWorldEditorDoc *doc = getDocument ();
	_Old = doc->_DataHierarchy[_Index].ZoneRegion;	
}

// ***************************************************************************

void CActionLigoResize::undo ()
{
	// Restaure old region zone
	CWorldEditorDoc *doc = getDocument ();
	doc->_DataHierarchy[_Index].ZoneRegion = _Old;
	doc->modifyDatabase (_Index);
}

// ***************************************************************************

bool CActionLigoResize::redo ()
{
	// Get the zone region
	CWorldEditorDoc *doc = getDocument ();
	NLLIGO::CZoneRegion &region = doc->_DataHierarchy[_Index].ZoneRegion;

	sint32 i, j;
	vector<CLigoData> newZones;
	newZones.resize ((1+_NewMaxX-_NewMinX)*(1+_NewMaxY-_NewMinY));

	sint32 newStride = 1+_NewMaxX-_NewMinX;
	sint32 Stride = 1+region.getMaxX ()-region.getMinX ();

	for (j = _NewMinY; j <= _NewMaxY; ++j)
	for (i = _NewMinX; i <= _NewMaxX; ++i)
	{
		// Ref on the new value
		CLigoData &data = newZones[(i-_NewMinX)+(j-_NewMinY)*newStride];

		// In the old array ?
		if ((i >= region.getMinX ())&&(i <= region.getMaxX ())&&(j >= region.getMinY ())&&(j <= region.getMaxY ()))
		{
			// Backup values
			data.PosX = region.getPosX (i, j);
			data.PosY = region.getPosY (i, j);
			data.ZoneName = region.getName (i, j);
			data.Rot = region.getRot (i, j);
			data.Flip = region.getFlip (i, j);
			data.SharingMatNames[0] = region.getSharingMatNames (i, j, 0);
			data.SharingMatNames[1] = region.getSharingMatNames (i, j, 1);
			data.SharingMatNames[2] = region.getSharingMatNames (i, j, 2);
			data.SharingMatNames[3] = region.getSharingMatNames (i, j, 3);
			data.SharingCutEdges[0] = region.getSharingCutEdges (i, j, 0);
			data.SharingCutEdges[1] = region.getSharingCutEdges (i, j, 1);
			data.SharingCutEdges[2] = region.getSharingCutEdges (i, j, 2);
			data.SharingCutEdges[3] = region.getSharingCutEdges (i, j, 3);
		}
		else
		{
			// By default
			data.PosX = 0;
			data.PosY = 0;
			data.ZoneName = "";
			data.Rot = 0;
			data.Flip = 0;
			data.SharingMatNames[0] = "";
			data.SharingMatNames[1] = "";
			data.SharingMatNames[2] = "";
			data.SharingMatNames[3] = "";
			data.SharingCutEdges[0] = 0;
			data.SharingCutEdges[1] = 0;
			data.SharingCutEdges[2] = 0;
			data.SharingCutEdges[3] = 0;
		}
	}
	region.resize (_NewMinX, _NewMaxX, _NewMinY, _NewMaxY);

	for (j = _NewMinY; j <= _NewMaxY; ++j)
	for (i = _NewMinX; i <= _NewMaxX; ++i)
	{
		// Ref on the new value
		const CLigoData &data = newZones[(i-_NewMinX)+(j-_NewMinY)*newStride];

		region.setName (i, j, data.ZoneName);
		region.setPosX (i, j, data.PosX);
		region.setPosY (i, j, data.PosY);
		region.setRot (i, j, data.Rot);
		region.setFlip (i, j, data.Flip);
		uint k;
		for (k=0; k<4; k++)
		{
			region.setSharingMatNames (i, j, k, data.SharingMatNames[k]);
			region.setSharingCutEdges (i, j, k, data.SharingCutEdges[k]);
		}
	}
	
	doc->modifyDatabase (_Index);
	return true;
}

// ***************************************************************************
// CActionImportPrimitive
// ***************************************************************************

CActionImportPrimitive::CActionImportPrimitive (const char *oldPrimFile)
{
	_Filename = oldPrimFile;
	_FirstLoad = true;
}

// ***************************************************************************

void CActionImportPrimitive::undo ()
{
	// Check we have a primitive
	CWorldEditorDoc *doc = getDocument ();
	nlassert (doc->_DataHierarchy.size () > 0);

	// Remove back primitive
	doc->_DataHierarchy.resize (doc->_DataHierarchy.size ()-1);
	doc->_DataHierarchy.recomputePointerArray ();

	// Invalidate tools
	getMainFrame ()->invalidateTools ();
	InvalidateAllPrimitives ();

	// Modify files
	doc->modifyProject ();
}

// ***************************************************************************

bool CActionImportPrimitive::redo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// First redo ?
	if (_FirstLoad)
	{
		// Load the old primitive	
		try
		{
			// The file
			CIFile fileIn;
			if (fileIn.open (_Filename))
			{
				// The region to load
				NLLIGO::CPrimRegion region;

				// Load it
				CIXml input;
				input.init (fileIn);
				region.serial (input);
				
				// Convert it
				NLLIGO::CPrimitives		Primitives;
				Primitives.convert (region);

				if (_Filename.find("audio") != std::string::npos)
				{
					// import audio prim

					// Create an audio node
					IPrimitive *audio = new CPrimNodeEditor;
					std::string audioName = region.Name;
					audioName = audioName.substr(0, audioName.find(".prim"));
					audio->addPropertyByName ("name", new CPropertyString (audioName.c_str()));
					audio->addPropertyByName ("class", new CPropertyString ("audio"));

					// Insert the audio node
					_Primitive.RootNode->insertChild (audio);

					IPrimitive *sounds = new CPrimNodeEditor;
					sounds->addPropertyByName ("name", new CPropertyString ("sounds"));
					sounds->addPropertyByName ("class", new CPropertyString ("sounds"));

					// Insert the soundnode
					audio->insertChild (sounds);

					IPrimitive *soundBanks = new CPrimNodeEditor;
					soundBanks->addPropertyByName ("name", new CPropertyString ("sample_banks"));
					soundBanks->addPropertyByName ("class", new CPropertyString ("sample_banks"));

					// Insert the soundnode
					audio->insertChild (soundBanks);

					IPrimitive *envFx = new CPrimNodeEditor;
					envFx->addPropertyByName ("name", new CPropertyString ("env_fx"));
					envFx->addPropertyByName ("class", new CPropertyString ("env_fx"));

					// Insert the soundnode
					audio->insertChild (envFx);

					// layer count count
					uint count = Primitives.RootNode->getNumChildren ();

					for (uint i=0; i<count; ++i)
					{
						// Get the layer node
						IPrimitive *layer;
						nlverify (Primitives.RootNode->getChild (layer, i));

						// Get the name
						string layerName;
						layer->getPropertyByName ("name", layerName);

						// Create a sound_folder node
						IPrimitive *soundFolder = new CPrimNodeEditor;
						soundFolder->addPropertyByName ("name", new CPropertyString (layerName.c_str ()));
						soundFolder->addPropertyByName ("class", new CPropertyString ("sound_folder"));

						// Insert the sound folder node
						sounds->insertChild (soundFolder);

						// sound count
						uint soundCount = layer->getNumChildren ();

						// Make sounds
						for (uint j=0; j<soundCount; j++)
						{
							// Get the primitive
							IPrimitive *prim;
							nlverify (layer->getChild (prim, j));

							// Get the name
							string rawName;
							prim->getPropertyByName ("name", rawName);
							string name;
							uint layerId = 0;

							// extract the sound name and, optinaly, the layer id
							uint n = std::count(rawName.begin(), rawName.end(), '-');
							std::vector<std::string>	splited;

							NLMISC::explode(rawName, string("-"), splited);

							if (n == 2)
							{
								// no layer spec
								name = splited[1];
							}
							else if (n==3)
							{
								// layer is specified
								name = splited[2];
								layerId = 'a' - splited[1][0];
								clamp(layerId, 0u, NLSOUND::BACKGROUND_LAYER-1);
							}
							else
							{
								nlwarning("sound %s have a bad format. Ignoring", rawName.c_str());
								// skip this one
								continue;
							}

							char layerString[1024];
							sprintf(layerString, "layer_%u", layerId);
							
							// Path ?
							const CPrimPathEditor *path = dynamic_cast<const CPrimPathEditor *> (prim);
							if (path)
							{
								// Copy the node
								CPrimPathEditor *newPrim = new CPrimPathEditor;
								*newPrim = *path;
								newPrim->addPropertyByName ("class", new CPropertyString ("sound_path"));
								newPrim->removePropertyByName("name");
								newPrim->addPropertyByName ("name", new CPropertyString (name.c_str()));
								newPrim->addPropertyByName ("layer", new CPropertyString (layerString));

								// Add the primitive
								soundFolder->insertChild (newPrim);
							}
							else
							{
								// Zone ?
								const CPrimZoneEditor *zone = dynamic_cast<const CPrimZoneEditor *> (prim);
								if (zone)
								{
									// Copy the node
									CPrimZoneEditor *newPrim = new CPrimZoneEditor;
									*newPrim = *zone;
									newPrim->addPropertyByName ("class", new CPropertyString ("sound_zone"));
									newPrim->removePropertyByName("name");
									newPrim->addPropertyByName ("name", new CPropertyString (name.c_str()));
									newPrim->addPropertyByName ("layer", new CPropertyString (layerString));

									// Add the primitive
									soundFolder->insertChild (newPrim);
								}
								else
								{
									// point ?
									const CPrimPointEditor *point = dynamic_cast<const CPrimPointEditor *> (prim);
									if (zone)
									{
										// Copy the node
										CPrimPointEditor *newPrim = new CPrimPointEditor;
										*newPrim = *point;
										newPrim->addPropertyByName ("class", new CPropertyString ("sound_point"));
										newPrim->removePropertyByName("name");
										newPrim->addPropertyByName ("name", new CPropertyString (name.c_str()));
										newPrim->addPropertyByName ("layer", new CPropertyString (layerString));

										// Add the primitive
										soundFolder->insertChild (newPrim);
									}
								}
							}
						}
					}
				}
				else
				{
					// import flora prim

					// Flora count
					uint count = Primitives.RootNode->getNumChildren ();

					// Make flora
					for (uint i=0; i<count; i++)
					{
						// Get the layer node
						IPrimitive *layer;
						nlverify (Primitives.RootNode->getChild (layer, i));

						// Get the name
						string layerName;
						layer->getPropertyByName ("name", layerName);

						// Create a flora node
						IPrimitive *flora = new CPrimNodeEditor;
						flora->addPropertyByName ("name", new CPropertyString (layerName.c_str ()));
						flora->addPropertyByName ("class", new CPropertyString ("flora"));

						// Insert the flora node
						_Primitive.RootNode->insertChild (flora);

						// Flora count
						uint floraCount = layer->getNumChildren ();

						// Make flora
						for (uint j=0; j<floraCount; j++)
						{
							// Get the primitive
							IPrimitive *prim;
							nlverify (layer->getChild (prim, j));

							// Get the name
							string name;
							prim->getPropertyByName ("name", name);

							// Path ?
							const CPrimPathEditor *path = dynamic_cast<const CPrimPathEditor *> (prim);
							if (path)
							{
								// Copy the node
								CPrimPathEditor *newPrim = new CPrimPathEditor;
								*newPrim = *path;
								newPrim->addPropertyByName ("class", new CPropertyString ("flora_path"));

								// Add the primitive
								flora->insertChild (newPrim);
							}
							else
							{
								// Zone ?
								const CPrimZoneEditor *zone = dynamic_cast<const CPrimZoneEditor *> (prim);
								if (zone)
								{
									// Copy the node
									CPrimZoneEditor *newPrim = new CPrimZoneEditor;
									*newPrim = *zone;
									newPrim->addPropertyByName ("class", new CPropertyString ("flora_zone"));

									// Add the primitive
									flora->insertChild (newPrim);
								}
							}
						}
					}
				}

				_FirstLoad = false;
			}
			else
			{
				theApp.errorMessage ("Can't open the file (%s) for reading.", _Filename.c_str ());
				return false;
			}
		}
		catch (Exception& e)
		{
			theApp.errorMessage ("Error reading the file (%s) : (%s).", _Filename.c_str (), e.what ());
			return false;
		}
	}

	// Push back the primitive
	doc->_DataHierarchy.push_back (CWorldEditorDoc::CDatabaseElement (CWorldEditorDoc::CDatabaseElement::Primitive));
	doc->_DataHierarchy.recomputePointerArray ();
	doc->_DataHierarchy.back ().Primitives = _Primitive;

	// Update default values
	doc->updateDefaultValues (doc->_DataHierarchy.size ()-1);

	// Invalidate tools
	getMainFrame ()->invalidateTools ();
	InvalidateAllPrimitives ();

	// Modify files
	doc->modifyProject ();

	return true;
}

// ***************************************************************************
// CActionLoadPrimitive
// ***************************************************************************

CActionLoadPrimitive::CActionLoadPrimitive (const char *oldPrimFile)
{
	_Filename = oldPrimFile;
	_FirstLoad = true;
}

// ***************************************************************************

void CActionLoadPrimitive::undo ()
{
	// Check we have a primitive
	CWorldEditorDoc *doc = getDocument ();
	nlassert (doc->_DataHierarchy.size () > 0);

	// Remove back primitive
	doc->_DataHierarchy.resize (doc->_DataHierarchy.size ()-1);
	doc->_DataHierarchy.recomputePointerArray ();
	InvalidateAllPrimitives ();

	// Invalidate tools
	getMainFrame ()->invalidateTools ();

	// Modify files
	doc->modifyProject ();
}

// ***************************************************************************

bool CActionLoadPrimitive::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	// First redo ?
	if (_FirstLoad)
	{
		// Load the old primitive	
		try
		{
			// The file
			CIFile fileIn;
			if (fileIn.open (_Filename))
			{
				// Load it
				CIXml input;
				input.init (fileIn);
				
				// Set the primitive context
				CPrimitiveContext::instance().CurrentPrimitive = &_Primitive;
				// Convert it
				_Primitive.read (input.getRootNode (), _Filename.c_str (), theApp.Config);

				// cleanup context
				CPrimitiveContext::instance().CurrentPrimitive = NULL;

				_FirstLoad = false;
			}
			else
			{
				theApp.errorMessage ("Can't open the file (%s) for reading.", _Filename.c_str ());
				return false;
			}
		}
		catch (Exception& e)
		{
			theApp.errorMessage ("Error reading the file (%s) : (%s).", _Filename.c_str (), e.what ());
			return false;
		}
	}

	// Push back the primitive
	doc->_DataHierarchy.push_back (CWorldEditorDoc::CDatabaseElement (CWorldEditorDoc::CDatabaseElement::Primitive));
	doc->_DataHierarchy.recomputePointerArray ();
	doc->_DataHierarchy.back ().Primitives = _Primitive;
	doc->_DataHierarchy.back ().Filename = _Filename;
	doc->_DataHierarchy.back ().LastModifedTime = NLMISC::CFile::getFileModificationDate (_Filename);

	// Update default values
	doc->updateDefaultValues (doc->_DataHierarchy.size ()-1);

	// Auto generate missing id
	doc->resetUniqueID(*(doc->_DataHierarchy.back().Primitives.RootNode), true);

	// Invalidate tools
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();

	// Modify files
	doc->modifyProject ();

	return true;
}

// ***************************************************************************
// CActionNewPrimitive
// ***************************************************************************

CActionNewPrimitive::CActionNewPrimitive ()
{
}

// ***************************************************************************

void CActionNewPrimitive::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	// Check we have a primitive
	nlassert (doc->_DataHierarchy.size () > 0);

	// Remove back primitive
	doc->_DataHierarchy.resize (doc->_DataHierarchy.size ()-1);
	doc->_DataHierarchy.recomputePointerArray ();

	// Invalidate tools
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();

	// Modify files
	doc->modifyProject ();
}

// ***************************************************************************

bool CActionNewPrimitive::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	// Push back the primitive
	doc->_DataHierarchy.push_back (CWorldEditorDoc::CDatabaseElement (CWorldEditorDoc::CDatabaseElement::Primitive));
	doc->_DataHierarchy.recomputePointerArray ();

	// Invalidate tools
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();

	// Modify files
	uint index = doc->_DataHierarchy.size ()-1;
	doc->modifyProject ();
	doc->modifyDatabase (index);

	return true;
}

// ***************************************************************************
// CActionClearPrimitives
// ***************************************************************************

CActionClearPrimitives::CActionClearPrimitives ()
{
	// Copy the primitives
	CWorldEditorDoc *doc = getDocument ();
	_Primitives = doc->_DataHierarchy;
}

// ***************************************************************************

void CActionClearPrimitives::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	// Remove all the primitives
	doc->_DataHierarchy = _Primitives;

	// Invalidate
	invalidateLeftView ();
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();

	// Modify files
	doc->modifyProject ();
}

// ***************************************************************************

bool CActionClearPrimitives::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	// Remove all the primitives
	doc->_DataHierarchy.clear ();
	doc->_DataHierarchy.recomputePointerArray ();

	// Invalidate
	invalidateLeftView ();
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();

	// Modify files
	doc->modifyProject ();

	return true;
}

// ***************************************************************************
// CActionUnselectAll
// ***************************************************************************

CActionUnselectAll::CActionUnselectAll ()
{
	CWorldEditorDoc *doc = getDocument ();
	doc->modifyPropertyDlg();
	std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
	while (ite != Selection.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);

		// Primitive ?
		if (locator.Primitive)
		{
			// Add this locator
			_SelectedPrimitives.push_back (locator);
		}

		ite++;
	}
}

// ***************************************************************************

void CActionUnselectAll::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	for (uint i=0; i<_SelectedPrimitives.size (); i++)
	{
		// Get the locator pointer
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, _SelectedPrimitives[i]);

		// Select the primitive
		if (!getPrimitiveEditor(locator.Primitive)->getSelected())
//		if (const_cast<IPrimitive*>(locator.Primitive)->addPropertyByName ("selected", new CPropertyString ()))
		{
			getPrimitiveEditor(locator.Primitive)->setSelected(true);
			InvalidatePrimitive (locator, SelectionState);
			invalidateLeftView ();

			// Modify files
			// doc->modifyDatabase (locator.getDatabaseIndex ());
		}
	}
}

// ***************************************************************************

bool CActionUnselectAll::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	bool modified = false;
	std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
	while (ite != Selection.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);

		// Primitive ?
		if (locator.Primitive)
		{
			// Remove selected flag
			if (getPrimitiveEditor(locator.Primitive)->getSelected())
//			if (const_cast<IPrimitive*>(locator.Primitive)->removePropertyByName ("selected"))
			{
				getPrimitiveEditor(locator.Primitive)->setSelected(false);
				//modified = true;
				InvalidatePrimitive (locator, SelectionState);
				invalidateLeftView ();

				// Modify files
				// doc->modifyDatabase (locator.getDatabaseIndex ());
			}
		}

		ite++;
	}

	return modified;
}

// ***************************************************************************
// CActionSelectAll
// ***************************************************************************

CActionSelectAll::CActionSelectAll ()
{
	CWorldEditorDoc *doc = getDocument ();
	doc->modifyPropertyDlg();
	std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
	while (ite != Selection.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);

		// Primitive ?
		if (locator.Primitive)
		{
			// Add this locator
			_SelectedPrimitives.push_back (locator);
		}

		ite++;
	}
}

// ***************************************************************************

void CActionSelectAll::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	CDatabaseLocatorPointer locator;
	doc->getFirstLocator (locator);
	do
	{
		// Primitive ?
		if (locator.Primitive)
		{
			// Remove selected flag
			if (getPrimitiveEditor(locator.Primitive)->getSelected())
//			if (const_cast<IPrimitive*>(locator.Primitive)->removePropertyByName ("selected"))
			{
				getPrimitiveEditor(locator.Primitive)->setSelected(false);
				InvalidatePrimitive (locator, SelectionState);
				invalidateLeftView ();

				// Modify files
				// doc->modifyDatabase (locator.getDatabaseIndex ());
			}
		}
	}
	while (locator.next ());
	
	for (uint i=0; i<_SelectedPrimitives.size (); i++)
	{
		// Get the locator pointer
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, _SelectedPrimitives[i]);

		// Select the primitive
		if (!getPrimitiveEditor(locator.Primitive)->getSelected())
//		if (const_cast<IPrimitive*>(locator.Primitive)->addPropertyByName ("selected", new CPropertyString ()))
		{
			getPrimitiveEditor(locator.Primitive)->setSelected(false);
			InvalidatePrimitive (locator, SelectionState);
			invalidateLeftView ();

			// Modify files
			// doc->modifyDatabase (locator.getDatabaseIndex ());
		}
	}
}

// ***************************************************************************

bool CActionSelectAll::redo ()
{
	bool modified = false;
	CWorldEditorDoc *doc = getDocument ();
	CDatabaseLocatorPointer locator;
	doc->getFirstLocator (locator);
	do
	{
		// Primitive ?
		if (locator.Primitive)
		{
			// Remove selected flag
			if (!getPrimitiveEditor(locator.Primitive)->getSelected())
//			if (const_cast<IPrimitive*>(locator.Primitive)->addPropertyByName ("selected", new CPropertyString ()))
			{
				getPrimitiveEditor(locator.Primitive)->setSelected(true);
				// modified = true;
				InvalidatePrimitive (locator, SelectionState);
				invalidateLeftView ();

				// Modify files
				// doc->modifyDatabase (locator.getDatabaseIndex ());
			}
		}
	}
	while (locator.next ());

	return modified;
}

// ***************************************************************************
// CActionSelect
// ***************************************************************************

CActionSelect::CActionSelect (const CDatabaseLocator &locator)
{
	getDocument()->modifyPropertyDlg();
	_Locator = locator;
}

// ***************************************************************************

void CActionSelect::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);
	if (locator.Primitive)
	{
		// Select it
//		nlverify (const_cast<IPrimitive*>(locator.Primitive)->removePropertyByName ("selected"));
		getPrimitiveEditor(locator.Primitive)->setSelected(false);

		// Modify files
		// doc->modifyDatabase (locator.getDatabaseIndex ());

		// Invalidate selection
		InvalidatePrimitive (locator, SelectionState);
		invalidateLeftView ();
	}
}

// ***************************************************************************

bool CActionSelect::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);
	if (locator.Primitive)
	{
		// Select it
		if (!getPrimitiveEditor(locator.Primitive)->getSelected())
//		if (const_cast<IPrimitive*>(locator.Primitive)->addPropertyByName ("selected", new CPropertyString ()))
		{
			getPrimitiveEditor(locator.Primitive)->setSelected(true);
			// Invalidate selection
			InvalidatePrimitive (locator, SelectionState);
			invalidateLeftView ();

			// Modify files
			// doc->modifyDatabase (locator.getDatabaseIndex ());

			// return true;
		}
	}
	return false;
}

// ***************************************************************************
// CActionUnselect
// ***************************************************************************

CActionUnselect::CActionUnselect (const CDatabaseLocator &locator)
{
	getDocument()->modifyPropertyDlg();
	_Locator = locator;
}

// ***************************************************************************

void CActionUnselect::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);
	if (locator.Primitive)
	{
		// Select it
//		nlverify (const_cast<IPrimitive*>(locator.Primitive)->addPropertyByName ("selected", new CPropertyString ()));
		getPrimitiveEditor(locator.Primitive)->setSelected(true);


		// Modify files
		// doc->modifyDatabase (locator.getDatabaseIndex ());

		// Invalidate selection
		InvalidatePrimitive (locator, SelectionState);
		invalidateLeftView ();
	}
}

// ***************************************************************************

bool CActionUnselect::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);
	if (locator.Primitive)
	{
		// Select it
		if (getPrimitiveEditor(locator.Primitive)->getSelected())
//		if (const_cast<IPrimitive*>(locator.Primitive)->removePropertyByName ("selected"))
		{
			getPrimitiveEditor(locator.Primitive)->setSelected(false);
			// Invalidate selection
			InvalidatePrimitive (locator, SelectionState);
			invalidateLeftView ();

			// Modify files
			// doc->modifyDatabase (locator.getDatabaseIndex ());

			return true;
		}
	}
	return false;
}

// ***************************************************************************

// ***************************************************************************
// CActionUnselectAllSub
// ***************************************************************************

CActionUnselectAllSub::CActionUnselectAllSub ()
{
	CWorldEditorDoc *doc = getDocument ();
	std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
	while (ite != Selection.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);

		// Primitive ?
		if (locator.Primitive)
		{
			// Selected ?
//			IProperty *prop;
			if (getPrimitiveEditor(locator.Primitive)->getSelected())
//			if (locator.Primitive->getPropertyByName ("selected", prop))
			{
				// Scan its subvertex
				uint numVert = locator.Primitive->getNumVector ();
				if (numVert)
				{
					const CPrimVector *vectors = locator.Primitive->getPrimVector ();
					for (uint vert=0; vert<numVert; vert++)
					{
						// Selected ?
						if (vectors[vert].Selected)
						{
							CDatabaseLocator subLocator = locator;
							subLocator.XSubPrim = vert;

							// Add this locator
							_SelectedPrimitives.push_back (subLocator);
						}
					}
				}
			}
		}

		ite++;
	}
}

// ***************************************************************************

void CActionUnselectAllSub::undo ()
{
	bool modified = false;
	CWorldEditorDoc *doc = getDocument ();
	for (uint i=0; i<_SelectedPrimitives.size (); i++)
	{
		// Get the locator pointer
		CDatabaseLocator &subLocator = _SelectedPrimitives[i];
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, subLocator);

		// Select its subvertex
		nlassert ((uint)subLocator.XSubPrim < locator.Primitive->getNumVector ());
		CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
		if (vectors[subLocator.XSubPrim].Selected == false)
		{
			vectors[subLocator.XSubPrim].Selected = true;
			modified = true;

			// Modify files
			// doc->modifyDatabase (locator.getDatabaseIndex ());
		}
	}

	// Invalidate selection
	if (modified)
		invalidateLeftView ();
}

// ***************************************************************************

bool CActionUnselectAllSub::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	bool modified = false;
	std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
	while (ite != Selection.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);

		// Primitive ?
		if (locator.Primitive)
		{
			// Selected ?
//			IProperty *prop;
			if (getPrimitiveEditor(locator.Primitive)->getSelected())
//			if (locator.Primitive->getPropertyByName ("selected", prop))
			{
				// Scan its subvertex
				uint numVert = locator.Primitive->getNumVector ();
				if (numVert)
				{
					CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
					for (uint vert=0; vert<numVert; vert++)
					{
						// Selected ?
						if (vectors[vert].Selected)
						{
							vectors[vert].Selected = false;
							modified = true;

							// Modify files
							// doc->modifyDatabase (locator.getDatabaseIndex ());
						}
					}
				}
			}
		}

		ite++;
	}

	// Invalidate selection
	if (modified)
		invalidateLeftView ();

	return modified;
}

// ***************************************************************************
// CActionSelectAllSub
// ***************************************************************************

CActionSelectAllSub::CActionSelectAllSub ()
{
	CWorldEditorDoc *doc = getDocument ();
	std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
	while (ite != Selection.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);

		// Primitive ?
		if (locator.Primitive)
		{
			// Selected ?
//			IProperty *prop;
			if (getPrimitiveEditor(locator.Primitive)->getSelected())
//			if (locator.Primitive->getPropertyByName ("selected", prop))
			{
				// Scan its subvertex
				uint numVert = locator.Primitive->getNumVector ();
				if (numVert)
				{
					const CPrimVector *vectors = locator.Primitive->getPrimVector ();
					for (uint vert=0; vert<numVert; vert++)
					{
						// Selected ?
						if (!vectors[vert].Selected)
						{
							CDatabaseLocator subLocator = locator;
							subLocator.XSubPrim = vert;

							// Add this locator
							_SelectedPrimitives.push_back (subLocator);
						}
					}
				}
			}
		}

		ite++;
	}
}

// ***************************************************************************

void CActionSelectAllSub::undo ()
{
	bool modified = false;
	CWorldEditorDoc *doc = getDocument ();
	for (uint i=0; i<_SelectedPrimitives.size (); i++)
	{
		// Get the locator pointer
		CDatabaseLocator &subLocator = _SelectedPrimitives[i];
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, subLocator);

		// Select its subvertex
		nlassert ((uint)subLocator.XSubPrim < locator.Primitive->getNumVector ());
		CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
		if (vectors[subLocator.XSubPrim].Selected)
		{
			vectors[subLocator.XSubPrim].Selected = false;
			modified = true;

			// Modify files
			// doc->modifyDatabase (locator.getDatabaseIndex ());
		}
	}

	// Invalidate selection
	if (modified)
		invalidateLeftView ();
}

// ***************************************************************************

bool CActionSelectAllSub::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	bool modified = false;
	std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
	while (ite != Selection.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);

		// Primitive ?
		if (locator.Primitive)
		{
			// Selected ?
//			IProperty *prop;
			if (getPrimitiveEditor(locator.Primitive)->getSelected())
//			if (locator.Primitive->getPropertyByName ("selected", prop))
			{
				// Scan its subvertex
				uint numVert = locator.Primitive->getNumVector ();
				if (numVert)
				{
					CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
					for (uint vert=0; vert<numVert; vert++)
					{
						// Selected ?
						if (!vectors[vert].Selected)
						{
							vectors[vert].Selected = true;
							modified = true;

							// Modify files
							// doc->modifyDatabase (locator.getDatabaseIndex ());
						}
					}
				}
			}
		}

		ite++;
	}

	// Invalidate selection
	if (modified)
		invalidateLeftView ();

	return modified;
}

// ***************************************************************************
// CActionSelectSub
// ***************************************************************************

CActionSelectSub::CActionSelectSub (const CDatabaseLocator &locator)
{
	_Locator = locator;
}

// ***************************************************************************

void CActionSelectSub::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);
	if (locator.Primitive)
	{
		// Select its subvertex
		nlassert ((uint)_Locator.XSubPrim < locator.Primitive->getNumVector ());
		CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
		nlassert (vectors[_Locator.XSubPrim].Selected);
		vectors[_Locator.XSubPrim].Selected = false;

		// Modify files
		// doc->modifyDatabase (locator.getDatabaseIndex ());

		// Invalidate selection
		invalidateLeftView ();
	}
}

// ***************************************************************************

bool CActionSelectSub::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);
	if (locator.Primitive)
	{
		// Select its subvertex
		nlassert ((uint)_Locator.XSubPrim < locator.Primitive->getNumVector ());
		CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
		if (!vectors[_Locator.XSubPrim].Selected)
		{
			vectors[_Locator.XSubPrim].Selected = true;

			// Modify files
			// doc->modifyDatabase (locator.getDatabaseIndex ());

			// Invalidate selection
			invalidateLeftView ();
			return true;
		}
	}
	return false;
}

// ***************************************************************************
// CActionUnselectSub
// ***************************************************************************

CActionUnselectSub::CActionUnselectSub (const CDatabaseLocator &locator)
{
	_Locator = locator;
}

// ***************************************************************************

void CActionUnselectSub::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);
	if (locator.Primitive)
	{
		// Select its subvertex
		nlassert ((uint)_Locator.XSubPrim < locator.Primitive->getNumVector ());
		CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
		nlassert (!vectors[_Locator.XSubPrim].Selected);
		vectors[_Locator.XSubPrim].Selected = true;

		// Modify files
		// doc->modifyDatabase (locator.getDatabaseIndex ());

		// Invalidate selection
		invalidateLeftView ();
	}
}

// ***************************************************************************

bool CActionUnselectSub::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);
	if (locator.Primitive)
	{
		// Select its subvertex
		nlassert ((uint)_Locator.XSubPrim < locator.Primitive->getNumVector ());
		CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
		if (vectors[_Locator.XSubPrim].Selected)
		{
			vectors[_Locator.XSubPrim].Selected = false;

			// Modify files
			// doc->modifyDatabase (locator.getDatabaseIndex ());

			// Invalidate selection
			invalidateLeftView ();
			return true;
		}
	}
	return false;
}

// ***************************************************************************
// CActionMove
// ***************************************************************************

CActionMove::CActionMove (bool subSelection)
{
	CWorldEditorDoc *doc = getDocument ();
	// Sub selection ?
	_SubSelection = subSelection;

	std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
	while (ite != Selection.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);

		// Primitive ?
		if (locator.Primitive)
		{
			// Shown ?
//			IProperty *prop;
			if (isPrimitiveVisible (locator.Primitive))
//			if (!locator.Primitive->getPropertyByName ("hidden", prop))
			{
				// Scan its subvertex
				uint numVert = locator.Primitive->getNumVector ();
				if (numVert)
				{
					CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
					for (uint vert=0; vert<numVert; vert++)
					{
						// Keep it ?
						if (!_SubSelection || vectors[vert].Selected)
						{
							// Save this entry
							_Entities.push_back (CUndoEntry (locator, vectors[vert]));

							// Set the sub index
							_Entities.back ().Locator.XSubPrim = vert;
						}
					}
				}
			}
		}

		ite++;
	}
}

// ***************************************************************************

void CActionMove::setTranslation (const NLMISC::CVector &translation)
{
	_Translation = translation;
}

// ***************************************************************************

void CActionMove::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	for (uint i=0; i<_Entities.size (); i++)
	{
		// Get the locator pointer
		CUndoEntry &entry = _Entities[i];
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, entry.Locator);

		// Select its subvertex
		CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
		vectors[entry.Locator.XSubPrim].CVector::operator= (entry.OldPosition);

		// Modify files
		doc->modifyDatabase (locator.getDatabaseIndex ());

		// Invalidate quad grid
		InvalidatePrimitive (locator, QuadTree);
	}

	// Invalidate selection
	invalidateLeftView ();
}

// ***************************************************************************

bool CActionMove::redo ()
{
	if (_Translation.isNull ())
		return false;

	CWorldEditorDoc *doc = getDocument ();
	for (uint i=0; i<_Entities.size (); i++)
	{
		// Get the locator pointer
		CUndoEntry &entry = _Entities[i];
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, entry.Locator);

		// Select its subvertex
		CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
		vectors[entry.Locator.XSubPrim].CVector::operator= (entry.OldPosition);
		vectors[entry.Locator.XSubPrim].CVector::operator+= (_Translation);

		// Modify files
		doc->modifyDatabase (locator.getDatabaseIndex ());

		// Invalidate quad grid
		InvalidatePrimitive (locator, QuadTree);
	}

	// Invalidate selection
	invalidateLeftView ();

	return true;
}

// ***************************************************************************

bool CActionMove::getText (string &result)
{
	result = toString ("Move %.2f, %.2f", _Translation.x, _Translation.y);
	return true;
}

// ***************************************************************************
// CActionRotate
// ***************************************************************************

CActionRotate::CActionRotate (bool subSelection, const CVector &pivot)
{
	CWorldEditorDoc *doc = getDocument ();
	// Sub selection ?
	_SubSelection = subSelection;

	// Pivot ?
	Pivot = pivot;

	std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
	while (ite != Selection.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);

		// Primitive ?
		if (locator.Primitive)
		{
			// Shown ?
//			IProperty *prop;
			if (isPrimitiveVisible (locator.Primitive))
//			if (!locator.Primitive->getPropertyByName ("hidden", prop))
			{
				// Scan its subvertex
				uint numVert = locator.Primitive->getNumVector ();
				CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
				const CPrimPointEditor *pointPrim = dynamic_cast<const CPrimPointEditor*> (locator.Primitive);
				for (uint vert=0; vert<numVert; vert++)
				{
					// Keep it ?
					if (!_SubSelection || vectors[vert].Selected)
					{
						// Save this entry
						_Entities.push_back (CUndoEntry (locator, vectors[vert], pointPrim!=NULL, pointPrim?pointPrim->Angle:0));

						// Set the sub index
						_Entities.back ().Locator.XSubPrim = vert;
					}
				}
			}
		}

		ite++;
	}
	_Rotate = false;
	_Turn = false;
}

// ***************************************************************************

void CActionRotate::setAngle (float angle, bool rotate, bool turn)
{
	_Angle = angle;
	_Rotate = rotate;
	_Turn = turn;
}

// ***************************************************************************

void CActionRotate::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	for (uint i=0; i<_Entities.size (); i++)
	{
		// Get the locator pointer
		CUndoEntry &entry = _Entities[i];
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, entry.Locator);

		// Select its subvertex
		CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
		vectors[entry.Locator.XSubPrim].CVector::operator= (entry.OldPosition);

		// Restaure the turn
		if (entry.PointPrimitive)
		{
			CPrimPointEditor *primPoint = safe_cast<CPrimPointEditor*> (const_cast<IPrimitive*> (locator.Primitive));
			primPoint->Angle = entry.PointPrimitiveAngle;
		}

		// Modify files
		doc->modifyDatabase (locator.getDatabaseIndex ());

		// Invalidate quad grid
		InvalidatePrimitive (locator, QuadTree);
	}

	// Invalidate selection
	invalidateLeftView ();
}

// ***************************************************************************

bool CActionRotate::redo ()
{
	if (_Angle == 0 || (!_Turn && !_Rotate))
		return false;

	CWorldEditorDoc *doc = getDocument ();
	for (uint i=0; i<_Entities.size (); i++)
	{
		// Get the locator pointer
		CUndoEntry &entry = _Entities[i];
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, entry.Locator);

		// Select its subvertex
		bool modified = false;
		if (_Rotate)
		{
			// Select its subvertex
			CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
			vectors[entry.Locator.XSubPrim].CVector::operator= (entry.OldPosition);

			// Transform point
			transformVector (vectors[entry.Locator.XSubPrim], _Angle, Pivot);
			modified = true;
		}

		// Transform point
		if (_Turn && entry.PointPrimitive)
		{
			// Select its subvertex
			CPrimPointEditor *primPoint = safe_cast<CPrimPointEditor*> (const_cast<IPrimitive*> (locator.Primitive));
			primPoint->Angle = entry.PointPrimitiveAngle;
			primPoint->Angle += _Angle;
			modified = true;
		}

		if (modified)
		{
			// Modify files
			doc->modifyDatabase (locator.getDatabaseIndex ());

			// Invalidate quad grid
			InvalidatePrimitive (locator, QuadTree);
		}
	}

	// Invalidate selection
	invalidateLeftView ();

	return true;
}

// ***************************************************************************

bool CActionRotate::getText (string &result)
{
	result = toString ("Rotate %.2f degrees", 180*_Angle/Pi);
	return true;
}

// ***************************************************************************

bool CActionRotate::getHelp (string &result)
{
	result = "Turn with CTRL";
	return _Rotate;
}

// ***************************************************************************
// CActionScale
// ***************************************************************************

CActionScale::CActionScale (bool subSelection, const CVector &pivot)
{
	CWorldEditorDoc *doc = getDocument ();
	// Sub selection ?
	_SubSelection = subSelection;

	// Pivot
	Pivot = pivot;

	std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
	while (ite != Selection.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);

		// Primitive ?
		if (locator.Primitive)
		{
			// Shown ?
//			IProperty *prop;
			if (isPrimitiveVisible (locator.Primitive))
//			if (!locator.Primitive->getPropertyByName ("hidden", prop))
			{
				// Scan its subvertex
				uint numVert = locator.Primitive->getNumVector ();
				CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());

				// Radius
				string radius;
				const bool hasRadius = locator.Primitive->getPropertyByName ("radius", radius);

				for (uint vert=0; vert<numVert; vert++)
				{
					// Keep it ?
					if (!_SubSelection || vectors[vert].Selected)
					{
						// Save this entry
						_Entities.push_back (CUndoEntry (locator, vectors[vert], hasRadius, hasRadius?(float)atof(radius.c_str()):0));

						// Set the sub index
						_Entities.back ().Locator.XSubPrim = vert;
					}
				}
			}
		}

		ite++;
	}
	_Scale = false;
	_Radius = false;
}

// ***************************************************************************

void CActionScale::setScale (float factor, bool scale, bool radius)
{
	_Factor = factor;
	_Scale = scale;
	_Radius = radius;
}

// ***************************************************************************

void CActionScale::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	for (uint i=0; i<_Entities.size (); i++)
	{
		// Get the locator pointer
		CUndoEntry &entry = _Entities[i];
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, entry.Locator);

		// Select its subvertex
		CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
		vectors[entry.Locator.XSubPrim].CVector::operator= (entry.OldPosition);

		// Does it have radius ?
		if (entry.HasRadius)
		{
			const_cast<IPrimitive*> (locator.Primitive)->removePropertyByName ("radius");
			const_cast<IPrimitive*> (locator.Primitive)->addPropertyByName ("radius", new CPropertyString (toString (entry.Radius).c_str()));
		}

		// Modify files
		doc->modifyDatabase (locator.getDatabaseIndex ());

		// Invalidate quad grid
		InvalidatePrimitive (locator, QuadTree);
	}

	// Invalidate selection
	invalidateLeftView ();
}

// ***************************************************************************

bool CActionScale::redo ()
{
	if ((_Factor == 1.f) || (!_Scale && !_Radius))
		return false;

	CWorldEditorDoc *doc = getDocument ();
	for (uint i=0; i<_Entities.size (); i++)
	{
		// Get the locator pointer
		CUndoEntry &entry = _Entities[i];
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, entry.Locator);

		bool modified = false;

		if (_Scale)
		{
			// Select its subvertex
			CPrimVector *vectors = const_cast<CPrimVector*> (locator.Primitive->getPrimVector ());
			vectors[entry.Locator.XSubPrim].CVector::operator= (entry.OldPosition);
			vectors[entry.Locator.XSubPrim].CVector::operator-= (Pivot);
			vectors[entry.Locator.XSubPrim].CVector::operator*= (_Factor);
			vectors[entry.Locator.XSubPrim].CVector::operator+= (Pivot);
			modified = true;
		}

		if (_Radius)
		{
			const_cast<IPrimitive*> (locator.Primitive)->removePropertyByName ("radius");
			const_cast<IPrimitive*> (locator.Primitive)->addPropertyByName ("radius", new CPropertyString (toString (entry.Radius*_Factor).c_str()));
			modified = true;
		}

		if (modified)
		{
			// Modify files
			doc->modifyDatabase (locator.getDatabaseIndex ());

			// Invalidate quad grid
			InvalidatePrimitive (locator, QuadTree);
		}
	}

	// Invalidate selection
	invalidateLeftView ();

	return true;
}

// ***************************************************************************

bool CActionScale::getText (string &result)
{
	result = toString ("Scale %.2f%%", _Factor*100);
	return true;
}

// ***************************************************************************

bool CActionScale::getHelp (string &result)
{
	result = "Radius with CTRL";
	return _Scale;
}

// ***************************************************************************
// CActionAddVertex
// ***************************************************************************

CActionAddVertex::CActionAddVertex (const CDatabaseLocator &locator, const CVector &newVertex)
{
	// Save the locator
	_Locator = locator;
	_NewVertex = newVertex;
}

// ***************************************************************************

void CActionAddVertex::undo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Is a path or a zone ?
	std::vector<CPrimVector> *primVector = NULL;
	CPrimPathEditor *path = dynamic_cast<CPrimPathEditor*> (const_cast<IPrimitive*> (locator.Primitive));
	if (path)
		primVector = &path->VPoints;
	else
	{
		CPrimZoneEditor *zone = dynamic_cast<CPrimZoneEditor*> (const_cast<IPrimitive*> (locator.Primitive));
		if (zone)
			primVector = &zone->VPoints;
	}

	// Found ?
	nlverify (primVector);

	// Delete the vertex
	primVector->erase (primVector->begin ()+(_Locator.XSubPrim+1));

	// Modify files
	doc->modifyDatabase (locator.getDatabaseIndex ());

	// Invalidate quad grid
	InvalidatePrimitive (locator, QuadTree);
}

// ***************************************************************************

bool CActionAddVertex::redo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Is a path or a zone ?
	std::vector<CPrimVector> *primVector = NULL;
	CPrimPathEditor *path = dynamic_cast<CPrimPathEditor*> (const_cast<IPrimitive*> (locator.Primitive));
	if (path)
		primVector = &path->VPoints;
	else
	{
		CPrimZoneEditor *zone = dynamic_cast<CPrimZoneEditor*> (const_cast<IPrimitive*> (locator.Primitive));
		if (zone)
			primVector = &zone->VPoints;
	}

	// Found ?
	nlverify (primVector);

	// Insert a vertex
	primVector->insert (primVector->begin ()+(_Locator.XSubPrim+1), CPrimVector());

	// Initialise it
	(*primVector)[_Locator.XSubPrim+1].CVector::operator= (_NewVertex);

	// Not selected
	(*primVector)[_Locator.XSubPrim+1].Selected = false;

	// Modify files
	doc->modifyDatabase (locator.getDatabaseIndex ());

	// Invalidate quad grid
	InvalidatePrimitive (locator, QuadTree);

	return true;
}

// ***************************************************************************
// CActionDeleteSub
// ***************************************************************************

CActionDeleteSub::CActionDeleteSub (const CDatabaseLocatorPointer &locator)
{
	// Save the locator
	_Locator = locator;

	// Is a path or a zone ?
	std::vector<CPrimVector> *primVector = NULL;
	CPrimPathEditor *path = dynamic_cast<CPrimPathEditor*> (const_cast<IPrimitive*> (locator.Primitive));
	if (path)
		primVector = &path->VPoints;
	else
	{
		CPrimZoneEditor *zone = dynamic_cast<CPrimZoneEditor*> (const_cast<IPrimitive*> (locator.Primitive));
		if (zone)
			primVector = &zone->VPoints;
	}

	// Found ?
	nlverify (primVector);

	// Delete the vertex
	_OldVertex = (*primVector)[_Locator.XSubPrim];
}

// ***************************************************************************

void CActionDeleteSub::undo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Is a path or a zone ?
	std::vector<CPrimVector> *primVector = NULL;
	CPrimPathEditor *path = dynamic_cast<CPrimPathEditor*> (const_cast<IPrimitive*> (locator.Primitive));
	if (path)
		primVector = &path->VPoints;
	else
	{
		CPrimZoneEditor *zone = dynamic_cast<CPrimZoneEditor*> (const_cast<IPrimitive*> (locator.Primitive));
		if (zone)
			primVector = &zone->VPoints;
	}

	// Found ?
	nlverify (primVector);

	// Insert a vertex
	primVector->insert (primVector->begin ()+_Locator.XSubPrim, CPrimVector());

	// Initialise it
	(*primVector)[_Locator.XSubPrim] = (_OldVertex);

	// Modify files
	doc->modifyDatabase (locator.getDatabaseIndex ());

	// Invalidate left view
	invalidateLeftView ();

	// Invalidate quad grid
	InvalidatePrimitive (locator, QuadTree);
}

// ***************************************************************************

bool CActionDeleteSub::redo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Is a path or a zone ?
	std::vector<CPrimVector> *primVector = NULL;
	CPrimPathEditor *path = dynamic_cast<CPrimPathEditor*> (const_cast<IPrimitive*> (locator.Primitive));
	if (path)
		primVector = &path->VPoints;
	else
	{
		CPrimZoneEditor *zone = dynamic_cast<CPrimZoneEditor*> (const_cast<IPrimitive*> (locator.Primitive));
		if (zone)
			primVector = &zone->VPoints;
	}

	// Found ?
	nlverify (primVector);

	// Delete the vertex
	primVector->erase (primVector->begin ()+_Locator.XSubPrim);

	// Modify files
	doc->modifyDatabase (locator.getDatabaseIndex ());

	// Invalidate left view
	invalidateLeftView ();

	// Invalidate quad grid
	InvalidatePrimitive (locator, QuadTree);

	return true;
}

// ***************************************************************************
// CActionDelete
// ***************************************************************************

CActionDelete::CActionDelete (const CDatabaseLocatorPointer &locator)
{
	// Save the locator
	_Locator = locator;

	// Copy the primitive
	_OldPrimitive = locator.Primitive->copy ();
}

// ***************************************************************************

CActionDelete::~CActionDelete ()
{
	delete _OldPrimitive;
}

// ***************************************************************************

void CActionDelete::undo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Copy the primtive
	IPrimitive *copy = _OldPrimitive->copy ();

	// Insert it	
	doc->insertPrimitive (_Locator, copy);

	// Modify files
	doc->modifyDatabase (_Locator.getDatabaseIndex ());

	// Invalidate left view
	invalidateLeftView ();
	getMainFrame ()->invalidateTools ();
	
	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Invalidate quad grid
	InvalidatePrimitiveRec (locator, QuadTree|LogicTreeStruct|SelectionState);
}

// ***************************************************************************

bool CActionDelete::redo ()
{
	CWorldEditorDoc *doc = getDocument ();

	doc->deletePrimitive (_Locator);

	// Modify files
	doc->modifyDatabase (_Locator.getDatabaseIndex ());

	// Invalidate left view
	invalidateLeftView ();
	getMainFrame ()->invalidateTools ();

	// Invalidate quad grid
	// Will be removed from the quad grid by the IPrimitiveEditor destructor

	return true;
}

// ***************************************************************************
// CActionSetPrimitivePropertyString
// ***************************************************************************

CActionSetPrimitivePropertyString::CActionSetPrimitivePropertyString (const CDatabaseLocatorPointer &locator, const char *propertyName, const char *newValue, bool _default)
{
	_PropertyName = propertyName;
	_PropertyNewValue = newValue;
	_PropertyNewValueDefault = _default;
	_Locator = locator;

	// Backup old value
	const IProperty *prop;
	_OldExist = locator.Primitive->getPropertyByName (propertyName, prop);

	if (_OldExist)
	{
		// Dynamic cast
		const CPropertyString *propString = dynamic_cast<const CPropertyString *> (prop);
		nlassert (propString);
		_PropertyOldValue = propString->String;
		_PropertyOldValueDefault = propString->Default;
	}
}

// ***************************************************************************

void CActionSetPrimitivePropertyString::undo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Remove the old property
	const_cast<IPrimitive*> (locator.Primitive)->removePropertyByName (_PropertyName.c_str ());

	// Add the old property
	if (_OldExist)
		const_cast<IPrimitive*> (locator.Primitive)->addPropertyByName (_PropertyName.c_str (), new CPropertyString (_PropertyOldValue.c_str (), _PropertyOldValueDefault));

	// Invalidate tools
	getMainFrame ()->invalidateToolsParam ();
	InvalidatePrimitive (locator, LogicTreeParam);

	// Modify files
	doc->modifyDatabase (locator.getDatabaseIndex ());

	// Invalidate right view
	invalidateLeftView ();
}

// ***************************************************************************

bool CActionSetPrimitivePropertyString::redo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Remove the old property
	const_cast<IPrimitive*> (locator.Primitive)->removePropertyByName (_PropertyName.c_str ());

	// Add the new property
	const_cast<IPrimitive*> (locator.Primitive)->addPropertyByName (_PropertyName.c_str (), new CPropertyString (_PropertyNewValue.c_str (), _PropertyNewValueDefault));

	// Invalidate tools
	getMainFrame ()->invalidateToolsParam ();
	InvalidatePrimitive (locator, LogicTreeParam);

	// Modify files
	doc->modifyDatabase (locator.getDatabaseIndex ());

	// Invalidate right view
	invalidateLeftView ();

	return true;
}

// ***************************************************************************
// CActionSetPrimitivePropertyStringArray
// ***************************************************************************

CActionSetPrimitivePropertyStringArray::CActionSetPrimitivePropertyStringArray (const CDatabaseLocatorPointer &locator, const char *propertyName, const std::vector<std::string> &newValue, bool _default)
{
	_PropertyName = propertyName;
	_PropertyNewValue = newValue;
	_Locator = locator;
	_PropertyNewValueDefault = _default;

	// Backup old value
	const IProperty *prop;
	_OldExist = locator.Primitive->getPropertyByName (propertyName, prop);

	if (_OldExist)
	{
		// Dynamic cast
		const CPropertyStringArray *propStringArray = dynamic_cast<const CPropertyStringArray *> (prop);
		nlassert (propStringArray);
		_PropertyOldValue = propStringArray->StringArray;
		_PropertyOldValueDefault = propStringArray->Default;
	}
}

// ***************************************************************************

void CActionSetPrimitivePropertyStringArray::undo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Remove the old property
	const_cast<IPrimitive*> (locator.Primitive)->removePropertyByName (_PropertyName.c_str ());

	// Add the old property
	if (_OldExist)
		const_cast<IPrimitive*> (locator.Primitive)->addPropertyByName (_PropertyName.c_str (), new CPropertyStringArray (_PropertyOldValue, _PropertyOldValueDefault));

	// Invalidate tools
	getMainFrame ()->invalidateToolsParam ();

	// Modify files
	doc->modifyDatabase (locator.getDatabaseIndex ());
}

// ***************************************************************************

bool CActionSetPrimitivePropertyStringArray::redo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Remove the old property
	const_cast<IPrimitive*> (locator.Primitive)->removePropertyByName (_PropertyName.c_str ());

	// Add the new property
	const_cast<IPrimitive*> (locator.Primitive)->addPropertyByName (_PropertyName.c_str (), new CPropertyStringArray (_PropertyNewValue, _PropertyNewValueDefault));

	// Invalidate tools
	getMainFrame ()->invalidateToolsParam ();

	// Modify files
	doc->modifyDatabase (locator.getDatabaseIndex ());

	return true;
}

// ***************************************************************************
// CActionAddPrimitiveByClass
// ***************************************************************************

CActionAddPrimitiveByClass::CActionAddPrimitiveByClass (const CDatabaseLocator &locator, const char *className, 
														const NLMISC::CVector &initPos, float deltaPos,
														const std::vector<CPrimitiveClass::CInitParameters>	initParameters)
{
	_Locator = locator;
	_ClassName = className;
	_OldPrimitive = NULL;
	_InitPos = initPos;
	_DeltaPos = deltaPos;
	_InitParameters = initParameters;
}

// ***************************************************************************

void CActionAddPrimitiveByClass::undo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Delete the primitive
	doc->deletePrimitive (_Locator);

	// Invalidate left view
	invalidateLeftView ();
	getMainFrame ()->invalidateTools ();

	// Modify files
	doc->modifyDatabase (_Locator.getDatabaseIndex ());

	// Invalidate quad grid
	// Will be removed from the quad grid by the IPrimitiveEditor destructor
}

// ***************************************************************************

bool CActionAddPrimitiveByClass::redo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// First time ?
	if (_OldPrimitive == NULL)
	{

		// Get the primitive
		CDatabaseLocator _parent = _Locator;
		_parent.getParent ();
		CDatabaseLocatorPointer parent;
		doc->getLocator (parent, _parent);

		// Get the class primitive
		IPrimitive *primitive = const_cast<IPrimitive*> (doc->createPrimitive (_Locator, _ClassName.c_str (), "", _InitPos, _DeltaPos, _InitParameters));
		// IPrimitive *primitive = const_cast<IPrimitive*> (doc->createPrimitive (_Locator, _ClassName.c_str (), (_ClassName+"_"+toString (parent.Primitive->getNumChildren ())).c_str (), _InitPos, _DeltaPos, _InitParameters));
		if (primitive != NULL)
		{
			// Invalidate left view
			invalidateLeftView ();
			getMainFrame ()->invalidateTools ();

			// Get the primitive
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, _Locator);

			// Modify files
			doc->modifyDatabase (_Locator.getDatabaseIndex ());

			// Final name
			string name;
			if (!primitive->getPropertyByName ("name", name) || name.empty())
			{
				string newValue = _ClassName+"_"+toString (parent.Primitive->getNumChildren ());
				if (!newValue.empty())
					doc->addModification (new CActionSetPrimitivePropertyString (locator, "name", newValue.c_str (), false));
			}

			// Backup it
			_OldPrimitive = locator.Primitive->copy ();

			// Invalidate quad grid
			InvalidatePrimitiveRec (locator, QuadTree|LogicTreeStruct|SelectionState);
			return true;
		}
	}
	else
	{
		// Make a copy
		IPrimitive *copy = _OldPrimitive->copy ();
		// Second time, restaure backuped primitive
		doc->insertPrimitive (_Locator, copy);

		// Modify files
		doc->modifyDatabase (_Locator.getDatabaseIndex ());

		// Invalidate left view
		invalidateLeftView ();
		getMainFrame ()->invalidateTools ();

		// Get the primitive
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, _Locator);

		// Invalidate quad grid
		InvalidatePrimitiveRec (locator, QuadTree|LogicTreeStruct|SelectionState);

		return true;
	}

	return false;
}

// ***************************************************************************
// CActionAddLandscape
// ***************************************************************************

CActionAddLandscape::CActionAddLandscape (const char *filename)
{
	_FirstTime = true;
	_Filename = filename;
}

// ***************************************************************************

void CActionAddLandscape::undo ()
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Remove last landscape
	doc->_DataHierarchy.resize (doc->_DataHierarchy.size ()-1);
	doc->_DataHierarchy.recomputePointerArray ();

	// Modify files
	doc->modifyProject ();

	// Invalidate all
	invalidateLeftView ();
	getMainFrame ()->invalidateLandscape ();
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();
}

// ***************************************************************************

bool CActionAddLandscape::redo ()
{
	// Disable events
	CNoInteraction nointeraction;
	
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Result
	bool result = true;

	if (_FirstTime)
	{
		// Load the file
		getMainFrame ()->launchLoadingDialog (string("loading land ") + _Filename);

		try
		{
			// Open it
			CIFile fileIn;
			if (fileIn.open (_Filename))
			{
				CIXml xml(true);
				xml.init (fileIn);
				_NewRegion.serial (xml);
			}
			else
			{
				getMainFrame ()->terminateLoadingDialog ();
				theApp.errorMessage ("Can't open file %s for reading", _Filename.c_str ());
				result = false;
			}
		}
		catch (Exception& e)
		{
			getMainFrame ()->terminateLoadingDialog ();
			theApp.errorMessage ("Error reading file %s : %s", _Filename.c_str (), e.what ());
			result = false;
		}

		// Done
		getMainFrame ()->terminateLoadingDialog ();
		_FirstTime = false;
	}

	if (result)
	{
		// Push the landscape
		doc->_DataHierarchy.push_back (CWorldEditorDoc::CDatabaseElement (CWorldEditorDoc::CDatabaseElement::Landscape));
		doc->_DataHierarchy.recomputePointerArray ();
		doc->_DataHierarchy.back ().ZoneRegion = _NewRegion;
		doc->_DataHierarchy.back ().Filename = _Filename;
		doc->_DataHierarchy.back ().LastModifedTime = NLMISC::CFile::getFileModificationDate (_Filename);

		// Init the landscape manager
		getMainFrame ()->invalidateLandscape ();

		// Invalidate pointers
		InvalidateAllPrimitives ();

		// Modify files
		doc->modifyProject ();

		// Invalidate all
		invalidateLeftView ();
		getMainFrame ()->invalidateTools ();
	}

	return result;
}

// ***************************************************************************
// CActionNewPrimitive
// ***************************************************************************

CActionNewLandscape::CActionNewLandscape ()
{
}

// ***************************************************************************

void CActionNewLandscape::undo ()
{
	CWorldEditorDoc *doc = getDocument ();
	// Check we have a primitive
	nlassert (doc->_DataHierarchy.size () > 0);

	// Remove back primitive
	doc->_DataHierarchy.resize (doc->_DataHierarchy.size ()-1);
	doc->_DataHierarchy.recomputePointerArray ();

	// Invalidate tools
	getMainFrame ()->invalidateLandscape ();
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();

	// Modify files
	doc->modifyProject ();
}

// ***************************************************************************

bool CActionNewLandscape::redo ()
{
	CWorldEditorDoc *doc = getDocument ();
	// Push back the primitive
	doc->_DataHierarchy.push_back (CWorldEditorDoc::CDatabaseElement (CWorldEditorDoc::CDatabaseElement::Landscape));
	doc->_DataHierarchy.recomputePointerArray ();

	// Invalidate tools
	getMainFrame ()->invalidateLandscape ();
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();

	// Modify files
	uint index = doc->_DataHierarchy.size ()-1;
	doc->modifyProject ();
	doc->modifyDatabase (index);

	return true;
}

// ***************************************************************************

CActionDeleteDatabaseElement::CActionDeleteDatabaseElement (uint prim)
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Backup the primitive
	_Index = prim;
	_Primitive = new CWorldEditorDoc::CDatabaseElement (doc->_DataHierarchy[_Index]);
}

// ***************************************************************************

CActionDeleteDatabaseElement::~CActionDeleteDatabaseElement ()
{
	if (_Primitive)
		delete _Primitive;
}

// ***************************************************************************

void CActionDeleteDatabaseElement::undo ()
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Insert the primitive
	std::list<CWorldEditorDoc::CDatabaseElement>::iterator ite = doc->_DataHierarchy.begin();
	for (uint i=0; i<_Index; i++)
		ite++;
	doc->_DataHierarchy.insert (ite, *_Primitive);
	doc->_DataHierarchy.recomputePointerArray ();

	// Is a landscape ?
	if (doc->isLandscape (_Index))
		getMainFrame ()->invalidateLandscape ();

	// Modify files
	doc->modifyProject ();

	// Invalidate all
	invalidateLeftView ();
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();
}

// ***************************************************************************

bool CActionDeleteDatabaseElement::redo ()
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Is a landscape ?
	if (doc->isLandscape (_Index))
		getMainFrame ()->invalidateLandscape ();

	// Delete the primitive
	std::list<CWorldEditorDoc::CDatabaseElement>::iterator ite = doc->_DataHierarchy.begin();
	for (uint i=0; i<_Index; i++)
		ite++;
	doc->_DataHierarchy.erase (ite);
	doc->_DataHierarchy.recomputePointerArray ();

	// Modify files
	doc->modifyProject ();

	// Invalidate all
	invalidateLeftView ();
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();

	return true;
}

// ***************************************************************************
// CActionXChgDatabaseElement
// ***************************************************************************

CActionXChgDatabaseElement::CActionXChgDatabaseElement (uint prim)
{
	// Backup the primitive
	_Index = prim;
}

// ***************************************************************************

CActionXChgDatabaseElement::~CActionXChgDatabaseElement ()
{
}

// ***************************************************************************

void CActionXChgDatabaseElement::undo ()
{
	// Same than redo
	redo ();
}

// ***************************************************************************

bool CActionXChgDatabaseElement::redo ()
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();
	nlassert (_Index+1 < doc->getNumDatabaseElement ());

	// Is a landscape ?
	if (doc->isLandscape (_Index) || doc->isLandscape (_Index+1))
		getMainFrame ()->invalidateLandscape ();

	// Delete the primitive
	uint i;
	std::list<CWorldEditorDoc::CDatabaseElement>::iterator ite0 = doc->_DataHierarchy.begin();
	for (i=0; i<_Index; i++)
		ite0++;
	std::list<CWorldEditorDoc::CDatabaseElement>::iterator ite1 = ite0;
	ite1++;
	doc->_DataHierarchy.splice (ite0, doc->_DataHierarchy, ite1);
	doc->_DataHierarchy.recomputePointerArray ();

	// Modify files
	doc->modifyProject ();

	// Invalidate all
	invalidateLeftView ();
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();

	return true;
}

// ***************************************************************************
// CActionAddPrimitive
// ***************************************************************************

CActionAddPrimitive::CActionAddPrimitive (const NLLIGO::IPrimitive &primitiveToAdd, const CDatabaseLocator &locator)
{
	_Primitive = primitiveToAdd.copy ();
	_Locator = locator;
}

// ***************************************************************************

CActionAddPrimitive::CActionAddPrimitive (NLLIGO::IPrimitive *primitiveToAdd, const CDatabaseLocator &locator)
{
	_Primitive = primitiveToAdd;
	_Locator = locator;
	_FirstTime = true;
}

// ***************************************************************************

CActionAddPrimitive::~CActionAddPrimitive ()
{
	delete _Primitive;
}

// ***************************************************************************

void CActionAddPrimitive::undo ()
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Add the primitive
	doc->deletePrimitive (_Locator);

	// Modify files
	doc->modifyDatabase (_Locator.getDatabaseIndex ());

	// Invalidate all
	invalidateLeftView ();
	getMainFrame ()->invalidateTools ();

	// Invalidate quad grid
	// Will be removed from the quad grid by the IPrimitiveEditor destructor
}

// ***************************************************************************

bool CActionAddPrimitive::redo ()
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Make a copy
	IPrimitive *copy = _Primitive->copy ();

	// Add the primitive
	doc->insertPrimitive (_Locator, copy);

	// Modify files
	doc->modifyDatabase (_Locator.getDatabaseIndex ());

	// Update unique ID
	if (_FirstTime)
		doc->resetUniqueID (*copy);

	// Invalidate all
	invalidateLeftView ();
	getMainFrame ()->invalidateTools ();

	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Invalidate quad grid
	InvalidatePrimitiveRec (locator, QuadTree|LogicTreeStruct|SelectionState);

	_FirstTime = false;

	// Done
	return true;
}


// ***************************************************************************
// CActionShowHide
// ***************************************************************************

CActionShowHide::CActionShowHide (const CDatabaseLocatorPointer &locator, bool show)
{
	_Show = show;
	_Locator = locator;
}

// ***************************************************************************

void CActionShowHide::undo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Show it ?
	if (_Show)
	{
		// Add the property
		getPrimitiveEditor(locator.Primitive)->setHidden(true);

//		nlverify (const_cast<IPrimitive*>(locator.Primitive)->addPropertyByName ("hidden", new CPropertyString ("")));
	}
	else
	{
		// Remove the property
		getPrimitiveEditor(locator.Primitive)->setHidden(false);
//		const IProperty *property;
//		nlassert (locator.Primitive->getPropertyByName ("hidden", property));
//		const_cast<IPrimitive*>(locator.Primitive)->removePropertyByName ("hidden");
	}

	// Modify files
	// doc->modifyDatabase (locator.getDatabaseIndex ());

	// Invalidate all
	invalidateLeftView ();
	getMainFrame ()->invalidateToolsParam ();
	InvalidatePrimitive (locator, LogicTreeParam);
}

// ***************************************************************************

bool CActionShowHide::redo ()
{
	CWorldEditorDoc *doc = getDocument ();

	// Get the primitive
	CDatabaseLocatorPointer locator;
	doc->getLocator (locator, _Locator);

	// Show it ?
	if (_Show)
	{
		// Remove the property
		getPrimitiveEditor(locator.Primitive)->setHidden(false);
//		const IProperty *property;
//		if (!locator.Primitive->getPropertyByName ("hidden", property))
//			// Does not exist
//			return false;
//
//		// Show it
//		nlverify (const_cast<IPrimitive*>(locator.Primitive)->removePropertyByName ("hidden"));
	}
	else
	{
		// Add the property
		getPrimitiveEditor(locator.Primitive)->setHidden(true);
//		const IProperty *property;
//		if (locator.Primitive->getPropertyByName ("hidden", property))
//			// Already exist
//			return false;
//
//		// Add the property
//		nlverify (const_cast<IPrimitive*>(locator.Primitive)->addPropertyByName ("hidden", new CPropertyString ("")));
	}

	// Modify files
	// doc->modifyDatabase (locator.getDatabaseIndex ());

	// Invalidate all
	invalidateLeftView ();
	getMainFrame ()->invalidateToolsParam ();
	InvalidatePrimitive (locator, LogicTreeParam);

	return true;
}

// ***************************************************************************
