// Ryzom Core Studio - Translation Manager Plugin
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


#ifndef UXT_EDITOR_H
#define UXT_EDITOR_H

#include "translation_manager_editor.h"

namespace TranslationManager
{

class UXTEditorPvt;

class UXTEditor : public CEditor
{
	Q_OBJECT
public:
	UXTEditor( QMdiArea *parent = NULL );
	~UXTEditor();
	
	void open( QString filename );
	void save();
	void saveAs( QString filename );
	void activateWindow();

public Q_SLOTS:
	void insertRow();
	void deleteRow();

protected:
	void closeEvent( QCloseEvent *e );
	void contextMenuEvent( QContextMenuEvent *e );

private Q_SLOTS:
	void onCellChanged( int row, int column );
	void markTranslated();
	void markUntranslated();
	void onSaveClicked();
	void onSaveAsClicked();

private:
	void setHeaderText( const QString &id, const QString &text );
	void blockTableSignals( bool block = false );
	void markRowTranslated( int row );
	void markRowUntranslated( int row );

	UXTEditorPvt *d_ptr;
};

}

#endif

