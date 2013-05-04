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


#ifndef NL_PLUGIN_INTERFACE_H
#define NL_PLUGIN_INTERFACE_H

#include "nel/misc/vector.h"
#include "nel/ligo/primitive.h"
#include "nel/misc/config_file.h"
#include <nel/ligo/primitive_class.h>
#include "nel/net/callback_client.h"
#include "nel/misc/sheet_id.h"
#include "display.h"

class IPluginCallback;
class IPrimitiveDisplayer;

extern "C"
{
	/// Factory method for plugins.
	//typedef void *(FCreateSoundPlugin)();
	typedef void *(FCreatePlugin)();
}

class CDipslay;

enum TValidationChannel
{
	// Invalidate the primitive because its position has benn modified
	QuadTree = 1,

	// Invalidate the primitive because its structure has changed
	LogicTreeStruct = 2,

	// Invalidate the primitive because its name, visibility, expand state has changed
	LogicTreeParam = 4,

	// Invalidate the selection for this primitive
	_SelectionSelectState = 8,

	// Invalidate the selection for this primitive
	SelectionState = (_SelectionSelectState|LogicTreeParam),
};

/** Interface to access the world editor from the plugin.
 *	This interface is implemented by the world editor and
 *	used by the sound plugin.
 */
class IPluginAccess
{
public:
	/// Retrieve the config file.
	virtual NLMISC::CConfigFile &getConfigFile() = 0;
	
	/// Retrieve the main window handle.
	virtual CWnd		*getMainWindow() =0;

	/// Output error messages
	virtual bool		yesNoMessage (const char *format, ... ) =0;
	virtual	void		errorMessage (const char *format, ... ) =0;
	virtual void		infoMessage (const char *format, ... ) =0;

	/// Add a toolbar in the main frame.
//	virtual void		addToolBar(CToolBar *ptoolBar) =0;

	/// Start the control of a position into the 2D view. Changed are reported throw IPluginCallback::positionMoved
	virtual void		startPositionControl(IPluginCallback *plugin, const NLMISC::CVector &initPos) =0;
	/// Stop the control of a position.
	virtual void		stopPositionControl(IPluginCallback *plugin) =0;

	/* Functions to create/remove the Root Primitive for server actions
		Returns the id of the root primitive
	*/
	virtual NLLIGO::IPrimitive *createRootPluginPrimitive (const char *name) = 0;
	virtual void deleteRootPluginPrimitive (void) = 0;
	/// get all the current root primitve in the world editor. only the editable one are returned
	virtual void getAllRootPluginPrimitive (std::vector<NLLIGO::IPrimitive*> &prims) = 0;
	/*
	 *	The players or information coming from the server are considered as primitives by the WorldEditor
	 *   so we need to create/delete/modify these primitives
	 */
	// Create a plugin primitive
	virtual const NLLIGO::IPrimitive *createPluginPrimitive (
		const char *className, 
		const char *primName, 
		const NLMISC::CVector &initPos, 
		float deltaPos, 
		const std::vector<NLLIGO::CPrimitiveClass::CInitParameters> &initParameters,
		NLLIGO::IPrimitive *parent
		) = 0;

	// delete a plugin primitive
	virtual void deletePluginPrimitive (const NLLIGO::IPrimitive *primitive) = 0;

	// indicates to the WorldEditor that the primitive has changed
	virtual void invalidatePluginPrimitive (const NLLIGO::IPrimitive *primitive, uint channels) = 0;

	// invalidate the left view completely
	virtual void invalidateLeftView() = 0;

	// get the display window coordinates
	virtual void	getWindowCoordinates(NLMISC::CVector &vmin, NLMISC::CVector &vmax) = 0;

	// Get the current selection
	virtual const std::list<NLLIGO::IPrimitive*>	&getCurrentSelection()=0;

	virtual void setCurrentSelection(std::vector<NLLIGO::IPrimitive*>& )=0;

	virtual const NLLIGO::IPrimitive* getRootNode(const std::string&)=0;

	virtual std::string& getRootFileName(NLLIGO::IPrimitive*)=0;

	// Register a primitive displayer
	virtual void registerPrimitiveDisplayer(IPrimitiveDisplayer *displayer, const std::vector<std::string> &primClassNames) =0;	

	// Get current selection of primitives
	virtual const std::list<NLLIGO::IPrimitive*> &getCurrentSelection() const = 0;

	// helper to test if a primitive is selected
	virtual bool isSelected(const NLLIGO::IPrimitive &prim) const = 0;

	// Refresh the current property dialog.
	virtual void refreshPropertyDialog() = 0;

	// Hide / show a primitive
	virtual void setPrimitiveHideFlag(NLLIGO::IPrimitive &prim, bool hidden) = 0;

	// Create a texture object
	virtual CPrimTexture *createTexture() = 0;

	// Delete a texture object
	virtual void deleteTexture(CPrimTexture *tex) = 0;

	/** Tool function : build a NLMISC CBitmap from a tga resource
	  * \return true if build was successful
	  */
	virtual bool buildNLBitmapFromTGARsc(HRSRC bm, HMODULE hm, NLMISC::CBitmap &dest) = 0;
		
};
/** Interface for plugin callback.
 *	This interface must be implemented by the plugin to receive
 *	notification about the edited document.
 *	\author Boris Boucher
*	\author Nevrax
 */
class IPluginCallback
{
public:
	/// Init the plugin. The plugin receive the world editor interface.
	virtual void		init(IPluginAccess *pluginAccess) =0;
	/// Delete the plugin.
	virtual ~IPluginCallback() {}

	/// The current region has changed.
//	virtual void		primRegionChanged(const std::vector<NLLIGO::CPrimRegion*> &regions) = 0;
	virtual void		primitiveChanged(const NLLIGO::IPrimitive *root) = 0;
	/// The region has been modifed.
//	virtual void		primRegionModifed() = 0;


	/// The position has been moved on the map.
	virtual void		positionMoved(const NLMISC::CVector &position) = 0;
	/// The plugin lost the control of the position
	virtual void		lostPositionControl() =0;

	// calling this routine manage the received data
	virtual void		onIdle()=0;

	// called after post render so that plugin can draw additionnal stuffs
	virtual void		postRender(CDisplay &display) {}

	//getting the name of the plugin
	virtual std::string&	getName()=0;

	//testing whether the plugin is active or not (currently in use or not)
	virtual bool		isActive()=0;


	virtual bool		activatePlugin()=0;

	virtual bool		closePlugin()=0;
};
// Interface for primitive display plusing
class IPrimitiveDisplayer
{
public:
	// a bunch of data to help pluging in drawing task
	struct TRenderContext
	{
		// need to display detail information
		bool				ShowDetail;
		// is the primitive selected ?
		bool				Selected;
		// Some colors as defined by primitive state and configuration files
		NLMISC::CRGBA		MainColor;
		NLMISC::CRGBA		ArrowColor;
		NLMISC::CRGBA		DotColor;
		NLMISC::CRGBA		LineColor;
		// The primitive class information 
		const NLLIGO::CPrimitiveClass		*PrimitiveClass;
		// The display
		CDisplay			*Display;
	};

	virtual void drawPrimitive(const NLLIGO::IPrimitive *primitive, const TRenderContext &renderContext) =0;
};

#endif // NL_PLUGIN_INTERFACE_H