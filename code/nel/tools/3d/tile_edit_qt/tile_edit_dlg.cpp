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

#include "common.h"
#include "tile_edit_dlg.h"
#include "items_edit_dlg.h"
#include "tile_browser_dlg.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;

CTileBank tileBank;
CTileBank tileBankBrowser;
bool tileSetSelectionChanging = false;

bool CheckPath (const std::string& path, const char* absolutePathToRemplace)
{
	// Look for absolute path in path
	if (strnicmp (path.c_str(), absolutePathToRemplace, strlen (absolutePathToRemplace))==0)
		return true;
	else
		return false;
}

bool RemovePath (std::string& path, const char* absolutePathToRemplace)
{
	// Look for absolute path in path
	if (strnicmp (path.c_str(), absolutePathToRemplace, strlen (absolutePathToRemplace))==0)
	{
		// New path
		std::string toto=path;
		path=toto.c_str()+strlen (absolutePathToRemplace);
		return true;
	}
	else
		return false;
}


 CTile_edit_dlg::CTile_edit_dlg(QWidget *parent)
     : QMainWindow(parent)

 {
	 ui.setupUi(this);
     initDialog();
 }



void CTile_edit_dlg::initDialog()
{
	ui.surfaceDataLineEdit->setValidator( new QIntValidator( ui.surfaceDataLineEdit ) );
}


void CTile_edit_dlg::on_landListWidget_itemSelectionChanged()
{
	int nindex = ui.landListWidget->currentRow();
	if (nindex != -1) 
	{
		ui.editLandPushButton->setEnabled(true);
		ui.deleteLandPushButton->setEnabled(true);
	}
	else
	{
		ui.editLandPushButton->setEnabled(false);
		ui.deleteLandPushButton->setEnabled(false);
	}
}



void CTile_edit_dlg::on_addLandPushButton_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add Land"), tr("Enter land name:"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
	{
		if (ui.landListWidget->findItems(text, Qt::MatchExactly).count() > 0)
		{
			QMessageBox::information( this, tr("Error Adding Land"), tr("This name already exists") );
		}
		else
		{
			tileBank.addLand( text.toUtf8().constData() );

			ui.landListWidget->addItem(text);
			ui.landListWidget->setCurrentRow(ui.landListWidget->count() - 1);
		}
	}
}
void CTile_edit_dlg::on_editLandPushButton_clicked()
{
	int nindex = ui.landListWidget->currentRow();
	if (nindex != -1) 
	{
		QStringList availableTileSetList;
		QStringList landTileSetList;
		for (int i=0; i<tileBank.getTileSetCount(); i++)
		{
			if (tileBank.getLand(nindex)->isTileSet (tileBank.getTileSet(i)->getName()))
				landTileSetList.append( QString( tileBank.getTileSet(i)->getName().c_str() ) );
			else
				availableTileSetList.append( QString( tileBank.getTileSet(i)->getName().c_str() ) );
		}

		bool ok = false;
		QStringList items = CItems_edit_dlg::getItems(this, tr("Edit Land"), ui.landListWidget->item(nindex)->text(), availableTileSetList, landTileSetList, &ok);

		if (ok)
		{
			for (int i=0; i<tileBank.getTileSetCount(); i++)
			{
				// remove tile set
				tileBank.getLand(nindex)->removeTileSet (tileBank.getTileSet(i)->getName());
			}
			for (int i=0; i<items.count(); i++)
			{
				QString rString = items[i];
				tileBank.getLand(nindex)->addTileSet( rString.toUtf8().constData() );
			}
		}
	}
	else
	{
		QMessageBox::information( this, tr("No Land Selected"), tr("Please, select the Land to edit first ...") );
	}
}


void CTile_edit_dlg::on_deleteLandPushButton_clicked()
{
	int nindex = ui.landListWidget->currentRow();
	if (nindex != -1) 
	{
		tileBank.removeLand (nindex);
		QListWidgetItem* item = ui.landListWidget->takeItem(nindex);
		delete item;
	}
	else
	{
		QMessageBox::information( this, tr("No Land Selected"), tr("Please, select the Land to delete first ...") );
	}
}

