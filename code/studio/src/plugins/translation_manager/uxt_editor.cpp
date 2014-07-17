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
#include <QMessageBox>

#include <QFile>
#include <QTextStream>

#include "nel/misc/diff_tool.h"

namespace TranslationManager
{

class UXTEditorPvt
{
public:

	UXTEditorPvt()
	{
		t = new QTableWidget();
		changed = false;
	}

	QTableWidget *t;
	std::vector< STRING_MANAGER::TStringInfo > infos;
	bool changed;
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
	
	infos.clear();
	STRING_MANAGER::loadStringFile( filename.toUtf8().constData(), infos, true );

	if( d_ptr->infos.size() == 0 )
		return;

	blockTableSignals( true );

	d_ptr->t->clear();
	d_ptr->t->setColumnCount( 2 );
	d_ptr->t->setRowCount( infos.size() );

	setHeaderText( "Id", "Text" );

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
	setCurrentFile( filename );
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

	std::vector< STRING_MANAGER::TStringInfo >::const_iterator itr = d_ptr->infos.begin();
	while( itr != d_ptr->infos.end() )
	{
		QString line = "";
		
		line += itr->Identifier.c_str();
		line += "\t";
		
		line += "[";
		line += itr->Text.toUtf8().c_str();
		line += "]";

		line += "\r\n";

		out << line;

		++itr;
	}

	f.close();

	d_ptr->changed = false;
}

void UXTEditor::activateWindow()
{
	showMaximized();
}


void UXTEditor::closeEvent( QCloseEvent *e )
{
	if( d_ptr->changed )
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

void UXTEditor::onCellChanged( int row, int column )
{
	QTableWidgetItem *item = d_ptr->t->item( row, column );
	STRING_MANAGER::TStringInfo &info = d_ptr->infos[ row ];

	if( column == 0 )
		info.Identifier = item->text().toUtf8().constData();
	else
	if( column == 1 )
		info.Text = item->text().toUtf8().constData();

	d_ptr->changed = true;
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
