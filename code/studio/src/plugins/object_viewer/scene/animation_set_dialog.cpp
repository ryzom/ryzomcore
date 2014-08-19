/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

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

#include "stdpch.h"
#include "animation_set_dialog.h"

// Qt includes
#include <QtGui/QFileDialog>

// NeL includes

// Project includes
#include "modules.h"
#include "entity.h"

namespace NLQT
{

CAnimationSetDialog::CAnimationSetDialog(QWidget *parent)
	: QDockWidget(parent)
{
	ui.setupUi(this);

	connect(ui.addToolButton, SIGNAL(clicked()), this, SLOT(addAnim()));
	connect(ui.removeToolButton, SIGNAL(clicked()), this, SLOT(removeAnim()));
	connect(ui.upToolButton, SIGNAL(clicked()), this, SLOT(upAnim()));
	connect(ui.downToolButton, SIGNAL(clicked()), this, SLOT(downAnim()));
	connect(ui.addAnimPushButton, SIGNAL(clicked()), this, SLOT(loadAnim()));
	connect(ui.addSwtPushButton, SIGNAL(clicked()), this, SLOT(loadSwt()));
	connect(ui.resetToolButton, SIGNAL(clicked()), this, SLOT(resetAnim()));

	connect(ui.objectsComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setCurrentShape(QString)));
}

CAnimationSetDialog::~CAnimationSetDialog()
{
}

void CAnimationSetDialog::setCurrentShape(const QString &name)
{
	if (name.isEmpty())
		return;

	Modules::objView().setCurrentObject(name.toUtf8().constData());

	updateListAnim();

	Q_EMIT changeCurrentShape(name);

}

void CAnimationSetDialog::updateListObject()
{
	ui.objectsComboBox->clear();

	std::vector<std::string> listObjects;
	Modules::objView().getListObjects(listObjects);

	for (size_t i = 0; i < listObjects.size(); ++i)
		ui.objectsComboBox->addItem(QString(listObjects[i].c_str()));

	if (listObjects.empty())
	{
		ui.addAnimPushButton->setEnabled(false);
		ui.addSwtPushButton->setEnabled(false);
		ui.resetToolButton->setEnabled(false);
		ui.setLengthToolButton->setEnabled(false);
	}
	else
	{
		ui.addAnimPushButton->setEnabled(true);
		ui.addSwtPushButton->setEnabled(true);
		ui.resetToolButton->setEnabled(true);
		ui.setLengthToolButton->setEnabled(true);
	}
}

void CAnimationSetDialog::updateListAnim()
{
	ui.animTreeWidget->clear();
	ui.animPlaylistWidget->clear();
	ui.skeletonTreeWidget->clear();

	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
		return;
	CEntity	&entity = Modules::objView().getEntity(curObj);

	std::vector<std::string> &animationList = entity.getAnimationList();
	std::vector<std::string> &swtList = entity.getSWTList();
	std::vector<std::string> &playListAnimation = entity.getPlayListAnimation();

	// update animation list widget
	for(size_t i = 0; i < animationList.size(); ++i)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem(ui.animTreeWidget);
		item->setText(0, QString(animationList[i].c_str()));
	}

	// update skeleton weight template list widget
	for(size_t i = 0; i < swtList.size(); ++i)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem(ui.skeletonTreeWidget);
		item->setText(0, QString(swtList[i].c_str()));
	}

	// update PlayList animation widget
	for(size_t i = 0; i < playListAnimation.size(); ++i)
	{
		QListWidgetItem *item = new QListWidgetItem(ui.animPlaylistWidget);
		item->setText(QString(playListAnimation[i].c_str()));
	}

	if (animationList.empty())
	{
		// lock buttons
		ui.addToolButton->setEnabled(false);
		ui.removeToolButton->setEnabled(false);
		ui.upToolButton->setEnabled(false);
		ui.downToolButton->setEnabled(false);
	}
	else
	{
		// unlock buttons
		ui.addToolButton->setEnabled(true);
		ui.removeToolButton->setEnabled(true);
		ui.upToolButton->setEnabled(true);
		ui.downToolButton->setEnabled(true);
	}
}