void CTile_edit_dlg::on_addTileSetPushButton_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add Tile Set"), tr("Enter Tile Set name:"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
	{
		if (ui.tileSetListWidget->findItems(text, Qt::MatchExactly).count() > 0)
		{
			QMessageBox::information( this, tr("Error Adding Tile Set"), tr("This name already exists") );
		}
		else
		{
			tileBank.addTileSet( text.toUtf8().constData() );

			ui.tileSetListWidget->addItem(text);
			ui.tileSetListWidget->setCurrentRow(ui.tileSetListWidget->count() - 1);
		}
	}
}
void CTile_edit_dlg::on_editTileSetPushButton_clicked()
{
	int nindex = ui.tileSetListWidget->currentRow();
	if (nindex != -1) 
	{
		tileBankBrowser=tileBank;
		CTile_browser_dlg *tileBrowser = new CTile_browser_dlg((QWidget*)this,(Qt::WindowFlags)0);
		tileBrowser->initDialog(nindex);
		if ( tileBrowser->exec() )
		{
			tileBank = tileBankBrowser;
		}
	}
	else
	{
		QMessageBox::information( this, tr("No Tile Set Selected"), tr("Please, select a Tile Set to edit first ...") );
	}
}
void CTile_edit_dlg::on_deleteTileSetPushButton_clicked()
{
	int nindex = ui.tileSetListWidget->currentRow();
	if (nindex != -1) 
	{

		//WORKAROUND : Qt 4.4 Behaviour (when removing at row 0 and currentRow equals to 0, currentRow is set to 1, it mess with the selectionChanged event
		if (nindex == 0)
		{
			if ( ui.tileSetListWidget->count() > 1)
				ui.tileSetListWidget->setCurrentRow(1);
			else
				ui.tileSetListWidget->setCurrentRow(-1);
		}
		//

		tileBank.removeTileSet( nindex );

		QListWidgetItem* item = ui.tileSetListWidget->takeItem(nindex);
		delete item;
	}
	else
	{
		QMessageBox::information( this, tr("No Tile Set Selected"), tr("Please, select a Tile Set to delete first ...") );
	}
}
void CTile_edit_dlg::on_chooseVegetPushButton_clicked()
{
	int nindex = ui.tileSetListWidget->currentRow();
	if (nindex != -1) 
	{
		QFileDialog::Options options;
		QString selectedFilter;
		QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Veget Set"), ui.chooseVegetPushButton->text() , tr("NeL VegetSet Files (*.vegetset);;All Files (*.*);;"), &selectedFilter, options);
		
		if (!fileName.isEmpty())
		{
			QFileInfo fi(fileName);
			tileBank.getTileSet (nindex)->setTileVegetableDescFileName (fi.fileName().toUtf8().constData());
			ui.chooseVegetPushButton->setText(fi.fileName());
		}
	}
	else
	{
		QMessageBox::information( this, tr("No Tile Set Selected"), tr("Please, select a Tile Set first ...") );
	}
}
void CTile_edit_dlg::on_resetVegetPushButton_clicked()
{
	int nindex = ui.tileSetListWidget->currentRow();
	if (nindex != -1) 
	{
		tileBank.getTileSet (nindex)->setTileVegetableDescFileName ("");
		ui.chooseVegetPushButton->setText("...");
	}
	else
	{
		QMessageBox::information( this, tr("No Tile Set Selected"), tr("Please, select a Tile Set first ...") );
	}
}

void CTile_edit_dlg::on_surfaceDataLineEdit_textChanged()
{
	if (!tileSetSelectionChanging)
	{
		int nindex = ui.tileSetListWidget->currentRow();
		if (nindex != -1) 
		{
			bool ok;
			uint intValue = ui.surfaceDataLineEdit->text().toUInt( &ok, 10 );
			if (ok)
			{
				tileBank.getTileSet (nindex)->SurfaceData = (uint32)intValue;
			}
		}
		else
		{
			QMessageBox::information( this, tr("No Tile Set Selected"), tr("Please, select a Tile Set first ...") );
		}
	}
}

