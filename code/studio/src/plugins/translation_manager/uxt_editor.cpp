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


#include "translation_manager_constants.h"
#include "uxt_editor.h"

#include <QTableWidget>
#include <QFormLayout>
#include <QCloseEvent>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QMenu>

#include <QFile>
#include <QTextStream>

#include "nel/misc/diff_tool.h"
#include "nel/misc/i18n.h"

namespace
{

QString getLang( const QString &fn )
{
	QString lang = fn;
	int idx = lang.lastIndexOf( '/' );
	if( idx == -1 )
		return "";

	lang = lang.mid( idx + 1 );
	idx = lang.lastIndexOf( '.' );
	if( idx == -1 )
		return "";

	lang = lang.left( idx );
	return lang;
}

}

namespace TranslationManager
{

class UXTEditorPvt
{
public:

	UXTEditorPvt()
	{
		t = new QTableWidget();
	}

	QTableWidget *t;
	std::vector< STRING_MANAGER::TStringInfo > infos;
};


UXTEditor::UXTEditor( QMdiArea *parent ) :
CEditor( parent )
{
	editor_type = Constants::ED_UXT;
	setAttribute( Qt::WA_DeleteOnClose );

	d_ptr = new UXTEditorPvt();

	blockTableSignals( false );
}

UXTEditor::~UXTEditor()
{
	delete d_ptr;
	d_ptr = NULL;
}

void UXTEditor::open( QString filename )
{
	std::vector< STRING_MANAGER::TStringInfo > &infos = d_ptr->infos;
	QString lang = getLang( filename );
	
	infos.clear();
	STRING_MANAGER::loadStringFile( filename.toUtf8().constData(), infos, true );

	if( d_ptr->infos.size() == 0 )
	{
		// The work file cannot be found, cannot proceed
		if( filename.endsWith( "wk.uxt" ) )
		{
			return;
		}

		int l = filename.lastIndexOf( "/" );
		if( l == -1 )
			return;

		QString fn = filename.left( l );		
		fn += "/wk.uxt";
		
		// The work file cannot be found, cannot proceed
		STRING_MANAGER::loadStringFile( fn.toUtf8().constData(), infos, true );
		if( d_ptr->infos.size() == 0 )
			return;
	}

	blockTableSignals( true );

	d_ptr->t->clear();
	d_ptr->t->setColumnCount( 2 );
	d_ptr->t->setRowCount( infos.size() );

	setHeaderText( "Id", lang.toUpper() + " Text" );

	int i = 0;

	std::vector< STRING_MANAGER::TStringInfo >::const_iterator itr = infos.begin();
	while( itr != infos.end() )
	{
		const STRING_MANAGER::TStringInfo &info = *itr;

		QTableWidgetItem *name = new QTableWidgetItem( info.Identifier.c_str() );
		QTableWidgetItem *text1 = new QTableWidgetItem( info.Text.toUtf8().c_str() );
		
		d_ptr->t->setItem( i, 0, name );
		d_ptr->t->setItem( i, 1, text1 );
		
		++itr;
		i++;
	}

	d_ptr->t->resizeColumnsToContents();

	blockTableSignals( false );

	setWidget( d_ptr->t );

	current_file = filename;
	setWindowTitle( filename + "[*]" );
	setWindowFilePath( filename );
}

void UXTEditor::save()
{
	saveAs( current_file );
}

void UXTEditor::saveAs( QString filename )
{
	QFile f( filename );
	if( !f.open( QIODevice::WriteOnly ) )
		return;

	QTextStream out( &f );

	int idx = 0;
	std::vector< STRING_MANAGER::TStringInfo >::const_iterator itr = d_ptr->infos.begin();
	while( itr != d_ptr->infos.end() )
	{
		QString hashLine = "// HASH_VALUE ";
		hashLine += QString( NLMISC::CI18N::hashToString( itr->HashValue ).c_str() ).toUpper();
		hashLine += "\r\n";
		
		QString idxLine = "// INDEX ";
		idxLine += QString::number( idx );
		idxLine += "\r\n";
		
		
		QString trLine = "";		
		trLine += itr->Identifier.c_str();
		trLine += "\t";		
		trLine += "[";
		trLine += itr->Text.toUtf8().c_str();
		trLine += "]";
		trLine += "\r\n";

		QString newLine = "\r\n";

		out << hashLine;
		out << idxLine;
		out << trLine;
		out << newLine;

		++itr;
		idx++;
	}

	f.close();

	setWindowModified( false );
}

void UXTEditor::activateWindow()
{
	showMaximized();
}


void UXTEditor::insertRow()
{
	d_ptr->infos.push_back( STRING_MANAGER::TStringInfo() );
	d_ptr->t->setRowCount( d_ptr->t->rowCount() + 1 );

	setWindowModified( true );
}


void UXTEditor::deleteRow()
{
	int r = d_ptr->t->currentRow();
	if( r < 0 )
		return;

	int answer = QMessageBox::question( this,
										tr( "Deleting a row" ),
										tr( "Are you sure you want to delete this row?" ),
										QMessageBox::Yes,
										QMessageBox::Cancel );
	if( QMessageBox::Yes != answer )
		return;							

	std::vector< STRING_MANAGER::TStringInfo >::iterator itr = d_ptr->infos.begin();
	itr += r;
	d_ptr->infos.erase( itr );

	d_ptr->t->removeRow( r );

	setWindowModified( true );
}

void UXTEditor::closeEvent( QCloseEvent *e )
{
	if( isWindowModified() )
	{
		int reply = QMessageBox::question( this,
										tr( "Table changed" ),
										tr( "The table has changed. Would you like to save your changes?" ),
										QMessageBox::Yes,
										QMessageBox::No
										);

		if( reply == QMessageBox::Yes )
			save();

	}

	e->accept();
	close();
}

void UXTEditor::contextMenuEvent( QContextMenuEvent *e )
{
	QMenu *menu = new QMenu( this );
	QAction *insertAction = new QAction( "Insert row", menu );
	QAction *deleteAction = new QAction( "Delete row", menu );

	connect( insertAction, SIGNAL( triggered( bool ) ), this, SLOT( insertRow() ) );
	connect( deleteAction, SIGNAL( triggered( bool ) ), this, SLOT( deleteRow() ) );	

	menu->addAction( insertAction );
	menu->addAction( deleteAction );
	menu->exec( e->globalPos() );
}

void UXTEditor::onCellChanged( int row, int column )
{
	QTableWidgetItem *item = d_ptr->t->item( row, column );
	STRING_MANAGER::TStringInfo &info = d_ptr->infos[ row ];

	if( column == 0 )
		info.Identifier = item->text().toUtf8().constData();
	else
	if( column == 1 )
		info.Text = item->text().toUtf8().constData();

	setWindowModified( true );
}

void UXTEditor::setHeaderText( const QString &id, const QString &text )
{
	QTableWidgetItem *h1 = new QTableWidgetItem( id );
	QTableWidgetItem *h2 = new QTableWidgetItem( text );
	h1->setTextAlignment( Qt::AlignLeft );
	h2->setTextAlignment( Qt::AlignLeft );
	d_ptr->t->setHorizontalHeaderItem( 0, h1 );
	d_ptr->t->setHorizontalHeaderItem( 1, h2 );
}

void UXTEditor::blockTableSignals( bool block )
{
	if( block )
		disconnect( d_ptr->t, SIGNAL( cellChanged( int, int ) ), this, SLOT( onCellChanged( int, int ) ) );
	else
		connect( d_ptr->t, SIGNAL( cellChanged( int, int ) ), this, SLOT( onCellChanged( int, int ) ) );
}

}
