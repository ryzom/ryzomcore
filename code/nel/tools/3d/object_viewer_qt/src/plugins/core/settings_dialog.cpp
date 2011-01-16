// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
// Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

#include "settings_dialog.h"

// Qt includes
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>

namespace Core
{

struct PageData
{
	int index;
	QString category;
	QString id;
};

Q_DECLARE_METATYPE(PageData);

CSettingsDialog::CSettingsDialog(ExtensionSystem::IPluginManager *pluginManager,
								 const QString &categoryId,
								 const QString &pageId,
								 QWidget *parent)
	: QDialog(parent),
	  _applied(false)
{
	_ui.setupUi(this);

	_plugMan = pluginManager;

	QString initialCategory = categoryId;
	QString initialPage = pageId;

	_ui.buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

	connect(_ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));

	_ui.splitter->setCollapsible(1, false);
	_ui.pageTree->header()->setVisible(false);

	connect(_ui.pageTree, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
			this, SLOT(pageSelected()));

	QMap<QString, QTreeWidgetItem *> categories;

	QList<IOptionsPage *> pages;
	QList<QObject *> all = _plugMan->allObjects();
	Q_FOREACH(QObject *obj, all)
	{
		IOptionsPage *page = dynamic_cast<IOptionsPage *>(obj);
		if (page)
			pages.append(page);
	}

	int index = 0;
	Q_FOREACH(IOptionsPage *page, pages)
	{
		PageData pageData;
		pageData.index = index;
		pageData.category = page->category();
		pageData.id = page->id();

		QTreeWidgetItem *item = new QTreeWidgetItem;
		item->setText(0, page->trName());
		item->setData(0, Qt::UserRole, qVariantFromValue(pageData));

		QStringList categoriesId = page->category().split(QLatin1Char('|'));
		QStringList trCategories = page->trCategory().split(QLatin1Char('|'));
		QString currentCategory = categoriesId.at(0);

		QTreeWidgetItem *treeitem;
		if (!categories.contains(currentCategory))
		{
			treeitem = new QTreeWidgetItem(_ui.pageTree);
			treeitem->setText(0, trCategories.at(0));
			treeitem->setData(0, Qt::UserRole, qVariantFromValue(pageData));
			categories.insert(currentCategory, treeitem);
		}

		int catCount = 1;
		while (catCount < categoriesId.count())
		{
			if (!categories.contains(currentCategory + QLatin1Char('|') + categoriesId.at(catCount)))
			{
				treeitem = new QTreeWidgetItem(categories.value(currentCategory));
				currentCategory +=  QLatin1Char('|') + categoriesId.at(catCount);
				treeitem->setText(0, trCategories.at(catCount));
				treeitem->setData(0, Qt::UserRole, qVariantFromValue(pageData));
				categories.insert(currentCategory, treeitem);
			}
			else
			{
				currentCategory +=  QLatin1Char('|') + categoriesId.at(catCount);
			}
			++catCount;
		}

		categories.value(currentCategory)->addChild(item);

		_pages.append(page);
		_ui.stackedPages->addWidget(page->createPage(_ui.stackedPages));

		if (page->id() == initialPage && currentCategory == initialCategory)
		{
			_ui.stackedPages->setCurrentIndex(_ui.stackedPages->count());
			_ui.pageTree->setCurrentItem(item);
		}

		index++;
	}

	QList<int> sizes;
	sizes << 150 << 300;
	_ui.splitter->setSizes(sizes);

	_ui.splitter->setStretchFactor(_ui.splitter->indexOf(_ui.pageTree), 0);
	_ui.splitter->setStretchFactor(_ui.splitter->indexOf(_ui.layoutWidget), 1);
}

CSettingsDialog::~CSettingsDialog()
{
}

void CSettingsDialog::pageSelected()
{
	QTreeWidgetItem *item = _ui.pageTree->currentItem();
	PageData data = item->data(0, Qt::UserRole).value<PageData>();
	int index = data.index;
	_currentCategory = data.category;
	_currentPage = data.id;
	_ui.stackedPages->setCurrentIndex(index);
}

void CSettingsDialog::accept()
{
	_applied = true;
	Q_FOREACH(IOptionsPage *page, _pages)
	{
		page->apply();
		page->finish();
	}
	done(QDialog::Accepted);
}

void CSettingsDialog::reject()
{
	Q_FOREACH(IOptionsPage *page, _pages)
	page->finish();
	done(QDialog::Rejected);
}

void CSettingsDialog::apply()
{
	Q_FOREACH(IOptionsPage *page, _pages)
	page->apply();
	_applied = true;
}

bool CSettingsDialog::execDialog()
{
	_applied = false;
	exec();
	return _applied;
}

void CSettingsDialog::done(int val)
{
	QDialog::done(val);
}

} /* namespace Core */