void CTile_edit_dlg::on_orientedCheckBox_stateChanged ( int state )
{
	if (!tileSetSelectionChanging)
	{
		int nindex = ui.tileSetListWidget->currentRow();
		if (nindex != -1) 
		{
			tileBank.getTileSet (nindex)->setOriented(ui.orientedCheckBox->isChecked());
		}
		else
		{
			QMessageBox::information( this, tr("No Tile Set Selected"), tr("Please, select a Tile Set first ...") );
		}
	}
}


void CTile_edit_dlg::on_tileSetListWidget_itemSelectionChanged()
{
	tileSetSelectionChanging = true;

	int nindex = ui.tileSetListWidget->currentRow();
	if (nindex != -1) 
	{
		if ( !tileBank.getTileSet(nindex)->getTileVegetableDescFileName().empty() )
			ui.chooseVegetPushButton->setText( QString( tileBank.getTileSet(nindex)->getTileVegetableDescFileName().c_str() ) );
		else
			ui.chooseVegetPushButton->setText("...");

		ui.surfaceDataLineEdit->setText( QString::number( tileBank.getTileSet(nindex)->SurfaceData) );
		ui.orientedCheckBox->setChecked( tileBank.getTileSet(nindex)->getOriented() );
		
		ui.selectedTileSetGroupBox->setEnabled(true);
		ui.editTileSetPushButton->setEnabled(true);
		ui.deleteTileSetPushButton->setEnabled(true);
	}
	else
	{
		//TODO titegus: Init DetailFrame Method
		ui.chooseVegetPushButton->setText("...");
		ui.surfaceDataLineEdit->clear();
		ui.orientedCheckBox->setChecked(false);

		ui.selectedTileSetGroupBox->setEnabled(false);
		ui.editTileSetPushButton->setEnabled(false);
		ui.deleteTileSetPushButton->setEnabled(false);
	}

	tileSetSelectionChanging = false;
}

void CTile_edit_dlg::on_quitPushButton_clicked()
{
	close();
}
void CTile_edit_dlg::on_loadPushButton_clicked()
{
	QFileDialog::Options options;
	QString selectedFilter;
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Bank"), ui.absolutePathPushButton->text() , tr("NeL tile bank files (*.bank);;All Files (*.*);;"), &selectedFilter, options);
	
	if (!fileName.isEmpty())
	{
		CIFile stream;
		if ( stream.open( fileName.toUtf8().constData() ) )
		{
			ui.landListWidget->clear();
			ui.tileSetListWidget->clear();
			tileBank.clear();
			tileBank.serial (stream);
		}
		
		int i;
		QStringList lands;
		for (i=0; i<tileBank.getLandCount(); i++)
		{
			// Add to the list
			lands.append( QString(tileBank.getLand(i)->getName().c_str()) );
		}
		ui.landListWidget->addItems(lands);


		QStringList tileSets;
		for (i=0; i<tileBank.getTileSetCount(); i++)
		{
			// Add to the list
			tileSets.append( QString( tileBank.getTileSet(i)->getName().c_str() ) );
		}
		ui.tileSetListWidget->addItems(tileSets);

		// Set MainFile
		mainFile = QFileInfo(fileName);

		ui.savePushButton->setEnabled(true);
		ui.absolutePathPushButton->setText( QString( tileBank.getAbsPath().c_str() ) );
	}
}

void CTile_edit_dlg::on_savePushButton_clicked()
{
	string fullPath = this->mainFile.absoluteFilePath().toUtf8().constData();
	if ( !fullPath.empty() )
	{
		COFile stream;
		if ( stream.open( fullPath.c_str() ) )
		{
			tileBank.serial (stream);
			QString s = tr("Bank %1 saved").arg( QString( fullPath.c_str() ) );
			QMessageBox::information(this, tr("Bank Saved"), s);
			return;
		}
	}

	QMessageBox::information(this, tr("Error"), tr("Can't Save Bank, check the path"));
}

