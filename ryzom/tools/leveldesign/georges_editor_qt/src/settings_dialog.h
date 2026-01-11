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

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <nel/misc/types_nl.h>
#include "ui_settings_form.h"

// STL includes

// NeL includes
#include <nel/misc/config_file.h>

// Project includes

namespace NLQT
{

	class CSettingsDialog: public QDialog
	{
		Q_OBJECT

	public:
		CSettingsDialog(QWidget *parent = 0);
		~CSettingsDialog();

	Q_SIGNALS:
		void ldPathChanged(QString);

	private Q_SLOTS:
		void addPath();
		void removePath();
		void upPath();
		void downPath();
		void applyPressed();
		void browseLeveldesignPath();

	private:

		Ui::CSettingsDialog ui;

	}; /* class CSettingsDialog */

} /* namespace NLQT */

#endif // SETTINGS_DIALOG_H
