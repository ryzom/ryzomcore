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
#include <QFileDialog>

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

void markItemTranslated( QTableWidgetItem *item )
{
	item->setBackground( QColor::fromRgb( 126, 247, 134 ) );
}

void markItemUntranslated( QTableWidgetItem *item )
{
	item->setBackground( QColor::fromRgb( 247, 126, 126 ) );
}

class UXTEditorPvt
{
public:

	UXTEditorPvt()
	{
		t = new QTableWidget();
		loadedFromWK = false;
	}

	QTableWidget *t;
	std::vector< STRING_MANAGER::TStringInfo > infos;
	bool loadedFromWK;
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
	STRING_MANAGER::loadStringFile( filename.toUtf8().constData(), infos, false );

	if( d_ptr->infos.size() == 0 )
	{
		// The work file cannot be found, cannot proceed
		if( filename.endsWith( "wk.uxt" ) )
		{
			QMessageBox::critical( this,
									tr( "Error opening file.." ),
									tr( "There was an error opening wk.uxt" ) );
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
		{
			QMessageBox::critical( this,
									tr( "Error opening Uxt file" ),
									tr( "Neither the specified file nor wk.uxt could be opened." ) );
			return;
		}

		d_ptr->loadedFromWK = true;
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

		if( ( info.HashValue != 0 ) && !d_ptr->loadedFromWK )
		{
			markItemTranslated( name );
			markItemTranslated( text1 );
		}
		else
		{
			markItemUntranslated( name );
			markItemUntranslated( text1 );
		}
		
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
		uint64 hash = 0;

		// If text2 is not empty we can assume the string was translated, so we store with the correct hash
		// If text2 is empty, it wasn't translated so we can just use the old hash.
		// Additionally, if the strings were loaded from the wk.uxt file, we use a hash of 0 so we know it was not translated
		if( itr->Text2.empty() )
		{
			if( d_ptr->loadedFromWK )
				hash = 0;
			else
				hash = itr->HashValue;
		}
		else	
		{
			hash = NLMISC::CI18N::makeHash( itr->Text2 );		
		}

		QString hashLine = "// HASH_VALUE ";
		hashLine += QString( NLMISC::CI18N::hashToString( hash ).c_str() ).toUpper();
		hashLine += "\r\n";
		
		QString idxLine = "// INDEX ";
		idxLine += QString::number( idx );
		idxLine += "\r\n";
		
		
		QString trLine = "";		
		trLine += itr->Identifier.c_str();
		trLine += "\t";		
		trLine += "[";
		
		if( itr->Text2.empty() )
			trLine += itr->Text.toUtf8().c_str();
		else
			trLine += itr->Text2.toUtf8().c_str();

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
	blockTableSignals( true );

	d_ptr->infos.push_back( STRING_MANAGER::TStringInfo() );
	d_ptr->t->setRowCount( d_ptr->t->rowCount() + 1 );
	int row = d_ptr->t->rowCount() - 1;

	QTableWidgetItem *item1 = new QTableWidgetItem();
	QTableWidgetItem *item2 = new QTableWidgetItem();
	d_ptr->t->setItem( row, 0, item1 );
	d_ptr->t->setItem( row, 1, item2 );

	markRowUntranslated( row );

	setWindowModified( true );

	blockTableSignals( false );
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
	QAction *markAction = new QAction( "Mark translated", menu );
	QAction *unmarkAction = new QAction( "Mark not-translated", menu );
	QAction *saveAction = new QAction( "Save", menu );
	QAction *saveAsAction = new QAction( "Save as..", menu );

	connect( insertAction, SIGNAL( triggered( bool ) ), this, SLOT( insertRow() ) );
	connect( deleteAction, SIGNAL( triggered( bool ) ), this, SLOT( deleteRow() ) );
	connect( markAction, SIGNAL( triggered( bool ) ), this, SLOT( markTranslated() ) );
	connect( unmarkAction, SIGNAL( triggered( bool ) ), this, SLOT( markUntranslated() ) );
	connect( saveAction, SIGNAL( triggered( bool ) ), this, SLOT( onSaveClicked() ) );
	connect( saveAsAction, SIGNAL( triggered( bool ) ), this, SLOT( onSaveAsClicked() ) );

	menu->addAction( insertAction );
	menu->addAction( deleteAction );
	menu->addAction( markAction );
	menu->addAction( unmarkAction );
	menu->addAction( saveAction );
	menu->addAction( saveAsAction );
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
		info.Text2 = item->text().toUtf8().constData();

	setWindowModified( true );

	markRowTranslated( row );
}

void UXTEditor::markTranslated()
{
	int r = d_ptr->t->currentRow();
	if( r < 0 )
		return;

	STRING_MANAGER::TStringInfo &info = d_ptr->infos[ r ];
	if( !info.Text2.empty() )
		return;

	info.Text2 = info.Text;

	setWindowModified( true );

	markRowTranslated( r );
}

void UXTEditor::markUntranslated()
{
	int r = d_ptr->t->currentRow();
	if( r < 0 )
		return;

	STRING_MANAGER::TStringInfo &info = d_ptr->infos[ r ];

	info.Text2.clear();
	info.HashValue = 0;

	setWindowModified( true );

	markRowUntranslated( r );
}

void UXTEditor::onSaveClicked()
{
	save();
}

void UXTEditor::onSaveAsClicked()
{
	QString path = current_file;
	int idx = path.lastIndexOf( '/' );
	if( idx < 0 )
		path = "";
	else
		path = path.left( idx + 1 );

	QString file = QFileDialog::getSaveFileName( this,
												tr( "Save Uxt as.." ),
												path,
												tr( "Uxt files ( *.uxt)" ) );

	if( file.isEmpty() )
		return;

	saveAs( file );
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

void UXTEditor::markRowTranslated( int row )
{
	blockTableSignals( true );

	QTableWidgetItem *item1 = d_ptr->t->item( row, 0 );
	QTableWidgetItem *item2 = d_ptr->t->item( row, 1 );
	markItemTranslated( item1 );
	markItemTranslated( item2 );

	blockTableSignals( false );
}

void UXTEditor::markRowUntranslated( int row )
{
	blockTableSignals( true );

	QTableWidgetItem *item1 = d_ptr->t->item( row, 0 );
	QTableWidgetItem *item2 = d_ptr->t->item( row, 1 );
	markItemUntranslated( item1 );
	markItemUntranslated( item2 );

	blockTableSignals( false );
}

}
