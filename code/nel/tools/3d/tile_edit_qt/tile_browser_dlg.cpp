// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QListWidgetItem>
#include <QtGui/QColor>
#include <QtCore/QList>
#include <QtGui/QAction>
#include <QtCore/QSettings>
#include "nel/3d/tile_bank.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/file.h"
#include "pic/readpic.h"
#include "pic/pic.h"
#include "tile_browser_dlg.h"
#include "tile_rotation_dlg.h"

using namespace NL3D;
using namespace NLMISC;

extern CTileBank tileBankBrowser;
int thread_actif = 0;
QSettings settings("NeL", "Tile Edit");



CTile_browser_dlg::CTile_browser_dlg(QWidget *parent, Qt::WindowFlags flags)
     : QDialog(parent, flags)
 {
	 ui.setupUi(this);
 }




void CTile_browser_dlg::initDialog(const int& tileSetIndex)
{
	this->tileSetIndex = tileSetIndex;


	//GroupBox Creation
	tileTypeButtonGroup = new QButtonGroup;
    tileTypeButtonGroup->setExclusive(true);
	tileTypeButtonGroup->addButton(ui._128x128RadioButton, _128x128);
	tileTypeButtonGroup->addButton(ui._256x256RadioButton, _256x256);
	tileTypeButtonGroup->addButton(ui.transitionRadioButton, Transition);
	tileTypeButtonGroup->addButton(ui.displaceRadioButton, Displace);

	tileTextureButtonGroup = new QButtonGroup;
    tileTextureButtonGroup->setExclusive(true);
	tileTextureButtonGroup->addButton(ui.diffuseRadioButton, Diffuse);
	tileTextureButtonGroup->addButton(ui.additiveRadioButton, Additive);
	tileTextureButtonGroup->addButton(ui.alphaRadioButton, Alpha);

	tileLabelButtonGroup = new QButtonGroup;
    tileLabelButtonGroup->setExclusive(true);
	tileLabelButtonGroup->addButton(ui.indexRadioButton, CTile_browser_dlg::Index);
	tileLabelButtonGroup->addButton(ui.fileNameRadioButton, CTile_browser_dlg::FileName);

	tileZoomButtonGroup = new QButtonGroup;
    tileZoomButtonGroup->setExclusive(true);
	tileZoomButtonGroup->addButton(ui.smallRadioButton, CTile_browser_dlg::Small);
	tileZoomButtonGroup->addButton(ui.normalRadioButton, CTile_browser_dlg::Normal);
	tileZoomButtonGroup->addButton(ui.bigRadioButton, CTile_browser_dlg::Big);

	//Contextual Actions Creation
	ui.tileBrowserListView->addAction(ui.actionAddTile);
	ui.tileBrowserListView->addAction(ui.actionDeleteTile);
	ui.tileBrowserListView->addAction(ui.actionReplaceImage);
	ui.tileBrowserListView->addAction(ui.actionDeleteImage);


	//Layout
    QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addWidget(ui.buttonFrame);
    mainLayout->addWidget(ui.tileBrowserListView);
    setLayout(mainLayout);

	//Apply Settings
	int width = settings.value("browser/width").toInt();
	int height = settings.value("browser/height").toInt();
	if (width > 0 && height > 0)
		this->resize(width, height);

	if ( settings.contains("browser/TileType") )
		tileTypeButtonGroup->button(settings.value("browser/TileType").toInt())->setChecked(true);

	if ( settings.contains("browser/TileTexture") )
		tileTextureButtonGroup->button(settings.value("browser/TileTexture").toInt())->setChecked(true);

	if ( settings.contains("browser/TileLabel") )
		tileLabelButtonGroup->button(settings.value("browser/TileLabel").toInt())->setChecked(true);

	if ( settings.contains("browser/TileZoom") )
		tileZoomButtonGroup->button(settings.value("browser/TileZoom").toInt())->setChecked(true);

	//GroupBox checkedIdChanged subscription
    connect(tileTypeButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(on_tileTypeButtonGroup_clicked(int)));
	connect(tileTextureButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(on_tileTextureButtonGroup_clicked(int)));
    connect(tileLabelButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(on_tileLabelButtonGroup_clicked(int)));
    connect(tileZoomButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(on_tileZoomButtonGroup_clicked(int)));

	//Tile View Model
	tileViewModel = new tiles_model(this);
    ui.tileBrowserListView->setModel(tileViewModel);
	connect(ui.tileBrowserListView->selectionModel(), SIGNAL(selectionChanged( const QItemSelection &, const QItemSelection & )), this, SLOT(on_tiles_model_selectionChanged(const QItemSelection &, const QItemSelection & )));

	// Tile Set
	browserModel._tileSet = tileSetIndex;

	// 128 Tiles
	int _128Count = tileBankBrowser.getTileSet (tileSetIndex)->getNumTile128 ();
	browserModel.theList128.resize (_128Count);
	for (int i=0; i<_128Count; i++)
	{
		browserModel.theList128[i].Init(i, _128x128);
	}
	browserModel.Reload (0, _128Count, _128x128);

	// 256 Tiles
	int _256Count = tileBankBrowser.getTileSet (tileSetIndex)->getNumTile256 ();
	browserModel.theList256.resize (_256Count);
	for (int i=0; i<_256Count; i++)
	{
		browserModel.theList256[i].Init(i, _256x256);
	}
	browserModel.Reload (0, _256Count, _256x256);

	// Transition Tiles
	for (int i=0; i<CTileSet::count; i++)
	{
		browserModel.theListTransition[i].Init(i, Transition);
	}
	browserModel.Reload (0, CTileSet::count, Transition);

	// Displacement Tiles
	for (int i=0; i<CTileSet::CountDisplace; i++)
	{
		browserModel.theListDisplacement[i].Init(i, Displace);
	}
	browserModel.Reload (0, CTileSet::CountDisplace, Displace);


	LoadInThread();

}

