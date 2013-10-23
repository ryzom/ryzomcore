// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
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

#include "translation_manager_settings_page.h"

// Core includes
#include "../core/icore.h"

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QListWidgetItem>

namespace TranslationManager
{

QString lastDir = ".";

CTranslationManagerSettingsPage::CTranslationManagerSettingsPage(QObject *parent)
	: IOptionsPage(parent),
	  _currentPage(NULL)
{
}

QString CTranslationManagerSettingsPage::id() const
{
	return QLatin1String("TranslationManagerPage");
}

QString CTranslationManagerSettingsPage::trName() const
{
	return tr("Translation Manager");
}

QString CTranslationManagerSettingsPage::category() const
{
	return QLatin1String("Translation Manager");
}

QString CTranslationManagerSettingsPage::trCategory() const
{
	return tr("Translation Manager");
}

QIcon CTranslationManagerSettingsPage::categoryIcon() const
{
	return QIcon();
}

QWidget *CTranslationManagerSettingsPage::createPage(QWidget *parent)
{
	_currentPage = new QWidget(parent);
	_ui.setupUi(_currentPage);
	readSettings();
	connect(_ui.filter_add, SIGNAL(clicked()), this, SLOT(filterAdd()));
	connect(_ui.filter_del, SIGNAL(clicked()), this, SLOT(filterDel()));
	connect(_ui.lang_add, SIGNAL(clicked()), this, SLOT(languageAdd()));
	connect(_ui.lang_del, SIGNAL(clicked()), this, SLOT(languageDel()));
	connect(_ui.translation_add, SIGNAL(clicked()), this, SLOT(translationAdd()));
	connect(_ui.work_add, SIGNAL(clicked()), this, SLOT(workAdd()));

	return _currentPage;
}

void CTranslationManagerSettingsPage::filterAdd()
{
	QString newValue = _ui.filter_edit->text();
	if (!newValue.isEmpty())
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(newValue);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.filter_list->addItem(newItem);
	}
}

void CTranslationManagerSettingsPage::filterDel()
{
	QListWidgetItem *removeItem = _ui.filter_list->takeItem(_ui.filter_list->currentRow());
	if (!removeItem)
		delete removeItem;
}

void CTranslationManagerSettingsPage::languageAdd()
{
	QString newValue = _ui.lang_edit->text();
	if (!newValue.isEmpty())
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(newValue);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.lang_list->addItem(newItem);
	}
}

void CTranslationManagerSettingsPage::languageDel()
{
	QListWidgetItem *removeItem = _ui.lang_list->takeItem(_ui.lang_list->currentRow());
	if (!removeItem)
		delete removeItem;
}

void CTranslationManagerSettingsPage::translationAdd()
{
	QString newPath = QFileDialog::getExistingDirectory(_currentPage, "");
	if (!newPath.isEmpty())
	{
		_ui.translation_edit->setText(newPath);
	}
}

void CTranslationManagerSettingsPage::workAdd()
{
	QString newPath = QFileDialog::getExistingDirectory(_currentPage, "");
	if (!newPath.isEmpty())
	{
		_ui.work_edit->setText(newPath);
	}
}

void CTranslationManagerSettingsPage::apply()
{
	writeSettings();
}

void CTranslationManagerSettingsPage::readSettings()
{
	QStringList filters, languages;
	QString ligo, translation, work;

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("translationmanager");

	filters = settings->value("filters").toStringList(); /* filters */
	languages = settings->value("trlanguages").toStringList(); /* languages */
	ligo = settings->value("ligo").toString();
	translation = settings->value("translation").toString();
	work = settings->value("work").toString();

	settings->endGroup();
	// filter
	Q_FOREACH(QString filter, filters)
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(filter);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.filter_list->addItem(newItem);
	}
	// languages
	Q_FOREACH(QString lang, languages)
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(lang);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.lang_list->addItem(newItem);
	}
	// translation
	_ui.translation_edit->setText(translation);
	// work
	_ui.work_edit->setText(work);

}

void CTranslationManagerSettingsPage::writeSettings()
{
	QStringList filters, languages;
	QString ligo, translation, work;
	// filters
	for (int i = 0; i < _ui.filter_list->count(); ++i)
		filters << _ui.filter_list->item(i)->text();
	// languages
	for (int i = 0; i < _ui.lang_list->count(); ++i)
		languages << _ui.lang_list->item(i)->text();
	// translations path
	translation = _ui.translation_edit->text();
	// work path
	work = _ui.work_edit->text();

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("translationmanager");
	settings->setValue("filters", filters);
	settings->setValue("trlanguages", languages);
	settings->setValue("translation", translation);
	settings->setValue("work", work);
	settings->endGroup();
	settings->sync();
}

}