void CAnimationSetDialog::loadAnim()
{
	CEntity	&entity = Modules::objView().getEntity(Modules::objView().getCurrentObject());

	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL anim file"), ".",
							tr("NeL anim files (*.anim);;"));

	setCursor(Qt::WaitCursor);

	if (!fileNames.isEmpty())
	{
		QStringList list = fileNames;
		QStringList::Iterator it = list.begin();
		while(it != list.end())
		{
			entity.loadAnimation(it->toUtf8().constData());
			++it;
		}
		updateListAnim();
	}

	setCursor(Qt::ArrowCursor);
}

void CAnimationSetDialog::loadSwt()
{
	CEntity	&entity = Modules::objView().getEntity(Modules::objView().getCurrentObject());

	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL anim file"), ".",
							tr("NeL Skeleton Weight Template files (*.swt);;"));

	setCursor(Qt::WaitCursor);

	if (!fileNames.isEmpty())
	{
		QStringList list = fileNames;
		QStringList::Iterator it = list.begin();
		while(it != list.end())
		{
			entity.loadSWT(it->toUtf8().constData());
			++it;
		}
		updateListAnim();
	}

	setCursor(Qt::ArrowCursor);
}

void CAnimationSetDialog::resetAnim()
{
	CEntity	&entity = Modules::objView().getEntity(Modules::objView().getCurrentObject());
	entity.reset();

	updateListAnim();
}

void CAnimationSetDialog::addAnim()
{
	CEntity	&entity = Modules::objView().getEntity(Modules::objView().getCurrentObject());
	QList<QTreeWidgetItem *> list = ui.animTreeWidget->selectedItems();

	Q_FOREACH(QTreeWidgetItem *item, list)
	{
		entity.addAnimToPlayList(item->text(0).toUtf8().constData());
		ui.animPlaylistWidget->addItem(item->text(0));
	}
}

void CAnimationSetDialog::removeAnim()
{
	CEntity	&entity = Modules::objView().getEntity(Modules::objView().getCurrentObject());
	QList<QListWidgetItem *> list = ui.animPlaylistWidget->selectedItems();

	Q_FOREACH(QListWidgetItem *item, list)
	{
		int row = ui.animPlaylistWidget->row(item);
		QListWidgetItem *removeItem = ui.animPlaylistWidget->takeItem(row);
		if (!removeItem)
			delete removeItem;
		entity.removeAnimToPlayList(row);
	}
}

void CAnimationSetDialog::upAnim()
{
	CEntity	&entity = Modules::objView().getEntity(Modules::objView().getCurrentObject());
	QList<QListWidgetItem *> list = ui.animPlaylistWidget->selectedItems();

	if (list.empty())
		return;

	int frontRow = ui.animPlaylistWidget->row(list.front());
	int backRow = ui.animPlaylistWidget->row(list.back());

	if (frontRow == 0)
		return;

	QListWidgetItem *item = ui.animPlaylistWidget->takeItem(frontRow - 1);
	ui.animPlaylistWidget->insertItem(backRow, item);

	for (int i = frontRow; i <= backRow; ++i)
		entity.swapAnimToPlayList(i - 1, i);
}

void CAnimationSetDialog::downAnim()
{
	CEntity	&entity = Modules::objView().getEntity(Modules::objView().getCurrentObject());
	QList<QListWidgetItem *> list = ui.animPlaylistWidget->selectedItems();

	if (list.empty())
		return;

	int frontRow = ui.animPlaylistWidget->row(list.front());
	int backRow = ui.animPlaylistWidget->row(list.back());

	if (backRow == (ui.animPlaylistWidget->count() - 1))
		return;

	QListWidgetItem *item = ui.animPlaylistWidget->takeItem(backRow + 1);
	ui.animPlaylistWidget->insertItem(frontRow, item);

	for (int i = backRow; i >= frontRow; --i)
		entity.swapAnimToPlayList(i, i + 1);
}

} /* namespace NLQT */