void CTile_browser_dlg::on_tileTypeButtonGroup_clicked(int id )
{
	switch(id)
	{
		case _128x128:
		case _256x256:
			if (tileTextureButtonGroup->checkedId() == Alpha)
				ui.diffuseRadioButton->setChecked(true);
			break;

		case Displace:
			ui.diffuseRadioButton->setChecked(true);
			break;
	
	}
	LoadInThread();
}

void CTile_browser_dlg::on_tileTextureButtonGroup_clicked(int id )
{
	LoadInThread();
}


void CTile_browser_dlg::on_tileLabelButtonGroup_clicked(int id )
{
	LoadInThread();
}

void CTile_browser_dlg::on_tileZoomButtonGroup_clicked(int id )
{
	LoadInThread();
}

void CTile_browser_dlg::on_tiles_model_selectionChanged (const QItemSelection & s, const QItemSelection & d)
{
	if ( ! ui.tileBrowserListView->selectionModel()->selectedRows().empty() )
	{
		ui.actionReplaceImage->setEnabled(true);
		ui.actionDeleteImage->setEnabled(true);

		if (tileTypeButtonGroup->checkedId() == _128x128 || tileTypeButtonGroup->checkedId() == _256x256)
		{
			ui.actionDeleteTile->setEnabled(true);
		}
	}
	else
	{
		ui.actionReplaceImage->setEnabled(false);
		ui.actionDeleteImage->setEnabled(false);

		ui.actionDeleteTile->setEnabled(false);
	}
}



void CTile_browser_dlg::on_actionAddTile_triggered(bool checked)
{
	QFileDialog::Options options;
	QString selectedFilter;
	QStringList fileNames = QFileDialog::getOpenFileNames(this, "Choose Bitmap", QString(tileBankBrowser.getAbsPath().c_str()) , "Targa Bitmap(*.tga);;All Files (*.*);;", &selectedFilter, options);

	qSort(fileNames.begin(), fileNames.end());

	if (!fileNames.isEmpty())
	{
		int tileId;
		QString fileName;
		switch (tileTypeButtonGroup->checkedId())
		{
			case _128x128:
				for (int i = 0; i < fileNames.size(); i++)
				{
					tileId = browserModel.addTile128 ();
					fileName = QDir::toNativeSeparators(fileNames.at(i));
					if ( ! browserModel.setTile128 ( tileId, fileName.toUtf8().constData(), (CTile::TBitmap) tileTextureButtonGroup->checkedId()) )
					{
						browserModel.removeTile128 (tileId);
						break;
					}
				}
				break;
			case _256x256:
				for (int i = 0; i < fileNames.size(); i++)
				{
					tileId = browserModel.addTile256 ();
					fileName = QDir::toNativeSeparators(fileNames.at(i));
					if ( ! browserModel.setTile256 ( tileId, fileName.toUtf8().constData(), (CTile::TBitmap) tileTextureButtonGroup->checkedId()) )
					{
						browserModel.removeTile256 (tileId);
						break;
					}
				}
				break;
			default:
				nlassert(0);
				break;
		}

		LoadInThread();
	}
}


