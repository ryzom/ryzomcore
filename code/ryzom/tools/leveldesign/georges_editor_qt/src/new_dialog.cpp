/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "new_dialog.h"

// Qt includes
#include <QtGui/QWidget>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QMessageBox>

#include <QDebug>

// NeL includes
#include <nel/misc/path.h>

// Project includes
#include "modules.h"
#include "completer_line_edit.h"

namespace NLQT
{

	CGeorgesNewDialog::CGeorgesNewDialog(QStringList& result, QWidget *parent)
		: QDialog(parent),
		_result(result),
		_descriptionTemplate(QString())
	{
		_ui.setupUi(this);

		setWindowIcon(QIcon(":/images/georges_logo.png"));

		_ui.parentLineEdit->setEnabled(false);
		_ui.addParentButton->setEnabled(true);

		// wizard page
		connect(_ui.wizardBtn, SIGNAL(clicked(bool)),
			this, SLOT(wizardBtnClicked(bool)));
		connect(_ui.wizardList, SIGNAL(itemClicked(QListWidgetItem*)),
			this, SLOT(wizardItemActivated(QListWidgetItem *)));
		
		// form page
		connect(_ui.formBtn, SIGNAL(clicked(bool)),
			this, SLOT(formBtnClicked(bool)));
		connect(_ui.addParentButton, SIGNAL(clicked()),
			this, SLOT(addParentClicked()));
		connect(_ui.deleteParentButton, SIGNAL(clicked()),
			this, SLOT(deleteParentClicked()));
		connect(_ui.formList, SIGNAL(itemActivated(QListWidgetItem*)),
			this, SLOT(formItemActivated(QListWidgetItem *)));
		connect(_ui.formList, SIGNAL(itemClicked(QListWidgetItem*)),
			this, SLOT(formItemActivated(QListWidgetItem *)));
		connect(_ui.parentLineEdit, SIGNAL(editingFinished()),
			this, SLOT(validateParentCombo()));

		// dfn page
		connect(_ui.dfnTypeBtn, SIGNAL(clicked(bool)),
			this, SLOT(dfnTypeClicked(bool)));

		connect(_ui.buttonBox, SIGNAL(accepted()),
			this, SLOT(buttonBoxAccepted()));
		connect(_ui.buttonBox, SIGNAL(rejected()),
			this, SLOT(buttonBoxRejected()));

		// wizard list
		QListWidgetItem *mpWiz = new QListWidgetItem(QIcon(":/images/mp_generic.png"),tr("Raw Material Generator"));
		_ui.wizardList->addItem(mpWiz);

		// form list
		QString path = Modules::mainWin().leveldesignPath();
		QStringList typelist;
		//nlinfo ("Searching files in directory '%s'...", dir.c_str());
		NLMISC::CPath::getPathContent(path.toUtf8().constData(),true,false,true,_files);

		getTypes( /* path.toUtf8() // incompatible parameter type */ );
		//nlinfo ("%d supported file types :",FileTypeToId.size());
		for ( std::map<std::string,uint8>::iterator it = FileTypeToId.begin(); it != FileTypeToId.end(); ++it )
		{
			typelist.append(QString((*it).first.c_str()));
			//nlinfo("%s",(*it).first.c_str());
		}
		_ui.formList->addItems(typelist);

		for(uint i = 0; i < _files.size(); i++)
		{
			std::string extStr = NLMISC::CFile::getExtension( _files[i] );

			// filter files without existing dfn
			if (!NLMISC::CPath::exists(extStr + ".dfn") &&
				!NLMISC::CPath::exists(extStr + ".typ"))
			{
				continue;
			}
			_filelist.append(QString(NLMISC::CFile::getFilename(_files[i]).c_str()));
		}		

		_ui.parentFrame->hide();

		// replace "Heading" and "Descriptive Text" with your string
		_descriptionTemplate = 
			"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\"" 
			"\"http://www.w3.org/TR/REC-html40/strict.dtd\">"
			"\n<html><head><meta name=\"qrichtext\" content=\"1\" />"
			"<style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style>"
			"</head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">"
			"\n<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
			"<span style=\" font-size:8pt; font-weight:600;\">Heading</span></p>"
			"\n<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
			"<span style=\" font-size:8pt;\">Descriptive Text</span></p></body></html>";
	}

	CGeorgesNewDialog::~CGeorgesNewDialog() 
	{

	}

	void CGeorgesNewDialog::wizardBtnClicked(bool p_checked) 
	{
		if(p_checked)
			_ui.stackedWidget->setCurrentWidget(_ui.wizardPage);
		else
			_ui.wizardBtn->setChecked(true);
	}

	void CGeorgesNewDialog::formBtnClicked(bool p_checked) 
	{
		if(p_checked)
			_ui.stackedWidget->setCurrentWidget(_ui.formPage);
		else
			_ui.formBtn->setChecked(true);
	}

	void CGeorgesNewDialog::dfnTypeClicked(bool p_checked) 
	{
		if(p_checked)
			_ui.stackedWidget->setCurrentWidget(_ui.dfnTypePage);
		else
			_ui.dfnTypeBtn->setChecked(true);
	}

	void CGeorgesNewDialog::addParentClicked() 
	{
		if (!_filelist.contains(_ui.parentLineEdit->text()))
		{
			_ui.parentLineEdit->clear();
			return;
		}

		_ui.parentFrame->show();
		
		QList<QListWidgetItem *> itemList = _ui.parentList->
			findItems(_ui.parentLineEdit->text(), Qt::MatchFixedString);
		if ((itemList.count() == 0) && (!_ui.parentLineEdit->text().isEmpty()))
		{
			_ui.parentList->insertItem(_ui.parentList->count(), _ui.parentLineEdit->text());
		}
		_ui.parentLineEdit->clear();
	}

