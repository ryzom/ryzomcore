// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QListWidgetItem>

// NeL includes

// Project includes
#include "../core/icore.h"

namespace Plugin
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
	return tr("Translation Manager page");
}

QString CTranslationManagerSettingsPage::category() const
{
	return QLatin1String("General");
}

QString CTranslationManagerSettingsPage::trCategory() const
{
	return tr("General");
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
	connect(_ui.paths_add, SIGNAL(clicked()), this, SLOT(pathAdd()));
	connect(_ui.paths_del, SIGNAL(clicked()), this, SLOT(pathDel()));
	connect(_ui.pathsR_add, SIGNAL(clicked()), this, SLOT(pathRAdd()));
	connect(_ui.pathsR_del, SIGNAL(clicked()), this, SLOT(pathRDel()));
	connect(_ui.georges_add, SIGNAL(clicked()), this, SLOT(georgeAdd()));
	connect(_ui.georges_del, SIGNAL(clicked()), this, SLOT(georgeDel()));
	connect(_ui.filter_add, SIGNAL(clicked()), this, SLOT(filterAdd()));
	connect(_ui.filter_del, SIGNAL(clicked()), this, SLOT(filterDel()));
	connect(_ui.lang_add, SIGNAL(clicked()), this, SLOT(languageAdd()));
	connect(_ui.lang_del, SIGNAL(clicked()), this, SLOT(languageDel()));
        connect(_ui.translation_add, SIGNAL(clicked()), this, SLOT(translationAdd()));
        connect(_ui.work_add, SIGNAL(clicked()), this, SLOT(workAdd()));
        
	return _currentPage;
}

void CTranslationManagerSettingsPage::pathAdd()
{
        QString newPath = QFileDialog::getExistingDirectory(_currentPage, "", lastDir);
	if (!newPath.isEmpty())
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(newPath);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.paths_list->addItem(newItem);
		lastDir = newPath;
	}
}

void CTranslationManagerSettingsPage::pathDel()
{
	QListWidgetItem *removeItem = _ui.paths_list->takeItem(_ui.paths_list->currentRow());
	if (!removeItem)
		delete removeItem;    
}

void CTranslationManagerSettingsPage::pathRAdd()
{
        QString newPath = QFileDialog::getExistingDirectory(_currentPage, "", lastDir);
	if (!newPath.isEmpty())
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(newPath);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.pathsR_list->addItem(newItem);
		lastDir = newPath;
	}
}

void CTranslationManagerSettingsPage::pathRDel()
{
	QListWidgetItem *removeItem = _ui.pathsR_list->takeItem(_ui.pathsR_list->currentRow());
	if (!removeItem)
		delete removeItem;    
}

void CTranslationManagerSettingsPage::georgeAdd()
{
        QString newPath = QFileDialog::getExistingDirectory(_currentPage, "", lastDir);
	if (!newPath.isEmpty())
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(newPath);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.georges_list->addItem(newItem);
		lastDir = newPath;
	}
}

void CTranslationManagerSettingsPage::georgeDel()
{
	QListWidgetItem *removeItem = _ui.georges_list->takeItem(_ui.georges_list->currentRow());
	if (!removeItem)
		delete removeItem;    
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
	QStringList paths, pathsR, georges, filters, languages;
        QString ligo, translation, work;
        
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("translationmanager");
        
	paths = settings->value("paths").toStringList(); /* paths */
        pathsR = settings->value("pathsR").toStringList(); /* pathsR */
        georges = settings->value("georges").toStringList(); /* georges */
        filters = settings->value("filters").toStringList(); /* filters */
        languages = settings->value("languages").toStringList(); /* languages */
        ligo = settings->value("ligo").toString();
        translation = settings->value("translation").toString();
        work = settings->value("work").toString();
        
	settings->endGroup();
        /* paths */
	Q_FOREACH(QString path, paths)
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(path);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.paths_list->addItem(newItem);
	}
        /* pathsR */
	Q_FOREACH(QString pathR, pathsR)
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(pathR);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.pathsR_list->addItem(newItem);
	}
        /* georges */
	Q_FOREACH(QString george, georges)
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(george);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.georges_list->addItem(newItem);
	}
        /* filter */
	Q_FOREACH(QString filter, filters)
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(filter);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.filter_list->addItem(newItem);
	}  
        /* languages */
	Q_FOREACH(QString lang, languages)
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(lang);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		_ui.lang_list->addItem(newItem);
	}  
        /* ligo */
        _ui.ligo_edit->setText(ligo);
        /* translation */
        _ui.translation_edit->setText(translation);
        /* work */
        _ui.work_edit->setText(work);
        
}

void CTranslationManagerSettingsPage::writeSettings()
{
	QStringList paths, pathsR, georges, filters, languages;
        QString ligo, translation, work;
        /* paths */
	for (int i = 0; i < _ui.paths_list->count(); ++i)
		paths << _ui.paths_list->item(i)->text();
        /* pathsR */
	for (int i = 0; i < _ui.pathsR_list->count(); ++i)
		pathsR << _ui.pathsR_list->item(i)->text();
        /* georges */
	for (int i = 0; i < _ui.georges_list->count(); ++i)
		georges << _ui.georges_list->item(i)->text();
        /* filters */
	for (int i = 0; i < _ui.filter_list->count(); ++i)
		filters << _ui.filter_list->item(i)->text();
        /* languages */
	for (int i = 0; i < _ui.lang_list->count(); ++i)
		languages << _ui.lang_list->item(i)->text();        
        /* ligo path */
        ligo = _ui.ligo_edit->text();
        /* translations path*/
        translation = _ui.translation_edit->text();
        work = _ui.work_edit->text();
        
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("translationmanager");
	settings->setValue("paths", paths);
        settings->setValue("pathsR", pathsR);
        settings->setValue("georges", georges);
        settings->setValue("filters", filters);
        settings->setValue("languages", languages);
        settings->setValue("ligo", ligo);
        settings->setValue("translation", translation);
        settings->setValue("work", work);
	settings->endGroup();
}


} /* namespace Plugin */