void CTile_browser_dlg::on_actionDeleteTile_triggered(bool checked)
{
		switch (tileTypeButtonGroup->checkedId())
		{
			case _128x128:
				for (int i=0; i<ui.tileBrowserListView->selectionModel()->selectedRows().count(); i++)
				{
					int tileId = (ui.tileBrowserListView->selectionModel()->selectedRows().at(i).data(Qt::UserRole + 1)).toInt();
					browserModel.removeTile128 (tileId);
				}
				break;
			case _256x256:
				for (int i=0; i<ui.tileBrowserListView->selectionModel()->selectedRows().count(); i++)
				{
					int tileId = (ui.tileBrowserListView->selectionModel()->selectedRows().at(i).data(Qt::UserRole + 1)).toInt();
					browserModel.removeTile256 (tileId);
				}
				break;
			default:
				nlassert(0);
				break;
		}

		LoadInThread();
}

void CTile_browser_dlg::on_actionReplaceImage_triggered(bool checked)
{
	QFileDialog::Options options;
	QString selectedFilter;
	QString fileName = QFileDialog::getOpenFileName(this, "Choose Bitmap", QString(tileBankBrowser.getAbsPath().c_str()) , "Targa Bitmap(*.tga);;All Files (*.*);;", &selectedFilter, options);
	
	if (!fileName.isEmpty())
	{
		bool ok = false;
		fileName = QDir::toNativeSeparators(fileName);
		switch (tileTypeButtonGroup->checkedId())
		{
			case _128x128:
				for (int i=0; i<ui.tileBrowserListView->selectionModel()->selectedRows().count(); i++)
				{
					int tileId = (ui.tileBrowserListView->selectionModel()->selectedRows().at(i).data(Qt::UserRole + 1)).toInt();
					
					if ( ! browserModel.setTile128 ( tileId, fileName.toUtf8().constData(), (CTile::TBitmap) tileTextureButtonGroup->checkedId()) )
						break;
				}
				break;
			case _256x256:
				for (int i=0; i<ui.tileBrowserListView->selectionModel()->selectedRows().count(); i++)
				{
					int tileId = (ui.tileBrowserListView->selectionModel()->selectedRows().at(i).data(Qt::UserRole + 1)).toInt();
					if ( ! browserModel.setTile256 (tileId, fileName.toUtf8().constData(), (CTile::TBitmap) tileTextureButtonGroup->checkedId()) )
						break;
				}
				break;
			case Transition:
				for (int i=0; i<ui.tileBrowserListView->selectionModel()->selectedRows().count(); i++)
				{
					int tileId = (ui.tileBrowserListView->selectionModel()->selectedRows().at(i).data(Qt::UserRole + 1)).toInt();
					if ( tileTextureButtonGroup->checkedId() != Alpha )
					{
						if ( ! browserModel.setTileTransition (tileId, fileName.toUtf8().constData(), (CTile::TBitmap) tileTextureButtonGroup->checkedId()) )
							break;
					}
					else
					{
						bool rotationOk = false;
						int rot = CTile_rotation_dlg::getRotation(this, &rotationOk);
						if (rotationOk)
						{
							if ( ! browserModel.setTileTransitionAlpha (tileId, fileName.toUtf8().constData(), rot) )
								break;
						}
					}
				}
				break;
			case Displace:
				for (int i=0; i<ui.tileBrowserListView->selectionModel()->selectedRows().count(); i++)
				{
					int tileId = (ui.tileBrowserListView->selectionModel()->selectedRows().at(i).data(Qt::UserRole + 1)).toInt();
					if ( ! browserModel.setDisplacement (tileId, fileName.toUtf8().constData(), (CTile::TBitmap) tileTextureButtonGroup->checkedId()) )
						break;
				}
				break;
			default:
				nlassert (0); // no!
		}

		LoadInThread();
	}
}

