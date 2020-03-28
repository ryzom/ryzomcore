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

//graph_plugin.h


#ifndef GRAPH_PLUGIN
#define GRAPH_PLUGIN

#include "..\world_editor\plugin_interface.h"
#include "world_editor_graph_plugin_dlg.h"



class CGraphPlugin : public IPluginCallback
{

public:
	CGraphPlugin();
	virtual ~CGraphPlugin();
	// @{
	// \name Overload for IPluginCallback
	virtual void		init(IPluginAccess *pluginAccess);

	/// The current region has changed.
//	virtual void		primRegionChanged(const std::vector<NLLIGO::CPrimRegion*> &regions);
	virtual void		primitiveChanged(const NLLIGO::IPrimitive *root);

	/// The region has been modifed.
//	virtual void		primRegionModifed();
	/// The listener has been moved on the map.
	virtual void		positionMoved(const NLMISC::CVector &position);
	/// The plugin lost the control of the position
	virtual void		lostPositionControl();	

	virtual void onIdle();

	virtual std::string& getName();

	virtual bool isActive();

	virtual bool activatePlugin();

	virtual bool closePlugin();

	void			doSelection(const std::string&);
	void			refreshPrimitives();
	void			refreshMachine();
	void			unsetDlgGraph();
	std::string		generateDotScript(NLLIGO::IPrimitive* managerNode);
	std::string		parseStateMachine(NLLIGO::IPrimitive* currentNode,std::string emiterNode,std::string tailLabel,std::string tag);
	std::string		spaceTo_(std::string strInput);
	std::string		createNode(NLLIGO::IPrimitive* managerNode,std::string strPredicate,uint& numClusters,std::string strShape);
	std::string		createParsedNode(NLLIGO::IPrimitive* managerNode,std::string strPredicate,uint& numClusters,std::string strShape);
private:

	bool createBitmap (const std::string& tmpPath);

	std::string				_DotPath;

	// Graph DLG
	CWorldEditorGraphPluginDlg			*GraphDlg;

	/// The world editor interface.
	IPluginAccess		*_PluginAccess;

	//Plugin's name
	std::string _Name;

	//set to true if the plugin is currently shown
	bool	_PluginActive;

	//Root's FileName, frome _dataHierarchy
	std::string _rootFileName;


};

extern "C"
{
	/// Export the C factory method for dynamic linking..
	//__declspec( dllexport ) void *createSoundPlugin();
	__declspec( dllexport ) void *createPlugin();



};

#endif