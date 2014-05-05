// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Emanuel Costea <cemycc@gmail.com>
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

#ifndef TRANSLATION_MANAGER_SETTINGS_PAGE_H
#define TRANSLATION_MANAGER_SETTINGS_PAGE_H

#include <QtCore/QObject>

#include "../core/ioptions_page.h"

#include "ui_translation_manager_settings_page.h"

namespace TranslationManager
{

class CTranslationManagerSettingsPage : public Core::IOptionsPage
{
	Q_OBJECT
public:
	CTranslationManagerSettingsPage(QObject *parent = 0);
	virtual ~CTranslationManagerSettingsPage() {}

	virtual QString id() const;
	virtual QString trName() const;
	virtual QString category() const;
	virtual QString trCategory() const;
	virtual QIcon categoryIcon() const;
	virtual QWidget *createPage(QWidget *parent);

	virtual void apply();
	virtual void finish() {}
private Q_SLOTS:
	void filterAdd();
	void filterDel();
	void languageAdd();
	void languageDel();
	void translationAdd();
	void workAdd();
private:
	QWidget *_currentPage;
	Ui::CTranslationManagerSettingsPage _ui;
	void writeSettings();
	void readSettings();
};

}

#endif // TRANSLATION_MANAGER_SETTINGS_H