void CTile_browser_dlg::on_actionDeleteImage_triggered(bool checked)
{
	switch (tileTypeButtonGroup->checkedId())
	{
		case _128x128:
			for (int i=0; i<ui.tileBrowserListView->selectionModel()->selectedRows().count(); i++)
			{
				int tileId = (ui.tileBrowserListView->selectionModel()->selectedRows().at(i).data(Qt::UserRole + 1)).toInt();
				browserModel.clearTile128 ( tileId, (CTile::TBitmap) tileTextureButtonGroup->checkedId());
			}
			break;
		case _256x256:
			for (int i=0; i<ui.tileBrowserListView->selectionModel()->selectedRows().count(); i++)
			{
				int tileId = (ui.tileBrowserListView->selectionModel()->selectedRows().at(i).data(Qt::UserRole + 1)).toInt();
				browserModel.clearTile256 ( tileId, (CTile::TBitmap) tileTextureButtonGroup->checkedId());
			}
			break;
		case Transition:
			for (int i=0; i<ui.tileBrowserListView->selectionModel()->selectedRows().count(); i++)
			{
				int tileId = (ui.tileBrowserListView->selectionModel()->selectedRows().at(i).data(Qt::UserRole + 1)).toInt();
				browserModel.clearTransition ( tileId, (CTile::TBitmap) tileTextureButtonGroup->checkedId());
			}
			break;
		case Displace:
			for (int i=0; i<ui.tileBrowserListView->selectionModel()->selectedRows().count(); i++)
			{
				int tileId = (ui.tileBrowserListView->selectionModel()->selectedRows().at(i).data(Qt::UserRole + 1)).toInt();
				browserModel.clearDisplacement ( tileId, (CTile::TBitmap) tileTextureButtonGroup->checkedId() );
			}
			break;
		default:
			nlassert (0); // no!
	}

	LoadInThread();
}

//TODO titegus: Useless
void CTile_browser_dlg::on_refreshPushButton_clicked()
{
	LoadInThread();
}