void CTile_edit_dlg::on_saveAsPushButton_clicked()
{
	QFileDialog::Options options;
	QString selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Bank"), this->mainFile.absoluteFilePath(), tr("NeL tile bank files (*.bank);;All Files (*.*);;"), &selectedFilter, options);
	if (!fileName.isEmpty())
	{
		// Set MainFile
		mainFile = QFileInfo(fileName);
		ui.savePushButton->setEnabled(true);


		string fullPath = this->mainFile.absoluteFilePath().toUtf8().constData();
		if ( !fullPath.empty() )
		{
			COFile stream;
			if ( stream.open( fullPath.c_str() ) )
			{
				tileBank.serial (stream);
				QString s = tr("Bank %1 saved").arg( QString( fullPath.c_str() ) );
				QMessageBox::information(this, tr("Bank Saved"), s);
				return;
			}
		}

		QMessageBox::information(this, tr("Error"), tr("Can't Save Bank, check the path"));

	}

}


void CTile_edit_dlg::on_exportPushButton_clicked()
{
	QFileDialog::Options options;
	QString selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export Bank"), this->mainFile.absolutePath() + QDir::separator() + tr("*.smallbank"), tr("NeL tile small bank files (*.smallbank);;All Files (*.*);;"), &selectedFilter, options);
	if (!fileName.isEmpty())
	{
		// Copy the bank
		CTileBank copy=tileBank;

		// Remove unused data
		copy.cleanUnusedData ();

		QFileInfo fileInfo(fileName);
		string fullPath = fileInfo.absoluteFilePath().toUtf8().constData();
		if ( !fullPath.empty() )
		{
			COFile stream;
			if ( stream.open( fullPath.c_str() ) )
			{
				copy.serial (stream);
				QString s = tr("Bank %1 exported").arg( QString( fullPath.c_str() ) );
				QMessageBox::information(this, tr("Bank Saved"), s);
				return;
			}
		}

		QMessageBox::information(this, tr("Error"), tr("Can't Export the Bank, check the path"));
	}
}

void CTile_edit_dlg::on_absolutePathPushButton_clicked()
{
	// Build the struct
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select the absolute base path of the bank"), ui.absolutePathPushButton->text(), options);

	// Select the path
	if (!directory.isEmpty())
	{
		// Convert item into path string
		QString path = QDir::toNativeSeparators(directory);

		// Add a final back slash
		if (!path.endsWith(QDir::separator()))
		{
			// Add a '\' at the end
			path.append(QDir::separator());
		}

		//// Last check
		QMessageBox::StandardButton reply;
		QString confirmMessage = tr("Do you really want to set %1 as base path of the bank?").arg(path);
		reply = QMessageBox::question(this, tr("Confirm Path"), confirmMessage, QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::Yes)
		{
			// Set as default path..

			// Old path
			const char* oldPath=tileBank.getAbsPath ().c_str();

			// Path are good
			bool goodPath=true;

			// If no absolute path, must check before use it
			if ((*oldPath)==0)
			{
				// Compute xref
				tileBank.computeXRef();

				// For all tiles, check we can change the path
				for (int tiles=0; tiles<tileBank.getTileCount(); tiles++)
				{
					// Get tile xref
					int tileSet;
					int number;
					CTileBank::TTileType type;
					tileBank.getTileXRef (tiles, tileSet, number, type);

					// Is tile used ?
					if (tileSet!=-1)
					{
						// 3 types of bitmaps
						int type;
						for (type=CTile::diffuse; type<CTile::bitmapCount; type++)
						{
							// Bitmap string
							const std::string& bitmapPath=tileBank.getTile(tiles)->getRelativeFileName ((CTile::TBitmap)type);

							// not empty ?
							if (!bitmapPath.empty())
							{
								// Check the path
								if ( CheckPath( bitmapPath, path.toUtf8() ) == false )
								{
									// Bad path
									goodPath=false;

									// Message
									QString continueMessage = tr("Path '%1' can't be found in bitmap '%2'. Continue ?").arg(path).arg(QString(bitmapPath.c_str()));
									reply = QMessageBox::question(this, tr("Continue"), continueMessage, QMessageBox::Yes | QMessageBox::No);
									if (reply  == QMessageBox::No)
										break;
								}
							}
						}
						if (type!=CTile::bitmapCount)
							break;
					}
				}

				// For all tiles, check we can change the path
				for (uint noise=1; noise<tileBank.getDisplacementMapCount (); noise++)
				{
					// Bitmap string
					const char *bitmapPath=tileBank.getDisplacementMap (noise);

					// not empty ?
					if (strcmp (bitmapPath, "")!=0)
					{
						// Check the path
						if (CheckPath( bitmapPath, path.toUtf8() )==false)
						{
							// Bad path
							goodPath=false;

							// Message
							QString continueMessage = tr("Path '%1' can't be found in bitmap '%2'. Continue ?").arg(path).arg(QString(bitmapPath));
							reply = QMessageBox::question(this, tr("Continue"), continueMessage, QMessageBox::Yes | QMessageBox::No);
							if (reply  == QMessageBox::No)
								break;
						}
					}
				}

				// Good path ?
				if (goodPath)
				{
					// Ok change the path

					// For all tiles, check we can change the path
					for (int tiles=0; tiles<tileBank.getTileCount(); tiles++)
					{
						// Get tile xref
						int tileSet;
						int number;
						CTileBank::TTileType type;
						tileBank.getTileXRef (tiles, tileSet, number, type);

						// Is tile used ?
						if (tileSet!=-1)
						{
							// 3 types of bitmaps
							for (int type=CTile::diffuse; type<CTile::bitmapCount; type++)
							{
								// Bitmap string
								std::string bitmapPath=tileBank.getTile(tiles)->getRelativeFileName ((CTile::TBitmap)type);

								// not empty ?
								if (!bitmapPath.empty())
								{
									// Remove the absolute path
									bool res=RemovePath (bitmapPath, path.toUtf8());
									nlassert (res);

									// Set the bitmap
									tileBank.getTile(tiles)->setFileName ((CTile::TBitmap)type, bitmapPath);
								}
							}
						}	
					}

					// For all tiles, check we can change the path
					for (uint noise=1; noise<tileBank.getDisplacementMapCount (); noise++)
					{
						// Bitmap string
						std::string bitmapPath=tileBank.getDisplacementMap (noise);

						// not empty ?
						if (!bitmapPath.empty())
						{
							// Remove the absolute path
							bool res=RemovePath (bitmapPath, path.toUtf8());
							nlassert (res);

							// Set the bitmap
							tileBank.setDisplacementMap (noise, bitmapPath.c_str());
						}
					}
				}
				else
				{
					// Info message
					QMessageBox::information(this, tr("Error"), tr("Can't set the path."));
				}
			}


			// Good path ?
			if (goodPath)
			{
				// Change the abs path of the bank
				tileBank.setAbsPath (path.toUtf8().constData());

				// Change the bouton text
				ui.absolutePathPushButton->setText(path);
			}
		}

		// Remove path from all tiles
		//tileBank
	}
}


