// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef MATERIAL_EDITOR_WINDOW_H
#define MATERIAL_EDITOR_WINDOW_H

#include "ui_material_editor_window.h"

namespace MaterialEditor
{

	class ShaderWidget;
	class RenderPassesWidget;
	class CNel3DInterface;
	class MaterialSplitter;
	class ViewPortWidget;

	class MaterialEditorWindow: public QMainWindow
	{
		Q_OBJECT
public:
		MaterialEditorWindow( QWidget *parent = NULL );		
		~MaterialEditorWindow();

		void onOpenClicked();

private Q_SLOTS:
		void onNewMaterialClicked();
		void onOpenMaterialClicked();
		void onSaveMaterialClicked();
		void onShadersClicked();
		void onPassesClicked();
		void onStartup();

		void onAddCubeClicked();
		void onAddSphereClicked();
		void onAddCylinderClicked();
		void onAddTeaPotClicked();

		void onClearSceneClicked();
		void onTriangleClicked();
		
private:
		void createMenus();
		void createDockWidgets();
		void setupConnections();

		CNel3DInterface *nl3dIface;

		ShaderWidget *shaderWidget;
		RenderPassesWidget *passesWidget;
		MaterialSplitter *materialSplitter;
		ViewPortWidget *viewPort;

		Ui::MaterialEditorWindow m_ui;

		QString lastShapeDir;
		QString lastMatDir;
	};

}

#endif