void CTile_browser_dlg::on_batchLoadPushButton_clicked()
{
	QFileDialog::Options options;
	QString selectedFilter;
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Bitmap"), QString(tileBankBrowser.getAbsPath().c_str()) , tr("Targa Bitmap (*.tga);;PNG Image (*.png);;All Files (*.*);;"), &selectedFilter, options);
	QFileInfo fi(fileName);
	QString baseName = fi.baseName() ;


	if (!fileName.isEmpty())
	{
		QRegExp rx("\\d{2}$");
		baseName = baseName.remove(rx);
	
		
		//TODO titegus: What's the point in asking for rotation if Texture != Alpha ???
		bool rotate = (QMessageBox::Yes == QMessageBox::question(this, tr("Import rotated tiles"), tr("Do you want to use rotation to reuse alpha tiles?"), QMessageBox::Yes | QMessageBox::No ));

		for (int i=0; i<CTileSet::count; i++)
		{
			if (tileTextureButtonGroup->checkedId() == Alpha)
			{
				// Current transition
				CTileSet::TTransition transition=(CTileSet::TTransition)i;



				// Transition to patch
				CTileSetTransition* trans=tileBankBrowser.getTileSet (tileSetIndex)->getTransition (transition);
				if (tileBankBrowser.getTile (trans->getTile())->getRelativeFileName (CTile::alpha)=="")
				{
					// Continue ?
					int ok;

					// Try to load transition with rotation
					for (int rot=0; rot<4; rot++)
					{
						// Try to load a tile with a file name like /tiletransition0.tga
						QString transitionNumber = QString::number(transition);
						QString batchNumber = transitionNumber.rightJustified(2, '0');
						QString nextBaseName = baseName + batchNumber;
						QString nextFileName = QDir::toNativeSeparators(fi.absolutePath()) + QDir::separator() + nextBaseName + QString(".") + fi.suffix();
						FILE *pFile=fopen (nextFileName.toUtf8().constData(), "rb");

						// Close the file and add the tile if opened
						if (pFile)
						{
							fclose (pFile);
							ok=browserModel.setTileTransitionAlpha (i, nextFileName.toUtf8().constData(), (4-rot)%4);

							// End
							break;
						}

						// Rotate the transition
						transition=CTileSet::rotateTransition (transition);

						if (!rotate)
							break;
					}
					if (!ok)
						break;
				}
			}
			else
			{

				//TODO titegus: Check that, Batch Load seems useless
				// Current transition
				CTileSet::TTransition transition=(CTileSet::TTransition)i;

				// Transition to patch
				//CTileSetTransition* trans=tileBankBrowser.getTileSet (tileSetIndex)->getTransition (transition);
				//if (tileBankBrowser.getTile (trans->getTile())->getRelativeFileName ((CTile::TBitmap)tileTextureButtonGroup->checkedId())=="")
				//{
				//	// Try to load a tile with a file name like /tiletransition0.tga
				//	char sName2[256];
				//	char sFinal[256];
				//	sprintf (sName2, "%s%02d", sName, (int)transition);
				//	_makepath (sFinal, sDrive, sPath, sName2, sExt);
				//	FILE *pFile=fopen (sFinal, "rb");

				//	// Close the file and add the tile if opened
				//	if (pFile)
				//	{
				//		fclose (pFile);
				//		if (!infoList.setTileTransition (i, sFinal, (CTile::TBitmap) tileTextureButtonGroup->checkedId()))
				//			break;
				//	}
				//}
			}
		}
		

		LoadInThread();

	}
}

//TODO titegus: replace that by 4 buttons Export128Diffuse, Export128Additive, Export256Diffuse, Export256Diffuse ?
void CTile_browser_dlg::on_exportBorderPushButton_clicked()
{
	// Select a file
	QFileDialog::Options options;
	QString selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Choose Bitmap"), QString(tileBankBrowser.getAbsPath().c_str()) , "Targa Bitmap(*.tga);;All Files (*.*);;", &selectedFilter, options);
	
	if (!fileName.isEmpty())
	{
		fileName = QDir::toNativeSeparators(fileName);
		// Get the border of the bank
		std::vector<NLMISC::CBGRA> array;

		// 256 or 128 ?
		int width, height;
		//TODO titegus: And So what if Alpha ??? and what about border256 ???
		tileBankBrowser.getTileSet (tileSetIndex)->getBorder128 ((CTile::TBitmap)tileTextureButtonGroup->checkedId())->get (width, height, array);

		// Make a bitmap
		if (width&&height)
		{
			NLMISC::CBitmap bitmap;
			bitmap.resize (width, height, NLMISC::CBitmap::RGBA);

			// Get pixel
			CRGBA *pPixel=(CRGBA*)&bitmap.getPixels()[0];

			// Make a copy
			for (int i=0; i<width*height; i++)
			{
				// Copy the pixel
				pPixel->R=array[i].R;
				pPixel->G=array[i].G;
				pPixel->B=array[i].B;
				pPixel->A=array[i].A;
				pPixel++;
			}

			// Write the bitmap
			bool error=false;
			try
			{
				COFile file;
				if (file.open (fileName.toUtf8().constData()))
				{
					// Export
					bitmap.writeTGA (file, 32);
				}
				else
					error=true;
			}
			catch (Exception& e)
			{
				const char *toto=e.what ();
				error=true;
			}

			// Error during export ?
			if (error)
			{
				// Error message
				QString s = tr("Can't write bitmap %1").arg(fileName);
				QMessageBox::information (this, tr("Export border"), s);
			}
		}
	}

}



