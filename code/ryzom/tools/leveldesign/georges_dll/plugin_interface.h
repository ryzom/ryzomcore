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

#ifndef NLGEORGES_PLUGIN_INTERFACE_H
#define NLGEORGES_PLUGIN_INTERFACE_H

#include <nel/georges/u_form.h>
#include "nel/misc/config_file.h"

namespace NLGEORGES
{

/**
  * Georges plug-ins are dlls. To be loaded, dll must be listed in the georges.cfg config file.
  * The dll must export 1 function:
  *
  * IEditPlugin *IGeorgesEditGetInterface (int version, IEdit *globalInterface);
  *
  * This method must first compare the version number with NLGEORGES_PLUGIN_INTERFACE_VERSION defined in 
  * plugin_interface.h. If the version is not the same, the function must returns NULL (can show an error message).
  * If the number is the same, the plugin must instanciate an interface and return its pointers. It should
  * do only basic initialisation stuff. This method is called only one time by georges editor session.
  * The IEditPlugin derived object must be allocated with new. It will be delete by the system.
  *
  * The globalInterface pointer is valid to the end of the session.
  */

class IEdit;
class IEditPlugin;
class IEditDocument;
class IEditDocumentPlugin;

// The get interface plugin DLL entry.
typedef IEditPlugin *(*IGeorgesEditGetInterface) (int version, IEdit *globalInterface, NLMISC::INelContext &nelContext);

#define NLGEORGES_PLUGIN_INTERFACE_VERSION 0x1

/**
  * Communication interface between the plugin and georges editor.
  *
  * This interface give access to global global events handling.
  *
  * If you use MFC, don't forget to call AFX_MANAGE_STATE(AfxGetStaticModuleState());
  * at the beginning of each callback of the interface.
  *
  * This interface is instancied by the plugin.
  */
class IEditPlugin
{
public:
	
	/**
	  * Destructor must uninitialise the plugin interface
	  */
	virtual ~IEditPlugin () {}

	/// Window related

	/**
	  * This method is called at dialog initialisation.
	  * Typicaly, this method will create / show some global edition tools. (Not document edition related dialog).
	  *
	  * \param is the HWND of georges editor main frame.
	  */
	virtual void dialogInit (HWND mainFrm) = 0;

	/**
	  * This method is called to activate / disactivate the plugin UI.
	  *
	  * \param activate is true to activate the plugin UI, false to hide the plugin UI
	  */
	virtual void activate (bool activate) = 0;

	/**
	  * Get the plugin name.
	  */
	virtual void getPluginName (std::string &name) = 0;

	/**
	  * Pretranslate message hook.
	  * This method give a chance to the plugin to pretranslate a message before the main frame.
	  * Useful for shortcuts etc..
	  * Default implementation returns "false";
	  */
	virtual bool pretranslateMessage (MSG *pMsg) = 0;

	/// Events

	/**
	  * This method is called when a document has been created / opened. 
	  */
	virtual void onCreateDocument (IEditDocument *document) = 0;
};

/**
  * This interface give access to global functions.
  *
  * This interface is instancied by the system.
  */
class IEdit
{
public:
	/**
	  * Return the active document. Return NULL if no current document.
	  */
	virtual IEditDocument	*getActiveDocument () = 0;

	/**
	  * Return the current search path.
	  */
	virtual void			getSearchPath (std::string &searchPath) = 0;

	/**
	  * Create a form in the editor.
	  *
	  * \param dfnName is the name of the DFN. Can't be NULL.
	  * \param pathName is the file name of the created document. Can be NULL.
	  * \return the document pointer or NULL if a problem occured.
	*/
	virtual IEditDocument	*createDocument (const char *dfnName, const char *pathName) = 0;

	/** 
	  * Retreive the loaded config file. 
	  * Usefull for plugins to read any configuration var.
	  */
	virtual NLMISC::CConfigFile		&getConfigFile() =0;
};

/**
  * Communication interface between the plugin and georges editor document.
  * For the time, the interface can't modify document. It can only read it.
  * The interface must be allocated with new. It will be released by the
  * system when the document will be destroyed.
  *
  * If you use MFC, don't forget to call AFX_MANAGE_STATE(AfxGetStaticModuleState());
  * at the beginning of each callback of the interface.
  *
  * This interface is instancied by the plugin.
  */
class IEditDocumentPlugin
{
public:

	/**
	  * Destructor must uninitialise the plugin interface
	  */
	virtual ~IEditDocumentPlugin () {}

	/// Event message

	/**
	  * This method is called to intialise dialog during IEditDocument::bind ().
	  * Typicaly, this method will create / show some document control tools.
	  *
	  * \param is the HWND of the document view.
	  */
	virtual void dialogInit (HWND documentView) = 0;

	/**
	  * Pretranslate message hook.
	  * This method give a chance to the plugin to pretranslate a message before the main frame.
	  * Useful for shortcuts etc..
	  * Default implementation returns "false";
	  */
	virtual bool pretranslateMessage (MSG *pMsg) = 0;

	/**
	  * This method is called when a document's view has been activated / unactivated.
	  * If activated == true, the view has been activated, else the view is unactivated.
	  */
	virtual void activate (bool activated) = 0;

	/**
	  * Called by the system when a georges value has been changed.
	  */
	virtual void onValueChanged (const char *formName) = 0;

	/**
	  * Called by the system when the current edition node has been changed.
	  * Call IEditDocument::getActiveNode () to get the active node form name.
	  */
	virtual void onNodeChanged () = 0;
};

/**
  * Document access interface.
  * By this interface, you can access some values of the document.
  *
  * This interface is instancied by the system.
  */
class IEditDocument
{
public:
	/// Get the georges form pointer
	virtual UForm *getForm () = 0;

	/// Get document DFN filename
	virtual void getDfnFilename (std::string &dfnName) = 0;

	/// Get the document active node form name. Can return false if the node is not a form node.
	virtual bool getActiveNode (std::string &formName) = 0;

	/// Refresh the document view.
	virtual void refreshView () = 0;

	/// Return document filename.
	virtual void getFilename (std::string &pathname) = 0;

	/// Return document title.
	virtual void getTitle (std::string &pathname) = 0;

	/**
	  * Attach a document interface to this document.
	  *
	  * \param plugin is the global plugin interface.
	  * \param docInterface is a document interface allocated with new. It will be released by the document.
	  */
	virtual void bind (NLGEORGES::IEditPlugin *plugin, NLGEORGES::IEditDocumentPlugin *docInterface) = 0;

	/// \name Modify the document

	/**
	  * Set a form value with its name. If the node doesn't exist, it is created.
	  *
	  * \param value is a reference on the value to set in the form.
	  * \param name is the form name of the value to set or create.
	  * \param slot is 0, 1, 2, 3. 0 is the current document, 1 is 1st snapshot, 2 the 2nd snapshot etc..
	  */
	virtual void setValue (const char *value, const char *name, uint slot=0) = 0;
};

} // NLGEORGES

#endif // NLGEORGES_PLUGIN_INTERFACE_H

