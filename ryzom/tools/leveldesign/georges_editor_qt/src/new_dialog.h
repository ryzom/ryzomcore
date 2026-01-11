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

#ifndef NEW_DIALOG_H
#define NEW_DIALOG_H

// Qt includes
#include <QtGui/QWidget>

// STL includes

// NeL includes
#include <nel/misc/types_nl.h>
#include <nel/misc/file.h>

// Project includes
#include "ui_new_form.h"

namespace NLQT 
{
	class CGeorgesNewDialog: public QDialog
	{
		Q_OBJECT

		union TFormId
		{
			uint32		Id;
			struct
			{
				uint32	Type	: 8;
				uint32	Id		: 24;
			} FormIDInfos;
			void serial(NLMISC::IStream &f) { f.serial(Id); };
		};

		Ui::CGeorgesNewDialog _ui;
		QStringList           _filelist;
		QStringList          &_result;
		QString               _descriptionTemplate;

		std::map<std::string,TFormId> FormToId;
		std::map<std::string,uint8>   FileTypeToId;
		std::map<uint8,std::string>   IdToFileType;

		std::vector<std::string> _files;

		void   getTypes( /* std::string& dir // not needed anymore? */ );
		void   addType( std::string fileName );
		bool   getFileType( std::string& fileName, std::string& fileType );
		sint16 getFirstFreeFileTypeId();

	public:
		CGeorgesNewDialog(QStringList& result, QWidget *parent = 0);
		~CGeorgesNewDialog();

	private Q_SLOTS:
		void wizardBtnClicked(bool checked);
		void formBtnClicked  (bool checked);
		void dfnTypeClicked  (bool p_checked);
		void addParentClicked();
		void deleteParentClicked();
		void formItemActivated(QListWidgetItem *);
		void wizardItemActivated(QListWidgetItem *);
		void validateParentCombo();
		void buttonBoxAccepted();
		void buttonBoxRejected();

	friend class CMainWindow;
	}; /* CGeorgesNewDialog */

} /* namespace NLQT */

#endif // NEW_DIALOG_H