//TODO titegus: What's the point in Importing a new border if there is no Pixel Compatibility check ?
void CTile_browser_dlg::on_importBorderPushButton_clicked()
{
	QFileDialog::Options options;
	QString selectedFilter;
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Bitmap"), QString(tileBankBrowser.getAbsPath().c_str()) , "Targa Bitmap(*.tga);;All Files (*.*);;", &selectedFilter, options);
	
	if (!fileName.isEmpty())
	{
		fileName = QDir::toNativeSeparators(fileName);
		// Get the border of the bank
		std::vector<NLMISC::CBGRA> array(128*128);

		// The bitmap
		NLMISC::CBitmap bitmap;

		// Read the bitmap
		bool error=false;
		try
		{
			CIFile file;
			if (file.open (fileName.toUtf8().constData()))
			{
				// Export
				bitmap.load (file);
			}
			else
				error=true;
		}
		catch (Exception& e)
		{
			const char *toto=e.what ();
			error=true;
		}

		// Error during import ?
		if (error)
		{
			// Error message
			QString s = tr("Can't read bitmap %1").arg(fileName);
			QMessageBox::information (this, tr("Import border"), s);
		}

		// Get pixel
		CRGBA *pPixel=(CRGBA*)&bitmap.getPixels()[0];

		// Good size
		if ((bitmap.getWidth()==128)&&(bitmap.getHeight()==128))
		{
			// Make a copy
			for (int i=0; i<128*128; i++)
			{
				// Copy the pixel
				array[i].R=pPixel->R;
				array[i].G=pPixel->G;
				array[i].B=pPixel->B;
				array[i].A=pPixel->A;
				pPixel++;
			}
		}
		else
		{
			// Error message
			QString s = tr("The bitmap must have a size of 128x128 (%1)").arg(fileName);
			QMessageBox::information (this, tr("Import border"), s);
		}

		// 256 or 128 ?
		CTileBorder border;
		border.set (128, 128, array);
		tileBankBrowser.getTileSet (tileSetIndex)->setBorder ((CTile::TBitmap) tileTextureButtonGroup->checkedId(), border);

		// Message
		QMessageBox::information (this, tr("Import border"), tr("The border has been changed."));
	}

}



void CTile_browser_dlg::EnableBrowserInteractions()
{
	//TileType
	switch(tileTypeButtonGroup->checkedId())
	{
		case _128x128:
		case _256x256:
			ui.diffuseRadioButton->setEnabled(true);
			ui.additiveRadioButton->setEnabled(true);
			ui.alphaRadioButton->setEnabled(false);

			ui.actionAddTile->setEnabled(true);
			break;

		case Transition:
			ui.diffuseRadioButton->setEnabled(true);
			ui.additiveRadioButton->setEnabled(true);
			ui.alphaRadioButton->setEnabled(true);

			ui.actionAddTile->setEnabled(false);
			ui.actionDeleteTile->setEnabled(false);
			break;

		case Displace:
			ui.diffuseRadioButton->setEnabled(false);
			ui.additiveRadioButton->setEnabled(false);
			ui.alphaRadioButton->setEnabled(false);

			ui.actionAddTile->setEnabled(false);
			ui.actionDeleteTile->setEnabled(false);
			break;
	
	}


	//Tile Texture
	if ( tileTextureButtonGroup->checkedId()  == Alpha)
	{
		ui.batchLoadPushButton->setEnabled(true);

		//TODO titegus:Makes no sense, ExportBorder should not be linked to the selected radio button , Replace exportBorder By ExportDiffuse and ExportAdditive??
		ui.exportBorderPushButton->setEnabled(false);
	}
	else
	{
		ui.batchLoadPushButton->setEnabled(false);

		//TODO titegus:Makes no sense, ExportBorder should not be linked to the selected radio button , Replace exportBorder By ExportDiffuse and ExportAdditive??
		ui.exportBorderPushButton->setEnabled(true);
	}
}

