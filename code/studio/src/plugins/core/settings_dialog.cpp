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

// Project includes
#include "settings_dialog.h"
#include "ioptions_page.h"

// Qt includes
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>

struct PageData
{
	int index;
	QString category;
	QString id;
};

Q_DECLARE_METATYPE(PageData);

namespace Core
{
SettingsDialog::SettingsDialog(ExtensionSystem::IPluginManager *pluginManager,
							   const QString &categoryId,
							   const QString &pageId,
							   QWidget *parent)
	: QDialog(parent),
	  m_applied(false)
{
	m_ui.setupUi(this);

	m_plugMan = pluginManager;

	QString initialCategory = categoryId;
	QString initialPage = pageId;

	m_ui.buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

	connect(m_ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));

	m_ui.splitter->setCollapsible(1, false);
	m_ui.pageTree->header()->setVisible(false);

	connect(m_ui.pageTree, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
			this, SLOT(pageSelected()));

	QMap<QString, QTreeWidgetItem *> categories;

	QList<IOptionsPage *> pages = m_plugMan->getObjects<IOptionsPage>();

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
			treeitem = new QTreeWidgetItem(m_ui.pageTree);
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

		m_pages.append(page);
		m_ui.stackedPages->addWidget(page->createPage(m_ui.stackedPages));

		if (page->id() == initialPage && currentCategory == initialCategory)
		{
			m_ui.stackedPages->setCurrentIndex(m_ui.stackedPages->count());
			m_ui.pageTree->setCurrentItem(item);
		}

		index++;
	}

	QList<int> sizes;
	sizes << 150 << 300;
	m_ui.splitter->setSizes(sizes);

	m_ui.splitter->setStretchFactor(m_ui.splitter->indexOf(m_ui.pageTree), 0);
	m_ui.splitter->setStretchFactor(m_ui.splitter->indexOf(m_ui.layoutWidget), 1);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::pageSelected()
{
	QTreeWidgetItem *item = m_ui.pageTree->currentItem();
	PageData data = item->data(0, Qt::UserRole).value<PageData>();
	int index = data.index;
	m_currentCategory = data.category;
	m_currentPage = data.id;
	m_ui.stackedPages->setCurrentIndex(index);
}

void SettingsDialog::accept()
{
	m_applied = true;
	Q_FOREACH(IOptionsPage *page, m_pages)
	{
		page->apply();
		page->finish();
	}
	done(QDialog::Accepted);
}

void SettingsDialog::reject()
{
	Q_FOREACH(IOptionsPage *page, m_pages)
	page->finish();
	done(QDialog::Rejected);
}

void SettingsDialog::apply()
{
	Q_FOREACH(IOptionsPage *page, m_pages)
	page->apply();
	m_applied = true;
}

bool SettingsDialog::execDialog()
{
	m_applied = false;
	exec();
	return m_applied;
}

void SettingsDialog::done(int val)
{
	QDialog::done(val);
}

} /* namespace Core */