void CTile_edit_dlg::on_downPushButton_clicked()
{
	int nindex = ui.tileSetListWidget->currentRow();
	if (nindex != -1) 
	{
		if (nindex < (ui.tileSetListWidget->count() - 1) )
		{
			tileBank.xchgTileset (nindex, nindex+1);
			QListWidgetItem* item = ui.tileSetListWidget->takeItem(nindex);
			ui.tileSetListWidget->insertItem(nindex + 1, item);

			ui.tileSetListWidget->setCurrentRow(nindex + 1);
		}
	}
	else
	{
		QMessageBox::information( this, tr("No Tile Set Selected"), tr("Please, select a Tile Set first ...") );
	}
}

void CTile_edit_dlg::on_upPushButton_clicked()
{
	int nindex = ui.tileSetListWidget->currentRow();
	if (nindex != -1) 
	{
		if (nindex > 0 )
		{
			tileBank.xchgTileset (nindex, nindex-1);
			QListWidgetItem* item = ui.tileSetListWidget->takeItem(nindex);
			ui.tileSetListWidget->insertItem(nindex - 1, item);

			ui.tileSetListWidget->setCurrentRow(nindex - 1);
		}
	}
	else
	{
		QMessageBox::information( this, tr("No Tile Set Selected"), tr("Please, select a Tile Set first ...") );
	}
}

 void CTile_edit_dlg::closeEvent(QCloseEvent *event)
 {
	int reply = QMessageBox::question(this, tr("Quit"), tr("Are you sure you want to quit?"), QMessageBox::Yes | QMessageBox::No);
	if (reply  == QMessageBox::Yes)
	{
		event->accept();
	} 
	else 
	{
		event->ignore();
	}
 }