void CTile_browser_dlg::RefreshView()
{
	thread_actif = 1;

	//Remove Tiles from the View model
	tileViewModel->removeAllTiles();

	//Add Tiles in the View model
	int listCount = browserModel.GetSize(tileTypeButtonGroup->checkedId());
	tilelist::iterator p = browserModel.GetFirst(tileTypeButtonGroup->checkedId());
	for (p = browserModel.GetFirst(tileTypeButtonGroup->checkedId()); p != browserModel.GetLast(tileTypeButtonGroup->checkedId()); p++)
	{
		std::vector<NLMISC::CBGRA>* bits;
		bool tileLoaded = false;
		std::string tilePath;
		switch(tileTextureButtonGroup->checkedId())
		{
			case Diffuse:
				bits = &p->Bits;
				tileLoaded = p->loaded;
				tilePath = p->path;
				break;

			case Additive:
				bits = &p->nightBits;
				tileLoaded = p->nightLoaded;
				tilePath = p->nightPath;
				break;

			case Alpha:
				bits = &p->alphaBits;
				tileLoaded = p->alphaLoaded;
				tilePath = p->alphaPath;
				break;
		}

		if (tileLoaded)
		{
			int itBufferPixel = 0;
			double bufferWidth = sqrt((double)(*bits).size());
			double bufferHeight = sqrt((double)(*bits).size());

			QImage image(QSize(bufferWidth,bufferHeight), QImage::Format_ARGB32 );
			for(int colIndex = 0;colIndex<bufferHeight;colIndex++)
			{			
				for(int lineIndex = 0;lineIndex<bufferWidth;lineIndex++)
				{
					image.setPixel(lineIndex,colIndex,qRgb((*bits)[itBufferPixel].R
											  ,(*bits)[itBufferPixel].G
											  ,(*bits)[itBufferPixel].B));
					itBufferPixel ++;
				}
			}

			image = image.scaled(tileZoomButtonGroup->checkedId() * (tileTypeButtonGroup->checkedId()==_256x256 ? 2 : 1), tileZoomButtonGroup->checkedId() * (tileTypeButtonGroup->checkedId()==_256x256 ? 2 : 1));

			QString fileInfo = QString::number(p->getId());
			if (tileLabelButtonGroup->checkedId() == CTile_browser_dlg::FileName)
			{
				QFileInfo fi = QFileInfo(QString( tilePath.c_str()));
				fileInfo = fi.fileName();
			}
			QPixmap pixmap = QPixmap::fromImage(image);
			TileModel tile = TileModel(pixmap, fileInfo, p->getId());
			tileViewModel->addTile(tile);

		}
		else
		{
			QString fileInfo;
			if (tileLabelButtonGroup->checkedId() == CTile_browser_dlg::Index)
			{
				fileInfo = QString::number(p->getId());
			}
			TileModel tile = TileModel(tileZoomButtonGroup->checkedId() * (tileTypeButtonGroup->checkedId()==_256x256 ? 2 : 1), fileInfo, p->getId());
			tileViewModel->addTile(tile);
		}

		tileViewModel->sort();

	}

	EnableBrowserInteractions();
	
	thread_actif = 0;
}


void CTile_browser_dlg::LoadInThread(void)
{
	if (!thread_actif)
		RefreshView();
}

void CTile_browser_dlg::closeEvent(QCloseEvent *event)
 {
	int reply = QMessageBox::question(this, tr("Quit"), tr("Are you sure you want to Quit TileSet Edition without Saving?"), QMessageBox::Yes | QMessageBox::No);
	if (reply  == QMessageBox::Yes)
	{
		event->accept();
	} 
	else 
	{
		event->ignore();
	}
 }

void CTile_browser_dlg::accept()
{
	// save settings
	settings.setValue("browser/width", this->width());
	settings.setValue("browser/height", this->height());
	settings.setValue("browser/TileType", this->tileTypeButtonGroup->checkedId());
	settings.setValue("browser/TileTexture", this->tileTextureButtonGroup->checkedId());
	settings.setValue("browser/TileLabel", this->tileLabelButtonGroup->checkedId());
	settings.setValue("browser/TileZoom", this->tileZoomButtonGroup->checkedId());	

	QDialog::accept();
}

void CTile_browser_dlg::reject()
{
	int reply = QMessageBox::question(this, tr("Quit"), tr("Are you sure you want to Quit TileSet Edition without Saving?"), QMessageBox::Yes | QMessageBox::No);
	if (reply  == QMessageBox::Yes)
	{
		QDialog::reject();
	} 
}