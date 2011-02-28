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
#include "value_gradient_dialog.h"

// Qt include
#include <QtGui/QMessageBox>

// NeL includes
#include <nel/3d/texture_grouped.h>
#include "nel/3d/texture_file.h"
#include "nel/misc/path.h"

// Projects include
#include "particle_texture_widget.h"

namespace NLQT
{

CGradientDialog::CGradientDialog(CWorkspaceNode *ownerNode,
								 IValueGradientClient *clientInterface,
								 bool destroyClientInterface,
								 bool canTuneNbStages,
								 uint minSize,
								 QWidget *parent)
	: QDialog(parent),
	  _MinSize(minSize),
	  _CanTuneNbStages(canTuneNbStages),
	  _ClientInterface(clientInterface),
	  _DestroyClientInterface(destroyClientInterface),
	  _Node(ownerNode)
{
	nlassert(_ClientInterface);

	resize(490, 210);
	_gridLayout = new QGridLayout(this);
	_listWidget = new QListWidget(this);
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(_listWidget->sizePolicy().hasHeightForWidth());
	_listWidget->setSizePolicy(sizePolicy);
	//_listWidget->setIconSize(QSize(16, 16));
	_listWidget->setMaximumSize(QSize(185, 16777215));
	_gridLayout->addWidget(_listWidget, 0, 0, 9, 1);

	_horizontalLayout = new QHBoxLayout();
	_addPushButton = new QPushButton(this);
	QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(_addPushButton->sizePolicy().hasHeightForWidth());
	_addPushButton->setSizePolicy(sizePolicy1);
	_addPushButton->setMaximumSize(QSize(36, 36));
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/images/list-add.png"), QSize(), QIcon::Normal, QIcon::Off);
	_addPushButton->setIcon(icon);
	_addPushButton->setIconSize(QSize(24, 24));
	_horizontalLayout->addWidget(_addPushButton);

	_removePushButton = new QPushButton(this);
	sizePolicy1.setHeightForWidth(_removePushButton->sizePolicy().hasHeightForWidth());
	_removePushButton->setSizePolicy(sizePolicy1);
	_removePushButton->setMinimumSize(QSize(0, 0));
	_removePushButton->setMaximumSize(QSize(36, 36));
	QIcon icon1;
	icon1.addFile(QString::fromUtf8(":/images/list-remove.png"), QSize(), QIcon::Normal, QIcon::Off);
	_removePushButton->setIcon(icon1);
	_removePushButton->setIconSize(QSize(24, 24));
	_horizontalLayout->addWidget(_removePushButton);

	_upPushButton = new QPushButton(this);
	sizePolicy1.setHeightForWidth(_upPushButton->sizePolicy().hasHeightForWidth());
	_upPushButton->setSizePolicy(sizePolicy1);
	_upPushButton->setMaximumSize(QSize(36, 36));
	QIcon icon2;
	icon2.addFile(QString::fromUtf8(":/images/go-up.png"), QSize(), QIcon::Normal, QIcon::Off);
	_upPushButton->setIcon(icon2);
	_upPushButton->setIconSize(QSize(24, 24));
	_horizontalLayout->addWidget(_upPushButton);

	_downPushButton = new QPushButton(this);
	sizePolicy1.setHeightForWidth(_downPushButton->sizePolicy().hasHeightForWidth());
	_downPushButton->setSizePolicy(sizePolicy1);
	_downPushButton->setMaximumSize(QSize(36, 36));
	QIcon icon3;
	icon3.addFile(QString::fromUtf8(":/images/go-down.png"), QSize(), QIcon::Normal, QIcon::Off);
	_downPushButton->setIcon(icon3);
	_downPushButton->setIconSize(QSize(24, 24));
	_horizontalLayout->addWidget(_downPushButton);

	_horizontalSpacer = new QSpacerItem(208, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	_horizontalLayout->addItem(_horizontalSpacer);
	_gridLayout->addLayout(_horizontalLayout, 0, 1, 1, 2);

	_label = new QLabel(this);
	_gridLayout->addWidget(_label, 1, 1, 1, 1);

	_nbStepWidget = new NLQT::CEditRangeUIntWidget(this);
	_gridLayout->addWidget(_nbStepWidget, 2, 1, 1, 2);

	_verticalSpacer = new QSpacerItem(20, 85, QSizePolicy::Minimum, QSizePolicy::Expanding);
	_gridLayout->addItem(_verticalSpacer, 3, 2, 1, 1);

	editWidget = _ClientInterface->createDialog(this);
	_gridLayout->addWidget(editWidget, 4, 1, 1, 2);

	setWindowTitle(_ClientInterface->getTitleDialog());
	_label->setText(tr("Num samples:"));

	if (canTuneNbStages)
	{
		_NbStepWrapper.OwnerNode = _Node;
		_NbStepWrapper.I = _ClientInterface;
		_nbStepWidget->setRange(1, 255);
		_nbStepWidget->enableLowerBound(0, true);
		_nbStepWidget->setWrapper(&_NbStepWrapper);
		_nbStepWidget->updateUi();
	}
	else
	{
		_nbStepWidget->hide();
		_label->hide();
	}

	connect(_addPushButton, SIGNAL(clicked()), this, SLOT(addValue()));
	connect(_removePushButton, SIGNAL(clicked()), this, SLOT(removeValue()));
	connect(_upPushButton, SIGNAL(clicked()), this, SLOT(valueUp()));
	connect(_downPushButton, SIGNAL(clicked()), this, SLOT(valueDown()));
	connect(_listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(changeCurrentRow(int)));
	connect(clientInterface, SIGNAL(itemChanged()), this, SLOT(updateItem()));

	_Size = _ClientInterface->getSchemeSize();
	for (uint k = 0; k < _Size; ++k)
	{
		QListWidgetItem *item = new QListWidgetItem();
		_ClientInterface->displayValue(k, item);
		_listWidget->addItem(item);
	}
	_removePushButton->setEnabled(_Size > _MinSize ? true : false);
	_listWidget->setCurrentRow(0);
}

CGradientDialog::~CGradientDialog()
{
	if (_DestroyClientInterface) delete _ClientInterface;
}

void CGradientDialog::addValue()
{
	nlassert(_ClientInterface);
	if (!_ClientInterface->modifyGradient(IValueGradientClient::Add, 0)) return;
	++_Size;
	QListWidgetItem *item = new QListWidgetItem();
	_ClientInterface->displayValue(_Size - 1, item);
	_listWidget->addItem(item);

	_removePushButton->setEnabled(true);

	_listWidget->setCurrentRow(_Size - 1);
}

void CGradientDialog::insertValue()
{
	nlassert(_ClientInterface);
	int oldIndex = _listWidget->currentRow();
	if (!_ClientInterface->modifyGradient(IValueGradientClient::Insert, oldIndex)) return;
	++_Size;
	QListWidgetItem *item = new QListWidgetItem();
	_ClientInterface->displayValue(_Size - 1, item);
	_listWidget->insertItem(oldIndex, item);
	_listWidget->setCurrentRow(oldIndex);
}

void CGradientDialog::removeValue()
{
	nlassert(_ClientInterface);

	int oldIndex = _listWidget->currentRow();
	if (oldIndex == -1)
		return;

	if (uint(oldIndex) == 0)
		_listWidget->setCurrentRow(oldIndex + 1);
	else
		_listWidget->setCurrentRow(oldIndex - 1);

	if (!_ClientInterface->modifyGradient(IValueGradientClient::Delete, oldIndex))
	{
		_listWidget->setCurrentRow(oldIndex);
		return;
	}

	--_Size;

	if (_Size <= _MinSize)
		_removePushButton->setEnabled(false);

	QListWidgetItem *removeItem = _listWidget->takeItem(oldIndex);
	if (!removeItem)
		delete removeItem;
}

void CGradientDialog::valueDown()
{
	nlassert(_ClientInterface);
	int currentRow = _listWidget->currentRow();
	if (!((currentRow == _listWidget->count()-1) || (currentRow == -1)))
	{
		if (!_ClientInterface->modifyGradient(IValueGradientClient::Down, currentRow)) return;
		QListWidgetItem *item = _listWidget->takeItem(currentRow);
		_listWidget->insertItem(++currentRow, item);
		_listWidget->setCurrentRow(currentRow);
	}
	_listWidget->setCurrentRow(currentRow);
}

void CGradientDialog::valueUp()
{
	nlassert(_ClientInterface);
	int currentRow = _listWidget->currentRow();
	if (!((currentRow == 0) || (currentRow == -1)))
	{
		if (!_ClientInterface->modifyGradient(IValueGradientClient::Up, currentRow)) return;
		QListWidgetItem *item = _listWidget->takeItem(currentRow);
		_listWidget->insertItem(--currentRow, item);
		_listWidget->setCurrentRow(currentRow);
	}
	_listWidget->setCurrentRow(currentRow);
}

void CGradientDialog::changeCurrentRow(int currentRow)
{
	_ClientInterface->setCurrentIndex(currentRow);
}

void CGradientDialog::updateItem()
{
	_ClientInterface->displayValue(_listWidget->currentRow(), _listWidget->currentItem());
}

QWidget *CTextureGradientInterface::createDialog(QWidget *parent)
{
	editWidget = new CParticleTextureWidget();

	_TextureWrapper.P = TP;
	_TextureWrapper.Index = 0;
	_TextureWrapper.OwnerNode = Node;
	editWidget->setWrapper(&_TextureWrapper);
	connect(editWidget, SIGNAL(textureChanged(QString)), this, SIGNAL(itemChanged()));
	return editWidget;
}

bool CTextureGradientInterface::modifyGradient(TAction action, uint index)
{
	nlassert(TP);
	nlassert(TP->getTextureGroup());

	NLMISC::CSmartPtr<NL3D::ITexture> tex = TP->getTextureGroup()->getTexture(index);
	std::string texName = (static_cast<NL3D::CTextureFile *>(tex.getPtr()))->getFileName().c_str();
	if (texName.empty())
		return false;

	std::vector< NLMISC::CSmartPtr<NL3D::ITexture> > textureList;
	textureList.resize(TP->getTextureGroup()->getNbTextures());
	TP->getTextureGroup()->getTextures(&textureList[0]);

	switch(action)
	{
	case IValueGradientClient::Add:
	{
		// we duplicate the last texture, so that they have the same size
		NLMISC::CSmartPtr<NL3D::ITexture> lastTex = textureList[textureList.size() - 1];
		textureList.push_back(lastTex);
	}
	break;
	case IValueGradientClient::Insert:
	{
		// we duplicate the current texture, so that they have the same size
		NLMISC::CSmartPtr<NL3D::ITexture> tex = textureList[index];
		textureList.insert(textureList.begin() + index, tex);
	}
	break;
	case IValueGradientClient::Delete:
		textureList.erase(textureList.begin() + index);
		break;
	case IValueGradientClient::Up:
		return false;
	case IValueGradientClient::Down:
		return false;
	}

	TP->getTextureGroup()->setTextures(&textureList[0], (uint)textureList.size());
	return true;
}

void CTextureGradientInterface::displayValue(uint index, QListWidgetItem *item)
{
	QPixmap pixmap;
	NLMISC::CSmartPtr<NL3D::ITexture> tex = TP->getTextureGroup()->getTexture(index);

	if (dynamic_cast<NL3D::CTextureFile *>(tex.getPtr()))
	{
		std::string texName = (static_cast<NL3D::CTextureFile *>(tex.getPtr()))->getFileName().c_str();
		if (!texName.empty())
		{
			pixmap.load(NLMISC::CPath::lookup(texName).c_str());
			item->setText(texName.c_str());
		}
		else
			item->setText("Dummy texture");
	}
	item->setIcon(QIcon(pixmap));
}

void CTextureGradientInterface::setCurrentIndex(uint index)
{
	_TextureWrapper.Index = index;
	editWidget->updateUi();
}

uint32 CTextureGradientInterface::getSchemeSize(void) const
{
	nlassert(TP->getTextureGroup());
	return TP->getTextureGroup()->getNbTextures();
}
uint32 CTextureGradientInterface::getNbSteps(void) const
{
	return 1;
}
void CTextureGradientInterface::setNbSteps(uint32 value)
{
	// this should never be called, as we don't allow nbsteps to be called
	nlassert(false);
}

NL3D::ITexture *CTextureGradientInterface::CTextureWrapper::get(void)
{
	nlassert(P);
	nlassert(P->getTextureGroup());
	return P->getTextureGroup()->getTexture(Index);
}

void CTextureGradientInterface::CTextureWrapper::set(NL3D::ITexture *t)
{
	nlassert(P);
	nlassert(P->getTextureGroup());

	// if a texture is added, it must have the same size than other textures
	if (P->getTextureGroup()->getNbTextures() > 1)
	{
		NLMISC::CSmartPtr<NL3D::ITexture> tex = P->getTextureGroup()->getTexture(0);
		tex->generate();
		t->generate();

		if (t->getWidth() != tex->getWidth() || t->getHeight() != tex->getHeight())
		{
			QMessageBox::critical(0, QString("Texture error"), QString("All textures must have the same size !"),  QMessageBox::Ok);
			return;
		}

		if (t->PixelFormat != tex->PixelFormat)
		{
			QMessageBox::critical(0, QString("Texture error"), QString("All textures must have the same pixel format !"),  QMessageBox::Ok);
			return;
		}
	}

	std::vector< NLMISC::CSmartPtr<NL3D::ITexture> > textureList;
	textureList.resize(P->getTextureGroup()->getNbTextures());
	P->getTextureGroup()->getTextures(&textureList[0]);

	textureList[Index] = t;


	P->getTextureGroup()->setTextures(&textureList[0], (uint)textureList.size());
}

} /* namespace NLQT */