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


#include "project_window.h"

namespace GUIEditor
{
	ProjectWindow::ProjectWindow( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		connect( okButton, SIGNAL( clicked(bool) ), this, SLOT( hide() ) );
		connect( cancelButton, SIGNAL( clicked(bool) ), this, SLOT( hide() ) );
	}

	ProjectWindow::~ProjectWindow()
	{
	}

	void ProjectWindow::setupFileList( const std::vector< std::string > &fileNames )
	{
		fileList->clear();

		std::vector< std::string >::const_iterator itr;
		for( itr = fileNames.begin(); itr != fileNames.end(); ++itr )
		{
			const std::string &s = *itr;
			fileList->addItem( s.c_str() );
		}
		fileList->sortItems();
	}
}


