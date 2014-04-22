// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#ifndef PROJECT_WINDOW_H
#define PROJECT_WINDOW_H

#include <vector>
#include <string>
#include "ui_project_window.h"
#include "project_files.h"

namespace GUIEditor
{
	class ProjectWindow : public QWidget, public Ui::ProjectWindow
	{
		Q_OBJECT
	public:
		ProjectWindow( QWidget *parent = NULL );
		~ProjectWindow();

		void setupFiles( SProjectFiles &projectFiles );
		void updateFiles( SProjectFiles &projectFiles );
		void clear();

	Q_SIGNALS:
		void projectFilesChanged();

	private Q_SLOTS:
		void onAddButtonClicked();
		void onRemoveButtonClicked();
		void onOKButtonClicked();

	private:
		bool filesChanged;
	};
}


#endif