	void CGeorgesNewDialog::deleteParentClicked() 
	{
		_ui.parentList->takeItem(_ui.parentList->currentRow());

		if (_ui.parentList->count() == 0)
		{
			_ui.parentFrame->hide();
		}
	}

	void CGeorgesNewDialog::buttonBoxAccepted() 
	{
		if (_ui.stackedWidget->currentWidget() == _ui.formPage)
		{
			_result << _ui.formFileNameEdit->text();
			for (int i = 0; i < _ui.parentList->count(); i++)
			{
				_result << _ui.parentList->item(i)->text();
			}
		} 
		else 
		{
			QMessageBox::information(this,"Information","Not yet included.\nSoon to come! :)");
		}
	}

	void CGeorgesNewDialog::buttonBoxRejected() 
	{
		// TODO
	}

	void CGeorgesNewDialog::formItemActivated(QListWidgetItem *item)
	{
		_ui.formFileNameEdit->setText(QString(tr("newfile.%1").arg(item->text())));
		_ui.parentLineEdit->setEnabled(true);
		_ui.parentLineEdit->setText("");
		//_ui.addParentButton->setEnabled(false);
		
		QStringList list = _filelist.filter(item->text());
		_ui.parentLineEdit->setStringList(list);
		_ui.formFileNameEdit->setFocus();
		_ui.formFileNameEdit->setSelection(0, tr("newfile").size());
	}

	void CGeorgesNewDialog::wizardItemActivated(QListWidgetItem *item)
	{
		QString myDescription = _descriptionTemplate;
		myDescription = myDescription.replace("Heading", item->text());

		if (item->text() == tr("Raw Material Generator"))
		{
			myDescription = myDescription.replace("Descriptive Text", 
				tr("Automatically creates MP (resources) for every creature in the assets."));
		}
		
		_ui.wizDescLabel->setText(myDescription);
	}

	void CGeorgesNewDialog::getTypes( /* std::string& dir // not needed anymore? */ )
	{
		//nlinfo ("Found %d files in directory '%s'", files.size(), dir.c_str());
		for(uint i = 0; i < _files.size(); i++)
		{
			addType(NLMISC::CFile::getFilename(_files[i]));
			QApplication::processEvents();
		}
	}

	void CGeorgesNewDialog::addType( std::string fileName )
	{
		if(fileName.empty() || fileName=="." || fileName==".." || fileName[0]=='_' || fileName.find(".#")==0)
		{
			//nlinfo("Discarding file '%s'", fileName.c_str());
			return;
		}
		else
		{
			std::string extStr = NLMISC::CFile::getExtension( fileName );

			if (!NLMISC::CPath::exists(extStr + ".dfn"))
			{
				return;
			}
		}

		// if the file is new
		std::map<std::string,TFormId>::iterator itFI = FormToId.find( fileName );
		if( itFI == FormToId.end() )
		{
			// double check : if file not found we check with lower case version of filename
			std::map<std::string,TFormId>::iterator itFILwr = FormToId.find( NLMISC::toLower(fileName) );
			if( itFILwr != FormToId.end() )
			{
				nlwarning("Trying to add %s but the file %s is already known ! be careful with lower case and upper case.", fileName.c_str(), NLMISC::toLower(fileName).c_str());
				return;
			}

			std::string fileType;
			if( getFileType( fileName, fileType ) )
			{
				std::map<std::string,uint8>::iterator itFTI = FileTypeToId.find( fileType );
				TFormId fid;
				memset( &fid, 0, sizeof(TFormId) );

				// if the type of this file is a new type
				if( itFTI == FileTypeToId.end() )
				{
					sint16 firstFreeFileTypeId = getFirstFreeFileTypeId();
					if( firstFreeFileTypeId == -1 )
					{
						nlwarning("MORE THAN 256 FILE TYPES!!!!");
					}
					else
					{
						FileTypeToId.insert( std::make_pair(fileType,(uint8)firstFreeFileTypeId) );
						IdToFileType.insert( std::make_pair((uint8)firstFreeFileTypeId,fileType) );
						
						fid.FormIDInfos.Type = (uint8)firstFreeFileTypeId;
						fid.FormIDInfos.Id = 0;

						//nlinfo("Adding file type '%s' with id %d", fileType.c_str(), firstFreeFileTypeId);
					}
				}
				else
				{
					return;
				}
				FormToId.insert( make_pair(fileName,fid) );
				//nlinfo("Adding file '%s' id %d with type '%s' id %d", fileName.c_str(), fid.FormIDInfos.Id, fileType.c_str(), fid.FormIDInfos.Type);
			}
			else
			{
				nlinfo("Unknown file type for the file : '%s' --> not added",fileName.c_str());
			}
		}
		else
		{
			nlinfo("Skipping file '%s', already in the file", fileName.c_str());
		}
	}

	bool CGeorgesNewDialog::getFileType( std::string& fileName, std::string& fileType )
	{
		fileType = NLMISC::CFile::getExtension(NLMISC::CFile::getFilename(fileName));
		return !fileType.empty();
	} 

	sint16 CGeorgesNewDialog::getFirstFreeFileTypeId()
	{
		for( sint16 id=0; id<256; ++id )
		{
			if( IdToFileType.find((uint8)id) == IdToFileType.end() )
			{
				return id;
			}
		}
		return -1;
	}

	void CGeorgesNewDialog::validateParentCombo()
	{
		// TODO: clear if no valid text
		//if (!_filelist.contains(_ui.parentLineEdit->text()))
		//	_ui.parentLineEdit->clear();
	}
} /* namespace NLQT */